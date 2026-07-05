#pragma once
#include "AnodyneSharp/Common.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Drawing/SpriteDrawer.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "AnodyneSharp/GameTimes.hpp"
#include "AnodyneSharp/Logger/DebugLogger.hpp"
#include <unordered_map>
#include <vector>
#include <optional>
#include <algorithm>
#include <numeric>
#include <string>
#include <sstream>

namespace AnodyneSharp::UI::Font {

// ---------------------------------------------------------------------------
// TextCharacter — one rendered glyph on screen
// ---------------------------------------------------------------------------
struct TextCharacter {
    std::optional<char>     Character;  // nullopt = button sprite
    float                   X;
    std::optional<Rectangle> Crop;

    TextCharacter(std::optional<char> character, float x, std::optional<Rectangle> crop)
        : Character(character), X(x), Crop(crop) {}
};

// ---------------------------------------------------------------------------
// SpriteFont — character-to-source-rect lookup backed by a texture
// ---------------------------------------------------------------------------
class SpriteFont {
public:
    const int lineHeight;
    const int spaceWidth;
    Texture2D* texture = nullptr;
    Color color;

    int lineSeparation() const { return lineHeight + 2; }

    SpriteFont(int lineHeight_, int spaceWidth_, const std::string& textureName,
               const std::string& characterOrder, Color col)
        : lineHeight(lineHeight_), spaceWidth(spaceWidth_), color(col)
    {
        texture = Resources::ResourceManager::GetTexture(textureName);
        _rects  = _buildRects(characterOrder);
    }

    std::optional<Rectangle> GetRectangle(char c) const {
        auto it = _rects.find(c);
        return (it != _rects.end()) ? std::optional<Rectangle>(it->second) : std::nullopt;
    }

private:
    std::unordered_map<char, Rectangle> _rects;

    std::unordered_map<char, Rectangle> _buildRects(const std::string& order) {
        std::unordered_map<char, Rectangle> d;
        int texWidth    = (texture && texture->Width > 0) ? texture->Width : 256;
        int charsPerRow = (spaceWidth > 0) ? texWidth / spaceWidth : 1;
        int i = 0;
        for (char c : order) {
            if (c != ' ' && !d.count(c)) {
                int ix = (i % charsPerRow) * spaceWidth;
                int iy = (i / charsPerRow) * lineHeight;
                d[c] = { ix, iy, spaceWidth, lineHeight };
            }
            ++i;
        }
        return d;
    }
};

// ---------------------------------------------------------------------------
// FontManager — factory for SpriteFont based on current language
// ---------------------------------------------------------------------------
class FontManager {
public:
    static const char* LanguageString() {
        // Language enum from GlobalState — include only what we need to avoid
        // circular includes; use raw int comparison here.
        // Language::EN=0, ES=1, ZH_CN=2, KR=3, JP=4
        // Default to English
        return en_string;
    }

    static SpriteFont InitFont(Color color, bool forceEnglish = false) {
        // For simplicity and to avoid pulling in GlobalState here, always
        // return English font.  Full language support can be wired up later.
        return SpriteFont(8, 7, "font-white-apple-7x8", en_string, color);
    }

    static int GetCharacterWidth(bool /*forceEnglish*/ = false) { return 7; }

private:
    static constexpr const char* en_string =
        "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "'1234567890.:,;'\"(!?)+-*/=$[]_";
};

} // namespace AnodyneSharp::UI::Font

// ---------------------------------------------------------------------------
// TextWriter — scrolling text renderer with word-wrap
// Lives in AnodyneSharp::UI to match the original C# namespace.
// ---------------------------------------------------------------------------
namespace AnodyneSharp::UI {

class TextWriter {
public:
    // ----  public state (mirroring C# public fields / properties) -----------
    std::string Text;
    bool  AtEndOfText    = false;
    bool  AtEndOfBox     = false;
    bool  JustWrittenChar = false;
    int   Speed          = DefaultTextSpeed;
    bool  DrawShadow     = false;
    bool  CenterText     = false;
    bool  IgnoreSoftLineBreaks = false;
    float Opacity        = 1.f;
    Drawing::DrawOrder drawLayer = Drawing::DrawOrder::TEXT;

    // ---- constructors -------------------------------------------------------
    TextWriter()                          : TextWriter(0, 0, 200, 200) {}
    TextWriter(int x, int y)              : TextWriter(x, y, 1, 1) {}
    TextWriter(int x, int y, int w, int h){
        Opacity = 1.f; Speed = DefaultTextSpeed;
        _setWriteAreaInternal(x, y + 2, w, h);
        _reset();
    }

