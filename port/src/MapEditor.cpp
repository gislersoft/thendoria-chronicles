#include <SDL.h>
#include <SDL_image.h>
#include <SDL_syswm.h>

#include "MapData.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#endif

namespace {

constexpr int kMapSize = MapData::kSize;
constexpr int kTileSrcSize = 16;
constexpr int kViewTilesX = 40;
constexpr int kViewTilesY = 40;
constexpr int kCellPx = 10;
constexpr int kMapViewW = kViewTilesX * kCellPx;
constexpr int kMapViewH = kViewTilesY * kCellPx;

constexpr int kPanelX = kMapViewW + 10;   // 410
constexpr int kPanelW = 300;
constexpr int kWindowW = 800;
constexpr int kWindowH = 600;

// Palette: right column of the right panel.
// 20x15 visible tiles fit in 800x600 by using 12px palette cells.
constexpr int kPaletteX = kPanelX + 145;  // 555
constexpr int kPaletteY = 22;             // label drawn at kPaletteY-16=6
constexpr int kPaletteCols = 20;
constexpr int kPaletteRowsVisible = 15;
constexpr int kPaletteCell = 12;
constexpr int kIconSize = 20;

// Info panels stacked below the map view.
constexpr int kInfoPanelW = kMapViewW - 4;
constexpr int kInfoPanelH = 58;
constexpr int kInfoCol1X = 2;
constexpr int kInfoRow1Y = kMapViewH + 8;               // 408
constexpr int kInfoRow2Y = kInfoRow1Y + kInfoPanelH + 6; // 472
constexpr int kInfoRow3Y = kInfoRow2Y + kInfoPanelH + 6; // 536
constexpr int kHoverPanelX = kWindowW - kInfoPanelW - 2;
constexpr int kHoverPanelY = kWindowH - kInfoPanelH - 2;

const SDL_Color kColBg = {0, 0, 0, 255};
const SDL_Color kColPanelFill = {35, 52, 79, 255};
const SDL_Color kColPanelBorder = {255, 255, 255, 255};
const SDL_Color kColButtonFill = {28, 46, 78, 255};
const SDL_Color kColButtonFillActive = {44, 160, 84, 255};
const SDL_Color kColButtonBorder = {170, 176, 188, 255};
const SDL_Color kColButtonHover = {244, 148, 64, 255};
const SDL_Color kColText = {226, 230, 238, 255};
const SDL_Color kColTitle = {255, 220, 64, 255};

enum CommandId {
    CMD_NONE = 0,
    CMD_SAVE = 1,
    CMD_OPEN = 2,
    CMD_EXIT = 3,
    CMD_ACCION = 4,
    CMD_ESTADO = 5,
    CMD_IMPORTANTE = 6,
    CMD_SOLIDO = 7,
    CMD_CICLICO = 8,
    CMD_TILES = 9,
    CMD_POSICION = 10,
    CMD_NOMBRE = 11,
    CMD_ARCHIVO = 12,
    CMD_COPY_MODE = 13,
    CMD_USE_CLIPBOARD = 14,
    CMD_USE_MODIFIED = 15,
    CMD_MAP_CLICK = 16,
    CMD_FILL = 17,
    CMD_CLONE = 18
};

struct Button {
    SDL_Rect rect{};
    CommandId id = CMD_NONE;
    std::string label;
};

enum InputAction {
    INPUT_NONE = 0,
    INPUT_OPEN,
    INPUT_NOMBRE,
    INPUT_ARCHIVO,
    INPUT_POS_X,
    INPUT_POS_Y
};

struct InputState {
    bool        active  = false;
    InputAction action  = INPUT_NONE;
    std::string buffer;
    std::string label;
    std::size_t maxLen  = 64;
    int         tempX   = 0;  // holds x while waiting for y
};

struct Glyph {
    char c;
    std::uint8_t rows[7];
};

const Glyph kGlyphs[] = {
    {'A', {14, 17, 17, 31, 17, 17, 17}},
    {'B', {30, 17, 17, 30, 17, 17, 30}},
    {'C', {14, 17, 16, 16, 16, 17, 14}},
    {'D', {30, 17, 17, 17, 17, 17, 30}},
    {'E', {31, 16, 16, 30, 16, 16, 31}},
    {'F', {31, 16, 16, 30, 16, 16, 16}},
    {'G', {14, 17, 16, 23, 17, 17, 14}},
    {'H', {17, 17, 17, 31, 17, 17, 17}},
    {'I', {31, 4, 4, 4, 4, 4, 31}},
    {'J', {1, 1, 1, 1, 17, 17, 14}},
    {'K', {17, 18, 20, 24, 20, 18, 17}},
    {'L', {16, 16, 16, 16, 16, 16, 31}},
    {'M', {17, 27, 21, 21, 17, 17, 17}},
    {'N', {17, 25, 21, 19, 17, 17, 17}},
    {'O', {14, 17, 17, 17, 17, 17, 14}},
    {'P', {30, 17, 17, 30, 16, 16, 16}},
    {'Q', {14, 17, 17, 17, 21, 18, 13}},
    {'R', {30, 17, 17, 30, 20, 18, 17}},
    {'S', {15, 16, 16, 14, 1, 1, 30}},
    {'T', {31, 4, 4, 4, 4, 4, 4}},
    {'U', {17, 17, 17, 17, 17, 17, 14}},
    {'V', {17, 17, 17, 17, 17, 10, 4}},
    {'W', {17, 17, 17, 21, 21, 21, 10}},
    {'X', {17, 17, 10, 4, 10, 17, 17}},
    {'Y', {17, 17, 10, 4, 4, 4, 4}},
    {'Z', {31, 1, 2, 4, 8, 16, 31}},
    {'0', {14, 17, 19, 21, 25, 17, 14}},
    {'1', {4, 12, 4, 4, 4, 4, 14}},
    {'2', {14, 17, 1, 2, 4, 8, 31}},
    {'3', {30, 1, 1, 14, 1, 1, 30}},
    {'4', {2, 6, 10, 18, 31, 2, 2}},
    {'5', {31, 16, 16, 30, 1, 1, 30}},
    {'6', {14, 16, 16, 30, 17, 17, 14}},
    {'7', {31, 1, 2, 4, 8, 8, 8}},
    {'8', {14, 17, 17, 14, 17, 17, 14}},
    {'9', {14, 17, 17, 15, 1, 1, 14}},
    {'-', {0, 0, 0, 31, 0, 0, 0}},
    {'_', {0, 0, 0, 0, 0, 0, 31}},
    {':', {0, 12, 12, 0, 12, 12, 0}},
    {'/', {1, 2, 4, 8, 16, 0, 0}},
    {'.', {0, 0, 0, 0, 0, 12, 12}},
    {'*', {0, 10, 4, 31, 4, 10, 0}},
    {' ', {0, 0, 0, 0, 0, 0, 0}}
};

const std::uint8_t *glyphRows(char ch) {
    for (const auto &g : kGlyphs) {
        if (g.c == ch) {
            return g.rows;
        }
    }
    static const std::uint8_t fallback[7] = {14, 17, 2, 4, 4, 0, 4};
    return fallback;
}

MapObject defaultObject() {
    MapObject o{};
    o.importante = 0;
    o.solido = 0;
    o.ciclico = 0;
    o.accion = 2;
    o.estado = 0;
    o.actual = 0;
    o.tocado = false;
    o.nombre = "NULL";
    o.archivo = "NULL";
    o.tiles = {0, 0, 0};
    o.x = 0;
    o.y = 0;
    return o;
}

void toUpperAscii(std::string &s) {
    for (char &ch : s) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
}

std::string firstExistingPath(const std::vector<std::string> &candidates) {
    for (const auto &c : candidates) {
        if (std::filesystem::exists(c)) {
            return c;
        }
    }
    return std::string();
}

std::string resolveMapPath(int argc, char **argv) {
    if (argc > 1 && argv[1] && std::string(argv[1]).size() > 0) {
        return argv[1];
    }

    const std::string existing = firstExistingPath({
        "MAPS/BASE.TXT",
        "../MAPS/BASE.TXT",
        "MAPS/CASA.TXT",
        "../MAPS/CASA.TXT"
    });
    if (!existing.empty()) {
        return existing;
    }

    return "MAPS/NEWMAP.TXT";
}

SDL_Rect tileSrcRect(int tileIndex0, int tilesPerRow) {
    SDL_Rect src{};
    src.w = kTileSrcSize;
    src.h = kTileSrcSize;
    src.x = (tileIndex0 % tilesPerRow) * kTileSrcSize;
    src.y = (tileIndex0 / tilesPerRow) * kTileSrcSize;
    return src;
}

void clearMap(MapData &map) {
    for (int y = 0; y < kMapSize; ++y) {
        for (int x = 0; x < kMapSize; ++x) {
            map.atMutable(x, y) = defaultObject();
        }
    }
}

std::string boolText(int v) {
    return (v == 0) ? "FALSE" : "TRUE";
}

std::string accionText(int v) {
    switch (v) {
        case 0: return "PREGUNTA";
        case 1: return "ACCION";
        case 2: return "NEUTRO";
        case 3: return "TELEACCION";
        case 4: return "SALIDA";
        default: return "?";
    }
}

std::string estadoText(int v) {
    switch (v) {
        case 0: return "ESTATICO";
        case 1: return "ANIMADO";
        case 2: return "SUICHE";
        default: return "?";
    }
}

void drawText(SDL_Renderer *renderer, int x, int y, const std::string &text, SDL_Color color, int scale = 2) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    int cx = x;
    for (char raw : text) {
        char ch = raw;
        if (ch >= 'a' && ch <= 'z') {
            ch = static_cast<char>(ch - 'a' + 'A');
        }

        if (ch == '\n') {
            cx = x;
            y += 8 * scale;
            continue;
        }

        const std::uint8_t *rows = glyphRows(ch);
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (rows[row] & (1u << (4 - col))) {
                    SDL_Rect px{cx + col * scale, y + row * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &px);
                }
            }
        }
        cx += 6 * scale;
    }
}

