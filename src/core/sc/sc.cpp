#include <vector>
#include <map>
#include <math.h>
#include <algorithm>
#include "../lib/stb/stb_image.h"
#include "../lib/stb/stb_image_write.h"
#include "../lib/stb/stb_image_resize2.h"

// SC: Sprite Controller for 2D sprite management and rendering

/*
Sprite Controller Specification
Max Sprites: 256
Max Texture Size: 256x256 pixels (65536 bytes per texture)

VRAM Layout:
0x1D000 - 0x1DBFF: Sprite Data (256 sprites, 12 bytes each)
0x1DC00 - 0x1FFFF: Reserved
*/

// VRAM access macros
#define vRead8(addr) (vram[addr])
#define vRead16(addr) (vRead8(addr) | (vRead8(addr + 1) << 8))
#define vRead24(addr) (vRead8(addr) | (vRead8(addr + 1) << 8) | (vRead8(addr + 2) << 16))
#define vWrite8(addr, val) (vram[addr] = (val) & 0xFF)
#define vWrite16(addr, val) { vWrite8(addr, (val) & 0xFF); vWrite8(addr + 1, ((val) >> 8) & 0xFF); }
#define vWrite24(addr, val) { vWrite8(addr, (val) & 0xFF); vWrite8(addr + 1, ((val) >> 8) & 0xFF); vWrite8(addr + 2, ((val) >> 16) & 0xFF); }

struct Texture2D {
    uint8_t width;  // Texture dimensions X, 1~256 (0 means 1, 255 means 256)
    uint8_t height; // Texture dimensions Y, 1~256 (0 means 1, 255 means 256)
    uint32_t offset; // Offset in VRAM (in 24-bit addressing)
};

struct SpriteAttributes {
    bool visible; // Visibility flag
    bool flipX;   // Flip horizontally
    bool flipY;   // Flip vertically
    uint8_t priority; // lower number = higher priority
    uint8_t rotation; // in degrees (0 means no rotation, 128 means 180 degree, degree = rotation * 1.40625)
};

struct Sprite {
    uint16_t x;
    uint16_t y;
    Texture2D texture;
    SpriteAttributes attributes;
};

class SpriteController {
public:
    SpriteController(std::vector<unsigned char>& vram, CPT_Screen &screen) : vram(vram), screen(screen) {

    }

    ~SpriteController() {
        // Destructor implementation (if needed)
    }

    void updateSprites() {
        // Load sprite data from VRAM and update internal sprite list
        sprites.clear();
        for (int i = 0; i < 256; i++) {
            int baseAddr = 0x1D000 + i * 12;
            Sprite sprite;
            sprite.x = vRead16(baseAddr);
            sprite.y = vRead16(baseAddr + 2);
            sprite.texture.width = vRead8(baseAddr + 4);
            sprite.texture.height = vRead8(baseAddr + 5);
            sprite.texture.offset = vRead24(baseAddr + 6);
            uint8_t attr = vRead8(baseAddr + 9);
            sprite.attributes.visible = (attr & 0x01) != 0;
            sprite.attributes.flipX = (attr & 0x02) != 0;
            sprite.attributes.flipY = (attr & 0x04) != 0;
            sprite.attributes.priority = vRead8(baseAddr + 10);
            sprite.attributes.rotation = vRead8(baseAddr + 11);
            sprites[i] = sprite;
        }
    }

    void renderSprites() {
        // Render Sprites based on VRAM data
        // Sort sprites by priority
        std::vector<std::pair<int, Sprite>> spriteList;
        for (const auto& pair : sprites) {
            if (pair.second.attributes.visible) {
                spriteList.push_back(pair);
            }
        }
        std::sort(spriteList.begin(), spriteList.end(), [](const auto& a, const auto& b) {
            return a.second.attributes.priority < b.second.attributes.priority;
        });

        // Render each sprite
        for (const auto& pair : spriteList) {
            const Sprite& sprite = pair.second;
            // Render logic here (omitted for brevity)
            // You would read the texture data from VRAM using sprite.texture.offset
            // and draw it on the screen at (sprite.x, sprite.y) with the specified attributes
        }
    }

    void loadSpriteFromLocalImage(int spriteIndex, const std::string& filepath) {
        // Load image from local file and upload to VRAM at sprite texture offset
        // This is a placeholder implementation
        if (sprites.find(spriteIndex) == sprites.end()) {
            return; // Sprite index not found
        }
        Sprite& sprite = sprites[spriteIndex];
        // Load image file using stb_image
        int width, height, channels;
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 3);
        if (data) {
            // Update sprite texture info
            sprite.texture.width = width-1; // 0 means 1
            sprite.texture.height = height-1; // 0 means 1
            // Upload to VRAM at sprite.texture.offset
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int vramAddr = sprite.texture.offset + (y * width + x);
                    int r = data[(y * width + x) * 3 + 0];
                    int g = data[(y * width + x) * 3 + 1];
                    int b = data[(y * width + x) * 3 + 2];
                    uint8_t colorIndex = fromRGB(r, g, b);
                    vWrite8(vramAddr, colorIndex);     // R
                }
            }
            stbi_image_free(data);
        }
    }

    int getSpriteCount() const {
        return sprites.size();
    }

    int getVisibleSpriteCount() const {
        int count = 0;
        for (const auto& pair : sprites) {
            if (pair.second.attributes.visible) {
                count++;
            }
        }
        return count;
    }

private:
    std::map<int, Sprite> sprites;
    std::vector<unsigned char>& vram;
    CPT_Screen &screen;
};