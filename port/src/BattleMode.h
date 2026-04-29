#pragma once

#include "GraphCompat.h"
#include "FontCompat.h"
#include "SpriteCompat.h"

#include <SDL.h>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

// RPG character stats — ported from PERSON.H / BATALLA.H
struct BattleChar {
    int hp = 0, hpMax = 0;
    int mp = 0, mpMax = 0;
    int defensa = 0, ataque = 0, magia = 0;
    float def = 0.f, atk = 0.f, mag = 0.f;
    int arr_magia[5] = {};
    int vivo = 1;

    void initAsHero();
    void initAsEnemy();

    // Returns damage dealt after attack roll (dado6)
    int calcAtacar(std::mt19937 &rng) const;

    // Applies incoming damage with defense roll; returns actual HP lost
    int calcDefender(int incoming, std::mt19937 &rng);

    // Convert integer to string (ported from Personaje::convertir)
    static void toStr(int n, char *buf, int buflen);
};

// Turn-based battle screen — ported from BattleMode class in THENDORIA.CPP
// Activation: caller sets battle mode on 'B', exit via wantsExit() on 'E'.
class BattleMode {
public:
    BattleMode();
    ~BattleMode() = default;

    // Load sprites and background from assets_png/IMG/.
    // Call once after SDL_image is initialised.
    bool load();

    // Set the current map name (base filename). Reloads the background image:
    //   starts with 'D' → cavernaf.png, 'C' → castlef.png, default → bosquef.png
    void setMapName(const std::string &name);

    // Reset all battle state for a new encounter.
    void reset();

    // Process input + advance the state machine (call every frame).
    void update(const std::uint8_t *keys, std::uint32_t nowMs);

    // Render the battle scene into g's buffers.
    // Writes background + sprites to the RGBA overlay and UI to pv2.
    // Does NOT call presentLayers — the main loop is responsible.
    void draw(GraphCompat &g, FontCompat &f, std::uint32_t nowMs);

    // True after the player presses 'E' to leave the battle.
    bool wantsExit() const { return wantsExit_; }

    // Read-only access to the loaded background pixels (ARGB, 320×200).
    const std::vector<std::uint32_t>& getBgPixels() const { return bgPixels_; }

private:
    static constexpr int kNEnemies = 3;
    static constexpr int kNHeroes  = 1;

    // Sprite sheets
    SpriteCompat enemySprites_[kNEnemies]; // planta.png  — 7 frames, 60px
    SpriteCompat heroSprites_[kNHeroes];   // prof.png    — 8 frames, 53px

    // Background (320×200 ARGB pixels — image chosen by map name)
    std::vector<std::uint32_t> bgPixels_;
    std::string mapName_;

    // Battle data
    BattleChar enemies_[kNEnemies];
    BattleChar heroes_[kNHeroes];

    // ---- State machine (mirrors the original member variables) ----
    int turno_;              // 1 = player turn, 0 = enemy turn
    int control1_;           // player action phase (0–4)
    int control2_;           // enemy action phase (0–4)
    int op_;                 // main menu cursor: 1=ATACAR 2=POSION (5=hidden)
    int eneActual_;          // current enemy index
    int proActual_;          // current hero index
    int accion_;             // enemy's chosen action
    bool selOpcion_;         // player is choosing menu option
    bool selEnemigo_;        // player is choosing target enemy

    // ---- Timing (ported from clock_t start/start2 + reloj/reloj2 flags) ----
    std::uint32_t startMs_;   // debounce timer origin
    std::uint32_t start2Ms_;  // animation/turn timer origin
    bool reloj_;              // true → restart debounce timer on next update
    bool reloj2_;             // true → restart turn timer on next update

    // Hit-number string shown during animation (e.g. "104")
    char strtemp_[8];

    // Previous-frame key states for edge detection
    bool prevUp_, prevDown_, prevLeft_, prevRight_;
    bool prevSpace_, prevEnter_, prevE_;

    std::mt19937 rng_;
    bool wantsExit_;
    bool loaded_;

    // ---- Battle-start stripe-wipe transition ----
    bool          stripeActive_;   // true while wipe is playing
    std::uint32_t stripeStartMs_;  // timestamp when wipe began

    // ---- Heal-flash feedback (after using a potion) ----
    bool          healFlash_;        // true while hero is flashing green
    std::uint32_t healFlashStartMs_; // timestamp when heal flash began

    // ---- Enemy pre-death shake + red fade (plays for 1 s after kill) ----
    bool          enemyDeathShake_[kNEnemies];
    std::uint32_t enemyDeathShakeMs_[kNEnemies]; // 0 = waiting for hit-anim to end



    // Screen-space anchor positions for each sprite
    // kEX[0]/kEY[0] = front enemy, kEX[2]/kEY[2] = back enemy (drawn first)
    static const int kEX[kNEnemies];
    static const int kEY[kNEnemies];
    static const int kHX[kNHeroes];
    static const int kHY[kNHeroes];

    // ---- Drawing helpers ----
    // Item inventory
    struct BattleItem { char name[16]; int qty; int healHp; };
    static constexpr int kMaxItems = 8;
    BattleItem  items_[kMaxItems];
    int         itemCount_;

    void drawBg(GraphCompat &g) const;
    void drawMenu(GraphCompat &g, FontCompat &f) const;
    void drawStats(GraphCompat &g, FontCompat &f, std::uint32_t nowMs) const;
    // drawEnemies / drawHeroes draw idle sprites and drive death animations.
    // Attack animations are driven directly in draw() to mirror original ordering.
    void drawEnemiesIdle(GraphCompat &g, std::uint32_t nowMs);
    void drawHeroesIdle(GraphCompat &g, std::uint32_t nowMs);
    void markSprite(const SpriteCompat &s, int color, GraphCompat &g) const;
    void showHit(const char *txt, const SpriteCompat &s,
                 FontCompat &f, GraphCompat &g, unsigned char color = 15) const;
    void showHeal(const char *txt, const SpriteCompat &s,
                  FontCompat &f, GraphCompat &g) const;
    void drawStripeWipe(GraphCompat &g, std::uint32_t nowMs);
    bool demorar(std::uint32_t t0, float secs, std::uint32_t now) const;
    void loadBackground();
    static std::string resolvePath(const std::string &name);
};