bool pointInRect(int x, int y, const SDL_Rect &r) {
    return x >= r.x && x < (r.x + r.w) && y >= r.y && y < (r.y + r.h);
}

const Button *findButton(const std::vector<Button> &buttons, CommandId id) {
    for (const auto &b : buttons) {
        if (b.id == id) {
            return &b;
        }
    }
    return nullptr;
}

std::vector<Button> buildButtons() {
    std::vector<Button> out;
    auto add = [&](int x, int y, int w, int h, CommandId id, const char *label) {
        out.push_back(Button{SDL_Rect{x, y, w, h}, id, label});
    };

    int y = 10;
    const int h = 24;
    const int w = 145;
    add(kPanelX, y, w, h, CMD_ACCION, "ACCION"); y += 28;
    add(kPanelX, y, w, h, CMD_ESTADO, "ESTADO"); y += 28;
    add(kPanelX, y, w, h, CMD_IMPORTANTE, "IMPORTANTE"); y += 28;
    add(kPanelX, y, w, h, CMD_SOLIDO, "SOLIDO"); y += 28;
    add(kPanelX, y, w, h, CMD_CICLICO, "CICLICO"); y += 28;
    add(kPanelX, y, w, h, CMD_TILES, "TILES"); y += 28;
    add(kPanelX, y, w, h, CMD_POSICION, "POSICION"); y += 28;
    add(kPanelX, y, w, h, CMD_NOMBRE, "NOMBRE"); y += 28;
    add(kPanelX, y, w, h, CMD_ARCHIVO, "ARCHIVO");

    add(kPanelX, 310, 90, 24, CMD_COPY_MODE, "COPIAR");
    add(kPanelX + 96, 310, 90, 24, CMD_FILL, "LLENAR");
    add(kPanelX + 192, 310, 90, 24, CMD_CLONE, "CLONAR");

    add(kPanelX, 340, 90, 24, CMD_USE_CLIPBOARD, "PEGAR");
    add(kPanelX + 96, 340, 90, 24, CMD_USE_MODIFIED, "MODIF");

    add(kPanelX, 370, 90, 24, CMD_SAVE, "GUARDAR");
    add(kPanelX + 96, 370, 90, 24, CMD_OPEN, "ABRIR");
    add(kPanelX + 192, 370, 90, 24, CMD_EXIT, "SALIR");
    return out;
}

