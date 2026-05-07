#include "IntroScreen.h"
#include <SDL_image.h>
#if defined(THENDORIA_HAVE_SDL_MIXER)
#include <SDL_mixer.h>
#endif
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cmath>
#include <random>

namespace {

std::uint32_t alphaBlendOver(std::uint32_t dst, std::uint32_t src) {
    const int srcA = static_cast<int>((src >> 24) & 0xFF);
    if (srcA <= 0) {
        return dst;
    }
    if (srcA >= 255) {
        return src;
    }

    const int dstA = static_cast<int>((dst >> 24) & 0xFF);
    const int srcR = static_cast<int>((src >> 16) & 0xFF);
    const int srcG = static_cast<int>((src >> 8) & 0xFF);
    const int srcB = static_cast<int>(src & 0xFF);
    const int dstR = static_cast<int>((dst >> 16) & 0xFF);
    const int dstG = static_cast<int>((dst >> 8) & 0xFF);
    const int dstB = static_cast<int>(dst & 0xFF);

    const int invSrcA = 255 - srcA;
    const int outA = srcA + ((dstA * invSrcA) / 255);
    if (outA <= 0) {
        return 0;
    }

    const int outR = (srcR * srcA + dstR * invSrcA) / 255;
    const int outG = (srcG * srcA + dstG * invSrcA) / 255;
    const int outB = (srcB * srcA + dstB * invSrcA) / 255;

    return (static_cast<std::uint32_t>(outA) << 24)
        | (static_cast<std::uint32_t>(outR) << 16)
        | (static_cast<std::uint32_t>(outG) << 8)
        | static_cast<std::uint32_t>(outB);
}

} // namespace

#if defined(THENDORIA_HAVE_SDL_MIXER)
// Game Boy DMG speaker low-pass filter (~8kHz cutoff, one-pole IIR).
// alpha = 1 - exp(-2*pi*8000/32768) ≈ 0.784
// Applied via Mix_SetPostMix when gameboy audio mode is active.
static float s_gbLpfState = 0.0f;

static void gbPostMixCB(void* /*udata*/, Uint8* stream, int len) {
    static constexpr float kAlpha    = 0.784f;
    static constexpr float kOneAlpha = 1.0f - kAlpha;
    for (int i = 0; i < len; ++i) {
        float s = static_cast<float>(stream[i]) - 128.0f;  // U8 center at 128
        s_gbLpfState = kAlpha * s + kOneAlpha * s_gbLpfState;
        const int out = static_cast<int>(s_gbLpfState + 128.5f);
        stream[i] = static_cast<Uint8>(out < 0 ? 0 : (out > 255 ? 255 : out));
    }
}
#endif

void IntroScreen::setupGameboyPostMix() {
#if defined(THENDORIA_HAVE_SDL_MIXER)
    s_gbLpfState = 0.0f;
    Mix_SetPostMix(gbPostMixCB, nullptr);
#endif
}

IntroScreen::IntroScreen(bool monoAudio) : monoAudio_(monoAudio) {}

IntroScreen::~IntroScreen() {
    stopIntroMusic();
}

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

bool IntroScreen::loadImageToBuffer(const std::string &path, SDL_Renderer *renderer, GraphCompat &graph, bool blendOverExisting) {
    (void)renderer;
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
        constexpr int kPixelCount = 320 * 200;
        if (!blendOverExisting || imageBuffer_.size() != static_cast<std::size_t>(kPixelCount)) {
            imageBuffer_.resize(kPixelCount);
            std::copy(pixels, pixels + kPixelCount, imageBuffer_.begin());
            std::copy(imageBuffer_.begin(), imageBuffer_.end(), overlay);
        } else {
            for (int i = 0; i < kPixelCount; ++i) {
                imageBuffer_[static_cast<std::size_t>(i)] = alphaBlendOver(imageBuffer_[static_cast<std::size_t>(i)], pixels[i]);
                overlay[i] = imageBuffer_[static_cast<std::size_t>(i)];
            }
        }
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
    (void)frequency;
    // Placeholder: SDL_mixer or similar would be used here for actual sound
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
}

