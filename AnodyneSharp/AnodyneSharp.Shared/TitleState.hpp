#pragma once

#include "State.hpp"
#include "KeyInput.hpp"
#include <SDL2/SDL.h>
#include <string>
#include <vector>

namespace AnodyneSharp
{
namespace States
{
namespace MainMenu
{
    class TitleState : public State
    {
    public:
        explicit TitleState(IStateSetter* stateSetter);
        void Create() override;
        void Update() override;
        void Draw() override;
        void DrawUI() override;

    private:
        IStateSetter* _stateSetter;
        enum class Phase
        {
            Intro,
            CreditsWrite,
            CreditsFade,
            CreditsFadeEnd,
            ScrollUp,
            DisplayTitle,
            PressStart,
            Pixelate,
            FadeOut,
        };

        Phase _phase = Phase::Intro;
        float _phaseTimer = 0.0f;
        bool _pressVisible = false;
        Uint32 _lastBlinkTimestamp = 0;
        float _scrollOffset = 0.0f;
        bool _titleShown = false;
        bool _secondNames = false;
        std::vector<std::string> _credits;
        std::vector<std::string> _visibleCredits;
        struct CreditLine
        {
            std::string fullText;
            std::string revealedText;
            int x = 0;
            int y = 0;
            float opacity = 1.0f;
        };

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

        std::vector<CreditLine> _creditLines;
        std::vector<std::pair<size_t, size_t>> _remainingCreditChars;
        float _textRevealTimer = 0.0f;
        float _blinkTimer = 0.0f;
        UIEntity _nexusImage;
        UIEntity _doorGlow;
        UIEntity _doorSpin1;
        UIEntity _doorSpin2;
        UIEntity _pressEnter;
        UIEntity _title;
        UIEntity _titleOverlay;
        UIEntity _subtitle;
        UIEntity _subtitleOverlay;
        size_t _nextCreditIndex = 0;
        std::vector<std::string> _titleLines;
        float _pixelation = 0.0f;
        float _fadeAlpha = 0.0f;
        void SetupCreditLines();
        Phase _lastDrawPhase = Phase::Intro;
        size_t _lastDrawCreditsCount = 0;
        bool _lastDrawPressVisible = false;
    };
}
}
}
