#include "sol.hpp"

sol::state lua;

int api_peek(int addr) {
    return ram_peek(ram, addr).toInt();
}
void api_poke(int addr, int value) {
    ram_poke(ram, addr, (Byte)value);
}
int api_vpeek(int addr) {
    return vram_peek(vram, addr).toInt();
}
void api_vpoke(int addr, int value) {
    vram_poke(vram, addr, (Byte)value);
}
void api_print(std::string text, int x, int y, int color) {
    font.print(text, x, y, color);
}
void api_pix(int x, int y, int color) {
    scr.pix(x, y, color);
}
void api_trace(std::string text) {
    printf((text+"\n").c_str());
}

void register_functions() {
    lua.set_function("peek",api_peek);
    lua.set_function("poke",api_poke);
    lua.set_function("vpeek",api_vpeek);
    lua.set_function("vpoke",api_vpoke);
    lua.set_function("print",api_print);
    lua.set_function("pix",api_pix);
    lua.set_function("trace",api_trace);
}

void init_lua() {
    lua.open_libraries(
    sol::lib::base,
    sol::lib::package,
    sol::lib::math,
    sol::lib::string,
    sol::lib::table);
    register_functions();
    lua.script_file("./src/main.lua");
    lua["BOOT"]();
}

void Lua_MainLoop() {
    lua["LOOP"]();
}
