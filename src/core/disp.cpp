#include <iostream>
#include <vector>
//#include "header/spec.hpp"
//#include "ram.cpp"
#include <math.h>

// VDC: Video Display Controller for 2D graphics rendering and character display

/*
SCREEN MODES (can be changed by screen(mode) API):
variable: int screenMode;
0: Graphics Mode, 384x288 pixels, direct pixel access
1: Character Mode (PCG), 48x24 characters, each character is 8x12 pixels
2: Alternative Character Mode, 96x48 characters, each character is 4x6 pixels (not yet implemented)
3: High Color Graphics Mode, 192x288 pixels, 16bpp (RGB565), big endian
*/

/*
VRAM Layout:
0x00000 - 0x1AFFF: Screen Data (384x288 pixels, 1 byte per pixel)
0x1B000 - 0x1B2FF: Color Lookup Table (CLUT) (256 colors, 3 bytes each: R, G, B)
0x1B300 - 0x1BEFF: Font Data (8x12 font, 256 characters, 12 bytes each)
0x1BF00 - 0x1BFFF: Reserved
0x1C000 - 0x1CD7F: PCG Data (8x12 font, 48x24 screen size, 3 bytes each: char code (as in CP437), fg color, bg color)
0x1CD80 - 0x1CFFF: Reserved 
0x1D000 - 0x1FFFF: Sprite Data 
0x20000 - 0x7FFFF: for VPU (3D engine registers, Mesh and Texture Buffers)
0x80000 - 0xFFFFF: Reserved (for user data)
*/

#define clamp(x,xMin,xMax) if ((x)<(xMin)) (x)=(xMin); else if ((x)>(xMax)) (x)=(xMax); else (x)=(x);

int wx,wy,ww,wh = 0;

class CPT_Screen {
public:

    CPT_Screen(std::vector<uint8_t>& vram) {
        // Any initialization code if needed
    }
    void init() {
        vram_poke2array(vram, 0x1b000, CLUT_DEF);
    }
    void blit(uint8_t *tmp_pixels) {
        
        std::vector<uint8_t> tmp = vram_peek2array(vram, 0, CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT);
        std::vector<uint8_t> CLUT = vram_peek2array(vram, 0x1b000, 0x300);
        int i = 0;
        if (screenMode == 3) {
            for (int y = 0; y < CPT_SCREEN_HEIGHT; y++){
                for (int x = 0; x < CPT_SCREEN_WIDTH/2; x++){
                    uint8_t byte1 = tmp[i*2+0];
                    uint8_t byte2 = tmp[i*2+1];
                    uint16_t pixel = (byte1 << 8) | byte2;
                    uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                    uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                    uint8_t b = (pixel & 0x1F) << 3;
                    tmp_pixels[i*8+0] = r;
                    tmp_pixels[i*8+1] = g;
                    tmp_pixels[i*8+2] = b;
                    tmp_pixels[i*8+3] = 255;
                    tmp_pixels[i*8+4] = r;
                    tmp_pixels[i*8+5] = g;
                    tmp_pixels[i*8+6] = b;
                    tmp_pixels[i*8+7] = 255;
                    i+=1;
                }
            }
        } else {
            for (int y = 0; y < CPT_SCREEN_HEIGHT; y++){
                for (int x = 0; x < CPT_SCREEN_WIDTH; x++){
                    uint8_t idx = tmp[i];
                    tmp_pixels[i*4+0] = CLUT[idx*3+0];
                    tmp_pixels[i*4+1] = CLUT[idx*3+1];
                    tmp_pixels[i*4+2] = CLUT[idx*3+2];
                    tmp_pixels[i*4+3] = (idx == 0) ? 0 : 255;
                    i+=1;
                }
            }
        }
        
        // Assuming clut is defined elsewhere
        // Replace this with your actual pixel drawing logic
        // You might need to handle the surface creation differently
        // and copy the pixel data from `tmp` to your drawing surface
    }

    void update(uint8_t *tmp_pixels) {
        // TODO Implement blit
        blit(tmp_pixels);
        // Update logic for the screen
    }

    std::tuple<int, int, int> mouse() {
        // Get the mouse position
        // placeholder
        int x=0;
        int y=0;
        SDL_GetMouseState(&x,&y);
        x = (int)((double)x/((double)ww/CPT_SCREEN_WIDTH))-((double)(wx)/((double)ww/CPT_SCREEN_WIDTH));
        y = (int)((double)y/((double)wh/CPT_SCREEN_HEIGHT))-((double)(wy)/((double)wh/CPT_SCREEN_HEIGHT));
        clamp(x,0,CPT_SCREEN_WIDTH-1);
        clamp(y,0,CPT_SCREEN_HEIGHT-1);
        if (screenMode == 3) {
            x = x / 2;
        }
        return std::make_tuple(x, y, mouseState);
    }

