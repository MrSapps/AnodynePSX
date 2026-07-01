#include "AnodyneSharp/Registry/GameConstants.hpp"

namespace AnodyneSharp::Registry {

std::string GameConstants::SavePath = "";

int GameConstants::FONT_LINE_HEIGHT() { return 8; }
int GameConstants::LineOffset()       { return 1; }

void GameConstants::Init() {
    // Set save path based on platform
    SavePath = "./Saves/";
}

} // namespace AnodyneSharp::Registry
