#include <vector>
#include <map>
#include <math.h>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
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
#define vRead8(addr) (vram_peek(vram, addr))
#define vRead16(addr) (vRead8(addr) | (vRead8(addr + 1) << 8))
#define vRead24(addr) (vRead8(addr) | (vRead8(addr + 1) << 8) | (vRead8(addr + 2) << 16))
#define vWrite8(addr, val) (vram_poke(vram, addr, val))
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
    uint8_t transparentColorIndex; // Color index treated as transparent
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

    void spr(int spriteIndex, bool enabled=true, int x=0, int y=0, float rotation = 0.0f) {
        vWrite16(0x1D000 + spriteIndex * 12 + 0, x);
        vWrite16(0x1D000 + spriteIndex * 12 + 2, y);
        uint8_t attr = vRead8(0x1D000 + spriteIndex * 12 + 9);
        if (enabled) {
            attr |= 0x01; // Set visible bit
        } else {
            attr &= ~0x01; // Clear visible bit
        }
        vWrite8(0x1D000 + spriteIndex * 12 + 9, attr);
        uint8_t rotValue = static_cast<uint8_t>(rotation / 1.40625f) & 0xFF;
        vWrite8(0x1D000 + spriteIndex * 12 + 11, rotValue);
        //printf("Sprite %d set to (%d,%d), enabled=%d, rotation=%.2f (raw=%d)\n", spriteIndex, x, y, enabled?1:0, rotation, rotValue);
    }

    void updateSprites() {
        // Load sprite data from VRAM and update internal sprite list
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
            /*printf("Sprite %d: pos=(%d,%d), tex=(%d,%d)@0x%06X, vis=%d, flipX=%d, flipY=%d, prio=%d, rot=%d\n",
                   i, sprite.x, sprite.y,
                   sprite.texture.width + 1, sprite.texture.height + 1, sprite.texture.offset,
                   sprite.attributes.visible ? 1 : 0,
                   sprite.attributes.flipX ? 1 : 0,
                   sprite.attributes.flipY ? 1 : 0,
                   sprite.attributes.priority,
                   sprite.attributes.rotation);*/
        }
    }

    void renderSprites() {
        // Render Sprites based on VRAM data
        // Sort sprites by priority
        std::vector<std::pair<int, Sprite>> spriteList;
        for (int i = 0; i < 256; i++) {
            if (sprites[i].attributes.visible) {
                spriteList.push_back({i, sprites[i]});
            }
        }
        std::sort(spriteList.begin(), spriteList.end(), [](const auto& a, const auto& b) {
            return a.second.attributes.priority < b.second.attributes.priority;
        });

        // Render each sprite
        for (const auto& pair : spriteList) {
            const Sprite& sprite = pair.second;
            // Render sprite at (sprite.x, sprite.y) with texture from VRAM
            int texWidth = sprite.texture.width + 1; // 0 means 1
            int texHeight = sprite.texture.height + 1; // 0 means 1
            for (int ty = 0; ty < texHeight; ty++) {
                for (int tx = 0; tx < texWidth; tx++) {
                    int drawX = sprite.x + (sprite.attributes.flipX ? (texWidth - 1 - tx) : tx);
                    int drawY = sprite.y + (sprite.attributes.flipY ? (texHeight - 1 - ty) : ty);
                    int vramAddr = sprite.texture.offset + (ty * texWidth + tx);
                    uint8_t colorIndex = vRead8(vramAddr);
                    if (colorIndex != sprite.attributes.transparentColorIndex) { // Assuming 0 is transparent
                        screen.pix(drawX, drawY, colorIndex);
                    }
                }
            }
        }
    }

    void loadSpriteFromLocalImage(int spriteIndex, const std::string& filepath, int offset = 0x20000, bool Dithered = true, int transparentColorIndex = 0) {
        // Load image from local file and upload to VRAM at sprite texture offset
        // This is a placeholder implementation
        Sprite& sprite = sprites[spriteIndex];
        //printf("Loading sprite %d from file: %s\n", spriteIndex, filepath.c_str());
        // Load image file using stb_image
        int width, height, channels;
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 3);
        if (width > 256 || height > 256) {
            printf("Image too large for sprite (max 256x256): %s\n", filepath.c_str());
            if (data) stbi_image_free(data);
            return;
        }
        if (data) {
            // Update sprite texture info
            sprite.texture.width = width-1; // 0 means 1
            sprite.texture.height = height-1; // 0 means 1
            sprite.texture.offset = offset;
            sprite.attributes.transparentColorIndex = transparentColorIndex;
            // Upload to VRAM at sprite.texture.offset
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int vramAddr = sprite.texture.offset + (y * width + x);
                    int r = data[(y * width + x) * 3 + 0];
                    int g = data[(y * width + x) * 3 + 1];
                    int b = data[(y * width + x) * 3 + 2];
                    uint8_t colorIndex;
                    if (Dithered) {
                        colorIndex = fromRGBDithered(x, y, r, g, b);
                    } else {
                        colorIndex = fromRGB(r, g, b);
                    }
                    vWrite8(vramAddr, colorIndex);     // R
                }
            }
            vWrite24(0x1D000 + spriteIndex * 12 + 6, sprite.texture.offset);
            vWrite8(0x1D000 + spriteIndex * 12 + 4, sprite.texture.width);
            vWrite8(0x1D000 + spriteIndex * 12 + 5, sprite.texture.height);
            stbi_image_free(data);
            //printf("Loaded sprite %d from %s (%dx%d)\n", spriteIndex, filepath.c_str(), width, height);
        } else {
            printf("Failed to load image: %s\n", filepath.c_str());
        }
    }

    int getVisibleSpriteCount() const {
        int count = 0;
        for (int i = 0; i < 256; i++) {
            if (sprites[i].attributes.visible) {
                count++;
            }
        }
        return count;
    }

private:
    Sprite sprites[256];
    std::vector<unsigned char>& vram;
    CPT_Screen &screen;
};