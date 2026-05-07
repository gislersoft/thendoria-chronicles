#include "BattleMode.h"

#include <SDL_image.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// Screen-space anchor positions (pixels on 320×200 canvas)
// Enemy index 0 = nearest (front-left), 2 = farthest (back-right)
// Drawing order in drawEnemiesIdle: sorted by Y ascending (back to front): 2,5,1,4,0,3
// Row 0 (original): slots 0,1,2
// Row 1 (new, +60x/+20y from row 0): slots 3,4,5
// ---------------------------------------------------------------------------
const int BattleMode::kEX[BattleMode::kNEnemies] = {  70, 110, 150, 130, 170, 210 };
const int BattleMode::kEY[BattleMode::kNEnemies] = {  80,  50,  20, 100,  70,  40 };
const int BattleMode::kHX[BattleMode::kNHeroes]  = { 210 };
const int BattleMode::kHY[BattleMode::kNHeroes]  = { 100 };

// ---------------------------------------------------------------------------
// BattleChar
// ---------------------------------------------------------------------------

void BattleChar::initAsHero() {
    hp = hpMax = 524;
    mp = mpMax = 100;
    defensa = 3; ataque = 100; magia = 15;
    def = 0.05f; atk = 0.03f; mag = 0.04f;
    arr_magia[0] = 1; arr_magia[1] = 2;
    arr_magia[2] = 0; arr_magia[3] = 2; arr_magia[4] = 3;
    vivo = 1;
}

void BattleChar::initAsEnemy() {
    hp = hpMax = 400;
    mp = mpMax = 100;
    defensa = 10; ataque = 30; magia = 10;
    def = 0.03f; atk = 0.01f; mag = 0.05f;
    for (int i = 0; i < 5; ++i) arr_magia[i] = 1;
    vivo = 1;
}

// Ported from Personaje::dado6 — random ±% variance on attack value
static float dado6(int n, std::mt19937 &rng) {
    float aux = 0.f;
    switch (rng() % 7) {
        case 0: aux = n * -0.05f; break;
        case 1: aux = n *  0.02f; break;
        case 2: aux = n *  0.04f; break;
        case 3: aux = n *  0.06f; break;
        case 4: aux = n *  0.08f; break;
        case 5: aux = n *  0.10f; break;
        default: break;
    }
    return static_cast<float>(n) + aux;
}

// Ported from Personaje::dado3 — random ±% variance on defense value
static float dado3(float n, std::mt19937 &rng) {
    float aux = 0.f;
    switch (rng() % 3) {
        case 1: aux =  n * 0.10f; break;
        case 2: aux = -n * 0.10f; break;
        default: break;
    }
    return n + aux;
}

int BattleChar::calcAtacar(std::mt19937 &rng) const {
    int aux = static_cast<int>(ataque * atk) + ataque;
    return static_cast<int>(dado6(aux, rng));
}

int BattleChar::calcDefender(int incoming, std::mt19937 &rng) {
    float aux = static_cast<float>(incoming);
    aux = aux - (static_cast<float>(defensa) + static_cast<float>(defensa) * def);
    aux = dado3(aux, rng);
    int damage = static_cast<int>(aux);
    if (damage < 0) damage = 0;
    if (damage < hp) {
        hp -= damage;
    } else {
        hp = 0;
        vivo = 0;
    }
    return damage;
}

void BattleChar::toStr(int n, char *buf, int buflen) {
    if (!buf || buflen < 2) return;
    if (n < 0) n = 0;
    // Build digit string (up to 4 digits like original convertir)
    char tmp[16];
    std::snprintf(tmp, sizeof(tmp), "%d", n);
    std::size_t len = std::strlen(tmp);
    std::size_t cap = static_cast<std::size_t>(buflen - 1);
    if (len > cap) {
        std::memmove(tmp, tmp + len - cap, cap + 1);
        len = cap;
    }
    int k;
    for (k = 0; k < buflen - 1 && tmp[k]; ++k) buf[k] = tmp[k];
    buf[k] = '\0'; // null-terminate right after last digit, not at end of buffer
}

// ---------------------------------------------------------------------------
// BattleMode — helpers
// ---------------------------------------------------------------------------

bool BattleMode::demorar(std::uint32_t t0, float secs, std::uint32_t now) const {
    return (now - t0) > static_cast<std::uint32_t>(secs * 1000.f);
}

