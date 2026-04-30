#include "Renderer320x200.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {

std::uint32_t blendArgb(std::uint32_t dst, std::uint32_t src) {
    const std::uint32_t srcA = (src >> 24) & 0xFFu;
    if (srcA == 0) {
        return dst;
    }
    if (srcA == 255) {
        return src;
    }

    const std::uint32_t dstR = (dst >> 16) & 0xFFu;
    const std::uint32_t dstG = (dst >> 8) & 0xFFu;
    const std::uint32_t dstB = dst & 0xFFu;
    const std::uint32_t srcR = (src >> 16) & 0xFFu;
    const std::uint32_t srcG = (src >> 8) & 0xFFu;
    const std::uint32_t srcB = src & 0xFFu;

    const std::uint32_t invA = 255u - srcA;
    const std::uint32_t outR = (srcR * srcA + dstR * invA) / 255u;
    const std::uint32_t outG = (srcG * srcA + dstG * invA) / 255u;
    const std::uint32_t outB = (srcB * srcA + dstB * invA) / 255u;
    return 0xFF000000u | (outR << 16) | (outG << 8) | outB;
}

} // namespace

// Map an ARGB color to a Game Boy green shade.
// Ramp: #0F380F (authentic DMG darkest) -> #9BBC0F (full bright).
// Grid is #306230 (neutral mid) — dark pixels contrast below, bright above.
// Smoothstep S-curve: deep shadows stay dark, mid-tones are well separated.
static std::uint32_t toGBGreen(std::uint32_t argb) {
    const std::uint32_t r = (argb >> 16) & 0xFFu;
    const std::uint32_t g = (argb >>  8) & 0xFFu;
    const std::uint32_t b =  argb        & 0xFFu;
    // BT.601 perceived luminance, 0-255.
    const std::uint32_t L = (77u * r + 150u * g + 29u * b) >> 8;
    // Smoothstep S-curve: t = 3x^2 - 2x^3
    const float x = L / 255.0f;
    const float t = x * x * (3.0f - 2.0f * x);
    const std::uint32_t ti = static_cast<std::uint32_t>(t * 255.0f + 0.5f);
    // Interpolate from #0F380F to #9BBC0F.
    const std::uint32_t gr = 0x0Fu + ((0x9Bu - 0x0Fu) * ti) / 255u;
    const std::uint32_t gg = 0x38u + ((0xBCu - 0x38u) * ti) / 255u;
    const std::uint32_t gb = 0x0Fu;
    return 0xFF000000u | (gr << 16) | (gg << 8) | gb;
}

Renderer320x200::Renderer320x200(SDL_Renderer *renderer, bool gameboyFilter, bool gridFilter)
    : renderer_(renderer), gridTexture_(nullptr), plainTexture_(nullptr),
      rgbaGrid_(kScaledPixels, 0u),
      gameboyFilter_(gameboyFilter), gridFilter_(gridFilter) {
    if (gameboyFilter_ || gridFilter_) {
        // Full scaled texture for LCD grid effect.
        gridTexture_ = SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            kScaledWidth,
            kScaledHeight
        );
        if (!gridTexture_) {
            std::cerr << "SDL_CreateTexture (grid) failed: " << SDL_GetError() << '\n';
        }
        SDL_SetTextureScaleMode(gridTexture_, SDL_ScaleModeNearest);
    } else {
        // Plain 320×200 texture; SDL logical size handles the upscale.
        plainTexture_ = SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            kWidth,
            kHeight
        );
        if (!plainTexture_) {
            std::cerr << "SDL_CreateTexture (plain) failed: " << SDL_GetError() << '\n';
        }
        SDL_RenderSetLogicalSize(renderer_, kWidth, kHeight);
    }

    initDefaultPalette();
}

Renderer320x200::~Renderer320x200() {
    if (gridTexture_) {
        SDL_DestroyTexture(gridTexture_);
        gridTexture_ = nullptr;
    }
    if (plainTexture_) {
        SDL_DestroyTexture(plainTexture_);
        plainTexture_ = nullptr;
    }
}

unsigned char *Renderer320x200::vga() {
    return vga_.data();
}

unsigned char *Renderer320x200::pv1() {
    return pv1_.data();
}

unsigned char *Renderer320x200::pv2() {
    return pv2_.data();
}

void Renderer320x200::clr(unsigned char *px, unsigned char color) {
    if (!px) {
        return;
    }
    std::memset(px, color, kPixels);
}

void Renderer320x200::volcar(unsigned char *dest, const unsigned char *src) {
    if (!dest || !src) {
        return;
    }
    std::memcpy(dest, src, kPixels);
}

