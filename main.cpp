#include <iostream>
#include <filesystem>

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

#include "raytracelib/raytrace.h"
#include "texture.h"


struct app_state_t;
using glm::normalize;

constexpr int default_max_bounces = 50;

#ifdef THREADS
std::atomic_bool g_quit_program = false;

// get processor count. hardware_concurrency can return 0 though, so if it does, just assume 4 processors
const auto processor_count = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 4;
#else
const auto processor_count = 1;
bool g_quit_program = false;
#endif



/**
 * \brief Stores general configuration information for the render
 */
struct render_config_t {
    render_config_t(scene_t scene, int default_samples_per_pixel, int default_num_threads):
        num_threads(default_num_threads), samples_per_pixel(default_samples_per_pixel),
        max_bounces(default_max_bounces),
        scn(std::move(scene)) {}

    // if this is turned on, half the threads start from the top of the screen
    // and half from the bottom, which lets you see the general image a bit quicker.
    // This only works if there's an even number of threads and the height of the screen
    // is even. 
    bool interlace = true;

    int num_threads;
    int samples_per_pixel;

    /**
     * the maximum number of bounces a ray_t can go through. Note that this is limited
     * by the stack size since the ray_color function is recursive. 
     */
    int max_bounces;

    /**
     * \brief The scaling factor for pixels.
     *  1 = window dimensions,
     *  2 = window dimensions / 2,
     *  3 = window dimensions / (2*2),
     *  4 = window dimensions / (2*2*2), 
     */
    int scale=1;

    scene_t scn;
};

enum class render_state_t {
    inactive,
    rendering,
    cancelled,
    done,
    cleanup
};

/**
 * \brief converts a render_state_t to a string;
 * \param state the state enumeration value
 * \return stringified render_state_t
 */
std::string render_state_to_string(const render_state_t state) {
    switch(state) {
        case render_state_t::inactive: return "inactive";
        case render_state_t::rendering: return "rendering";
        case render_state_t::cancelled: return "cancelled";
        case render_state_t::done: return "done";
        case render_state_t::cleanup: return "cleanup";
    }
    assert(false);
    return "undefined";
}

#ifdef THREADS

/**
 * \brief Stores the information needed for tracking
 * and managing the progress of a multi-threaded render
 */
class threaded_render_state_t {
public:

    threaded_render_state_t() = delete;
    threaded_render_state_t(render_state_t state): m_state(state) {}
    ~threaded_render_state_t() {
        m_state = render_state_t::cleanup;
        for(auto& t : m_render_threads) {
            if(t.joinable()) {
                t.join();
            }
        }
    }

    void cancel() { m_state = render_state_t::cancelled; }
    void finish() { m_state = render_state_t::done; }

    [[nodiscard]] double time_ns() const { return m_time_ns; }
    void add_time_ns(double t) { m_time_ns += t; }

    [[nodiscard]] render_state_t state() const { return m_state; }

    [[nodiscard]] double progress() const { return static_cast<double>(m_pixels_rendered) / m_total_pixels;}

    void add_pixels_rendered(int num) { m_pixels_rendered += num; }

    void set_total_pixels(int total) { m_total_pixels = total; }
    
    [[nodiscard]] bool screen_needs_update() const { return m_screen_needs_update; }
    void mark_screen_for_update() { m_screen_needs_update = true; }
    void clear_update_flag() { m_screen_needs_update = false; }
       

    std::vector<std::thread> m_render_threads;

protected:
    std::atomic_int m_total_pixels = 1; // don't default to 0 to avoid divide by zero (should never happen, but better to be safe)
    std::atomic_int m_pixels_rendered = 0;
    std::atomic_bool m_screen_needs_update = false;

    // not atomic because only the main thread uses this
    double m_time_ns = 0.0;

    std::atomic<render_state_t> m_state = render_state_t::inactive;
};
typedef threaded_render_state_t render_status_t;

#else

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

    [[nodiscard]] double time_ns() const { return m_time_ns; }
    void add_time_ns(double t) { m_time_ns += t; }

protected:
    int m_x = 0;
    int m_y = 0;
    bool m_scanline_finished = false;
    double m_progress = 0.0;

    double m_time_ns = 0.0;

    render_state_t m_state = render_state_t::inactive;
};
typedef iterative_render_state_t render_status_t;
#endif


