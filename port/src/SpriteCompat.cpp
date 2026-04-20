#include "SpriteCompat.h"

#include <SDL_image.h>

#include <algorithm>

SpriteCompat::SpriteCompat()
    : numframes(0),
      tam(0),
      x1(0),
      y1(0),
      x2(0),
      y2(0),
      inicia(0),
      inicia2(0),
      animacion(0),
      reloj_(1),
      reloj2_(1),
      cf_(0),
      startMs_(0),
      start2Ms_(0),
    hasTrueColor_(false),
      tiempo_(0.1f) {}

void SpriteCompat::crear(int n, int t, float tm) {
    numframes = std::max(0, n);
    tam = std::max(0, t);
    tiempo_ = tm;

    inicia = 0;
    inicia2 = 0;
    animacion = 0;
    reloj_ = 1;
    reloj2_ = 1;
    cf_ = 0;

    frames_.clear();
    frames_.resize(static_cast<std::size_t>(numframes), std::vector<unsigned char>(static_cast<std::size_t>(tam * tam), 0));
    rgbaFrames_.clear();
    rgbaFrames_.resize(static_cast<std::size_t>(numframes), std::vector<std::uint32_t>(static_cast<std::size_t>(tam * tam), 0));
    hasTrueColor_ = false;
}

int SpriteCompat::status() const {
    if (numframes <= 0 || tam <= 0) {
        return 0;
    }
    if (static_cast<int>(frames_.size()) != numframes) {
        return 0;
    }
    for (const auto &f : frames_) {
        if (static_cast<int>(f.size()) != tam * tam) {
            return 0;
        }
    }
    return 1;
}

void SpriteCompat::posicionar(int x, int y) {
    x1 = x;
    y1 = y;
    x2 = x + tam;
    y2 = y + tam;
}

void SpriteCompat::dibujar(int f, unsigned char transparentColor, GraphCompat &g) const {
    if (f < 0 || f >= numframes) {
        return;
    }
    const std::size_t idx = static_cast<std::size_t>(f);
    if (hasTrueColor_ && idx < rgbaFrames_.size() && !rgbaFrames_[idx].empty()) {
        g.drawRgbaFrame(x1, y1, tam, tam, rgbaFrames_[idx].data());
    } else {
        g.putframe(g.pv1, x1, y1, tam, frames_[idx].data(), transparentColor);
    }
}

void SpriteCompat::dibujart(int ini, int fin, unsigned char transparentColor, std::uint32_t nowMs, GraphCompat &g) {
    if (ini > fin || ini < 0 || fin >= numframes) {
        return;
    }

    if ((cf_ < ini) || (cf_ > fin)) {
        inicia = 0;
    }
    if (inicia == 0) {
        cf_ = ini;
        inicia = 1;
    }
    if (reloj_ == 1) {
        startMs_ = nowMs;
        reloj_ = 0;
    }

    if ((nowMs - startMs_) > static_cast<std::uint32_t>(tiempo_ * 1000.0f)) {
        cf_++;
        if (cf_ > fin) {
            cf_ = ini;
        }
        reloj_ = 1;
    }

    dibujar(cf_, transparentColor, g);
}

void SpriteCompat::animar(int ini, int fin, unsigned char transparentColor, std::uint32_t nowMs, GraphCompat &g) {
    if (ini > fin || ini < 0 || fin >= numframes) {
        return;
    }

    if (animacion == 1) {
        if ((cf_ < ini) || (cf_ > fin)) {
            inicia2 = 0;
        }
        if (inicia2 == 0) {
            cf_ = ini;
            inicia2 = 1;
        }
        if (reloj2_ == 1) {
            start2Ms_ = nowMs;
            reloj2_ = 0;
        }
        if ((nowMs - start2Ms_) > static_cast<std::uint32_t>(tiempo_ * 1000.0f)) {
            cf_++;
            if (cf_ > fin) {
                animacion = 0;
            }
            reloj2_ = 1;
        }
        if (animacion == 1) {
            dibujar(cf_, transparentColor, g);
        }
    }
}

unsigned char *SpriteCompat::frameData(int index) {
    if (index < 0 || index >= numframes) {
        return nullptr;
    }
    return frames_[static_cast<std::size_t>(index)].data();
}

const unsigned char *SpriteCompat::frameData(int index) const {
    if (index < 0 || index >= numframes) {
        return nullptr;
    }
    return frames_[static_cast<std::size_t>(index)].data();
}

int SpriteCompat::cargarSpritePNG(const std::string &path) {
    if (status() == 0) {
        return 0;
    }

    SDL_Surface *loaded = IMG_Load(path.c_str());
    if (!loaded) {
        return 0;
    }

    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loaded);
    if (!rgba) {
        return 0;
    }

    int o = 0;
    int u = 0;
    for (int k = 0; k < numframes; ++k) {
        if ((o + tam) > rgba->w) {
            o = 0;
            u += tam;
        }
        if ((u + tam) > rgba->h) {
            SDL_FreeSurface(rgba);
            return 0;
        }

        for (int y = 0; y < tam; ++y) {
            const unsigned char *row = static_cast<const unsigned char *>(rgba->pixels) + (u + y) * rgba->pitch;
            for (int x = 0; x < tam; ++x) {
                const unsigned char *p = row + (o + x) * 4;
                const std::size_t idx = static_cast<std::size_t>(x + y * tam);
                frames_[static_cast<std::size_t>(k)][idx] = (p[3] < 16) ? 0 : 15;
                rgbaFrames_[static_cast<std::size_t>(k)][idx] = (static_cast<std::uint32_t>(p[3]) << 24) |
                                                               (static_cast<std::uint32_t>(p[0]) << 16) |
                                                               (static_cast<std::uint32_t>(p[1]) << 8) |
                                                               static_cast<std::uint32_t>(p[2]);
            }
        }

        o += tam;
    }

    SDL_FreeSurface(rgba);
    hasTrueColor_ = true;
    return 1;
}
