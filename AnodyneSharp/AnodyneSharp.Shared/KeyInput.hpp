#pragma once

#include <string>
#include <vector>
#include <array>
#include <map>

namespace AnodyneSharp
{
    enum class Keys
    {
        F11,
        F12,
        Up,
        Right,
        Down,
        Left,
        W,
        A,
        S,
        D,
        C,
        X,
        Space,
        Enter,
        Escape,
        PageDown,
        PageUp,
        RightShift,
        D1,
        D2,
        D3,
        D4,
        F4,
        F5
    };

    enum class KeyFunctions
    {
        Up,
        Down,
        Left,
        Right,
        Accept,
        Cancel,
        Pause,
        PreviousPage,
        NextPage,
        Broom1,
        Broom2,
        Broom3,
        Broom4,
        QuickSave,
        QuickLoad
    };

    struct RebindableKey
    {
        std::vector<Keys> KeysList;
    };

    class KeyInput
    {
    public:
        static void Update();
        static bool JustPressedKey(Keys key);
        static bool JustPressedRebindableKey(KeyFunctions functionName);
        static bool IsRebindableKeyPressed(KeyFunctions functionName);
        static bool LoadInputConfig(const std::string& filePath);
        static bool SaveInputConfig(const std::string& filePath);
        static std::map<KeyFunctions, RebindableKey> RebindableKeys;

    private:
        static constexpr int KeyCount = static_cast<int>(Keys::F5) + 1;
        static std::array<bool, KeyCount> _previousKeyState;
        static std::array<bool, KeyCount> _currentKeyState;
        static std::vector<Keys> _pressedKeys;
        static bool StringToKey(const std::string& input, Keys& output);
        static int KeyToScancode(Keys key);
        static std::string KeyToString(Keys key);
        static bool TryParseKey(int keyId, Keys& output);
    };
}
