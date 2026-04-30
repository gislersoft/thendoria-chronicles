#include "GraphCompat.h"

GraphCompat::GraphCompat(int useMaskBuffers, SDL_Renderer *renderer, bool gameboyFilter, bool gridFilter)
    : vga(nullptr),
      pv1(nullptr),
      pv2(nullptr),
      pvm(nullptr),
      pvf(nullptr),
      useMaskBuffers_(useMaskBuffers),
      renderer_(renderer, gameboyFilter, gridFilter) {
    vga = renderer_.vga();
    pv1 = renderer_.pv1();
    pv2 = renderer_.pv2();

    if (useMaskBuffers_ == 1) {
        // Placeholder until we add extra mask/front buffers.
        pvm = pv1;
        pvf = pv2;
    }
}

int GraphCompat::status() const {
    if (vga == nullptr || pv1 == nullptr || pv2 == nullptr) {
        return 0;
    }
    return 1;
}

void GraphCompat::modo_video(int modo) {
    (void)modo;
}

void GraphCompat::volcar(unsigned char *dest, const unsigned char *src) {
    renderer_.volcar(dest, src);
}

void GraphCompat::volcarT(unsigned char *dest, const unsigned char *src) {
    if (!dest || !src) {
        return;
    }
    for (int i = 0; i < Renderer320x200::kPixels; ++i) {
        if (src[i] != 0) {
            dest[i] = src[i];
        }
    }
}

void GraphCompat::clr(unsigned char *px, unsigned char color) {
    renderer_.clr(px, color);
}

void GraphCompat::putpixel(unsigned char *px, int x, int y, unsigned char color) {
    renderer_.putpixel(px, x, y, color);
}

unsigned char GraphCompat::getpixel(const unsigned char *px, int x, int y) const {
    return renderer_.getpixel(px, x, y);
}

void GraphCompat::getframe(const unsigned char *px, int x, int y, int a, unsigned char *img) const {
    renderer_.getframe(px, x, y, a, img);
}

void GraphCompat::putframe(unsigned char *px, int x, int y, int a, const unsigned char *img, unsigned char transparentColor) {
    renderer_.putframe(px, x, y, a, img, transparentColor);
}

void GraphCompat::hline(unsigned char *px, int x1, int x2, int y, unsigned char color) {
    renderer_.hline(px, x1, x2, y, color);
}

void GraphCompat::vline(unsigned char *px, int x, int y1, int y2, unsigned char color) {
    renderer_.vline(px, x, y1, y2, color);
}

void GraphCompat::fillbox(unsigned char *px, int x1, int y1, int x2, int y2, unsigned char color) {
    renderer_.fillbox(px, x1, y1, x2, y2, color);
}

void GraphCompat::box(unsigned char *px, int x1, int y1, int x2, int y2, unsigned char color) {
    renderer_.box(px, x1, y1, x2, y2, color);
}

void GraphCompat::line(unsigned char *px, int x1, int y1, int x2, int y2, unsigned char color) {
    renderer_.line(px, x1, y1, x2, y2, color);
}

void GraphCompat::wait_retrace() {
    renderer_.wait_retrace();
}

void GraphCompat::presentVGA() {
    renderer_.present(vga);
}

void GraphCompat::presentLayers(const unsigned char *base, const unsigned char *top) {
    renderer_.present(base, top);
}

void GraphCompat::clearOverlay() {
    renderer_.clearOverlay();
}

void GraphCompat::drawRgbaFrame(int x, int y, int w, int h, const std::uint32_t *img) {
    renderer_.drawRgbaFrame(x, y, w, h, img);
}

std::uint32_t *GraphCompat::getOverlay() {
    return renderer_.getOverlay();
}

void GraphCompat::pal() {
}

void GraphCompat::pal2() {
}

void GraphCompat::copiar() {
}

void GraphCompat::clrpal2() {
}

void GraphCompat::fadeout(int demora) {
    (void)demora;
}

void GraphCompat::fade12(int demora) {
    (void)demora;
}

void GraphCompat::fade21(int demora) {
    (void)demora;
}
