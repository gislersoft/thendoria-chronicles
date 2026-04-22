#include <SDL.h>
#if defined(THENDORIA_HAVE_SDL_MIXER)
#include <SDL_mixer.h>
#endif

#include "GraphCompat.h"
#include "FontCompat.h"
#include "MapData.h"
#include "SpriteCompat.h"
#include "TileSetCompat.h"
#include "IntroScreen.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {

enum Direction {
    DIR_NORTE,
    DIR_ESTE,
    DIR_SUR,
    DIR_OESTE
};

bool existsRelativeToWorkspace(const std::string &relativePath) {
    std::filesystem::path p = std::filesystem::current_path() / ".." / relativePath;
    return std::filesystem::exists(p);
}

std::string firstExistingPath(const std::initializer_list<std::string> &candidates) {
    for (const auto &c : candidates) {
        if (std::filesystem::exists(c)) {
            return c;
        }
    }
    return std::string();
}

bool loadTextLines(const std::string &path, std::vector<std::string> &out) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }
    out.clear();
    std::string line;
    while (std::getline(in, line)) {
        out.push_back(line);
    }
    return true;
}

std::string resolveDialogPath(const std::string &baseName) {
    std::string name = baseName;
    name.erase(std::remove_if(name.begin(), name.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    }), name.end());

    if (name.empty() || name == "NULL" || name == "null") {
        return std::string();
    }

    auto stripExtension = [](const std::string &s) {
        if (s.size() >= 4) {
            const std::string ext = s.substr(s.size() - 4);
            if (ext == ".TXT" || ext == ".txt") {
                return s.substr(0, s.size() - 4);
            }
        }
        return s;
    };

    const std::string nameNoExt = stripExtension(name);
    return firstExistingPath({
        "DIALOGS/" + nameNoExt + ".TXT",
        "DIALOGS/" + nameNoExt + ".txt",
        "../DIALOGS/" + nameNoExt + ".TXT",
        "../DIALOGS/" + nameNoExt + ".txt"
    });
}

std::string resolveMapPath(const std::string &baseName) {
    std::string name = baseName;
    name.erase(std::remove_if(name.begin(), name.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    }), name.end());

    if (name.empty()) {
        return std::string();
    }

    if (name == "NULL" || name == "null") {
        return std::string();
    }

    std::replace(name.begin(), name.end(), '\\', '/');

    auto toUpper = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        return s;
    };

    auto stripExtension = [](const std::string &s) {
        if (s.size() >= 4) {
            const std::string ext = s.substr(s.size() - 4);
            if (ext == ".TXT" || ext == ".txt") {
                return s.substr(0, s.size() - 4);
            }
        }
        return s;
    };

    static const std::map<std::string, std::string> kMapAliases = {
        {"BOSQ2", "BOSQ3"}
    };

    // Fast path for common direct references.
    std::string nameNoExt = stripExtension(name);
    const std::string upperName = toUpper(nameNoExt);
    const auto aliasIt = kMapAliases.find(upperName);
    if (aliasIt != kMapAliases.end()) {
        nameNoExt = aliasIt->second;
    }

    const std::string direct = firstExistingPath({
        nameNoExt + ".TXT",
        nameNoExt + ".txt",
        "MAPS/" + nameNoExt + ".TXT",
        "MAPS/" + nameNoExt + ".txt",
        "../MAPS/" + nameNoExt + ".TXT",
        "../MAPS/" + nameNoExt + ".txt"
    });
    if (!direct.empty()) {
        return direct;
    }

    const std::string target = toUpper(nameNoExt);
    const std::vector<std::filesystem::path> roots = {
        "MAPS",
        "maps",
        "../MAPS",
        "../maps"
    };

    std::map<std::string, std::string> stemsToPaths;
    for (const auto &root : roots) {
        if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
            continue;
        }
        for (const auto &entry : std::filesystem::directory_iterator(root)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const std::string ext = toUpper(entry.path().extension().string());
            if (ext != ".TXT") {
                continue;
            }
            const std::string stem = toUpper(entry.path().stem().string());
            if (stemsToPaths.find(stem) == stemsToPaths.end()) {
                stemsToPaths.emplace(stem, entry.path().generic_string());
            }
        }
    }

    const auto exactIt = stemsToPaths.find(target);
    if (exactIt != stemsToPaths.end()) {
        return exactIt->second;
    }

    // Legacy data has abbreviated targets like BOSQ that should match BOSQ1.
    for (const auto &kv : stemsToPaths) {
        if (kv.first.rfind(target, 0) == 0) {
            return kv.second;
        }
    }

    return std::string();
}

