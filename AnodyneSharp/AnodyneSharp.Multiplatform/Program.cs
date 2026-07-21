using AnodyneSharp.Logging;
using AnodyneSharp.Recording;
using AnodyneSharp.Resources;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace AnodyneSharp.Multiplatform
{
    public static class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            try
            {
                ResourceManager.BaseDir = AppDomain.CurrentDomain.BaseDirectory;

                // Parse recording/playback options
                var (recordPath, playbackPath, verifyMode) = RecordingManager.ParseCommandLine(args);

                using AnodyneGame game = new AnodyneGame();
                
                // Initialize recording/playback if specified
                if (!string.IsNullOrEmpty(recordPath))
                {
                    RecordingManager.StartRecording(recordPath);
                }
                else if (!string.IsNullOrEmpty(playbackPath))
                {
                    RecordingManager.StartPlayback(playbackPath, verifyMode);
                }

                game.Run();

                // Clean up recording
                RecordingManager.Stop();
            }
            catch (Exception ex)
            {
                DebugLogger.AddException(ex);
            }

        }
    }
}