/**
 * \brief Stores most of the mutable state of the application. We're using this as a struct
 * instead of a class because both the emscripten loop function and the render_thread function
 * need to be function pointers, so using class methods would be a pain. 
 */
struct app_state_t {
    app_state_t(
        render_config_t config, 
        const shared_ptr<streaming_image_texture_t>& screen, 
        SDL_Renderer* renderer, 
        SDL_Window* window)
        : renderer(renderer), window(window), screen(screen), cfg(std::move(config)) {
#ifndef THREADS
        render_status = make_unique<iterative_render_state_t>(render_state_t::inactive);
#else 
        render_status = make_unique<threaded_render_state_t>(render_state_t::inactive);
#endif
    }

    void handle_resize() {
        int window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
        const int scaled_width = static_cast<int>(static_cast<double>(window_width) / pow(2, cfg.scale-1));
        const int scaled_height = static_cast<int>(static_cast<double>(window_height) / pow(2, cfg.scale-1));

        cfg.scn.cam.resize(scaled_width, scaled_height);
        screen->resize(scaled_width, scaled_height);

#ifdef THREADS
        render_status->set_total_pixels(scaled_width * scaled_height);
#endif
    }

    SDL_Renderer* renderer;
    SDL_Window* window;

    shared_ptr<streaming_image_texture_t> screen;

    render_config_t cfg;

    unique_ptr<render_status_t> render_status;
};


