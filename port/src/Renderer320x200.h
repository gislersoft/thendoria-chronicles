#pragma once

#include <SDL.h>

#include <array>
#include <cstdint>

class Renderer320x200 {
public:
    static constexpr int kWidth = 320;
    static constexpr int kHeight = 200;
    static constexpr int kPixels = kWidth * kHeight;

    explicit Renderer320x200(SDL_Renderer *renderer);
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
    SDL_Texture *texture_;

    std::array<unsigned char, kPixels> vga_{};
    std::array<unsigned char, kPixels> pv1_{};
    std::array<unsigned char, kPixels> pv2_{};

    std::array<std::uint32_t, kPixels> rgbaScratch_{};
    std::array<std::uint32_t, kPixels> overlay_{};
    std::array<std::uint32_t, 256> palette_{};

    void initDefaultPalette();
};
