#include "AnodyneSharp/Registry/Settings.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"
#include <fstream>
#include <sstream>

namespace AnodyneSharp::Registry {

// Minimal key=value file format
static std::string settingsPath() {
    return GameConstants::SavePath + "Settings.cfg";
}

static bool readBool(const std::string& v)  { return v == "1" || v == "true"; }
static int  readInt (const std::string& v)  { try { return std::stoi(v); } catch(...) { return 0; } }
static float readFloat(const std::string& v){ try { return std::stof(v); } catch(...) { return 0.f; } }

Settings Settings::Load() {
    Settings s;
    std::ifstream f(settingsPath());
    if (!f.is_open()) return s;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        if      (key == "music_volume")  s.music_volume_scale  = readFloat(val);
        else if (key == "sfx_volume")    s.sfx_volume_scale    = readFloat(val);
        else if (key == "autosave")      s.autosave_on         = readBool(val);
        else if (key == "fast_text")     s.fast_text           = readBool(val);
        else if (key == "invincible")    s.invincible          = readBool(val);
        else if (key == "screenshake")   s.screenshake         = readBool(val);
        else if (key == "scale")         s.scale               = readInt(val);
        else if (key == "extended_coyote") s.extended_coyote   = readBool(val);
        else if (key == "flash_brightness") s.flash_brightness = readFloat(val);
    }
    return s;
}

void Settings::Save() const {
    // Ensure directory exists (best-effort)
    std::ofstream f(settingsPath());
    if (!f.is_open()) return;
    f << "music_volume="    << music_volume_scale << "\n";
    f << "sfx_volume="      << sfx_volume_scale   << "\n";
    f << "autosave="        << (autosave_on  ? "1" : "0") << "\n";
    f << "fast_text="       << (fast_text    ? "1" : "0") << "\n";
    f << "invincible="      << (invincible   ? "1" : "0") << "\n";
    f << "screenshake="     << (screenshake  ? "1" : "0") << "\n";
    f << "scale="           << scale                      << "\n";
    f << "extended_coyote=" << (extended_coyote ? "1" : "0") << "\n";
    f << "flash_brightness="<< flash_brightness            << "\n";
}

} // namespace AnodyneSharp::Registry
