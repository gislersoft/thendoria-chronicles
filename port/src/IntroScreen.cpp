#include "IntroScreen.h"
#include <SDL_image.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono>

IntroScreen::IntroScreen() {}

IntroScreen::~IntroScreen() {}

std::string IntroScreen::resolvePath(const std::string &relPath) {
    std::vector<std::string> candidates = {
        relPath,
        "../" + relPath,
        "port/" + relPath,
        "port/../" + relPath
    };

    for (const auto &c : candidates) {
        if (std::filesystem::exists(c)) {
            return c;
        }
    }
    return std::string();
}

bool IntroScreen::loadImageToBuffer(const std::string &path, SDL_Renderer *renderer, GraphCompat &graph) {
    std::string resolvedPath = resolvePath(path);
    if (resolvedPath.empty()) {
        std::cerr << "Could not find image: " << path << '\n';
        return false;
    }

    SDL_Surface *img = IMG_Load(resolvedPath.c_str());
    if (!img) {
        std::cerr << "Failed to load image " << resolvedPath << ": " << IMG_GetError() << '\n';
        return false;
    }

    // Convert to 320x200 if needed and ensure RGBA format
    SDL_Surface *scaled = nullptr;
    if (img->w != 320 || img->h != 200) {
        scaled = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_ARGB8888, 0);
        SDL_Surface *final = SDL_CreateRGBSurface(0, 320, 200, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
        if (scaled && final) {
            SDL_Rect src = {0, 0, scaled->w, scaled->h};
            SDL_Rect dst = {0, 0, 320, 200};
            SDL_BlitScaled(scaled, &src, final, &dst);
            SDL_FreeSurface(scaled);
            scaled = final;
        } else if (final) {
            SDL_FreeSurface(final);
        }
        SDL_FreeSurface(img);
        img = scaled;
    } else {
        scaled = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_ARGB8888, 0);
        SDL_FreeSurface(img);
        img = scaled;
    }

    if (!img) {
        return false;
    }

    // Copy RGBA pixels to overlay buffer
    uint32_t *pixels = static_cast<uint32_t *>(img->pixels);
    uint32_t *overlay = graph.getOverlay();
    if (overlay && pixels) {
        std::copy(pixels, pixels + (320 * 200), overlay);
        // Guardar la imagen original para fades
        imageBuffer_.resize(320 * 200);
        std::copy(pixels, pixels + (320 * 200), imageBuffer_.begin());
    }

    SDL_FreeSurface(img);
    return true;
}