    // ---- accessors ----------------------------------------------------------
    int   LinesPerBox()     const { return _fnt ? _writeArea.Height / _fnt->lineSeparation() : 3; }
    bool  FirstLineEmpty()  const { return _lines.empty() || _lines[0].empty(); }
    char  NextCharacter()   const { return (_pos < (int)Text.size()) ? Text[_pos] : '\0'; }
    Point Position()        const { return { _writeArea.X, _writeArea.Y }; }
    Vector2 WriteAreaTopLeft() const { return { (float)_writeArea.X, (float)_writeArea.Y }; }
    Vector2 WriteAreaSize()    const { return { (float)_writeArea.Width, (float)_writeArea.Height }; }
    Rectangle WriteArea()      const { return _writeArea; }
    int   GetLineHeight()   const { return _fnt ? _fnt->lineHeight : 8; }
    int   LineCount()        const { return (int)_lines.size(); }

    void SetWriteArea(Rectangle r) {
        if (_writeArea.X!=r.X||_writeArea.Y!=r.Y||_writeArea.Width!=r.Width||_writeArea.Height!=r.Height)
            _setWriteAreaInternal(r.X, r.Y + 2, r.Width, r.Height);
    }
    void SetWriteArea(int w, int h)               { _writeArea.Width=w; _writeArea.Height=h; }
    void SetWriteArea(int x, int y, int w, int h) { _setWriteAreaInternal(x, y+2, w, h); }

    void SetSpriteFont(Font::SpriteFont* font, Texture2D* buttons) { _fnt=font; _btn=buttons; }
    void SetColor(Color c) { if (_fnt) _fnt->color = c; }

    // ---- text progress ------------------------------------------------------
    void ResetTextProgress() { _reset(); }
    void SkipCharacter()  { _pos++; _syncFlags(); }

    void ProgressText() {
        if (!AtEndOfBox && !AtEndOfText) { _write(Text[_pos]); _syncFlags(); }
    }
    void ProgressTextToEnd() { while (!AtEndOfText && !AtEndOfBox) ProgressText(); }

    void Update() {
        JustWrittenChar = false;
        if (!AtEndOfBox && !AtEndOfText) {
            _step += GameTimes::DeltaTime() * Speed;
            while (_step >= 1.f) { _step -= 1.f; ProgressText(); JustWrittenChar = true; }
        }
    }

    // ---- layout helpers -----------------------------------------------------
    float GetTextLength()    const { return _fnt ? (float)Text.size() * _fnt->spaceWidth : 0; }
    float MaxLineWidth()     const {
        if (!_fnt) return 0;
        size_t mx = 0;
        for (auto& l : _splitLines(Text)) mx = std::max(mx, l.size());
        return (float)mx * _fnt->spaceWidth;
    }
    int   TotalTextHeight()  const {
        if (!_fnt) return 0;
        return (int)_splitLines(Text).size() * _fnt->lineSeparation();
    }
    float GetLineWidth(int line) const {
        if (!_fnt) return 0;
        auto ls = _splitLines(Text);
        return (line < (int)ls.size()) ? (float)ls[line].size() * _fnt->spaceWidth : 0;
    }
    std::vector<std::string> LinesOverLength(int maxLen) const {
        std::vector<std::string> out;
        if (!_fnt) return out;
        for (auto& l : _splitLines(Text))
            if ((int)(l.size() * _fnt->spaceWidth) > maxLen) out.push_back(l);
        return out;
    }

    // ---- scroll helpers -----------------------------------------------------
    void RemoveFirstLine() {
        if (!_lines.empty()) _lines.erase(_lines.begin());
        if (_line > 0) --_line;
        if (_lines.empty()) _lineW = 0;  // reset X cursor when page is cleared
        _syncFlags();  // re-evaluate AtEndOfBox so writing can resume
    }
    void PushTextUp() { _firstY -= _fnt ? _fnt->lineSeparation() / 2.f : 5.f; }

    // ---- drawing ------------------------------------------------------------
    void Draw() {
        if (!_fnt) return;
        float z  = Drawing::DrawingUtilities::GetDrawingZ(drawLayer);
        float sz = z + 0.01f;
        float cy = _firstY;
        // button sprite dimensions (from GameConstants: BUTTON_WIDTH=13, BUTTON_HEIGHT=14)
        static constexpr float BUTTON_HEIGHT = 14.f;
        static constexpr float SCREEN_WIDTH  = 160.f;
        for (int i = 0; i < (int)_lines.size(); i++) {
            for (auto& c : _lines[i]) {
                if (!c.Character.has_value()) {
                    if (_btn && c.Crop) {
                        //Rectangle cr = *c.Crop;
                        Drawing::SpriteDrawer::DrawSprite(_btn,
                            WriteAreaTopLeft() + Vector2{c.X, cy - BUTTON_HEIGHT / 4.f},
                            nullptr, nullptr, 0.f, 1.f, z);
                    }
                } else {
                    Vector2 pos;
                    if (CenterText) {
                        float lw = GetLineWidth(i);
                        pos = WriteAreaTopLeft()
                            + Vector2{c.X, cy}
                            + Vector2{SCREEN_WIDTH / 2.f - lw / 2.f, 0};
                    } else {
                        pos = WriteAreaTopLeft() + Vector2{c.X, cy};
                    }
                    if (c.Crop) {
                        Rectangle cr = *c.Crop;
                        Color col = _fnt->color;
                        col.A = (uint8_t)(col.A * Opacity);
                        Drawing::SpriteDrawer::DrawSprite(_fnt->texture, pos, &cr, &col, 0.f, 1.f, z);
                        if (DrawShadow) {
                            Color blk = Color::Black;
                            blk.A = (uint8_t)(blk.A * Opacity);
                            Drawing::SpriteDrawer::DrawSprite(_fnt->texture, pos + Vector2{0,1}, &cr, &blk, 0.f, 1.f, sz);
                        }
                    }
                }
            }
            cy += _fnt->lineSeparation();
        }
    }

private:
    static constexpr int DefaultTextSpeed = 30;

