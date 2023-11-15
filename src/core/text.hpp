#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <map>


class Font {
public:
    Font(Screen &screen) : screen(screen) {
        loadFontData();
    }

    void drawChar(char ch, int x, int y, int color) {
        int ind = (int)ch;
        if (fontData.find(ind) == fontData.end()) {
            ind = 96; // Default to a certain character if not found
        }

        const std::vector<int> &charData = fontData[ind];
        for (int i = 0; i < 12; ++i) {
            for (int j = 0; j < 8; ++j) {
                int dataIndex = i * 8 + j;
                char data = '0';
                if (dataIndex < charData.size()) {
                    data = charData[dataIndex] + '0';
                }
                if (data == '1') {
                    screen.pix(x + j, y + i, color);
                }
            }
        }
    }

    void print(const std::string &text, int x = 0, int y = 0, int color = 255) {
        for (size_t i = 0; i < text.length(); ++i) {
            drawChar(text[i], (x + 8 * i) % SCREEN_WIDTH, y + (i / 48) * 12, color);
        }
    }

private:
    Screen &screen;
    std::map<int, std::vector<int>> fontData;

    void loadFontData() {
        // Load font data from a file or any data source and store it in fontData
        // Example: fontData[characterCode] = vector of pixel data
        // Note: This is a placeholder, you need to implement this part based on your data source
    }
};
