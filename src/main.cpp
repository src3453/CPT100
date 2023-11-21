#include <iostream>
#include <stdio.h>
#include <SDL.h>

int mouseState = 0;

#include "core/header/types.hpp"
#include "core/header/spec.hpp"
#include "core/ram.cpp"
#include "core/disp.cpp"
#include "core/text.cpp"
#include "core/sound.cpp"

CPT_Screen scr(vram);
Font font(scr);

#include "lua_api.cpp"


void cpt_init(int argv, char** args) {
    if (argv == 1) {
        throw "Please specify lua source path.";
    }
    std::string opening_msg = 
    "+------------------------------------------------+\n"
    "|  CPT100 High-spec Fantasy Console              |\n"
    "|                                                |\n"
    "|  (c) src3453 2023 Released under MIT Licence.  |\n"
    "+------------------------------------------------+\n"
    ;
    std::cout << opening_msg << std::endl;
    ram_boot(ram, vram);
    scr.init();
    initSound();

    init_lua((std::string)args[1]);

    
    //Set callback
}

std::tuple<int,int,int,int> blitToMainWindow(SDL_Window *window, SDL_Texture *texture, SDL_Renderer *renderer, uint8_t *pixels) {
    
    
    // update texture with new data
    int texture_pitch = 0;
    void* texture_pixels = NULL;
    if (SDL_LockTexture(texture, NULL, &texture_pixels, &texture_pitch) != 0) {
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
    }
    else {
        memcpy(texture_pixels, pixels, texture_pitch * CPT_SCREEN_HEIGHT);
    }
    SDL_UnlockTexture(texture);
    int w,h = 0;
    SDL_GetWindowSize(window,&w,&h);
    SDL_Rect _rect;
    _rect.h=(int)((double)w*std::max((double)1,((double)w/(double)h))*((double)CPT_SCREEN_WIDTH/(double)CPT_SCREEN_HEIGHT));
    _rect.w=(int)((double)h*std::max((double)1,((double)w/(double)h))*((double)CPT_SCREEN_WIDTH/(double)CPT_SCREEN_HEIGHT));
    _rect.x=(int)((double)std::max((double)0,w-(double)h*((double)CPT_SCREEN_WIDTH/(double)CPT_SCREEN_HEIGHT)))/2;
    _rect.y=(int)((double)std::max((double)0,h-(double)w*((double)CPT_SCREEN_WIDTH/(double)CPT_SCREEN_HEIGHT)))/2;
    SDL_Rect *rect = &_rect;

    SDL_RenderCopy(renderer, texture, NULL,(const SDL_Rect*)rect);
    return std::make_tuple(_rect.x,_rect.y,_rect.w,_rect.h);
}

uint8_t finalPixels[CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT * 3] = {0};

void MainTick(SDL_Window *window, SDL_Texture* texture, SDL_Renderer* renderer) {
    Lua_MainLoop(); //60Hz
    scr.update(finalPixels);
    std::tuple<int,int,int,int> winRect = blitToMainWindow(window, texture, renderer, finalPixels);
    wx = std::get<0>(winRect);
    wy = std::get<1>(winRect);
    ww = std::get<2>(winRect);
    wh = std::get<3>(winRect);
}


int main(int argv, char** args) {
    
    
    SDL_Init(SDL_INIT_EVERYTHING);
    
    SDL_Window* window;
    SDL_Renderer* renderer;
    window = SDL_CreateWindow("CPT100 High-spec Fantasy Console", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CPT_SCREEN_WIDTH, CPT_SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    bool isRunning = true;

    cpt_init(argv,args);
    
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        CPT_SCREEN_WIDTH,
        CPT_SCREEN_HEIGHT);
        if (texture == NULL) {
            SDL_Log("Unable to create texture: %s", SDL_GetError());
            return 1;
        }

    while (isRunning) {

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            if (event.type == SDL_KEYDOWN) {
                Lua_OnKeyDown((int)SDL_GetScancodeFromKey(event.key.keysym.sym));
            }
            if (event.type == SDL_KEYUP) {
                Lua_OnKeyUp((int)SDL_GetScancodeFromKey(event.key.keysym.sym));
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) 
            {
                if(event.button.button == SDL_BUTTON_LEFT){
                    mouseState += 1;
                }
                else if(event.button.button == SDL_BUTTON_RIGHT){
                    mouseState += 2;
                }
                else if(event.button.button == SDL_BUTTON_MIDDLE){
                    mouseState += 4;
                }
            }
            if (event.type == SDL_MOUSEBUTTONUP) 
            {
                if(event.button.button == SDL_BUTTON_LEFT){
                    mouseState -= 1;
                }
                else if(event.button.button == SDL_BUTTON_RIGHT){
                    mouseState -= 2;
                }
                else if(event.button.button == SDL_BUTTON_MIDDLE){
                    mouseState -= 4;
                }
            }
        }

        SDL_RenderClear(renderer);
        
        MainTick(window,texture,renderer);

        SDL_RenderPresent(renderer);
        //SDL_Delay(1000/CALLBACK_FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(dev);
    SDL_Quit();
    
    return 0;
}
