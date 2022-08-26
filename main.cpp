#include <iostream>

#include <SDL.h>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#ifdef THREADS
#include <thread>
#endif

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif


#include <sstream>

#include "camera.h"
#include "debug_utils.h"
#include "hittable.h"
#include "hittable_list.h"
#include "types.h"
#include "image_buffer.h"
#include "texture.h"
#include "ray.h"
#include "sphere.h"
#include "material.h"
#include "scene.h"

using glm::normalize;

constexpr int default_max_bounces = 50;

#ifdef THREADS
std::atomic_int g_dirty = 0;
std::atomic_bool g_quit_program = false;
const auto processor_count = std::thread::hardware_concurrency();
#else
const auto processor_count = 1;
bool g_quit_program = false;
#endif


color ray_color(const ray& r, const hittable& world, const int stack_depth, const int max_bounces) {
    hit_record rec{};

    if(stack_depth > max_bounces) {
        return {0, 0, 0, 1};
    }

    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, stack_depth+1, max_bounces);
        return {0,0,0};
    }
    const vec3_d unit_direction = normalize(r.direction());
    auto t = 0.5*(unit_direction.y + 1.0);
    return (1.0-t)*color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}


struct render_config {

    render_config(scene_t scene, int default_samples_per_pixel, int default_num_threads):
        samples_per_pixel(default_samples_per_pixel), max_bounces(default_max_bounces), num_threads(default_num_threads),
        scn(std::move(scene)) {}

    int num_threads;
    int samples_per_pixel;
    int max_bounces;
    int scale=1;

    scene_t scn;
};

enum class render_state_t {
    inactive,
    rendering,
    cancelled,
    done
};

std::string render_state_to_string(render_state_t state) {
    switch(state) {
        case render_state_t::inactive: return "inactive";
        case render_state_t::rendering: return "rendering";
        case render_state_t::cancelled: return "cancelled";
        case render_state_t::done: return "done";
    }
    assert(false);
    return "undefined";
}

#ifdef THREADS
class threaded_render_state_t {

protected:
    int total_pixels = 0;
};
#endif

class iterative_render_state_t {
public:

    iterative_render_state_t(render_state_t state): m_state(state) {}

    void cancel() { m_state = render_state_t::cancelled; }
    void finish() { m_state = render_state_t::done; }

    [[nodiscard]] bool is_scanline_finished() const { return m_scanline_finished; }
    void mark_scanline_finished() { m_scanline_finished = true; }
    void clear_scanline_flag() { m_scanline_finished = false; }

    [[nodiscard]] render_state_t state() const { return m_state; }

    [[nodiscard]] double progress() const { return m_progress;}
    void progress(double p) { m_progress = p; }

    [[nodiscard]] int x() const { return m_x; }
    void x(const int x) { m_x = x; }

    [[nodiscard]] int y() const { return m_y; }
    void y(const int y) { m_y = y; }

    [[nodiscard]] double time() const { return m_time; }
    void add_time(double t) { m_time += t; }

protected:
    int m_x = 0;
    int m_y = 0;
    bool m_scanline_finished = false;
    double m_progress = 0.0;

    double m_time = 0.0;

    render_state_t m_state = render_state_t::inactive;
};

struct app_state {

    app_state(
        render_config config, 
        const shared_ptr<streaming_image_texture_t>& screen, 
        SDL_Renderer* renderer, 
        SDL_Window* window)
        : renderer(renderer), window(window), screen(screen), cfg(std::move(config)) {
#ifndef THREADS
        render_status = make_unique<iterative_render_state_t>(render_state_t::inactive);
#else 
        render_status = make_unique<threaded_render_state_t>();
#endif
    }

    void handle_resize() {
        int window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
        int scaled_width = static_cast<int>(static_cast<double>(window_width) / pow(2, cfg.scale-1));
        int scaled_height = static_cast<int>(static_cast<double>(window_height) / pow(2, cfg.scale-1));

        cfg.scn.cam.resize(scaled_width, scaled_height);
        screen->resize(scaled_width, scaled_height);
    }

    SDL_Renderer* renderer;
    SDL_Window* window;

    shared_ptr<streaming_image_texture_t> screen;

    render_config cfg;

#ifndef THREADS
    unique_ptr<iterative_render_state_t> render_status;
#else
    unique_ptr<threaded_render_state_t> render_status;
#endif
    
#ifdef THREADS
    std::vector<std::thread> render_threads;
#endif
};


