#include "TitleState.hpp"
#include "MainMenuState.hpp"
#include "SpriteDrawer.hpp"
#include <algorithm>
#include <random>

namespace AnodyneSharp
{
namespace States
{
namespace MainMenu
{
    namespace
    {
        struct UIEntity
        {
            int x = 0;
            int y = 0;
            int width = 0;
            int height = 0;
            bool visible = false;
            float opacity = 1.0f;
            float rotation = 0.0f;
            float angularVelocity = 0.0f;
            float velocityY = 0.0f;
        };

        struct CreditLine
        {
            std::string fullText;
            std::string revealedText;
            int x = 0;
            int y = 0;
            float opacity = 1.0f;
        };

        std::mt19937& GetRng()
        {
            static std::mt19937 rng(static_cast<unsigned int>(SDL_GetTicks()));
            return rng;
        }

        int GetCenteredX(const std::string& text, int scale)
        {
            const int charWidth = 5;
            const int spacing = 1;
            const int totalWidth = static_cast<int>(text.size()) * (charWidth + spacing) * scale;
            return std::max(0, (160 - totalWidth) / 2);
        }
    }

    TitleState::TitleState(IStateSetter* stateSetter)
        : _stateSetter(stateSetter)
    {
    }

    void TitleState::Create()
    {
        _phase = Phase::Intro;
        _phaseTimer = 0.0f;
        _textRevealTimer = 0.0f;
        _blinkTimer = 0.0f;
        _pressVisible = false;
        _pixelation = 0.0f;
        _fadeAlpha = 0.0f;
        _secondNames = false;
        _lastBlinkTimestamp = SDL_GetTicks();
        _nextCreditIndex = 0;
        _scrollOffset = 0.0f;
        _titleShown = false;
        _credits = {
            "A pixelated fan remake of the classic cult adventure.",
            "Featuring updated controls and a nostalgic soundtrack.",
            "Press any key to advance through the title sequence."
        };

        _titleLines = {
            "ANODYNE FAN REMAKE",
            "PRESS START"
        };

        _nexusImage = { 0, 180, 160, 116, false, 1.0f, 0.0f, 0.0f, -20.0f };
        _doorGlow = { 48, 197, 64, 32, false, 0.0f, 0.0f, 0.0f, 0.0f };
        _doorSpin1 = { 48, 180, 64, 64, false, 1.0f, 0.0f, 0.0f, 0.0f };
        _doorSpin2 = { 48, 180, 64, 64, false, 1.0f, 0.0f, 0.0f, 0.0f };
        _pressEnter = { 32, 160, 96, 16, false, 1.0f, 0.0f, 0.0f, 0.0f };
        _title = { 16, 16, 128, 48, false, 1.0f, 0.0f, 0.0f, 0.0f };
        _titleOverlay = _title;
        _titleOverlay.visible = false;
        _titleOverlay.opacity = 1.0f;
        _subtitle = { 45, 47, 71, 11, false, 1.0f, 0.0f, 0.0f, 0.0f };
        _subtitleOverlay = _subtitle;
        _subtitleOverlay.visible = false;
        _subtitleOverlay.opacity = 1.0f;

        SetupCreditLines();
    }

    void TitleState::SetupCreditLines()
    {
        _visibleCredits.clear();
        _creditLines.clear();
        _remainingCreditChars.clear();
        int y = 84;

        for (size_t index = 0; index < _credits.size(); ++index)
        {
            const std::string& text = _credits[index];
            int lineX = GetCenteredX(text, 2);
            _creditLines.push_back({ text, std::string(text.size(), ' '), lineX, y, 1.0f });

            for (size_t charIndex = 0; charIndex < text.size(); ++charIndex)
            {
                _remainingCreditChars.emplace_back(index, charIndex);
            }

            y += 18;
        }
    }

    void TitleState::Update()
    {
        const Uint32 now = SDL_GetTicks();
        const float deltaSeconds = (now - _lastBlinkTimestamp) / 1000.0f;
        _lastBlinkTimestamp = now;
        _phaseTimer += deltaSeconds;
        _textRevealTimer += deltaSeconds;
        _blinkTimer += deltaSeconds;

        const bool anyKeyPressed =
            KeyInput::JustPressedRebindableKey(KeyFunctions::Accept) ||
            KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel) ||
            KeyInput::JustPressedRebindableKey(KeyFunctions::Pause);

