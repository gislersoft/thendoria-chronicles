#pragma once

#include <SDL.h>

#include "Renderer320x200.h"

#include <cstdint>

class GraphCompat {
public:
    unsigned char *vga;
    unsigned char *pv1;
    unsigned char *pv2;
    unsigned char *pvm;
    unsigned char *pvf;

    GraphCompat(int useMaskBuffers, SDL_Renderer *renderer, bool gameboyFilter = false);

    int status() const;
    void modo_video(int modo);

    void volcar(unsigned char *dest, const unsigned char *src);
    void volcarT(unsigned char *dest, const unsigned char *src);
    void clr(unsigned char *px, unsigned char color);

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
    void presentVGA();
    void presentLayers(const unsigned char *base, const unsigned char *top);
    void clearOverlay();
    void drawRgbaFrame(int x, int y, int w, int h, const std::uint32_t *img);
    std::uint32_t *getOverlay();

    // Returns the last composited 320×200 ARGB frame (valid after presentLayers).
    const std::uint32_t *getLastFrame() const { return renderer_.getLastFrame(); }

    void pal();
    void pal2();
    void copiar();
    void clrpal2();
    void fadeout(int demora);
    void fade12(int demora);
    void fade21(int demora);

private:
    int useMaskBuffers_;
    Renderer320x200 renderer_;
};
