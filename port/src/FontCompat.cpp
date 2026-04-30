#include "FontCompat.h"

#include <cctype>
#include <cstdint>

namespace {

struct Glyph {
    char c;
    std::uint8_t rows[7];
};

const Glyph kGlyphs[] = {
    {'A', {14, 17, 17, 31, 17, 17, 17}},
    {'B', {30, 17, 17, 30, 17, 17, 30}},
    {'C', {14, 17, 16, 16, 16, 17, 14}},
    {'D', {30, 17, 17, 17, 17, 17, 30}},
    {'E', {31, 16, 16, 30, 16, 16, 31}},
    {'F', {31, 16, 16, 30, 16, 16, 16}},
    {'G', {14, 17, 16, 23, 17, 17, 14}},
    {'H', {17, 17, 17, 31, 17, 17, 17}},
    {'I', {31, 4, 4, 4, 4, 4, 31}},
    {'J', {1, 1, 1, 1, 17, 17, 14}},
    {'K', {17, 18, 20, 24, 20, 18, 17}},
    {'L', {16, 16, 16, 16, 16, 16, 31}},
    {'M', {17, 27, 21, 21, 17, 17, 17}},
    {'N', {17, 25, 21, 19, 17, 17, 17}},
    {'O', {14, 17, 17, 17, 17, 17, 14}},
    {'P', {30, 17, 17, 30, 16, 16, 16}},
    {'Q', {14, 17, 17, 17, 21, 18, 13}},
    {'R', {30, 17, 17, 30, 20, 18, 17}},
    {'S', {15, 16, 16, 14, 1, 1, 30}},
    {'T', {31, 4, 4, 4, 4, 4, 4}},
    {'U', {17, 17, 17, 17, 17, 17, 14}},
    {'V', {17, 17, 17, 17, 17, 10, 4}},
    {'W', {17, 17, 17, 21, 21, 21, 10}},
    {'X', {17, 17, 10, 4, 10, 17, 17}},
    {'Y', {17, 17, 10, 4, 4, 4, 4}},
    {'Z', {31, 1, 2, 4, 8, 16, 31}},
    {'0', {14, 17, 19, 21, 25, 17, 14}},
    {'1', {4, 12, 4, 4, 4, 4, 14}},
    {'2', {14, 17, 1, 2, 4, 8, 31}},
    {'3', {30, 1, 1, 14, 1, 1, 30}},
    {'4', {2, 6, 10, 18, 31, 2, 2}},
    {'5', {31, 16, 16, 30, 1, 1, 30}},
    {'6', {14, 16, 16, 30, 17, 17, 14}},
    {'7', {31, 1, 2, 4, 8, 8, 8}},
    {'8', {14, 17, 17, 14, 17, 17, 14}},
    {'9', {14, 17, 17, 15, 1, 1, 14}},
    {'.', {0, 0, 0, 0, 0, 12, 12}},
    {',', {0, 0, 0, 0, 0, 12, 8}},
    {':', {0, 12, 12, 0, 12, 12, 0}},
    {';', {0, 12, 12, 0, 12, 8, 0}},
    {'!', {4, 4, 4, 4, 4, 0, 4}},
    {'?', {14, 17, 1, 2, 4, 0, 4}},
    {'-', {0, 0, 0, 31, 0, 0, 0}},
    {'_', {0, 0, 0, 0, 0, 0, 31}},
    {'/', {1, 2, 4, 8, 16, 0, 0}},
    {'(', {2, 4, 8, 8, 8, 4, 2}},
    {')', {8, 4, 2, 2, 2, 4, 8}},
    {'\'', {4, 4, 2, 0, 0, 0, 0}},
    {'"', {10, 10, 0, 0, 0, 0, 0}},
    {'+', {0, 4, 4, 31, 4, 4, 0}},
    {'*', {0, 10, 4, 31, 4, 10, 0}},
    {'=', {0, 31, 0, 31, 0, 0, 0}},
    {' ', {0, 0, 0, 0, 0, 0, 0}},
};

const std::uint8_t *glyphRows(char ch) {
    for (const auto &g : kGlyphs) {
        if (g.c == ch) {
            return g.rows;
        }
    }
    static const std::uint8_t fallback[7] = {14, 17, 2, 4, 4, 0, 4};
    return fallback;
}

unsigned char textureColorByRow(int textureId, int row) {
    static const unsigned char tex1[5] = {82, 83, 84, 83, 82};
    static const unsigned char tex2[5] = {22, 190, 127, 190, 22};
    static const unsigned char tex3[5] = {186, 141, 140, 235, 186};

    const int idx = std::max(0, std::min(4, (row * 5) / 7));
    switch (textureId) {
        case 1:
            return tex1[idx];
        case 2:
            return tex2[idx];
        case 3:
            return tex3[idx];
        default:
            return 15;
    }
}

std::uint32_t decodeUtf8(const std::string &s, std::size_t &i) {
    const unsigned char c = static_cast<unsigned char>(s[i]);
    if ((c & 0x80) == 0) {
        ++i;
        return c;
    }
    if ((c & 0xE0) == 0xC0 && i + 1 < s.size()) {
        const std::uint32_t cp = ((c & 0x1Fu) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3Fu);
        i += 2;
        return cp;
    }
    if ((c & 0xF0) == 0xE0 && i + 2 < s.size()) {
        const std::uint32_t cp = ((c & 0x0Fu) << 12) |
                                 ((static_cast<unsigned char>(s[i + 1]) & 0x3Fu) << 6) |
                                 (static_cast<unsigned char>(s[i + 2]) & 0x3Fu);
        i += 3;
        return cp;
    }
    ++i;
    return '?';
}

char mapCodepointToAscii(std::uint32_t cp) {
    switch (cp) {
        case 0x00E1: case 0x00C1: case 0x00E0: case 0x00C0: case 0x00E2: case 0x00C2: case 0x00E4: case 0x00C4:
            return 'A';
        case 0x00E9: case 0x00C9: case 0x00E8: case 0x00C8: case 0x00EA: case 0x00CA: case 0x00EB: case 0x00CB:
            return 'E';
        case 0x00ED: case 0x00CD: case 0x00EC: case 0x00CC: case 0x00EE: case 0x00CE: case 0x00EF: case 0x00CF:
            return 'I';
        case 0x00F3: case 0x00D3: case 0x00F2: case 0x00D2: case 0x00F4: case 0x00D4: case 0x00F6: case 0x00D6:
            return 'O';
        case 0x00FA: case 0x00DA: case 0x00F9: case 0x00D9: case 0x00FB: case 0x00DB: case 0x00FC: case 0x00DC:
            return 'U';
        case 0x00F1: case 0x00D1:
            return 'N';
        case 0x00BF:
            return '?';
        case 0x00A1:
            return '!';
        default:
            break;
    }

    if (cp < 128u) {
        return static_cast<char>(cp);
    }
    return '?';
}

} // namespace

