#include "config.h"

#ifdef WASM_BUILD
#include <emscripten.h>
#endif
#include <iostream>
#include <stdio.h>
#include <SDL.h>

int mouseState = 0;
std::string inputText = "";

#include "core/header/types.hpp"
#include "core/header/spec.hpp"
#include "core/ram.cpp"
#include "core/disp.cpp"
#include "core/text.cpp"
#include "core/sound.cpp"

CPT_Screen scr(vram);
Font font(scr);

#include "lua_api.cpp"

std::string padTo(std::string str, const size_t num, const char paddingChar = ' ')
{
    std::string out = str;
    if(num > str.size())
        out.insert(str.size(), num - str.size(), paddingChar);
    return out;
}

void cpt_init(int argv, char** args) {
    if (argv == 1) {
        throw (std::string)"Please specify lua source path.";
    }
    std::string version = "Version " VERSION_MAJOR "." VERSION_MINOR "." VERSION_REVISION VERSION_STATUS " (" VERSION_HASH ")";
    version = padTo(version,44);
    std::string opening_msg = 
    (std::string)
    "+------------------------------------------------+\n"
    "|  CPT100 High-spec Fantasy Console              |\n"
    "|  " + padTo(version,44) +       (std::string)"  |\n"
    "|  (c) src3453 2023 Released under MIT Licence.  |\n"
    "+------------------------------------------------+\n";
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
    double aspect_ratio = (double)CPT_SCREEN_WIDTH / (double)CPT_SCREEN_HEIGHT;

    // 新しいサイズを縦横比を維持して計算
    if ((double)w / aspect_ratio <= (double)h) {
        _rect.w = w;
        _rect.h = (int)((double)w / aspect_ratio);
    } else {
        _rect.h = h;
        _rect.w = (int)((double)h * aspect_ratio);
    }
    _rect.x = (w - _rect.w) / 2;
    _rect.y = (h - _rect.h) / 2;

    SDL_Rect *rect = &_rect;

    SDL_RenderCopy(renderer, texture, NULL,(const SDL_Rect*)rect);
    return std::make_tuple(_rect.x,_rect.y,_rect.w,_rect.h);
}

uint8_t finalPixels[CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT * 3] = {0};
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture *texture;
void MainTick() {
    Lua_MainLoop(); //60Hz
    scr.update(finalPixels);
    std::tuple<int,int,int,int> winRect = blitToMainWindow(window, texture, renderer, finalPixels);
    wx = std::get<0>(winRect);
    wy = std::get<1>(winRect);
    ww = std::get<2>(winRect);
    wh = std::get<3>(winRect);
}


void MainLoop() {
    SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                exit(0);
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
            if (event.type == SDL_TEXTINPUT) {
                inputText += (std::string)(event.text.text);
                Lua_OnTextInput((std::string)(event.text.text));
            }
        }

        SDL_RenderClear(renderer);
        
        MainTick();

        SDL_RenderPresent(renderer);
}

int main(int argv, char** args) {
    
    
    SDL_Init(SDL_INIT_EVERYTHING);
    
    
    window = SDL_CreateWindow("CPT100 High-spec Fantasy Console v" VERSION_MAJOR "." VERSION_MINOR "." VERSION_REVISION VERSION_STATUS " (" VERSION_HASH ")", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CPT_SCREEN_WIDTH, CPT_SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        printf("SDL Window could not be initialized. SDL_Error: %s\n", SDL_GetError());
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("SDL Renderer could not be initialized. SDL_Error: %s\n", SDL_GetError());
    }
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        CPT_SCREEN_WIDTH,
        CPT_SCREEN_HEIGHT);
    bool isRunning = true;

    cpt_init(argv,args);
    
    
        if (texture == NULL) {
            SDL_Log("Unable to create texture: %s", SDL_GetError());
            return 1;
        }
    #ifdef WASM_BUILD
    emscripten_set_main_loop(MainLoop, 0, 1);
    #endif
    #ifndef WASM_BUILD
    while(1) {
        MainLoop();
    }
    #endif
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(dev);
    SDL_Quit();
    
    return 0;
}