    void cls(uint16_t color = 0) {
        if (screenMode == 3) {
            uint8_t byte1 = (color >> 8) & 0xFF;
            uint8_t byte2 = color & 0xFF;
            for (int i = 0; i < CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT; i+=2) {
                vram_poke(vram, i, byte1);
                vram_poke(vram, i+1, byte2);
            }
        } else {
            vram_pokefill(vram, 0, CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT, color);
        }
    }

    void pix(int x, int y, uint16_t color) {

        if (screenMode == 3) {
            if (x >= 0 && y >= 0 && x < 192 && y < 288) {
                vram_poke(vram, y * CPT_SCREEN_WIDTH + x*2, (color >> 8) & 0xFF);
                vram_poke(vram, y * CPT_SCREEN_WIDTH + x*2+1, color & 0xFF);
            }
        } else {
            if (x >= 0 && y >= 0 && x < 384 && y < 288) {
                vram_poke(vram, y * CPT_SCREEN_WIDTH + x, color);
            }
        }
    }

    void pixarr(int x, int y, int w, int h, std::vector<uint16_t> &colors) {
        int i = 0;
        for (int posY = y; posY < y + h; ++posY) {
            for (int posX = x; posX < x + w; ++posX) {
                pix(posX, posY, colors[i]);
                ++i;
            }
        }
    }

    // BitBlt (linear to block)
    void bitblt(int addr, int x, int y, int w, int h, int transparent_color=-1) {
        std::vector<uint8_t> tmp = vram_peek2array(vram, addr, w * h);
        int i = 0;
        for (int j = 0; j < h; ++j) {
            for (int i = 0; i < w; ++i) {
                uint8_t color = tmp[j * w + i];
                if (transparent_color == -1 || color != transparent_color) {
                    pix(x + i, y + j, color);
                }
            }
        }
    }

    // BitBlt (block to block)
    void bitblt_block_to_block(int base_addr, int src_x, int src_y, int image_w, int image_h, int x, int y, int w, int h, int transparent_color=-1) {
        for (int j = 0; j < h; ++j) {
            for (int i = 0; i < w; ++i) {
                int src_addr = base_addr + (src_y + j) * image_w + (src_x + i);
                uint8_t color = vram_peek(vram, src_addr);
                if (transparent_color == -1 || color != transparent_color) {
                    pix(x + i, y + j, color);
                }
            }
        }
    }
    
    void spr(int num, int x, int y, int w=1, int h=1) {
        //FIXME: adopt to 16bpp mode
        //std::vector<uint16_t> data = vram_peek2array(vram,num*64,64);
        //pixarr(x,y,8,8,data);
    }

    void rect(int x, int y, int w, int h, uint16_t color) {
        for (int posY = y; posY < y + h; ++posY) {
            for (int posX = x; posX < x + w; ++posX) {
                pix(posX, posY, color);
            }
        }
    }

    void rectb(int x, int y, int w, int h, uint16_t color) {
        for(int X=x;X<x+w;X++){
            pix(X, y, color);
        }
        for(int Y=y;Y<y+h;Y++){
            pix(x, Y, color);
        }
        for(int X=x;X<x+w;X++){
            pix(X, y+h, color);
        }
        for(int Y=y;Y<=y+h;Y++){
            pix(x+w, Y, color);
        }
    }


    void line(
    int xs, /* 線の始点のx座標 */
    int ys, /* 線の始点のy座標 */
    int xe, /* 線の終点のx座標 */
    int ye, /* 線の終点のy座標 */
    uint16_t color
    ){
        int x, y;
        int dx, dy;
        double rad;
        unsigned int length;
        unsigned int l;
        int sigx, sigy;

        /* 始点と終点のx座標とy座標の差を計算 */
        dx = xe - xs;
        dy = ye - ys;

        /* 線の長さを計算 */
        length = sqrt(dx * dx + dy * dy);

        /* 横軸との成す角を計算 */
        rad = atan2(dy, dx);

        /* 長さ分の線を描画 */
        for(l = 0; l < length; l++){
            /* x座標とy座標を計算 */
            x = xs + l * cos(rad);
            y = ys + l * sin(rad);

            /* ビットマップ外の点は描画しない */
            pix((int)x,(int)y,(uint16_t)color);

        }
    }
};

