using GameManagement;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Media;
using SharpDX;
using System;
using System.Collections.Generic;
using System.Diagnostics.PerformanceData;
using System.Net.Security;
using System.Numerics;
using System.Text;
using Color = Microsoft.Xna.Framework.Color;
using Matrix = Microsoft.Xna.Framework.Matrix;
using Vector2 = Microsoft.Xna.Framework.Vector2;
using Vector3 = Microsoft.Xna.Framework.Vector3;

namespace I_Robot
{
    public static class ConverterExtensions
    {
        public static Vector3 ToNormal(this Mathbox.Vector3 v)
        {
            Vector3 n = ToVector(v);
            n.Normalize();
            return n;
        }

        public static Vector3 ToVector(this Mathbox.Vector3 v)
        {
            return new Vector3(v.X, v.Y, v.Z);
        }

        public static Matrix3x3 ToMatrix3(this Mathbox.Matrix m)
        {
            const float scale = 1.0f / 0x4000;
            return new Matrix3x3(
                m.M11 * scale, m.M12 * -scale, m.M13 * scale,
                m.M21 * scale, m.M22 * -scale, m.M23 * scale,
                m.M31 * scale, m.M32 * -scale, m.M33 * scale);
        }

        public static Matrix ToMatrix4(this Mathbox.Matrix m)
        {
            const float scale = 1.0f / 0x4000;
            return new Matrix(
                m.M11 * scale, m.M12 * -scale, m.M13 * scale, 0,
                m.M21 * scale, m.M22 * -scale, m.M23 * scale, 0,
                m.M31 * scale, m.M32 * -scale, m.M33 * scale, 0,
                0, 0, 0, 1);
        }
    }

    unsafe public class MathboxRenderer : Mathbox.IInterpreter
    {
        readonly Game Game;
        readonly ScreenManager ScreenManager;

        // two video buffers on game hardware
        readonly RenderTarget2D[] Buffers = new RenderTarget2D[2];

        public RenderTarget2D VideoBuffer;
        public Texture2D Texture => Buffers[0];

        Hardware mHardware;
        public Hardware Hardware
        {
            get => mHardware;
            set { mHardware = value; Memory = value.Mathbox.Memory; }
        }

        // Pointer to Mathbox memory
        UInt16* Memory;

        Vector3 Light;
        Vector3 ViewPosition;
        Matrix ViewRotation;
        Vector3 WorldPosition;
        Matrix WorldRotation;

        UInt16 ObjectVertexTable;

        // must be retained between calls of ParseObjectList()
        UInt16 SurfaceList;

        public MathboxRenderer(ScreenManager screenManager)
        {
            ScreenManager = screenManager;

            if (!(screenManager.Game is I_Robot.Game game))
                throw new Exception("VideoInterpreter can only be used with I_Robot.Game");
            Game = game;

            // create our render target buffers
            for (int n = 0; n < Buffers.Length; n++)
            {
                Buffers[n] = new RenderTarget2D(
                    Game.GraphicsDevice,
                    Game.GraphicsDevice.Viewport.Width,
                    Game.GraphicsDevice.Viewport.Height,
                    false,
                    Game.GraphicsDevice.PresentationParameters.BackBufferFormat,
                    DepthFormat.Depth24);
            }
        }

        const UInt16 VIEW_POSITION_ADDRESS = 0x12;
        const UInt16 LIGHT_ADDRESS = 0x44;

#region HELPER
        Vector3 GetVectorAt(UInt16 address) 
        {
            System.Diagnostics.Debug.Assert(address <= (0x8000 - 3));
            return ((Mathbox.Vector3*)&Memory[address])->ToVector(); 
        }

        Vector3 GetVertexAt(UInt16 word) 
        {
            return GetVectorAt((UInt16)(ObjectVertexTable + (word & 0x3FFF)));
        }

        Matrix3x3 GetMatrix3At(UInt16 address) 
        {
            System.Diagnostics.Debug.Assert(address <= (0x8000 - 18));
            return ((Mathbox.Matrix*)&Memory[address])->ToMatrix3(); 
        }

        Matrix GetMatrix4At(UInt16 address) 
        {
            System.Diagnostics.Debug.Assert(address <= (0x8000 - 18));
            return ((Mathbox.Matrix*)&Memory[address])->ToMatrix4(); 
        }

