#include "TileSetCompat.h"

#include <SDL_image.h>

TileSetCompat::TileSetCompat() : tileSize_(16), hasTrueColor_(false) {}

bool TileSetCompat::loadFromPngSheet(const std::string &path, int tileSize) {
    tileSize_ = tileSize;
    tiles_.clear();
    rgbaTiles_.clear();
    hasTrueColor_ = false;

    SDL_Surface *loaded = IMG_Load(path.c_str());
    if (!loaded) {
        return false;
    }

    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loaded);
    if (!rgba) {
        return false;
    }

    const int cols = rgba->w / tileSize_;
    const int rows = rgba->h / tileSize_;

    for (int ry = 0; ry < rows; ++ry) {
        for (int rx = 0; rx < cols; ++rx) {
            std::vector<unsigned char> tile(static_cast<std::size_t>(tileSize_ * tileSize_), 0);
            std::vector<std::uint32_t> rgbaTile(static_cast<std::size_t>(tileSize_ * tileSize_), 0);
            for (int y = 0; y < tileSize_; ++y) {
                const unsigned char *row = static_cast<const unsigned char *>(rgba->pixels) + (ry * tileSize_ + y) * rgba->pitch;
                for (int x = 0; x < tileSize_; ++x) {
                    const unsigned char *p = row + (rx * tileSize_ + x) * 4;
                    const std::size_t idx = static_cast<std::size_t>(x + y * tileSize_);
                    tile[idx] = (p[3] < 16) ? 0 : 15;
                    rgbaTile[idx] = (static_cast<std::uint32_t>(p[3]) << 24) |
                                    (static_cast<std::uint32_t>(p[0]) << 16) |
                                    (static_cast<std::uint32_t>(p[1]) << 8) |
                                    static_cast<std::uint32_t>(p[2]);
                }
            }
            tiles_.push_back(tile);
            rgbaTiles_.push_back(rgbaTile);
        }
    }

    SDL_FreeSurface(rgba);
    hasTrueColor_ = !rgbaTiles_.empty();
    return !tiles_.empty();
}

bool TileSetCompat::drawTile(GraphCompat &g, unsigned char *dest, int tileIndex, int x, int y, unsigned char transparentColor) const {
    if (tileIndex < 0 || tileIndex >= static_cast<int>(tiles_.size())) {
        return false;
    }
    const std::size_t idx = static_cast<std::size_t>(tileIndex);
    if (hasTrueColor_ && idx < rgbaTiles_.size() && !rgbaTiles_[idx].empty()) {
        g.drawRgbaFrame(x, y, tileSize_, tileSize_, rgbaTiles_[idx].data());
    } else {
        g.putframe(dest, x, y, tileSize_, tiles_[idx].data(), transparentColor);
    }
    return true;
}

int TileSetCompat::tileCount() const {
    return static_cast<int>(tiles_.size());
}

int TileSetCompat::tileSize() const {
    return tileSize_;
}