        if (_phase == Phase::Intro)
        {
            if (_phaseTimer >= 1.5f)
            {
                _phase = Phase::CreditsWrite;
                _phaseTimer = 0.0f;
            }
        }
        else if (_phase == Phase::CreditsWrite)
        {
            if (_textRevealTimer >= 0.03f && !_remainingCreditChars.empty())
            {
                _textRevealTimer = 0.0f;
                std::uniform_int_distribution<size_t> dist(0, _remainingCreditChars.size() - 1);
                const auto [lineIndex, charIndex] = _remainingCreditChars[dist(GetRng())];
                CreditLine& line = _creditLines[lineIndex];
                line.revealedText[charIndex] = line.fullText[charIndex];
                _remainingCreditChars.erase(_remainingCreditChars.begin() + dist(GetRng()));
            }

            if (anyKeyPressed)
            {
                _phase = Phase::PressStart;
                _phaseTimer = 0.0f;
                _pressVisible = true;
                _blinkTimer = 0.0f;
            }
            else if (_remainingCreditChars.empty() && _phaseTimer >= 1.0f)
            {
                _phase = Phase::CreditsFade;
                _phaseTimer = 0.0f;
            }
        }
        else if (_phase == Phase::CreditsFade)
        {
            bool allFaded = true;
            for (auto& line : _creditLines)
            {
                line.opacity = std::max(0.0f, line.opacity - deltaSeconds * 0.7f);
                if (line.opacity > 0.0f)
                {
                    allFaded = false;
                }
            }

            if (anyKeyPressed)
            {
                _phase = Phase::PressStart;
                _phaseTimer = 0.0f;
                _pressVisible = true;
                _blinkTimer = 0.0f;
            }
            else if (allFaded)
            {
                if (!_secondNames)
                {
                    _secondNames = true;
                    _credits = {
                        "A community project celebrating the original game.",
                        "Created to preserve the spirit of Anodyne.",
                        "Thanks for playing this fan remake."
                    };
                    _phase = Phase::CreditsWrite;
                    _phaseTimer = 0.0f;
                    SetupCreditLines();
                }
                else
                {
                    _phase = Phase::ScrollUp;
                    _phaseTimer = 0.0f;
                    _nexusImage.visible = true;
                    _doorGlow.visible = false;
                    _doorSpin1.visible = false;
                    _doorSpin2.visible = false;
                }
            }
        }
        else if (_phase == Phase::ScrollUp)
        {
            _nexusImage.y = static_cast<int>(_nexusImage.y + _nexusImage.velocityY * deltaSeconds);
            _doorGlow.y = _nexusImage.y + 17;
            _doorSpin1.y = _nexusImage.y;
            _doorSpin2.y = _nexusImage.y;

            if (_phaseTimer >= 0.8f || anyKeyPressed || _nexusImage.y <= 64)
            {
                _phase = Phase::DisplayTitle;
                _phaseTimer = 0.0f;
                _title.visible = true;
                _titleOverlay.visible = true;
                _subtitle.visible = true;
                _subtitleOverlay.visible = true;
                _pressEnter.visible = true;
                _doorGlow.visible = true;
                _doorSpin1.visible = true;
                _doorSpin2.visible = true;
                _doorSpin1.angularVelocity = 3.0f;
                _doorSpin2.angularVelocity = -3.0f;
                _doorSpin1.opacity = 1.0f;
                _doorSpin2.opacity = 1.0f;
            }
        }
        else if (_phase == Phase::DisplayTitle)
        {
            _titleOverlay.opacity = std::max(0.0f, _titleOverlay.opacity - deltaSeconds * 2.5f);
            _subtitleOverlay.opacity = std::max(0.0f, _subtitleOverlay.opacity - deltaSeconds * 2.5f);

            if (_phaseTimer >= 0.5f)
            {
                _phase = Phase::PressStart;
                _phaseTimer = 0.0f;
                _pressVisible = true;
                _blinkTimer = 0.0f;
            }
        }
        else if (_phase == Phase::PressStart)
        {
            if (_blinkTimer >= 0.75f)
            {
                _blinkTimer = 0.0f;
                _pressVisible = !_pressVisible;
            }

            _doorSpin1.rotation += _doorSpin1.angularVelocity * deltaSeconds;
            _doorSpin2.rotation += _doorSpin2.angularVelocity * deltaSeconds;

            if (anyKeyPressed)
            {
                _phase = Phase::Pixelate;
                _phaseTimer = 0.0f;
                _pixelation = 0.0f;
                _fadeAlpha = 0.0f;
            }
        }
        else if (_phase == Phase::Pixelate)
        {
            _pixelation += deltaSeconds * 30.0f;
            if (_pixelation >= 15.0f)
            {
                _phase = Phase::FadeOut;
                _phaseTimer = 0.0f;
            }
        }
        else if (_phase == Phase::FadeOut)
        {
            _fadeAlpha = std::min(1.0f, _fadeAlpha + deltaSeconds * 1.5f);
            if (_fadeAlpha >= 1.0f)
            {
                _stateSetter->ChangeState(std::make_unique<MainMenuState>(_stateSetter));
                return;
            }
        }

