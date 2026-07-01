#pragma once
#include <cmath>
#include <algorithm>
#include <string>
#include <memory>
#include <functional>
#include <optional>
#include <chrono>
#include <random>

namespace Microsoft { namespace Xna { namespace Framework {

// ---------------------------------------------------------------
// TimeSpan
// ---------------------------------------------------------------
struct TimeSpan {
    long long Ticks = 0; // 100-nanosecond intervals
    double TotalSeconds() const { return Ticks / 10000000.0; }
    double TotalMilliseconds() const { return Ticks / 10000.0; }
    double TotalMinutes() const { return TotalSeconds() / 60.0; }
    static TimeSpan FromSeconds(double s) { return {static_cast<long long>(s * 10000000)}; }
    static TimeSpan FromMilliseconds(double ms) { return {static_cast<long long>(ms * 10000)}; }
    TimeSpan operator+(const TimeSpan& o) const { return {Ticks + o.Ticks}; }
    TimeSpan operator-(const TimeSpan& o) const { return {Ticks - o.Ticks}; }
    bool operator==(const TimeSpan& o) const { return Ticks == o.Ticks; }
    bool operator!=(const TimeSpan& o) const { return Ticks != o.Ticks; }
};

// ---------------------------------------------------------------
// GameTime
// ---------------------------------------------------------------
struct GameTime {
    TimeSpan ElapsedGameTime;
    TimeSpan TotalGameTime;
    bool IsRunningSlowly = false;
};

// ---------------------------------------------------------------
// Vector2
// ---------------------------------------------------------------
struct Vector2 {
    float X = 0.f, Y = 0.f;

    Vector2() = default;
    Vector2(float x, float y) : X(x), Y(y) {}
    explicit Vector2(float v) : X(v), Y(v) {}

    static const Vector2 Zero;
    static const Vector2 One;
    static const Vector2 UnitX;
    static const Vector2 UnitY;

    Vector2 operator+(const Vector2& o) const { return {X+o.X, Y+o.Y}; }
    Vector2 operator-(const Vector2& o) const { return {X-o.X, Y-o.Y}; }
    Vector2 operator*(float s) const { return {X*s, Y*s}; }
    Vector2 operator*(const Vector2& o) const { return {X*o.X, Y*o.Y}; }
    Vector2 operator/(float s) const { return {X/s, Y/s}; }
    Vector2 operator/(const Vector2& o) const { return {X/o.X, Y/o.Y}; }
    Vector2 operator-() const { return {-X, -Y}; }
    Vector2& operator+=(const Vector2& o) { X+=o.X; Y+=o.Y; return *this; }
    Vector2& operator-=(const Vector2& o) { X-=o.X; Y-=o.Y; return *this; }
    Vector2& operator*=(float s) { X*=s; Y*=s; return *this; }
    Vector2& operator/=(float s) { X/=s; Y/=s; return *this; }
    bool operator==(const Vector2& o) const { return X==o.X && Y==o.Y; }
    bool operator!=(const Vector2& o) const { return !(*this==o); }

    float Length() const { return std::sqrt(X*X+Y*Y); }
    float LengthSquared() const { return X*X+Y*Y; }
    void Normalize() {
        float len = Length();
        if (len > 0.f) { X/=len; Y/=len; }
    }
    static Vector2 Normalize(const Vector2& v) { Vector2 r=v; r.Normalize(); return r; }
    static float Dot(const Vector2& a, const Vector2& b) { return a.X*b.X+a.Y*b.Y; }
    static float Distance(const Vector2& a, const Vector2& b) { return (a-b).Length(); }
    static Vector2 Lerp(const Vector2& a, const Vector2& b, float t) { return a+(b-a)*t; }

    struct Point ToPoint() const;
};

inline Vector2 operator*(float s, const Vector2& v) { return v*s; }

// ---------------------------------------------------------------
// Vector3
// ---------------------------------------------------------------
struct Vector3 {
    float X = 0.f, Y = 0.f, Z = 0.f;

    Vector3() = default;
    Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}
    Vector3(const Vector2& v, float z) : X(v.X), Y(v.Y), Z(z) {}

