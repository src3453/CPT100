#include <sol/sol.hpp>
#include <time.h>

sol::state lua;
int timerStart = 0;
std::string LuaSrcPath = "";

void api__maincall() {
    timerStart = clock();
    lua.script_file(LuaSrcPath);
    sol::function func = lua["BOOT"];
    if (func != sol::nil) func();
}
int api_peek(float addr) {
    return ram_peek(ram, (int)addr).toInt();
}
void api_poke(float addr, float value) {
    ram_poke(ram, (int)addr, (Byte)((int)value%256));
}
int api_vpeek(float addr) {
    return vram_peek(vram, (int)addr).toInt();
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
    scr.cls((Byte)(int)color);
}
int api_rgb(float r, float g, float b) {
    return scr.fromRGB((int)r,(int)g,(int)b).toInt();
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
    std::cout << buf;
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
        out.push_back(ram_peek(ram, i).toInt());
    }
    return out;
}
std::vector<int> api_vpeekarr(float addr, float block) {
    std::vector<int> out;
    for (int i = (int)addr; i < (int)addr + (int)block; i++)
    {
        out.push_back(vram_peek(vram, i).toInt());
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

void register_functions() {
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
}

void init_lua(std::string LuaSrcPath) {
    lua.open_libraries(
    sol::lib::base,
    sol::lib::package,
    sol::lib::math,
    sol::lib::string,
    sol::lib::table);
    register_functions();
    lua["_CPT_VERSION"] = (std::string)"" VERSION_MAJOR "." VERSION_MINOR "." VERSION_REVISION VERSION_STATUS " (" VERSION_HASH ")";
}

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