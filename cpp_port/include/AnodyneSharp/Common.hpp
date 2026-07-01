#pragma once
// Master include for all XNA stubs + common C++ headers used throughout the port
#include "XNA/Framework.hpp"
#include "XNA/Graphics.hpp"
#include "XNA/Input.hpp"
#include "XNA/Audio.hpp"
#include "XNA/Media.hpp"
#include "AnodyneSharp/Coroutine.hpp"
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <array>
#include <variant>
#include <chrono>
#include <random>
#include <sstream>
#include <format>
#include <stdexcept>
#include <type_traits>
#include <iostream>

// ---------------------------------------------------------------
// Guid (simple UUID type)
// ---------------------------------------------------------------
struct Guid {
    uint64_t Hi = 0, Lo = 0;
    bool operator==(const Guid& o) const { return Hi==o.Hi && Lo==o.Lo; }
    bool operator!=(const Guid& o) const { return !(*this==o); }
    bool operator<(const Guid& o) const { return Hi<o.Hi||(Hi==o.Hi&&Lo<o.Lo); }
    static Guid NewGuid() {
        static uint64_t counter = 1;
        return {counter++, counter++};
    }
    static Guid Empty;
    std::string ToString() const { return std::format("{:016x}{:016x}", Hi, Lo); }

    // Parse standard UUID string: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    static Guid Parse(const std::string& s) {
        std::string clean;
        for (char c : s) if (c != '-') clean += c;
        if (clean.size() != 32) return Empty;
        try {
            uint64_t hi = std::stoull(clean.substr(0,  16), nullptr, 16);
            uint64_t lo = std::stoull(clean.substr(16, 16), nullptr, 16);
            return {hi, lo};
        } catch (...) { return Empty; }
    }

    // Nested hasher so unordered_map<Guid, V, Guid::Hash> works
    struct Hash {
        size_t operator()(const Guid& g) const {
            return std::hash<uint64_t>{}(g.Hi) ^ (std::hash<uint64_t>{}(g.Lo) << 1);
        }
    };
};

// Also specialise std::hash<Guid> for convenience
template<> struct std::hash<Guid> {
    size_t operator()(const Guid& g) const { return Guid::Hash{}(g); }
};

// ---------------------------------------------------------------
// DateTime / TimeSpan utilities
// ---------------------------------------------------------------
struct DateTime {
    std::chrono::system_clock::time_point tp;
    DateTime() : tp(std::chrono::system_clock::now()) {}
    DateTime(std::chrono::system_clock::time_point t) : tp(t) {}
    static DateTime Now() { return {std::chrono::system_clock::now()}; }
    TimeSpan operator-(const DateTime& o) const {
        auto diff = tp - o.tp;
        auto ticks = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 100;
        return TimeSpan{ticks};
    }
};

// ---------------------------------------------------------------
// Random number generator (mirrors C# Random)
// ---------------------------------------------------------------
class Random {
    std::mt19937_64 _rng;
public:
    Random() : _rng(std::random_device{}()) {}
    Random(int seed) : _rng(seed) {}
    int Next() { return std::uniform_int_distribution<int>{}(_rng); }
    int Next(int maxValue) { return std::uniform_int_distribution<int>{0, maxValue-1}(_rng); }
    int Next(int minValue, int maxValue) {
        if (minValue >= maxValue) return minValue;
        return std::uniform_int_distribution<int>{minValue, maxValue-1}(_rng);
    }
    double NextDouble() { return std::uniform_real_distribution<double>{0.0, 1.0}(_rng); }
};

// ---------------------------------------------------------------
// Math helpers (mirrors C# Math / MathF)
// ---------------------------------------------------------------
namespace MathF {
    inline float Abs(float v) { return std::abs(v); }
    inline float Sqrt(float v) { return std::sqrt(v); }
    inline float Sin(float v) { return std::sin(v); }
    inline float Cos(float v) { return std::cos(v); }
    inline float Tan(float v) { return std::tan(v); }
    inline float Atan2(float y, float x) { return std::atan2(y, x); }
    inline float Floor(float v) { return std::floor(v); }
    inline float Ceiling(float v) { return std::ceil(v); }
    inline float Round(float v) { return std::round(v); }
    inline float Pow(float b, float e) { return std::pow(b, e); }
    inline float Log(float v) { return std::log(v); }
    inline float Max(float a, float b) { return std::max(a, b); }
    inline float Min(float a, float b) { return std::min(a, b); }
    constexpr float PI = 3.14159265358979323846f;
}

namespace Math {
    inline int Abs(int v) { return std::abs(v); }
    inline float Abs(float v) { return std::abs(v); }
    inline double Abs(double v) { return std::abs(v); }
    inline int Max(int a, int b) { return std::max(a, b); }
    inline int Min(int a, int b) { return std::min(a, b); }
    inline float Max(float a, float b) { return std::max(a, b); }
    inline float Min(float a, float b) { return std::min(a, b); }
    template<typename T> T Clamp(T v, T lo, T hi) { return std::clamp(v, lo, hi); }
    constexpr double PI = 3.14159265358979323846;
    inline float Sqrt(float v) { return std::sqrt(v); }
    inline double Sqrt(double v) { return std::sqrt(v); }
}

