#include "AnodyneSharp/States/TitleState.hpp"
#include "AnodyneSharp/States/AllStates.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"
#include "AnodyneSharp/Sounds/SoundManager.hpp"
#include "AnodyneSharp/Input/KeyInput.hpp"
#include "AnodyneSharp/Dialogue/DialogueManager.hpp"
#include "AnodyneSharp/Drawing/Effects/Effects.hpp"

namespace AnodyneSharp::States
{
    class TextDrawTimer : public AnodyneSharp::FSM::TimerState
    {
    public:
        TextDrawTimer()
        {
            AddTimer(0.03f, "DrawText");
        }
    };

    class TextFadeTimer : public AnodyneSharp::FSM::TimerState
    {
    public:
        TextFadeTimer()
        {
            AddTimer(1.0f, "EndFade");
        }
    };

    class PressEnterTimer : public AnodyneSharp::FSM::TimerState
    {
    public:
        PressEnterTimer()
        {
            AddTimer(1.0f, "BlinkEnter");
        }
    };

    // ---- TitleState ----
    TitleState::TitleState()
    {
        StateMachineBuilder smb;
            _state = smb
                
                .State("IntroFade")
                    .Enter([](AbstractState* state)
                    {
                        GlobalState::flash.Flash(2.0f, Color::Black);
                        //GlobalState::TitleScreenFinish.Darkness = ResourceManager::GetTexture("title_overlay1");
                        //GlobalState::TitleScreenFinish.ForceAlpha(1); // TODO Not on darkness ??
                    })
                    .Condition([this]() { return AnyKeyPressed(); }, [](AbstractState* state) { state->ChangeState("PressStart"); })
                    .Condition([this]() { return !GlobalState::flash.Active(); }, [](AbstractState* state) { state->ChangeState("CreditsWrite"); })
                .End()

                .State<TextDrawTimer>("CreditsWrite")
                    .Enter([this](AbstractState* state)
                    {
                        int lineH = GameConstants::FONT_LINE_HEIGHT() + 2;
                        int center = GameConstants::SCREEN_WIDTH_IN_PIXELS / 2;
                        int charWidth = FontManager::GetCharacterWidth();
                        int charWidthEng = FontManager::GetCharacterWidth(true);

                       Microsoft::Xna::Framework::Color color(68, 109, 113);

                        mCreditLabels[0] = new UILabel(Vector2(center - (mCredits[0].length() * charWidth)/2, 88 -lineH-4), false, std::string(' ', mCredits[0].length()), color);
                        mCreditLabels[1] = new UILabel(Vector2(center - (mCredits[1].length() * charWidthEng)/2, 88), false, std::string(' ', mCredits[1].length()), color); // TOOD: Force english
                        mCreditLabels[2] = new UILabel(Vector2(center - (mCredits[2].length() * charWidthEng)/2, 88 + lineH), false, std::string(' ', mCredits[2].length()), color); // TOOD: Force english

                        // TODO: !!
                        //GlobalState::TitleScreenFinish.Labels = mCreditLabels;

                        // Build list of all (i, j) pairs
                        notVisibleYet.clear();
                        for (int i = 0; i < 3; ++i)
                        {
                            for (size_t j = 0; j < mCredits[i].size(); ++j)
                            {
                                notVisibleYet.emplace_back(i, j);
                            }
                        }
                    })
                    .Event("DrawText", [this](AbstractState* state)
                    {
                        if (notVisibleYet.empty())
                        {
                            return;
                        }
                        
                        int index = GlobalState::RNG.Next(0, notVisibleYet.size());

                        auto [labelIndex, charIndex] = notVisibleYet[index];

                        UILabel* l = mCreditLabels[labelIndex];

                        std::string text = l->GetText();          // C++: get current label text
                        text[charIndex] = mCredits[labelIndex][charIndex];

                        l->SetText(text);

                        notVisibleYet.erase(notVisibleYet.begin() + index);
                    })
                    .Condition([this]() { return AnyKeyPressed(); }, [](AbstractState* state) { state->ChangeState("PressStart"); })
                    .Condition([this]() { return notVisibleYet.empty(); }, [](AbstractState* state) { state->ChangeState("CreditsFade"); })
                .End()


                .Build();

        _state->ChangeState("IntroFade");
    }