        void LoadLightVector() { Light = -GetVectorAt(LIGHT_ADDRESS); }
        void LoadViewPosition() { ViewPosition = GetVectorAt(VIEW_POSITION_ADDRESS); }
        void LoadViewMatrix(UInt16 address)
        {
            System.Diagnostics.Debug.Assert(address < 0x8000);
            if (address == 0x787C)
                address = 0x15;
            ViewRotation = GetMatrix4At(address);
        }

        void LoadRotationMatrix(UInt16 address)
        {
            System.Diagnostics.Debug.Assert(address < 0x8000);
            if (address == 0x787C)
                address = 0x15;
            WorldRotation = GetMatrix4At(address);
        }
#endregion

#region RASTERIZER

        void StartRender()
        {

        }

        void EndRender()
        {

        }

        void SetColor(int index)
        {
            index &= 0x3F;
            // set rasterizer color
            var c = Hardware.Palette.Color[index & 0x3F];
        }

        void SetColor(int index, float shade)
        {
//            System.Diagnostics.Debug.WriteLine($"index {index & 0x3F}   offset {shade}");
            int offset = (int)shade;
            index += offset;
            index &= 0x3F;
            Color c = Hardware.Palette.Color[index];
            if ((index & 7) != 7)
                c = Color.Lerp(c, Hardware.Palette.Color[index + 1], shade - offset);
        }

        void SetWorldMatrix(ref Matrix rotation)
        {
            // Device->SetTransform(D3DTS_WORLD, &rotation);
        }

        void SetWorldMatrix(ref Vector3 position, ref Matrix rotation)
        {
            //Matrix world;
            //D3DXMatrixTranslation(&world, position.x, position.y, position.z);
            //world = rotation* world;
            //Device->SetTransform(D3DTS_WORLD, &world);
        }

        Vector3* LockVertexBuffer()
        {
            //        VertexBuffer.Object->Lock(0, 256 * sizeof(VECTOR), (VOID**) &VertexBuffer.pVertices, D3DLOCK_DISCARD);
            //        return VertexBuffer.pVertices;
            return null;
        }

        void UnlockVertexBuffer(int numvertices)
        {
            //VertexBuffer.Length = numvertices;

            // if object is to be rendered as a vector, we must close the object
            // by making the endpoint equal to the start point
            //VertexBuffer.pVertices[numvertices] = VertexBuffer.pVertices[0];

            //VertexBuffer.Object->Unlock();
        }

        void Dot()
        {
#if false
            if (Settings.ShowDots)
                Device->DrawPrimitive(D3DPT_POINTLIST, 0, VertexBuffer.Length);
#endif
        }

        void Vector()
        {
#if false
            if (VertexBuffer.Length < 2)
                Dot();
            else if (Settings.ShowVectors)
                Device->DrawPrimitive(D3DPT_LINESTRIP, 0, VertexBuffer.Length - 1 + (VertexBuffer.Length > 2));
#endif
        }

        void Polygon()
        {
#if false
            if (VertexBuffer.Length < 3)
                Vector();
            else if (Settings.ShowPolygons)
                Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, VertexBuffer.Length - 2);
#endif
        }


#endregion

#region WIP

        void ParseObjectList(UInt16 address)
        {
            int index;

            for (; ; )
            {
                // check for end of list
                if (address == 0 || address >= 0x8000)
                    return;

                // Control word encoding
                // 0x8000 = stop flag
                // 0x4000 = don't load camera matrix
                // 0x1000 = ?
                // 0x0800 = don't load base address or rotation matrix
                // 0x0400 = x/y/z value is relative offset, not absolute position
                UInt16 control = Memory[address + 3];

                // load camera matrix
                if ((control & 0x4000) == 0)
                    LoadViewMatrix(Memory[address + 4]);

                // Get new base address and rotation matrix if they exist
                if ((control & 0x0800) != 0)
                    index = 5;
                else
                {
                    SurfaceList = Memory[address + 6];
                    if (SurfaceList >= 0x8000)
                        return;
                    LoadRotationMatrix(Memory[address + 5]);
                    index = 7;
                }

                // Don't render invalid objects
                if (SurfaceList >= 0x8000)
                    return;

                // Determine position of object
                Vector3 pt = GetVectorAt(address);
                if ((control & 0x0400) != 0)
                {
                    // relative position
                    WorldPosition += Vector3.Transform(pt, ViewRotation);
                }
                else
                {
                    // absolute position
                    pt -= ViewPosition;
                    WorldPosition = Vector3.Transform(pt, ViewRotation);
                }

                SetWorldMatrix(ref WorldPosition, ref WorldRotation);

                // parese the surfaces in this object
                ParseSurfaceList(SurfaceList);

                // parse all child objects
                for (; ; )
                {
                    UInt16 child = Memory[address + (index++)];
                    if (child == 0 || child >= 0x8000)
                        return;
                    if (child == 0x0002)
                    {
                        address += 8;
                        break;
                    }

                    ParseObjectList(child);
                }
            }
        }