void drawButton(SDL_Renderer *renderer, const Button &b, bool hovered, bool active) {
    SDL_Color fill = active ? kColButtonFillActive : kColButtonFill;
    SDL_Color border = hovered ? kColButtonHover : kColButtonBorder;

    SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(renderer, &b.rect);
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(renderer, &b.rect);

    drawText(renderer, b.rect.x + 6, b.rect.y + 6, b.label, kColText, 2);
}

void drawObjectPanel(SDL_Renderer *renderer, const char *title, const MapObject &o, int x, int y, SDL_Color titleColor,
                     SDL_Texture *tileTexture, int tileCount, int tilesPerRow) {
    SDL_Rect panel{x, y, kInfoPanelW, kInfoPanelH};
    SDL_SetRenderDrawColor(renderer, kColPanelFill.r, kColPanelFill.g, kColPanelFill.b, kColPanelFill.a);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, kColPanelBorder.r, kColPanelBorder.g, kColPanelBorder.b, kColPanelBorder.a);
    SDL_RenderDrawRect(renderer, &panel);

    drawText(renderer, x + 6, y + 6, title, titleColor, 1);
    drawText(renderer, x + 6, y + 20, "ACCION:" + accionText(o.accion) + "  EST:" + estadoText(o.estado), kColText, 1);
    drawText(renderer, x + 6, y + 30, "I/S/C:" + boolText(o.importante) + "/" + boolText(o.solido) + "/" + boolText(o.ciclico), kColText, 1);
    drawText(renderer, x + 6, y + 40, "N:" + o.nombre + "  A:" + o.archivo + "  XY:" + std::to_string(o.x) + "," + std::to_string(o.y), kColText, 1);

    // Tile preview: show up to 3 tile slots as scaled images on the right side of the panel
    constexpr int kPreviewSize = 40;
    constexpr int kPreviewGap  = 2;
    const int previewBaseX = x + kInfoPanelW - 3 * (kPreviewSize + kPreviewGap) - 2;
    const int previewY     = y + (kInfoPanelH - kPreviewSize) / 2;
    for (int si = 0; si < 3; ++si) {
        int t = o.tiles[si];
        SDL_Rect dst{previewBaseX + si * (kPreviewSize + kPreviewGap), previewY, kPreviewSize, kPreviewSize};
        if (tileTexture && t > 0 && t <= tileCount) {
            SDL_Rect src = tileSrcRect(t - 1, tilesPerRow);
            SDL_RenderCopy(renderer, tileTexture, &src, &dst);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &dst);
        }
        SDL_SetRenderDrawColor(renderer, kColPanelBorder.r, kColPanelBorder.g, kColPanelBorder.b, kColPanelBorder.a);
        SDL_RenderDrawRect(renderer, &dst);
    }
}

