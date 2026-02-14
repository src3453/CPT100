#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <map>
#include <algorithm>

#include "res/font8x12.hpp"
#include "res/fontuni16_k8x12.hpp"
#include "res/cp437tou16table.hpp"

// Forward declaration or include the appropriate header

/*
SCREEN MODES (can be changed by screen(mode) API):
variable: int screenMode;
0: Graphics Mode, 384x288 pixels, direct pixel access
1: Character Mode (PCG), 48x24 characters, each character is 8x12 pixels
2: Alternative Character Mode, 96x48 characters, each character is 4x6 pixels (not yet implemented)
3: High Color Graphics Mode, 192x288 pixels, 16bpp (RGB565), big endian
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

    void drawChar(char ch, int x, int y, uint16_t color) {
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

    void drawCharUnicode16(uint16_t unicode, int x, int y, uint16_t color) {
        // Draw a Unicode character from the unicode16_font array
        for (int i = 0; i < 12; ++i) {
            unsigned char row;
            try{
                row = unicode16_font.at(unicode)[i];
            } catch (const std::out_of_range& e) {
                // Character not found, use blank character 
                row = 0x00;
            }
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

    void setAutoWrapPCG(bool enable) {
        auto_wrap = enable;
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
        else if (mode == 2) {
            // 16bit Unicode PCG mode
            int i = 0;
            for (int y = 0; y < PCG_SCREEN_HEIGHT; ++y) {
                for (int x = 0; x < PCG_SCREEN_WIDTH; ++x) {
                    if (cursor_visible && x == cursor_x && y == cursor_y && blinktimer % (BLINK_INTERVAL*2) < BLINK_INTERVAL) {
                        screen.rect(x * 8, y * 12, 8, 12, vram_peek(vram, PCG_OFFSET+i*4+2));
                        uint16_t unicode = (vram_peek(vram, PCG_OFFSET+i*4) << 8) | vram_peek(vram, PCG_OFFSET+i*4+1);
                        try{
                            uint8_t cp437_mapped_char = cp437tou16table.at(unicode);
                            drawChar((char)(cp437_mapped_char), x * 8, y * 12, vram_peek(vram, PCG_OFFSET+i*4+3));
                        } catch (const std::out_of_range& e) {
                            drawCharUnicode16(unicode, x * 8, y * 12, vram_peek(vram, PCG_OFFSET+i*4+3));
                        }
                    } else {
                        screen.rect(x * 8, y * 12, 8, 12, vram_peek(vram, PCG_OFFSET+i*4+3));
                        uint16_t unicode = (vram_peek(vram, PCG_OFFSET+i*4) << 8) | vram_peek(vram, PCG_OFFSET+i*4+1);
                        try{
                            uint8_t cp437_mapped_char = cp437tou16table.at(unicode);
                            drawChar((char)(cp437_mapped_char), x * 8, y * 12, vram_peek(vram, PCG_OFFSET+i*4+2));
                        } catch (const std::out_of_range& e) {
                            drawCharUnicode16(unicode, x * 8, y * 12, vram_peek(vram, PCG_OFFSET+i*4+2));
                        }
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
                if (screenMode == 2) {
                    throw std::runtime_error("Unicode PCG print not supported in this function with String type. (you misused lua api?)");
                } else {
                    vram_poke(vram, PCG_OFFSET+(cursor_y*(PCG_SCREEN_WIDTH)+cursor_x)*3+0, text[i]);
                    vram_poke(vram, PCG_OFFSET+(cursor_y*(PCG_SCREEN_WIDTH)+cursor_x)*3+1, colorFG);
                    vram_poke(vram, PCG_OFFSET+(cursor_y*(PCG_SCREEN_WIDTH)+cursor_x)*3+2, colorBG);
                }
                cursor_x += 1;
            }
            if (cursor_x >= (PCG_SCREEN_WIDTH) && auto_wrap) {
                cursor_x =  0; // carriage return
                cursor_y += 1; // line feed
            }
            if (cursor_y >= PCG_SCREEN_HEIGHT && auto_wrap) {
                cursor_y = PCG_SCREEN_HEIGHT - 1;
                scrollPCG(1);
            }
        }
    }

    void printPCG(std::u16string text) {
        for (size_t i = 0; i < text.length(); ++i) {
            if (text[i] == u'\n') {
                cursor_x = 0; // reset to start of line
                cursor_y += 1; // move to next line
            } else if (text[i] == u'\r') {
                cursor_x = 0; // carriage return
            }
            else if (text[i] == u'\t') {
                cursor_x += 4; // tab, move 4 spaces
            } else {
                if (screenMode != 2) {
                    throw std::runtime_error("Unicode PCG print only supported in mode 2 with Unicode16 type.");
                } else {
                    // CPT200 uses UTF-16BE encoding for Unicode characters
                    int n = 1;
                    uint16_t be_char;
                    be_char = text[i];
                    vram_poke(vram, PCG_OFFSET+(cursor_y*(PCG_SCREEN_WIDTH)+cursor_x)*4+0, (be_char >> 8) & 0xFF);
                    vram_poke(vram, PCG_OFFSET+(cursor_y*(PCG_SCREEN_WIDTH)+cursor_x)*4+1, be_char & 0xFF);
                    vram_poke(vram, PCG_OFFSET+(cursor_y*(PCG_SCREEN_WIDTH)+cursor_x)*4+2, colorFG);
                    vram_poke(vram, PCG_OFFSET+(cursor_y*(PCG_SCREEN_WIDTH)+cursor_x)*4+3, colorBG);
                }
                cursor_x += 1;
            }
            if (cursor_x >= (PCG_SCREEN_WIDTH) && auto_wrap) {
                cursor_x =  0; // carriage return
                cursor_y += 1; // line feed
            }
            if (cursor_y >= PCG_SCREEN_HEIGHT && auto_wrap) {
                cursor_y = PCG_SCREEN_HEIGHT - 1;
                scrollPCG(1);
            }
        }
    }

    void scrollPCG(int lines) {
        if (screenMode == 2) {
            for (int i = 0; i < lines; ++i) {
                for (int y = 0; y < PCG_SCREEN_HEIGHT-1; ++y) {
                    for (int x = 0; x < PCG_SCREEN_WIDTH; ++x) {
                        vram_poke(vram, PCG_OFFSET+(y*(PCG_SCREEN_WIDTH)+x)*4+0, vram_peek(vram, PCG_OFFSET+((y+1)*(PCG_SCREEN_WIDTH)+x)*4+0));
                        vram_poke(vram, PCG_OFFSET+(y*(PCG_SCREEN_WIDTH)+x)*4+1, vram_peek(vram, PCG_OFFSET+((y+1)*(PCG_SCREEN_WIDTH)+x)*4+1));
                        vram_poke(vram, PCG_OFFSET+(y*(PCG_SCREEN_WIDTH)+x)*4+2, vram_peek(vram, PCG_OFFSET+((y+1)*(PCG_SCREEN_WIDTH)+x)*4+2));
                        vram_poke(vram, PCG_OFFSET+(y*(PCG_SCREEN_WIDTH)+x)*4+3, vram_peek(vram, PCG_OFFSET+((y+1)*(PCG_SCREEN_WIDTH)+x)*4+3));
                    }
                }
                // Clear the last line
                for (int x = 0; x < PCG_SCREEN_WIDTH; ++x)
                {
                    vram_poke(vram, PCG_OFFSET+((PCG_SCREEN_HEIGHT-1)*(PCG_SCREEN_WIDTH)+x)*4+0, (char)0);
                    vram_poke(vram, PCG_OFFSET+((PCG_SCREEN_HEIGHT-1)*(PCG_SCREEN_WIDTH)+x)*4+1, (char)0);
                    vram_poke(vram, PCG_OFFSET+((PCG_SCREEN_HEIGHT-1)*(PCG_SCREEN_WIDTH)+x)*4+2, 255);
                    vram_poke(vram, PCG_OFFSET+((PCG_SCREEN_HEIGHT-1)*(PCG_SCREEN_WIDTH)+x)*4+3, (uint8_t)0);
                }
            }
        } else {
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
    }

    void clearPCG(uint8_t color) {
        if (screenMode == 2) {
            for (int i = 0; i < CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT / 96; ++i) {
                vram_poke(vram, PCG_OFFSET + i * 4 + 0, (char)0);
                vram_poke(vram, PCG_OFFSET + i * 4 + 1, (char)0);
                vram_poke(vram, PCG_OFFSET + i * 4 + 2, 255);
                vram_poke(vram, PCG_OFFSET + i * 4 + 3, (uint8_t)color);
            }
        } else {
            for (int i = 0; i < CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT / 96; ++i) {
                vram_poke(vram, PCG_OFFSET + i * 3 + 0, (char)0);
                vram_poke(vram, PCG_OFFSET + i * 3 + 1, 255);
                vram_poke(vram, PCG_OFFSET + i * 3 + 2, (uint8_t)color);
            }
        }
    }


    void print(const std::string text, int x = 0, int y = 0, uint16_t color = 255) {
        for (size_t i = 0; i < text.length(); ++i) {
            drawChar((char)text[i], (x + 8 * i), y, color);
        }
    }

    void printUnicode16(const std::u16string text, int x = 0, int y = 0, uint16_t color = 255) {
        for (size_t i = 0; i < text.length(); ++i) {
            //printf("Printing Unicode char: U+%04X\n", text[i]);
            uint16_t unicode = text[i];
            try{
                uint8_t cp437_mapped_char = cp437tou16table.at(unicode);
                drawChar((char)(cp437_mapped_char), (x + 8 * i), y, color);
            } catch (const std::out_of_range& e) {
                drawCharUnicode16(unicode, (x + 8 * i), y, color);
            }
        }
    }


private:
    CPT_Screen &screen;
    int cursor_x = 0;
    int cursor_y = 0;
    int colorFG = 255;
    int colorBG = 0;
    int blinktimer = 0;
    bool auto_wrap = true;
    bool cursor_visible = true;

};