    static const Vector3 Zero;
    static const Vector3 One;
    static const Vector3 Up;
    static const Vector3 Forward;

    Vector3 operator+(const Vector3& o) const { return {X+o.X,Y+o.Y,Z+o.Z}; }
    Vector3 operator-(const Vector3& o) const { return {X-o.X,Y-o.Y,Z-o.Z}; }
    Vector3 operator*(float s) const { return {X*s,Y*s,Z*s}; }
    Vector3 operator/(float s) const { return {X/s,Y/s,Z/s}; }
    Vector3 operator-() const { return {-X,-Y,-Z}; }
    Vector3& operator+=(const Vector3& o) { X+=o.X;Y+=o.Y;Z+=o.Z; return *this; }
    bool operator==(const Vector3& o) const { return X==o.X&&Y==o.Y&&Z==o.Z; }
    float& operator[](int i) { return (&X)[i]; }
    const float& operator[](int i) const { return (&X)[i]; }

    float Length() const { return std::sqrt(X*X+Y*Y+Z*Z); }
    Vector2 ToVector2() const { return {X,Y}; }
};

// ---------------------------------------------------------------
// Vector4
// ---------------------------------------------------------------
struct Vector4 {
    float X=0,Y=0,Z=0,W=0;
    Vector4() = default;
    Vector4(float x,float y,float z,float w):X(x),Y(y),Z(z),W(w){}
};

// ---------------------------------------------------------------
// Point
// ---------------------------------------------------------------
struct Point {
    int X = 0, Y = 0;
    Point() = default;
    Point(int x, int y) : X(x), Y(y) {}
    bool operator==(const Point& o) const { return X==o.X&&Y==o.Y; }
    bool operator!=(const Point& o) const { return !(*this==o); }
    bool operator<(const Point& o) const { return X<o.X||(X==o.X&&Y<o.Y); }
    Vector2 ToVector2() const { return {(float)X,(float)Y}; }
    Point operator*(const Point& o) const { return {X*o.X,Y*o.Y}; }
    Point operator+(const Point& o) const { return {X+o.X,Y+o.Y}; }
    Point operator-(const Point& o) const { return {X-o.X,Y-o.Y}; }

    // Size alias helpers (Rectangle.Size returns Point)
    Vector2 ToVector2Checked() const { return {(float)X,(float)Y}; }
};

inline Point Vector2::ToPoint() const { return Point{(int)X,(int)Y}; }

// ---------------------------------------------------------------
// Rectangle
// ---------------------------------------------------------------
struct Rectangle {
    int X=0,Y=0,Width=0,Height=0;

    Rectangle() = default;
    Rectangle(int x,int y,int w,int h):X(x),Y(y),Width(w),Height(h){}
    Rectangle(const Point& loc, const Point& size):X(loc.X),Y(loc.Y),Width(size.X),Height(size.Y){}

    int Left()   const { return X; }
    int Right()  const { return X+Width; }
    int Top()    const { return Y; }
    int Bottom() const { return Y+Height; }
    Point Location() const { return {X,Y}; }
    Point Size()    const { return {Width,Height}; }
    Vector2 Center() const { return {X+Width/2.f, Y+Height/2.f}; }
    bool IsEmpty() const { return Width==0&&Height==0; }

    bool Intersects(const Rectangle& o) const {
        return X < o.X+o.Width && X+Width > o.X &&
               Y < o.Y+o.Height && Y+Height > o.Y;
    }
    bool Contains(int px,int py) const {
        return px>=X && px<X+Width && py>=Y && py<Y+Height;
    }
    bool Contains(const Point& p) const { return Contains(p.X,p.Y); }
    bool Contains(const Vector2& p) const { return Contains((int)p.X,(int)p.Y); }
    bool Contains(const Rectangle& o) const {
        return X<=o.X && Y<=o.Y && X+Width>=o.X+o.Width && Y+Height>=o.Y+o.Height;
    }
    bool operator==(const Rectangle& o) const { return X==o.X&&Y==o.Y&&Width==o.Width&&Height==o.Height; }

