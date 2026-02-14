// Define SOL2 flags
#define SOL_ALL_SAFETIES_ON 1
#define SOL_EXCEPTIONS_ALWAYS_UNSAFE 1
#define SOL_EXCEPTIONS_SAFE_PROPAGATION 1



#include "sol/sol.hpp"
#include <time.h>
#include "SDL_timer.h"
#include <locale>
#include <codecvt>

sol::state lua;
int timerStart = 0;
unsigned int timerCounter = 0;
std::string LuaSrcPath = "";

// Thread-safe timer event queue
#include <queue>
#include <mutex>
struct TimerEvent {
    unsigned int count;
    unsigned int timestamp;
};
std::queue<TimerEvent> timerEventQueue;
std::mutex timerQueueMutex;

#include "lua/main.lua.hpp"
#include "lua/opening.lua.hpp"
#include "lua/opening_ja.lua.hpp"
#define Byte unsigned char

// API functions, these are called from Lua frontend

// Function to halt the main loop by removing LOOP and POSTDRAW functions

std::u16string convU8toU16(std::string utf8text) {
    std::u16string u16text;
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    try {
        u16text = convert.from_bytes(utf8text);
    } catch (const std::range_error& e) {
        // Conversion failed, replace with placeholder
        u16text = u"?"; // Replacement character
    }
    return u16text;
}

void printAsU16(std::string utf8text) {
    font.printPCG(convU8toU16(utf8text));
}

// placeholder, does nothing (because NULL or nullptr as Lua function will make exception)
void api__NULL() {
    // does nothing
}

// Function to halt the main loop by removing LOOP and POSTDRAW functions
void haltLoop() {
    lua.set_function("LOOP", api__NULL);    // Remove LOOP function to stop further calls
    lua.set_function("POSTDRAW", api__NULL);// Remove POSTDRAW function to stop further calls
    lua.set_function("ONKEYDOWN", api__NULL); // Remove input handlers
    lua.set_function("ONKEYUP", api__NULL);
    lua.set_function("ONINPUT", api__NULL);
    lua.set_function("TIMERINT", api__NULL); // Remove timer handler
}

void api_resetvram() {
    scr.init(); // reset palette
    font.loadFontData(); // Reload font data
}

// Helper function to report Lua errors
void report_lua_error(const sol::error& e) {
    std::cerr << "Lua error: " << e.what() << std::endl;
    haltLoop(); // Stop the main loop
    api_resetvram();
    screenMode = 2; // Switch to text mode
    font.clearPCG(fromRGB(192,0,0));
    font.locatePCG(0,0);
    font.setFGColor(fromRGB(192,0,0));
    font.setBGColor(255);
    printAsU16(TR_STR("致命的な問題が発生しました\n", "A fatal error has occurred\n"));
    font.setBGColor(fromRGB(192,0,0));
    font.setFGColor(255);
    printAsU16(TR_STR("\nスクリプト エラー: ", "\nScript error: "));
    if (e.what() == nullptr) {
        printAsU16(TR_STR("エラーの詳細情報を特定できませんでした。", "Could not determine detailed error information."));

    } else {
        printAsU16(e.what());
    }
    printAsU16(TR_STR("\n\nプログラムの実行を停止しました。\n", "\n\nThe program has been halted.\n"));
    font.locatePCG(0, PCG_SCREEN_HEIGHT - 2);
    printAsU16(CPT_PRODUCT_NAME);
    printAsU16((std::string)TR_STR("\nバージョン ", "\nVersion ") + (std::string)(VERSION_MAJOR "." VERSION_MINOR "." VERSION_REVISION VERSION_STATUS " (" VERSION_HASH  ")"));
}