    void TitleState::Create()
    {
        mCredits[0] = Dialogue::DialogueManager::GetDialogue("misc", "any", "credits", 2);
        mCredits[1] = Dialogue::DialogueManager::GetDialogue("misc", "any", "credits", 3);
        mCredits[2] = Dialogue::DialogueManager::GetDialogue("misc", "any", "credits", 4);

        //_background.Load("title_bg", 0.f, -30.f);

        // nexusImage
        //    mNexusImage = std::make_unique<UIEntity>(Vector2(0, 180), "door", GameConstants::SCREEN_WIDTH_IN_PIXELS, 116, DrawOrder::UI_OBJECTS);
        mNexusImage = std::make_unique<UIEntity>(Vector2(0, -180), "door", GameConstants::SCREEN_WIDTH_IN_PIXELS, 116, DrawOrder::UI_OBJECTS);

        mNexusImage->velocity.Y = 20.0f;

        mDoorGlow = std::make_unique<UIEntity>(Vector2(64, 32), "door_glow", 64, 32, new RefLayer(mNexusImage->layer_def_get(), 1));     // TODO: fix leak
        mDoorSpin1 = std::make_unique<UIEntity>(Vector2(0, 0), "door_spinglow1", 64, 64, new RefLayer(mNexusImage->layer_def_get(), 2)); // TODO: fix leak
        mDoorSpin2 = std::make_unique<UIEntity>(Vector2(0, 0), "door_spinglow2", 64, 64, new RefLayer(mNexusImage->layer_def_get(), 2)); // TODO: fix leak
        mPressEnterTex = std::make_unique<UIEntity>(Vector2((GameConstants::SCREEN_WIDTH_IN_PIXELS - 96) / 2, GameConstants::SCREEN_HEIGHT_IN_PIXELS), "press_enter", 96, 16, DrawOrder::MENUTEXT);

        mTitleTex = std::make_unique<UIEntity>(Vector2(0, 0), "title_text", 128, 48, DrawOrder::MENUTEXT);
        mTitleOverlay = std::make_unique<UIEntity>(Vector2(0, 0), "title_text_white", 128, 48, DrawOrder::TEXTBOX);

        mSubtitle = std::make_unique<UIEntity>(Vector2(16), "title_remake", 71, 11, DrawOrder::MENUTEXT);
        mSubtitleOverlay = std::make_unique<UIEntity>(Vector2(0, 0), "title_remake_white", 71, 11, DrawOrder::TEXTBOX);

        // TODO: Not here in CS
        GlobalState::flash.Flash(2.f, Color::Black);

        // TODO ??
        // GlobalState::TitleScreenFinish;
        // GlobalState.TitleScreenFinish.Entities.Add(pressEnter);

        Sounds::SoundManager::PlaySong("title");
    }

    void TitleState::Update()
    {
       
        /*
        if (_pixelating)
        {
            GlobalState::pixelation.AddPixelation(15.f);
            GlobalState::black_overlay.ChangeAlpha(0.54f);
            if (GlobalState::black_overlay.alpha >= 1.f)
            {
                GlobalState::pixelation.SetPixelation(0.f);
                GlobalState::black_overlay.alpha = 0.f;
                GlobalState::flash.Deactivate();
                GlobalState::GameState->SetState<MainMenuState>();
            }
            return;
        }

        if (!GlobalState::flash.Active())
        {
            _blinkTimer -= GameTimes::DeltaTime();
            if (_blinkTimer <= 0.f)
            {
                _blinkTimer = 1.f;
                _pressEnterVisible = !_pressEnterVisible;
            }
            if (AnyKeyPressed())
            {
                _pixelating = true;
            }
        }
*/
        _state->Update(GameTimes::DeltaTime());

        _background.Update();

        mNexusImage->Update();
        mDoorGlow->Update();
        mDoorSpin1->Update();
        mDoorSpin2->Update();
        mTitleTex->Update();
        mTitleOverlay->Update();
        mPressEnterTex->Update();

        mNexusImage->PostUpdate();
        mDoorGlow->PostUpdate();
        mDoorSpin1->PostUpdate();
        mDoorSpin2->PostUpdate();
        mTitleTex->PostUpdate();
        mTitleOverlay->PostUpdate();
        mPressEnterTex->PostUpdate();
    }

    void TitleState::Draw() {}

    void TitleState::DrawUI()
    {
        float bgZ = DrawingUtilities::GetDrawingZ(DrawOrder::BACKGROUND);
        // float uiZ   = DrawingUtilities::GetDrawingZ(DrawOrder::UI_OBJECTS);
        // float menuZ = DrawingUtilities::GetDrawingZ(DrawOrder::MENUTEXT);

        _background.Draw(bgZ);

        mNexusImage->Draw();
        mDoorGlow->Draw();
        mDoorSpin1->Draw();
        mDoorSpin2->Draw();
        mTitleTex->Draw();
        mTitleOverlay->Draw();

        mSubtitle->Draw();
        mSubtitleOverlay->Draw();

        // The UI labels get drawn in the TitleScreen overlay
        /*
        if (_pressEnterVisible && _pressEnterTex && !GlobalState::flash.Active()) {
            int px = (160 - _pressEnterTex->Width) / 2;
            Rectangle dst{px, 160, _pressEnterTex->Width, _pressEnterTex->Height};
            SpriteDrawer::DrawSprite(_pressEnterTex, dst, nullptr, nullptr, 0.f,
                                     SpriteEffects::None, menuZ);
        }
        */
    }

    bool TitleState::AnyKeyPressed() const
    {
        using namespace Input;
        return (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept) ||
                KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel) ||
                KeyInput::JustPressedRebindableKey(KeyFunctions::Pause));
    }

}
