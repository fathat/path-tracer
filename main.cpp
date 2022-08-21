#include <iostream>

#include <SDL.h>

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "types.h"
#include "image_buffer.h"
#include "ray.h"
#include "sphere.h"
#include "material.h"

using glm::normalize;

constexpr int max_bounces = 50;

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


int main(int argc, char* argv[])
{
    constexpr size_t image_width = 480;
    constexpr size_t image_height = 270;
    constexpr int samples_per_pixel = 50;
        
    image_buffer image_buffer(image_width, image_height);

    // world
    auto R = cos(pi/4);
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

    point3 look_from(3,3,2);
    point3 look_at(0,0,-1);
    vec3_d vup(0,1,0);
    auto dist_to_focus = glm::length(look_from-look_at);
    auto aperture = 1.0;

    camera cam(
        image_width, 
        image_height, 
        60.0, 
        look_from, 
        look_at, 
        vup,
        aperture,
        dist_to_focus);

    // render
    for(int y = image_height-1; y>=0; --y) {
        std::cout << "\rScanlines remaining: " << (image_buffer.height()-1) - y << std::endl;
        for(int x = image_width-1; x>=0; --x) {
            color pixel_color;

            for(int s = 0; s<samples_per_pixel; s++) {
                double u = (x+random_double()) / static_cast<double>(image_width-1);
                double v = (y+random_double()) / static_cast<double>(image_height-1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world);
            }

            image_buffer.write(x, (image_height-1)-y, pixel_color, samples_per_pixel);
        }
    }


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
        image_buffer.data(), 
        image_buffer.pitch());


    SDL_SetRenderDrawColor(renderer, 0, 20, 80, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    
    while (true) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}