void Renderer320x200::putpixel(unsigned char *px, int x, int y, unsigned char color) {
    if (!px) {
        return;
    }
    if (x < 0 || x >= kWidth || y < 0 || y >= kHeight) {
        return;
    }
    px[x + y * kWidth] = color;
}

unsigned char Renderer320x200::getpixel(const unsigned char *px, int x, int y) const {
    if (!px) {
        return 0;
    }
    if (x < 0 || x >= kWidth || y < 0 || y >= kHeight) {
        return 0;
    }
    return px[x + y * kWidth];
}

void Renderer320x200::getframe(const unsigned char *px, int x, int y, int a, unsigned char *img) const {
    if (!px || !img || a <= 0) {
        return;
    }
    for (int j = 0; j < a; ++j) {
        for (int i = 0; i < a; ++i) {
            img[i + (j * a)] = getpixel(px, x + i, y + j);
        }
    }
}

void Renderer320x200::putframe(unsigned char *px, int x, int y, int a, const unsigned char *img, unsigned char transparentColor) {
    if (!px || !img || a <= 0) {
        return;
    }
    for (int j = 0; j < a; ++j) {
        for (int i = 0; i < a; ++i) {
            unsigned char c = img[i + (j * a)];
            if (c != transparentColor) {
                putpixel(px, x + i, y + j, c);
            }
        }
    }
}

void Renderer320x200::hline(unsigned char *px, int x1, int x2, int y, unsigned char color) {
    if (!px) {
        return;
    }
    if (y < 0 || y >= kHeight) {
        return;
    }
    if (x1 > x2) {
        std::swap(x1, x2);
    }
    x1 = std::max(0, x1);
    x2 = std::min(kWidth - 1, x2);
    for (int x = x1; x <= x2; ++x) {
        putpixel(px, x, y, color);
    }
}

void Renderer320x200::vline(unsigned char *px, int x, int y1, int y2, unsigned char color) {
    if (!px) {
        return;
    }
    if (x < 0 || x >= kWidth) {
        return;
    }
    if (y1 > y2) {
        std::swap(y1, y2);
    }
    y1 = std::max(0, y1);
    y2 = std::min(kHeight - 1, y2);
    for (int y = y1; y <= y2; ++y) {
        putpixel(px, x, y, color);
    }
}

void Renderer320x200::fillbox(unsigned char *px, int x1, int y1, int x2, int y2, unsigned char color) {
    if (!px) {
        return;
    }
    if (x1 > x2) {
        std::swap(x1, x2);
    }
    if (y1 > y2) {
        std::swap(y1, y2);
    }
    x1 = std::max(0, x1);
    y1 = std::max(0, y1);
    x2 = std::min(kWidth - 1, x2);
    y2 = std::min(kHeight - 1, y2);
    for (int y = y1; y <= y2; ++y) {
        hline(px, x1, x2, y, color);
    }
}

void Renderer320x200::box(unsigned char *px, int x1, int y1, int x2, int y2, unsigned char color) {
    hline(px, x1, x2, y1, color);
    hline(px, x1, x2, y2, color);
    vline(px, x1, y1, y2, color);
    vline(px, x2, y1, y2, color);
}

