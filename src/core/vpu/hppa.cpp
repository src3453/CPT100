// HPPA: Hardware Picture Processing Accelerator
// Dithering, Image processing, Image filtering

#include <cmath>
#include <cstdint>

int dither(int x, int y, float value)
{
    // ordered dithering helper function
    int ditherMatrix[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}};

    if (fmod(value, 1) > ditherMatrix[y &0b11][x &0b11] / 16.0)
    {
    return ceil(value); // 白
    }
    else
    {
    return floor(value); // 黒
    }
}

#define FACTOR (255.0/5.0)
#define FACTOR5 (255.0/31.0)
#define FACTOR6 (255.0/63.0)

uint16_t fromRGB(int r, int g, int b) {
    if (screenMode == 3) {
        // RGB565
        uint16_t r5 = r/FACTOR5;
        uint16_t g6 = g/FACTOR6;
        uint16_t b5 = b/FACTOR5;
        return (r5 << 11) | (g6 << 5) | b5;
    } else {
        r = (int)(r%256/FACTOR);
        g = (int)(g%256/FACTOR);
        b = (int)(b%256/FACTOR);
        if (r+g+b >= 15) {
            return uint8_t(255);
        } else {
            return (uint8_t)((r*36+g*6+b)%215);
        }
    }
}

#define clamp(x,xMin,xMax) if ((x)<(xMin)) (x)=(xMin); else if ((x)>(xMax)) (x)=(xMax); else (x)=(x);

uint16_t fromRGBDithered(int x, int y, int r, int g, int b) {
    if (screenMode == 3) {
        // RGB565
        uint16_t r5 = dither(x,y,(float)(r%256)/FACTOR5);
        uint16_t g6 = dither(x,y,(float)(g%256)/FACTOR6);
        uint16_t b5 = dither(x,y,(float)(b%256)/FACTOR5);
        clamp(r5,0,31);
        clamp(g6,0,63);
        clamp(b5,0,31);
        return (r5 << 11) | (g6 << 5) | b5;
    } else {
        int _r = dither(x,y,(float)(r%256)/FACTOR);
        int _g = dither(x,y,(float)(g%256)/FACTOR);
        int _b = dither(x,y,(float)(b%256)/FACTOR);
        if (_r+_g+_b >= 15) {
            return uint8_t(255);
        } else {
            return (uint8_t)((_r*36+_g*6+_b)%215);
        }
    }
}

#undef FACTOR
#undef FACTOR5
#undef FACTOR6