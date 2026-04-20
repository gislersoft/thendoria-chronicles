#pragma once

#include "GraphCompat.h"

#include <cstdint>
#include <string>
#include <vector>

class TileSetCompat {
public:
    TileSetCompat();

    bool loadFromPngSheet(const std::string &path, int tileSize);
    bool drawTile(GraphCompat &g, unsigned char *dest, int tileIndex, int x, int y, unsigned char transparentColor) const;

    int tileCount() const;
    int tileSize() const;

private:
    std::vector<std::vector<unsigned char>> tiles_;
    std::vector<std::vector<std::uint32_t>> rgbaTiles_;
    int tileSize_;
    bool hasTrueColor_;
};
