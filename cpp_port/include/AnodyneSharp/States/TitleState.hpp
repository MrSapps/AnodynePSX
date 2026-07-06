#pragma once

#include "AnodyneSharp/States/Base/State.hpp"
#include "AnodyneSharp/Drawing/ScrollingTex.hpp"
#include "AnodyneSharp/UI/UITypes.hpp"
#include <vector>
#include <memory>

namespace AnodyneSharp::States {

// ---- TitleState ----
class TitleState : public State {
public:
    TitleState();
    void Create()  override;
    void Update()  override;
    void Draw()    override;
    void DrawUI()  override;

private:
    Drawing::ScrollingTex _background;

    std::unique_ptr<UIEntity> mTitleTex;
    std::unique_ptr<UIEntity> mTitleOverlay;    

    std::unique_ptr<UIEntity> mSubtitle;
    std::unique_ptr<UIEntity> mSubtitleOverlay;

    std::unique_ptr<UIEntity> mNexusImage;

    std::unique_ptr<UIEntity> mDoorGlow;
    std::unique_ptr<UIEntity> mDoorSpin1;
    std::unique_ptr<UIEntity> mDoorSpin2;

    std::unique_ptr<UIEntity> mPressEnterTex;

    bool  _pressEnterVisible = false;
    float _blinkTimer        = 1.f;
    bool  _pixelating        = false;
};
}
