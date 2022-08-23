#include <iostream>

#include <SDL.h>
#include <chrono>

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
#include "ray.h"
#include "sphere.h"
#include "material.h"

using glm::normalize;

constexpr int max_bounces = 50;

#ifdef THREADS
std::atomic_int g_dirty = 0;
std::atomic_bool g_quit_program = false;
const auto processor_count = std::thread::hardware_concurrency();
#endif

struct scene {
    hittable_list entities;
    camera cam;
};

color ray_color(const ray& r, const hittable& world, int stack_depth=0) {
    hit_record rec{};

    if(stack_depth > max_bounces) {
        return {0, 0, 0, 1};
    }

    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, stack_depth+1);
        return {0,0,0};
    }
    const vec3_d unit_direction = normalize(r.direction());
    auto t = 0.5*(unit_direction.y + 1.0);
    return (1.0-t)*color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list random_scene() {
    hittable_list world;

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

    return world;
}

hittable_list test_scene() {
    hittable_list world;

    auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto material_left   = make_shared<dielectric>(1.5);
    auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

    world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.0),   0.5, material_center));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0), -0.45, material_left));
    world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));
    return world;
}

struct render_config {

    render_config(const camera& _camera, const hittable_list& _world, int _image_width, int _image_height, int _samples_per_pixel):
        cam(_camera),
        world(_world),
        image_width(_image_width),
        image_height(_image_height),
        samples_per_pixel(_samples_per_pixel)
    {
        
    }

    int samples_per_pixel;
    int max_bounces;
    int image_width;
    int image_height;
    hittable_list world;
    camera cam;
};

struct render_status {

    render_status(const shared_ptr<image_buffer>& buffer): screen(buffer) {}
    int x = 0;
    int y = 0;
    shared_ptr<image_buffer> screen;
    bool done=false;
    bool scanline_finished = false;
};

struct loop_context {

    loop_context(render_config config, const shared_ptr<image_buffer>& buffer, SDL_Texture* texture, SDL_Renderer* renderer)
        : texture(texture), renderer(renderer), screen(buffer), cfg(std::move(config)), status(buffer) {
        int w, h;
        SDL_QueryTexture(texture, &texture_format, nullptr, &w, &h);
        assert(w == buffer->width());
        assert(h == buffer->height());
    }

    SDL_Texture* texture;
    uint32_t texture_format;

    SDL_Renderer* renderer;

    shared_ptr<image_buffer> screen;

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
    
    color pixel_color;
    
    for(int s = 0; s<cfg.samples_per_pixel; s++) {
        double u = (cs.x+random_double()) / static_cast<double>(cfg.image_width-1);
        double v = (cs.y+random_double()) / static_cast<double>(cfg.image_height-1);
        ray r = cfg.cam.get_ray(u, v);
        pixel_color += ray_color(r, cfg.world);
    }

    cs.screen->write(cs.x, (cfg.image_height-1)-cs.y, pixel_color, cfg.samples_per_pixel);

    cs.x++;
    if(cs.x >= cfg.image_width) {
        cs.x = 0;
        cs.y++;
        cs.scanline_finished = true;
    }

    if(cs.y >= cfg.image_height) {
        cs.done = true;
    }

    const auto finish = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
}

#ifdef THREADS
void render_thread(const render_config& config, const shared_ptr<image_buffer>& buffer, int base, int offset) {
    std::stringstream msg;
    msg << "starting thread, base: " << base << ", offset: " << offset << std::endl;
    std::cout << msg.str();

    auto scanline = std::make_unique<color[]>(buffer->width());

    for(int y = base; y < config.image_height; y += offset) {
        for(int x=0; x<config.image_width; x++) {
            color pixel_color;
    
            for(int s = 0; s<config.samples_per_pixel; s++) {
                const double u = (x+random_double()) / static_cast<double>(config.image_width-1);
                const double v = (y+random_double()) / static_cast<double>(config.image_height-1);
                ray r = config.cam.get_ray(u, v);
                pixel_color += ray_color(r, config.world);
            }
            scanline[x] = pixel_color;
        }
        buffer->write_line_sync((config.image_height-1) - y, scanline.get(), config.samples_per_pixel);
        ++g_dirty;

        if(g_quit_program) {
            break;
        }
    }
}
#endif

void loop_fn(void* arg) {
    const auto context = static_cast<loop_context*>(arg); 
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
        uint8_t* pixels;
        int pitch;
        SDL_LockTexture(context->texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);
        memcpy(pixels, context->screen->data(), context->cfg.image_width*context->cfg.image_height*4);
        SDL_UnlockTexture(context->texture);
    }
#else
    if(g_dirty) {
        context->screen->update_texture_sync(context->texture);
        g_dirty = 0;
    }
#endif

    SDL_RenderCopy(context->renderer, context->texture, nullptr, nullptr);
    SDL_RenderPresent(context->renderer);
}

// This is the naieve/simple ray trace (not iterative or threaded, so blocks the interface)
void raytrace_all_linear(const render_config& config, const shared_ptr<image_buffer>& screen) {
    for(int y = screen->height()-1; y>=0; --y) {
        std::cout << "\rScanlines remaining: " << (screen->height()-1) - y << std::endl;
        for(int x = screen->width()-1; x>=0; --x) {
            color pixel_color;

            for(int s = 0; s<config.samples_per_pixel; s++) {
                double u = (x+random_double()) / static_cast<double>(screen->width()-1);
                double v = (y+random_double()) / static_cast<double>(screen->height()-1);
                ray r = config.cam.get_ray(u, v);
                pixel_color += ray_color(r, config.world);
            }

            screen->write(x, (screen->height()-1)-y, pixel_color, config.samples_per_pixel);
        }
    }
}

int main(int argc, char* argv[])
{
    constexpr int image_width = 480;
    constexpr int image_height = 270;
    constexpr int samples_per_pixel = 75;
        
    auto screen = make_shared<image_buffer>(image_width, image_height);

    // world
    auto R = cos(pi/4);
    
    /*
    point3 look_from(3,3,2);
    point3 look_at(0,0,-1);
    vec3_d vup(0,1,0);
    auto dist_to_focus = glm::length(look_from-look_at);
    auto aperture = 1.0;*/
            
    point3 look_from(13,2,3);
    point3 look_at(0,0,0);
    vec3_d vup(0,1,0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;

    camera cam(
        image_width, 
        image_height, 
        60.0, 
        look_from, 
        look_at, 
        vup,
        aperture,
        dist_to_focus);

    render_config config(cam, random_scene(), image_width, image_height, samples_per_pixel);
    
    render_test_pattern(*screen);

    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "SDL2Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        image_width,
        image_height,
        SDL_WINDOW_RESIZABLE);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        image_width, 
        image_height);

    SDL_UpdateTexture(
        texture, 
        nullptr, 
        screen->data(), 
        screen->pitch());


    SDL_SetRenderDrawColor(renderer, 0, 20, 80, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    loop_context context(config, screen, texture, renderer);

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
        //t.detach();
        SDL_Delay(100); // stagger the thread timing a bit so they're not writing to the buffer all at the same time
    }

#endif

#ifdef EMSCRIPTEN
    std::cout << "using emscripten loop" << std::endl;
    emscripten_set_main_loop_arg(loop_fn, &context, -1, 1);
#else
    while (true) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;

        loop_fn(&context);
    }
#endif
    g_quit_program = true;

    for(auto& t : threads) {
        if(t.joinable())
            t.join();
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}