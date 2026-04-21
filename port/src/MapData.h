#pragma once

#include <array>
#include <string>

struct MapObject {
    int importante;
    int solido;
    int ciclico;
    int accion;
    int estado;
    int actual;
    bool tocado;
    std::string nombre;
    std::string archivo;
    std::array<int, 3> tiles;
    int x;
    int y;
};

class MapData {
public:
    static constexpr int kSize = 40;

    bool loadFromFile(const std::string &path);
    bool saveToFile(const std::string &path) const;
    const MapObject &at(int x, int y) const;
    MapObject &atMutable(int x, int y);

private:
    std::array<std::array<MapObject, kSize>, kSize> data_{};
};