std::string FontCompat::normalizeText(const std::string &text) const {
    std::string out;
    out.reserve(text.size());

    for (std::size_t i = 0; i < text.size();) {
        const std::uint32_t cp = decodeUtf8(text, i);
        char ch = mapCodepointToAscii(cp);
        if (ch == '*') {
            ch = ' ';
        }
        const unsigned char uch = static_cast<unsigned char>(ch);
        if (uch < 128) {
            ch = static_cast<char>(std::toupper(uch));
        }
        out.push_back(ch);
    }

    return out;
}

void FontCompat::drawChar(unsigned char *dest, int x, int y, char ch, GraphCompat &g, unsigned char bg, unsigned char fg) const {
    if (bg != 255) {
        g.fillbox(dest, x, y, x + 5, y + 7, bg);
    }

    const std::uint8_t *rows = glyphRows(ch);
    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 5; ++col) {
            if (rows[row] & (1u << (4 - col))) {
                g.putpixel(dest, x + col, y + row, fg);
            }
        }
    }
}

void drawCharTexturedCompat(unsigned char *dest, int x, int y, char ch, GraphCompat &g, unsigned char bg, int textureId) {
    if (bg != 255) {
        g.fillbox(dest, x, y, x + 5, y + 7, bg);
    }

    const std::uint8_t *rows = glyphRows(ch);
    for (int row = 0; row < 7; ++row) {
        const unsigned char fg = textureColorByRow(textureId, row);
        for (int col = 0; col < 5; ++col) {
            if (rows[row] & (1u << (4 - col))) {
                g.putpixel(dest, x + col, y + row, fg);
            }
        }
    }
}

