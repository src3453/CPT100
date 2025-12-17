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

    if (fmod(value, 1) < ditherMatrix[y % 4][x % 4] / 16.0)
    {
    return ceil(value); // 白
    }
    else
    {
    return floor(value); // 黒
    }
}

uint8_t fromRGB(int r, int g, int b) {
    r = (int)(r%256/42.666666666666666);
    g = (int)(g%256/42.666666666666666);
    b = (int)(b%256/42.666666666666666);
    if (r+g+b == 15) {
        return uint8_t(255);
    } else {
        return (uint8_t)((r*36+g*6+b)%215);
    }
}

uint8_t fromRGBDithered(int x, int y, int r, int g, int b) {
    float _r = dither(x,y,(float)(r%256)/42.666666666666666);
    float _g = dither(x,y,(float)(g%256)/42.666666666666666);
    float _b = dither(x,y,(float)(b%256)/42.666666666666666);
    if (r+g+b == 15) {
        return uint8_t(255);
    } else {
        return (uint8_t)((r*36+g*6+b)%215);
    }
}