// Helper to call Lua functions safely
template<typename... Args>
void safe_lua_call(const std::string& name, Args&&... args) {
    try {
        // First, safely check if the value exists and is a function
        sol::object obj = lua[name];
        if (!obj.valid() || obj.get_type() != sol::type::function) {
            // Not a function or doesn't exist, silently return
            return;
        }
        
        // Now safely get the protected function
        sol::optional<sol::protected_function> maybe_func = lua[name];
        if (!maybe_func) {
            return;
        }
        
        sol::protected_function func = *maybe_func;
        sol::protected_function_result result = func(std::forward<Args>(args)...);
        if (!result.valid()) {
            sol::error err = result;
            report_lua_error(err);
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in safe_lua_call(" << name << "): " << e.what() << std::endl;
        haltLoop();
    } catch (...) {
        std::cerr << "Unknown exception in safe_lua_call(" << name << ")" << std::endl;
        haltLoop();
    }
}

void api__maincall() {
    // it is a special API function that is called once at the beginning, works as main entry point
    // will be override LOOP() function used in opening.lua
    timerStart = clock();
    auto result = lua.safe_script(main_source);
    if (!result.valid()) {
        sol::error err = result;
        report_lua_error(err);
        return;
    }
    safe_lua_call("BOOT");
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
void api_print(std::string text, float x, float y, float color, float flag) {
    if ((int)flag == 1) {
        std::u16string u16text = convU8toU16(text);
        font.printUnicode16(u16text, (int)x, (int)y, (int)color);
    } else {
        font.print((std::string)text, (int)x, (int)y, (int)color);
    }
}

void api_print(std::string text, float x, float y, float color) {
    font.print((std::string)text, (int)x, (int)y, (int)color);
}

void api_pix(float x, float y, float color) {
    scr.pix((int)x, (int)y, (int)color);
}
void api_trace(std::string text) {
    printf(((std::string)text+"\n").c_str());
}

void api_trace(float num) {
    printf("%d\n", (int)num);
}

void api_cls(float color) {
    if (screenMode == 1 || screenMode == 2) {
        font.clearPCG((int)color);
    } else {
        scr.cls((int)color);
    }
}
int api_rgb(float r, float g, float b) {
    return fromRGB((int)r,(int)g,(int)b);
}

int api_rgbd(float x, float y, float r, float g, float b) {
    return fromRGBDithered((int)x,(int)y,(int)r,(int)g,(int)b);
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
    scr.rect((int)x, (int)y, (int)w, (int)h, (int)color);
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
    scr.rectb((int)x, (int)y, (int)w, (int)h, (int)color);
}
void api_line(float xs, float ys, float xe, float ye, float color) {
    scr.line((int)xs, (int)ys, (int)xe, (int)ye, (int)color);
}
void api_sprraw(float num, float x, float y, float w=1, float h=1) {
    scr.spr((int)num,(int)x,(int)y,(int)w,(int)h);
}
int api_showcur(float toggle=-1) {
    return SDL_ShowCursor((int)toggle);
}
int api_showcurp(float toggle=-1) {
    if (toggle != -1) {
        mode1_cursorShow = (int)toggle;
    }
    font.setCursorVisibilityPCG(mode1_cursorShow!=0);
    return mode1_cursorShow;
}
void api_startinput() {
    SDL_StartTextInput();
}
void api_stopinput() {
    SDL_StopTextInput();
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

void api_vpu_draw_debug(float scale) {
    vpu.renderDebug(scale);
}

void api_vpu_draw_debug2(float scale) {
    vpu.renderDebug2(scale);
}

void api_vpu_update_state() {
    vpu.updateState();
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

void api_printp(std::string text) {
    if (screenMode == 2) {
        printAsU16(text);
    } else {
        font.printPCG(text);
    }
}

void api_printp(float c) {
    char *text = new char[2];
    text[0] = (char)(int)c;
    text[1] = '\0';
    font.printPCG((std::string)text);
}

void api_printlnp(std::string text) {
    api_printp(text + "\n");
}

void api_lc(float x, float y) {
    font.locatePCG((int)x, (int)y);
}

void api_cursorp(bool visible) {
    font.setCursorVisibilityPCG(visible);
}

void api_colorfg(float color) {
    font.setFGColor((Byte)(int)color);
}

void api_colorbg(float color) {
    font.setBGColor((Byte)(int)color);
}

void api_scrollp(float x, float y) {
    font.scrollPCG((int)y);
}

void api_movecursor(float dx, float dy) {
    font.moveCursorPCG((int)dx, (int)dy);
}

void api_loadspr(float index, std::string filename, float offset = 0x20000, bool dither = true, float transparentColorIndex = 0) {
    sc.loadSpriteFromLocalImage((int)index, (std::string)filename, (int)offset, dither, (int)transparentColorIndex);
}

void api_spr(float spriteIndex, bool enabled, float x, float y, float rotation = 0.0f) {
    sc.spr((int)spriteIndex, (int)enabled, (int)x, (int)y, rotation);
}

void api_include(std::string content) {
    // This function is used to include a Lua script from a string
    // It can be used to dynamically load Lua code at runtime
    auto result = lua.safe_script(content);
    if (!result.valid()) {
        sol::error err = result;
        report_lua_error(err);
    }
}

void api_init_sound_input(int samples=1024) {
    initSoundInput(samples);
}

std::vector<float> api_acquire_sound_input() {
    return acquireSoundInput();
}

std::vector<float> api_acquire_sound_input_fft(int fft_size) {
    return acquireSoundInputFFT(fft_size);
}

void api_setpwrap(bool enable) {
    font.setAutoWrapPCG(enable);
}

void Lua_TimerInt(unsigned int count) {
    // Add timer event to thread-safe queue instead of calling directly
    {
        std::lock_guard<std::mutex> lock(timerQueueMutex);
        timerEventQueue.push({count, SDL_GetTicks()});
    }
}

uint32_t timerCallback(uint32_t interval, void* param) {
    timerCounter += 1;
    Lua_TimerInt(interval);
    return interval; // continue the timer
}

int api_settimerint(float interval_ms) {
    int timerID = SDL_AddTimer((uint32_t)interval_ms, timerCallback, nullptr);
    if (timerID == 0) {
        std::cerr << "Failed to create timer: " << SDL_GetError() << std::endl;
    }
    return timerID;
}

bool api_disposetimerint(int timerID) {
    return SDL_RemoveTimer((SDL_TimerID)timerID);
    // タイマを破棄したときSDL_TRUE, タイマがないときSDL_FALSEを戻す. 
}

// BitBlt (linear to block)
void api_bitblt_lb(float addr, float x, float y, float w, float h, float transparent_color=-1) {
    scr.bitblt((int)addr, (int)x, (int)y, (int)w, (int)h, (int)transparent_color);
}

// BitBlt (block to block)
void api_bitblt_bb(float base_addr, float block_x, float block_y, float block_w, float block_h, float x, float y, float w, float h, float transparent_color=-1) {
    scr.bitblt_block_to_block((int)base_addr, (int)block_x, (int)block_y, (int)block_w, (int)block_h, (int)x, (int)y, (int)w, (int)h, (int)transparent_color);
}


void register_functions() {
    // Register all API functions
    lua.set_function("_maincall",api__maincall);
    lua.set_function("peek",api_peek);
    lua.set_function("poke",api_poke);
    lua.set_function("vpeek",api_vpeek);
    lua.set_function("vpoke",api_vpoke);
    lua.set_function("print",sol::overload(
        static_cast<void(*)(std::string, float, float, float, float)>(&api_print),
        static_cast<void(*)(std::string, float, float, float)>(&api_print)
    ));
    lua.set_function("pix",api_pix);
    lua.set_function("trace",sol::overload(
        static_cast<void(*)(std::string)>(&api_trace),
        static_cast<void(*)(float)>(&api_trace)
    ));
    lua.set_function("cls",api_cls);
    lua.set_function("rgb",api_rgb);
    lua.set_function("rgbd",api_rgbd);
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
    lua.set_function("loadspr",api_loadspr);
    lua.set_function("sprraw",api_sprraw);
    lua.set_function("showcur",api_showcur);
    lua.set_function("showcurp",api_showcurp);
    lua.set_function("startinput",api_startinput);
    lua.set_function("stopinput",api_stopinput);
    lua.set_function("resetinput",api_resetinput);
    lua.set_function("getinput",api_getinput);
    lua.set_function("resetgate",api_resetgate);
    lua.set_function("wtsync",api_wtsync);
    lua.set_function("put_dma_buffer", api_put_dma_buffer);
    lua.set_function("get_dma_buffer_length", api_get_dma_buffer_length);
    lua.set_function("vpu_init", api_vpu_init);
    lua.set_function("vpu_draw_debug", api_vpu_draw_debug);
    lua.set_function("vpu_draw_debug2", api_vpu_draw_debug2);
    lua.set_function("vpu_update_state", api_vpu_update_state);
    lua.set_function("file_open", api_file_open);
    lua.set_function("screen", api_screen);
    lua.set_function("printp", sol::overload(
        static_cast<void(*)(std::string)>(&api_printp),
        static_cast<void(*)(float)>(&api_printp)
    ));
    lua.set_function("printlnp", api_printlnp);
    lua.set_function("scrollp", api_scrollp);
    lua.set_function("lc", api_lc);
    lua.set_function("cursorp", api_cursorp);
    lua.set_function("colorfg", api_colorfg);
    lua.set_function("colorbg", api_colorbg);
    lua.set_function("movecursor", api_movecursor);
    lua.set_function("include", api_include);
    lua.set_function("init_sound_input", api_init_sound_input);
    lua.set_function("acquire_sound_input", api_acquire_sound_input);
    lua.set_function("acquire_sound_input_fft", api_acquire_sound_input_fft);
    lua.set_function("setpwrap", api_setpwrap);
    lua.set_function("settimerint", api_settimerint);
    lua.set_function("disposetimerint", api_disposetimerint);
    lua.set_function("resetvram", api_resetvram);
    lua.set_function("bitblt_lb", api_bitblt_lb);
    lua.set_function("bitblt_bb", api_bitblt_bb);
}

void init_lua() {
    // it will call opening screen, then it will be overridden by main entry point
    lua.open_libraries(
    sol::lib::base,
    sol::lib::package,
    sol::lib::math,
    sol::lib::string,
    sol::lib::table,
    sol::lib::package,
    sol::lib::utf8,
    sol::lib::io,
    sol::lib::os,
    sol::lib::coroutine,
    sol::lib::debug,
    sol::lib::bit32,
    sol::lib::ffi, // if using LuaJIT
    sol::lib::jit // if using LuaJIT
    );
    register_functions();
    lua["_CPT_VERSION"] = (std::string)VERSION_MAJOR "." VERSION_MINOR "." VERSION_REVISION VERSION_STATUS " (" VERSION_HASH ")";
    // in WASM build, some features will be limited
    #ifdef WASM_BUILD
    lua["_CPT_IS_WASM"] = 1;
    #else 
    lua["_CPT_IS_WASM"] = 0;
    #endif
    lua["_CPT_PRODUCT_NAME"] = CPT_PRODUCT_NAME;
    lua["_CPT_LANG"] = (std::string)APP_LANG;
    lua["_CPT_IS_LUAJIT"] = std::string(IS_LUAJIT) == "ON" ? 1 : 0;

    std::string subroutines_source = "";
    #include "lua/subroutine/s_io.lua.hpp"
    #include "lua/subroutine/s_game.lua.hpp"
    #include "lua/subroutine/s_image.lua.hpp"
    #include "lua/subroutine/s_common.lua.hpp"
    subroutines_source += s_io_source+"\n";
    subroutines_source += s_game_source+"\n";
    subroutines_source += s_image_source+"\n";
    subroutines_source += s_common_source+"\n";
    
    auto result = lua.safe_script(subroutines_source);
    if (!result.valid()) {
        sol::error err = result;
        report_lua_error(err);
    }
    
    result = lua.safe_script(APP_LANG == "ja" ? opening_ja_source : opening_source);
    if (!result.valid()) {
        sol::error err = result;
        report_lua_error(err);
    }
}

// These functions are called when specific events occur by C++ backend

void Lua_OnKeyDown(int key) {
    safe_lua_call("ONKEYDOWN", key);
}

void Lua_OnKeyUp(int key) {
    safe_lua_call("ONKEYUP", key);
}

// Process all pending timer events from the queue (call from main thread only)
void ProcessTimerEvents() {
    std::lock_guard<std::mutex> lock(timerQueueMutex);
    while (!timerEventQueue.empty()) {
        TimerEvent event = timerEventQueue.front();
        timerEventQueue.pop();
        // Now safely call Lua from main thread
        safe_lua_call("TIMERINT", (float)event.count);
    }
}

void Lua_MainLoop() {
    // Process any pending timer events first
    ProcessTimerEvents();
    // Then call the main loop
    safe_lua_call("LOOP");
}

void Lua_PostDraw() {
    safe_lua_call("POSTDRAW");
}

void Lua_OnInput(std::string inputChar) {
    safe_lua_call("ONINPUT", inputChar);
}