void FontCompat::putstrWithState(unsigned char *dest, int x, int y, const std::string &text, GraphCompat &g, unsigned char bg, unsigned char fg,
                                 bool &plusMode, bool &minusMode) const {
    const std::string norm = normalizeText(text);

    int cx = x;
    int cy = y;
    for (char ch : norm) {
        if (ch == '\n') {
            cx = x;
            cy += 9;
            continue;
        }

        // Legacy dialog markers: +...+ and -...- change text color and hide delimiters.
        if (ch == '+') {
            plusMode = !plusMode;
            cx += 6;
            continue;
        }
        if (ch == '-') {
            minusMode = !minusMode;
            cx += 6;
            continue;
        }

        unsigned char currentFg = fg;
        if (plusMode) {
            currentFg = 69;
        } else if (minusMode) {
            currentFg = 32;
        }

        drawChar(dest, cx, cy, ch, g, bg, currentFg);
        cx += 6;
    }
}

void FontCompat::putstr(unsigned char *dest, int x, int y, const std::string &text, GraphCompat &g, unsigned char bg, unsigned char fg) const {
    bool plusMode = false;
    bool minusMode = false;
    putstrWithState(dest, x, y, text, g, bg, fg, plusMode, minusMode);
}

void FontCompat::putstrScaled(unsigned char *dest, int x, int y, const std::string &text, GraphCompat &g, unsigned char bg, unsigned char fg, int scale) const {
    if (scale <= 1) { putstr(dest, x, y, text, g, bg, fg); return; }
    const std::string norm = normalizeText(text);
    int cx = x;
    for (char ch : norm) {
        const std::uint8_t *rows = glyphRows(ch);
        // Optional background block for the whole character cell
        if (bg != 255) {
            g.fillbox(dest, cx, y, cx + 5 * scale - 1, y + 7 * scale - 1, bg);
        }
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (rows[row] & (1u << (4 - col))) {
                    g.fillbox(dest,
                              cx + col * scale,         y + row * scale,
                              cx + col * scale + scale - 1, y + row * scale + scale - 1,
                              fg);
                }
            }
        }
        cx += 6 * scale;
    }
}

void FontCompat::putstrTexturedWithState(unsigned char *dest, int x, int y, const std::string &text, GraphCompat &g, unsigned char bg, int textureId,
                                         bool &plusMode, bool &minusMode) const {
    const std::string norm = normalizeText(text);

    int cx = x;
    int cy = y;
    for (char ch : norm) {
        if (ch == '\n') {
            cx = x;
            cy += 9;
            continue;
        }

        // Legacy markers are invisible but keep one character of spacing.
        if (ch == '+') {
            plusMode = !plusMode;
            cx += 6;
            continue;
        }
        if (ch == '-') {
            minusMode = !minusMode;
            cx += 6;
            continue;
        }

        if (plusMode) {
            drawChar(dest, cx, cy, ch, g, bg, 69);
        } else if (minusMode) {
            drawChar(dest, cx, cy, ch, g, bg, 32);
        } else {
            drawCharTexturedCompat(dest, cx, cy, ch, g, bg, textureId);
        }
        cx += 6;
    }
}

void FontCompat::putstrTextured(unsigned char *dest, int x, int y, const std::string &text, GraphCompat &g, unsigned char bg, int textureId) const {
    bool plusMode = false;
    bool minusMode = false;
    putstrTexturedWithState(dest, x, y, text, g, bg, textureId, plusMode, minusMode);
}

int FontCompat::textWidth(const std::string &text) const {
    int w = 0;
    int cur = 0;
    for (char ch : text) {
        if (ch == '\n') {
            if (cur > w) {
                w = cur;
            }
            cur = 0;
        } else {
            cur += 6;
        }
    }
    if (cur > w) {
        w = cur;
    }
    return w;
}