void IntroScreen::fadeIn(GraphCompat &graph, int steps) {
    // Fade in from black: interpolate from black to original image
    uint32_t *overlay = graph.getOverlay();
    
    if (!overlay || imageBuffer_.empty()) {
        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
        return;
    }

    for (int step = 0; step <= steps; ++step) {
        // Calculate blend factor: 0 at start (black), 1 at end (original image)
        float blend = static_cast<float>(step) / static_cast<float>(steps);
        
        // Interpolate between black (0,0,0) and original image
        for (int i = 0; i < 320 * 200; ++i) {
            uint32_t orig = imageBuffer_[i];
            
            uint8_t origA = (orig >> 24) & 0xFF;
            uint8_t origR = (orig >> 16) & 0xFF;
            uint8_t origG = (orig >> 8) & 0xFF;
            uint8_t origB = orig & 0xFF;
            
            // Blend: start black, end original
            uint8_t outR = static_cast<uint8_t>(origR * blend);
            uint8_t outG = static_cast<uint8_t>(origG * blend);
            uint8_t outB = static_cast<uint8_t>(origB * blend);
            uint8_t outA = origA; // Keep original alpha
            
            overlay[i] = (outA << 24) | (outR << 16) | (outG << 8) | outB;
        }
        
        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

void IntroScreen::fadeOut(GraphCompat &graph, int steps) {
    // Fade out to black: interpolate from original image to black
    uint32_t *overlay = graph.getOverlay();
    
    if (!overlay || imageBuffer_.empty()) {
        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
        return;
    }

    for (int step = 0; step <= steps; ++step) {
        // Calculate blend factor: 1 at start (original), 0 at end (black)
        float blend = 1.0f - (static_cast<float>(step) / static_cast<float>(steps));
        
        // Interpolate between original image and black (0,0,0)
        for (int i = 0; i < 320 * 200; ++i) {
            uint32_t orig = imageBuffer_[i];
            
            uint8_t origA = (orig >> 24) & 0xFF;
            uint8_t origR = (orig >> 16) & 0xFF;
            uint8_t origG = (orig >> 8) & 0xFF;
            uint8_t origB = orig & 0xFF;
            
            // Blend: start original, end black
            uint8_t outR = static_cast<uint8_t>(origR * blend);
            uint8_t outG = static_cast<uint8_t>(origG * blend);
            uint8_t outB = static_cast<uint8_t>(origB * blend);
            uint8_t outA = origA; // Keep original alpha
            
            overlay[i] = (outA << 24) | (outR << 16) | (outG << 8) | outB;
        }
        
        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    
    // Clear overlay at the end
    graph.clearOverlay();
}

void IntroScreen::playSound(int frequency, int durationMs) {
    // Placeholder: SDL_mixer or similar would be used here for actual sound
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
}

void IntroScreen::waitForKey() {
    bool waiting = true;
    bool keyWasDown = false;

    while (waiting) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    exit(0);
                }
                if (event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN) {
                    waiting = false;
                    break;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void IntroScreen::showLogoGislersoft(GraphCompat &graph, SDL_Renderer *renderer) {
    graph.clr(graph.pv1, 0);
    graph.clr(graph.pv2, 0);
    graph.clearOverlay();

    if (!loadImageToBuffer("port/assets_png/IMG/gsoft.png", renderer, graph)) {
        std::cerr << "Failed to load gsoft.png, skipping\n";
        return;
    }

    fadeIn(graph, 30);

    // Play sounds
    playSound(440, 500);
    playSound(350, 300);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    playSound(440, 500);
    playSound(293, 300);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    fadeOut(graph, 30);
}

void IntroScreen::showIntroScreen(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer) {
    graph.clr(graph.pv1, 0);
    graph.clr(graph.pv2, 0);
    graph.clearOverlay();

    if (!loadImageToBuffer("port/assets_png/IMG/intro.png", renderer, graph)) {
        std::cerr << "Failed to load intro.png, skipping\n";
        return;
    }

    fadeIn(graph, 25);

    // Play sounds
    playSound(262, 500);
    playSound(247, 500);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    playSound(350, 500);
    playSound(247, 500);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    playSound(220, 1000);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    font.putstr(graph.pv2, 70, 180, "PRESIONE CUALQUIER TECLA PARA CONTINUAR...", graph, 0, 15);
    graph.wait_retrace();
    graph.presentLayers(graph.vga, graph.pv2);

    waitForKey();
    fadeOut(graph, 50);
}

void IntroScreen::showAboutScreen(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer) {
    graph.clr(graph.pv1, 0);
    graph.clr(graph.pv2, 0);
    graph.clearOverlay();

    if (!loadImageToBuffer("port/assets_png/IMG/lostgame.png", renderer, graph)) {
        std::cerr << "Failed to load lostgame.png, skipping\n";
        return;
    }

    fadeIn(graph, 25);

    playSound(262, 300);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Load and display dialog
    std::vector<std::string> dialogLines;
    if (loadDialogFile("DIALOGS/LIN02.TXT", dialogLines)) {
        drawDialogBox(graph, font, dialogLines, 0, 5);
        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
    }

    waitForKey();
    fadeOut(graph, 50);
}

void IntroScreen::showHistoriaParte1(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer) {
    graph.clr(graph.pv1, 0);
    graph.clr(graph.pv2, 0);
    graph.clearOverlay();

    if (!loadImageToBuffer("port/assets_png/IMG/histo1.png", renderer, graph)) {
        std::cerr << "Failed to load histo1.png, skipping\n";
        return;
    }

    fadeIn(graph, 25);

    std::vector<std::string> dialogLines;
    
    // First dialog: LIN03.TXT
    if (loadDialogFile("DIALOGS/LIN03.TXT", dialogLines)) {
        drawDialogBox(graph, font, dialogLines, 0, 5);
        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
    }

    waitForKey();

    // Second dialog: LIN04.TXT
    dialogLines.clear();
    if (loadDialogFile("DIALOGS/LIN04.TXT", dialogLines)) {
        graph.clr(graph.pv2, 0);
        drawDialogBox(graph, font, dialogLines, 0, 5);
        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
    }

    waitForKey();
    fadeOut(graph, 50);
}

void IntroScreen::showHistoriaParte2(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer) {
    graph.clr(graph.pv1, 0);
    graph.clr(graph.pv2, 0);
    graph.clearOverlay();

    if (!loadImageToBuffer("port/assets_png/IMG/histo2.png", renderer, graph)) {
        std::cerr << "Failed to load histo2.png, skipping\n";
        return;
    }

    fadeIn(graph, 25);

    std::vector<std::string> dialogLines;
    
    // First dialog: LIN05.TXT
    if (loadDialogFile("DIALOGS/LIN05.TXT", dialogLines)) {
        drawDialogBox(graph, font, dialogLines, 0, 5);
        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
    }

    waitForKey();

    // Second dialog: LIN06.TXT
    dialogLines.clear();
    if (loadDialogFile("DIALOGS/LIN06.TXT", dialogLines)) {
        graph.clr(graph.pv2, 0);
        drawDialogBox(graph, font, dialogLines, 0, 5);
        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
    }

    waitForKey();

    // Third dialog: LIN07.TXT
    dialogLines.clear();
    if (loadDialogFile("DIALOGS/LIN07.TXT", dialogLines)) {
        graph.clr(graph.pv2, 0);
        drawDialogBox(graph, font, dialogLines, 0, 5);
        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
    }

    waitForKey();

    // Fourth dialog: LIN08.TXT
    dialogLines.clear();
    if (loadDialogFile("DIALOGS/LIN08.TXT", dialogLines)) {
        graph.clr(graph.pv2, 0);
        drawDialogBox(graph, font, dialogLines, 0, 5);
        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
    }

    waitForKey();

    fadeOut(graph, 50);
}

bool IntroScreen::loadDialogFile(const std::string &filename, std::vector<std::string> &out) {
    std::string resolved = resolvePath(filename);
    if (resolved.empty()) {
        std::cerr << "Could not find dialog file: " << filename << '\n';
        return false;
    }

    std::ifstream file(resolved);
    if (!file.is_open()) {
        std::cerr << "Failed to open dialog file: " << resolved << '\n';
        return false;
    }

    out.clear();
    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (!line.empty()) {
            out.push_back(line);
        }
    }
    file.close();
    return true;
}

void IntroScreen::drawDialogBox(GraphCompat &graph, FontCompat &font, const std::vector<std::string> &lines, int startLine, int lineCount) {
    constexpr int kDialogMaxChars = 40;
    constexpr int kScreenWidth = 320;
    constexpr int kDialogTop = 140;
    constexpr int kCharStepPx = 6;
    constexpr int kInnerPad = 6;

    std::vector<std::string> pageLines;
    pageLines.reserve(static_cast<std::size_t>(lineCount));
    int maxChars = 0;

    for (int i = 0; i < lineCount; ++i) {
        int idx = startLine + i;
        std::string lineText;
        if (idx >= 0 && idx < static_cast<int>(lines.size())) {
            lineText = lines[idx];
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

    // Draw dialog box background
    graph.fillbox(graph.pv2, boxX1, boxY1, boxX2, boxY2, 230);
    graph.box(graph.pv2, boxX1, boxY1, boxX2, boxY2, 15);

    // Draw text lines
    bool plusMode = false;
    bool minusMode = false;
    for (int i = 0; i < lineCount; ++i) {
        const std::string &lineText = pageLines[static_cast<std::size_t>(i)];
        if (!lineText.empty()) {
            if (i == 0) {
                font.putstrWithState(graph.pv2, boxX1 + kInnerPad, boxY1 + kInnerPad + i * 8, lineText, graph, 230, 46, plusMode, minusMode);
            } else if (i == lineCount - 1) {
                font.putstrWithState(graph.pv2, boxX1 + kInnerPad, boxY1 + kInnerPad + i * 8, lineText, graph, 230, 32, plusMode, minusMode);
            } else {
                font.putstrTexturedWithState(graph.pv2, boxX1 + kInnerPad, boxY1 + kInnerPad + i * 8, lineText, graph, 230, 3, plusMode, minusMode);
            }
        }
    }
}

void IntroScreen::playFullIntro(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer) {
    showLogoGislersoft(graph, renderer);
    showIntroScreen(graph, font, renderer);
    showAboutScreen(graph, font, renderer);
    showHistoriaParte1(graph, font, renderer);
    showHistoriaParte2(graph, font, renderer);
}