void drawInputOverlay(SDL_Renderer *renderer, const InputState &inp) {
    if (!inp.active) {
        return;
    }
    // Dark overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect full{0, 0, kWindowW, kWindowH};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_Rect box{50, kWindowH / 2 - 24, kWindowW - 100, 48};
    SDL_SetRenderDrawColor(renderer, 24, 36, 60, 255);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, 255, 220, 64, 255);
    SDL_RenderDrawRect(renderer, &box);
    SDL_Rect inner{box.x + 1, box.y + 1, box.w - 2, box.h - 2};
    SDL_RenderDrawRect(renderer, &inner);

    drawText(renderer, box.x + 10, box.y + 10,
             inp.label + inp.buffer + "_",
             SDL_Color{255, 220, 64, 255}, 2);
    drawText(renderer, box.x + 10, box.y + 32,
             "ENTER=OK  ESC=CANCELAR",
             SDL_Color{160, 170, 190, 255}, 1);
}

void setWindowTitle(SDL_Window *window, const std::string &mapPath, int camX, int camY, bool dirty, bool copyMode) {
    if (!window) {
        return;
    }
    std::string t = "Thendoria Map Editor - " + mapPath + " | cam=" + std::to_string(camX) + "," + std::to_string(camY);
    if (copyMode) {
        t += " | COPY";
    }
    if (dirty) {
        t += " *";
    }
    SDL_SetWindowTitle(window, t.c_str());
}

bool pickMapFileDialog(std::string &selectedPath, const std::string &currentMapPath, SDL_Window *ownerWindow) {
#if defined(_WIN32)
    std::array<char, MAX_PATH> fileBuffer{};

    std::filesystem::path mapPath(currentMapPath);
    std::filesystem::path initDir = mapPath.has_parent_path() ? mapPath.parent_path() : std::filesystem::current_path();
    std::string initDirStr = initDir.string();

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    if (ownerWindow) {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        if (SDL_GetWindowWMInfo(ownerWindow, &wmInfo) == SDL_TRUE) {
            ofn.hwndOwner = wmInfo.info.win.window;
        }
    }
    ofn.lpstrFile = fileBuffer.data();
    ofn.nMaxFile = static_cast<DWORD>(fileBuffer.size());
    ofn.lpstrFilter = "Map files (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
    ofn.lpstrInitialDir = initDirStr.empty() ? nullptr : initDirStr.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "txt";

    if (!GetOpenFileNameA(&ofn)) {
        return false;
    }

    selectedPath = fileBuffer.data();
    return !selectedPath.empty();
#else
    (void)selectedPath;
    (void)currentMapPath;
    (void)ownerWindow;
    return false;
#endif
}

} // namespace

