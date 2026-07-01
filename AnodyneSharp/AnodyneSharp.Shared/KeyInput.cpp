#include "KeyInput.hpp"
#include <SDL2/SDL.h>
// TODO: This input stub is temporary. Replace with the C# KeyInput and input mapping logic from Input/KeyInput.cs.
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace AnodyneSharp
{
    std::array<bool, KeyInput::KeyCount> KeyInput::_previousKeyState{};
    std::array<bool, KeyInput::KeyCount> KeyInput::_currentKeyState{};
    std::vector<Keys> KeyInput::_pressedKeys;
    std::map<KeyFunctions, RebindableKey> KeyInput::RebindableKeys;

    bool KeyInput::StringToKey(const std::string& input, Keys& output)
    {
        if (input.empty())
        {
            output = Keys::Enter;
            return true;
        }

        if (input == "up" || input == "w")
        {
            output = Keys::Up;
            return true;
        }
        if (input == "down" || input == "s")
        {
            output = Keys::Down;
            return true;
        }
        if (input == "left" || input == "a")
        {
            output = Keys::Left;
            return true;
        }
        if (input == "right" || input == "d")
        {
            output = Keys::Right;
            return true;
        }
        if (input == "enter")
        {
            output = Keys::Enter;
            return true;
        }
        if (input == "escape" || input == "esc")
        {
            output = Keys::Escape;
            return true;
        }
        if (input == "c")
        {
            output = Keys::C;
            return true;
        }
        if (input == "x")
        {
            output = Keys::X;
            return true;
        }
        if (input == "f11")
        {
            output = Keys::F11;
            return true;
        }
        if (input == "f12")
        {
            output = Keys::F12;
            return true;
        }
        if (input == "f4")
        {
            output = Keys::F4;
            return true;
        }
        if (input == "f5")
        {
            output = Keys::F5;
            return true;
        }
        return false;
    }

    void KeyInput::Update()
    {
        _previousKeyState = _currentKeyState;

        SDL_PumpEvents();
        const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
        for (int i = 0; i < KeyCount; ++i)
        {
            int scancode = KeyToScancode(static_cast<Keys>(i));
            _currentKeyState[i] = scancode != SDL_SCANCODE_UNKNOWN && keyboardState[scancode] != 0;
        }
    }

    bool KeyInput::JustPressedKey(Keys key)
    {
        const size_t index = static_cast<size_t>(key);
        return _currentKeyState[index] && !_previousKeyState[index];
    }

    bool KeyInput::JustPressedRebindableKey(KeyFunctions functionName)
    {
        auto it = RebindableKeys.find(functionName);
        if (it == RebindableKeys.end())
        {
            return false;
        }

        for (Keys key : it->second.KeysList)
        {
            if (JustPressedKey(key))
            {
                return true;
            }
        }

        return false;
    }

    bool KeyInput::IsRebindableKeyPressed(KeyFunctions functionName)
    {
        auto it = RebindableKeys.find(functionName);
        if (it == RebindableKeys.end())
        {
            return false;
        }

        for (Keys key : it->second.KeysList)
        {
            if (_currentKeyState[static_cast<size_t>(key)])
            {
                return true;
            }
        }

        return false;
    }

    int KeyInput::KeyToScancode(Keys key)
    {
        switch (key)
        {
        case Keys::F11:
            return SDL_SCANCODE_F11;
        case Keys::F12:
            return SDL_SCANCODE_F12;
        case Keys::Up:
            return SDL_SCANCODE_UP;
        case Keys::Right:
            return SDL_SCANCODE_RIGHT;
        case Keys::Down:
            return SDL_SCANCODE_DOWN;
        case Keys::Left:
            return SDL_SCANCODE_LEFT;
        case Keys::W:
            return SDL_SCANCODE_W;
        case Keys::A:
            return SDL_SCANCODE_A;
        case Keys::S:
            return SDL_SCANCODE_S;
        case Keys::D:
            return SDL_SCANCODE_D;
        case Keys::C:
            return SDL_SCANCODE_C;
        case Keys::X:
            return SDL_SCANCODE_X;
        case Keys::Space:
            return SDL_SCANCODE_SPACE;
        case Keys::Enter:
            return SDL_SCANCODE_RETURN;
        case Keys::Escape:
            return SDL_SCANCODE_ESCAPE;
        case Keys::PageDown:
            return SDL_SCANCODE_PAGEDOWN;
        case Keys::PageUp:
            return SDL_SCANCODE_PAGEUP;
        case Keys::RightShift:
            return SDL_SCANCODE_RSHIFT;
        case Keys::D1:
            return SDL_SCANCODE_1;
        case Keys::D2:
            return SDL_SCANCODE_2;
        case Keys::D3:
            return SDL_SCANCODE_3;
        case Keys::D4:
            return SDL_SCANCODE_4;
        case Keys::F4:
            return SDL_SCANCODE_F4;
        case Keys::F5:
            return SDL_SCANCODE_F5;
        default:
            return SDL_SCANCODE_UNKNOWN;
        }
    }

    std::string KeyInput::KeyToString(Keys key)
    {
        return std::to_string(static_cast<int>(key));
    }

    bool KeyInput::TryParseKey(int keyId, Keys& output)
    {
        if (keyId < 0 || keyId >= KeyCount)
        {
            return false;
        }
        output = static_cast<Keys>(keyId);
        return true;
    }

    static std::string TrimString(const std::string& value)
    {
        size_t start = 0;
        while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
        {
            ++start;
        }
        size_t end = value.size();
        while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
        {
            --end;
        }
        return value.substr(start, end - start);
    }

    static bool ReadIntLine(std::istream& stream, int& result)
    {
        std::string line;
        while (std::getline(stream, line))
        {
            line = TrimString(line);
            if (line.empty())
            {
                continue;
            }
            try
            {
                result = std::stoi(line);
                return true;
            }
            catch (...) {}
            return false;
        }
        return false;
    }

    static bool ReadBlock(std::istream& stream, std::vector<int>& values)
    {
        std::string line;
        if (!std::getline(stream, line))
        {
            return false;
        }
        if (TrimString(line) != "{")
        {
            return false;
        }
        while (std::getline(stream, line))
        {
            line = TrimString(line);
            if (line.empty())
            {
                continue;
            }
            if (line == "}")
            {
                return true;
            }
            try
            {
                values.push_back(std::stoi(line));
            }
            catch (...) {
                return false;
            }
        }
        return false;
    }

    bool KeyInput::LoadInputConfig(const std::string& filePath)
    {
        std::ifstream input(filePath);
        if (!input)
        {
            return false;
        }

        std::map<KeyFunctions, RebindableKey> loadedKeys;
        while (true)
        {
            int functionId;
            if (!ReadIntLine(input, functionId))
            {
                break;
            }

            std::string line;
            if (!std::getline(input, line) || TrimString(line) != "{")
            {
                return false;
            }

            std::vector<int> keyIds;
            std::vector<int> buttonIds;
            while (std::getline(input, line))
            {
                line = TrimString(line);
                if (line.empty())
                {
                    continue;
                }
                if (line == "}")
                {
                    break;
                }
                if (line == "k")
                {
                    if (!ReadBlock(input, keyIds))
                    {
                        return false;
                    }
                    continue;
                }
                if (line.rfind("b", 0) == 0)
                {
                    if (!ReadBlock(input, buttonIds))
                    {
                        return false;
                    }
                    continue;
                }
            }

            RebindableKey rKey;
            for (int keyId : keyIds)
            {
                Keys key;
                if (TryParseKey(keyId, key))
                {
                    rKey.KeysList.push_back(key);
                }
            }
            loadedKeys.emplace(static_cast<KeyFunctions>(functionId), std::move(rKey));
        }

        if (!loadedKeys.empty())
        {
            RebindableKeys = std::move(loadedKeys);
            return true;
        }
        return false;
    }

    bool KeyInput::SaveInputConfig(const std::string& filePath)
    {
        std::ofstream output(filePath, std::ios::binary);
        if (!output)
        {
            return false;
        }

        for (const auto& [functionName, rebindableKey] : RebindableKeys)
        {
            output << static_cast<int>(functionName) << "\n";
            output << "{\n";
            output << "\tk\n";
            output << "\t{\n";
            for (Keys key : rebindableKey.KeysList)
            {
                output << "\t\t" << static_cast<int>(key) << "\n";
            }
            output << "\t}\n";
            output << "\tb\t-1\n";
            output << "\t{\n";
            output << "\t}\n";
            output << "}\n";
        }
        return true;
    }
}
