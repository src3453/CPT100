#include <iostream>
#include <vector>


class Screen {
public:
    Ram ram;

    void init() {
        // Any initialization code if needed
    }

    void blit() {
        std::vector<Byte> tmp = ram.peek2array(0, VRAM_SIZE);
        // Assuming clut is defined elsewhere
        // Replace this with your actual pixel drawing logic
        // You might need to handle the surface creation differently
        // and copy the pixel data from `tmp` to your drawing surface
    }

    void update() {
        blit();
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
        ram.pokefill(0, VRAM_SIZE, color);
    }

    void pix(int x, int y, int color) {
        ram.poke(y * SCREEN_WIDTH + x, color);
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

