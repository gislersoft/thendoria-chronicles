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
// Drawing order in drawEnemiesIdle: 2 → 1 → 0 (back to front, as original)
// ---------------------------------------------------------------------------
const int BattleMode::kEX[BattleMode::kNEnemies] = { 70, 110, 150 };
const int BattleMode::kEY[BattleMode::kNEnemies] = { 80,  50,  20 };
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
    for (int k = 0; k < buflen - 1 && tmp[k]; ++k) buf[k] = tmp[k];
    buf[buflen - 1] = '\0';
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
    // zero-init state
    turno_ = control1_ = control2_ = 0;
    op_ = op2_ = op3_ = magiaEscogida_ = 0;
    eneActual_ = proActual_ = accion_ = 0;
    selOpcion_ = selEnemigo_ = selMagia_ = selSub_ = false;
    startMs_ = start2Ms_ = 0;
    reloj_ = reloj2_ = true;
    prevUp_ = prevDown_ = prevLeft_ = prevRight_ = false;
    prevSpace_ = prevEnter_ = prevE_ = false;
    stripeActive_   = false;
    stripeStartMs_  = 0;
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

    // --- Hero sprite (prof.png: 8 frames × 53px) ---
    const std::string profPath = resolvePath("prof.png");
    for (int i = 0; i < kNHeroes; ++i) {
        heroSprites_[i].crear(8, 53, 0.2f);
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
        enemySprites_[i].animacion = 0;
    }
    for (int i = 0; i < kNHeroes; ++i) {
        heroes_[i].initAsHero();
        heroSprites_[i].animacion = 0;
    }

    turno_    = 1;
    control1_ = 0;
    control2_ = 0;
    op_       = 1;
    op2_      = 1;
    op3_      = 1;
    magiaEscogida_ = 0;
    eneActual_     = 0;
    proActual_     = 0;
    accion_        = 1;
    selOpcion_ = false;
    selEnemigo_ = false;
    selMagia_  = false;
    selSub_    = false;

    startMs_  = 0;
    start2Ms_ = 0;
    reloj_    = true;
    reloj2_   = true;

    std::memset(strtemp_, '*', 4);
    strtemp_[4] = '\0';

    prevUp_ = prevDown_ = prevLeft_ = prevRight_ = false;
    prevSpace_ = prevEnter_ = prevE_ = false;
    wantsExit_ = false;

    // Start the stripe-wipe animation when battle begins
    stripeActive_  = true;
    stripeStartMs_ = 0; // will be stamped on first draw call
}

// ---------------------------------------------------------------------------
// BattleMode::update — input + state machine (one call per frame)
// ---------------------------------------------------------------------------