    static Rectangle Empty;
};

// ---------------------------------------------------------------
// Color
// ---------------------------------------------------------------
struct Color {
    unsigned char R=255,G=255,B=255,A=255;

    Color() = default;
    Color(unsigned char r,unsigned char g,unsigned char b,unsigned char a=255):R(r),G(g),B(b),A(a){}
    Color(int r,int g,int b,int a=255):R((unsigned char)r),G((unsigned char)g),B((unsigned char)b),A((unsigned char)a){}
    Color(float r,float g,float b,float a=1.f)
        : R((unsigned char)(r*255)),G((unsigned char)(g*255)),
          B((unsigned char)(b*255)),A((unsigned char)(a*255)){}

    Color operator*(float alpha) const {
        return {R,G,B,(unsigned char)(A*alpha)};
    }
    bool operator==(const Color& o) const { return R==o.R&&G==o.G&&B==o.B&&A==o.A; }
    bool operator!=(const Color& o) const { return !(*this==o); }

    // Named colors
    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Cyan;
    static const Color Magenta;
    static const Color Transparent;
    static const Color LightBlue;
    static const Color Gray;
    static const Color DarkGray;
    static const Color Orange;
    static const Color Purple;
    static const Color Pink;
    static const Color Brown;
    static const Color LightGreen;
    static const Color DarkGreen;
    static const Color Navy;
    static const Color Teal;
    static const Color Maroon;
    static const Color Olive;
    static const Color Silver;
    static const Color Aqua;
    static const Color Fuchsia;
    static const Color Lime;
    static const Color Coral;
    static const Color Salmon;
    static const Color Gold;
    static const Color Goldenrod;
    static const Color Violet;
    static const Color Indigo;
    static const Color Ivory;
    static const Color Khaki;
};

// ---------------------------------------------------------------
// Matrix (4x4)
// ---------------------------------------------------------------
struct Matrix {
    float M[4][4] = {};

    Matrix() { for(int i=0;i<4;i++) for(int j=0;j<4;j++) M[i][j]=(i==j?1.f:0.f); }

    static Matrix Identity() { return Matrix{}; }

    static Matrix CreateTranslation(const Vector3& v) {
        Matrix m;
        m.M[3][0]=v.X; m.M[3][1]=v.Y; m.M[3][2]=v.Z;
        return m;
    }
    static Matrix CreateTranslation(float x,float y,float z) { return CreateTranslation({x,y,z}); }

    static Matrix CreateScale(const Vector3& s) {
        Matrix m;
        m.M[0][0]=s.X; m.M[1][1]=s.Y; m.M[2][2]=s.Z;
        return m;
    }
    static Matrix CreateScale(float sx,float sy,float sz) { return CreateScale({sx,sy,sz}); }
    static Matrix CreateScale(float s) { return CreateScale({s,s,s}); }

    static Matrix CreateRotationZ(float angle) {
        Matrix m;
        float c=std::cos(angle), s=std::sin(angle);
        m.M[0][0]=c; m.M[0][1]=s;
        m.M[1][0]=-s; m.M[1][1]=c;
        return m;
    }
    static Matrix CreateRotationX(float angle) {
        Matrix m;
        float c=std::cos(angle), s=std::sin(angle);
        m.M[1][1]=c; m.M[1][2]=s;
        m.M[2][1]=-s; m.M[2][2]=c;
        return m;
    }
    static Matrix CreateRotationY(float angle) {
        Matrix m;
        float c=std::cos(angle), s=std::sin(angle);
        m.M[0][0]=c; m.M[0][2]=-s;
        m.M[2][0]=s; m.M[2][2]=c;
        return m;
    }

    static Matrix CreateLookAt(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector) {
        // Stub - returns identity
        return Matrix{};
    }