        void ParseSurfaceList(UInt16 address)
        {
            // get pointer to vertex list
            ObjectVertexTable = Memory[address++];

            for (; ; )
            {
                // get face pointer
                UInt16 pface = Memory[address++];

                // exit when end of surface list is encountered
                if (pface >= 0x8000)
                    return;

                // get control flags
                UInt16 flags = Memory[address++];

                // fill vertex buffer
                bool visible = RenderFace(pface, flags);

                // keep/remove hidden surface 'groups'/'chunks'
                // 8000 = jump always
                // 9000 = jump if surface is visible
                // A000 = jump if this surface is invisible
                if ((flags & 0x8000) != 0)
                {
                    if (((flags & 0x2000) != 0) && visible)
                        address++;
                    else if (((flags & 0x1000) != 0) && !visible)
                        address++;
                    else
                        address += Memory[address]; // Int16, could jump forward or backward
                }
            }
        }

        bool RenderFace(UInt16 address, int flags)
        {
            float shade = 0;

            if ((Memory[address] & 0x4000) == 0)
            {
                Vector3 normal = GetVertexAt(Memory[address]);
                normal = Vector3.Transform(normal, WorldRotation);

                Vector3 pt = GetVertexAt(Memory[address + 1]);
                Vector3.Transform(pt, WorldRotation);
                pt += WorldPosition;

                // check if surface is visible
                if (Vector3.Dot(normal, pt) <= 0)
                    return false; // not visible

                // if shading enabled
                if ((flags & 0x0040) != 0)
                {
                    const float scale = 1.0f / (1 << 25);
                    shade = Vector3.Dot(normal, -Light) * scale;
                    shade = Math.Min(7, Math.Max(0, shade));
                }
            }

            if ((flags & 0x3000) != 0)
                return true; // don't render

            // prepare color
            SetColor(flags, shade);

            // prepare the vertex buffer
            PrepareVertexBuffer(address);

            // render surface using appropriate method
            switch (flags & 0x0300)
            {
                case 0x0000: Polygon(); break;
                case 0x0100: Vector(); break;
                case 0x0200: Dot(); break;
            }

            return true;
        }

        void PrepareVertexBuffer(UInt16 address)
        {
            UInt16* pface = &Memory[address + 1];
            Vector3* dst = LockVertexBuffer();

            // add points to buffer
            for (int count = 0; ;)
            {
                UInt16 word = *pface++;
#if DEBUG
                GetVertexAt(word);
#else
                *dst++ = GetVertexAt(word);
#endif
                count++;

                if ((word & 0x8000) != 0)
                {
                    UnlockVertexBuffer(count);
                    return;
                }
            }
        }



#endregion

#region MATHBOX INTERFACE

        void Mathbox.IInterpreter.SetVideoBuffer(int index)
        {
            VideoBuffer = Buffers[index];
        }

        void Mathbox.IInterpreter.EraseVideoBuffer()
        {
            // this is essentially the same as "starting" a new display list
            // so we should clear/cache the old one while we build the new one

        }

        void Mathbox.IInterpreter.RasterizeObject(UInt16 address)
        {
            StartRender();
            LoadLightVector();
            LoadViewPosition();
            ParseObjectList(address);
            EndRender();
        }

        void Mathbox.IInterpreter.RasterizePlayfield()
        {
        }

        void Mathbox.IInterpreter.UnknownCommand()
        {
        }
#endregion

        /// <summary>
        /// Renders alphanumerics onto the overlay itself in native resolution
        /// </summary>
        /// <param name="graphicsDevice"></param>
        public void Render(GraphicsDevice graphicsDevice)
        {
            graphicsDevice.SetRenderTarget(Buffers[0]);
            graphicsDevice.Clear(Color.Transparent);
        }

        public void Draw(GraphicsDevice graphicsDevice)
        {
            ScreenManager.SpriteBatch.Begin();
            ScreenManager.SpriteBatch.Draw(Texture, Vector2.Zero, Color.White);
            ScreenManager.SpriteBatch.End();
        }
    }
}