void BattleMode::update(const std::uint8_t *keys, std::uint32_t nowMs) {

    // --- Exit key (always active, even during stripe wipe) ---
    const bool pressE = keys[SDL_SCANCODE_E] != 0;
    if (pressE && !prevE_) {
        wantsExit_ = true;
    }
    prevE_ = pressE;
    if (wantsExit_) return;

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
            if (selSub_) {
                selSub_   = false;
                control1_ = 3;
                op3_ = 1; op2_ = 1; op_ = 1;
            } else if (selMagia_) {
                op2_      = 7;   // triggers submenu display next frame
                selMagia_ = false;
                selSub_   = true;
            } else if (selEnemigo_) {
                selEnemigo_ = false;
                control1_   = 2;
            } else if (selOpcion_) {
                control1_  = 1;
                selOpcion_ = false;
            }
            reloj_ = true;
        }

        // --- Navigate enemy selection (left / right) ---
        if (inputOk && justLeft && selEnemigo_) {
            eneActual_ = (eneActual_ > 0) ? eneActual_ - 1 : kNEnemies - 1;
            reloj_ = true;
        }
        if (inputOk && justRight && selEnemigo_) {
            eneActual_ = (eneActual_ < kNEnemies - 1) ? eneActual_ + 1 : 0;
            reloj_ = true;
        }

        // --- Navigate menu (up / down) ---
        if (inputOk && justUp) {
            if (selOpcion_) { --op_;  reloj_ = true; }
            if (selMagia_)  { --op2_; reloj_ = true; }
            if (selSub_)    { --op3_; reloj_ = true; }
        }
        if (inputOk && justDown) {
            if (selOpcion_) { ++op_;  reloj_ = true; }
            if (selMagia_)  { ++op2_; reloj_ = true; }
            if (selSub_)    { ++op3_; reloj_ = true; }
        }

        // Wrap menu cursors (original: case 0/4 for op, 0/6 for op2, 0/4 for op3)
        if (op_  < 1) op_  = 3;  if (op_  > 3) op_  = 1;
        if (op2_ < 1) op2_ = 5;  if (op2_ > 5) op2_ = 1;
        if (op3_ < 1) op3_ = 3;  if (op3_ > 3) op3_ = 1;

        // --- Player state machine ---
        switch (control1_) {
            case 0:
                selOpcion_ = true;
                break;
            case 1:
                selEnemigo_ = true;
                break;
            case 2:
                if (enemies_[eneActual_].vivo == 1) {
                    switch (op_) {
                        case 1: // ATACAR
                            control1_ = 3;
                            break;
                        case 2: // MAGIA
                            if (!selSub_) selMagia_ = true;
                            else          selMagia_ = false;
                            op_ = 5; // hide normal menu while in magic sub-menu
                            break;
                        case 3: // ITEM (not implemented — return to option select)
                            control1_ = 0;
                            break;
                    }
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
                heroSprites_[proActual_].animacion  = 1;
                control1_ = 4;
                op_       = 5; // hide menu during animation
                reloj2_   = true;
                break;
            }
            case 4:
                // Wait for animations + 2-second pause before advancing turn
                if (demorar(start2Ms_, 2.f, nowMs) &&
                    enemySprites_[eneActual_].animacion == 0 &&
                    heroSprites_[proActual_].animacion  == 0) {
                    ++proActual_;
                    op_ = 1;
                    if (proActual_ >= kNHeroes) {
                        proActual_  = 0;
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
                        eneActual_ = 0;
                        proActual_ = 0;
                        control2_  = 0;
                        turno_     = 1; // back to player
                    }
                }
                break;
            case 3: {
                // Calculate enemy attack on current hero
                int dmg = heroes_[proActual_].calcDefender(
                    enemies_[eneActual_].calcAtacar(rng_), rng_);
                BattleChar::toStr(dmg, strtemp_, static_cast<int>(sizeof(strtemp_)));
                heroSprites_[proActual_].animacion  = 1;
                enemySprites_[eneActual_].animacion = 1;
                control2_ = 4;
                reloj2_   = true;
                break;
            }
            case 4:
                if (demorar(start2Ms_, 2.f, nowMs) &&
                    heroSprites_[proActual_].animacion  == 0 &&
                    enemySprites_[eneActual_].animacion == 0) {
                    ++eneActual_;
                    if (eneActual_ >= kNEnemies) {
                        eneActual_  = 0;
                        proActual_  = 0;
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

    // 3. Attack-animation sprites (mirror original control1_==4 / control2_==4 drawing)
    if (turno_ == 1 && control1_ == 4) {
        // Enemy gets hit (plays frames 0-4 one-shot)
        if (enemySprites_[eneActual_].animacion == 1) {
            enemySprites_[eneActual_].posicionar(kEX[eneActual_], kEY[eneActual_]);
            enemySprites_[eneActual_].animar(0, 4, 0, nowMs, g);
        }
        // Hero attacks (moves toward enemy, plays frames 2-4 one-shot)
        if (heroSprites_[proActual_].animacion == 1) {
            heroSprites_[proActual_].posicionar(
                kEX[eneActual_] + 5,
                kEY[eneActual_] + 5);
            heroSprites_[proActual_].animar(2, 4, 0, nowMs, g);
        }
        showHit(strtemp_, enemySprites_[eneActual_], f, g);

    } else if (turno_ == 0 && control2_ == 4) {
        // Hero gets hit (plays frames 5-7 one-shot)
        if (heroSprites_[proActual_].animacion == 1) {
            heroSprites_[proActual_].posicionar(kHX[proActual_], kHY[proActual_]);
            heroSprites_[proActual_].animar(5, 7, 0, nowMs, g);
        }
        // Enemy attacks (moves toward hero position, plays frames 5-6 one-shot)
        if (enemySprites_[eneActual_].animacion == 1) {
            enemySprites_[eneActual_].posicionar(
                kHX[proActual_] - 20,
                kHY[proActual_] -  5);
            enemySprites_[eneActual_].animar(5, 6, 0, nowMs, g);
        }
        showHit(strtemp_, heroSprites_[proActual_], f, g);
    }

    // 4. Draw all enemies that are NOT currently in an attack animation
    drawEnemiesIdle(g, nowMs);

    // 5. Selection marker around target enemy
    if (selEnemigo_) {
        enemySprites_[eneActual_].posicionar(kEX[eneActual_], kEY[eneActual_]);
        markSprite(enemySprites_[eneActual_],
                   enemies_[eneActual_].vivo == 1 ? 157 : 69, g);
    }

    // 6. Draw all heroes that are NOT currently in a hurt animation
    drawHeroesIdle(g, nowMs);

    // 7. UI drawn on top layer (pv2)
    drawStats(g, f);
    drawMenu(g, f);
    if (selMagia_ || selSub_) {
        drawMagicMenu(g, f);
    }

    // 8. Minimal "press E to exit" hint
    g.fillbox(g.pv2, 238, 188, 319, 198, 230);
    f.putstr(g.pv2, 240, 190, "E: SALIR", g, 230, 15);
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
    // Draw from back (index 2) to front (index 0) for correct Z-order
    for (int i = kNEnemies - 1; i >= 0; --i) {
        if (enemies_[i].vivo == 1) {
            if (enemySprites_[i].animacion == 0) {
                enemySprites_[i].posicionar(kEX[i], kEY[i]);
                enemySprites_[i].dibujart(0, 3, 0, nowMs, g); // idle loop frames 0–3
            }
        } else {
            // Play death animation (one-shot, frames 0–4); nothing drawn after it ends
            if (enemySprites_[i].animacion == 1) {
                enemySprites_[i].posicionar(kEX[i], kEY[i]);
                enemySprites_[i].animar(0, 4, 0, nowMs, g);
            }
        }
    }
}

void BattleMode::drawHeroesIdle(GraphCompat &g, std::uint32_t nowMs) {
    for (int i = 0; i < kNHeroes; ++i) {
        if (heroes_[i].vivo == 1 && heroSprites_[i].animacion == 0) {
            heroSprites_[i].posicionar(kHX[i], kHY[i]);
            heroSprites_[i].dibujart(0, 2, 0, nowMs, g); // idle loop frames 0–2
        }
    }
}

void BattleMode::markSprite(const SpriteCompat &s, int color, GraphCompat &g) const {
    // Draw a selection rectangle around the sprite on pv2
    g.box(g.pv2, s.x1, s.y1, s.x2, s.y2, static_cast<unsigned char>(color));
}

void BattleMode::showHit(const char *txt, const SpriteCompat &s,
                          FontCompat &f, GraphCompat &g) const {
    // Draw damage number near the top-left of the sprite (pv2 layer)
    const int tx = s.x1;
    const int ty = std::max(0, s.y1 - 8);
    g.fillbox(g.pv2, tx - 1, ty - 1, tx + 28, ty + 8, 0);
    f.putstr(g.pv2, tx, ty, txt, g, 0, 15);
}

// ---------------------------------------------------------------------------
// drawStats — HP / MP table at bottom-left (mirrors Batalla::Mostrartabla)
// ---------------------------------------------------------------------------
void BattleMode::drawStats(GraphCompat &g, FontCompat &f) const {
    const BattleChar &hero = heroes_[0];

    // Background box
    g.fillbox(g.pv2,  0, 162, 165, 199, 230);
    g.box(g.pv2,      0, 162, 165, 199, 15);

    // HP row
    f.putstr(g.pv2,  4, 171, "HP", g, 230, 15);

    char curHp[8], maxHp[8];
    BattleChar::toStr(hero.hp,    curHp, 8);
    BattleChar::toStr(hero.hpMax, maxHp, 8);
    f.putstr(g.pv2,  28, 164, curHp, g, 230, 32);  // current HP (green)
    f.putstr(g.pv2,  70, 164, maxHp, g, 230, 15);  // max HP

    const int hpBar = (hero.hpMax > 0)
        ? std::clamp(static_cast<int>(33.f * hero.hp / hero.hpMax), 0, 33)
        : 0;
    g.fillbox(g.pv2, 28, 170, 28 + hpBar, 173, 32); // green fill
    g.box(g.pv2,     28, 170, 61,         173,  0); // black border

    // MP row
    f.putstr(g.pv2, 4, 182, "MP", g, 230, 84);

    char curMp[8], maxMp[8];
    BattleChar::toStr(hero.mp,    curMp, 8);
    BattleChar::toStr(hero.mpMax, maxMp, 8);
    f.putstr(g.pv2,  28, 177, curMp, g, 230, 84);  // current MP (blue)
    f.putstr(g.pv2,  70, 177, maxMp, g, 230, 15);

    const int mpBar = (hero.mpMax > 0)
        ? std::clamp(static_cast<int>(33.f * hero.mp / hero.mpMax), 0, 33)
        : 0;
    g.fillbox(g.pv2, 28, 183, 28 + mpBar, 186, 84); // blue fill
    g.box(g.pv2,     28, 183, 61,         186,  0);

    // Party slots 2 & 3 (placeholder)
    f.putstr(g.pv2, 90, 177, "VACIO", g, 230, 15);
    f.putstr(g.pv2, 130, 177, "VACIO", g, 230, 15);
}

// ---------------------------------------------------------------------------
// drawMenu — main action menu at bottom-right (mirrors Batalla::menu)
// ---------------------------------------------------------------------------
void BattleMode::drawMenu(GraphCompat &g, FontCompat &f) const {
    g.fillbox(g.pv2, 167, 162, 237, 199, 230);
    g.box(g.pv2,     167, 162, 237, 199,  15);

    // Turn indicator
    if (turno_ == 1) {
        f.putstr(g.pv2, 170, 163, "TU TURNO", g, 230, 32);
    } else {
        f.putstr(g.pv2, 170, 163, "ENEMIGO", g, 230, 69);
    }

    // op_ == 5 means menu is hidden (during animation or magic sub-menu)
    const bool show = (op_ >= 1 && op_ <= 3);
    const unsigned char colNormal   = 15;  // white
    const unsigned char colSelected = 46;  // yellow-gold

    f.putstr(g.pv2, 170, 172, "ATACAR", g, 230,
             (show && op_ == 1) ? colSelected : colNormal);
    f.putstr(g.pv2, 170, 182, "MAGIA",  g, 230,
             (show && op_ == 2) ? colSelected : colNormal);
    f.putstr(g.pv2, 170, 192, "ITEM",   g, 230,
             (show && op_ == 3) ? colSelected : colNormal);
}

// ---------------------------------------------------------------------------
// drawMagicMenu — magic type + level sub-menus (mirrors Batalla::menu_magia)
// ---------------------------------------------------------------------------
void BattleMode::drawMagicMenu(GraphCompat &g, FontCompat &f) {
    // Magic type list box
    g.fillbox(g.pv2, 65, 105, 105, 161, 230);
    g.box(g.pv2,     65, 105, 105, 161,  15);

    static const char *kMagics[5] = {"AIRE", "AGUA", "FUEGO", "TIERRA", "VIDA"};
    for (int i = 0; i < 5; ++i) {
        const unsigned char col = (selMagia_ && op2_ == i + 1) ? 46 : 15;
        f.putstr(g.pv2, 70, 110 + i * 10, kMagics[i], g, 230, col);
    }

    // Magic level sub-menu (shown after type is chosen)
    if (selSub_ || op2_ == 7) {
        g.fillbox(g.pv2, 106, 125, 160, 161, 230);
        g.box(g.pv2,     106, 125, 160, 161,  15);

        const BattleChar &hero = heroes_[proActual_];
        const int maxLvl = (magiaEscogida_ >= 0 && magiaEscogida_ < 5)
                            ? hero.arr_magia[magiaEscogida_] : 0;

        for (int lvl = 1; lvl <= 3; ++lvl) {
            const char *label = (lvl == 1) ? "NIVEL1" :
                                (lvl == 2) ? "NIVEL2" : "NIVEL3";
            const unsigned char col = (lvl <= maxLvl)
                ? ((selSub_ && op3_ == lvl) ? 46 : 15)
                : 186; // greyed out
            f.putstr(g.pv2, 110, 125 + (lvl - 1) * 10, label, g, 230, col);
        }
    }
}
