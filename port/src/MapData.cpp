#include "MapData.h"

#include <filesystem>
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

bool MapData::saveToFile(const std::string &path) const {
    const std::filesystem::path p(path);
    if (!p.parent_path().empty()) {
        std::error_code ec;
        std::filesystem::create_directories(p.parent_path(), ec);
    }

    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    for (int j = 0; j < kSize; ++j) {
        for (int i = 0; i < kSize; ++i) {
            const MapObject &o = data_[i][j];
            out << o.importante << ' '
                << o.solido << ' '
                << o.ciclico << ' '
                << o.accion << ' '
                << o.estado << ' '
                << o.nombre << ' '
                << o.archivo << ' '
                << o.tiles[0] << ' '
                << o.tiles[1] << ' '
                << o.tiles[2] << ' '
                << o.x << ' '
                << o.y << '\n';
        }
    }

    return out.good();
}

const MapObject &MapData::at(int x, int y) const {
    return data_[x][y];
}

MapObject &MapData::atMutable(int x, int y) {
    return data_[x][y];
}
