#include <iostream>

#include <SDL.h>

#include "hittable.h"
#include "hittable_list.h"
#include "types.h"
#include "image_buffer.h"
#include "ray.h"
#include "sphere.h"


/*double hit_sphere(const vec3_d& center, double radius, const ray& ray) {
    const vec3_d oc = ray.origin() - center;
    const auto a = glm::length2(ray.direction());
    const auto half_b = dot(oc, ray.direction());
    const auto c = glm::length2(oc) - radius*radius;
    const auto discriminant = half_b*half_b - a*c;

    if (discriminant < 0) {
        return -1.0;
    } else {
        return (-half_b - sqrt(discriminant) ) / a;
    }
}*/


color ray_color(const ray& ray, const hittable& world) {
    hit_record rec;
    if (world.hit(ray, 0, infinity, rec)) {
        return 0.5 * (rec.normal + color(1,1,1));
    }
    const vec3_d unit_direction = glm::normalize(ray.direction());
    auto t = 0.5*(unit_direction.y + 1.0);
    return (1.0-t)*color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}



int main(int argc, char* argv[])
{
    constexpr size_t image_width = 950;
    constexpr size_t image_height = 540;
    constexpr double aspect_ratio = static_cast<double>(image_width) / static_cast<double>(image_height);
        
    image_buffer image_buffer(image_width, image_height);

    // world
    hittable_list world;
    world.add(make_shared<sphere>(point3(0,0,-1), 0.5));
    world.add(make_shared<sphere>(point3(0,-100.5,-1), 100));

    // camera
    double viewport_height = 2.0;
    double viewport_width = aspect_ratio * viewport_height;
    double focal_length = 1.0;

    vec3_d origin {0, 0, 0};
    vec3_d horizontal {viewport_width, 0, 0};
    vec3_d vertical {0, viewport_height, 0};
    vec3_d lower_left_corner = origin - horizontal/2.0 - vertical/2.0 - vec3_d(0, 0, focal_length);

    // render
    for(int y = image_height-1; y>=0; --y) {
        std::cout << "\rScanlines remaining: " << (image_buffer.height()-1) - y << std::endl;
        for(int x = image_width-1; x>=0; --x) {
            double u = static_cast<double>(x) / (image_width-1);
            double v = static_cast<double>(y) / (image_height-1);
            ray ray(origin, lower_left_corner+u*horizontal + v*vertical - origin);
            color pixel_color = ray_color(ray, world);
            image_buffer.write(x, (image_height-1)-y, pixel_color);
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