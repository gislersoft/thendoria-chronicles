#include "SimpleFont.h"

#include <cctype>
#include <cstdint>

namespace {

void glyphFor(char ch, std::uint8_t out[7]) {
    for (int i = 0; i < 7; ++i) {
        out[i] = 0;
    }

    switch (ch) {
        case 'A': { std::uint8_t t[7] = {14, 17, 17, 31, 17, 17, 17}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'B': { std::uint8_t t[7] = {30, 17, 17, 30, 17, 17, 30}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'C': { std::uint8_t t[7] = {14, 17, 16, 16, 16, 17, 14}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'D': { std::uint8_t t[7] = {30, 17, 17, 17, 17, 17, 30}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'E': { std::uint8_t t[7] = {31, 16, 16, 30, 16, 16, 31}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'F': { std::uint8_t t[7] = {31, 16, 16, 30, 16, 16, 16}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'G': { std::uint8_t t[7] = {14, 17, 16, 23, 17, 17, 14}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'H': { std::uint8_t t[7] = {17, 17, 17, 31, 17, 17, 17}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'I': { std::uint8_t t[7] = {31, 4, 4, 4, 4, 4, 31}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'J': { std::uint8_t t[7] = {1, 1, 1, 1, 17, 17, 14}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'K': { std::uint8_t t[7] = {17, 18, 20, 24, 20, 18, 17}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'L': { std::uint8_t t[7] = {16, 16, 16, 16, 16, 16, 31}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'M': { std::uint8_t t[7] = {17, 27, 21, 21, 17, 17, 17}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'N': { std::uint8_t t[7] = {17, 25, 21, 19, 17, 17, 17}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'O': { std::uint8_t t[7] = {14, 17, 17, 17, 17, 17, 14}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'P': { std::uint8_t t[7] = {30, 17, 17, 30, 16, 16, 16}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'Q': { std::uint8_t t[7] = {14, 17, 17, 17, 21, 18, 13}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'R': { std::uint8_t t[7] = {30, 17, 17, 30, 20, 18, 17}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'S': { std::uint8_t t[7] = {15, 16, 16, 14, 1, 1, 30}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'T': { std::uint8_t t[7] = {31, 4, 4, 4, 4, 4, 4}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'U': { std::uint8_t t[7] = {17, 17, 17, 17, 17, 17, 14}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'V': { std::uint8_t t[7] = {17, 17, 17, 17, 17, 10, 4}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'W': { std::uint8_t t[7] = {17, 17, 17, 21, 21, 21, 10}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'X': { std::uint8_t t[7] = {17, 17, 10, 4, 10, 17, 17}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'Y': { std::uint8_t t[7] = {17, 17, 10, 4, 4, 4, 4}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case 'Z': { std::uint8_t t[7] = {31, 1, 2, 4, 8, 16, 31}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '0': { std::uint8_t t[7] = {14, 17, 19, 21, 25, 17, 14}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '1': { std::uint8_t t[7] = {4, 12, 4, 4, 4, 4, 14}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '2': { std::uint8_t t[7] = {14, 17, 1, 2, 4, 8, 31}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '3': { std::uint8_t t[7] = {30, 1, 1, 14, 1, 1, 30}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '4': { std::uint8_t t[7] = {2, 6, 10, 18, 31, 2, 2}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '5': { std::uint8_t t[7] = {31, 16, 16, 30, 1, 1, 30}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '6': { std::uint8_t t[7] = {14, 16, 16, 30, 17, 17, 14}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '7': { std::uint8_t t[7] = {31, 1, 2, 4, 8, 8, 8}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '8': { std::uint8_t t[7] = {14, 17, 17, 14, 17, 17, 14}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '9': { std::uint8_t t[7] = {14, 17, 17, 15, 1, 1, 14}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '.': { std::uint8_t t[7] = {0, 0, 0, 0, 0, 12, 12}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case ',': { std::uint8_t t[7] = {0, 0, 0, 0, 0, 12, 8}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case ':': { std::uint8_t t[7] = {0, 12, 12, 0, 12, 12, 0}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case ';': { std::uint8_t t[7] = {0, 12, 12, 0, 12, 8, 0}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '!': { std::uint8_t t[7] = {4, 4, 4, 4, 4, 0, 4}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '?': { std::uint8_t t[7] = {14, 17, 1, 2, 4, 0, 4}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '-': { std::uint8_t t[7] = {0, 0, 0, 31, 0, 0, 0}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '_': { std::uint8_t t[7] = {0, 0, 0, 0, 0, 0, 31}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '/': { std::uint8_t t[7] = {1, 2, 4, 8, 16, 0, 0}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case '(': { std::uint8_t t[7] = {2, 4, 8, 8, 8, 4, 2}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case ')': { std::uint8_t t[7] = {8, 4, 2, 2, 2, 4, 8}; for (int i = 0; i < 7; ++i) out[i] = t[i]; } break;
        case ' ': default:
            break;
    }
}

} // namespace

void SimpleFont::drawChar(GraphCompat &g, unsigned char *dest, int x, int y, char ch, unsigned char color) const {
    if (ch == '*') {
        ch = ' ';
    }

    unsigned char uch = static_cast<unsigned char>(ch);
    if (uch < 128) {
        ch = static_cast<char>(std::toupper(uch));
    } else {
        ch = '?';
    }

    std::uint8_t bits[7];
    glyphFor(ch, bits);
    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 5; ++col) {
            if (bits[row] & (1u << (4 - col))) {
                g.putpixel(dest, x + col, y + row, color);
            }
        }
    }
}

void SimpleFont::drawText(GraphCompat &g, unsigned char *dest, int x, int y, const std::string &text, unsigned char color) const {
    int cx = x;
    int cy = y;
    for (char ch : text) {
        if (ch == '\n') {
            cx = x;
            cy += 9;
            continue;
        }
        drawChar(g, dest, cx, cy, ch, color);
        cx += 6;
    }
}
