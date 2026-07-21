using AnodyneSharp.Logging;
using AnodyneSharp.Registry;
using AnodyneSharp.Resources;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Media;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AnodyneSharp.Sounds
{
    public static class SoundManager
    {
        public static string CurrentSongName { get; private set; }

        public static bool IsPlayingSong
        {
            get
            {
                return CurrentSongName != "";
            }
        }

        private static float masterVolume = 1f;
        private static SongPlayer bgm = new();
        private static float currentVolume = 1f;
        private static SongPlayer ambience = new();
        private static float ambienceVolume = 1f;

        // Track active SFX by remaining game-time seconds
        private static Dictionary<SoundEffectInstance, float> _activeSfx = new();

        static SoundManager()
        {
            CurrentSongName = "";
        }

        public static bool PlaySong(string name, float volume = 1f)
        {
            if(CurrentSongName == name)
            {
                SetSongVolume(volume);
                return false;
            }
            string song = ResourceManager.GetMusicPath(name);
            if (song != null)
            {
                CurrentSongName = name;
                SetSongVolume(volume);
                bgm.Play(song);
                return true;
            }
            else
            {
                StopSong();
                return false;
            }
        }

        public static void SetMasterVolume(float volume)
        {
            masterVolume = Math.Clamp(volume, 0, 1);
            SetSongVolume(currentVolume);
        }

        public static float GetMasterVolume() => masterVolume;

        public static void SetSongVolume(float volume)
        {
            currentVolume = Math.Clamp(volume,0,1);
            bgm.SetVolume(masterVolume * currentVolume * GlobalState.settings.music_volume_scale);
            SetAmbienceVolume(ambienceVolume);
        }

        public static void SetAmbienceVolume(float volume)
        {
            ambienceVolume = Math.Clamp(volume, 0, 1);
            ambience.SetVolume(masterVolume * currentVolume * ambienceVolume * GlobalState.settings.music_volume_scale);
        }

        public static float GetVolume() => currentVolume;

        public static void SetSongVolume() => SetSongVolume(1f);

        public static bool StopSong()
        {
            if (IsPlayingSong)
            {
                bgm.Stop();
                CurrentSongName = "";
                return true;
            }
            return false;
        }

        public static void PlayAmbience(string name, float volume = 1f)
        {
            string sound = ResourceManager.GetAmbiencePath(name);
            SetAmbienceVolume(volume);
            if(sound != null)
            {
                ambience.Play(sound);
            }
            else
            {
                ambience.Stop();
            }
        }

        public static SoundEffectInstance PlaySoundEffect(params string[] names)
        {
            var availableNames = names.Where(n => ResourceManager.GetSFXDuration(n) > 0f).ToArray();
            if (availableNames.Length == 0) return null;

            string chosen = availableNames[GlobalState.RNG.Next(0, availableNames.Length)];
            var inst = ResourceManager.GetSFX(chosen);
            float dur = ResourceManager.GetSFXDuration(chosen);
            return CreateSoundInstance(inst, 1f, 0f, dur);
        }

        public static void PlayPitchedSoundEffect(string name, float pitch, float volume = 1)
        {
            float dur = ResourceManager.GetSFXDuration(name);
            CreateSoundInstance(ResourceManager.GetSFX(name), volume, pitch, dur);
        }

        private static SoundEffectInstance CreateSoundInstance(SoundEffectInstance sfx, float volume = 1, float pitch = 0, float durationSeconds = 0)
        {
            if (sfx != null)
            {
                sfx.Pitch = pitch;
                sfx.Volume = volume * GlobalState.settings.sfx_volume_scale;
                sfx.Play();

                if (durationSeconds > 0f)
                {
                    _activeSfx[sfx] = durationSeconds;
                }
            }
            return sfx;
        }

        // Update should be called once per frame (after GameTimes.UpdateTimes) to decrement tracked SFX timers.
        public static void Update()
        {
            if (_activeSfx.Count == 0) return;
            var toRemove = new List<SoundEffectInstance>();
            float dt = GameTimes.DeltaTime;
            foreach (var kv in _activeSfx)
            {
                float rem = kv.Value - dt;
                if (rem <= 0f)
                {
                    toRemove.Add(kv.Key);
                }
                else
                {
                    _activeSfx[kv.Key] = rem;
                }
            }
            foreach (var k in toRemove) _activeSfx.Remove(k);
        }

        // Query whether an instance is playing according to game time tracking. Falls back to actual SoundState if not tracked.
        public static bool IsPlaying(SoundEffectInstance sfx)
        {
            if (sfx == null) return false;
            if (_activeSfx.TryGetValue(sfx, out float rem))
            {
                return rem > 0f;
            }
            return sfx.State == SoundState.Playing;
        }
    }
}