#pragma once
#include "AnodyneSharp/Common.hpp"
#include "XNA/Input.hpp"
#include <vector>

namespace AnodyneSharp::Input {

enum class MouseButton { None = 0, LeftButton, RightButton, MiddleButton };

enum class KeyFunctions {
    Up = 1, Down, Left, Right,
    Accept, Cancel, Pause,
    PreviousPage, NextPage,
    Broom1, Broom2, Broom3, Broom4,
    QuickSave, QuickLoad
};

struct RebindableKey {
    std::vector<Keys>        mKeys;
    std::vector<Buttons>     ButtonsList;
    std::vector<MouseButton> MouseButtons;
    std::vector<GestureType> TouchGestures;
    std::optional<PlayerIndex> GamePadPlayerIndex;

    PlayerIndex PlayerIndexOrDefault() const {
        return GamePadPlayerIndex.value_or(PlayerIndex::One);
    }

    // Constructors mirroring C#
    RebindableKey() = default;
    RebindableKey(const std::vector<::Keys>& keys,
                  const std::vector<Buttons>& buttons = {},
                  PlayerIndex idx = PlayerIndex::One)
        : mKeys(keys), ButtonsList(buttons), GamePadPlayerIndex(idx) {}
};

struct PlayerButton {
    Buttons       Button;
    PlayerIndex   CurrentPlayerIndex = PlayerIndex::One;
};

class KeyInput {
public:
    static std::unordered_map<KeyFunctions, RebindableKey> RebindableKeys;
    static bool ControllerMode;
    static bool ControllerModeChanged;
    static int  ControllerButtonOffset;
    static bool FaceButtonsSwitched;

    // Populate RebindableKeys with default keyboard/controller bindings.
    static void InitDefaults();

    static void Update();
    static bool IsRebindableKeyPressed(KeyFunctions functionName);
    static bool JustPressedRebindableKey(KeyFunctions functionName);
    static bool IsKeyPressed(::Keys key);
    static bool JustPressedKey(::Keys key);
    static bool IsAnyKeyPressed(std::optional<::Keys>& pressed);
    static bool IsAnyButtonPressed(std::optional<Buttons>& pressed);
    static void SwapFaceButtons();

private:
    enum class InputState { NONE, HELD, PRESSED };
    static InputState UpdateInput(InputState current, bool pressed);
    static InputState GetRebindableKeyState(KeyFunctions name);

    static std::unordered_map<int, InputState> _keyState;    // Keys->int
    static std::vector<std::unordered_map<int, InputState>> _controllerState;
};

} // namespace AnodyneSharp::Input

using AnodyneSharp::Input::KeyInput;
using AnodyneSharp::Input::KeyFunctions;
using AnodyneSharp::Input::RebindableKey;
using AnodyneSharp::Input::PlayerButton;
using AnodyneSharp::Input::MouseButton;
