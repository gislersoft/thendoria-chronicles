#ifndef INTROSCREEN_H
#define INTROSCREEN_H

#include "GraphCompat.h"
#include "FontCompat.h"
#include <SDL.h>
#if defined(THENDORIA_HAVE_SDL_MIXER)
#include <SDL_mixer.h>
#endif
#include <string>
#include <cstdint>
#include <vector>

class IntroScreen {
public:
    explicit IntroScreen(bool monoAudio = false);
    ~IntroScreen();

    // Registers the Game Boy speaker low-pass filter via Mix_SetPostMix.
    // Call once after Mix_OpenAudio when gameboy audio mode is active.
    static void setupGameboyPostMix();

    // Main intro sequence
    void playFullIntro(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer, std::uint32_t appLaunchTicks);

    // Individual screens
    void showLogoGislersoft(GraphCompat &graph, SDL_Renderer *renderer);
    void showIntroScreen(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer);
    void showAboutScreen(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer);
    void showHistoriaParte1(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer);
    void showHistoriaParte2(GraphCompat &graph, FontCompat &font, SDL_Renderer *renderer);

private:
    // Helper functions
    bool loadImageToBuffer(const std::string &path, SDL_Renderer *renderer, GraphCompat &graph, bool blendOverExisting = false);
    void fadeIn(GraphCompat &graph, int steps);
    void fadeOut(GraphCompat &graph, int steps);
    void playSound(int frequency, int durationMs);
    void waitForKey(bool anyKey = false);
    bool startIntroMusic(const std::string &path);
    void stopIntroMusic();
    void drawDialogBox(GraphCompat &graph, FontCompat &font, const std::vector<std::string> &lines, int startLine, int lineCount);
    bool loadDialogFile(const std::string &filename, std::vector<std::string> &out);
    
    // Path resolution
    std::string resolvePath(const std::string &relPath);
    
    // Image buffer for fades
    std::vector<std::uint32_t> imageBuffer_;

    // Intro song state
    bool introMusicPlaying_ = false;
    bool monoAudio_ = false;
    std::uint32_t appLaunchTicks_ = 0;
#if defined(THENDORIA_HAVE_SDL_MIXER)
    bool mixerReady_ = false;
    Mix_Music *introMusic_ = nullptr;
#endif
};

#endif // INTROSCREEN_H
