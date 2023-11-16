#include <iostream>
#include <vector>


class CPT_Screen {
public:

    CPT_Screen(std::vector<Byte>& vram) {
        // Any initialization code if needed
    }

    uint8_t *blit(uint8_t *tmp_pixels) {
        
        std::vector<Byte> tmp = vram_peek2array(vram, 0, SCREEN_WIDTH * SCREEN_HEIGHT - 1);
        int i = 0;
        for (int y = 0; y < SCREEN_HEIGHT; y++){
            for (int x = 0; x < SCREEN_WIDTH; x++){
                tmp_pixels[i*3+0] = CLUT_DEF[tmp[i].toInt()*3+0];
                tmp_pixels[i*3+1] = CLUT_DEF[tmp[i].toInt()*3+1];
                tmp_pixels[i*3+2] = CLUT_DEF[tmp[i].toInt()*3+2];
                i+=1;
            }
        }
        return tmp_pixels;
        // Assuming clut is defined elsewhere
        // Replace this with your actual pixel drawing logic
        // You might need to handle the surface creation differently
        // and copy the pixel data from `tmp` to your drawing surface
    }

    uint8_t *update(uint8_t *tmp_pixels) {
        // TODO Implement blit
        return blit(tmp_pixels);
        // Update logic for the screen
    }

    std::tuple<int, int> mouse() {
        // Get the mouse position
        // placeholder
        int x=0;
        int y=0;
        return std::make_tuple(x, y);
    }

    void cls(int color = 0) {
        vram_pokefill(vram, 0, VRAM_SIZE, color);
    }

    void pix(int x, int y, Byte color) {
        vram_poke(vram, y * SCREEN_WIDTH + x, color);
    }

    void pixarr(int x, int y, std::vector<int> &colors) {
        int i = 0;
        for (int posY = y; posY < y + 256; ++posY) {
            for (int posX = x; posX < x + 384; ++posX) {
                pix(posX, posY, colors[i]);
                ++i;
            }
        }
    }
};

