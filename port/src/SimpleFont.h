#pragma once

#include "GraphCompat.h"

#include <string>

class SimpleFont {
public:
    void drawText(GraphCompat &g, unsigned char *dest, int x, int y, const std::string &text, unsigned char color) const;

private:
    void drawChar(GraphCompat &g, unsigned char *dest, int x, int y, char ch, unsigned char color) const;
};
