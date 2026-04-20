#include "MapData.h"

#include <fstream>
#include <sstream>

bool MapData::loadFromFile(const std::string &path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    for (int j = 0; j < kSize; ++j) {
        for (int i = 0; i < kSize; ++i) {
            std::string line;
            if (!std::getline(in, line)) {
                return false;
            }

            std::istringstream iss(line);
            MapObject o{};
            if (!(iss >> o.importante >> o.solido >> o.ciclico >> o.accion >> o.estado >> o.nombre >> o.archivo >>
                  o.tiles[0] >> o.tiles[1] >> o.tiles[2] >> o.x >> o.y)) {
                return false;
            }

            o.actual = 0;
            o.tocado = false;
            data_[i][j] = o;
        }
    }

    return true;
}

const MapObject &MapData::at(int x, int y) const {
    return data_[x][y];
}

MapObject &MapData::atMutable(int x, int y) {
    return data_[x][y];
}
