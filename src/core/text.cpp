#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <map>
#include <algorithm>

#include "res/font8x12.hpp"

class Font {
public:
    Font(CPT_Screen &screen) : screen(screen) {
        loadFontData();
    }

    void drawChar(char ch, int x, int y, int color) {
        std::vector<char>::iterator itr;
        int ind = 0;
        itr = std::find(fontChars.begin(), fontChars.end(), ch);
        if (itr == fontChars.end()) {
            ind = 0; // Default to a certain character if not found
        }
        ind = std::distance(fontChars.begin(), itr);

        const string &charData = fontData[ind];
        for (int i = 0; i < 12; ++i) {
            for (int j = 0; j < 8; ++j) {
                int dataIndex = i * 8 + j;
                char data = '0';
                if (dataIndex < charData.size()) {
                    data = (char)charData[dataIndex];
                }
                if (data == '1') {
                    screen.pix(x + j, y + i, color);
                }
            }
        }
    }

    void print(const std::string &text, int x = 0, int y = 0, int color = 255) {
        for (size_t i = 0; i < text.length(); ++i) {
            drawChar((char)text[i], (x + 8 * i) % CPT_SCREEN_WIDTH, y + (i / 48) * 12, color);
        }
    }

private:
    CPT_Screen &screen;
    std::vector<string> fontData;
    std::vector<char> fontChars;

    void loadFontData() {
        // Load font data from a file or any data source and store it in fontData
        // Example: fontData[characterCode] = vector of pixel data
        // Note: This is a placeholder, you need to implement this part based on your data source
        fontData = FONT8X12_DATA;
        fontChars = FONT8X12_CHARS;
    }
};
