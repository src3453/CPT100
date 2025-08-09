#include "sol/sol.hpp"
#include <time.h>

sol::state lua;
int timerStart = 0;
std::string LuaSrcPath = "";

#include "lua/main.lua.hpp"
#include "lua/opening.lua.hpp"

#define Byte unsigned char

// API functions, these are called from Lua frontend

void api__maincall() {
    // it is a special API function that is called once at the beginning, works as main entry point
    // will be override LOOP() function used in opening.lua
    timerStart = clock();
    lua.script(main_source);
    sol::function func = lua["BOOT"]; // it will be called once at the beginning, useful for initialization
    if (func != sol::nil) func();
}
int api_peek(float addr) {
    return ram_peek(ram, (int)addr);
}
void api_poke(float addr, float value) {
    ram_poke(ram, (int)addr, (Byte)((int)value%256));
}
int api_vpeek(float addr) {
    return vram_peek(vram, (int)addr);
}
void api_vpoke(float addr, float value) {
    vram_poke(vram, (int)addr, (Byte)((int)value%256));
}
void api_print(std::string text, float x, float y, float color) {
    font.print((std::string)text, (int)x, (int)y, (Byte)(int)color);
}
void api_pix(float x, float y, float color) {
    scr.pix((int)x, (int)y, (Byte)(int)color);
}
void api_trace(std::string text) {
    printf(((std::string)text+"\n").c_str());
}
void api_cls(float color) {
    if (screenMode == 0) {
        scr.cls((Byte)(int)color);
    } else if (screenMode == 1) {
        font.clearPCG((Byte)(int)color);
    }
}
int api_rgb(float r, float g, float b) {
    return scr.fromRGB((int)r,(int)g,(int)b);
}
int api_time() {
    return clock()-timerStart;
}
int api_int(float num) {
    return (int)num;
}
int api_key(float keycode) {
    int length = 0;
    const Uint8* buf = SDL_GetKeyboardState(&length);
    const std::vector<Uint8> keystates(buf,buf+length);
    //std::cout << buf;
    return keystates.at((int)keycode);
}
std::string api_to_key_name(float keycode) {
    return (std::string)SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)(int)keycode));
}
int api_from_key_name(std::string keyname) {
    return (int)SDL_GetScancodeFromName(keyname.c_str());
}
void api_rect(float x, float y, float w, float h, float color) {
    scr.rect((int)x, (int)y, (int)w, (int)h, (Byte)color);
}
std::tuple<int, int, int> api_mouse() {
    return scr.mouse();
}
std::vector<int> api_peekarr(float addr, float block) {
    std::vector<int> out;
    for (int i = (int)addr; i < (int)addr + (int)block; i++)
    {
        out.push_back(ram_peek(ram, i));
    }
    return out;
}
std::vector<int> api_vpeekarr(float addr, float block) {
    std::vector<int> out;
    for (int i = (int)addr; i < (int)addr + (int)block; i++)
    {
        out.push_back(vram_peek(vram, i));
    }
    return out;
}
void api_pokearr(float addr, std::vector<float> vals) {
    std::vector<Byte> values;
    for (int i = (int)addr; i < (int)addr + vals.size(); i++)
    {
        values.push_back((Byte)(int)vals.at(i-(int)addr));
    }
    ram_poke2array(ram,(int)addr,values);
}
void api_vpokearr(float addr, std::vector<float> vals) {
    std::vector<Byte> values;
    for (int i = (int)addr; i < (int)addr + vals.size(); i++)
    {
        values.push_back((Byte)(int)vals.at(i-(int)addr));
    }
    vram_poke2array(vram,(int)addr,values);
}
void api_rectb(float x, float y, float w, float h, float color) {
    scr.rectb((int)x, (int)y, (int)w, (int)h, (Byte)(int)color);
}
void api_line(float xs, float ys, float xe, float ye, float color) {
    scr.line((int)xs, (int)ys, (int)xe, (int)ye, (Byte)(int)color);
}
void api_spr(float num, float x, float y, float w=1, float h=1) {
    scr.spr((int)num,(int)x,(int)y,(int)w,(int)h);
}
int api_showcur(float toggle=-1) {
    return SDL_ShowCursor((int)toggle);
}
void api_startinput() {
    SDL_StartTextInput();
}
void api_stopinput() {
    SDL_StartTextInput();
}
void api_resetinput() {
    inputText = "";
}
std::string api_getinput() {
    return (std::string)inputText;
}
void api_resetgate(float ch) {
    chip.resetGate((int)ch);
}
void api_wtsync(float ch) {
    chip.wtSync((int)ch);
}
int api_put_dma_buffer(float ch, std::vector<float> data) {
    std::vector<unsigned char> byteData;
    //printf("data size: %d\n", (int)data.size());
    for (size_t i = 0; i < data.size(); i++){
        byteData.push_back((unsigned char)(int)data[i]);
    }
    return chip.putDMABuffer((int)ch, byteData.data(), data.size());
}

