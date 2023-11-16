#include <array>
#include <vector>
#include <string>
#include <list>


// 定数の定義
#define RAM_SIZE 0x20000  // 128KB
#define VRAM_SIZE 0x20000 // 128KB

#define SCREEN_WIDTH 384
#define SCREEN_HEIGHT 288

// 色の定義
uint8_t CLUT_DEF[768] = {0,0,0,0,0,85,0,0,170,0,0,255,0,85,0,0,85,85,0,85,170,0,85,255,0,170,0,0,170,85,0,170,170,0,170,255,0,255,0,0,255,85,0,255,170,0,255,255,85,0,0,85,0,85,85,0,170,85,0,255,85,85,0,85,85,85,85,85,170,85,85,255,85,170,0,85,170,85,85,170,170,85,170,255,85,255,0,85,255,85,85,255,170,85,255,255,170,0,0,170,0,85,170,0,170,170,0,255,170,85,0,170,85,85,170,85,170,170,85,255,170,170,0,170,170,85,170,170,170,170,170,255,170,255,0,170,255,85,170,255,170,170,255,255,255,0,0,255,0,85,255,0,170,255,0,255,255,85,0,255,85,85,255,85,170,255,85,255,255,170,0,255,170,85,255,170,170,255,170,255,255,255,0,255,255,85,255,255,170,255,255,255,0,0,0,0,0,85,0,0,170,0,0,255,0,85,0,0,85,85,0,85,170,0,85,255,0,170,0,0,170,85,0,170,170,0,170,255,0,255,0,0,255,85,0,255,170,0,255,255,85,0,0,85,0,85,85,0,170,85,0,255,85,85,0,85,85,85,85,85,170,85,85,255,85,170,0,85,170,85,85,170,170,85,170,255,85,255,0,85,255,85,85,255,170,85,255,255,170,0,0,170,0,85,170,0,170,170,0,255,170,85,0,170,85,85,170,85,170,170,85,255,170,170,0,170,170,85,170,170,170,170,170,255,170,255,0,170,255,85,170,255,170,170,255,255,255,0,0,255,0,85,255,0,170,255,0,255,255,85,0,255,85,85,255,85,170,255,85,255,255,170,0,255,170,85,255,170,170,255,170,255,255,255,0,255,255,85,255,255,170,255,255,255,0,0,0,0,0,85,0,0,170,0,0,255,0,85,0,0,85,85,0,85,170,0,85,255,0,170,0,0,170,85,0,170,170,0,170,255,0,255,0,0,255,85,0,255,170,0,255,255,85,0,0,85,0,85,85,0,170,85,0,255,85,85,0,85,85,85,85,85,170,85,85,255,85,170,0,85,170,85,85,170,170,85,170,255,85,255,0,85,255,85,85,255,170,85,255,255,170,0,0,170,0,85,170,0,170,170,0,255,170,85,0,170,85,85,170,85,170,170,85,255,170,170,0,170,170,85,170,170,170,170,170,255,170,255,0,170,255,85,170,255,170,170,255,255,255,0,0,255,0,85,255,0,170,255,0,255,255,85,0,255,85,85,255,85,170,255,85,255,255,170,0,255,170,85,255,170,170,255,170,255,255,255,0,255,255,85,255,255,170,255,255,255,0,0,0,0,0,85,0,0,170,0,0,255,0,85,0,0,85,85,0,85,170,0,85,255,0,170,0,0,170,85,0,170,170,0,170,255,0,255,0,0,255,85,0,255,170,0,255,255,85,0,0,85,0,85,85,0,170,85,0,255,85,85,0,85,85,85,85,85,170,85,85,255,85,170,0,85,170,85,85,170,170,85,170,255,85,255,0,85,255,85,85,255,170,85,255,255,170,0,0,170,0,85,170,0,170,170,0,255,170,85,0,170,85,85,170,85,170,170,85,255,170,170,0,170,170,85,170,170,170,170,170,255,170,255,0,170,255,85,170,255,170,170,255,255,255,0,0,255,0,85,255,0,170,255,0,255,255,85,0,255,85,85,255,85,170,255,85,255,255,170,0,255,170,85,255,170,170,255,170,255,255,255,0,255,255,85,255,255,170,255,255,255};

std::string FONT_8_12_DIR = "core/res/font8x8.fnt";
std::string FONT_6_12_DIR = ""; // TODO: 4x8フォントを作成する
