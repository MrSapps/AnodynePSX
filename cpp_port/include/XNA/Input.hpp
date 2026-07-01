#pragma once
#include "Framework.hpp"
#include <SDL3/SDL.h>
#include <optional>
#include <string>

namespace Microsoft { namespace Xna { namespace Framework { namespace Input {

// ---------------------------------------------------------------
// Keys enum (all XNA keys)
// ---------------------------------------------------------------
enum class Keys {
    None = 0,
    Back = 8,
    Tab = 9,
    Enter = 13,
    CapsLock = 20,
    Escape = 27,
    Space = 32,
    PageUp = 33,
    PageDown = 34,
    End = 35,
    Home = 36,
    Left = 37,
    Up = 38,
    Right = 39,
    Down = 40,
    Select = 41,
    Print = 42,
    Execute = 43,
    PrintScreen = 44,
    Insert = 45,
    Delete = 46,
    Help = 47,
    D0 = 48,
    D1 = 49,
    D2 = 50,
    D3 = 51,
    D4 = 52,
    D5 = 53,
    D6 = 54,
    D7 = 55,
    D8 = 56,
    D9 = 57,
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    LeftWindows = 91,
    RightWindows = 92,
    Apps = 93,
    NumPad0 = 96,
    NumPad1 = 97,
    NumPad2 = 98,
    NumPad3 = 99,
    NumPad4 = 100,
    NumPad5 = 101,
    NumPad6 = 102,
    NumPad7 = 103,
    NumPad8 = 104,
    NumPad9 = 105,
    Multiply = 106,
    Add = 107,
    Separator = 108,
    Subtract = 109,
    Decimal = 110,
    Divide = 111,
    F1 = 112,
    F2 = 113,
    F3 = 114,
    F4 = 115,
    F5 = 116,
    F6 = 117,
    F7 = 118,
    F8 = 119,
    F9 = 120,
    F10 = 121,
    F11 = 122,
    F12 = 123,
    NumLock = 144,
    Scroll = 145,
    LeftShift = 160,
    RightShift = 161,
    LeftControl = 162,
    RightControl = 163,
    LeftAlt = 164,
    RightAlt = 165,
    OemSemicolon = 186,
    OemPlus = 187,
    OemComma = 188,
    OemMinus = 189,
    OemPeriod = 190,
    OemQuestion = 191,
    OemTilde = 192,
    OemOpenBrackets = 219,
    OemPipe = 220,
    OemCloseBrackets = 221,
    OemQuotes = 222,
    OemBackslash = 226,
    BrowserBack = 166,
    BrowserForward = 167,
    MediaPreviousTrack = 177,
    MediaNextTrack = 176,
    MediaStop = 178,
    MediaPlayPause = 179,
    VolumeMute = 173,
    VolumeDown = 174,
    VolumeUp = 175,
    Pause = 19
};

// ---------------------------------------------------------------
// KeyboardState — snapshot of SDL3 keyboard
// ---------------------------------------------------------------
class KeyboardState {
public:
    // SDL_GetKeyboardState returns a pointer to an internal array;
    // we store a copy indexed by SDL_Scancode.
    bool sdlKeys[SDL_SCANCODE_COUNT] = {};

    bool IsKeyDown(Keys key) const;
    bool IsKeyUp(Keys key)   const { return !IsKeyDown(key); }
};

// Mapping from XNA Keys (Windows VK codes) to SDL_Scancode
SDL_Scancode XNAKeyToScancode(Keys k);

// ---------------------------------------------------------------
// Keyboard — polls SDL3 key state
// ---------------------------------------------------------------
class Keyboard {
public:
    static KeyboardState GetState() {
        KeyboardState ks;
        int numKeys = 0;
        const bool* sdl = SDL_GetKeyboardState(&numKeys);
        int n = numKeys < SDL_SCANCODE_COUNT ? numKeys : SDL_SCANCODE_COUNT;
        for (int i = 0; i < n; ++i) ks.sdlKeys[i] = sdl[i];
        return ks;
    }
};

// ---------------------------------------------------------------
// PlayerIndex
// ---------------------------------------------------------------
enum class PlayerIndex {
    One = 0,
    Two = 1,
    Three = 2,
    Four = 3
};

// ---------------------------------------------------------------
// Buttons enum
// ---------------------------------------------------------------
enum class Buttons {
    DPadUp = 1,
    DPadDown = 2,
    DPadLeft = 4,
    DPadRight = 8,
    Start = 16,
    Back = 32,
    LeftStick = 64,
    RightStick = 128,
    LeftShoulder = 256,
    RightShoulder = 512,
    BigButton = 2048,
    A = 4096,
    B = 8192,
    X = 16384,
    Y = 32768,
    LeftThumbstickLeft = 2097152,
    RightTrigger = 1024,
    LeftTrigger = 4096000,
    RightThumbstickUp = 4194304,
    RightThumbstickDown = 8388608,
    RightThumbstickRight = 16777216,
    RightThumbstickLeft = 33554432,
    LeftThumbstickUp = 67108864,
    LeftThumbstickDown = 134217728,
    LeftThumbstickRight = 268435456
};

// ---------------------------------------------------------------
// GamePadCapabilities (stub)
// ---------------------------------------------------------------
struct GamePadCapabilities {
    bool IsConnected = false;
    std::string DisplayName;
};

// ---------------------------------------------------------------
// GamePadState — wraps SDL3 gamepad
// ---------------------------------------------------------------
class GamePadState {
public:
    bool IsConnected = false;
    SDL_Gamepad* _pad = nullptr;