int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Thendoria Map Editor",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kWindowW,
        kWindowH,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    const std::string tileSheetPath = firstExistingPath({
        "port/assets_png/IMG/16x16.png",
        "assets_png/IMG/16x16.png",
        "../assets_png/IMG/16x16.png",
        "../port/assets_png/IMG/16x16.png"
    });
    if (tileSheetPath.empty()) {
        std::cerr << "Could not find 16x16 tile sheet PNG.\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Surface *sheetSurface = IMG_Load(tileSheetPath.c_str());
    if (!sheetSurface) {
        std::cerr << "Failed to load tile sheet: " << IMG_GetError() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *tileTexture = SDL_CreateTextureFromSurface(renderer, sheetSurface);
    if (!tileTexture) {
        std::cerr << "Failed to create tile texture: " << SDL_GetError() << '\n';
        SDL_FreeSurface(sheetSurface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    const int tilesPerRow = std::max(1, sheetSurface->w / kTileSrcSize);
    const int tilesPerCol = std::max(1, sheetSurface->h / kTileSrcSize);
    const int tileCount = tilesPerRow * tilesPerCol;
    SDL_FreeSurface(sheetSurface);

    SDL_Texture *iconTexture = nullptr;
    int iconTilesPerRow = 0;
    const std::string iconSheetPath = firstExistingPath({
        "port/assets_png/IMG/ico_til.png",
        "assets_png/IMG/ico_til.png",
        "../assets_png/IMG/ico_til.png",
        "../port/assets_png/IMG/ico_til.png"
    });
    if (!iconSheetPath.empty()) {
        SDL_Surface *iconSurface = IMG_Load(iconSheetPath.c_str());
        if (iconSurface) {
            iconTexture = SDL_CreateTextureFromSurface(renderer, iconSurface);
            iconTilesPerRow = std::max(1, iconSurface->w / kIconSize);
            SDL_FreeSurface(iconSurface);
        }
    }

    std::string currentMapPath = resolveMapPath(argc, argv);

    MapData map;
    if (!map.loadFromFile(currentMapPath)) {
        clearMap(map);
        std::cout << "Creating new map in memory: " << currentMapPath << '\n';
    } else {
        std::cout << "Loaded map: " << currentMapPath << '\n';
    }

    std::cout << "TILCRE PORT - COMMANDS\n";
    std::cout << " Buttons are clickable like old tool.\n";
    std::cout << " Keyboard shortcuts:\n";
    std::cout << "  ARROWS pan map, ESC exits, S save, O open\n";

    int camX = 0;
    int camY = 0;
    int selectedTile1 = 1;
    int paletteOffsetRow = 0;

    bool running = true;
    bool dirty = false;
    bool copyMode = false;

    bool tilePickMode = false;
    int tilePickTargetCount = 1;
    int tilePickIndex = 0;
    bool showNeighborhoodPreview = true;

    InputState inputState;

    MapObject actual = defaultObject();
    MapObject modificado = defaultObject();
    MapObject pegado = defaultObject();

    actual.tiles[0] = 1;
    modificado.tiles[0] = 1;

    auto openMapByPath = [&](const std::string &path) {
        if (path.empty()) {
            return;
        }
        if (map.loadFromFile(path)) {
            currentMapPath = path;
            camX = 0;
            camY = 0;
            dirty = false;
            std::cout << "Opened: " << currentMapPath << '\n';
        } else {
            std::cout << "Failed to open: " << path << '\n';
        }
    };

    auto requestOpenMap = [&]() {
        std::string pickedPath;
        if (pickMapFileDialog(pickedPath, currentMapPath, window)) {
            openMapByPath(pickedPath);
            return;
        }

#if !defined(_WIN32)
        inputState.buffer.clear();
        inputState.label = "ABRIR: ";
        inputState.maxLen = 64;
        inputState.action = INPUT_OPEN;
        inputState.active = true;
        SDL_StartTextInput();
#endif
    };

    setWindowTitle(window, currentMapPath, camX, camY, dirty, copyMode);

    bool mouseHeld = false;
    while (running) {
        int mouseX = 0;
        int mouseY = 0;
        SDL_GetMouseState(&mouseX, &mouseY);

        std::vector<Button> buttons = buildButtons();

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                break;
            }

            // --- SDL text input mode (overlay prompt) ---
            if (inputState.active) {
                if (e.type == SDL_TEXTINPUT) {
                    for (int i = 0; e.text.text[i] && inputState.buffer.size() < inputState.maxLen; ++i) {
                        char c = static_cast<char>(std::toupper(static_cast<unsigned char>(e.text.text[i])));
                        if (c >= 32 && c < 127) {
                            inputState.buffer += c;
                        }
                    }
                } else if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_ESCAPE) {
                        inputState.active = false;
                        SDL_StopTextInput();
                    } else if (e.key.keysym.sym == SDLK_BACKSPACE && !inputState.buffer.empty()) {
                        inputState.buffer.pop_back();
                    } else if (e.key.keysym.sym == SDLK_RETURN) {
                        switch (inputState.action) {
                            case INPUT_OPEN:
                                openMapByPath(inputState.buffer);
                                inputState.active = false;
                                SDL_StopTextInput();
                                break;
                            case INPUT_NOMBRE:
                                if (!inputState.buffer.empty()) {
                                    modificado.nombre = inputState.buffer;
                                }
                                inputState.active = false;
                                SDL_StopTextInput();
                                break;
                            case INPUT_ARCHIVO:
                                if (!inputState.buffer.empty()) {
                                    modificado.archivo = inputState.buffer;
                                }
                                inputState.active = false;
                                SDL_StopTextInput();
                                break;
                            case INPUT_POS_X:
                                try { inputState.tempX = std::clamp(std::stoi(inputState.buffer), 0, kMapSize - 1); } catch (...) {}
                                inputState.buffer.clear();
                                inputState.label  = "POS Y (0-39): ";
                                inputState.maxLen = 3;
                                inputState.action = INPUT_POS_Y;
                                // stay active — wait for y
                                break;
                            case INPUT_POS_Y:
                                try { modificado.y = std::clamp(std::stoi(inputState.buffer), 0, kMapSize - 1); } catch (...) {}
                                modificado.x = inputState.tempX;
                                inputState.active = false;
                                SDL_StopTextInput();
                                break;
                            default:
                                inputState.active = false;
                                SDL_StopTextInput();
                                break;
                        }
                        setWindowTitle(window, currentMapPath, camX, camY, dirty, copyMode);
                    }
                }
                continue; // eat all other events while input overlay is open
            }

            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_LEFT:
                        camX = std::max(0, camX - 1);
                        break;
                    case SDLK_RIGHT:
                        camX = std::min(kMapSize - kViewTilesX, camX + 1);
                        break;
                    case SDLK_UP:
                        camY = std::max(0, camY - 1);
                        break;
                    case SDLK_DOWN:
                        camY = std::min(kMapSize - kViewTilesY, camY + 1);
                        break;
                    case SDLK_s:
                        if (map.saveToFile(currentMapPath)) {
                            std::cout << "Saved map: " << currentMapPath << '\n';
                            dirty = false;
                        } else {
                            std::cout << "Failed to save map: " << currentMapPath << '\n';
                        }
                        break;
                    case SDLK_o: {
                        requestOpenMap();
                        break;
                    }
                    case SDLK_z:
                        showNeighborhoodPreview = !showNeighborhoodPreview;
                        break;
                    default:
                        break;
                }

                setWindowTitle(window, currentMapPath, camX, camY, dirty, copyMode);
            }

            if (e.type == SDL_MOUSEWHEEL) {
                if (mouseX >= kPaletteX && mouseX < (kPaletteX + kPaletteCols * kPaletteCell) &&
                    mouseY >= kPaletteY && mouseY < (kPaletteY + kPaletteRowsVisible * kPaletteCell)) {
                    paletteOffsetRow = std::max(0, paletteOffsetRow - e.wheel.y);
                    const int maxOffset = std::max(0, tilesPerCol - kPaletteRowsVisible);
                    paletteOffsetRow = std::min(maxOffset, paletteOffsetRow);
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                mouseHeld = true;
                const int x = e.button.x;
                const int y = e.button.y;

                CommandId clicked = CMD_NONE;
                for (const auto &b : buttons) {
                    if (pointInRect(x, y, b.rect)) {
                        clicked = b.id;
                        break;
                    }
                }

                if (clicked != CMD_NONE) {
                    switch (clicked) {
                        case CMD_SAVE:
                            if (map.saveToFile(currentMapPath)) {
                                std::cout << "Saved map: " << currentMapPath << '\n';
                                dirty = false;
                            } else {
                                std::cout << "Failed to save map: " << currentMapPath << '\n';
                            }
                            break;
                        case CMD_OPEN: {
                            requestOpenMap();
                            break;
                        }
                        case CMD_EXIT:
                            running = false;
                            break;
                        case CMD_ACCION:
                            modificado.accion = (modificado.accion + 1) % 5;
                            break;
                        case CMD_ESTADO:
                            modificado.estado = (modificado.estado + 1) % 3;
                            break;
                        case CMD_IMPORTANTE:
                            modificado.importante = 1 - modificado.importante;
                            break;
                        case CMD_SOLIDO:
                            modificado.solido = 1 - modificado.solido;
                            break;
                        case CMD_CICLICO:
                            modificado.ciclico = 1 - modificado.ciclico;
                            break;
                        case CMD_TILES: {
                            tilePickTargetCount = (tilePickTargetCount % 3) + 1;
                            tilePickIndex = 0;
                            tilePickMode = true;
                            modificado.tiles = {0, 0, 0};
                            std::cout << "Tile pick mode: click " << tilePickTargetCount << " tile(s) in palette.\n";
                            break;
                        }
                        case CMD_POSICION: {
                            inputState.buffer.clear();
                            inputState.label = "POS X (0-39): ";
                            inputState.maxLen = 3;
                            inputState.action = INPUT_POS_X;
                            inputState.tempX  = modificado.x;
                            inputState.active = true;
                            SDL_StartTextInput();
                            break;
                        }
                        case CMD_NOMBRE: {
                            inputState.buffer = modificado.nombre == "NULL" ? "" : modificado.nombre;
                            inputState.label  = "NOMBRE (max 8): ";
                            inputState.maxLen = 8;
                            inputState.action = INPUT_NOMBRE;
                            inputState.active = true;
                            SDL_StartTextInput();
                            break;
                        }
                        case CMD_ARCHIVO: {
                            inputState.buffer = modificado.archivo == "NULL" ? "" : modificado.archivo;
                            inputState.label  = "ARCHIVO (max 9): ";
                            inputState.maxLen = 9;
                            inputState.action = INPUT_ARCHIVO;
                            inputState.active = true;
                            SDL_StartTextInput();
                            break;
                        }
                        case CMD_COPY_MODE:
                            copyMode = true;
                            break;
                        case CMD_USE_CLIPBOARD:
                            actual = pegado;
                            copyMode = false;
                            break;
                        case CMD_USE_MODIFIED:
                            actual = modificado;
                            copyMode = false;
                            break;
                        case CMD_FILL:
                            for (int yy = 0; yy < kMapSize; ++yy) {
                                for (int xx = 0; xx < kMapSize; ++xx) {
                                    map.atMutable(xx, yy) = actual;
                                }
                            }
                            dirty = true;
                            break;
                        case CMD_CLONE:
                            modificado = actual;
                            break;
                        default:
                            break;
                    }

                    setWindowTitle(window, currentMapPath, camX, camY, dirty, copyMode);
                    continue;
                }

                // Palette click
                if (x >= kPaletteX && x < (kPaletteX + kPaletteCols * kPaletteCell) &&
                    y >= kPaletteY && y < (kPaletteY + kPaletteRowsVisible * kPaletteCell)) {
                    const int col = (x - kPaletteX) / kPaletteCell;
                    const int row = (y - kPaletteY) / kPaletteCell;
                    const int tileX = col;
                    const int tileY = row + paletteOffsetRow;
                    const int tileIdx0 = tileY * tilesPerRow + tileX;
                    if (tileIdx0 >= 0 && tileIdx0 < tileCount) {
                        selectedTile1 = tileIdx0 + 1;

                        if (tilePickMode) {
                            modificado.tiles[tilePickIndex] = selectedTile1;
                            tilePickIndex++;
                            if (tilePickIndex >= tilePickTargetCount) {
                                tilePickMode = false;
                                std::cout << "Tile pick completed.\n";
                            }
                        }
                    }
                    continue;
                }

                // Map click -> CMD_MAP_CLICK
                if (x >= 0 && x < kMapViewW && y >= 0 && y < kMapViewH) {
                    const int tx = camX + (x / kCellPx);
                    const int ty = camY + (y / kCellPx);
                    if (tx >= 0 && tx < kMapSize && ty >= 0 && ty < kMapSize) {
                        if (!copyMode) {
                            map.atMutable(tx, ty) = actual;
                            dirty = true;
                        } else {
                            pegado = map.at(tx, ty);
                            actual = pegado;
                            copyMode = false;
                        }
                    }
                    setWindowTitle(window, currentMapPath, camX, camY, dirty, copyMode);
                }
            }

            // Soporte para click sostenido (drag/hold) en el grid
            if ((e.type == SDL_MOUSEMOTION && (e.motion.state & SDL_BUTTON_LMASK)) ||
                (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)) {
                // Usar la posición actual del mouse
                int x = (e.type == SDL_MOUSEMOTION) ? e.motion.x : e.button.x;
                int y = (e.type == SDL_MOUSEMOTION) ? e.motion.y : e.button.y;

                // Solo pintar si el mouse está sobre el grid
                if (x >= 0 && x < kMapViewW && y >= 0 && y < kMapViewH) {
                    const int tx = camX + (x / kCellPx);
                    const int ty = camY + (y / kCellPx);
                    if (tx >= 0 && tx < kMapSize && ty >= 0 && ty < kMapSize) {
                        if (!copyMode) {
                            map.atMutable(tx, ty) = actual;
                            dirty = true;
                        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                            pegado = map.at(tx, ty);
                            actual = pegado;
                            copyMode = false;
                        }
                    }
                    setWindowTitle(window, currentMapPath, camX, camY, dirty, copyMode);
                }
            }

            if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                mouseHeld = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, kColBg.r, kColBg.g, kColBg.b, kColBg.a);
        SDL_RenderClear(renderer);

        // Map view
        for (int vy = 0; vy < kViewTilesY; ++vy) {
            for (int vx = 0; vx < kViewTilesX; ++vx) {
                const int mx = camX + vx;
                const int my = camY + vy;
                const MapObject &o = map.at(mx, my);

                SDL_Rect dst{vx * kCellPx, vy * kCellPx, kCellPx, kCellPx};
                const int tile1 = o.tiles[0];
                if (tile1 > 0 && tile1 <= tileCount) {
                    const SDL_Rect src = tileSrcRect(tile1 - 1, tilesPerRow);
                    SDL_RenderCopy(renderer, tileTexture, &src, &dst);
                } else {
                    SDL_SetRenderDrawColor(renderer, kColPanelFill.r, kColPanelFill.g, kColPanelFill.b, kColPanelFill.a);
                    SDL_RenderFillRect(renderer, &dst);
                }

                SDL_SetRenderDrawColor(renderer, 58, 82, 122, 255);
                SDL_RenderDrawRect(renderer, &dst);

                if (pointInRect(mouseX, mouseY, dst)) {
                    SDL_SetRenderDrawColor(renderer, 255, 220, 64, 255);
                    SDL_RenderDrawRect(renderer, &dst);
                }
            }
        }

        // Buttons
        for (const auto &b : buttons) {
            bool hovered = pointInRect(mouseX, mouseY, b.rect);
            bool active = (b.id == CMD_COPY_MODE && copyMode);
            drawButton(renderer, b, hovered, active);
        }

        // Icon sprites for save/open/exit buttons (legacy ICO_TIL look).
        if (iconTexture && iconTilesPerRow > 0) {
            const struct { CommandId id; int frame; } kIconMap[] = {
                {CMD_SAVE, 4},
                {CMD_OPEN, 5},
                {CMD_EXIT, 7}
            };
            for (const auto &it : kIconMap) {
                const Button *btn = findButton(buttons, it.id);
                if (!btn) {
                    continue;
                }
                SDL_Rect src{(it.frame % iconTilesPerRow) * kIconSize, (it.frame / iconTilesPerRow) * kIconSize, kIconSize, kIconSize};
                SDL_Rect dst{btn->rect.x + btn->rect.w - kIconSize - 2, btn->rect.y + 2, kIconSize, kIconSize};
                SDL_RenderCopy(renderer, iconTexture, &src, &dst);
            }
        }

        // 5x5 neighborhood preview — only shown when mouse is over map (matches original TILCRE.CPP)
        if (showNeighborhoodPreview &&
            mouseX >= 0 && mouseX < kMapViewW &&
            mouseY >= 0 && mouseY < kMapViewH) {

            const int centerX = camX + (mouseX / kCellPx);
            const int centerY = camY + (mouseY / kCellPx);

            // Draw below the palette, same column
            const int previewX = kPaletteX;
            const int previewY = kPaletteY + kPaletteRowsVisible * kPaletteCell + 18;
            const int pCell = 16; // match original 16px tile size
            const int pN    = 5;  // 5x5 exactly as in original

            SDL_Rect pBox{previewX - 2, previewY - 18, pCell * pN + 4, 14};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &pBox);
            drawText(renderer, previewX - 2, previewY - 16, "VISTA 5X5 [Z]", kColTitle, 1);

            for (int py = 0; py < pN; ++py) {
                for (int px = 0; px < pN; ++px) {
                    const int mx = centerX + px - 2;
                    const int my = centerY + py - 2;
                    SDL_Rect dst{previewX + px * pCell, previewY + py * pCell, pCell, pCell};
                    if (mx >= 0 && mx < kMapSize && my >= 0 && my < kMapSize) {
                        const int tile1 = map.at(mx, my).tiles[0];
                        if (tile1 > 0 && tile1 <= tileCount) {
                            SDL_Rect src = tileSrcRect(tile1 - 1, tilesPerRow);
                            SDL_RenderCopy(renderer, tileTexture, &src, &dst);
                        } else {
                            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                            SDL_RenderFillRect(renderer, &dst);
                        }
                    } else {
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        SDL_RenderFillRect(renderer, &dst);
                    }
                }
            }
            // Highlight center tile — matches original: h.box(start_x+32, 32, start_x+48, 48, 67)
            SDL_Rect centerRect{previewX + 2 * pCell, previewY + 2 * pCell, pCell, pCell};
            SDL_SetRenderDrawColor(renderer, 244, 148, 64, 255);
            SDL_RenderDrawRect(renderer, &centerRect);
        }

        // Info panels
        // Info panels: 2×2 grid in bottom-left zone (below map view)
        drawObjectPanel(renderer, "ACTUAL",     actual,    kInfoCol1X, kInfoRow1Y, SDL_Color{44, 160, 84, 255},  tileTexture, tileCount, tilesPerRow);
        drawObjectPanel(renderer, "MODIFICADO", modificado, kInfoCol1X, kInfoRow2Y, SDL_Color{112, 164, 216, 255}, tileTexture, tileCount, tilesPerRow);
        drawObjectPanel(renderer, "PEGADO",     pegado,    kInfoCol1X, kInfoRow3Y, SDL_Color{255, 140, 64, 255}, tileTexture, tileCount, tilesPerRow);

        // Hover object details
        if (mouseX >= 0 && mouseX < kMapViewW && mouseY >= 0 && mouseY < kMapViewH) {
            const int tx = camX + (mouseX / kCellPx);
            const int ty = camY + (mouseY / kCellPx);
            const MapObject &hovered = map.at(tx, ty);
            drawObjectPanel(renderer, "MOUSE OVER", hovered, kHoverPanelX, kHoverPanelY, SDL_Color{255, 220, 64, 255}, tileTexture, tileCount, tilesPerRow);
        }

        // Palette — clamp visible rows to actual rows in the sheet
        const int paletteRowsDraw = std::min(kPaletteRowsVisible, tilesPerCol - paletteOffsetRow);
        SDL_Rect palRect{kPaletteX - 2, kPaletteY - 2, kPaletteCols * kPaletteCell + 4, paletteRowsDraw * kPaletteCell + 4};
        SDL_SetRenderDrawColor(renderer, kColPanelBorder.r, kColPanelBorder.g, kColPanelBorder.b, kColPanelBorder.a);
        SDL_RenderDrawRect(renderer, &palRect);

        for (int row = 0; row < paletteRowsDraw; ++row) {
            for (int col = 0; col < kPaletteCols; ++col) {
                const int tileX = col;
                const int tileY = row + paletteOffsetRow;
                const int idx0 = tileY * tilesPerRow + tileX;
                if (idx0 < 0 || idx0 >= tileCount) {
                    continue;
                }

                SDL_Rect dst{kPaletteX + col * kPaletteCell, kPaletteY + row * kPaletteCell, kPaletteCell, kPaletteCell};
                SDL_Rect src = tileSrcRect(idx0, tilesPerRow);
                SDL_RenderCopy(renderer, tileTexture, &src, &dst);

                if (idx0 + 1 == selectedTile1) {
                    SDL_SetRenderDrawColor(renderer, 255, 220, 64, 255);
                    SDL_RenderDrawRect(renderer, &dst);
                    SDL_Rect inner{dst.x + 1, dst.y + 1, dst.w - 2, dst.h - 2};
                    SDL_RenderDrawRect(renderer, &inner);
                }
            }
        }

        drawText(renderer, kPaletteX, kPaletteY - 16,
                 (tilePickMode ? std::string("PICK TILE  R") : std::string("PALETA     R")) + std::to_string(paletteOffsetRow),
                 kColTitle, 2);

        // Tile slots previews (right panel, below action buttons)
        drawText(renderer, kPanelX, 398, tilePickMode
                 ? "TILES [PICK: " + std::to_string(tilePickTargetCount - tilePickIndex) + " LEFT]"
                 : "TILES:", kColTitle, 2);
        for (int i = 0; i < 3; ++i) {
            SDL_Rect slot{kPanelX + i * 38, 414, 34, 34};
            SDL_SetRenderDrawColor(renderer, kColPanelBorder.r, kColPanelBorder.g, kColPanelBorder.b, kColPanelBorder.a);
            SDL_RenderDrawRect(renderer, &slot);

            int t = modificado.tiles[i];
            if (t > 0 && t <= tileCount) {
                SDL_Rect src = tileSrcRect(t - 1, tilesPerRow);
                SDL_RenderCopy(renderer, tileTexture, &src, &slot);
            }

            drawText(renderer, slot.x + 12, slot.y + 36, std::to_string(i + 1), kColText, 2);
        }

        // Input overlay (drawn last so it sits on top of everything)
        drawInputOverlay(renderer, inputState);

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    SDL_DestroyTexture(tileTexture);
    if (iconTexture) {
        SDL_DestroyTexture(iconTexture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
