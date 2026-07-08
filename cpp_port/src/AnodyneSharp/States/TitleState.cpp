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
                .State("CreditsFade")
                    .Update([this](AbstractState* state, float t)
                    {
                        bool endFade = false;

                        for (UILabel* label : mCreditLabels)
                        {
                            float o = label->opacity;

                            if (MathUtilities::MoveTo(o, 0.3f, 0.6f))
                            {
                                endFade = true;
                            }
                            label->opacity = o;
                        }

                        if (endFade)
                        {
                            _state->ChangeState("CreditsFadeEnd");
                        }
                    })
                    .Condition([this]() { return AnyKeyPressed(); },
                            [this](AbstractState* s) { _state->ChangeState("PressStart"); })
                .End()
                .State<TextFadeTimer>("CreditsFadeEnd")
                    .Update([this](AbstractState* state, float t)
                    {
                        for (UILabel* label : mCreditLabels)
                        {
                            float o = label->opacity;
                            MathUtilities::MoveTo(o, 0.0f, 2.0f);
                            label->opacity = o;
                        }
                    })
                    .Condition([this]() { return AnyKeyPressed(); },
                            [this](AbstractState* s) { _state->ChangeState("PressStart"); })
                    .Event("EndFade", [this](AbstractState* state)
                    {
                        if (_secondNames)
                        {
                            _state->ChangeState("ScollUp");
                        }
                        else
                        {
                            _secondNames = true;

                            mCredits[0] = DialogueManager::GetDialogue("misc", "any", "title", 5);
                            mCredits[1] = DialogueManager::GetDialogue("misc", "any", "title", 6);
                            mCredits[2] = DialogueManager::GetDialogue("misc", "any", "title", 7);

                            _state->ChangeState("CreditsWrite");
                        }
                    })
                .End()
                .State("ScollUp")
                    .Enter([this](AbstractState* state)
                    {
                        mNexusImage->velocity.Y = -20;
                    })
                    .Condition([this]()
                    {
                        return AnyKeyPressed() ||
                            (mNexusImage->Position.Y + mNexusImage->height <= 180);
                    },
                    [this](AbstractState* s)
                    {
                        _state->ChangeState("PressStart");
                    })
                .End()
                .State("PressStart")
                    .Enter([this](AbstractState* s)
                    {
                        GlobalState::flash.Flash(
                            1.5f,
                            Color::White,
                            [this]() { _state->ChangeState("DisplayTitle"); }
                        );
                    })
                .End()
                .State<PressEnterTimer>("DisplayTitle")
                    .Enter([this](AbstractState* state)
                    {
                        for (UILabel* label : mCreditLabels)
                        {
                            label->visible = false;
                        }

                        //GlobalState::TitleScreenFinish.Darkness = ResourceManager::GetTexture("title_overlay2");

                        mNexusImage->Position.Y = 180 - mNexusImage->height;
                        mNexusImage->velocity.Y = 0;

                        mDoorGlow->Position = { (160 - 64) / 2.0f, mNexusImage->Position.Y + 17 };

                        Vector2 spinPos = { mDoorGlow->Position.X, mNexusImage->Position.Y };
                        mDoorSpin1->Position = spinPos;
                        mDoorSpin2->Position = spinPos;
                        mDoorSpin1->angularVelocity = MathHelper::ToRadians(90);
                        mDoorSpin2->angularVelocity = MathHelper::ToRadians(-90);

                        mDoorGlow->visible = true;
                        mDoorSpin1->visible = true;
                        mDoorSpin2->visible = true;

                        mPressEnterTex->visible = true;
                        mTitleTex->visible = true;
                        mTitleOverlay->visible = true;

                        mSubtitle->visible = true;
                        mSubtitleOverlay->visible = true;
                    })
                    .Update([this](AbstractState* state, float t)
                    {
                        MathUtilities::MoveTo(mTitleOverlay->opacity, 0.0f, 0.4f);
                        MathUtilities::MoveTo(mSubtitleOverlay->opacity, 0.0f, 0.4f);
                    })
                    .Event("BlinkEnter", [this](AbstractState* state)
                    {
                        mPressEnterTex->visible = !mPressEnterTex->visible;
                    })
                    .Condition([this]() { return AnyKeyPressed(); },
                            [this](AbstractState* s) { _state->ChangeState("Pixelate"); })
                .End()
                .State("Pixelate")
                    .Update([this](AbstractState* state, float t)
                    {
                        GlobalState::pixelation.AddPixelation(15);
                         //GlobalState::black_overlay.ChangeAlpha(0.36f);
                    })
                    .Condition([this]()
                    {
                        return GlobalState::pixelation.GetPixelSize() >= 15.0f;
                    },
                    [this](AbstractState* state)
                    {
                        _state->ChangeState("FadeOut");
                    })
                .End()
                .State("FadeOut")
                    .Update([this](AbstractState* state, float t)
                    {
                        GlobalState::pixelation.AddPixelation(15);
                        GlobalState::black_overlay.ChangeAlpha(0.72f);
                    })
                    .Condition([this]()
                    {
                        return GlobalState::black_overlay.alpha >= 1.0f;
                    },
                    [this](AbstractState* state)
                    {
                        GlobalState::pixelation.SetPixelation(0);
                        /*

                        GlobalState::black_overlay.alpha = 0;

                        GlobalState::flash.ForceAlpha(0);
                        GlobalState::TitleScreenFinish.ForceAlpha(0);

                        GlobalState::TitleScreenFinish.Entities.clear();
                        GlobalState::TitleScreenFinish.Labels.clear();
*/
                        GlobalState::GameState->SetState<MainMenuState>();
                        
                    })
                .End()
                .Build();

        _state->ChangeState("IntroFade");
    }

    void TitleState::Create()
    {
        mCredits[0] = Dialogue::DialogueManager::GetDialogue("misc", "any", "credits", 2);
        mCredits[1] = Dialogue::DialogueManager::GetDialogue("misc", "any", "credits", 3);
        mCredits[2] = Dialogue::DialogueManager::GetDialogue("misc", "any", "credits", 4);

        _background.Load("title_bg", Vector2(0.0f, -30.f), AnodyneSharp::Drawing::DrawOrder::BACKGROUND);

        mNexusImage = std::make_unique<UIEntity>(Vector2(0, -180), "door", GameConstants::SCREEN_WIDTH_IN_PIXELS, 116, DrawOrder::UI_OBJECTS);

        mDoorGlow = std::make_unique<UIEntity>(Vector2(64, 32), "door_glow", 64, 32, new RefLayer(mNexusImage->layer_def_get(), 1));     // TODO: fix leak
        mDoorGlow->visible = false;

        mDoorSpin1 = std::make_unique<UIEntity>(Vector2(0, 0), "door_spinglow1", 64, 64, new RefLayer(mNexusImage->layer_def_get(), 2)); // TODO: fix leak
        mDoorSpin1->visible = false;

        mDoorSpin2 = std::make_unique<UIEntity>(Vector2(0, 0), "door_spinglow2", 64, 64, new RefLayer(mNexusImage->layer_def_get(), 2)); // TODO: fix leak
        mDoorSpin2->visible = false;

        mPressEnterTex = std::make_unique<UIEntity>(Vector2((GameConstants::SCREEN_WIDTH_IN_PIXELS - 96) / 2, GameConstants::SCREEN_HEIGHT_IN_PIXELS), "press_enter", 96, 16, DrawOrder::MENUTEXT);
        mPressEnterTex->visible = false;

        mTitleTex = std::make_unique<UIEntity>(Vector2(16, 16), "title_text", 128, 48, DrawOrder::MENUTEXT);
        mTitleTex->visible = false;

        mTitleOverlay = std::make_unique<UIEntity>(Vector2(16, 16), "title_text_white", 128, 48, DrawOrder::TEXTBOX);
        mTitleOverlay->visible = false;

        mSubtitle = std::make_unique<UIEntity>(Vector2(45, 47), "title_remake", 71, 11, DrawOrder::MENUTEXT);
        mSubtitle->visible = false;

        mSubtitleOverlay = std::make_unique<UIEntity>(mSubtitle->Position, "title_remake_white", 71, 11, DrawOrder::TEXTBOX);
        mSubtitleOverlay->visible = false;

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
        // float uiZ   = DrawingUtilities::GetDrawingZ(DrawOrder::UI_OBJECTS);
        // float menuZ = DrawingUtilities::GetDrawingZ(DrawOrder::MENUTEXT);

        _background.DrawUI();

        mNexusImage->Draw();
        mDoorGlow->Draw();
        mDoorSpin1->Draw();
        mDoorSpin2->Draw();
        mTitleTex->Draw();
        mTitleOverlay->Draw();

        mSubtitle->Draw();
        mSubtitleOverlay->Draw();

        mPressEnterTex->Draw();

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