    Font::SpriteFont* _fnt = nullptr;
    Texture2D*        _btn = nullptr;

    std::vector<std::vector<Font::TextCharacter>> _lines;
    Rectangle _writeArea = {0, 0, 200, 200};
    int   _line   = 0;
    float _lineW  = 0;
    float _step   = 0;
    bool  _newWord= true;
    int   _pos    = 0;
    float _firstY = 0;

    void _setWriteAreaInternal(int x, int y, int w, int h) {
        _writeArea = {x, y, w, h};
    }
    void _reset() {
        _lineW=0; _pos=0; _step=0; _newWord=true;
        _lines.clear(); _line=0; _firstY=0;
        _syncFlags();
    }
    void _syncFlags() {
        AtEndOfText = (_pos >= (int)Text.size());
        AtEndOfBox  = (_line >= LinesPerBox());
    }

    static std::vector<std::string> _splitLines(const std::string& t) {
        std::vector<std::string> v; std::istringstream ss(t); std::string l;
        while (std::getline(ss, l)) { if (!l.empty() && l.back()=='\r') l.pop_back(); v.push_back(l); }
        if (v.empty()) v.push_back("");
        return v;
    }

    float _wordLen() const {
        if (!_fnt) return 0;
        auto idx = Text.find_first_of(" ¶\n", _pos);
        int len = (idx!=std::string::npos) ? (int)(idx-_pos) : (int)(Text.size()-_pos);
        return (float)len * _fnt->spaceWidth;
    }

    void _ensureLine() {
        while ((int)_lines.size()-1 < _line) _lines.push_back({});
    }

    bool _keepInBounds(const Rectangle& r) {
        _ensureLine();
        if (_lines[_line].empty()) return true;
        bool nl;
        if (_newWord) {
            float wl = _wordLen();
            nl = (wl > _writeArea.Width)
                ? (_lineW + r.Width > _writeArea.Width)
                : (_lineW + wl     > _writeArea.Width);
            _newWord = false;
        } else {
            nl = (_lineW + r.Width > _writeArea.Width);
        }
        if (nl) { _lineW = 0; _line++; return _line < LinesPerBox(); }
        else if (_pos > 0 && _fnt) { _lineW += _fnt->spaceWidth; }
        return true;
    }

    void _doSpace()  { _newWord=true; if(_fnt) _lineW+=_fnt->spaceWidth; _pos++; }
    void _newLine()  { _lineW=0; _line++; _pos++; if(_pos<(int)Text.size()&&Text[_pos]==' ') _pos++; }

    void _write(char ch) {
        static const std::string softBreaks = ".!?";
        _ensureLine();

        if (ch == ' ') { _doSpace(); return; }
        if (ch == '\n' || ch == '\r') { _newLine(); return; }
        // '^' is Anodyne's hard page-break: force AtEndOfBox so DialogueState page-flips
        if (ch == '^') { _line = LinesPerBox(); _pos++; _syncFlags(); return; }

        if (!_fnt) { _pos++; return; }
        auto rect = _fnt->GetRectangle(ch);
        if (!rect) {
            DebugLogger::AddError(std::string("Missing char: ") + ch, false);
            _pos++; return;
        }
        if (!_keepInBounds(*rect)) return;
        _ensureLine();
        _lines[_line].push_back(Font::TextCharacter{ch, _lineW, rect});
        _pos++;

        // soft line break after sentence-ending punctuation followed by space
        if (!IgnoreSoftLineBreaks && softBreaks.find(ch) != std::string::npos
            && _pos < (int)Text.size() && Text[_pos] == ' ')
            _newLine();
    }
};

} // namespace AnodyneSharp::UI

using AnodyneSharp::UI::Font::TextCharacter;
using AnodyneSharp::UI::Font::SpriteFont;
using AnodyneSharp::UI::Font::FontManager;
using AnodyneSharp::UI::TextWriter;
