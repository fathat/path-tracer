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

using glm::normalize;

constexpr int default_max_bounces = 50;

#ifdef THREADS
std::atomic_int g_dirty = 0;
std::atomic_bool g_quit_program = false;
const auto processor_count = std::thread::hardware_concurrency();
#else
bool g_quit_program = false;
#endif

struct scene {
    scene(const camera& camera, const hittable_list_t& ents): cam(camera), entities(ents) {}
    hittable_list_t entities;
    camera cam;
};

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

scene random_scene(int image_width, int image_height) {
    hittable_list_t world;

    const point3 look_from(13,2,3);
    const point3 look_at(0,0,0);
    const vec3_d vup(0,1,0);
    const auto dist_to_focus = 10.0;
    const auto aperture = 0.1;

    camera cam(
        image_width, 
        image_height, 
        60.0, 
        look_from, 
        look_at, 
        vup,
        aperture,
        dist_to_focus);

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return {cam, world};
}

scene test_scene(int image_width, int image_height) {
    hittable_list_t world;

    constexpr point3 look_from(3,3,2);
    constexpr point3 look_at(0,0,-1);
    constexpr vec3_d vup(0,1,0);
    const auto dist_to_focus = glm::length(look_from-look_at);
    constexpr auto aperture = 1.0;

    camera cam {image_width, image_height, 60.0, look_from, look_at, vup, aperture, dist_to_focus};


    auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto material_left   = make_shared<dielectric>(1.5);
    auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

    world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.0),   0.5, material_center));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0), -0.45, material_left));
    world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));
    return {cam, world};
}

struct render_config {

    render_config(scene scene, int samples_per_pixel):
        samples_per_pixel(samples_per_pixel), max_bounces(default_max_bounces),
        scn(std::move(scene)) {}

    int samples_per_pixel;
    int max_bounces;
    scene scn;
};

struct render_status {

    render_status(const shared_ptr<streaming_image_texture_t>& screen): screen(screen) {}
    int x = 0;
    int y = 0;
    shared_ptr<streaming_image_texture_t> screen;
    bool done=false;
    bool scanline_finished = false;
};

struct app_state {

    app_state(
        render_config config, 
        const shared_ptr<streaming_image_texture_t>& screen, 
        SDL_Renderer* renderer, 
        SDL_Window* window)
        : renderer(renderer), window(window), screen(screen), cfg(std::move(config)), status(screen) {}

    SDL_Renderer* renderer;
    SDL_Window* window;

    shared_ptr<streaming_image_texture_t> screen;

    render_config cfg;
    render_status status;
    
};


// this method is for rendering the image without blocking the
// window loop (so things stay responsive). Also lets you see
// the image as it renders instead of just the final result.
// returns the time taken in nanoseconds
int64_t next_pixel(const render_config& cfg, render_status& cs) {

    if(cs.done) {
        return 0;
    }

    const auto start = std::chrono::system_clock::now();

    const auto& scn = cfg.scn;
    const auto& cam = scn.cam;
    
    color pixel_color;
    
    for(int s = 0; s<cfg.samples_per_pixel; s++) {
        double u = (cs.x+random_double()) / static_cast<double>(cam.width()-1);
        double v = (cs.y+random_double()) / static_cast<double>(cam.height()-1);
        ray r = scn.cam.get_ray(u, v);
        pixel_color += ray_color(r, scn.entities, 0, cfg.max_bounces);
    }

    cs.screen->image()->write(cs.x, (cam.height()-1)-cs.y, pixel_color, cfg.samples_per_pixel);

    cs.x++;
    if(cs.x >= cam.width()) {
        cs.x = 0;
        cs.y++;
        cs.scanline_finished = true;
    }

    if(cs.y >= cam.height()) {
        cs.done = true;
    }

    const auto finish = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
}

#ifdef THREADS
void render_thread(const render_config& config, const shared_ptr<streaming_image_texture_t>& screen, int base, int offset) {
    std::stringstream msg;
    msg << "starting thread, base: " << base << ", offset: " << offset << std::endl;
    std::cout << msg.str();

    SDL_Delay(static_cast<int>(random_double(0, 250))); // stagger the thread timing a bit so they're not writing to the buffer all at the same time

    const auto& scn = config.scn;
    const auto& cam = scn.cam;
    auto scanline = std::make_unique<color[]>(screen->width());

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
        screen->image()->write_line_sync((cam.height()-1) - y, scanline.get(), config.samples_per_pixel);
        ++g_dirty;

        if(g_quit_program) {
            break;
        }
    }
}
#endif

void loop_fn(void* arg) {
    const auto context = static_cast<app_state*>(arg); 
    SDL_SetRenderDrawColor(context->renderer, 255, 255, 255, 255);

#ifndef THREADS
    // iterative solution
    uint64_t time_spent_rendering = 0;
    constexpr uint64_t rendering_budget = 30 * 1000000; // 30 milliseconds (so we get ~30 fps for UI)
    while(!context->status.done && time_spent_rendering<rendering_budget) {
        time_spent_rendering += next_pixel(context->cfg, context->status);
    }

    if(context->status.scanline_finished) {
        context->status.scanline_finished = false;
        context->screen->update_texture_sync();
    }
#else
    if(g_dirty) {
        context->screen->update_texture_sync();
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

        if (event.type == SDL_WINDOWEVENT 
            && event.window.event == SDL_WINDOWEVENT_CLOSE 
            && event.window.windowID == SDL_GetWindowID(context->window)) {
            g_quit_program = true;
        }

    }

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    {
        static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        static bool show_demo_window=false;
        static bool show_another_window=false;
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Ray Tracer");

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Render"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            std::cout << "cancelled" << std::endl;
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();

    context->screen->present();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(context->renderer);
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
        render_config config(random_scene(default_image_width, default_image_height), default_samples_per_pixel);
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
        // default to two threads if we can't detect the processor count, otherwise use
        // 3/4ths of the processors (we want to leave a few open so the OS is responsive)
        int pcount = static_cast<int>(processor_count*0.75);
        if(pcount <= 1) {
            pcount = 2;
        }
        std::cout << "processor count " << pcount << std::endl;
        std::vector<std::thread> threads;

        for(int i=0; i<pcount; i++) {
            auto& t = threads.emplace_back(&render_thread, config, screen, i, pcount);
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
