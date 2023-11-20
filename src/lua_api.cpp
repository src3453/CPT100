#include "sol.hpp"
#include <time.h>

sol::state lua;
int timerStart = 0;

int api_peek(std::any addr) {
    return ram_peek(ram, (int)addr).toInt();
}
void api_poke(std::any addr, std::any value) {
    ram_poke(ram, (int)addr, (Byte)(value%256));
}
int api_vpeek(std::any addr) {
    return vram_peek(vram, (int)addr).toInt();
}
void api_vpoke(std::any addr, std::any value) {
    vram_poke(vram, (int)addr, (Byte)(value%256));
}
void api_print(std::any text, std::any x, std::any y, std::any color) {
    font.print((std::string)text, (int)x, (int)y, (int)color);
}
void api_pix(std::any x, std::any y, std::any color) {
    scr.pix((int)x, (int)y, (int)color);
}
void api_trace(std::any text) {
    printf(((std::string)text+"\n").c_str());
}
void api_cls(std::any color) {
    scr.cls((int)color);
}
int api_rgb(std::any r, std::any g, std::any b) {
    return scr.fromRGB(r,g,b).toInt();
}
int api_time() {
    return clock()-timerStart;
}
int api_int(std::any num) {
    return (int)num;
}
int api_key(std::any keycode) {
    int length = 0;
    const Uint8* buf = SDL_GetKeyboardState(&length);
    const std::vector<Uint8> keystates(buf,buf+length);
    std::cout << buf;
    return keystates.at(keycode);
}
std::string api_to_key_name(std::any keycode) {
    return (std::string)SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)keycode));
}
int api_from_key_name(std::any keyname) {
    return (int)SDL_GetScancodeFromName(keyname.c_str());
}
void api_rect(std::any x, std::any y, std::any w, std::any h, std::any color) {
    scr.rect((int)x, (int)y, (int)w, (int)h, (Byte)color);
}
std::tuple<int, int> api_mouse() {
    return scr.mouse();
}
std::vector<int> api_peekarr(std::any addr, std::any block) {
    std::vector<int> out;
    for (int i = (int)addr; i < (int)addr + (int)block; i++)
    {
        out.push_back(ram_peek(ram, i).toInt());
    }
    return out;
}
std::vector<int> api_vpeekarr(std::any addr, std::any block) {
    std::vector<int> out;
    for (int i = (int)addr; i < (int)addr + (int)block; i++)
    {
        out.push_back(vram_peek(vram, i).toInt());
    }
    return out;
}
std::vector<int> api_pokearr(std::any addr, std::any vals) {
    std::vector<Byte> values;
    for (int i = (int)addr; i < (int)addr + vals.size(); i++)
    {
        values.push_back((Byte)vals.at(i-addr));
    }
    ram_poke2array(ram,addr,values);
}
std::vector<int> api_vpokearr(std::any addr, std::any vals) {
    std::vector<Byte> values;
    for (int i = (int)addr; i < (int)addr + vals.size(); i++)
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

void Lua_OnKeyDown(int key) {
    lua["ONKEYDOWN"](key);
}

void Lua_OnKeyUp(int key) {
    lua["ONKEYUP"](key);
}

