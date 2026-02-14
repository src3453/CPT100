#include "config.h"

#ifdef WASM_BUILD
#include <emscripten.h>
#endif
#include <iostream>
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
//#include "boost/tuple/tuple.hpp"

int mouseState = 0;
int screenMode = 0;
int mode1_cursorShow = 1;
std::string inputText = "";

#define Byte unsigned char
#ifndef APP_LANG
#define APP_LANG "ja"
#endif
#define TR_STR(str_jp, str_en) (std::string(APP_LANG) == "ja" ? str_jp : str_en)

#define CPT_PRODUCT_NAME TR_STR("CPT200 ハイスペック ファンタジーコンソール", "CPT200 High-spec Fantasy Console")

#include "core/header/spec.hpp"
#include "core/ram.cpp"
#include "core/vpu/hppa.cpp"
#include "core/disp.cpp"
#include "core/text.cpp"
#include "core/music.cpp"
#include "core/vpu/vpu.cpp"
#include "core/sc/sc.cpp"
#include "core/sound_drv_SDL.cpp"

CPT_Screen scr(vram);
VPU vpu(scr);
Font font(scr);
SpriteController sc(vram, scr);

#include "lua_api.cpp"

std::string padTo(std::string str, const size_t num, const char paddingChar = ' ')
{
    std::string out = str;
    if(num > str.size())
        out.insert(str.size(), num - str.size(), paddingChar);
    return out;
}

void cpt_init(int argv, char** args) {
    std::string version = "Version " VERSION_MAJOR "." VERSION_MINOR "." VERSION_REVISION VERSION_STATUS " (" VERSION_HASH ")";
    version = padTo(version,44);
    std::string opening_msg = 
    (std::string)
    "+-----------------------------------------------------+\n"
    "|  " +padTo("CPT200 16bit Retro Game Framework",44)+(std::string)"       |\n"
    "|  " + padTo(version,44) +       (std::string)"       |\n"
    "|  (c) src3453 2023-2026 Released under MIT Licence.  |\n"
    "+-----------------------------------------------------+\n";
    std::cout << opening_msg << std::endl;
    ram_boot(ram, vram);
    scr.init();
    font.loadFontData();
    initSound();
    init_lua();   
    //Set callback
}

uint8_t finalPixels[CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT * 4] = {0};
SDL_Window* window;
SDL_GLContext glContext;
GLuint screenTexture;

void MainTick() {
    Lua_MainLoop(); //60Hz
    font.drawCharPCG(screenMode);
    sc.updateSprites();
    sc.renderSprites();
    Lua_PostDraw(); //after sprite render
    // test
    //font.drawCharUnicode16(0x3042, 0, 0, 0xFFFF); // Draw 'あ' at (0,0)
    scr.update(finalPixels);
    
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    glViewport(0, 0, w, h);
    
    // Upload 2D texture
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CPT_SCREEN_WIDTH, CPT_SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, finalPixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Draw 2D Quad over 3D
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
    glEnable(GL_TEXTURE_2D);
    
    double aspect_ratio = (double)CPT_SCREEN_WIDTH / (double)CPT_SCREEN_HEIGHT;
    int rw, rh, rx, ry;
    if ((double)w / aspect_ratio <= (double)h) {
        rw = w;
        rh = (int)((double)w / aspect_ratio);
    } else {
        rh = h;
        rw = (int)((double)h * aspect_ratio);
    }
    rx = (w - rw) / 2;
    ry = (h - rh) / 2;

    #define INTEGER_SCALING
    #ifdef INTEGER_SCALING
    int int_rw = (CPT_SCREEN_WIDTH * (rw / CPT_SCREEN_WIDTH));
    int int_rh = (CPT_SCREEN_HEIGHT * (rh / CPT_SCREEN_HEIGHT));
    rx = (w - int_rw) / 2;
    ry = (h - int_rh) / 2;
    rw = int_rw;
    rh = int_rh;
    #endif
    
    wx = rx; wy = ry; ww = rw; wh = rh;
    #define BACKGROUND_COLOR 0.1f, 0.1f, 0.1f, 1.0f
    
    glClearColor(BACKGROUND_COLOR);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2i(rx, ry);
    glTexCoord2f(1, 0); glVertex2i(rx + rw, ry);
    glTexCoord2f(1, 1); glVertex2i(rx + rw, ry + rh);
    glTexCoord2f(0, 1); glVertex2i(rx, ry + rh);
    glEnd();
    
    glDisable(GL_BLEND);
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
                //printf("Text Input Event: %s\n", event.text.text);
                //inputText += (std::string)(event.text.text);
                Lua_OnInput((std::string)(event.text.text));
            }
        }

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        MainTick();

        SDL_GL_SwapWindow(window);
}

int main(int argv, char** args) {
    
    
    SDL_Init(SDL_INIT_EVERYTHING);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    
    window = SDL_CreateWindow("CPT200 v" VERSION_MAJOR "." VERSION_MINOR "." VERSION_REVISION VERSION_STATUS " (" VERSION_HASH ")", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        CPT_SCREEN_WIDTH, 
        CPT_SCREEN_HEIGHT, 
            SDL_WINDOW_RESIZABLE|
            SDL_WINDOW_OPENGL|
            SDL_WINDOW_ALLOW_HIGHDPI|
            SDL_WINDOW_MOUSE_FOCUS|
            SDL_WINDOW_INPUT_FOCUS
        );
    if (!window)
    {
        printf("SDL Window could not be initialized. SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetWindowMinimumSize(window, CPT_SCREEN_WIDTH, CPT_SCREEN_HEIGHT);
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        printf("OpenGL Context could not be created. SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_MakeCurrent(window, glContext);
    
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CPT_SCREEN_WIDTH, CPT_SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    bool isRunning = true;

    cpt_init(argv,args);
    
    #ifdef WASM_BUILD
    emscripten_set_main_loop(MainLoop, 0, 1);
    #endif
    #ifndef WASM_BUILD
    const int desired_fps = 60;
    int desired_frame_duration = 1000/desired_fps;
    while(1) {
        int ticks_before = SDL_GetTicks();
        MainLoop();
        int ticks_after = SDL_GetTicks();
        int ticks_passed = ticks_after - ticks_before;
        int amount_to_wait = desired_frame_duration - ticks_passed;
    
        if(amount_to_wait > 0) {
            SDL_Delay(amount_to_wait);
        }
    }
    #endif
    closeSound();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(dev);
    SDL_Quit();
    
    return 0;
}
