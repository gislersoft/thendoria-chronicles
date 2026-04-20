#pragma once

#include "GraphCompat.h"

#include <cstdint>
#include <string>
#include <vector>

class SpriteCompat {
public:
    SpriteCompat();

    void crear(int n, int t, float tm);
    int status() const;

    void posicionar(int x, int y);
    void dibujar(int f, unsigned char transparentColor, GraphCompat &g) const;
    void dibujart(int ini, int fin, unsigned char transparentColor, std::uint32_t nowMs, GraphCompat &g);
    void animar(int ini, int fin, unsigned char transparentColor, std::uint32_t nowMs, GraphCompat &g);
    int cargarSpritePNG(const std::string &path);

    unsigned char *frameData(int index);
    const unsigned char *frameData(int index) const;

    int numframes;
    int tam;
    int x1;
    int y1;
    int x2;
    int y2;
    int inicia;
    int inicia2;
    int animacion;

private:
    std::vector<std::vector<unsigned char>> frames_;
    std::vector<std::vector<std::uint32_t>> rgbaFrames_;
    bool hasTrueColor_;

    int reloj_;
    int reloj2_;
    int cf_;
    std::uint32_t startMs_;
    std::uint32_t start2Ms_;
    float tiempo_;
};
