#pragma once
#include <string>

namespace Microsoft { namespace Xna { namespace Framework { namespace Media {

// ---------------------------------------------------------------
// MediaState
// ---------------------------------------------------------------
enum class MediaState {
    Playing,
    Paused,
    Stopped
};

// ---------------------------------------------------------------
// Song (stub)
// ---------------------------------------------------------------
class Song {
public:
    std::string Name;
    float Duration = 0.f;
    virtual ~Song() = default;
};

// ---------------------------------------------------------------
// MediaPlayer (stub)
// ---------------------------------------------------------------
class MediaPlayer {
public:
    static float Volume;
    static bool IsRepeating;
    static bool IsMuted;
    static bool IsShuffled;
    static MediaState State;

    static void Play(Song* song) {}
    static void Pause() {}
    static void Resume() {}
    static void Stop() {}
};

}}}} // namespace Microsoft::Xna::Framework::Media

using Microsoft::Xna::Framework::Media::MediaState;
using Microsoft::Xna::Framework::Media::Song;
using Microsoft::Xna::Framework::Media::MediaPlayer;
