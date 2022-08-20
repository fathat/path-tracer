#include <iostream>

#include <SDL.h>

#include "types.h"
#include "image_buffer.h"
#include "ray.h"


double hit_sphere(const vec3_d& center, double radius, const Ray& ray) {
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
}


color ray_color(const Ray& ray) {
    auto t = hit_sphere(point3(0,0,-1), 0.5, ray);
    if (t > 0.0) {
        const vec3_d surface_normal = glm::normalize(ray.at(t) - vec3_d(0,0,-1));
        return 0.5*color(surface_normal.x+1, surface_normal.y+1, surface_normal.z+1);
    }
    const vec3_d unit_direction = glm::normalize(ray.direction());
    t = 0.5*(unit_direction.y + 1.0);
    return (1.0-t)*color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

void render_test_pattern(image_buffer& buffer)
{
    for (size_t y = 0; y < buffer.height(); y++)
    {
        std::cout << "\rScanlines remaining: " << (buffer.height()-1) - y << std::endl;
        for (size_t x = 0; x < buffer.width(); x++)
        {
            const double r = static_cast<double>(x) / static_cast<double>(buffer.width() - 1);
            const double g = static_cast<double>(y) / static_cast<double>(buffer.height() - 1);
            constexpr double b = 0.25;
            constexpr double a = 1.0;

            buffer.write(x, y, {r,g,b,a});
        }
    }
}

int main(int argc, char* argv[])
{
    constexpr size_t image_width = 950;
    constexpr size_t image_height = 540;
    constexpr double aspect_ratio = static_cast<double>(image_width) / static_cast<double>(image_height);
        
    image_buffer image_buffer(image_width, image_height);
    render_test_pattern(image_buffer);

    
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
            Ray ray(origin, lower_left_corner+u*horizontal + v*vertical - origin);
            color pixelColor = ray_color(ray);
            image_buffer.write(x, (image_height-1)-y, pixelColor);
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