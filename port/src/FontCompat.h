#pragma once

#include "GraphCompat.h"

#include <string>

class FontCompat {
public:
    // Similar shape to original putstr usage: background + foreground colors.
    void putstr(unsigned char *dest, int x, int y, const std::string &text, GraphCompat &g, unsigned char bg, unsigned char fg) const;
    void putstrTextured(unsigned char *dest, int x, int y, const std::string &text, GraphCompat &g, unsigned char bg, int textureId) const;
    void putstrWithState(unsigned char *dest, int x, int y, const std::string &text, GraphCompat &g, unsigned char bg, unsigned char fg,
                         bool &plusMode, bool &minusMode) const;
    void putstrTexturedWithState(unsigned char *dest, int x, int y, const std::string &text, GraphCompat &g, unsigned char bg, int textureId,
                                 bool &plusMode, bool &minusMode) const;
    int textWidth(const std::string &text) const;

private:
    void drawChar(unsigned char *dest, int x, int y, char ch, GraphCompat &g, unsigned char bg, unsigned char fg) const;
    std::string normalizeText(const std::string &text) const;
};
