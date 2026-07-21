using Microsoft.Xna.Framework;
using System.Collections.Generic;
using System.Linq;

namespace AnodyneSharp
{
    public static class GameTimes
    {
        public static float FPS { get; private set; }
        public static float TimeScale { get; set; }
        public const float FIXED_DELTA_TIME = 1f / 60f; // Fixed 60 FPS delta time
        public static float TrueDeltaTime { get; private set; } = FIXED_DELTA_TIME;

        public static float DeltaTime
        {
            get
            {
                return TrueDeltaTime * TimeScale;
            }
        }

        private static Queue<float> _fpsQueue;
        private static int _maxSamples = 100;

        static GameTimes()
        {
            _fpsQueue = new Queue<float>(_maxSamples);

            TimeScale = 1;
        }

        public static void UpdateTimes(GameTime gameTime)
        {
            TrueDeltaTime = FIXED_DELTA_TIME;
        }

        public static void UpdateFPS(GameTime gameTime)
        {
            if (_fpsQueue.Count > _maxSamples)
            {
                _fpsQueue.Dequeue();
                FPS = _fpsQueue.Average(i => i);
            }
            _fpsQueue.Enqueue( 1f / (float)gameTime.ElapsedGameTime.TotalSeconds);
        }
    }
}
