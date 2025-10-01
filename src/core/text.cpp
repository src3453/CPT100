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
        clearPCG(0);
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
    #define PCG_SCREEN_WIDTH CPT_SCREEN_WIDTH/8
    #define PCG_SCREEN_HEIGHT CPT_SCREEN_HEIGHT/12

    void locatePCG(int x, int y) {
        // Set the cursor position for PCG
        cursor_x = x;
        cursor_y = y;
    }

    void moveCursorPCG(int dx, int dy) {
        cursor_x += dx;
        cursor_y += dy;

        if (cursor_x < 0) {cursor_x = 0; cursor_y--;}
        if (cursor_x >= PCG_SCREEN_WIDTH) {cursor_x = 0; cursor_y++;}
        if (cursor_y < 0) {cursor_y = 0;}
        if (cursor_y >= PCG_SCREEN_HEIGHT) {cursor_y = PCG_SCREEN_HEIGHT - 1;}
    }

    void setCursorVisibilityPCG(bool visible) {
        cursor_visible = visible;
    }

    void drawCharPCG(int mode) {
        if (mode == 1) {
            // Draw a character in mode 1
            //screen.cls(0);
            int i = 0;
            for (int y = 0; y < PCG_SCREEN_HEIGHT; ++y) {
                for (int x = 0; x < PCG_SCREEN_WIDTH; ++x) {
                    if (cursor_visible && x == cursor_x && y == cursor_y && blinktimer % 20 < 10) {
                        screen.rect(x * 8, y * 12, 8, 12, vram_peek(vram, PCG_OFFSET+i*3+1));
                        drawChar(vram_peek(vram, PCG_OFFSET+i*3), x * 8, y * 12, vram_peek(vram, PCG_OFFSET+i*3+2));
                    } else {
                        screen.rect(x * 8, y * 12, 8, 12, vram_peek(vram, PCG_OFFSET+i*3+2));
                        drawChar(vram_peek(vram, PCG_OFFSET+i*3), x * 8, y * 12, vram_peek(vram, PCG_OFFSET+i*3+1));
                    }
                   
                    i++;
                    
                }
            }
            blinktimer++;
        }
    }

    void setFGColor(Byte color) {
        colorFG = color;
    }

    void setBGColor(Byte color) {
        colorBG = color;
    }

    void printPCG(std::string text) {
        for (size_t i = 0; i < text.length(); ++i) {
            if (text[i] == '\n') {
                cursor_x = 0; // reset to start of line
                cursor_y += 1; // move to next line
            } else if (text[i] == '\r') {
                cursor_x = 0; // carriage return
            }
            else if (text[i] == '\t') {
                cursor_x += 4; // tab, move 4 spaces
            } else {
                vram_poke(vram, PCG_OFFSET+(cursor_y*(PCG_SCREEN_WIDTH)+cursor_x)*3+0, text[i]);
                vram_poke(vram, PCG_OFFSET+(cursor_y*(PCG_SCREEN_WIDTH)+cursor_x)*3+1, colorFG);
                vram_poke(vram, PCG_OFFSET+(cursor_y*(PCG_SCREEN_WIDTH)+cursor_x)*3+2, colorBG);
                cursor_x += 1;
            }
            if (cursor_x >= (PCG_SCREEN_WIDTH)) {
                cursor_x =  0; // carriage return
                cursor_y += 1; // line feed
            }
        }
    }

    void scrollPCG(int lines) {
        for (int i = 0; i < lines; ++i) {
            for (int y = 0; y < PCG_SCREEN_HEIGHT; ++y) {
                for (int x = 0; x < PCG_SCREEN_WIDTH; ++x) {
                    vram_poke(vram, PCG_OFFSET+(y*(PCG_SCREEN_WIDTH)+x)*3+0, vram_peek(vram, PCG_OFFSET+((y+1)*(PCG_SCREEN_WIDTH)+x)*3+0));
                    vram_poke(vram, PCG_OFFSET+(y*(PCG_SCREEN_WIDTH)+x)*3+1, vram_peek(vram, PCG_OFFSET+((y+1)*(PCG_SCREEN_WIDTH)+x)*3+1));
                    vram_poke(vram, PCG_OFFSET+(y*(PCG_SCREEN_WIDTH)+x)*3+2, vram_peek(vram, PCG_OFFSET+((y+1)*(PCG_SCREEN_WIDTH)+x)*3+2));
                }
            }
        }
    }

    void clearPCG(uint8_t color) {
        for (int i = 0; i < CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT / 96; ++i) {
            vram_poke(vram, PCG_OFFSET + i * 3 + 0, (char)0);
            vram_poke(vram, PCG_OFFSET + i * 3 + 1, 255);
            vram_poke(vram, PCG_OFFSET + i * 3 + 2, (uint8_t)color);
        }
    }


    void print(const std::string text, int x = 0, int y = 0, Byte color = 255) {
        for (size_t i = 0; i < text.length(); ++i) {
            drawChar((char)text[i], (x + 8 * i) % CPT_SCREEN_WIDTH, y + (i / 48) * 12, color);
        }
    }

private:
    CPT_Screen &screen;
    int cursor_x = 0;
    int cursor_y = 0;
    int colorFG = 255;
    int colorBG = 0;
    int blinktimer = 0;
    bool cursor_visible = true;

    void loadFontData() {
        // Load font data from a file or any data source and store it in fontData
        // Example: fontData[characterCode] = vector of pixel data
        // Note: This is a placeholder, you need to implement this part based on your data source
    }
};