bool IntroScreen::startIntroMusic(const std::string &path) {
#if defined(THENDORIA_HAVE_SDL_MIXER)
    stopIntroMusic();

    std::string resolvedPath = resolvePath(path);
    if (resolvedPath.empty()) {
        std::cerr << "Could not find intro music: " << path << '\n';
        return false;
    }

    if (!mixerReady_) {
        const int   audioFreq     = monoAudio_ ? 32768 : 44100;
        const Uint16 audioFormat  = monoAudio_ ? AUDIO_U8 : MIX_DEFAULT_FORMAT;
        const int   audioChannels = monoAudio_ ? 1 : 2;
        if (Mix_OpenAudio(audioFreq, audioFormat, audioChannels, 1024) != 0) {
            std::cerr << "Mix_OpenAudio failed: " << Mix_GetError() << '\n';
            return false;
        }
        mixerReady_ = true;
        if (monoAudio_) {
            setupGameboyPostMix();
        }
    }

    introMusic_ = Mix_LoadMUS(resolvedPath.c_str());
    if (!introMusic_) {
        std::cerr << "Mix_LoadMUS failed for " << resolvedPath << ": " << Mix_GetError() << '\n';
        return false;
    }

    if (Mix_PlayMusic(introMusic_, -1) != 0) {
        std::cerr << "Mix_PlayMusic failed: " << Mix_GetError() << '\n';
        Mix_FreeMusic(introMusic_);
        introMusic_ = nullptr;
        return false;
    }

    introMusicPlaying_ = true;
    return true;
#else
    std::cerr << "[AUDIO-DBG] startIntroMusic(\"" << path << "\") — SDL_mixer no compilado, musica no disponible.\n";
    (void)path;
    return false;
#endif
}

void IntroScreen::stopIntroMusic() {
#if defined(THENDORIA_HAVE_SDL_MIXER)
    if (introMusicPlaying_) {
        Mix_HaltMusic();
        introMusicPlaying_ = false;
    }

    if (introMusic_) {
        Mix_FreeMusic(introMusic_);
        introMusic_ = nullptr;
    }
#endif
}

