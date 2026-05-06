#include "InputHandler.h"

#include <iostream>

// ---------------------------------------------------------------------------
// Ctor / Dtor
// ---------------------------------------------------------------------------

InputHandler::InputHandler() {
    // SDL_INIT_JOYSTICK must already have been called by the time this ctor runs.
    // Open the first available joystick, if any.
    openFirstJoystick();
}

InputHandler::~InputHandler() {
    closeJoystick();
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

bool InputHandler::handleEvent(const SDL_Event &event) {
    switch (event.type) {
        // --- joystick connected -------------------------------------------------
        case SDL_JOYDEVICEADDED: {
            if (joy_ == nullptr) {
                openFirstJoystick();
            }
            return true;
        }

        // --- joystick disconnected ----------------------------------------------
        case SDL_JOYDEVICEREMOVED: {
            if (joy_ != nullptr && event.jdevice.which == joyInstanceId_) {
                std::cout << "[InputHandler] Joystick disconnected.\n";
                closeJoystick();
                // Try to fall back to another joystick, if one is still present.
                openFirstJoystick();
            }
            return true;
        }

        // --- hat (D-pad) --------------------------------------------------------
        case SDL_JOYHATMOTION: {
            if (joy_ != nullptr && event.jhat.which == joyInstanceId_) {
                applyHat(event.jhat.value);
            }
            return true;
        }

        // --- analog axis --------------------------------------------------------
        case SDL_JOYAXISMOTION: {
            if (joy_ != nullptr && event.jaxis.which == joyInstanceId_) {
                applyAxis(event.jaxis.axis, event.jaxis.value);
            }
            return true;
        }

        // --- buttons ------------------------------------------------------------
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP: {
            if (joy_ != nullptr && event.jbutton.which == joyInstanceId_) {
                applyButton(event.jbutton.button,
                            event.jbutton.state == SDL_PRESSED);
            }
            return true;
        }

        default:
            return false;
    }
}

InputSnapshot InputHandler::poll() const {
    const Uint8 *keys = SDL_GetKeyboardState(nullptr);

    InputSnapshot s;
    // Keyboard
    s.up      = (keys[SDL_SCANCODE_UP]    != 0) || (keys[SDL_SCANCODE_W] != 0);
    s.down    = (keys[SDL_SCANCODE_DOWN]  != 0) || (keys[SDL_SCANCODE_S] != 0);
    s.left    = (keys[SDL_SCANCODE_LEFT]  != 0) || (keys[SDL_SCANCODE_A] != 0);
    s.right   = (keys[SDL_SCANCODE_RIGHT] != 0) || (keys[SDL_SCANCODE_D] != 0);
    s.action  =  keys[SDL_SCANCODE_SPACE]  != 0;
    s.confirm =  keys[SDL_SCANCODE_RETURN] != 0;
    s.back    =  keys[SDL_SCANCODE_E]      != 0;
    s.battle  =  keys[SDL_SCANCODE_B]      != 0;
    s.quit    =  keys[SDL_SCANCODE_ESCAPE] != 0;

    // OR-in joystick state
    s.up      = s.up      || joyUp_;
    s.down    = s.down    || joyDown_;
    s.left    = s.left    || joyLeft_;
    s.right   = s.right   || joyRight_;
    s.action  = s.action  || joyAction_;
    s.confirm = s.confirm || joyConfirm_;
    s.back    = s.back    || joyBack_;
    s.battle  = s.battle  || joyBattle_;
    s.quit    = s.quit    || joyQuit_;

    return s;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void InputHandler::openFirstJoystick() {
    const int count = SDL_NumJoysticks();
    if (count <= 0) {
        return;
    }
    joy_ = SDL_JoystickOpen(0);
    if (!joy_) {
        std::cerr << "[InputHandler] SDL_JoystickOpen failed: " << SDL_GetError() << '\n';
        return;
    }
    joyInstanceId_ = SDL_JoystickInstanceID(joy_);
    std::cout << "[InputHandler] Joystick connected: \""
              << SDL_JoystickName(joy_) << "\""
              << "  axes=" << SDL_JoystickNumAxes(joy_)
              << "  buttons=" << SDL_JoystickNumButtons(joy_)
              << "  hats=" << SDL_JoystickNumHats(joy_) << '\n';
}

void InputHandler::closeJoystick() {
    if (joy_) {
        SDL_JoystickClose(joy_);
        joy_ = nullptr;
        joyInstanceId_ = -1;
        resetJoyState();
    }
}

void InputHandler::resetJoyState() {
    joyUp_     = false;
    joyDown_   = false;
    joyLeft_   = false;
    joyRight_  = false;
    joyAction_  = false;
    joyConfirm_ = false;
    joyBack_    = false;
    joyBattle_  = false;
    joyQuit_    = false;
}

void InputHandler::applyHat(Uint8 hatVal) {
    joyUp_    = (hatVal & SDL_HAT_UP)    != 0;
    joyDown_  = (hatVal & SDL_HAT_DOWN)  != 0;
    joyLeft_  = (hatVal & SDL_HAT_LEFT)  != 0;
    joyRight_ = (hatVal & SDL_HAT_RIGHT) != 0;
}

void InputHandler::applyAxis(int axis, Sint16 value) {
    if (axis == 0) {
        // X axis → left / right
        joyLeft_  = (value < -kDeadzone);
        joyRight_ = (value >  kDeadzone);
    } else if (axis == 1) {
        // Y axis → up / down
        joyUp_   = (value < -kDeadzone);
        joyDown_ = (value >  kDeadzone);
    }
    // Axes 2+ (e.g. triggers on gamepads) are ignored for now.
}

void InputHandler::applyButton(int button, bool pressed) {
    switch (button) {
        case 0: joyAction_  = pressed; break;  // A / first face button  → interact
        case 1: joyConfirm_ = pressed; break;  // B / second face button → dialog
        case 2: joyBattle_  = pressed; break;  // X / third face button  → battle
        case 3: joyBack_    = pressed; break;  // Y / fourth face button → exit battle
        case 6: joyQuit_    = pressed; break;  // Start                  → quit
        case 7: joyQuit_    = pressed; break;  // Select / back          → quit
        default: break;
    }
}
