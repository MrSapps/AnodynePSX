using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.IO;

namespace AnodyneSharp.Recording
{
    /// <summary>
    /// Represents entity/npc state at a specific frame for verification purposes.
    /// </summary>
    [Serializable]
    public class FrameEntityData
    {
        public struct EntitySnapshot
        {
            public string TypeName;
            public Vector2 Position;
            public Vector2 Velocity;
            public float Rotation;
            public float Scale;
            public float Opacity;
            public bool Exists;
            public bool Visible;
            public int FrameIndex;

            public void Write(BinaryWriter writer)
            {
                writer.Write(TypeName ?? "");
                writer.Write(Position.X);
                writer.Write(Position.Y);
                writer.Write(Velocity.X);
                writer.Write(Velocity.Y);
                writer.Write(Rotation);
                writer.Write(Scale);
                writer.Write(Opacity);
                writer.Write(Exists);
                writer.Write(Visible);
                writer.Write(FrameIndex);
            }

            public static EntitySnapshot Read(BinaryReader reader)
            {
                return new()
                {
                    TypeName = reader.ReadString(),
                    Position = new(reader.ReadSingle(), reader.ReadSingle()),
                    Velocity = new(reader.ReadSingle(), reader.ReadSingle()),
                    Rotation = reader.ReadSingle(),
                    Scale = reader.ReadSingle(),
                    Opacity = reader.ReadSingle(),
                    Exists = reader.ReadBoolean(),
                    Visible = reader.ReadBoolean(),
                    FrameIndex = reader.ReadInt32()
                };
            }
        }

        public long FrameNumber;
        public List<EntitySnapshot> Entities = new();

        public void Write(BinaryWriter writer)
        {
            writer.Write(FrameNumber);
            writer.Write(Entities.Count);
            foreach (var ent in Entities)
            {
                ent.Write(writer);
            }
        }

        public static FrameEntityData Read(BinaryReader reader)
        {
            var data = new FrameEntityData();
            data.FrameNumber = reader.ReadInt64();
            int count = reader.ReadInt32();
            for (int i = 0; i < count; i++)
            {
                data.Entities.Add(EntitySnapshot.Read(reader));
            }
            return data;
        }
    }
}
