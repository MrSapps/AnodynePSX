using AnodyneSharp.Entities;
using AnodyneSharp.Input;
using AnodyneSharp.Logging;
using AnodyneSharp.Resources;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace AnodyneSharp.Recording
{
    public static class RecordingManager
    {
        public enum RecordingMode
        {
            None,
            Recording,
            Playback,
            Verification
        }

        private static RecordingMode _mode = RecordingMode.None;
        private static BinaryWriter _recordingWriter;
        private static BinaryWriter _verificationWriter;
        private static BinaryReader _playbackReader;
        private static Queue< Dictionary<Microsoft.Xna.Framework.Input.Keys, KeyInput.InputState>> _playbackQueue;
        private static long _frameNumber = 0;
        private static string _recordingPath;
        private static string _verificationPath;

        public static RecordingMode Mode => _mode;
        public static long FrameNumber => _frameNumber;

        /// <summary>
        /// Parse command-line arguments for recording/playback options.
        /// Returns (recordPath, playbackPath, verifyMode)
        /// </summary>
        public static (string record, string playback, bool verify) ParseCommandLine(string[] args)
        {
            args = new string[]
            {
                //"-record=recordings/test_recording3.bin"
                "-playback=recordings/test_recording3.bin"
                //"-verify=true"
            };


            string recordPath = null;
            string playbackPath = null;
            bool verifyMode = false;

            foreach (var arg in args)
            {
                if (arg.StartsWith("-record=", StringComparison.OrdinalIgnoreCase))
                {
                    recordPath = arg.Substring("-record=".Length);
                }
                else if (arg.StartsWith("-playback=", StringComparison.OrdinalIgnoreCase))
                {
                    playbackPath = arg.Substring("-playback=".Length);
                }
                else if (arg.StartsWith("-verify=", StringComparison.OrdinalIgnoreCase))
                {
                    string verifyStr = arg.Substring("-verify=".Length);
                    if (bool.TryParse(verifyStr, out bool result))
                    {
                        verifyMode = result;
                    }
                }
            }

            return (recordPath, playbackPath, verifyMode);
        }

        /// <summary>
        /// Start recording input to a file.
        /// </summary>
        public static void StartRecording(string filePath)
        {
            try
            {
                string dir = Path.GetDirectoryName(filePath);
                if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                {
                    Directory.CreateDirectory(dir);
                }

                _recordingPath = filePath;
                _recordingWriter = new BinaryWriter(File.Open(filePath, FileMode.Create, FileAccess.Write));
                _frameNumber = 0;
                _mode = RecordingMode.Recording;
                DebugLogger.AddInfo($"[Recording] Started recording to {filePath}");
            }
            catch (Exception ex)
            {
                DebugLogger.AddException(ex);
            }
        }

        /// <summary>
        /// Start playback of input from a file.
        /// </summary>
        public static void StartPlayback(string filePath, bool verifyMode = false)
        {
            try
            {
                _playbackReader = new BinaryReader(File.Open(filePath, FileMode.Open, FileAccess.Read));
                _playbackQueue = new ();
                _frameNumber = 0;

                // Preload all frames
                while (_playbackReader.BaseStream.Position < _playbackReader.BaseStream.Length)
                {
                    _playbackQueue.Enqueue(Read(_playbackReader));
                }

                _mode = verifyMode ? RecordingMode.Verification : RecordingMode.Playback;

                if (verifyMode)
                {
                    _verificationPath = Path.Combine(
                        Path.GetDirectoryName(filePath),
                        Path.GetFileNameWithoutExtension(filePath) + ".verify"
                    );
                    _verificationWriter = new BinaryWriter(File.Open(_verificationPath, FileMode.Create, FileAccess.Write));
                    DebugLogger.AddInfo($"[Recording] Started playback with verification (output: {_verificationPath})");
                }
                else
                {
                    DebugLogger.AddInfo($"[Recording] Started playback from {filePath}");
                }
            }
            catch (Exception ex)
            {
                DebugLogger.AddException(ex);
            }
        }

        /// <summary>
        /// Called once per frame to record or playback input.
        /// </summary>
        public static void FrameUpdate()
        {
            _frameNumber++;
        }


        public static void SaveHardwareInputs()
        {
            if (_recordingWriter == null) return;

            Write(_recordingWriter);
        }

        public static void Write(BinaryWriter writer)
        {
            writer.Write(CurrentInput.Count);

            foreach (var kvp in CurrentInput)
            {
                writer.Write((int)kvp.Key);                 // Keys enum → int
                writer.Write((byte)kvp.Value);              // InputState enum → byte
            }
        }

        public static  Dictionary<Microsoft.Xna.Framework.Input.Keys, KeyInput.InputState> Read(BinaryReader reader)
        {
            var r = new Dictionary<Microsoft.Xna.Framework.Input.Keys, KeyInput.InputState>();

            int count = reader.ReadInt32();

            for (int i = 0; i < count; i++)
            {
                var key = (Microsoft.Xna.Framework.Input.Keys)reader.ReadInt32();
                var state = (KeyInput.InputState)reader.ReadByte();

                r[key] = state;
            }
            return r;
        }


        public static Dictionary<Microsoft.Xna.Framework.Input.Keys, KeyInput.InputState> CurrentInput { get; set; } = new();


        public static void ReadInputToInject()
        {
            if (_playbackQueue == null || _playbackQueue.Count == 0) return;

            CurrentInput = _playbackQueue.Dequeue();
        }

        /// <summary>
        /// Record entity/NPC state for verification.
        /// </summary>
        public static void RecordFrameEntities(List<Entity> entities)
        {
            if (_mode != RecordingMode.Verification || _verificationWriter == null) return;

            var frameData = new FrameEntityData
            {
                FrameNumber = _frameNumber
            };

            foreach (var ent in entities.Where(e => e.exists))
            {
                frameData.Entities.Add(new FrameEntityData.EntitySnapshot
                {
                    TypeName = ent.GetType().Name,
                    Position = ent.Position,
                    Velocity = ent.velocity,
                    Rotation = ent.rotation,
                    Scale = ent.scale,
                    Opacity = ent.opacity,
                    Exists = ent.exists,
                    Visible = ent.visible,
                    FrameIndex = ent.FrameIndex
                });
            }

            frameData.Write(_verificationWriter);
        }

        /// <summary>
        /// Stop recording/playback and close files.
        /// </summary>
        public static void Stop()
        {
            if (_recordingWriter != null)
            {
                _recordingWriter.Flush();
                _recordingWriter.Close();
                DebugLogger.AddInfo($"[Recording] Recording stopped. Saved to {_recordingPath}");
            }

            if (_verificationWriter != null)
            {
                _verificationWriter.Flush();
                _verificationWriter.Close();
                DebugLogger.AddInfo($"[Recording] Verification data saved to {_verificationPath}");
            }

            if (_playbackReader != null)
            {
                _playbackReader.Close();
            }

            _mode = RecordingMode.None;
            _frameNumber = 0;
        }
    }
}
