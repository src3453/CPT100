#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <map>
#include <algorithm>

#include "res/font8x12.hpp"

/*
SCREEN MODES (can be changed by screen(mode) API):
0: Graphics Mode, 384x288 pixels, direct pixel access
1: Character Mode (PCG), 48x24 characters, each character is 8x12 pixels
2: Alternative Character Mode, 96x48 characters, each character is 4x6 pixels (not yet implemented)
*/
// TODO: Add 4x6 font and 96x48 screen mode (screen(2))


#define PCG_OFFSET 0x1c000
#define FONT_OFFSET 0x1b300

class Font {
public:
    Font(CPT_Screen &screen) : screen(screen) {
        
    }

    void loadFontData() {
        // Load font data into VRAM at FONT_OFFSET
        for (int i = 0; i < 256; ++i) {
            for (int j = 0; j < 12; ++j) {
                vram_poke(vram, FONT_OFFSET + i * 12 + j, font8x12[i][j]);
            }
        }
    }

    void drawChar(char ch, int x, int y, Byte color) {
        // Get the character index (0-255)
        unsigned char charIndex = static_cast<unsigned char>(ch);
        
        // Draw the character
        for (int i = 0; i < 12; ++i) {
            unsigned char row = vram_peek(vram, FONT_OFFSET + charIndex * 12 + i);
            for (int j = 0; j < 8; ++j) {
                // Check if the bit is set (1 = pixel on, 0 = pixel off)
                if ((row >> (7 - j)) & 0x01) {
                    screen.pix(x + j, y + i, color);
                }
            }
        }
    }

    #define BLINK_INTERVAL 10 // Number of frames for blink toggle
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
                    if (cursor_visible && x == cursor_x && y == cursor_y && blinktimer % (BLINK_INTERVAL*2) < BLINK_INTERVAL) {
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
            if (cursor_y >= PCG_SCREEN_HEIGHT) {
                cursor_y = PCG_SCREEN_HEIGHT - 1;
                scrollPCG(1);
            }
        }
    }

    void scrollPCG(int lines) {
        for (int i = 0; i < lines; ++i) {
            for (int y = 0; y < PCG_SCREEN_HEIGHT-1; ++y) {
                for (int x = 0; x < PCG_SCREEN_WIDTH; ++x) {
                    vram_poke(vram, PCG_OFFSET+(y*(PCG_SCREEN_WIDTH)+x)*3+0, vram_peek(vram, PCG_OFFSET+((y+1)*(PCG_SCREEN_WIDTH)+x)*3+0));
                    vram_poke(vram, PCG_OFFSET+(y*(PCG_SCREEN_WIDTH)+x)*3+1, vram_peek(vram, PCG_OFFSET+((y+1)*(PCG_SCREEN_WIDTH)+x)*3+1));
                    vram_poke(vram, PCG_OFFSET+(y*(PCG_SCREEN_WIDTH)+x)*3+2, vram_peek(vram, PCG_OFFSET+((y+1)*(PCG_SCREEN_WIDTH)+x)*3+2));
                }
            }
            // Clear the last line
            for (int x = 0; x < PCG_SCREEN_WIDTH; ++x)
            {
                vram_poke(vram, PCG_OFFSET+((PCG_SCREEN_HEIGHT-1)*(PCG_SCREEN_WIDTH)+x)*3+0, (char)0);
                vram_poke(vram, PCG_OFFSET+((PCG_SCREEN_HEIGHT-1)*(PCG_SCREEN_WIDTH)+x)*3+1, 255);
                vram_poke(vram, PCG_OFFSET+((PCG_SCREEN_HEIGHT-1)*(PCG_SCREEN_WIDTH)+x)*3+2, (uint8_t)0);
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

};
