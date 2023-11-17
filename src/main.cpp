#include <iostream>
#include <stdio.h>
#include "SDL.h"

#include "core/include.hpp"

CPT_Screen scr(vram);
Font font(scr);

#include "lua_api.cpp"

void cpt_init() {

    std::string opening_msg = 
    "+------------------------------------------------+\n"
    "|  CPT100 High-spec Fantasy Console              |\n"
    "|                                                |\n"
    "|  (c) src3453 2023 Released under MIT Licence.  |\n"
    "+------------------------------------------------+\n"
    ;
    std::cout << opening_msg << std::endl;
    ram_boot(ram, vram);
    init_lua();
    //Set callback
}

void blitToMainWindow(SDL_Texture *texture, SDL_Renderer *renderer, uint8_t *pixels) {
    
    
    // update texture with new data
    int texture_pitch = 0;
    void* texture_pixels = NULL;
    if (SDL_LockTexture(texture, NULL, &texture_pixels, &texture_pitch) != 0) {
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
    }
    else {
        memcpy(texture_pixels, pixels, texture_pitch * SCREEN_HEIGHT);
    }
    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
}

int main(int argv, char** args) {
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("CPT100 High-spec Fantasy Console", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT);
    if (texture == NULL) {
        SDL_Log("Unable to create texture: %s", SDL_GetError());
        return 1;
    }

    
    bool isRunning = true;

    cpt_init();

    while (isRunning) {

        Lua_MainLoop();

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        SDL_RenderClear(renderer);

        uint8_t finalPixels[SCREEN_WIDTH * SCREEN_HEIGHT * 3] = {0};
        scr.update(finalPixels);
        blitToMainWindow(texture, renderer, finalPixels);
        SDL_RenderPresent(renderer);
        playSound();
        SDL_Delay(1000/CALLBACK_FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
