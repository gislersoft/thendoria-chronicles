#pragma once

#include <SDL.h>

// ---------------------------------------------------------------------------
// Logical input snapshot — all actions that the game cares about.
// True means "currently held / active this frame".
// ---------------------------------------------------------------------------
struct InputSnapshot {
    bool up      = false;  // move north
    bool down    = false;  // move south
    bool left    = false;  // move west
    bool right   = false;  // move east
    bool action  = false;  // interact / SPACE       / joystick button 0
    bool confirm = false;  // advance dialog / ENTER / joystick button 1
    bool back    = false;  // exit battle / E        / joystick button 3
    bool battle  = false;  // trigger battle / B     / joystick button 2
    bool quit    = false;  // exit / ESC             / joystick button 6 or 7
};

// ---------------------------------------------------------------------------
// InputHandler — abstracts keyboard and the first connected joystick/gamepad.
//
// Usage:
//   InputHandler input;
//   // inside SDL_PollEvent loop:
//   input.handleEvent(event);
//   // after event loop:
//   InputSnapshot snap = input.poll();
//
// Joystick button layout (generic arcade / USB stick):
//   Button 0  → action  (interact / SPACE)
//   Button 1  → confirm (dialog advance / ENTER)
//   Button 2  → battle  (B key)
//   Button 6  → quit    (Start button / ESC)
//   Button 7  → quit    (Select / alternative start)
//
// D-pad hat:   standard SDL hat values → up/down/left/right
// Axis 0 (X):  left (-) / right (+)   with 8 000-unit deadzone
// Axis 1 (Y):  up (-)   / down (+)    with 8 000-unit deadzone
// ---------------------------------------------------------------------------
class InputHandler {
public:
    InputHandler();
    ~InputHandler();

    // Feed every SDL event here while inside your SDL_PollEvent loop.
    // Returns true if the event was a joystick event (informational only).
    bool handleEvent(const SDL_Event &event);

    // Build and return the merged keyboard + joystick state.
    // Call once per frame, after all events have been processed.
    InputSnapshot poll() const;

private:
    SDL_Joystick *joy_  = nullptr;
    SDL_JoystickID joyInstanceId_ = -1;

    // Joystick digital state (updated by handleEvent)
    bool joyUp_     = false;
    bool joyDown_   = false;
    bool joyLeft_   = false;
    bool joyRight_  = false;
    bool joyAction_  = false;
    bool joyConfirm_ = false;
    bool joyBack_    = false;
    bool joyBattle_  = false;
    bool joyQuit_    = false;

    void openFirstJoystick();
    void closeJoystick();
    void resetJoyState();

    void applyHat(Uint8 hatVal);
    void applyAxis(int axis, Sint16 value);
    void applyButton(int button, bool pressed);

    static constexpr Sint16 kDeadzone = 8000;
};