std::string BattleMode::resolvePath(const std::string &name) {
    for (const std::string &prefix : {
            "port/assets_png/IMG/",
            "assets_png/IMG/",
            "../assets_png/IMG/",
            "../port/assets_png/IMG/"
         }) {
        const std::string path = prefix + name;
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    return {};
}

// ---------------------------------------------------------------------------
// BattleMode — constructor / load / reset
// ---------------------------------------------------------------------------

BattleMode::BattleMode() : rng_(std::random_device{}()) {
    std::memset(strtemp_, '*', 4);
    strtemp_[4] = '\0';
    loaded_ = false;
    wantsExit_ = false;
    wantsRestart_ = false;
    // zero-init state
    turno_ = control1_ = control2_ = 0;
    op_ = 0;
    eneActual_ = proActual_ = accion_ = 0;
    selOpcion_ = selEnemigo_ = false;
    itemCount_ = 0;
    startMs_ = start2Ms_ = 0;
    reloj_ = reloj2_ = true;
    prevUp_ = prevDown_ = prevLeft_ = prevRight_ = false;
    prevSpace_ = prevEnter_ = prevE_ = false;
    prevAnyKey_     = false;
    stripeActive_   = false;
    stripeStartMs_  = 0;
    healFlash_        = false;
    healFlashStartMs_ = 0;
    victoryActive_    = false;
    victoryStartMs_   = 0;
    gameOverActive_   = false;
    gameOverStartMs_  = 0;
    for (int i = 0; i < kNEnemies; ++i) {
        enemyDeathShake_[i]   = false;
        enemyDeathShakeMs_[i] = 0;
        enemyHitShake_[i]     = false;
        enemyHitShakeMs_[i]   = 0;
        enemyIsBig_[i]        = false;
    }
    for (int i = 0; i < kNHeroes; ++i) {
        heroShake_[i]   = false;
        heroShakeMs_[i] = 0;
    }
}

bool BattleMode::load() {
    if (loaded_) return true;

    // --- Enemy sprite (planta.png: 7 frames × 60px) ---
    const std::string plantaPath = resolvePath("planta.png");
    for (int i = 0; i < kNEnemies; ++i) {
        enemySprites_[i].crear(7, 60, 0.13f);
        if (!plantaPath.empty()) {
            if (enemySprites_[i].cargarSpritePNG(plantaPath) == 1) {
                if (i == 0) std::cout << "BattleMode: enemy sprite <- " << plantaPath << '\n';
            } else {
                std::cerr << "BattleMode: failed to load enemy sprite from " << plantaPath << '\n';
            }
        } else {
            std::cerr << "BattleMode: planta.png not found — using blank enemy sprite\n";
        }
    }
    // --- Big plant sprite (plantabig.png: 7 frames × 90px) ---
    const std::string plantaBigPath = resolvePath("plantabig.png");
    for (int i = 0; i < kNEnemies; ++i) {
        plantaBigSprites_[i].crear(7, 60, 0.13f);
        if (!plantaBigPath.empty()) {
            if (plantaBigSprites_[i].cargarSpritePNG(plantaBigPath) == 1) {
                if (i == 0) std::cout << "BattleMode: big plant sprite <- " << plantaBigPath << '\n';
            } else {
                std::cerr << "BattleMode: failed to load big plant sprite from " << plantaBigPath << '\n';
            }
        } else {
            std::cerr << "BattleMode: plantabig.png not found \u2014 big plant will use normal sprite\n";
        }
    }
    // --- Hero sprite (prof.png: 9 frames × 53px) ---
    const std::string profPath = resolvePath("prof.png");
    for (int i = 0; i < kNHeroes; ++i) {
        heroSprites_[i].crear(9, 53, 0.2f);
        if (!profPath.empty()) {
            if (heroSprites_[i].cargarSpritePNG(profPath) == 1) {
                if (i == 0) std::cout << "BattleMode: hero sprite <- " << profPath << '\n';
            } else {
                std::cerr << "BattleMode: failed to load hero sprite from " << profPath << '\n';
            }
        } else {
            std::cerr << "BattleMode: prof.png not found — using blank hero sprite\n";
        }
    }

    loadBackground();

    loaded_ = true;
    return true;
}

void BattleMode::setMapName(const std::string &name) {
    mapName_ = name;
    loadBackground();
}

void BattleMode::loadBackground() {
    const char first = mapName_.empty() ? '\0'
        : static_cast<char>(std::toupper(static_cast<unsigned char>(mapName_[0])));
    const std::string bgFile = (first == 'D') ? "cavernaf.png"
                             : (first == 'C') ? "castlef.png"
                             : "bosquef.png";

    bgPixels_.assign(320 * 200, 0xFF102040u); // dark-blue fallback
    const std::string bgPath = resolvePath(bgFile);
    if (!bgPath.empty()) {
        SDL_Surface *raw = IMG_Load(bgPath.c_str());
        if (raw) {
            SDL_Surface *rgba = SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(raw);
            if (rgba) {
                const int srcW = rgba->w;
                const int srcH = rgba->h;
                for (int y = 0; y < std::min(200, srcH); ++y) {
                    for (int x = 0; x < std::min(320, srcW); ++x) {
                        const auto *p = static_cast<const unsigned char *>(rgba->pixels)
                                        + y * rgba->pitch + x * 4;
                        bgPixels_[x + y * 320] =
                            (std::uint32_t(p[3]) << 24) |
                            (std::uint32_t(p[0]) << 16) |
                            (std::uint32_t(p[1]) <<  8) |
                            std::uint32_t(p[2]);
                    }
                }
                SDL_FreeSurface(rgba);
                std::cout << "BattleMode: background <- " << bgPath
                          << " (" << srcW << "x" << srcH << ")\n";
            }
        } else {
            std::cerr << "BattleMode: IMG_Load failed for " << bgPath
                      << ": " << IMG_GetError() << '\n';
        }
    } else {
        std::cerr << "BattleMode: " << bgFile << " not found -- using solid background\n";
    }
}

void BattleMode::reset() {
    for (int i = 0; i < kNEnemies; ++i) {
        enemies_[i].initAsEnemy();
        enemySprites_[i].animacion     = 0;
        plantaBigSprites_[i].animacion = 0;
        enemyIsBig_[i]                 = false;
    }
    for (int i = 0; i < kNHeroes; ++i) {
        heroes_[i].initAsHero();
        heroSprites_[i].animacion = 0;
    }

    // --- Random enemy count (weighted) ---
    // Weights: 1→2, 2→2, 3→3, 4→1, 5→1, 6→1  (total=10)
    static constexpr int kCountWeights[kNEnemies] = { 2, 2, 3, 1, 1, 1 };
    int total = 0;
    for (int w : kCountWeights) total += w;
    int roll = static_cast<int>(rng_() % static_cast<unsigned>(total));
    int nActive = kNEnemies;
    for (int n = 0; n < kNEnemies; ++n) {
        roll -= kCountWeights[n];
        if (roll < 0) { nActive = n + 1; break; }
    }
    // Kill slots that exceed the active count
    for (int i = nActive; i < kNEnemies; ++i)
        enemies_[i].vivo = 0;
    std::cout << "BattleMode: " << nActive << " enemies spawned\n";

    // plantabig: 40% chance if active count is exactly 2 or 3 (center slot = 1)
    if (nActive > 2 && nActive <= 3 && (rng_() % 100) < 40) {
        enemyIsBig_[1]   = true;
        enemies_[1].hp   = enemies_[1].hpMax = enemies_[1].hpMax * 2;
        std::cout << "BattleMode: big plant spawned at slot 1 (HP=" << enemies_[1].hp << ")\n";
    }

    turno_    = 1;
    control1_ = 0;
    control2_ = 0;
    op_        = 1;
    eneActual_ = 0;
    enePlayer_ = 0;
    proActual_ = 0;
    accion_    = 1;
    selOpcion_  = false;
    selEnemigo_ = false;

    // Initialize item inventory
    itemCount_ = 1;
    items_[0] = {"POSION", 99, 300};

    multipleQty_ = 3;

    startMs_  = 0;
    start2Ms_ = 0;
    reloj_    = true;
    reloj2_   = true;

    std::memset(strtemp_, '*', 4);
    strtemp_[4] = '\0';

    prevUp_ = prevDown_ = prevLeft_ = prevRight_ = false;
    prevSpace_ = prevEnter_ = prevE_ = false;
    prevAnyKey_   = false;
    hasJoystick_  = false;
    wantsExit_ = false;
    wantsRestart_ = false;

    healFlash_        = false;
    healFlashStartMs_ = 0;
    victoryActive_    = false;
    victoryStartMs_   = 0;
    gameOverActive_   = false;
    gameOverStartMs_  = 0;
    for (int i = 0; i < kNEnemies; ++i) {
        enemyDeathShake_[i]   = false;
        enemyDeathShakeMs_[i] = 0;
        enemyHitShake_[i]     = false;
        enemyHitShakeMs_[i]   = 0;
    }
    for (int i = 0; i < kNHeroes; ++i) {
        heroShake_[i]   = false;
        heroShakeMs_[i] = 0;
    }

    // Start the stripe-wipe animation when battle begins
    stripeActive_  = true;
    stripeStartMs_ = 0; // will be stamped on first draw call
}

// ---------------------------------------------------------------------------
// BattleMode::update — input + state machine (one call per frame)
// ---------------------------------------------------------------------------

void BattleMode::update(const std::uint8_t *keys, std::uint32_t nowMs, bool joyAnyButton, bool hasJoystick) {
    hasJoystick_ = hasJoystick;

    // --- Exit key (always active, even during stripe wipe) ---
    const bool pressE = keys[SDL_SCANCODE_E] != 0;
    if (pressE && !prevE_) {
        wantsExit_ = true;
    }
    prevE_ = pressE;
    if (wantsExit_) return;

    // --- Victory: wait for any key press (after 500 ms grace period) ---
    if (victoryActive_) {
        if (victoryStartMs_ != 0 && (nowMs - victoryStartMs_) >= 500) {
            const bool pressAny = joyAnyButton ||
                                  (keys[SDL_SCANCODE_SPACE]  != 0) ||
                                  (keys[SDL_SCANCODE_RETURN] != 0) ||
                                  (keys[SDL_SCANCODE_Z]      != 0) ||
                                  (keys[SDL_SCANCODE_X]      != 0);
                                  (keys[SDL_SCANCODE_X]      != 0);
            if (pressAny && !prevAnyKey_) {
                wantsExit_ = true;
            }
            prevAnyKey_ = pressAny;
        }
        return; // suppress all other input during victory
    }

    // --- Game Over: wait for any key press (after 500 ms grace period) ---
    if (gameOverActive_) {
        if (gameOverStartMs_ != 0 && (nowMs - gameOverStartMs_) >= 500) {
            const bool pressAny = joyAnyButton ||
                                  (keys[SDL_SCANCODE_SPACE]  != 0) ||
                                  (keys[SDL_SCANCODE_RETURN] != 0) ||
                                  (keys[SDL_SCANCODE_Z]      != 0) ||
                                  (keys[SDL_SCANCODE_X]      != 0);
            if (pressAny && !prevAnyKey_) {
                wantsRestart_ = true;
            }
            prevAnyKey_ = pressAny;
        }
        return; // suppress all other input during game over
    }

    // Suppress battle input while the stripe-wipe is playing
    if (stripeActive_) return;

    // --- Edge-detect movement / confirm keys ---
    const bool pressUp    = keys[SDL_SCANCODE_UP]    != 0;
    const bool pressDown  = keys[SDL_SCANCODE_DOWN]  != 0;
    const bool pressLeft  = keys[SDL_SCANCODE_LEFT]  != 0;
    const bool pressRight = keys[SDL_SCANCODE_RIGHT] != 0;
    const bool pressSpace = keys[SDL_SCANCODE_SPACE] != 0;
    const bool pressEnter = keys[SDL_SCANCODE_RETURN] != 0;

    const bool justUp    = pressUp    && !prevUp_;
    const bool justDown  = pressDown  && !prevDown_;
    const bool justLeft  = pressLeft  && !prevLeft_;
    const bool justRight = pressRight && !prevRight_;
    const bool justConf  = (pressSpace && !prevSpace_) || (pressEnter && !prevEnter_);

    prevUp_    = pressUp;   prevDown_  = pressDown;
    prevLeft_  = pressLeft; prevRight_ = pressRight;
    prevSpace_ = pressSpace; prevEnter_ = pressEnter;

    // --- Restart timers when flagged ---
    if (reloj_)  { startMs_  = nowMs; reloj_  = false; }
    if (reloj2_) { start2Ms_ = nowMs; reloj2_ = false; }

    const bool inputOk = demorar(startMs_, 0.18f, nowMs);

    // =========================================================
    // PLAYER TURN
    // =========================================================
    if (turno_ == 1) {

        // --- Confirm / back ---
        if (justConf && inputOk) {
            if (selEnemigo_) {
                selEnemigo_ = false;
                control1_   = 2;
            } else if (selOpcion_) {
                if (op_ == 2) { // POSION — use directly, no submenu
                    if (items_[0].qty > 0) {
                        BattleChar &hero = heroes_[proActual_];
                        hero.hp = std::min(hero.hpMax, hero.hp + items_[0].healHp);
                        --items_[0].qty;
                    }
                    BattleChar::toStr(items_[0].healHp, strtemp_, static_cast<int>(sizeof(strtemp_)));
                    healFlash_        = true;
                    healFlashStartMs_ = nowMs;
                    selOpcion_ = false;
                    op_        = 5;
                    control1_  = 4;
                    // Assign start2Ms_ directly — reloj2_=true would be
                    // processed next frame, leaving the old stale timestamp
                    // and causing case 4 to fire immediately this same frame.
                    start2Ms_ = nowMs;
                    reloj2_   = false;
                } else if (op_ == 3) { // MULTIPLE
                    if (multipleQty_ > 0) {
                        --multipleQty_;
                        control1_  = 5;
                        selOpcion_ = false;
                        op_        = 5; // hide menu during animation
                        start2Ms_  = nowMs;
                        reloj2_    = false;
                    }
                } else { // ATACAR (op_ == 1)
                    control1_  = 1;
                    selOpcion_ = false;
                }
            }
            reloj_ = true;
        }

        // --- Navigate enemy selection (left / right) — skip dead enemies ---
        if (inputOk && justLeft && selEnemigo_) {
            int next = (eneActual_ > 0) ? eneActual_ - 1 : kNEnemies - 1;
            for (int tries = 0; tries < kNEnemies; ++tries) {
                if (enemies_[next].vivo == 1) break;
                next = (next > 0) ? next - 1 : kNEnemies - 1;
            }
            eneActual_ = next;
            reloj_ = true;
        }
        if (inputOk && justRight && selEnemigo_) {
            int next = (eneActual_ < kNEnemies - 1) ? eneActual_ + 1 : 0;
            for (int tries = 0; tries < kNEnemies; ++tries) {
                if (enemies_[next].vivo == 1) break;
                next = (next < kNEnemies - 1) ? next + 1 : 0;
            }
            eneActual_ = next;
            reloj_ = true;
        }

        // --- Navigate menu (up / down) ---
        if (inputOk && justUp)   { if (selOpcion_) { --op_; reloj_ = true; } }
        if (inputOk && justDown) { if (selOpcion_) { ++op_; reloj_ = true; } }

        // Wrap main menu cursor (ATACAR=1, ITEM=2, MULTIPLE=3)
        if (op_ < 1) op_ = 3;  if (op_ > 3) op_ = 1;
        // Skip MULTIPLE when it is disabled (0 uses left)
        if (op_ == 3 && multipleQty_ == 0) { op_ = justDown ? 1 : 2; }

        // --- Player state machine ---
        switch (control1_) {
            case 0:
                selOpcion_ = true;
                break;
            case 1:
                // Advance to first living enemy before entering selection
                for (int i = 0; i < kNEnemies; ++i) {
                    if (enemies_[eneActual_].vivo == 1) break;
                    eneActual_ = (eneActual_ + 1) % kNEnemies;
                }
                selEnemigo_ = true;
                break;
            case 2:
                if (enemies_[eneActual_].vivo == 1) {
                    enePlayer_ = eneActual_; // save player's chosen target
                    if (op_ == 1) { // ATACAR
                        control1_ = 3;
                    }
                    // ITEM is handled in the justConf/selOpcion_ branch — no enemy pick needed
                } else {
                    // Target is dead — reselect
                    control1_ = 0;
                }
                break;
            case 3: {
                // Calculate player attack on current enemy
                int dmg = enemies_[eneActual_].calcDefender(
                    heroes_[proActual_].calcAtacar(rng_), rng_);
                BattleChar::toStr(dmg, strtemp_, static_cast<int>(sizeof(strtemp_)));
                enemySprites_[eneActual_].animacion = 1;
                eSprite(eneActual_).animacion       = 1;
                heroSprites_[proActual_].animacion  = 1;
                enemyHitShake_[eneActual_]   = true;
                enemyHitShakeMs_[eneActual_] = nowMs;
                // If killing blow, arm the pre-death shake
                if (enemies_[eneActual_].vivo == 0) {
                    enemyDeathShake_[eneActual_]   = true;
                    enemyDeathShakeMs_[eneActual_] = 0;
                }
                control1_ = 4;
                op_       = 5; // hide menu during animation
                reloj2_   = true;
                break;
            }
            case 4:
                // Wait for animations + 2-second pause before advancing turn
                if (demorar(start2Ms_, 2.f, nowMs) &&
                    eSprite(eneActual_).animacion     == 0 &&
                    heroSprites_[proActual_].animacion == 0) {
                    healFlash_ = false;
                    // Check all enemies dead
                    bool allDead = true;
                    for (int i = 0; i < kNEnemies; ++i)
                        if (enemies_[i].vivo == 1) { allDead = false; break; }
                    if (allDead) {
                        victoryActive_  = true;
                        victoryStartMs_ = nowMs;
                        op_             = 5;
                        break;
                    }
                    ++proActual_;
                    op_ = 1;
                    if (proActual_ >= kNHeroes) {
                        proActual_  = 0;
                        // Do NOT reset eneActual_ — enemy turn iterates from 0 itself.
                        // enePlayer_ already holds the player's confirmed target.
                        eneActual_  = 0;
                        control1_   = 0;
                        turno_      = 0; // switch to enemy turn
                        op_         = 5;
                    } else {
                        control1_ = 0;
                    }
                    reloj2_ = true;
                }
                break;

            case 5: {
                // MULTIPLE: find first alive enemy and begin attacking
                eneActual_ = 0;
                for (int i = 0; i < kNEnemies; ++i) {
                    if (enemies_[i].vivo == 1) { eneActual_ = i; break; }
                }
                control1_ = 6;
                break;
            }
            case 6: {
                // MULTIPLE: attack eneActual_, then wait
                int dmg = enemies_[eneActual_].calcDefender(
                    heroes_[proActual_].calcAtacar(rng_), rng_);
                BattleChar::toStr(dmg, strtemp_, static_cast<int>(sizeof(strtemp_)));
                enemySprites_[eneActual_].animacion = 1;
                eSprite(eneActual_).animacion       = 1;
                heroSprites_[proActual_].animacion  = 1;
                enemyHitShake_[eneActual_]   = true;
                enemyHitShakeMs_[eneActual_] = nowMs;
                if (enemies_[eneActual_].vivo == 0) {
                    enemyDeathShake_[eneActual_]   = true;
                    enemyDeathShakeMs_[eneActual_] = 0;
                }
                control1_ = 7;
                reloj2_   = true;
                break;
            }
            case 7: {
                // MULTIPLE: wait for animation, then advance to next alive enemy or end turn
                if (demorar(start2Ms_, 2.f, nowMs) &&
                    eSprite(eneActual_).animacion     == 0 &&
                    heroSprites_[proActual_].animacion == 0) {
                    // Find next alive enemy after current
                    int next = -1;
                    for (int i = eneActual_ + 1; i < kNEnemies; ++i) {
                        if (enemies_[i].vivo == 1) { next = i; break; }
                    }
                    if (next != -1) {
                        eneActual_ = next;
                        control1_  = 6; // attack next
                    } else {
                        // All enemies hit — check if all are dead (victory)
                        bool allDead = true;
                        for (int i = 0; i < kNEnemies; ++i)
                            if (enemies_[i].vivo == 1) { allDead = false; break; }
                        if (allDead) {
                            victoryActive_  = true;
                            victoryStartMs_ = nowMs;
                            op_             = 5;
                        } else {
                            // End player turn normally
                            healFlash_ = false;
                            ++proActual_;
                            op_ = 1;
                            if (proActual_ >= kNHeroes) {
                                proActual_  = 0;
                                eneActual_  = 0;
                                control1_   = 0;
                                turno_      = 0;
                                op_         = 5;
                            } else {
                                control1_ = 0;
                            }
                            reloj2_ = true;
                        }
                    }
                }
                break;
            }
        }

    // =========================================================
    } else {
        // ENEMY TURN
        // =========================================================
        switch (control2_) {
            case 0:
                control2_ = 1;
                accion_   = 1;
                break;
            case 1:
                proActual_ = 0;
                control2_  = 2;
                break;
            case 2:
                if (enemies_[eneActual_].vivo == 1) {
                    // Only basic attack implemented
                    control2_ = 3;
                } else {
                    // Skip dead enemy
                    ++eneActual_;
                    if (eneActual_ >= kNEnemies) {
                        // All enemies iterated — end enemy turn, back to player
                        // Restore player's last target; if dead advance to next alive
                        eneActual_ = enePlayer_;
                        for (int t = 0; t < kNEnemies; ++t) {
                            if (enemies_[eneActual_].vivo == 1) break;
                            eneActual_ = (eneActual_ + 1) % kNEnemies;
                        }
                        enePlayer_ = eneActual_;
                        proActual_ = 0;
                        control2_  = 0;
                        turno_     = 1;
                        op_        = 1;
                    }
                }
                break;
            case 3: {
                // Calculate enemy attack on current hero
                int dmg = heroes_[proActual_].calcDefender(
                    enemies_[eneActual_].calcAtacar(rng_), rng_);
                if (enemyIsBig_[eneActual_]) dmg *= 2;
                BattleChar::toStr(dmg, strtemp_, static_cast<int>(sizeof(strtemp_)));
                enemySprites_[eneActual_].animacion = 1;
                eSprite(eneActual_).animacion       = 1;
                heroSprites_[proActual_].animacion  = 1;
                // Arm hero shake — start immediately so offset is visible
                // as soon as the enemy sprite moves toward the hero
                heroShake_[proActual_]   = true;
                heroShakeMs_[proActual_] = nowMs;
                // Detect hero death
                if (heroes_[proActual_].vivo == 0) {
                    heroSprites_[proActual_].animacion = 1; // keep animating until we show tombstone
                    gameOverActive_  = true;
                    gameOverStartMs_ = 0; // stamped on first draw
                    op_              = 5; // hide menu
                }
                control2_ = 4;
                reloj2_   = true;
                break;
            }
            case 4:
                if (demorar(start2Ms_, 2.f, nowMs) &&
                    heroSprites_[proActual_].animacion == 0 &&
                    eSprite(eneActual_).animacion      == 0) {
                    ++eneActual_;
                    if (eneActual_ >= kNEnemies) {
                        proActual_  = 0;
                        // Restore player's last target; if dead advance to next alive
                        eneActual_ = enePlayer_;
                        for (int t = 0; t < kNEnemies; ++t) {
                            if (enemies_[eneActual_].vivo == 1) break;
                            eneActual_ = (eneActual_ + 1) % kNEnemies;
                        }
                        enePlayer_  = eneActual_;
                        control2_   = 0;
                        turno_      = 1;
                        op_         = 1;
                    } else {
                        control2_ = 0;
                    }
                    reloj2_ = true;
                }
                break;
        }
    }
}

// ---------------------------------------------------------------------------
// BattleMode::draw — render everything for one frame
// Drawing order mirrors original modoBatalla():
//   background → attack-animation sprites → idle enemies → idle heroes → UI
// ---------------------------------------------------------------------------

void BattleMode::draw(GraphCompat &g, FontCompat &f, std::uint32_t nowMs) {
    // Stripe-wipe intro — plays once when battle first starts
    if (stripeActive_) {
        if (stripeStartMs_ == 0) stripeStartMs_ = nowMs;
        drawStripeWipe(g, nowMs);
        if (nowMs - stripeStartMs_ >= 1000) stripeActive_ = false;
        return;
    }

    // 1. Fill base with black (battle replaces map; pv1 must be clear)
    g.clr(g.pv1, 0);
    g.clr(g.pv2, 0);

    // 2. Blit background to RGBA overlay
    drawBg(g);

    // 3. Idle enemies always drawn first (back layer).
    //    drawEnemiesIdle skips any enemy with animacion==1 (currently animating).
    drawEnemiesIdle(g, nowMs);

    // 4. Selection marker around target enemy (on top of idle enemies, under heroes)
    if (selEnemigo_) {
        eSprite(eneActual_).posicionar(kEX[eneActual_], kEY[eneActual_]);
        markSprite(eSprite(eneActual_),
                   enemies_[eneActual_].vivo == 1 ? 157 : 69, g);
    }

    // 5. Attack animations — order chosen to keep hero on top
    if (turno_ == 1 && (control1_ == 4 || control1_ == 7) && !healFlash_) {
        // PLAYER ATTACKS:
        // a) Enemy hit at its position (behind hero)
        if (eSprite(eneActual_).animacion == 1) {
            int shakeOff = 0;
            if (enemyHitShake_[eneActual_] && enemyHitShakeMs_[eneActual_] != 0) {
                const float el =
                    static_cast<float>(nowMs - enemyHitShakeMs_[eneActual_]) / 1000.f;
                if (el < 1.0f)
                    shakeOff = static_cast<int>(
                        std::sin(el * 50.f) * 4.f * (1.f - el));
            }
            eSprite(eneActual_).posicionar(kEX[eneActual_] + shakeOff, kEY[eneActual_]);
            eSprite(eneActual_).animar(0, 4, 0, nowMs, g);
        }
        // b) Idle heroes (none animating during player attack, but keep consistent)
        drawHeroesIdle(g, nowMs);
        // c) Hero attacks toward enemy — drawn last (on top of everything)
        if (heroSprites_[proActual_].animacion == 1) {
            heroSprites_[proActual_].posicionar(
                kEX[eneActual_] + 5,
                kEY[eneActual_] + 5);
            heroSprites_[proActual_].animar(2, 4, 0, nowMs, g);
        }
        if (!victoryActive_ && !gameOverActive_)
            showHit(strtemp_, eSprite(eneActual_), f, g);

    } else if (turno_ == 0 && control2_ == 4) {
        // ENEMY ATTACKS:
        // a) Idle heroes drawn first (hurt hero has animacion==1, skipped by drawHeroesIdle)
        drawHeroesIdle(g, nowMs);
        // b) Enemy moves toward hero — second to last
        if (eSprite(eneActual_).animacion == 1) {
            eSprite(eneActual_).posicionar(
                kHX[proActual_] - 20,
                kHY[proActual_] -  5);
            eSprite(eneActual_).animar(5, 6, 0, nowMs, g);
        }
        // c) Hero hurt animation — OR tombstone if dead
        if (gameOverActive_) {
            // Hero is dead: show tombstone (frame 8) instead of hurt animation
            heroSprites_[proActual_].posicionar(kHX[proActual_], kHY[proActual_]);
            heroSprites_[proActual_].dibujar(8, 0, g);
        } else if (heroSprites_[proActual_].animacion == 1) {
            int shakeOff = 0;
            if (heroShake_[proActual_] && heroShakeMs_[proActual_] != 0) {
                const float el =
                    static_cast<float>(nowMs - heroShakeMs_[proActual_]) / 1000.f;
                if (el < 1.0f)
                    shakeOff = static_cast<int>(
                        std::sin(el * 50.f) * 4.f * (1.f - el));
            }
            heroSprites_[proActual_].posicionar(kHX[proActual_] + shakeOff, kHY[proActual_]);
            heroSprites_[proActual_].animar(5, 7, 0, nowMs, g);
        }
        if (!gameOverActive_ && !victoryActive_)
            showHit(strtemp_, heroSprites_[proActual_], f, g, 46); // yellow — hero takes damage

    } else {
        // IDLE / HEAL TURN: snapshot hero region, draw idle heroes, apply heal flash
        const bool doHealFlash = (turno_ == 1 && control1_ == 4 && healFlash_);
        const bool flashOn     = doHealFlash && ((nowMs / 80) % 2 == 0);
        int hfX1 = 0, hfY1 = 0, hfX2 = 0, hfY2 = 0, hfW = 0;
        std::vector<std::uint32_t> heroPreDraw;
        if (flashOn) {
            heroSprites_[proActual_].posicionar(kHX[proActual_], kHY[proActual_]);
            hfX1 = std::max(0, heroSprites_[proActual_].x1);
            hfY1 = std::max(0, heroSprites_[proActual_].y1);
            hfX2 = std::min(319, heroSprites_[proActual_].x2);
            hfY2 = std::min(199, heroSprites_[proActual_].y2);
            hfW  = hfX2 - hfX1 + 1;
            const int hfH = hfY2 - hfY1 + 1;
            heroPreDraw.resize(static_cast<std::size_t>(hfW * hfH));
            const std::uint32_t *ov = g.getOverlay();
            for (int py = hfY1; py <= hfY2; ++py)
                for (int px = hfX1; px <= hfX2; ++px)
                    heroPreDraw[static_cast<std::size_t>((py - hfY1) * hfW + (px - hfX1))]
                        = ov[py * 320 + px];
        }
        drawHeroesIdle(g, nowMs);
        if (doHealFlash) {
            heroSprites_[proActual_].posicionar(kHX[proActual_], kHY[proActual_]);
            if (flashOn && !heroPreDraw.empty()) {
                std::uint32_t *ov = g.getOverlay();
                for (int py = hfY1; py <= hfY2; ++py) {
                    for (int px = hfX1; px <= hfX2; ++px) {
                        std::uint32_t &pixel = ov[py * 320 + px];
                        const std::uint32_t saved =
                            heroPreDraw[static_cast<std::size_t>((py - hfY1) * hfW + (px - hfX1))];
                        if (pixel == saved) continue;
                        const std::uint32_t r  = (pixel >> 16) & 0xFFu;
                        const std::uint32_t gv = (pixel >>  8) & 0xFFu;
                        const std::uint32_t b  =  pixel        & 0xFFu;
                        pixel = 0xFF000000u
                              | ((r  * 3u / 10u) << 16)
                              | (std::min(255u, gv + 120u) << 8)
                              | (b  * 3u / 10u);
                    }
                }
            }
            if (!victoryActive_ && !gameOverActive_)
                showHeal(strtemp_, heroSprites_[proActual_], f, g);
        }
    }

    // 6. UI drawn on top layer (pv2)
    drawStats(g, f, nowMs);
    if (!victoryActive_ && !gameOverActive_) {
        drawMenu(g, f);
    }

    // 7. Victory overlay
    if (victoryActive_) {
        // Stamp start time on first draw
        if (victoryStartMs_ == 0) victoryStartMs_ = nowMs;

        // Shake: horizontal offset oscillates
        const float elapsed = static_cast<float>(nowMs - victoryStartMs_) / 1000.f;
        const int shakeX = static_cast<int>(std::sin(elapsed * 40.f) * 3.f);

        // Flash: alternate between green (color 32) and white (15) every 120 ms
        const unsigned char vcol = ((nowMs / 120) % 2 == 0) ? 32 : 15;

        // "VICTORY" scaled 3×: 7 chars × 6px × 3 = 126px wide, 7px × 3 = 21px tall
        const int vw = 7 * 6 * 3;
        const int vh = 7 * 3;
        const int vx = (320 - vw) / 2 + shakeX;
        const int vy = (200 - vh) / 2;
        f.putstrScaled(g.pv2, vx, vy, "VICTORY", g, 0, vcol, 3);

        // Pulsing "press any key" prompt — shown after 500 ms grace period
        if ((nowMs - victoryStartMs_) >= 500) {
            const char *kPrompt = hasJoystick_
                ? "PRESIONE CUALQUIER BOTON PARA CONTINUAR..."
                : "PRESIONE CUALQUIER TECLA PARA CONTINUAR...";
            const float pt      = elapsed;
            const float ppulse  = 0.5f * (std::sinf(pt * 7.0f) + 1.0f);
            const int   pbounce = static_cast<int>(std::sinf(pt * 4.0f) * 2.0f);
            const unsigned char pcol = (ppulse > 0.5f) ? 15 : 46; // white / yellow
            const int px = (320 - 42 * 6) / 2;
            const int py = vy + vh + 15 + pbounce;
            f.putstr(g.pv2, px, py, kPrompt, g, 0, pcol);
        }
    }

    // 8. Game Over overlay
    if (gameOverActive_) {
        // Stamp start time on first draw
        if (gameOverStartMs_ == 0) gameOverStartMs_ = nowMs;

        // Draw tombstone (frame 8) at hero position
        heroSprites_[proActual_].posicionar(kHX[proActual_], kHY[proActual_]);
        heroSprites_[proActual_].dibujar(8, 0, g);

        // Shake + flash: red (color 69) and white (15)
        const float goElapsed = static_cast<float>(nowMs - gameOverStartMs_) / 1000.f;
        const int goShakeX = static_cast<int>(std::sin(goElapsed * 40.f) * 3.f);
        const unsigned char gocol = ((nowMs / 120) % 2 == 0) ? 69 : 15;

        // "GAME OVER" = 9 chars × 6px × 3 = 162px wide, 21px tall
        const int gow = 9 * 6 * 3;
        const int goh = 7 * 3;
        const int gox = (320 - gow) / 2 + goShakeX;
        const int goy = (200 - goh) / 2;
        f.putstrScaled(g.pv2, gox, goy, "GAME OVER", g, 0, gocol, 3);

        // Pulsing "press any key" prompt — shown after 500 ms grace period
        if ((nowMs - gameOverStartMs_) >= 500) {
            const char *kPrompt = hasJoystick_
                ? "PRESIONE CUALQUIER BOTON PARA CONTINUAR..."
                : "PRESIONE CUALQUIER TECLA PARA CONTINUAR...";
            const float gpt      = goElapsed;
            const float gppulse  = 0.5f * (std::sinf(gpt * 7.0f) + 1.0f);
            const int   gpbounce = static_cast<int>(std::sinf(gpt * 4.0f) * 2.0f);
            const unsigned char gpcol = (gppulse > 0.5f) ? 15 : 46; // white / yellow
            const int gpx = (320 - 42 * 6) / 2;
            const int gpy = goy + goh + 15 + gpbounce;
            f.putstr(g.pv2, gpx, gpy, kPrompt, g, 0, gpcol);
        }
    }
}

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------

void BattleMode::drawStripeWipe(GraphCompat &g, std::uint32_t nowMs) {
    // 16 horizontal bands slide in from alternating sides over 1000 ms,
    // revealing the battle background. Safe smoothstep easing.
    g.clr(g.pv1, 0);
    g.clr(g.pv2, 0);

    const float rawT = std::min(1.0f,
        static_cast<float>(nowMs - stripeStartMs_) / 1000.f);
    const float t   = rawT * rawT * (3.f - 2.f * rawT);
    const int   off = static_cast<int>(320.f * (1.f - t));

    constexpr int kStripes = 16;
    constexpr int kStripeH = 200 / kStripes;

    if (bgPixels_.empty()) return;

    // Build a shifted composite into a temporary buffer and push via drawRgbaFrame
    static std::vector<std::uint32_t> frame(320 * 200, 0xFF000000u);
    frame.assign(320 * 200, 0xFF000000u);

    for (int s = 0; s < kStripes; ++s) {
        const int y0   = s * kStripeH;
        const int yEnd = (s == kStripes - 1) ? 200 : y0 + kStripeH;
        const int shift = (s % 2 == 0) ? off : -off;
        for (int y = y0; y < yEnd; ++y) {
            for (int x = 0; x < 320; ++x) {
                const int srcX = x + shift;
                if (srcX >= 0 && srcX < 320)
                    frame[y * 320 + x] = bgPixels_[y * 320 + srcX];
            }
        }
    }
    g.drawRgbaFrame(0, 0, 320, 200, frame.data());
}

void BattleMode::drawBg(GraphCompat &g) const {
    if (bgPixels_.empty()) return;
    g.drawRgbaFrame(0, 0, 320, 200, bgPixels_.data());
}

void BattleMode::drawEnemiesIdle(GraphCompat &g, std::uint32_t nowMs) {
    // Back-to-front order sorted by Y ascending: 2(y=20),5(y=40),1(y=50),4(y=70),0(y=80),3(y=100)
    static constexpr int kDrawOrder[kNEnemies] = { 2, 5, 1, 4, 0, 3 };
    for (int di = 0; di < kNEnemies; ++di) {
        const int i = kDrawOrder[di];
        if (enemies_[i].vivo == 1) {
            if (eSprite(i).animacion == 0) {
                // Apply hit shake offset if active
                int shakeOff = 0;
                if (enemyHitShake_[i] && enemyHitShakeMs_[i] != 0) {
                    const float el =
                        static_cast<float>(nowMs - enemyHitShakeMs_[i]) / 1000.f;
                    if (el < 1.0f)
                        shakeOff = static_cast<int>(
                            std::sin(el * 50.f) * 4.f * (1.f - el));
                    else
                        enemyHitShake_[i] = false;
                }
                eSprite(i).posicionar(kEX[i] + shakeOff, kEY[i]);

                // Snapshot overlay before drawing so we can tint only sprite pixels
                const bool lowHp = (enemies_[i].hp < 50);
                const bool redFlash = lowHp && ((nowMs / 250) % 2 == 0);
                std::vector<std::uint32_t> snap;
                int bx1 = 0, by1 = 0, bx2 = 0, by2 = 0, bw = 0;
                if (redFlash) {
                    std::uint32_t *ov = g.getOverlay();
                    bx1 = std::max(0, eSprite(i).x1);
                    by1 = std::max(0, eSprite(i).y1);
                    bx2 = std::min(319, eSprite(i).x2);
                    by2 = std::min(199, eSprite(i).y2);
                    bw  = bx2 - bx1 + 1;
                    const int bh = by2 - by1 + 1;
                    snap.resize(static_cast<std::size_t>(bw * bh));
                    for (int py = by1; py <= by2; ++py)
                        for (int px = bx1; px <= bx2; ++px)
                            snap[static_cast<std::size_t>((py - by1) * bw + (px - bx1))]
                                = ov[py * 320 + px];
                }

                eSprite(i).dibujart(0, 3, 0, nowMs, g); // idle loop frames 0–3

                if (redFlash && !snap.empty()) {
                    std::uint32_t *ov = g.getOverlay();
                    for (int py = by1; py <= by2; ++py) {
                        for (int px = bx1; px <= bx2; ++px) {
                            std::uint32_t &pixel = ov[py * 320 + px];
                            const std::uint32_t sv = snap[static_cast<std::size_t>(
                                (py - by1) * bw + (px - bx1))];
                            if (pixel == sv) continue; // background — skip
                            const std::uint32_t r  = (pixel >> 16) & 0xFFu;
                            const std::uint32_t gv = (pixel >>  8) & 0xFFu;
                            const std::uint32_t b  =  pixel        & 0xFFu;
                            pixel = 0xFF000000u
                                  | (std::min(255u, r + 120u) << 16)
                                  | ((gv * 3u / 10u) << 8)
                                  | (b * 3u / 10u);
                        }
                    }
                }
            }
        } else {
            // Dead enemy — pre-death shake+tint, then death animation
            if (enemyDeathShake_[i]) {
                if (eSprite(i).animacion == 1) {
                    // Hit animation still playing in the attack block — skip here
                    // Hit animation ended: arm the shake timer on the first frame
                    if (enemyDeathShakeMs_[i] == 0) enemyDeathShakeMs_[i] = nowMs;

                    const float elapsed =
                        static_cast<float>(nowMs - enemyDeathShakeMs_[i]) / 1000.f;

                    if (elapsed < 1.0f) {
                        // Decaying horizontal oscillation
                        const int shakeOff = static_cast<int>(
                            std::sin(elapsed * 45.f) * 4.f * (1.f - elapsed));
                        eSprite(i).posicionar(kEX[i] + shakeOff, kEY[i]);

                        // Snapshot overlay region before drawing
                        std::uint32_t *ov = g.getOverlay();
                        const int bx1 = std::max(0, eSprite(i).x1);
                        const int by1 = std::max(0, eSprite(i).y1);
                        const int bx2 = std::min(319, eSprite(i).x2);
                        const int by2 = std::min(199, eSprite(i).y2);
                        const int bw  = bx2 - bx1 + 1;
                        const int bh  = by2 - by1 + 1;
                        std::vector<std::uint32_t> snap(
                            static_cast<std::size_t>(bw * bh));
                        for (int py = by1; py <= by2; ++py)
                            for (int px = bx1; px <= bx2; ++px)
                                snap[static_cast<std::size_t>(
                                    (py - by1) * bw + (px - bx1))]
                                    = ov[py * 320 + px];

                        // Draw static frame 4 (hit frame) while dissolving
                        eSprite(i).dibujar(4, 0, g);

                        // Tint sprite pixels red, then interpolate toward the
                        // background (snap) so the sprite dissolves by elapsed=1
                        for (int py = by1; py <= by2; ++py) {
                            for (int px = bx1; px <= bx2; ++px) {
                                std::uint32_t &pixel = ov[py * 320 + px];
                                const std::uint32_t sv = snap[static_cast<std::size_t>(
                                    (py - by1) * bw + (px - bx1))];
                                if (pixel == sv) continue; // background — skip
                                const std::uint32_t r  = (pixel >> 16) & 0xFFu;
                                const std::uint32_t gv = (pixel >>  8) & 0xFFu;
                                const std::uint32_t b  =  pixel        & 0xFFu;
                                // Red-tinted version of the sprite pixel
                                const std::uint32_t tr = std::min(255u, r + 110u);
                                const std::uint32_t tg = gv * 3u / 10u;
                                const std::uint32_t tb = b  * 3u / 10u;
                                // Background pixel (what was behind the sprite)
                                const std::uint32_t bkR = (sv >> 16) & 0xFFu;
                                const std::uint32_t bkG = (sv >>  8) & 0xFFu;
                                const std::uint32_t bkB =  sv        & 0xFFu;
                                // Lerp: elapsed=0 → full red sprite; elapsed=1 → background
                                const std::uint32_t fr = static_cast<std::uint32_t>(
                                    static_cast<float>(tr) * (1.f - elapsed) + static_cast<float>(bkR) * elapsed);
                                const std::uint32_t fg = static_cast<std::uint32_t>(
                                    static_cast<float>(tg) * (1.f - elapsed) + static_cast<float>(bkG) * elapsed);
                                const std::uint32_t fb = static_cast<std::uint32_t>(
                                    static_cast<float>(tb) * (1.f - elapsed) + static_cast<float>(bkB) * elapsed);
                                pixel = 0xFF000000u | (fr << 16) | (fg << 8) | fb;
                            }
                        }
                    } else {
                        // Shake finished — sprite disappears
                        enemyDeathShake_[i] = false;
                    }
                }
            } else {
                // Play death animation (one-shot, frames 0–4); nothing drawn after it ends
                if (eSprite(i).animacion == 1) {
                    eSprite(i).posicionar(kEX[i], kEY[i]);
                    eSprite(i).animar(0, 4, 0, nowMs, g);
                }
            }
        }
    }
}

void BattleMode::drawHeroesIdle(GraphCompat &g, std::uint32_t nowMs) {
    for (int i = 0; i < kNHeroes; ++i) {
        if (heroes_[i].vivo == 1 && heroSprites_[i].animacion == 0) {
            if (heroShake_[i] && heroShakeMs_[i] != 0) {
                const float elapsed =
                    static_cast<float>(nowMs - heroShakeMs_[i]) / 1000.f;
                if (elapsed < 1.0f) {
                    const int shakeOff = static_cast<int>(
                        std::sin(elapsed * 50.f) * 4.f * (1.f - elapsed));
                    heroSprites_[i].posicionar(kHX[i] + shakeOff, kHY[i]);
                } else {
                    heroShake_[i] = false;
                    heroSprites_[i].posicionar(kHX[i], kHY[i]);
                }
            } else {
                heroSprites_[i].posicionar(kHX[i], kHY[i]);
            }
            heroSprites_[i].dibujart(0, 2, 0, nowMs, g);
        }
    }
}

void BattleMode::markSprite(const SpriteCompat &s, int color, GraphCompat &g) const {
    // Draw a selection rectangle around the sprite on pv2
    g.box(g.pv2, s.x1, s.y1, s.x2, s.y2, static_cast<unsigned char>(color));
}

void BattleMode::showHit(const char *txt, const SpriteCompat &s,
                          FontCompat &f, GraphCompat &g, unsigned char color) const {
    // Draw damage number near the top-left of the sprite (pv2 layer)
    const int tx = s.x1;
    const int ty = std::max(0, s.y1 - 8);
    g.fillbox(g.pv2, tx - 1, ty - 1, tx + 28, ty + 8, 0);
    f.putstr(g.pv2, tx, ty, txt, g, 0, color);
}

void BattleMode::showHeal(const char *txt, const SpriteCompat &s,
                           FontCompat &f, GraphCompat &g) const {
    // Draw heal number above the hero sprite in bright green (pv2 layer)
    // Color 190 = (76, 212, 116) bright green in the custom palette
    const int tx = s.x1;
    const int ty = std::max(0, s.y1 - 8);
    g.fillbox(g.pv2, tx - 1, ty - 1, tx + 28, ty + 8, 0);
    f.putstr(g.pv2, tx, ty, txt, g, 0, 190);
}

// ---------------------------------------------------------------------------
// drawStats — HP table at bottom-left
// ---------------------------------------------------------------------------
void BattleMode::drawStats(GraphCompat &g, FontCompat &f, std::uint32_t nowMs) const {
    const BattleChar &hero = heroes_[0];

    // HP bar color:
    //   hp >  300 → green  (190)
    //   hp >  100 → yellow (46)
    //   hp <= 100 → flicker yellow (46) / red (69) every 300 ms
    unsigned char barColor;
    if (hero.hp > 300) {
        barColor = 190; // green
    } else if (hero.hp > 100) {
        barColor = 46;  // yellow
    } else {
        barColor = ((nowMs / 300) % 2 == 0) ? 46 : 69; // yellow / red flicker
    }

    // Current HP number uses same color as the bar; max HP in white
    char curHp[8] = {}, maxHp[8] = {};
    BattleChar::toStr(hero.hp,    curHp, 8);
    BattleChar::toStr(hero.hpMax, maxHp, 8);
    f.putstr(g.pv2,  4, 170, curHp, g, 0, barColor);
    f.putstr(g.pv2, 32, 170, maxHp, g, 0, 15); // white

    // HP bar with "HP:" label to the left
    f.putstr(g.pv2, 4, 178, "HP:", g, 0, 15);
    constexpr int kBarX = 31;
    constexpr int kBarW = 36;
    const int hpBar = (hero.hpMax > 0)
        ? std::clamp(static_cast<int>((float)kBarW * hero.hp / hero.hpMax), 0, kBarW)
        : 0;
    g.fillbox(g.pv2, kBarX, 178, kBarX + hpBar, 183, barColor);
    g.box(g.pv2,     kBarX, 178, kBarX + kBarW, 183, 15); // white border
}

// ---------------------------------------------------------------------------
// drawMenu — main action menu at bottom-right
// ---------------------------------------------------------------------------
void BattleMode::drawMenu(GraphCompat &g, FontCompat &f) const {
    // Turn indicator
    if (turno_ == 1) {
        f.putstr(g.pv2, 250, 177, "TU TURNO", g, 0, 157); // orange
    } else {
        f.putstr(g.pv2, 250, 177, "ENEMIGO",  g, 0, 69);  // red
    }

    // op_ == 5 means menu is hidden during animation
    const bool show = (op_ >= 1 && op_ <= 3);
    const unsigned char colNormal   = 15;  // white
    const unsigned char colSelected = 46;  // yellow-gold

    f.putstr(g.pv2, 170, 164, "ATACAR", g, 0,
             (show && op_ == 1) ? colSelected : colNormal);

    // POSION with current count
    char pocionLabel[12] = {};
    const int qty = (itemCount_ > 0) ? items_[0].qty : 0;
    std::snprintf(pocionLabel, sizeof(pocionLabel), "POSION %d", qty);
    f.putstr(g.pv2, 170, 177, pocionLabel, g, 0,
             (show && op_ == 2) ? colSelected : colNormal);

    // MULTIPLE with current count
    char multiLabel[16] = {};
    std::snprintf(multiLabel, sizeof(multiLabel), "MULTIPLE %d", multipleQty_);
    f.putstr(g.pv2, 170, 190, multiLabel, g, 0,
             multipleQty_ == 0 ? 8 : (show && op_ == 3) ? colSelected : colNormal);
}