        if (KeyInput::JustPressedKey(Keys::Escape))
        {
            _stateSetter->ExitGame();
        }
    }

    void TitleState::Draw()
    {
        const Microsoft::Xna::Framework::Color doorColor(64, 140, 180, 255);
        const Microsoft::Xna::Framework::Color glowColor(220, 200, 80, 190);
        const Microsoft::Xna::Framework::Color spinColor(180, 220, 255, 180);

        if (_nexusImage.visible)
        {
            SpriteDrawer::DrawFilledRect(_nexusImage.x, _nexusImage.y, _nexusImage.width, _nexusImage.height, doorColor);
        }

        if (_doorGlow.visible)
        {
            SpriteDrawer::DrawFilledRect(_doorGlow.x, _doorGlow.y, _doorGlow.width, _doorGlow.height, glowColor);
        }

        if (_doorSpin1.visible)
        {
            SpriteDrawer::DrawFilledRect(_doorSpin1.x, _doorSpin1.y, _doorSpin1.width, 4, spinColor);
            SpriteDrawer::DrawFilledRect(_doorSpin1.x + 8, _doorSpin1.y + 8, _doorSpin1.width - 16, 4, spinColor);
        }

        if (_doorSpin2.visible)
        {
            SpriteDrawer::DrawFilledRect(_doorSpin2.x + 8, _doorSpin2.y + 8, _doorSpin2.width - 16, 4, spinColor);
            SpriteDrawer::DrawFilledRect(_doorSpin2.x, _doorSpin2.y + _doorSpin2.height - 4, _doorSpin2.width, 4, spinColor);
        }

        if (_phase == Phase::Pixelate)
        {
            const int pixelSize = 8;
            const Microsoft::Xna::Framework::Color pixelColor(20, 20, 20, 64);
            for (int py = 0; py < 180; py += pixelSize)
            {
                for (int px = 0; px < 160; px += pixelSize)
                {
                    SpriteDrawer::DrawFilledRect(px, py, pixelSize, pixelSize, pixelColor);
                }
            }
        }

        if (_phase == Phase::FadeOut)
        {
            const uint8_t alpha = static_cast<uint8_t>(_fadeAlpha * 255.0f);
            SpriteDrawer::DrawFilledRect(0, 0, 160, 180, Microsoft::Xna::Framework::Color(0, 0, 0, alpha));
        }
    }

    void TitleState::DrawUI()
    {
        if (_phase == Phase::CreditsWrite || _phase == Phase::CreditsFade)
        {
            for (const auto& line : _creditLines)
            {
                Microsoft::Xna::Framework::Color textColor(255, 255, 255, static_cast<uint8_t>(line.opacity * 255.0f));
                SpriteDrawer::DrawText(line.revealedText, line.x, line.y, textColor, 2);
            }
            return;
        }

        for (size_t i = 0; i < _titleLines.size(); ++i)
        {
            int scale = (i == 0 ? 4 : 3);
            SpriteDrawer::DrawText(_titleLines[i], 16, 16 + (i == 0 ? 0 : 40), Microsoft::Xna::Framework::Color::White(), scale);
        }

        if (_titleOverlay.visible)
        {
            const auto whiteOverlay = Microsoft::Xna::Framework::Color(255, 255, 255, static_cast<uint8_t>(_titleOverlay.opacity * 180.0f));
            SpriteDrawer::DrawFilledRect(_titleOverlay.x - 4, _titleOverlay.y - 4, _titleOverlay.width + 8, _titleOverlay.height + 8, whiteOverlay);
        }

        if (_subtitle.visible)
        {
            SpriteDrawer::DrawText("A FAN REMAKE", 45, 72, Microsoft::Xna::Framework::Color::White(), 2);
        }

        if (_subtitleOverlay.visible)
        {
            const auto whiteOverlay = Microsoft::Xna::Framework::Color(255, 255, 255, static_cast<uint8_t>(_subtitleOverlay.opacity * 180.0f));
            SpriteDrawer::DrawFilledRect(_subtitleOverlay.x - 2, _subtitleOverlay.y - 2, _subtitleOverlay.width + 4, _subtitleOverlay.height + 4, whiteOverlay);
        }

        if (_pressEnter.visible && _pressVisible)
        {
            SpriteDrawer::DrawText("PRESS START", 32, 160, Microsoft::Xna::Framework::Color::White(), 3);
        }
    }
}
}
}
