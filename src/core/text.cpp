#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <map>
#include <algorithm>

#include "res/font8x12.hpp"

class Font {
public:
    Font(CPT_Screen &screen) : screen(screen) {
    }


    void drawChar(char ch, int x, int y, Byte color) {
        // Get the character index (0-255)
        unsigned char charIndex = static_cast<unsigned char>(ch);
        
        // Draw the character
        for (int i = 0; i < 12; ++i) {
            unsigned char row = font8x12[charIndex][i];
            for (int j = 0; j < 8; ++j) {
                // Check if the bit is set (1 = pixel on, 0 = pixel off)
                if ((row >> (7 - j)) & 0x01) {
                    screen.pix(x + j, y + i, color);
                }
            }
        }
    }

    #define PCG_OFFSET 0x1c000

    void drawCharPCG(int mode) {
        if (mode == 1) {
            // Draw a character in mode 1
            //screen.cls(0);
            int i = 0;
            for (int y = 0; y < CPT_SCREEN_HEIGHT/12; ++y) {
                for (int x = 0; x < CPT_SCREEN_WIDTH/8; ++x) {
                    screen.rect(x * 8, y * 12, 8, 12, vram_peek(vram, PCG_OFFSET+i*3+2));
                    drawChar(vram_peek(vram, PCG_OFFSET+i*3), x * 8, y * 12, vram_peek(vram, PCG_OFFSET+i*3+1));
                    i++;
                }
            }
        }
    }

    void printPCG(std::string text, int x = 0, int y = 0, Byte color = 255, Byte BGcolor = 0) {
        for (size_t i = 0; i < text.length(); ++i) {
            vram_poke(vram, PCG_OFFSET+(y*(CPT_SCREEN_WIDTH/8)+x+i)*3+0, text[i]);
            vram_poke(vram, PCG_OFFSET+(y*(CPT_SCREEN_WIDTH/8)+x+i)*3+1, color);
            vram_poke(vram, PCG_OFFSET+(y*(CPT_SCREEN_WIDTH/8)+x+i)*3+2, BGcolor);
        }
    }

    void scrollPCG(int lines) {
        for (int i = 0; i < lines; ++i) {
            for (int y = 0; y < CPT_SCREEN_HEIGHT/12; ++y) {
                for (int x = 0; x < CPT_SCREEN_WIDTH/8; ++x) {
                    vram_poke(vram, PCG_OFFSET+(y*(CPT_SCREEN_WIDTH/8)+x)*3+0, vram_peek(vram, PCG_OFFSET+((y+1)*(CPT_SCREEN_WIDTH/8)+x)*3+0));
                    vram_poke(vram, PCG_OFFSET+(y*(CPT_SCREEN_WIDTH/8)+x)*3+1, vram_peek(vram, PCG_OFFSET+((y+1)*(CPT_SCREEN_WIDTH/8)+x)*3+1));
                    vram_poke(vram, PCG_OFFSET+(y*(CPT_SCREEN_WIDTH/8)+x)*3+2, vram_peek(vram, PCG_OFFSET+((y+1)*(CPT_SCREEN_WIDTH/8)+x)*3+2));
                }
            }
        }
    }


    void print(const std::string text, int x = 0, int y = 0, Byte color = 255) {
        for (size_t i = 0; i < text.length(); ++i) {
            drawChar((char)text[i], (x + 8 * i) % CPT_SCREEN_WIDTH, y + (i / 48) * 12, color);
        }
    }

private:
    CPT_Screen &screen;

    void loadFontData() {
        // Load font data from a file or any data source and store it in fontData
        // Example: fontData[characterCode] = vector of pixel data
        // Note: This is a placeholder, you need to implement this part based on your data source
    }
};
