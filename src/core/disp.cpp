#include <iostream>
#include <vector>
#include <math.h>

class CPT_Screen {
public:

    CPT_Screen(std::vector<Byte>& vram) {
        // Any initialization code if needed
    }
    void init() {
        vram_poke2array(vram, 0x1b000, CLUT_DEF);
    }
    void blit(uint8_t *tmp_pixels) {
        
        std::vector<Byte> tmp = vram_peek2array(vram, 0, CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT);
        std::vector<Byte> CLUT = vram_peek2array(vram, 0x1b000, 0x300);
        int i = 0;
        for (int y = 0; y < CPT_SCREEN_HEIGHT; y++){
            for (int x = 0; x < CPT_SCREEN_WIDTH; x++){
                tmp_pixels[i*3+0] = CLUT.at(tmp.at(i).toInt()*3+0).toInt();
                tmp_pixels[i*3+1] = CLUT.at(tmp.at(i).toInt()*3+1).toInt();
                tmp_pixels[i*3+2] = CLUT.at(tmp.at(i).toInt()*3+2).toInt();
                i+=1;
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
        
        return std::make_tuple(x, y, mouseState);
    }

    void cls(Byte color = 0) {
        vram_pokefill(vram, 0, CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT, color);
    }

    void pix(int x, int y, Byte color) {
        if (x < 384 && y < 288) {
        vram_poke(vram, y * CPT_SCREEN_WIDTH + x, color);
        }
    }

    void pixarr(int x, int y, int w, int h, std::vector<Byte> &colors) {
        int i = 0;
        for (int posY = y; posY < y + h; ++posY) {
            for (int posX = x; posX < x + w; ++posX) {
                pix(posX, posY, colors[i]);
                ++i;
            }
        }
    }

    void rect(int x, int y, int w, int h, Byte color) {
        for (int posY = y; posY < y + h; ++posY) {
            for (int posX = x; posX < x + w; ++posX) {
                pix(posX, posY, color);
            }
        }
    }

    void rectb(int x, int y, int w, int h, Byte color) {
        for(int X=x;X<x+w;X++){
            pix(X, y, color);
        }
        for(int Y=y;Y<y+h;Y++){
            pix(x, Y, color);
        }
        for(int X=x;X<x+w;X++){
            pix(X, y+h, color);
        }
        for(int Y=y;Y<y+h;Y++){
            pix(x+w, Y, color);
        }
    }

    Byte fromRGB(int r, int g, int b) {
        r = (int)(r%256/42.666666666666666);
        g = (int)(g%256/42.666666666666666);
        b = (int)(b%256/42.666666666666666);
        if (r+g+b == 15) {
            return Byte(255);
        } else {
            return (Byte)((r*36+g*6+b)%215);
        }
    }

    void line(
    int xs, /* 線の始点のx座標 */
    int ys, /* 線の始点のy座標 */
    int xe, /* 線の終点のx座標 */
    int ye, /* 線の終点のy座標 */
    Byte color
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
            pix((int)x,(int)y,(Byte)color);

        }
    }
};

