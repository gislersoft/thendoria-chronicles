#pragma once

#include <SDL.h>

#include <array>
#include <cstdint>
#include <vector>

class Renderer320x200 {
public:
    static constexpr int kWidth = 320;
    static constexpr int kHeight = 200;
    static constexpr int kPixels = kWidth * kHeight;

    // LCD grid effect: each game pixel is rendered as kScale×kScale screen pixels,
    // with a 1-pixel black border on the right and bottom edge of every block.
    static constexpr int kScale       = 3;
    static constexpr int kScaledWidth  = kWidth  * kScale;
    static constexpr int kScaledHeight = kHeight * kScale;
    static constexpr int kScaledPixels = kScaledWidth * kScaledHeight;
    static constexpr std::uint32_t kGridColor = 0xFF306230u;  // DMG shade 2 — neutral mid-tone grid

    // Brightness boost applied to lit pixels to compensate for the dark grid lines
    // consuming ~55% of screen area. Expressed as num/den fraction, clamped to 255.
    // 3/2 = 1.5× — raise or lower kBrightNum to taste.
    static constexpr std::uint32_t kBrightNum = 6u;
    static constexpr std::uint32_t kBrightDen = 5u;

    // Scanline effect: the second visible screen row inside each pixel block (sy==1)
    // is dimmed by this fraction to simulate the dark gap between CRT phosphor lines.
    // 1/2 = 50% brightness on the scanline row.
    static constexpr std::uint32_t kScanlineNum = 1u;
    static constexpr std::uint32_t kScanlineDen = 2u;

    explicit Renderer320x200(SDL_Renderer *renderer, bool gameboyFilter = false);
    ~Renderer320x200();

    unsigned char *vga();
    unsigned char *pv1();
    unsigned char *pv2();

    void clr(unsigned char *px, unsigned char color);
    void volcar(unsigned char *dest, const unsigned char *src);
    void putpixel(unsigned char *px, int x, int y, unsigned char color);
    unsigned char getpixel(const unsigned char *px, int x, int y) const;
    void getframe(const unsigned char *px, int x, int y, int a, unsigned char *img) const;
    void putframe(unsigned char *px, int x, int y, int a, const unsigned char *img, unsigned char transparentColor);
    void hline(unsigned char *px, int x1, int x2, int y, unsigned char color);
    void vline(unsigned char *px, int x, int y1, int y2, unsigned char color);
    void fillbox(unsigned char *px, int x1, int y1, int x2, int y2, unsigned char color);
    void box(unsigned char *px, int x1, int y1, int x2, int y2, unsigned char color);
    void line(unsigned char *px, int x1, int y1, int x2, int y2, unsigned char color);
    void wait_retrace();

    void setPaletteEntry(unsigned char index, unsigned char r, unsigned char g, unsigned char b);
    void clearOverlay();
    void drawRgbaFrame(int x, int y, int w, int h, const std::uint32_t *img);
    std::uint32_t *getOverlay();
    void present(const unsigned char *px);
    void present(const unsigned char *px, const unsigned char *top);

private:
    SDL_Renderer *renderer_;
    SDL_Texture *gridTexture_;   // kScaledWidth × kScaledHeight — used with gameboyFilter_
    SDL_Texture *plainTexture_;  // kWidth × kHeight — used without gameboyFilter_

    std::array<unsigned char, kPixels> vga_{};
    std::array<unsigned char, kPixels> pv1_{};
    std::array<unsigned char, kPixels> pv2_{};

    std::array<std::uint32_t, kPixels>  rgbaScratch_{};
    std::vector<std::uint32_t>             rgbaGrid_;   // kScaledPixels — heap allocated
    std::array<std::uint32_t, kPixels>  overlay_{};
    std::array<std::uint32_t, 256>           palette_{};

    bool gameboyFilter_ = false;  // enables LCD grid + Game Boy green colour mapping

    void initDefaultPalette();
};
