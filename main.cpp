#include <iostream>

#include "SDL.h"
#include <glm/glm.hpp>

constexpr size_t imageWidth = 640;
constexpr size_t imageHeight = 480;

int main(int argc, char* argv[])
{
    //lets make a buffer!
    const auto buffer = new uint8_t[imageWidth * imageHeight * 4];

    for (size_t y = 0; y < imageHeight; y++)
    {
        for (size_t x = 0; x < imageWidth; x++)
        {
            const double r = static_cast<double>(x) / static_cast<double>(imageWidth - 1);
            const double g = static_cast<double>(y) / static_cast<double>(imageHeight - 1);
            const double b = 0.0;
            const double a = 1.0;

            const size_t baseIndex = x * 4 + y * imageWidth * 4;

            buffer[baseIndex] = static_cast<uint8_t>(a * 255);
            buffer[baseIndex + 1] = static_cast<uint8_t>(b * 255);
            buffer[baseIndex + 2] = static_cast<uint8_t>(g * 255);
            buffer[baseIndex + 3] = static_cast<uint8_t>(r * 255);
        }
    }

    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "SDL2Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        imageWidth,
        imageHeight,
        0);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, imageWidth, imageHeight);

    SDL_UpdateTexture(texture, nullptr, buffer, imageWidth * 4);


    SDL_SetRenderDrawColor(renderer, 0, 20, 80, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_Rect r;
    r.x = 200;
    r.y = 200;
    r.w = imageWidth;
    r.h = imageHeight;

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