void IntroScreen::waitForKey(bool anyKey) {
    bool waiting = true;

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
                if (anyKey || event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN) {
                    waiting = false;
                    break;
                }
            }
            if (event.type == SDL_JOYBUTTONDOWN) {
                waiting = false;
                break;
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
    (void)renderer;
    graph.clr(graph.pv1, 0);
    graph.clr(graph.pv2, 0);
    graph.clearOverlay();

    auto loadImagePixels = [&](const std::string &path, std::vector<std::uint32_t> &out) -> bool {
        const std::string resolvedPath = resolvePath(path);
        if (resolvedPath.empty()) {
            return false;
        }

        SDL_Surface *img = IMG_Load(resolvedPath.c_str());
        if (!img) {
            return false;
        }

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

        out.resize(320 * 200);
        std::copy(static_cast<std::uint32_t *>(img->pixels), static_cast<std::uint32_t *>(img->pixels) + (320 * 200), out.begin());
        SDL_FreeSurface(img);
        return true;
    };

    std::vector<std::uint32_t> bgPixels;
    std::vector<std::uint32_t> titlePixels;
    const bool hasSeamlessBackground = loadImagePixels("port/assets_png/IMG/seamlessintrobg.png", bgPixels);
    const bool hasTitleOverlay = loadImagePixels("port/assets_png/IMG/intro.png", titlePixels);

    if (!hasSeamlessBackground && !hasTitleOverlay) {
        std::cerr << "Failed to load seamlessintrobg.png and intro.png, skipping\n";
        return;
    }

    if (!hasSeamlessBackground) {
        bgPixels = titlePixels;
    }

    if (hasSeamlessBackground && !hasTitleOverlay) {
        std::cerr << "Failed to load intro.png overlay, using seamlessintrobg.png only\n";
    }

    // Seed fadeIn with the correct composed intro frame so previous screens
    // (e.g. gsoft logo) are never reused during this transition.
    imageBuffer_.resize(320 * 200);
    uint32_t *overlay = graph.getOverlay();
    for (int y = 0; y < 200; ++y) {
        for (int x = 0; x < 320; ++x) {
            std::uint32_t pixel = bgPixels[static_cast<std::size_t>(y * 320 + x)];
            if (hasTitleOverlay) {
                pixel = alphaBlendOver(pixel, titlePixels[static_cast<std::size_t>(y * 320 + x)]);
            }
            imageBuffer_[static_cast<std::size_t>(y * 320 + x)] = pixel;
            if (overlay) {
                overlay[y * 320 + x] = pixel;
            }
        }
    }

    fadeIn(graph, 25);

    const bool joystickPresent = SDL_NumJoysticks() > 0;
    const char *prompt = joystickPresent
        ? "PRESIONE CUALQUIER BOTON PARA CONTINUAR..."
        : "PRESIONE CUALQUIER TECLA PARA CONTINUAR...";
    const std::uint32_t introStartTicks = SDL_GetTicks();
    std::uint32_t lastDirectionChangeTicks = introStartTicks;
    bool waiting = true;
    int offsetX = 0;
    int offsetY = 0;

    std::mt19937 rng(static_cast<std::uint32_t>(introStartTicks));
    std::uniform_int_distribution<int> dirDist(-1, 1);
    std::uniform_int_distribution<int> changeMsDist(1200, 2600);
    int dirX = 1;
    int dirY = 0;
    int nextDirectionChangeMs = changeMsDist(rng);

    bool musicStarted = introMusicPlaying_;
    bool musicStartAttempted = musicStarted;

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
                waiting = false;
                break;
            }
            if (event.type == SDL_JOYBUTTONDOWN) {
                waiting = false;
                break;
            }
        }

        const std::uint32_t nowTicks = SDL_GetTicks();
        if (!musicStartAttempted && (nowTicks - appLaunchTicks_) >= 3000U) {
            musicStarted = startIntroMusic("sound/music/introSong.mp3");
            musicStartAttempted = true;
        }
        if (static_cast<int>(nowTicks - lastDirectionChangeTicks) >= nextDirectionChangeMs) {
            int newDirX = 0;
            int newDirY = 0;
            while (newDirX == 0 && newDirY == 0) {
                newDirX = dirDist(rng);
                newDirY = dirDist(rng);
            }
            dirX = newDirX;
            dirY = newDirY;
            lastDirectionChangeTicks = nowTicks;
            nextDirectionChangeMs = changeMsDist(rng);
        }

        offsetX = (offsetX + dirX + 320) % 320;
        offsetY = (offsetY + dirY + 200) % 200;

        overlay = graph.getOverlay();
        if (overlay) {
            imageBuffer_.resize(320 * 200);
            for (int y = 0; y < 200; ++y) {
                const int srcY = (y + offsetY) % 200;
                for (int x = 0; x < 320; ++x) {
                    const int srcX = (x + offsetX) % 320;
                    std::uint32_t pixel = bgPixels[static_cast<std::size_t>(srcY * 320 + srcX)];
                    if (hasTitleOverlay) {
                        pixel = alphaBlendOver(pixel, titlePixels[static_cast<std::size_t>(y * 320 + x)]);
                    }
                    imageBuffer_[static_cast<std::size_t>(y * 320 + x)] = pixel;
                    overlay[y * 320 + x] = pixel;
                }
            }
        }

        const float t = static_cast<float>(nowTicks - introStartTicks) * 0.001f;
        const float pulse = 0.5f * (std::sinf(t * 7.0f) + 1.0f);
        const int bounce = static_cast<int>(std::sinf(t * 4.0f) * 4.0f);

        constexpr unsigned char kPromptWhite = 15;
        constexpr unsigned char kPromptYellow = 46;
        const unsigned char mainColor = (pulse > 0.5f) ? kPromptWhite : kPromptYellow;
        const unsigned char mainColorU8 = static_cast<unsigned char>(mainColor);
        const int baseY = 178 + bounce;
        const int x = 42;

        graph.clr(graph.pv2, 0);

        // Blink only between yellow and white.
        font.putstr(graph.pv2, x, baseY, prompt, graph, 0, mainColorU8);

        if (!musicStarted) {
            font.putstr(graph.pv2, 82, 190, "MUSICA INTRO NO DISPONIBLE", graph, 0, 12);
        }

        graph.wait_retrace();
        graph.presentLayers(graph.vga, graph.pv2);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    stopIntroMusic();
    fadeOut(graph, 50);
}

void IntroScreen::showAboutScreen(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer) {
    startIntroMusic("sound/music/historyMusic.mp3");

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

void IntroScreen::playFullIntro(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer, std::uint32_t appLaunchTicks) {
    appLaunchTicks_ = appLaunchTicks;
    showLogoGislersoft(graph, renderer);
    showIntroScreen(graph, font, renderer);
    showAboutScreen(graph, font, renderer);
    showHistoriaParte1(graph, font, renderer);
    showHistoriaParte2(graph, font, renderer);
    // Ensure story loop ends before entering exploration/gameplay.
    stopIntroMusic();
}