void Renderer320x200::line(unsigned char *px, int x1, int y1, int x2, int y2, unsigned char color) {
    if (!px) {
        return;
    }

    int dx = std::abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -std::abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        putpixel(px, x1, y1, color);
        if (x1 == x2 && y1 == y2) {
            break;
        }
        int e2 = err << 1;
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void Renderer320x200::wait_retrace() {
    // Modern equivalent is handled by VSYNC in the renderer.
}

void Renderer320x200::setPaletteEntry(unsigned char index, unsigned char r, unsigned char g, unsigned char b) {
    palette_[index] = 0xFF000000u | (static_cast<std::uint32_t>(r) << 16) |
                      (static_cast<std::uint32_t>(g) << 8) | static_cast<std::uint32_t>(b);
}

void Renderer320x200::clearOverlay() {
    overlay_.fill(0);
}

std::uint32_t *Renderer320x200::getOverlay() {
    return overlay_.data();
}

void Renderer320x200::drawRgbaFrame(int x, int y, int w, int h, const std::uint32_t *img) {
    if (!img || w <= 0 || h <= 0) {
        return;
    }

    for (int srcY = 0; srcY < h; ++srcY) {
        const int dstY = y + srcY;
        if (dstY < 0 || dstY >= kHeight) {
            continue;
        }
        for (int srcX = 0; srcX < w; ++srcX) {
            const int dstX = x + srcX;
            if (dstX < 0 || dstX >= kWidth) {
                continue;
            }
            const std::uint32_t src = img[srcX + srcY * w];
            if (((src >> 24) & 0xFFu) == 0) {
                continue;
            }
            overlay_[dstX + dstY * kWidth] = src;
        }
    }
}

void Renderer320x200::present(const unsigned char *px) {
    present(px, nullptr);
}

void Renderer320x200::present(const unsigned char *px, const unsigned char *top) {
    if (!px) {
        return;
    }

    // Step 1: Composite palette + overlay + top layer into rgbaScratch_ (320×200).
    for (int i = 0; i < kPixels; ++i) {
        rgbaScratch_[i] = palette_[px[i]];
        if ((overlay_[i] >> 24) != 0) {
            rgbaScratch_[i] = blendArgb(rgbaScratch_[i], overlay_[i]);
        }
        if (top && top[i] != 0) {
            rgbaScratch_[i] = palette_[top[i]];
        }
        if (gameboyFilter_) {
            rgbaScratch_[i] = toGBGreen(rgbaScratch_[i]);
        }
        // gridFilter_ keeps the original colour — no mapping needed
    }

    if (gameboyFilter_ || gridFilter_) {
        // Step 2: Upscale into rgbaGrid_ (960×600) with LCD grid border.
        // gameboyFilter_ uses DMG green as the grid colour; gridFilter_ uses pure black.
        const std::uint32_t gridLineColor = gridFilter_ ? 0xFF000000u : kGridColor;
        for (int gy = 0; gy < kHeight; ++gy) {
            for (int gx = 0; gx < kWidth; ++gx) {
                const std::uint32_t boosted = rgbaScratch_[gx + gy * kWidth];
                const int baseX = gx * kScale;
                const int baseY = gy * kScale;
                for (int sy = 0; sy < kScale; ++sy) {
                    const bool yGrid = (sy == kScale - 1);
                    const int rowOff = (baseY + sy) * kScaledWidth;
                    for (int sx = 0; sx < kScale; ++sx) {
                        const bool isGrid = yGrid || (sx == kScale - 1);
                        rgbaGrid_[baseX + sx + rowOff] = isGrid ? gridLineColor : boosted;
                    }
                }
            }
        }
        SDL_UpdateTexture(gridTexture_, nullptr, rgbaGrid_.data(), kScaledWidth * static_cast<int>(sizeof(std::uint32_t)));
        SDL_RenderClear(renderer_);
        SDL_Rect dst{0, 0, kScaledWidth, kScaledHeight};
        SDL_RenderCopy(renderer_, gridTexture_, nullptr, &dst);
    } else {
        // Plain path: upload 320×200 directly, logical size handles upscale.
        SDL_UpdateTexture(plainTexture_, nullptr, rgbaScratch_.data(), kWidth * static_cast<int>(sizeof(std::uint32_t)));
        SDL_RenderClear(renderer_);
        SDL_RenderCopy(renderer_, plainTexture_, nullptr, nullptr);
    }

    SDL_RenderPresent(renderer_);
}

void Renderer320x200::initDefaultPalette() {
    clearOverlay();

    for (int i = 0; i < 256; ++i) {
        unsigned char c = static_cast<unsigned char>(i);
        setPaletteEntry(c, c, c, c);
    }

    // Custom palette entries used by legacy dialog/UI rendering.
    setPaletteEntry(0, 0, 0, 0);
    setPaletteEntry(15, 255, 255, 255);
    setPaletteEntry(22, 176, 144, 64);
    setPaletteEntry(32, 44, 160, 84);
    setPaletteEntry(46, 255, 220, 64);
    setPaletteEntry(69, 235, 80, 80);
    setPaletteEntry(80, 28, 46, 78);
    setPaletteEntry(82, 88, 136, 188);
    setPaletteEntry(83, 112, 164, 216);
    setPaletteEntry(84, 56, 104, 172);
    setPaletteEntry(89, 232, 188, 56);
    setPaletteEntry(127, 108, 200, 148);
    setPaletteEntry(140, 198, 202, 212);
    setPaletteEntry(141, 170, 176, 188);
    setPaletteEntry(157, 255, 140, 64);
    setPaletteEntry(181, 244, 148, 64);
    setPaletteEntry(186, 132, 138, 150);
    setPaletteEntry(190, 76, 212, 116);
    setPaletteEntry(226, 58, 82, 122);
    setPaletteEntry(230, 35, 52, 79);
    setPaletteEntry(235, 226, 230, 238);
}
