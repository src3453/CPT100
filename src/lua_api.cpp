#include "sol.hpp"
#include <time.h>

sol::state lua;
int timerStart = 0;

int api_peek(int addr) {
    return ram_peek(ram, (int)addr).toInt();
}
void api_poke(int addr, int value) {
    ram_poke(ram, (int)addr, (Byte)(value%256));
}
int api_vpeek(int addr) {
    return vram_peek(vram, (int)addr).toInt();
}
void api_vpoke(int addr, int value) {
    vram_poke(vram, (int)addr, (Byte)(value%256));
}
void api_print(std::string text, int x, int y, int color) {
    font.print((std::string)text, (int)x, (int)y, (int)color);
}
void api_pix(int x, int y, int color) {
    scr.pix((int)x, (int)y, (int)color);
}
void api_trace(std::string text) {
    printf(((std::string)text+"\n").c_str());
}
void api_cls(int color) {
    scr.cls((int)color);
}
int api_from_rgb(int r, int g, int b) {
    return scr.fromRGB(r,g,b).toInt();
}
int api_time() {
    return clock()-timerStart;
}
int api_int(float num) {
    return (int)num;
}
int api_key(int keycode) {
    int length = 0;
    const Uint8* buf = SDL_GetKeyboardState(&length);
    const std::vector<Uint8> keystates(buf,buf+length);
    std::cout << buf;
    return keystates.at(keycode);
}
std::string api_to_key_name(int keycode) {
    return (std::string)SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)keycode));
}
int api_from_key_name(std::string keyname) {
    return (int)SDL_GetScancodeFromName(keyname.c_str());
}
void api_rect(int x, int y, int w, int h, int color) {
    scr.rect((int)x, (int)y, (int)w, (int)h, (Byte)color);
}
std::tuple<int, int> api_mouse() {
    return scr.mouse();
}
std::vector<int> api_peekarr(int addr, int block) {
    std::vector<int> out;
    for (int i = addr; i < addr + block; i++)
    {
        out.push_back(ram_peek(ram, i).toInt());
    }
    return out;
}
std::vector<int> api_vpeekarr(int addr, int block) {
    std::vector<int> out;
    for (int i = addr; i < addr + block; i++)
    {
        out.push_back(vram_peek(vram, i).toInt());
    }
    return out;
}
std::vector<int> api_pokearr(int addr, std::vector<int> vals) {
    std::vector<Byte> values;
    for (int i = addr; i < addr + vals.size(); i++)
    {
        values.push_back((Byte)vals.at(i-addr));
    }
    ram_poke2array(ram,addr,values);
}
std::vector<int> api_vpokearr(int addr, std::vector<int> vals) {
    std::vector<Byte> values;
    for (int i = addr; i < addr + vals.size(); i++)
    {
        values.push_back((Byte)vals.at(i-addr));
    }
    vram_poke2array(vram,addr,values);
}

void register_functions() {
    lua.set_function("peek",api_peek);
    lua.set_function("poke",api_poke);
    lua.set_function("vpeek",api_vpeek);
    lua.set_function("vpoke",api_vpoke);
    lua.set_function("print",api_print);
    lua.set_function("pix",api_pix);
    lua.set_function("trace",api_trace);
    lua.set_function("cls",api_cls);
    lua.set_function("from_rgb",api_from_rgb);
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
}

void init_lua(std::string LuaSrcPath) {
    lua.open_libraries(
    sol::lib::base,
    sol::lib::package,
    sol::lib::math,
    sol::lib::string,
    sol::lib::table);
    register_functions();
    timerStart = clock();
    lua.script_file(LuaSrcPath);
    lua["BOOT"]();
}

void Lua_MainLoop() {
    lua["LOOP"]();
}

void Lua_OnKeyDown(int key) {
    lua["ONKEYDOWN"](key);
}

void Lua_OnKeyUp(int key) {
    lua["ONKEYUP"](key);
}

