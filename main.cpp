#include <iostream>

#include <SDL.h>
#include <glm/glm.hpp>

#include "color.h"

void renderIntoBuffer(ImageBuffer& buffer)
{
    for (size_t y = 0; y < buffer.height(); y++)
    {
        std::cout << "\rScanlines remaining: " << (buffer.height()-1) - y << std::endl;
        for (size_t x = 0; x < buffer.width(); x++)
        {
            const double r = static_cast<double>(x) / static_cast<double>(buffer.width() - 1);
            const double g = static_cast<double>(y) / static_cast<double>(buffer.height() - 1);
            const double b = 0.25;
            const double a = 1.0;

            buffer.write(x, y, {r,g,b,a});
        }
    }
}

int main(int argc, char* argv[])
{
    constexpr size_t imageWidth = 512;
    constexpr size_t imageHeight = 384;


    //lets make a buffer!
    ImageBuffer buffer(imageWidth, imageHeight);
    renderIntoBuffer(buffer);

    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "SDL2Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        imageWidth,
        imageHeight,
        SDL_WINDOW_RESIZABLE);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        imageWidth, 
        imageHeight);

    SDL_UpdateTexture(
        texture, 
        nullptr, 
        buffer.data(), 
        buffer.pitch());


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