struct LaunchOptions {
    bool hasMap = false;
    bool hasX = false;
    bool hasY = false;
    std::string mapName;
    int startX = 0;
    int startY = 0;
};

bool parseInt(const std::string &s, int &out) {
    try {
        size_t idx = 0;
        int v = std::stoi(s, &idx, 10);
        if (idx != s.size()) {
            return false;
        }
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

LaunchOptions parseLaunchOptions(int argc, char **argv) {
    LaunchOptions options;

    auto toLower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return s;
    };

    for (int i = 1; i < argc; ++i) {
        if (!argv[i]) {
            continue;
        }

        std::string arg = argv[i];
        if (arg.empty()) {
            continue;
        }

        while (!arg.empty() && (arg[0] == '-' || arg[0] == '/')) {
            arg.erase(arg.begin());
        }
        if (arg.empty()) {
            continue;
        }

        std::string key;
        std::string value;
        const size_t eq = arg.find('=');
        if (eq != std::string::npos) {
            key = arg.substr(0, eq);
            value = arg.substr(eq + 1);
        } else {
            key = arg;
            if (i + 1 < argc && argv[i + 1]) {
                std::string next = argv[i + 1];
                if (!next.empty() && next[0] != '-' && next[0] != '/') {
                    value = next;
                    ++i;
                }
            }
        }

        key = toLower(key);
        if (key == "map") {
            if (!value.empty()) {
                options.hasMap = true;
                options.mapName = value;
            }
        } else if (key == "x") {
            int parsed = 0;
            if (parseInt(value, parsed)) {
                options.hasX = true;
                options.startX = parsed;
            }
        } else if (key == "y") {
            int parsed = 0;
            if (parseInt(value, parsed)) {
                options.hasY = true;
                options.startY = parsed;
            }
        }
    }

    return options;
}

} // namespace

