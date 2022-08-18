#include "SDL.h"
#include <glm/glm.hpp>

int main(int argc, char *argv[])
{
    
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
        "SDL2Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        480,
        0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawColor(renderer, 0, 20, 80, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_Rect r;
    r.x = 200;
    r.y = 200;
    r.w = 400;
    r.h = 200;

    while (true) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;

        SDL_SetRenderDrawColor(renderer, 0, 20, 80, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &r);
        
        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}