// ---------------------------------------------------------------
// String utilities
// ---------------------------------------------------------------
inline bool IsNullOrWhiteSpace(const std::string& s) {
    return s.empty() || std::all_of(s.begin(), s.end(), ::isspace);
}

inline std::string StringFormat(const std::string& fmt) { return fmt; }

// ---------------------------------------------------------------
// Enum.GetNames equivalent
// ---------------------------------------------------------------
// For specific enums, we provide manual name arrays (reflection not available in C++)

// ---------------------------------------------------------------
// Action / Func types
// ---------------------------------------------------------------
using Action = std::function<void()>;
template<typename T> using ActionT = std::function<void(T)>;
template<typename T, typename U> using ActionT2 = std::function<void(T, U)>;
template<typename R> using FuncR = std::function<R()>;
template<typename T, typename R> using FuncTR = std::function<R(T)>;

// ---------------------------------------------------------------
// IEnumerable<Entity*> helper
// ---------------------------------------------------------------
// We use std::vector<T*> everywhere for IEnumerable<T> in C++

// ---------------------------------------------------------------
// Null-safe pointer invoke helper (mirrors C# ?. operator)
// ---------------------------------------------------------------
template<typename T, typename F>
auto NullSafeInvoke(T* ptr, F&& fn) -> decltype(fn(ptr)) {
    if (ptr) return fn(ptr);
    return {};
}

// ---------------------------------------------------------------
// XNA inline constant definitions
// ---------------------------------------------------------------
inline const Vector2 Vector2::Zero = {0.f, 0.f};
inline const Vector2 Vector2::One  = {1.f, 1.f};
inline const Vector2 Vector2::UnitX = {1.f, 0.f};
inline const Vector2 Vector2::UnitY = {0.f, 1.f};

inline const Vector3 Vector3::Zero    = {0.f,0.f,0.f};
inline const Vector3 Vector3::One     = {1.f,1.f,1.f};
inline const Vector3 Vector3::Up      = {0.f,1.f,0.f};
inline const Vector3 Vector3::Forward = {0.f,0.f,-1.f};

inline Rectangle Rectangle::Empty = {};
inline Guid Guid::Empty = {0, 0};

inline const Color Color::White       = {255,255,255,255};
inline const Color Color::Black       = {0,0,0,255};
inline const Color Color::Red         = {255,0,0,255};
inline const Color Color::Green       = {0,255,0,255};
inline const Color Color::Blue        = {0,0,255,255};
inline const Color Color::Yellow      = {255,255,0,255};
inline const Color Color::Cyan        = {0,255,255,255};
inline const Color Color::Magenta     = {255,0,255,255};
inline const Color Color::Transparent = {0,0,0,0};
inline const Color Color::LightBlue   = {173,216,230,255};
inline const Color Color::Gray        = {128,128,128,255};
inline const Color Color::DarkGray    = {64,64,64,255};
inline const Color Color::Orange      = {255,165,0,255};
inline const Color Color::Purple      = {128,0,128,255};
inline const Color Color::Pink        = {255,192,203,255};
inline const Color Color::Brown       = {139,69,19,255};
inline const Color Color::LightGreen  = {144,238,144,255};
inline const Color Color::DarkGreen   = {0,100,0,255};
inline const Color Color::Navy        = {0,0,128,255};
inline const Color Color::Teal        = {0,128,128,255};
inline const Color Color::Maroon      = {128,0,0,255};
inline const Color Color::Olive       = {128,128,0,255};
inline const Color Color::Silver      = {192,192,192,255};
inline const Color Color::Aqua        = {0,255,255,255};
inline const Color Color::Fuchsia     = {255,0,255,255};
inline const Color Color::Lime        = {0,255,0,255};
inline const Color Color::Coral       = {255,127,80,255};
inline const Color Color::Salmon      = {250,128,114,255};
inline const Color Color::Gold        = {255,215,0,255};
inline const Color Color::Goldenrod   = {218,165,32,255};
inline const Color Color::Violet      = {238,130,238,255};
inline const Color Color::Indigo      = {75,0,130,255};
inline const Color Color::Ivory       = {255,255,240,255};
inline const Color Color::Khaki       = {240,230,140,255};

inline BlendState BlendState::AlphaBlend = {};
inline BlendState BlendState::Additive = {};
inline BlendState BlendState::NonPremultiplied = {};
inline BlendState BlendState::Opaque = {};

inline SamplerState SamplerState::PointClamp = {};
inline SamplerState SamplerState::PointWrap = {};
inline SamplerState SamplerState::LinearClamp = {};
inline SamplerState SamplerState::LinearWrap = {};
inline SamplerState SamplerState::AnisotropicClamp = {};
inline SamplerState SamplerState::AnisotropicWrap = {};

inline GraphicsAdapter GraphicsAdapter::DefaultAdapter = {};