#ifndef THREADS
// This method is for rendering the image without blocking the
// window loop (so things stay responsive). It also lets you see
// the image as it renders instead of just the final result.
// This returns the time_ns taken in nanoseconds, and we keep calling
// this in the main loop as long as we're within our frame-budget.
// Once we exceed the budget, we handle ui & input and present to the
// screen, then resume again.
int64_t next_pixel(app_state_t& state) {
    if(state.render_status->state() != render_state_t::rendering) {
        return 0;
    }

    const auto start = std::chrono::system_clock::now();

    const auto& cfg = state.cfg;
    const auto& scn = state.cfg.scn;
    const auto& cam = scn.cam;
    auto& cs = state.render_status;
    
    color_t pixel_color;
    
    for(int s = 0; s<cfg.samples_per_pixel; s++) {
        double u = (cs->x()+random_double()) / static_cast<double>(cam.width()-1);
        double v = (cs->y()+random_double()) / static_cast<double>(cam.height()-1);
        ray_t r = scn.cam.get_ray(u, v);
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
    cs->add_time_ns(static_cast<double>(ns));
    return ns;
}
#endif

#ifdef THREADS
// This is the multi-threaded version of rendering. Each render thread handles
// a subset of the scan lines. To keep it simple, we just use a base and offset.
// So, if we had 4 threads, the first thread would handle lines (0, 4, 8, 12, ...) then
// the second thread will handle (1, 5, 9, 13, ...) and the third thread would handle
// (2, 6, 10, 14, ...) and so on.
void render_thread(app_state_t* state, int base, int offset) {
    std::stringstream msg;
    msg << "starting thread, base: " << base << ", offset: " << offset << std::endl;
    std::cout << msg.str();

    const auto& config = state->cfg;
    
    SDL_Delay(static_cast<int>(random_double(0, 250))); // stagger the thread timing a bit so they're not writing to the buffer all at the same time

    const auto& scn = config.scn;
    const auto& cam = scn.cam;
    auto scanline = std::make_unique<color_t[]>(state->screen->width());

    bool done = false;

    //for(int y = base; y < cam.height(); y += offset) {
    int y = base;
    while(!done) {

        if(offset > 0) {
            if(y > cam.height()-1) {
                done = true;
                break;
            }
        } else {
            if(y < 0) {
                done = true;
                break;
            }
        }

        for(int x = 0; x < cam.width(); x++) {
            color_t pixel_color;
    
            for(int s = 0; s<config.samples_per_pixel; s++) {
                const double u = (x+random_double()) / static_cast<double>(cam.width()-1);
                const double v = (y+random_double()) / static_cast<double>(cam.height()-1);
                ray_t r = scn.cam.get_ray(u, v);
                pixel_color += ray_color(r, scn, *scn.root.get(), config.max_bounces);
            }
            scanline[x] = pixel_color;

            if(g_quit_program 
                || state->render_status->state() == render_state_t::cancelled
                || state->render_status->state() == render_state_t::cleanup) {
                return;
            }
        }
        state->screen->image()->write_line_sync((cam.height()-1) - y, scanline.get(), config.samples_per_pixel);
        state->render_status->add_pixels_rendered(state->screen->width());
        state->render_status->mark_screen_for_update();

        if(g_quit_program 
            || state->render_status->state() == render_state_t::cancelled
            || state->render_status->state() == render_state_t::cleanup) {
            break;
        }

        y += offset;
    }
}
#endif

// Helper to display a little (?) mark which shows a tooltip when hovered.
static void help_marker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void loop_fn(void* arg) {
    auto state = static_cast<app_state_t*>(arg); 
    SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);

#ifndef THREADS

    // iterative solution
    uint64_t time_spent_rendering = 0;

    // 30 milliseconds (so we get ~30 fps for UI, assuming that the gui
    // rendering and texture copying take less than 2 ms)
    constexpr uint64_t rendering_budget = 30 * 1000000; 

    while(state->render_status->state() == render_state_t::rendering && time_spent_rendering < rendering_budget) {
        time_spent_rendering += next_pixel(*state);
    }

    if(state->render_status->is_scanline_finished()) {
        state->render_status->clear_scanline_flag();
        state->screen->update_texture_sync();
    }

#else
    const auto start = std::chrono::system_clock::now();

    if(state->render_status->screen_needs_update()) {
        state->screen->update_texture_sync();
        state->render_status->clear_update_flag();
    }

    if(state->render_status->progress() >= 1.0) {
        state->render_status->finish();
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
                render_gradient_pattern(*state->screen->image());
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
        const char* scenes[] {"Random Spheres", "Test Scene", "Earth", "Two Perlin Spheres", "Simple Light", "Simple Box", "Cornell Box"};
        if(ImGui::Combo("Scene", &current_scene, scenes, sizeof(scenes) / sizeof(const char*))) {
            if(current_scene == 0) {
                state->cfg.scn = random_scene(state->screen->width(), state->screen->height());
            } else if(current_scene == 1) {
                state->cfg.scn = three_spheres_scene(state->screen->width(), state->screen->height());
            } else  if(current_scene == 2) {
                state->cfg.scn = earth_scene(state->screen->width(), state->screen->height());
            }  if(current_scene == 3) {
                state->cfg.scn = two_perlin_spheres_scene(state->screen->width(), state->screen->height());
            }  if(current_scene == 4) {
                state->cfg.scn = simple_light(state->screen->width(), state->screen->height());
            }  if(current_scene == 5) {
                state->cfg.scn = simple_box(state->screen->width(), state->screen->height());
            }  if(current_scene == 6) {
                state->cfg.scn = cornell_box(state->screen->width(), state->screen->height());
            }
            
        }

        ImGui::EndDisabled();

        int w_width, w_height;
        SDL_GetWindowSize(state->window, &w_width, &w_height);
        ImGui::Text("Window Dimensions %d x %d", w_width, w_height);
        ImGui::Text("Render Dimensions %d x %d", state->screen->width(), state->screen->height());

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::BeginDisabled(state->render_status->state() == render_state_t::rendering);
#ifdef THREADS
        ImGui::SliderInt("Threads", &state->cfg.num_threads, 1, static_cast<int>(processor_count));

        bool can_interlace = state->cfg.num_threads % 2 != 0 || state->screen->height() % 2 != 0;
        ImGui::BeginDisabled(can_interlace);
        ImGui::Checkbox("Interlaced Rendering", &state->cfg.interlace);
        ImGui::EndDisabled();
        ImGui::SameLine();
        help_marker("If enabled, renders from top and bottom simultaneously to resolve the image more quickly. Does not affect final quality.");
#endif

        if(ImGui::SliderInt("Pixel Scale", &state->cfg.scale, 1, 4)) {
            state->render_status = make_unique<render_status_t>(render_state_t::inactive);
            state->handle_resize();
            render_gradient_pattern(*state->screen->image());
            state->screen->update_texture_sync();
        }
        ImGui::SameLine();
        help_marker("The larger the pixel scale, the chunkier the pixels, but the faster it will render.");

        ImGui::SliderInt("Bounces", &state->cfg.max_bounces, 1, 2000);
        ImGui::SameLine();
        help_marker("More bounces results in higher quality, but slower render times.");

        ImGui::SliderInt("Samples Per Pixel", &state->cfg.samples_per_pixel, 1, 2000);
        ImGui::SameLine();
        help_marker("More samples results in higher quality, but slower render times.");

        ImGui::EndDisabled();

        
        ImGui::Text("State: %s, Time: %ds", render_state_to_string(state->render_status->state()).c_str(), static_cast<int>((state->render_status->time_ns() / 1000000.0) / 1000.0));

        ImVec4 color = (state->render_status->state() == render_state_t::rendering || state->render_status->state() == render_state_t::done)
                        ? ImVec4{ 66.0f/255.0f, 245.0f/255.0f, 191.0f/255.0f, 1.0}
                        : ImVec4{ 245.0f/255.0f, 66.0f/255.0f, 93.0f/255.0f, 1.0};

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
        ImGui::ProgressBar(static_cast<float>(state->render_status->progress()));
        ImGui::PopStyleColor();
        
        

        
        ImGui::BeginDisabled(state->render_status->state() == render_state_t::rendering);
        if (ImGui::Button("Render")) {
            std::cout << "start render" << std::endl;
            render_gradient_pattern(*state->screen->image());
            state->render_status = make_unique<render_status_t>(render_state_t::rendering);

#ifdef THREADS
            state->render_status->set_total_pixels(state->screen->width() * state->screen->height());

            bool can_interlace = state->cfg.num_threads % 2 == 0 && state->screen->height() % 2 == 0;
            can_interlace &= state->cfg.interlace;

            for(int i = 0; i<state->cfg.num_threads; i++) {
                int num_threads = state->cfg.num_threads;

                if(i%2 == 0 || !can_interlace) {
                    state->render_status->m_render_threads.emplace_back(&render_thread, state, i, num_threads);
                } else {
                    state->render_status->m_render_threads.emplace_back(&render_thread, state, state->screen->height()-i, -num_threads);
                }
            }
#endif
        } 
        ImGui::EndDisabled();
        
        ImGui::SameLine();

        ImGui::BeginDisabled(state->render_status->state() != render_state_t::rendering);

        if (ImGui::Button("Cancel")) {
            state->render_status->cancel();
        }

        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::BeginDisabled(state->render_status->state() == render_state_t::inactive || state->render_status->state() == render_state_t::rendering);
        if (ImGui::Button("Reset")) {
            state->render_status->cancel();
            state->render_status = make_unique<render_status_t>(render_state_t::inactive);
            state->handle_resize();
            render_gradient_pattern(*state->screen->image());
            state->screen->update_texture_sync();
        }
        ImGui::EndDisabled();

        
        ImGui::End();
    }

    ImGui::Render();

    state->screen->present();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(state->renderer);

#ifdef THREADS
    const auto finish = std::chrono::system_clock::now();

    if(state->render_status->state() == render_state_t::rendering) {
        const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
        state->render_status->add_time_ns(static_cast<double>(ns));
    }
#endif
}

