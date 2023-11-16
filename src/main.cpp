#include <iostream>
#include <stdio.h>
#include "SDL.h"

#include "core/include.hpp"

CPT_Screen scr(vram);

void cpt_init() {
    ram_boot(ram, vram);
    
    std::string opening_msg = 
    "+------------------------------------------------+\n"
    "|  CPT100 High-spec Fantasy Console              |\n"
    "|                                                |\n"
    "|  (c) src3453 2023 Released under MIT Licence.  |\n"
    "+------------------------------------------------+\n"
    ;
    std::cout << opening_msg << std::endl;
   
}

int main(int argv, char** args) {
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("CPT100 High-spec Fantasy Console", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool isRunning = true;

    cpt_init();

    while (isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        SDL_RenderClear(renderer);
        // ここにコードを追加してください
        scr.update();
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
