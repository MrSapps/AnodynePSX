#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp {
namespace Dialogue { enum class Language { EN=0, ES, IT, JP, KR, PT_BR, ZH_CN }; }

namespace Registry {

enum class Resolution { Windowed, Scaled, Stretch };
enum class FPS       { Fixed, VSync, Unlocked };

class Settings {
public:
    AnodyneSharp::Dialogue::Language language = AnodyneSharp::Dialogue::Language::EN;
    float music_volume_scale = 1.0f;
    float sfx_volume_scale   = 1.0f;
    bool  autosave_on        = true;
    bool  pause_on_unfocus   = true;
    bool  fast_text          = false;
    bool  invincible         = false;
    bool  extended_coyote    = false;
    bool  guaranteed_health  = false;
    Resolution resolution    = Resolution::Windowed;
    int   scale              = 3;
    FPS   fps                = FPS::Fixed;
    float flash_brightness   = 1.0f;
    float flash_easing       = 0.0f;
    bool  screenshake        = true;

    static Settings Load();
    void  Save() const;
};

} // namespace Registry
} // namespace AnodyneSharp

using AnodyneSharp::Registry::Settings;
using AnodyneSharp::Registry::Resolution;
using AnodyneSharp::Dialogue::Language;