// This is the naieve/simple ray trace (not iterative or threaded, so blocks the interface)
void raytrace_all_linear(const render_config_t& config, const shared_ptr<image_buffer_t>& screen) {
    for(int y = screen->height()-1; y>=0; --y) {
        std::cout << "\rScanlines remaining: " << (screen->height()-1) - y << std::endl;
        for(int x = screen->width()-1; x>=0; --x) {
            color_t pixel_color;

            for(int s = 0; s<config.samples_per_pixel; s++) {
                double u = (x+random_double()) / static_cast<double>(screen->width()-1);
                double v = (y+random_double()) / static_cast<double>(screen->height()-1);
                ray_t r = config.scn.cam.get_ray(u, v);
                pixel_color += ray_color(r, config.scn, config.scn.entities, config.max_bounces);
            }

            screen->write(x, (screen->height()-1)-y, pixel_color, config.samples_per_pixel);
        }
    }
}

int main(int argc, char* argv[])
{
    //print cwd
    std::cout << std::filesystem::current_path() << endl;

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
        "Ray Tracer",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        default_image_width,
        default_image_height,
        SDL_WINDOW_RESIZABLE);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    {
        render_config_t config(random_scene(default_image_width, default_image_height), default_samples_per_pixel, default_thread_count);
        auto screen = make_shared<streaming_image_texture_t>(renderer, default_image_width, default_image_height);

        // draw the test pattern so we don't have a black screen (also sanity
        // checks that our texture is ok)
        render_gradient_pattern(*screen->image());
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

        app_state_t state(config, screen, renderer, window);
        

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