// this method is for rendering the image without blocking the
// window loop (so things stay responsive). Also lets you see
// the image as it renders instead of just the final result.
// returns the time taken in nanoseconds
int64_t next_pixel(app_state& state) {
    if(state.render_status->state() != render_state_t::rendering) {
        return 0;
    }

    const auto start = std::chrono::system_clock::now();

    const auto& cfg = state.cfg;
    const auto& scn = state.cfg.scn;
    const auto& cam = scn.cam;
    auto& cs = state.render_status;
    
    color pixel_color;
    
    for(int s = 0; s<cfg.samples_per_pixel; s++) {
        double u = (cs->x()+random_double()) / static_cast<double>(cam.width()-1);
        double v = (cs->y()+random_double()) / static_cast<double>(cam.height()-1);
        ray r = scn.cam.get_ray(u, v);
        pixel_color += ray_color(r, scn.entities, 0, cfg.max_bounces);
    }

    state.screen->image()->write(cs->x(), (cam.height()-1)-cs->y(), pixel_color, cfg.samples_per_pixel);

    cs->x(cs->x()+1);

    if(cs->x() >= cam.width()) {
        cs->x(0);
        cs->y(cs->y()+1);
        cs->mark_scanline_finished();
    }

    cs->progress(static_cast<double>(cs->x() + cs->y() * state.screen->width()) / static_cast<double>(state.screen->width()*state.screen->height()));

    if(cs->y() >= cam.height()) {
        cs->finish();
        cs->progress(1.0);
    }

    const auto finish = std::chrono::system_clock::now();
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
    cs->add_time(ns);
    return ns;
}

#ifdef THREADS
void render_thread(const app_state& state, int base, int offset) {
    std::stringstream msg;
    msg << "starting thread, base: " << base << ", offset: " << offset << std::endl;
    std::cout << msg.str();

    const auto& config = state.cfg;
    
    SDL_Delay(static_cast<int>(random_double(0, 250))); // stagger the thread timing a bit so they're not writing to the buffer all at the same time

    const auto& scn = config.scn;
    const auto& cam = scn.cam;
    auto scanline = std::make_unique<color[]>(state.screen->width());

    for(int y = base; y < cam.height(); y += offset) {
        for(int x = 0; x < cam.width(); x++) {
            color pixel_color;
    
            for(int s = 0; s<config.samples_per_pixel; s++) {
                const double u = (x+random_double()) / static_cast<double>(cam.width()-1);
                const double v = (y+random_double()) / static_cast<double>(cam.height()-1);
                ray r = scn.cam.get_ray(u, v);
                pixel_color += ray_color(r, scn.entities, 0, config.max_bounces);
            }
            scanline[x] = pixel_color;

            if(g_quit_program) {
                return;
            }
        }
        state.screen->image()->write_line_sync((cam.height()-1) - y, scanline.get(), config.samples_per_pixel);
        ++g_dirty;

        if(g_quit_program) {
            break;
        }
    }
}
#endif

void loop_fn(void* arg) {
    const auto state = static_cast<app_state*>(arg); 
    SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);

#ifndef THREADS

    // iterative solution
    uint64_t time_spent_rendering = 0;

    // 30 milliseconds (so we get ~30 fps for UI, assuming that the gui
    // rendering and texture copying take less than 2 ms)
    constexpr uint64_t rendering_budget = 30 * 1000000; 

    while(state->render_status->state() == render_state_t::rendering && time_spent_rendering<rendering_budget) {
        time_spent_rendering += next_pixel(*state);
    }

    if(state->render_status->is_scanline_finished()) {
        state->render_status->clear_scanline_flag();
        state->screen->update_texture_sync();
    }

#else
    if(g_dirty) {
        state->screen->update_texture_sync();
        g_dirty = 0;
    }
#endif

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT) {
            g_quit_program = true;
        }

        if (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_RESIZED) {
            int w_width, w_height;
            SDL_GetWindowSize(state->window, &w_width, &w_height);

            if(state->render_status->state() != render_state_t::rendering) {
                state->handle_resize();
                state->screen->update_texture_sync();
            }

            if(state->render_status->state() == render_state_t::inactive) {
                render_test_pattern(*state->screen->image());
                state->screen->update_texture_sync();
            }
            
        }

        if (event.type == SDL_WINDOWEVENT 
            && event.window.event == SDL_WINDOWEVENT_CLOSE 
            && event.window.windowID == SDL_GetWindowID(state->window)) {
            g_quit_program = true;
        }

    }

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Ray Tracer");

        ImGui::BeginDisabled(state->render_status->state() == render_state_t::rendering);

        static int current_scene = 0;
        const char* scenes[] {"Random Spheres", "Test Scene"};
        if(ImGui::Combo("Scene", &current_scene, scenes, 2)) {
            if(current_scene == 0) {
                state->cfg.scn = random_scene(state->screen->width(), state->screen->height());
            } else {
                state->cfg.scn = test_scene(state->screen->width(), state->screen->height());
            }
            
        }

        ImGui::EndDisabled();

        int w_width, w_height;
        SDL_GetWindowSize(state->window, &w_width, &w_height);
        ImGui::Text("Window Dimensions %d x %d", w_width, w_height);
        ImGui::Text("Render Dimensions %d x %d", state->screen->width(), state->screen->height());

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

#ifdef THREADS
        ImGui::SliderInt("Threads", &state->cfg.num_threads, 1, static_cast<int>(processor_count));