    bool IsButtonDown(Buttons button) const;
    bool IsButtonUp(Buttons button)   const { return !IsButtonDown(button); }
};

// ---------------------------------------------------------------
// GamePad — polls SDL3 gamepad
// ---------------------------------------------------------------
class GamePad {
public:
    static GamePadState GetState(PlayerIndex playerIndex) { return GetState((int)playerIndex); }
    static GamePadState GetState(int playerIndex);
    static GamePadCapabilities GetCapabilities(PlayerIndex playerIndex) {
        GamePadCapabilities c;
        SDL_Gamepad* g = SDL_GetGamepadFromPlayerIndex((int)playerIndex);
        c.IsConnected = (g != nullptr);
        return c;
    }
};

// ---------------------------------------------------------------
// ButtonState
// ---------------------------------------------------------------
enum class ButtonState {
    Released = 0,
    Pressed = 1
};

// ---------------------------------------------------------------
// MouseState (stub)
// ---------------------------------------------------------------
class MouseState {
public:
    int X = 0, Y = 0;
    ButtonState LeftButton = ButtonState::Released;
    ButtonState RightButton = ButtonState::Released;
    ButtonState MiddleButton = ButtonState::Released;
    int ScrollWheelValue = 0;
    Point Position() const { return {X, Y}; }
};

// ---------------------------------------------------------------
// Mouse — polls SDL3 mouse state
// ---------------------------------------------------------------
class Mouse {
public:
    static MouseState GetState() {
        MouseState ms;
        float fx, fy;
        SDL_MouseButtonFlags b = SDL_GetMouseState(&fx, &fy);
        ms.X = (int)fx; ms.Y = (int)fy;
        ms.LeftButton   = (b & SDL_BUTTON_LMASK) ? ButtonState::Pressed : ButtonState::Released;
        ms.RightButton  = (b & SDL_BUTTON_RMASK) ? ButtonState::Pressed : ButtonState::Released;
        ms.MiddleButton = (b & SDL_BUTTON_MMASK) ? ButtonState::Pressed : ButtonState::Released;
        return ms;
    }
    static void SetPosition(int x, int y) {
        extern SDL_Window* g_SDLWindow;
        // SDL_WarpMouseInWindow(g_SDLWindow, x, y);  // skip for now
    }
};

// ---------------------------------------------------------------
// GestureType (stub for touch)
// ---------------------------------------------------------------
enum class GestureType {
    None = 0,
    Tap = 1,
    DoubleTap = 2,
    Hold = 4,
    HorizontalDrag = 8,
    VerticalDrag = 16,
    FreeDrag = 32,
    Pinch = 64,
    PinchComplete = 128,
    Flick = 256,
    DragComplete = 512
};

}}}} // namespace Microsoft::Xna::Framework::Input

// Bring input types into scope
using Microsoft::Xna::Framework::Input::Keys;
using Microsoft::Xna::Framework::Input::KeyboardState;
using Microsoft::Xna::Framework::Input::Keyboard;
using Microsoft::Xna::Framework::Input::PlayerIndex;
using Microsoft::Xna::Framework::Input::Buttons;
using Microsoft::Xna::Framework::Input::GamePadCapabilities;
using Microsoft::Xna::Framework::Input::GamePadState;
using Microsoft::Xna::Framework::Input::GamePad;
using Microsoft::Xna::Framework::Input::ButtonState;
using Microsoft::Xna::Framework::Input::MouseState;
using Microsoft::Xna::Framework::Input::Mouse;
using Microsoft::Xna::Framework::Input::GestureType;
