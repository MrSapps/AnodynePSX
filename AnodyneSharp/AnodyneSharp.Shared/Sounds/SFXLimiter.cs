using Microsoft.Xna.Framework.Audio;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AnodyneSharp.Sounds
{
    public class SFXLimiter
    {
        List<SoundEffectInstance> pool;
        public float DurationSeconds { get; private set; }

        public SFXLimiter(SoundEffect effect, int n)
        {
            DurationSeconds = (float)effect.Duration.TotalSeconds;
            pool = Enumerable.Repeat(true, n).Select((t) => effect.CreateInstance()).ToList();
        }

        public SoundEffectInstance Get()
        {
            return pool.Find((e) => e.State == SoundState.Stopped);
        }
    }
}