#endif

        if(ImGui::SliderInt("Scale", &state->cfg.scale, 1, 4)) {
            state->render_status = make_unique<iterative_render_state_t>(render_state_t::inactive);
            state->handle_resize();
            render_test_pattern(*state->screen->image());
            state->screen->update_texture_sync();
        }

        ImGui::SliderInt("Bounces", &state->cfg.max_bounces, 1, 200);
        ImGui::SliderInt("Samples Per Pixel", &state->cfg.samples_per_pixel, 1, 200);

        if(state->render_status->state() != render_state_t::inactive) {
            ImGui::Text("State: %s, Time: %ds", render_state_to_string(state->render_status->state()).c_str(), static_cast<int>((state->render_status->time() / 1000000.0) / 1000.0));

            ImGui::PushStyleColor(ImGuiCol_Header, {1.0, 0.0, 0.0, 1.0});
            ImGui::ProgressBar(state->render_status->progress());
            ImGui::PopStyleColor();
        }

        if (ImGui::Button("Render")) {
            std::cout << "start render" << std::endl;
            render_test_pattern(*state->screen->image());
            state->render_status = make_unique<iterative_render_state_t>(render_state_t::rendering);
        } 

        
        ImGui::SameLine();

        ImGui::BeginDisabled(state->render_status->state() != render_state_t::rendering);

        if (ImGui::Button("Cancel")) {
            state->render_status->cancel();
        }

        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::BeginDisabled(state->render_status->state() == render_state_t::inactive);
        if (ImGui::Button("Reset")) {
            state->render_status = make_unique<iterative_render_state_t>(render_state_t::inactive);
            state->handle_resize();
            render_test_pattern(*state->screen->image());
            state->screen->update_texture_sync();
        }
        ImGui::EndDisabled();

        
        ImGui::End();
    }

    ImGui::Render();

    state->screen->present();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(state->renderer);
}

// This is the naieve/simple ray trace (not iterative or threaded, so blocks the interface)
void raytrace_all_linear(const render_config& config, const shared_ptr<image_buffer_t>& screen) {
    for(int y = screen->height()-1; y>=0; --y) {
        std::cout << "\rScanlines remaining: " << (screen->height()-1) - y << std::endl;
        for(int x = screen->width()-1; x>=0; --x) {
            color pixel_color;

            for(int s = 0; s<config.samples_per_pixel; s++) {
                double u = (x+random_double()) / static_cast<double>(screen->width()-1);
                double v = (y+random_double()) / static_cast<double>(screen->height()-1);
                ray r = config.scn.cam.get_ray(u, v);
                pixel_color += ray_color(r, config.scn.entities, 0, config.max_bounces);
            }

            screen->write(x, (screen->height()-1)-y, pixel_color, config.samples_per_pixel);
        }
    }
}

int main(int argc, char* argv[])
{
    constexpr int default_image_width = 480*2;
    constexpr int default_image_height = 270*2;
    constexpr int default_samples_per_pixel = 75;
    // default to two threads if we can't detect the processor count, otherwise use
    // 3/4ths of the processors (we want to leave a few open so the OS is responsive)
    int default_thread_count = static_cast<int>(processor_count*0.75);
    if(default_thread_count <= 1) {
        default_thread_count = 2;
    }
        
    
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "SDL2Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        default_image_width,
        default_image_height,
        SDL_WINDOW_RESIZABLE);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    {
        render_config config(random_scene(default_image_width, default_image_height), default_samples_per_pixel, default_thread_count);
        auto screen = make_shared<streaming_image_texture_t>(renderer, default_image_width, default_image_height);

        // draw the test pattern so we don't have a black screen (also sanity
        // checks that our texture is ok)
        render_test_pattern(*screen->image());
        SDL_SetRenderDrawColor(renderer, 0, 20, 80, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        screen->update_texture_sync();
        screen->present();
        SDL_RenderPresent(renderer);
        
        // setup dear imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer_Init(renderer);
        

        SDL_SetRenderDrawColor(renderer, 0, 20, 80, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen->texture(), nullptr, nullptr);
        SDL_RenderPresent(renderer);

        app_state state(config, screen, renderer, window);

    #ifdef THREADS
        
        std::cout << "processor count " << default_thread_count << std::endl;
        std::vector<std::thread> threads;

        for(int i=0; i<default_thread_count; i++) {
            auto& t = threads.emplace_back(&render_thread, config, screen, i, default_thread_count);
        }

    #endif

    #ifdef EMSCRIPTEN
        std::cout << "using emscripten loop" << std::endl;
        emscripten_set_main_loop_arg(loop_fn, &state, -1, 1);
    #else
        while (!g_quit_program) {
            loop_fn(&state);
        }
    #endif

    #ifdef THREADS
        g_quit_program = true;
            
        for(auto& t : threads) {
            if(t.joinable())
                t.join();
        }
    #endif

        ImGui_ImplSDLRenderer_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
