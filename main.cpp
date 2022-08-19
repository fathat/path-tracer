#include <iostream>

#include "SDL.h"
#include <glm/glm.hpp>

int main(int argc, char *argv[])
{
    //lets make a buffer!
    const int imageWidth = 128;
    const int imageHeight = 128;
    uint8_t* buffer = new uint8_t[imageWidth*imageHeight*4];

    for(size_t y = 0; y<imageHeight; y++)
    {
        for(size_t x = 0; x<imageWidth; x++)
        {
            double r = double(x) / double(imageWidth-1);
            double g = double(y) / double(imageHeight-1);
            double b = 0.0;
            double a = 1.0;

            size_t baseIndex = x*4 + y*imageWidth*4;

            buffer[baseIndex] = static_cast<uint8_t>(a * 255);
            buffer[baseIndex + 1] = static_cast<uint8_t>(b * 255);
            buffer[baseIndex + 2] = static_cast<uint8_t>(g * 255);
            buffer[baseIndex + 3] = static_cast<uint8_t>(r * 255);
        }
    }
    
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
        "SDL2Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        480,
        0);
        
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, imageWidth, imageHeight);

    uint32_t format;
    int access;
    int actualWidth;
    int actualHeight;
    SDL_QueryTexture(texture, &format, &access, &actualWidth, &actualHeight);

    auto ok = SDL_UpdateTexture(texture, nullptr, buffer, imageWidth * 4);
    std::cout << "was ok? " << ok << std::endl;


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

        /*SDL_SetRenderDrawColor(renderer, 0, 20, 80, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &r);*/

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderCopy(renderer, texture, nullptr, &r);
        
        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}