int main(int argc, char **argv) {
    const LaunchOptions launchOptions = parseLaunchOptions(argc, argv);

    const std::uint32_t appLaunchTicks = SDL_GetTicks();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Thendoria Port (SDL2)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        960,
        600,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const bool hasMapAsset = !firstExistingPath({"MAPS/BASE.TXT", "MAPS/base.txt", "../MAPS/BASE.TXT", "../MAPS/base.txt"}).empty();
    const bool hasDialogAsset = !firstExistingPath({"DIALOGS/LIN02.TXT", "DIALOGS/lin02.txt", "../DIALOGS/LIN02.TXT", "../DIALOGS/lin02.txt"}).empty();
    const bool hasImgAsset = !firstExistingPath({"IMG/intro.pcx", "IMG/INTRO.PCX", "../IMG/intro.pcx", "../IMG/INTRO.PCX"}).empty();

    std::cout << "Workspace assets check:" << '\n';
    std::cout << " - MAPS/base.txt:   " << (hasMapAsset ? "OK" : "MISSING") << '\n';
    std::cout << " - DIALOGS/lin02:   " << (hasDialogAsset ? "OK" : "MISSING") << '\n';
    std::cout << " - IMG/intro.pcx:   " << (hasImgAsset ? "OK" : "MISSING") << '\n';

    GraphCompat graph(0, renderer);
    if (graph.status() == 0) {
        std::cerr << "GraphCompat initialization failed" << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SpriteCompat hero;
    hero.crear(8, 16, 0.12f);
    if (hero.status() == 0) {
        std::cerr << "SpriteCompat allocation failed" << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const std::string heroSheet = firstExistingPath({
        "port/assets_png/IMG/b16.png",
        "port/assets_png/IMG/PROF.png",
        "assets_png/IMG/b16.png",
        "../assets_png/IMG/b16.png",
        "../port/assets_png/IMG/b16.png",
        "../port/assets_png/IMG/PROF.png",
        "../assets_png/IMG/PROF.png",
        "assets_png/IMG/PROF.png"
    });

    bool loadedFromPng = false;
    if (!heroSheet.empty()) {
        loadedFromPng = (hero.cargarSpritePNG(heroSheet) == 1);
        if (loadedFromPng) {
            std::cout << "Loaded hero sprite sheet: " << heroSheet << '\n';
        }
    }

    if (!loadedFromPng) {
        std::cout << "Using procedural fallback sprite (PNG sheet not found or failed to load)." << '\n';
        for (int f = 0; f < 8; ++f) {
            unsigned char base = static_cast<unsigned char>(40 + f * 20);
            for (int y = 0; y < 16; ++y) {
                for (int x = 0; x < 16; ++x) {
                    unsigned char c = 0;
                    if ((x > 3 && x < 12) && (y > 2 && y < 14)) {
                        c = base;
                    }
                    if (((x + y + f) % 6) == 0) {
                        c = 15;
                    }
                    hero.frameData(f)[x + y * 16] = c;
                }
            }
        }
    }

    TileSetCompat tiles;
    const std::string tileSheet = firstExistingPath({
        "port/assets_png/IMG/16x16.png",
        "assets_png/IMG/16x16.png",
        "../assets_png/IMG/16x16.png",
        "../port/assets_png/IMG/16x16.png"
    });
    bool tilesLoaded = false;
    if (!tileSheet.empty()) {
        tilesLoaded = tiles.loadFromPngSheet(tileSheet, 16);
        if (tilesLoaded) {
            std::cout << "Loaded tile sheet: " << tileSheet << " (tiles=" << tiles.tileCount() << ")" << '\n';
        }
    }

    MapData world;
    std::string mapPath;
    if (launchOptions.hasMap) {
        mapPath = resolveMapPath(launchOptions.mapName);
        if (mapPath.empty()) {
            std::cout << "Could not resolve map from -map=" << launchOptions.mapName << ". Falling back to default map.\n";
        }
    }
    if (mapPath.empty()) {
        mapPath = firstExistingPath({
        "MAPS/CASA.TXT",
        "../MAPS/CASA.TXT",
        "maps/CASA.TXT",
        "../maps/CASA.TXT"
        });
    }
    bool mapLoaded = false;
    if (!mapPath.empty()) {
        mapLoaded = world.loadFromFile(mapPath);
        if (mapLoaded) {
            std::cout << "Loaded map: " << mapPath << '\n';
        }
    }

    int xpos_actual = 20;
    int ypos_actual = 18;
    if (launchOptions.hasX) {
        xpos_actual = std::clamp(launchOptions.startX, 0, MapData::kSize - 1);
    }
    if (launchOptions.hasY) {
        ypos_actual = std::clamp(launchOptions.startY, 0, MapData::kSize - 1);
    }
    int xpos_ref = xpos_actual;
    int ypos_ref = ypos_actual;
    int xpos_scroll = 0;
    int ypos_scroll = 0;
    const int scroll_vel = 4;
    const int tope = 16 - scroll_vel;
    bool scroll = false;
    Direction brujula = DIR_SUR;

    bool dialogo = false;
    std::vector<std::string> dialogLines;
    int dialogStart0 = 0;
    int dialogEnd0 = -1;
    int dialogPage0 = 0;
    bool prevSpace = false;
    bool prevEnter = false;
    int frame = 0;
    const int quantum = 4;
    bool quanto = false;
    constexpr std::uint64_t targetFrameMs = 1000 / 30;
    std::string pendingMapPath;
    std::string pendingMapName;
    int pendingSpawnX = 0;
    int pendingSpawnY = 0;

    FontCompat font;

    auto triggerTileAction = [&](int tx, int ty, bool allowDialog) {
        if (tx < 0 || tx >= MapData::kSize || ty < 0 || ty >= MapData::kSize) {
            return;
        }

        const MapObject obj = world.at(tx, ty);
        switch (obj.accion) {
            case 0: {
                if (!allowDialog) {
                    return;
                }
                const std::string dialogPath = resolveDialogPath(obj.archivo);
                if (!dialogPath.empty() && loadTextLines(dialogPath, dialogLines)) {
                        if (obj.x <= 0 || obj.y <= 0) {
                            // Legacy maps sometimes store 0/0 for dialog range; default to first 5 lines.
                            dialogStart0 = 0;
                            dialogEnd0 = std::min(static_cast<int>(dialogLines.size()) - 1, 4);
                        } else {
                            dialogStart0 = std::max(0, obj.x - 1);
                            dialogEnd0 = std::max(dialogStart0, obj.y - 1);
                        }

                        dialogEnd0 = std::min(dialogEnd0, static_cast<int>(dialogLines.size()) - 1);
                    dialogPage0 = dialogStart0;
                    dialogo = true;
                } else if (obj.archivo != "NULL" && obj.archivo != "null") {
                    std::cout << "Failed to load dialog target: " << obj.archivo << '\n';
                }
                world.atMutable(tx, ty).tocado = true;
                break;
            }
            case 1:
                world.atMutable(tx, ty).tocado = true;
                break;
            case 3: {
                // TELEACCION: actua sobre el objeto remoto en (x, y) del objeto activador.
                const int rx = obj.x;
                const int ry = obj.y;
                if (rx >= 0 && rx < MapData::kSize && ry >= 0 && ry < MapData::kSize) {
                    MapObject &remote = world.atMutable(rx, ry);
                    // Legacy behavior: only affects ESTATICO targets.
                    if (remote.estado == 0) {
                        remote.solido = (remote.solido == 0) ? 1 : 0;
                        remote.actual = (remote.actual == 0) ? 1 : 0;
                        remote.tocado = true;
                    }
                }
                world.atMutable(tx, ty).tocado = true;
                break;
            }
            case 4: {
                const std::string newMapPath = resolveMapPath(obj.archivo);
                if (!newMapPath.empty()) {
                    pendingMapPath = newMapPath;
                    pendingMapName = obj.archivo;
                    pendingSpawnX = std::max(0, std::min(MapData::kSize - 1, obj.x - 1));
                    pendingSpawnY = std::max(0, std::min(MapData::kSize - 1, obj.y - 1));
                    world.atMutable(tx, ty).tocado = true;
                } else {
                    std::cout << "Failed to resolve map transition target: " << obj.archivo << '\n';
                }
                break;
            }
            default:
                break;
        }
    };

    // Play intro sequence
    IntroScreen intro;
    if (!launchOptions.hasMap) {
        intro.playFullIntro(graph, font, renderer, appLaunchTicks);
    } else {
        std::cout << "Quick start enabled (map argument provided): skipping intro." << '\n';
    }

#if defined(THENDORIA_HAVE_SDL_MIXER)
    Mix_Music *exploreMusic = nullptr;
    const std::string exploreMusicPath = firstExistingPath({
        "sound/music/exploreMusic.mp3",
        "port/sound/music/exploreMusic.mp3",
        "../port/sound/music/exploreMusic.mp3",
        "../sound/music/exploreMusic.mp3"
    });

    if (!exploreMusicPath.empty()) {
        int freq = 0;
        std::uint16_t format = 0;
        int channels = 0;
        if (Mix_QuerySpec(&freq, &format, &channels) == 0) {
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
                std::cerr << "Mix_OpenAudio failed for gameplay music: " << Mix_GetError() << '\n';
            }
        }

        if (Mix_QuerySpec(&freq, &format, &channels) != 0) {
            exploreMusic = Mix_LoadMUS(exploreMusicPath.c_str());
            if (!exploreMusic) {
                std::cerr << "Mix_LoadMUS failed for " << exploreMusicPath << ": " << Mix_GetError() << '\n';
            } else if (Mix_PlayMusic(exploreMusic, -1) != 0) {
                std::cerr << "Mix_PlayMusic failed for gameplay music: " << Mix_GetError() << '\n';
                Mix_FreeMusic(exploreMusic);
                exploreMusic = nullptr;
            }
        }
    } else {
        std::cerr << "Could not find gameplay music: exploreMusic.mp3" << '\n';
    }
#endif

    bool running = true;
    while (running) {
        const std::uint64_t frameStartMs = SDL_GetTicks64();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        const std::uint8_t *keys = SDL_GetKeyboardState(nullptr);
        const bool pressedSpace = keys[SDL_SCANCODE_SPACE] != 0;
        const bool pressedEnter = keys[SDL_SCANCODE_RETURN] != 0;
        const bool justPressedSpace = pressedSpace && !prevSpace;
        const bool justPressedEnter = pressedEnter && !prevEnter;
        prevSpace = pressedSpace;
        prevEnter = pressedEnter;

        frame++;
        if (frame >= quantum) {
            frame = 0;
            quanto = true;
        } else {
            quanto = false;
        }

        if (quanto && mapLoaded) {
            for (int j = 0; j < MapData::kSize; ++j) {
                for (int i = 0; i < MapData::kSize; ++i) {
                    MapObject &o = world.atMutable(i, j);
                    if (o.estado == 1) {
                        if (o.ciclico && o.actual == 2) {
                            o.actual = 0;
                        } else if (o.actual < 2) {
                            o.actual++;
                        }
                    } else if (o.estado == 2 && o.tocado) {
                        if (o.ciclico) {
                            o.actual = (o.actual == 1) ? 0 : 1;
                        } else if (o.actual == 0) {
                            o.actual = 1;
                        }
                    }
                }
            }
        }

        if (dialogo) {
            if (justPressedEnter) {
                dialogPage0 += 5;
                if (dialogPage0 > dialogEnd0) {
                    dialogo = false;
                }
            }
        }

        if (mapLoaded && !scroll && !dialogo) {
            int tx = xpos_actual;
            int ty = ypos_actual;

            if (keys[SDL_SCANCODE_UP]) {
                brujula = DIR_NORTE;
                ty = ypos_actual - 1;
            } else if (keys[SDL_SCANCODE_RIGHT]) {
                brujula = DIR_ESTE;
                tx = xpos_actual + 1;
            } else if (keys[SDL_SCANCODE_DOWN]) {
                brujula = DIR_SUR;
                ty = ypos_actual + 1;
            } else if (keys[SDL_SCANCODE_LEFT]) {
                brujula = DIR_OESTE;
                tx = xpos_actual - 1;
            }

            if (tx != xpos_actual || ty != ypos_actual) {
                if (tx >= 0 && tx < MapData::kSize && ty >= 0 && ty < MapData::kSize) {
                    const MapObject &target = world.at(tx, ty);
                    if (target.solido == 0) {
                        xpos_actual = tx;
                        ypos_actual = ty;
                        scroll = true;
                        triggerTileAction(tx, ty, true);
                    }
                }
            }
        }

        if (mapLoaded && !scroll && !dialogo && justPressedSpace) {
            int tx = xpos_actual;
            int ty = ypos_actual;
            switch (brujula) {
                case DIR_NORTE:
                    ty = ypos_actual - 1;
                    break;
                case DIR_ESTE:
                    tx = xpos_actual + 1;
                    break;
                case DIR_SUR:
                    ty = ypos_actual + 1;
                    break;
                case DIR_OESTE:
                    tx = xpos_actual - 1;
                    break;
            }

            if (tx >= 0 && tx < MapData::kSize && ty >= 0 && ty < MapData::kSize) {
                triggerTileAction(tx, ty, true);
            }
        }

        if (!pendingMapPath.empty() && !scroll && !dialogo) {
            if (world.loadFromFile(pendingMapPath)) {
                xpos_actual = pendingSpawnX;
                ypos_actual = pendingSpawnY;
                xpos_ref = xpos_actual;
                ypos_ref = ypos_actual;
                xpos_scroll = 0;
                ypos_scroll = 0;
                scroll = false;
                dialogo = false;
                dialogLines.clear();
                std::cout << "Loaded map: " << pendingMapPath << " (target=" << pendingMapName << ")" << '\n';
            } else {
                std::cout << "Failed to load map file: " << pendingMapPath << " (target=" << pendingMapName << ")" << '\n';
            }
            pendingMapPath.clear();
            pendingMapName.clear();
        }

        graph.clearOverlay();
        graph.clr(graph.pv1, 0);
        graph.clr(graph.pv2, 0);

        if (tilesLoaded && mapLoaded) {
            for (int j = -1; j < 14; ++j) {
                for (int i = -1; i < 21; ++i) {
                    const int wx = xpos_ref - 10 + i;
                    const int wy = ypos_ref - 6 + j;
                    const int sx = (i << 4) + xpos_scroll;
                    const int sy = (j << 4) + ypos_scroll;

                    if (wx >= 0 && wx < MapData::kSize && wy >= 0 && wy < MapData::kSize) {
                        const MapObject &o = world.at(wx, wy);
                        int actual = o.actual;
                        if (actual < 0 || actual > 2) {
                            actual = 0;
                        }
                        int tileId = o.tiles[actual] - 1;
                        if (tileId >= 0) {
                            tiles.drawTile(graph, graph.pv1, tileId, sx, sy, 0);
                        } else {
                            graph.fillbox(graph.pv1, sx, sy, sx + 15, sy + 15, 0);
                        }
                    } else {
                        graph.fillbox(graph.pv1, sx, sy, sx + 15, sy + 15, 0);
                    }
                }
            }

            if (scroll) {
                switch (brujula) {
                    case DIR_NORTE:
                        if (ypos_scroll >= tope) {
                            ypos_scroll = 0;
                            scroll = false;
                            xpos_ref = xpos_actual;
                            ypos_ref = ypos_actual;
                        } else {
                            ypos_scroll += scroll_vel;
                            xpos_scroll = 0;
                        }
                        break;
                    case DIR_ESTE:
                        if (xpos_scroll <= -tope) {
                            xpos_scroll = 0;
                            scroll = false;
                            xpos_ref = xpos_actual;
                            ypos_ref = ypos_actual;
                        } else {
                            xpos_scroll -= scroll_vel;
                            ypos_scroll = 0;
                        }
                        break;
                    case DIR_SUR:
                        if (ypos_scroll <= -tope) {
                            ypos_scroll = 0;
                            scroll = false;
                            xpos_ref = xpos_actual;
                            ypos_ref = ypos_actual;
                        } else {
                            ypos_scroll -= scroll_vel;
                            xpos_scroll = 0;
                        }
                        break;
                    case DIR_OESTE:
                        if (xpos_scroll >= tope) {
                            xpos_scroll = 0;
                            scroll = false;
                            xpos_ref = xpos_actual;
                            ypos_ref = ypos_actual;
                        } else {
                            xpos_scroll += scroll_vel;
                            ypos_scroll = 0;
                        }
                        break;
                }
            } else {
                xpos_ref = xpos_actual;
                ypos_ref = ypos_actual;
            }

            int heroFrame = 2;
            switch (brujula) {
                case DIR_NORTE:
                    heroFrame = (scroll && ypos_scroll >= 8) ? 4 : 0;
                    break;
                case DIR_ESTE:
                    heroFrame = (scroll && xpos_scroll <= -8) ? 5 : 1;
                    break;
                case DIR_SUR:
                    heroFrame = (scroll && ypos_scroll <= -8) ? 6 : 2;
                    break;
                case DIR_OESTE:
                    heroFrame = (scroll && xpos_scroll >= 8) ? 7 : 3;
                    break;
            }
            hero.posicionar(160, 94);
            hero.dibujar(heroFrame, 0, graph);
        } else {
            graph.fillbox(graph.pv1, 20, 20, 299, 179, 230);
            graph.box(graph.pv1, 20, 20, 299, 179, 186);
            graph.line(graph.pv1, 20, 20, 299, 179, 84);
            graph.line(graph.pv1, 299, 20, 20, 179, 84);

            int t = static_cast<int>((SDL_GetTicks() / 12) % 240);
            int px = 40 + (t % 220);
            int py = 90 + ((t / 3) % 50);
            hero.posicionar(px, py);
            hero.dibujart(0, 1, 0, SDL_GetTicks(), graph);
        }

        if (dialogo) {
            constexpr int kDialogMaxChars = 40;
            constexpr int kScreenWidth = 320;
            constexpr int kDialogTop = 140;
            constexpr int kCharStepPx = 6;
            constexpr int kInnerPad = 6;

            std::vector<std::string> pageLines;
            pageLines.reserve(5);
            int maxChars = 0;

            for (int l = 0; l < 5; ++l) {
                int idx = dialogPage0 + l;
                std::string lineText;
                if (idx >= 0 && idx < static_cast<int>(dialogLines.size()) && idx <= dialogEnd0) {
                    lineText = dialogLines[static_cast<std::size_t>(idx)];
                    while (!lineText.empty() && lineText.back() == ' ') {
                        lineText.pop_back();
                    }
                    if (lineText.size() > static_cast<std::size_t>(kDialogMaxChars)) {
                        lineText.resize(static_cast<std::size_t>(kDialogMaxChars));
                    }
                }
                maxChars = std::max(maxChars, static_cast<int>(lineText.size()));
                pageLines.push_back(lineText);
            }

            const int textWidthPx = std::max(1, maxChars) * kCharStepPx;
            const int boxWidth = std::min(304, textWidthPx + (kInnerPad * 2));
            const int boxHeight = 56;
            const int boxX1 = std::max(0, (kScreenWidth - boxWidth) / 2);
            const int boxY1 = kDialogTop;
            const int boxX2 = boxX1 + boxWidth;
            const int boxY2 = boxY1 + boxHeight;

            graph.fillbox(graph.pv2, boxX1, boxY1, boxX2, boxY2, 230);
            graph.box(graph.pv2, boxX1, boxY1, boxX2, boxY2, 15);

            bool plusMode = false;
            bool minusMode = false;
            for (int l = 0; l < 5; ++l) {
                const std::string &lineText = pageLines[static_cast<std::size_t>(l)];
                if (!lineText.empty()) {
                    if (l == 0) {
                        font.putstrWithState(graph.pv2, boxX1 + kInnerPad, boxY1 + kInnerPad + l * 8, lineText, graph, 230, 46, plusMode, minusMode);
                    } else if (l == 4) {
                        font.putstrWithState(graph.pv2, boxX1 + kInnerPad, boxY1 + kInnerPad + l * 8, lineText, graph, 230, 32, plusMode, minusMode);
                    } else {
                        font.putstrTexturedWithState(graph.pv2, boxX1 + kInnerPad, boxY1 + kInnerPad + l * 8, lineText, graph, 230, 3, plusMode, minusMode);
                    }
                }
            }

            bool hasEnterInLastLine = false;
            if (!pageLines.empty()) {
                std::string last = pageLines.back();
                std::transform(last.begin(), last.end(), last.begin(), [](unsigned char c) {
                    return static_cast<char>(std::toupper(c));
                });
                hasEnterInLastLine = (last.find("ENTER") != std::string::npos);
            }

            if (!hasEnterInLastLine) {
                font.putstr(graph.pv2, boxX2 - 34, boxY2 - 9, "ENTER", graph, 230, 32);
            }
        }

        graph.wait_retrace();
        graph.volcar(graph.vga, graph.pv1);
        graph.presentLayers(graph.vga, graph.pv2);

        const std::uint64_t frameElapsedMs = SDL_GetTicks64() - frameStartMs;
        if (frameElapsedMs < targetFrameMs) {
            SDL_Delay(static_cast<std::uint32_t>(targetFrameMs - frameElapsedMs));
        }
    }

#if defined(THENDORIA_HAVE_SDL_MIXER)
    Mix_HaltMusic();
    if (exploreMusic) {
        Mix_FreeMusic(exploreMusic);
        exploreMusic = nullptr;
    }
#endif

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