int api_get_dma_buffer_length(float ch) {
    return chip.getDMABufferLength((int)ch);
}

void api_vpu_init() {
    
}

void api_file_open(float slot, std::string filename) {
    std::cout << "Opening file: " << filename << std::endl;
    if (slot < 0 || slot >= 4) {
        std::cerr << "Invalid slot number. Must be between 0 and 3." << std::endl;
        return;
    }
    if (filename.empty()) {
        std::cerr << "Filename cannot be empty." << std::endl;
        return;
    }
    
}

void api_screen(int mode) {
    screenMode = (int)mode;
} 

void api_printp(std::string text, float x, float y, float color=255, float bgColor=0) {
    font.printPCG((std::string)text, (int)x, (int)y, (Byte)(int)color, (Byte)(int)bgColor);
}

void api_scrollp(float x, float y) {
    font.scrollPCG((int)y);
}

void api_include(std::string content) {
    // This function is used to include a Lua script from a string
    // It can be used to dynamically load Lua code at runtime
    lua.script(content);
}

void register_functions() {
    // Register all API functions
    lua.set_function("_maincall",api__maincall);
    lua.set_function("peek",api_peek);
    lua.set_function("poke",api_poke);
    lua.set_function("vpeek",api_vpeek);
    lua.set_function("vpoke",api_vpoke);
    lua.set_function("print",api_print);
    lua.set_function("pix",api_pix);
    lua.set_function("trace",api_trace);
    lua.set_function("cls",api_cls);
    lua.set_function("rgb",api_rgb);
    lua.set_function("time",api_time);
    lua.set_function("int",api_int);
    lua.set_function("key",api_key);
    lua.set_function("to_key_name",api_to_key_name);
    lua.set_function("from_key_name",api_from_key_name);
    lua.set_function("rect",api_rect);
    lua.set_function("mouse",api_mouse);
    lua.set_function("peekarr",api_peekarr);
    lua.set_function("vpeekarr",api_vpeekarr);
    lua.set_function("pokearr",api_pokearr);
    lua.set_function("vpokearr",api_vpokearr);
    lua.set_function("rectb",api_rectb);
    lua.set_function("line",api_line);
    lua.set_function("spr",api_spr);
    lua.set_function("showcur",api_showcur);
    lua.set_function("startinput",api_startinput);
    lua.set_function("stopinput",api_stopinput);
    lua.set_function("resetinput",api_resetinput);
    lua.set_function("getinput",api_getinput);
    lua.set_function("resetgate",api_resetgate);
    lua.set_function("wtsync",api_wtsync);
    lua.set_function("put_dma_buffer", api_put_dma_buffer);
    lua.set_function("get_dma_buffer_length", api_get_dma_buffer_length);
    lua.set_function("vpu_init", api_vpu_init);
    lua.set_function("screen", api_screen);
    lua.set_function("printp", api_printp);
    lua.set_function("scrollp", api_scrollp);
    lua.set_function("include", api_include);

}

void init_lua() {
    // it will call opening screen, then it will be overridden by main entry point
    lua.open_libraries(
    sol::lib::base,
    sol::lib::package,
    sol::lib::math,
    sol::lib::string,
    sol::lib::table,
    sol::lib::io);
    register_functions();
    lua["_CPT_VERSION"] = (std::string)VERSION_MAJOR "." VERSION_MINOR "." VERSION_REVISION VERSION_STATUS " (" VERSION_HASH ")";
    // in WASM build, some features will be limited
    #ifdef WASM_BUILD
    lua["_CPT_IS_WASM"] = 1;
    #else 
    lua["_CPT_IS_WASM"] = 0;
    #endif
    lua.script(opening_source);
}

// These functions are called when specific events occur by C++ backend

void Lua_OnKeyDown(int key) {
    sol::function func = lua["ONKEYDOWN"];
    if (func != sol::nil) func(key);
    
}

void Lua_OnKeyUp(int key) {
    sol::function func = lua["ONKEYUP"];
    if (func != sol::nil) func(key);
}

void Lua_MainLoop() {
    sol::function func = lua["LOOP"];
    if (func != sol::nil) func();
}

void Lua_OnTextInput(std::string inputChar) {
    sol::function func = lua["ONINPUT"];
    if (func != sol::nil) func((std::string)inputChar);
}