    static Matrix CreateOrthographicOffCenter(float left,float right,float bottom,float top,float zNear,float zFar) {
        Matrix m;
        m.M[0][0]=2.f/(right-left);
        m.M[1][1]=2.f/(top-bottom);
        m.M[2][2]=1.f/(zNear-zFar);
        m.M[3][0]=-(right+left)/(right-left);
        m.M[3][1]=-(top+bottom)/(top-bottom);
        m.M[3][2]=zNear/(zNear-zFar);
        m.M[3][3]=1.f;
        return m;
    }

    static Vector2 Transform(const Vector2& v, const Matrix& m) {
        return {v.X*m.M[0][0]+v.Y*m.M[1][0]+m.M[3][0],
                v.X*m.M[0][1]+v.Y*m.M[1][1]+m.M[3][1]};
    }

    Matrix operator*(const Matrix& o) const {
        Matrix r;
        for(int i=0;i<4;i++) for(int j=0;j<4;j++){
            r.M[i][j]=0;
            for(int k=0;k<4;k++) r.M[i][j]+=M[i][k]*o.M[k][j];
        }
        return r;
    }
};

// ---------------------------------------------------------------
// MathHelper
// ---------------------------------------------------------------
struct MathHelper {
    static constexpr float Pi = 3.14159265358979323846f;
    static constexpr float TwoPi = Pi*2;
    static constexpr float PiOver2 = Pi/2;
    static constexpr float PiOver4 = Pi/4;
    static constexpr float E = 2.71828182845904523536f;

    static float Clamp(float v,float min,float max) { return std::clamp(v,min,max); }
    static int   Clamp(int v, int min, int max) { return std::clamp(v,min,max); }
    static float Lerp(float a,float b,float t) { return a+(b-a)*t; }
    static float ToDegrees(float rad) { return rad*(180.f/Pi); }
    static float ToRadians(float deg) { return deg*(Pi/180.f); }
    static float WrapAngle(float angle) {
        angle = std::fmod(angle + Pi, TwoPi);
        if(angle<0) angle+=TwoPi;
        return angle-Pi;
    }
    static float SmoothStep(float a,float b,float t) {
        t=std::clamp((t-a)/(b-a),0.f,1.f);
        return t*t*(3-2*t);
    }
    static float Distance(float a,float b) { return std::abs(b-a); }
    static float Max(float a,float b) { return std::max(a,b); }
    static float Min(float a,float b) { return std::min(a,b); }
};

// ---------------------------------------------------------------
// ContentManager (stub)
// ---------------------------------------------------------------
class ContentManager {
public:
    std::string RootDirectory;
    template<typename T>
    T Load(const std::string& assetName) { return T{}; }
};

// ---------------------------------------------------------------
// GameWindow (stub)
// ---------------------------------------------------------------
class GameWindow {
public:
    std::string Title;
    bool AllowUserResizing = false;
};

// ---------------------------------------------------------------
// Game (base class — SDL3 loop implemented in SDL3Backend.cpp)
// ---------------------------------------------------------------
class Game {
public:
    ContentManager Content;
    GameWindow Window;
    bool IsActive = true;
    bool IsFixedTimeStep = true;

    virtual ~Game() = default;
    virtual void Initialize() {}
    virtual void LoadContent() {}
    virtual void Update(const GameTime& gameTime) {}
    virtual void Draw(const GameTime& gameTime) {}

    void Run();   // implemented in SDL3Backend.cpp
    void Exit() { _running = false; }

protected:
    bool _running = true;
};

}}} // namespace Microsoft::Xna::Framework

// Bring common types into global scope for convenience (mirrors C# using)
using Microsoft::Xna::Framework::Vector2;
using Microsoft::Xna::Framework::Vector3;
using Microsoft::Xna::Framework::Vector4;
using Microsoft::Xna::Framework::Point;
using Microsoft::Xna::Framework::Rectangle;
using Microsoft::Xna::Framework::Color;
using Microsoft::Xna::Framework::Matrix;
using Microsoft::Xna::Framework::GameTime;
using Microsoft::Xna::Framework::TimeSpan;
using Microsoft::Xna::Framework::MathHelper;
using Microsoft::Xna::Framework::ContentManager;
using Microsoft::Xna::Framework::GameWindow;
using Microsoft::Xna::Framework::Game;
