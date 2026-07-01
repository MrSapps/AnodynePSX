#include "AnodyneSharp/Input/KeyInput.hpp"

namespace AnodyneSharp::Input {

std::unordered_map<KeyFunctions, RebindableKey> KeyInput::RebindableKeys;
bool KeyInput::ControllerMode = false;
bool KeyInput::ControllerModeChanged = false;
int  KeyInput::ControllerButtonOffset = 0;
bool KeyInput::FaceButtonsSwitched = false;
std::unordered_map<int, KeyInput::InputState> KeyInput::_keyState;
std::vector<std::unordered_map<int, KeyInput::InputState>> KeyInput::_controllerState(4);

KeyInput::InputState KeyInput::UpdateInput(InputState current, bool pressed) {
    if (!pressed) return InputState::NONE;
    if (current == InputState::NONE) return InputState::PRESSED;
    return InputState::HELD;
}

void KeyInput::InitDefaults() {
    using KF = KeyFunctions;
    RebindableKeys = {
        { KF::Up,           RebindableKey({ Keys::Up,    Keys::W },     { Buttons::DPadUp,    Buttons::LeftThumbstickUp    }) },
        { KF::Down,         RebindableKey({ Keys::Down,  Keys::S },     { Buttons::DPadDown,  Buttons::LeftThumbstickDown  }) },
        { KF::Left,         RebindableKey({ Keys::Left,  Keys::A },     { Buttons::DPadLeft,  Buttons::LeftThumbstickLeft  }) },
        { KF::Right,        RebindableKey({ Keys::Right, Keys::D },     { Buttons::DPadRight, Buttons::LeftThumbstickRight }) },
        { KF::Accept,       RebindableKey({ Keys::Z,     Keys::Enter }, { Buttons::A }) },
        { KF::Cancel,       RebindableKey({ Keys::X,     Keys::Escape}, { Buttons::B }) },
        { KF::Pause,        RebindableKey({ Keys::Escape,Keys::P },    { Buttons::Start }) },
        { KF::PreviousPage, RebindableKey({ Keys::Q },                  { Buttons::LeftShoulder }) },
        { KF::NextPage,     RebindableKey({ Keys::E },                  { Buttons::RightShoulder }) },
        { KF::Broom1,       RebindableKey({ Keys::D1 },                 { Buttons::Y }) },
        { KF::Broom2,       RebindableKey({ Keys::D2 },                 { Buttons::X }) },
        { KF::Broom3,       RebindableKey({ Keys::D3 },                 { Buttons::LeftTrigger }) },
        { KF::Broom4,       RebindableKey({ Keys::D4 },                 { Buttons::RightTrigger }) },
        { KF::QuickSave,    RebindableKey({ Keys::F5 },                 {}) },
        { KF::QuickLoad,    RebindableKey({ Keys::F9 },                 {}) },
    };
}

void KeyInput::Update() {
    static const Buttons allButtons[] = {
        Buttons::DPadUp,    Buttons::DPadDown,    Buttons::DPadLeft,   Buttons::DPadRight,
        Buttons::Start,     Buttons::Back,        Buttons::LeftStick,  Buttons::RightStick,
        Buttons::LeftShoulder, Buttons::RightShoulder,
        Buttons::A, Buttons::B, Buttons::X, Buttons::Y,
        Buttons::LeftTrigger, Buttons::RightTrigger,
        Buttons::LeftThumbstickLeft,  Buttons::LeftThumbstickRight,
        Buttons::LeftThumbstickUp,    Buttons::LeftThumbstickDown,
        Buttons::RightThumbstickLeft, Buttons::RightThumbstickRight,
        Buttons::RightThumbstickUp,   Buttons::RightThumbstickDown,
    };

    ControllerModeChanged = false;
    KeyboardState s = Keyboard::GetState();

    for (int k = 0; k <= 255; ++k) {
        bool isDown = s.IsKeyDown(static_cast<Keys>(k));
        _keyState[k] = UpdateInput(_keyState[k], isDown);
        if (ControllerMode && isDown) {
            ControllerMode = false;
            ControllerModeChanged = true;
        }
    }

    for (int i = 0; i < 4; ++i) {
        GamePadState g = GamePad::GetState(static_cast<PlayerIndex>(i));
        for (Buttons b : allButtons) {
            bool isDown = g.IsButtonDown(b);
            _controllerState[i][(int)b] = UpdateInput(_controllerState[i][(int)b], isDown);
            if (!ControllerMode && isDown) {
                ControllerMode = true;
                ControllerModeChanged = true;
            }
        }
    }
}

KeyInput::InputState KeyInput::GetRebindableKeyState(KeyFunctions name) {
    auto it = RebindableKeys.find(name);
    if (it == RebindableKeys.end()) return InputState::NONE;
    InputState state = InputState::NONE;
    for (auto& k : it->second.Keys) {
        auto sit = _keyState.find((int)k);
        if (sit != _keyState.end() && sit->second > state) state = sit->second;
    }
    int pi = (int)it->second.PlayerIndexOrDefault();
    if (pi < (int)_controllerState.size()) {
        for (auto& b : it->second.ButtonsList) {
            auto sit = _controllerState[pi].find((int)b);
            if (sit != _controllerState[pi].end() && sit->second > state)
                state = sit->second;
        }
    }
    return state;
}

bool KeyInput::IsRebindableKeyPressed(KeyFunctions functionName) {
    return GetRebindableKeyState(functionName) != InputState::NONE;
}
bool KeyInput::JustPressedRebindableKey(KeyFunctions functionName) {
    return GetRebindableKeyState(functionName) == InputState::PRESSED;
}
bool KeyInput::IsKeyPressed(::Keys key) {
    auto it = _keyState.find((int)key);
    return it != _keyState.end() && it->second != InputState::NONE;
}
bool KeyInput::JustPressedKey(::Keys key) {
    auto it = _keyState.find((int)key);
    return it != _keyState.end() && it->second == InputState::PRESSED;
}
bool KeyInput::IsAnyKeyPressed(std::optional<::Keys>& pressed) {
    pressed = std::nullopt;
    for (auto& [k, s] : _keyState) {
        if (s == InputState::PRESSED) { pressed = static_cast<::Keys>(k); return true; }
    }
    return false;
}
bool KeyInput::IsAnyButtonPressed(std::optional<Buttons>& pressed) {
    pressed = std::nullopt;
    for (auto& playerState : _controllerState) {
        for (auto& [b, s] : playerState) {
            if (s == InputState::PRESSED) {
                pressed = static_cast<Buttons>(b);
                return true;
            }
        }
    }
    return false;
}
void KeyInput::SwapFaceButtons() { FaceButtonsSwitched = !FaceButtonsSwitched; }

} // namespace AnodyneSharp::Input
