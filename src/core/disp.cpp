#include <iostream>
#include <vector>


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

    std::tuple<int, int> mouse() {
        // Get the mouse position
        // placeholder
        int x=0;
        int y=0;
        SDL_GetMouseState(&x,&y);
        return std::make_tuple(x, y);
    }

    void cls(int color = 0) {
        vram_pokefill(vram, 0, CPT_SCREEN_WIDTH * CPT_SCREEN_HEIGHT, color);
    }

    void pix(int x, int y, Byte color) {
        if (x < 384 && y < 288) {
        vram_poke(vram, y * CPT_SCREEN_WIDTH + x, color);
        }
    }

    void pixarr(int x, int y, int w, int h, std::vector<int> &colors) {
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
};

