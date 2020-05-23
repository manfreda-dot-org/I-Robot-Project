// Copyright 2020 by John Manfreda. All Rights Reserved.
// https://www.manfreda.org/
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see<https://www.gnu.org/licenses/>.

using GameManagement;
using I_Robot.Emulation;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Windows.Shapes;
using Color = Microsoft.Xna.Framework.Color;
using Matrix = Microsoft.Xna.Framework.Matrix;
using Matrix3x3 = SharpDX.Matrix3x3;
using Texture2D = Microsoft.Xna.Framework.Graphics.Texture2D;
using Vector2 = Microsoft.Xna.Framework.Vector2;
using Vector3 = Microsoft.Xna.Framework.Vector3;

namespace I_Robot
{
    unsafe public class MathboxRenderer : IRasterizer
    {
        readonly Game Game;
        readonly ScreenManager ScreenManager;

        Machine mMachine;
        public Machine Machine
        {
            get => mMachine;
            set { mMachine = value; Memory = value.Mathbox.Memory; }
        }


        // two video buffers on game hardware
        readonly RenderTarget2D[] ScreenBuffers = new RenderTarget2D[2];

        // a buffer to render our scene into
        // when done, we will paint the scene onto our ScreenBuffer

        readonly RenderTarget2D SceneBuffer;

        // BUFSEL controls which buffer is being rendered to and which is being displayed
        bool BUFSEL = false;
        RenderTarget2D ScreenBuffer => ScreenBuffers[BUFSEL ? 0 : 1];
        public Texture2D Texture => ScreenBuffers[BUFSEL ? 0 : 1];

        bool EXT_START;
        bool ERASE;
        bool mEXT_DONE = true;


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

        class PlayfieldManagement
        {
            public class RowInfo
            {
                public int Count;
                public int Objects;
                public UInt16* Prev;
                public UInt16* This;
                public UInt16* Next;
            }

            public Mathbox.RenderMode Mode;
            public Matrix Rotation;
            public UInt16 ObjectList;

            public readonly RowInfo Row = new RowInfo();
        }
        readonly PlayfieldManagement Playfield = new PlayfieldManagement();

        /// <summary>
        /// Represents a primitive to render on the display list
        /// </summary>
        class DisplayListPrimitive : IDisposable
        {
            /// <summary>
            /// The RenderMode of the primitive
            /// </summary>
            public Mathbox.RenderMode RenderMode;

            /// <summary>
            /// The number of primitives to render
            /// </summary>
            public int NumPrimitives;

            /// <summary>
            /// The VertexBuffer holding the primitives
            /// </summary>
            public readonly VertexBuffer VertexBuffer;

            public DisplayListPrimitive(GraphicsDevice graphicsDevice)
            {
                VertexBuffer = new VertexBuffer(graphicsDevice, typeof(VertexPositionColor), 1024 * 3, BufferUsage.WriteOnly);
            }

            public void Dispose()
            {
                VertexBuffer.Dispose();
            }
        }

        /// <summary>
        /// A class that manages building of a display list and coordiantion with rendering.
        /// Essentially there are two display list buffers
        ///   - one being read from (displayed)
        ///   - one being written to (built)
        /// When Commit() is called the two buffers are swapped
        /// </summary>
        class DisplayListBuilder : IEnumerable<DisplayListPrimitive>, IDisposable
        {
            readonly GraphicsDevice GraphicsDevice;

            // two buffers
            // - one buffer is always active (ready for render)
            // - the other buffer is always available for staging new display list commands
            public ConcurrentStack<DisplayListPrimitive>[] SwapBuffer = new ConcurrentStack<DisplayListPrimitive>[2];

            // buffer selection
            ConcurrentStack<DisplayListPrimitive> Committed => SwapBuffer[Index & 1];
            ConcurrentStack<DisplayListPrimitive> Staging => SwapBuffer[(Index & 1) ^ 1];
            int Index = 0; // incrementing this flips the two buffers

            // a resource pool of display list elements to be reused
            readonly ConcurrentQueue<DisplayListPrimitive> Pool = new ConcurrentQueue<DisplayListPrimitive>();
            int Count = 0;

            public DisplayListBuilder(GraphicsDevice graphicsDevice)
            {
                GraphicsDevice = graphicsDevice;
                for (int n = 0; n < SwapBuffer.Length; n++)
                    SwapBuffer[n] = new ConcurrentStack<DisplayListPrimitive>();
            }

            public void Dispose()
            {
                // move all elements back into Pool
                Commit();
                Commit();

                foreach (var v in Pool)
                    v.Dispose();
            }

            public IEnumerator<DisplayListPrimitive> GetEnumerator()
            {
                return ((IEnumerable<DisplayListPrimitive>)Committed).GetEnumerator();
            }

            IEnumerator IEnumerable.GetEnumerator()
            {
                return ((IEnumerable<DisplayListPrimitive>)Committed).GetEnumerator();
            }

            public DisplayListPrimitive GetNext()
            {
                if (!Pool.TryDequeue(out DisplayListPrimitive? result))
                {
                    result = new DisplayListPrimitive(GraphicsDevice);
                    Count++;
                    //                    System.Diagnostics.Debug.WriteLine($"new VertexBuffer(), total = {Count}");
                }
                Staging.Push(result);
                return result;
            }

            /// <summary>
            /// Commits the active display list for rendering
            /// </summary>
            public void Commit()
            {
                // bump index, which causes active / staging buffers to be swapped
                Index++;

                // reusue objects from previous display list
                foreach (var v in Staging)
                    Pool.Enqueue(v);
                Staging.Clear();
            }
        }

        readonly DisplayListBuilder DisplayList;
        Vector3[] Vertices = new Vector3[2048];
        Vector3 camTarget;
        Vector3 camPosition;
        Matrix projectionMatrix;
        Matrix viewMatrix;
        Matrix worldMatrix;
        BasicEffect basicEffect;

        public MathboxRenderer(ScreenManager screenManager)
        {
            ScreenManager = screenManager;

            if (!(screenManager.Game is I_Robot.Game game))
                throw new Exception("VideoInterpreter can only be used with I_Robot.Game");
            Game = game;

            DisplayList = new DisplayListBuilder(Game.GraphicsDevice);

            SceneBuffer = new RenderTarget2D(
             Game.GraphicsDevice,
             Game.GraphicsDevice.Viewport.Width,
             Game.GraphicsDevice.Viewport.Height,
             false,
             Game.GraphicsDevice.PresentationParameters.BackBufferFormat,
             DepthFormat.Depth24);

            // create our render target buffers
            for (int n = 0; n < ScreenBuffers.Length; n++)
            {
                ScreenBuffers[n] = new RenderTarget2D(
                    Game.GraphicsDevice,
                    Game.GraphicsDevice.Viewport.Width,
                    Game.GraphicsDevice.Viewport.Height,
                    false,
                    Game.GraphicsDevice.PresentationParameters.BackBufferFormat,
                    DepthFormat.None,
                    0,
                    RenderTargetUsage.PreserveContents);
            }

            //Setup Camera
            camTarget = new Vector3(0f, 0f, 1f);
            camPosition = new Vector3(0f, 0f, 0f);

            double scaleToMonitor = Emulation.Machine.MonitorAspectRatio / Emulation.Machine.NativeAspectRatio;

            projectionMatrix = Matrix.CreatePerspectiveFieldOfView(
                MathHelper.ToRadians(45f),
                (float)(Game.GraphicsDevice.Viewport.AspectRatio / scaleToMonitor),
                0.1f,
                65536f);
            viewMatrix = Matrix.CreateLookAt(camPosition, camTarget, Vector3.Up); // Y up
            worldMatrix = Matrix.CreateWorld(camTarget, Vector3.Forward, Vector3.Down);
            basicEffect = new BasicEffect(Game.GraphicsDevice);
            basicEffect.Alpha = 1f;
            basicEffect.VertexColorEnabled = true; // Want to see the colors of the vertices, this needs to be on
            // Lighting requires normal information which VertexPositionColor does not have
            // If you want to use lighting and VPC you need to create a custom def
            basicEffect.LightingEnabled = false;
        }


        public void Dispose()
        {
            DisplayList.Dispose();
            foreach (var b in ScreenBuffers)
                b.Dispose();
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

        void LoadLightVector() { Light = GetVectorAt(LIGHT_ADDRESS); }
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

        Color GetColor(int index)
        {
            // set rasterizer color
            return Machine.Palette.Color[index & 0x3F];
        }

        Color GetColor(int index, float shade)
        {
            int offset = (int)shade;
            index += offset;
            Color c = Machine.Palette.Color[index & 0x3F];
            if ((index & 7) != 7)
                c = Color.Lerp(c, Machine.Palette.Color[(index + 1) & 0x3F], shade - offset);
            return c;
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

        VertexPositionColor[] buf = new VertexPositionColor[2048];
        void CommitPrimitive(int numvertices, Color color, Mathbox.RenderMode renderMode)
        {
            if (numvertices <= 0)
                return;
            else if (numvertices == 1)
                renderMode = Mathbox.RenderMode.Dot;
            else if (numvertices == 2 && renderMode == Mathbox.RenderMode.Polygon)
                renderMode = Mathbox.RenderMode.Vector;

            int i = 0;
            int numPrimitives = 0;

            switch (renderMode)
            {
                default: return;
                case Mathbox.RenderMode.Dot:
                    for (int n = 0; n < numvertices; n++)
                    {
                        buf[i++].Position = Vertices[n];
                        buf[i++].Position = Vertices[n] + 10 * Vector3.Right;
                        buf[i++].Position = Vertices[n] + 10 * Vector3.Down;

                        buf[i++].Position = Vertices[n] + 10 * Vector3.Right;
                        buf[i++].Position = Vertices[n] + 10 * Vector3.Right + 10 * Vector3.Down;
                        buf[i++].Position = Vertices[n] + 10 * Vector3.Down;
                    }
                    numPrimitives = numvertices * 2;
                    break;
                case Mathbox.RenderMode.Vector:
                    for (int n = 0; n < numvertices; n++)
                        buf[i++].Position = Vertices[n];
                    numPrimitives = numvertices - 1;
                    break;
                case Mathbox.RenderMode.Polygon:
                    // convert triangle fan
                    int i1 = 1;
                    int i2 = numvertices - 1;

                    buf[i++].Position = Vertices[0];
                    buf[i++].Position = Vertices[i1];
                    for (; ; )
                    {
                        buf[i++].Position = Vertices[i2];
                        int next = i1 + 1;
                        if (next == i2)
                            break;
                        i1 = i2; i2 = next;

                        buf[i++].Position = Vertices[i2];
                        next = i1 - 1;
                        if (next == i2)
                            break;
                        i1 = i2; i2 = next;

                    }
                    numPrimitives = numvertices - 2;
                    break;
            }

            for (int n = 0; n < i; n++)
                buf[n].Color = color;

            var obj = DisplayList.GetNext();
            obj.RenderMode = renderMode;
            obj.NumPrimitives = numPrimitives;
            obj.VertexBuffer.SetData<VertexPositionColor>(buf, 0, i);
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

            // should we check the normal vector to see if this polygon is visible
            if ((Memory[address] & 0x4000) == 0)
            {
                // get the normal vector
                Vector3 normal = GetVertexAt(Memory[address]);
                normal = Vector3.Transform(normal, WorldRotation);

                // get the first coordinate
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
                    shade = Vector3.Dot(normal, Light) * scale;
                    shade = Math.Min(7, Math.Max(0, shade));
                }
            }

            if ((flags & 0x3000) != 0)
                return true; // don't render

            // prepare color
            Color color = GetColor(flags, shade);

            // prepare the vertex buffer
            PrepareVertexBuffer(address, color, (Mathbox.RenderMode)flags & Mathbox.RenderMode.Mask);

            return true;
        }

        void PrepareVertexBuffer(UInt16 address, Color color, Mathbox.RenderMode type)
        {
            UInt16* pface = &Memory[address + 1];

            // add points to buffer
            for (int index = 0; ;)
            {
                UInt16 word = *pface++;

                Vector3 pt = GetVertexAt(word);
                pt = Vector3.Transform(pt, WorldRotation) + WorldPosition;
                Vertices[index++] = pt;

                // is this the last point of the plygon?
                if ((word & 0x8000) != 0)
                {
                    CommitPrimitive(index, color, type);
                    return;
                }
            }
        }



        #endregion

        #region EMULATOR INTERFACE

        bool IRasterizer.EXT_DONE => mEXT_DONE;

        void IRasterizer.EXT_START(bool bufsel, bool erase)
        {
            // simulate rasterizer being busy
            mEXT_DONE = false;

            // commit the new display list
            //BUFSEL = bufsel; // select buffer as necessary
            ERASE = erase; // erase as necesasary
            DisplayList.Commit(); // commit the display list

            // start processing of the newly committed display list
            EXT_START = true;

            // simulate rasterizer being done
            // makes more sense moving this to when render is complete (provided if it doesn't cause frames to be dropped)
            mEXT_DONE = true;
        }

        void IRasterizer.RasterizeObject(UInt16 address)
        {
            StartRender();
            LoadLightVector();
            LoadViewPosition();
            ParseObjectList(address);
            EndRender();
        }

        void IRasterizer.RasterizePlayfield()
        {
            StartRender();

            LoadLightVector();
            LoadViewPosition();
            LoadViewMatrix(0x15);
            Playfield.Rotation = ViewRotation;
            SetWorldMatrix(ref Playfield.Rotation);

            // determine rendering method (dot/vector/polygon)
            switch (Memory[0x72])
            {
                case 0x0000: Playfield.Mode = Mathbox.RenderMode.Polygon; break;
                case 0x0100: Playfield.Mode = Mathbox.RenderMode.Vector; break;
                default: Playfield.Mode = Mathbox.RenderMode.Dot; break;
            }

            // get address of playfield object buffer
            Playfield.ObjectList = Memory[0x6B];

            Int16 z_max = (Int16)Memory[0x6C];
            Int16 z_min = (Int16)Memory[0x6D];
            Int16 x = (Int16)Memory[0x74];
            Int16 z_frac = (Int16)Memory[0x75];
            Int16 x_offset = (Int16)Memory[0x76];
            Int16 z_offset = (Int16)Memory[0x77];

            // locate left front tile corner (x1,y1,z1)
            //         +-------+
            //        /       /|
            //       /       / |           x2 = x1 + TILE_SIZE_X
            // y1-- +-------+  |           y2 = y1 + TILE_SIZE_Y
            //      |       |  + --z2      z2 = z1 + TILE_SIZE_Z
            //      |       | /
            //      |       |/
            // y2-- +-------+ --z1
            //      |       |
            //      x1      x2
            Vector3 corner = new Vector3(
                Mathbox.TILE_SIZE_X - x_offset * 128 - x,
                -ViewPosition.Y,
                z_max * 128 - z_frac);

            // render each row in the playfield
            int row = z_offset / 16 + z_max; // first absolute row to be rendered
            Playfield.Row.Count = z_max - z_min + 1; // number of rows to display
            while (Playfield.Row.Count-- > 0)
            {
                DrawPlayfieldRow(row-- & 31, corner);
                corner.Z -= Mathbox.TILE_SIZE_Z; // move to next row along the Z axis
            }

            EndRender();
        }

        void IRasterizer.UnknownCommand()
        {
        }
        #endregion

        void DrawPlayfieldRow(int row, Vector3 corner)
        {
            // Get address of the three rows that determine how tiles
            // in the current row are drawn
            int rowbase = 0xE00; // Memory[0x70]
            Playfield.Row.Prev = &Memory[rowbase + 16 * ((row - 1) & 31)];
            Playfield.Row.This = &Memory[rowbase + 16 * row];
            Playfield.Row.Next = &Memory[rowbase + 16 * ((row + 1) & 31)];

            // there are 15 tiles per row
            // some are to the left of the camera, some to the right
            // we render from outside -> in, stopping once camera is reached

            int TilesLeft = 15;

            // reset row object counter
            Playfield.Row.Objects = 0;

            // left side (not including center)
            if (corner.X < -Mathbox.TILE_SIZE_X)
            {
                int n = 1;
                while (corner.X < -Mathbox.TILE_SIZE_X && TilesLeft > 0)
                {
                    DrawPlayfieldTile(n++, corner);
                    corner.X += Mathbox.TILE_SIZE_X;
                    TilesLeft--;
                }
            }

            // right side (includes center)
            if (TilesLeft > 0)
            {
                corner.X += (TilesLeft - 1) * Mathbox.TILE_SIZE_X;

                int n = 15;
                while (TilesLeft-- > 0)
                {
                    DrawPlayfieldTile(n--, corner);
                    corner.X -= Mathbox.TILE_SIZE_X;
                }
            }

            // any object lists to deal with?
            if (Playfield.Row.Objects > 0)
            {
                // render the objects above this row
                do
                {
                    int count = Memory[Playfield.ObjectList++] + 1;
                    while (count-- > 0)
                        ParseObjectList(Memory[Playfield.ObjectList++]);
                } while (--Playfield.Row.Objects > 0);

                // reset the world matrix to what it was before
                SetWorldMatrix(ref Playfield.Rotation);
            }
        }

        struct TILE_HEIGHT
        {
            public float a, b, c, d;
            public TILE_HEIGHT(float _a, float _b, float _c, float _d)
            {
                a = _a;
                b = _b;
                c = _c;
                d = _d;
            }
        }

        TILE_HEIGHT GetTileHeight(UInt16 a, UInt16 b, UInt16 c, UInt16 d)
        {
            // #define TileHeight(tile) (((sbyte)((tile) >> 8)) << 2)

            if ((a & 0xFF00) == 0x8000)
            {
                // no tile
                float h = -ViewPosition.Y + Mathbox.TILE_SIZE_Y;
                return new TILE_HEIGHT(h, h, h, h);
            }
            else if ((a & 0x0040) != 0)
            {
                // surface is flat
                float h = -ViewPosition.Y + (((sbyte)((a) >> 8)) << 2);
                return new TILE_HEIGHT(h, h, h, h);
            }
            else
            {
                // surface is sloped
                return new TILE_HEIGHT(-ViewPosition.Y + (((sbyte)((a) >> 8)) << 2),
                -ViewPosition.Y + (((sbyte)((b) >> 8)) << 2),
                -ViewPosition.Y + (((sbyte)((c) >> 8)) << 2),
                -ViewPosition.Y + (((sbyte)((d) >> 8)) << 2));
            }
        }

        void DrawPlayfieldTile(int index, Vector3 corner)
        {
            // local tile map (we are rendering tile A)
            //  TileF   TileD   TileC
            //  TileE   TileA   TileB
            //          TileG   TileH

            // get this tile
            UInt16 TileA = Playfield.Row.This[(index + 0) & 15];

            // check if object must be rendered with this tile
            if ((TileA & 0x0080) != 0)
                Playfield.Row.Objects++;

            // check if tile is empty
            if ((TileA & 0xFF00) == 0x8000)
                return; // nothing to draw

            // get base tile color
            int colorIndex = TileA & 0x003F;

            // locate tile corners
            //       d +-------+ c
            //        /       /|
            //       /       / |          x2 = x1 + TILE_SIZE_X
            // y1-- +-------+  |          y2 = y1 + TILE_SIZE_Y
            //      |a     b| g+ --z2     z2 = z1 + TILE_SIZE_Z
            //      |       | /
            //      |e     f|/
            // y2-- +-------+ --z1
            //      |       |
            //      x1      x2
            float x1 = corner.X;
            float x2 = x1 + Mathbox.TILE_SIZE_X;
            float z1 = corner.Z;
            float z2 = corner.Z + Mathbox.TILE_SIZE_Z;

            // determine tile height offset
            UInt16 TileB = Playfield.Row.This[(index + 1) & 15];
            UInt16 TileC = Playfield.Row.Next[(index + 1) & 15];
            UInt16 TileD = Playfield.Row.Next[(index + 0) & 15];
            TILE_HEIGHT height = GetTileHeight(TileA, TileB, TileC, TileD);

            // draw left or right side of cube
            if (x1 > 0)
            {
                // left side of the cube
                TILE_HEIGHT side;

                if (index == 1)
                    // leftmost tile
                    side.b = side.c = -ViewPosition.Y + Mathbox.TILE_SIZE_Y;
                else
                {
                    UInt16 TileE = Playfield.Row.This[(index - 1) & 15];
                    UInt16 TileF = Playfield.Row.Next[(index - 1) & 15];
                    side = GetTileHeight(TileE, TileA, TileD, TileF);
                }

                // draw if tile to the left is lower than current tile
                if (height.a < side.b || height.d < side.c)
                {
                    Vertices[0] = new Vector3(x1, height.a, z1);
                    Vertices[1] = new Vector3(x1, height.d, z2);
                    Vertices[2] = new Vector3(x1, side.c, z2);
                    Vertices[3] = new Vector3(x1, side.b, z1);
                    CommitPrimitive(4, GetColor(colorIndex - 1), Playfield.Mode);
                }
            }
            else if (x2 < 0)
            {
                // right side of the cube
                TILE_HEIGHT side;

                if (index == 15)
                    // rightmost tile
                    side.a = side.d = -ViewPosition.Y + Mathbox.TILE_SIZE_Y;
                else
                    side = GetTileHeight(TileB, TileB, TileC, TileC);

                // draw if tile to the right is lower than current tile
                if (height.b < side.a || height.c < side.d)
                {
                    Vertices[0] = new Vector3(x2, height.b, z1);
                    Vertices[1] = new Vector3(x2, side.a, z1);
                    Vertices[2] = new Vector3(x2, side.d, z2);
                    Vertices[3] = new Vector3(x2, height.c, z2);
                    CommitPrimitive(4, GetColor(colorIndex - 1), Playfield.Mode);
                }
            }

            // if tile is flat
            if ((TileA & 0x0040) != 0)
            {
                // Draw the front of the cube if:
                //   - This is the last row to render
                // OR
                //   - Tile in front is lower than current tile (or empty)
                TILE_HEIGHT side;
                if (Playfield.Row.Count == 0)
                    side.c = side.d = -ViewPosition.Y + Mathbox.TILE_SIZE_Y;
                else
                {
                    UInt16 TileG = Playfield.Row.Prev[(index + 0) & 15];
                    UInt16 TileH = Playfield.Row.Prev[(index + 1) & 15];
                    side = GetTileHeight(TileG, TileH, TileB, TileA);
                }
                if (height.a < side.d || height.b < side.c)
                {
                    Vertices[0] = new Vector3(x1, height.a, z1);
                    Vertices[1] = new Vector3(x1, side.d, z1);
                    Vertices[2] = new Vector3(x2, side.c, z1);
                    Vertices[3] = new Vector3(x2, height.b, z1);
                    CommitPrimitive(4, GetColor(colorIndex - 2), Playfield.Mode);
                }
            }

            // draw the top of the cube
            Vertices[0] = new Vector3(x1, height.a, z1);
            Vertices[1] = new Vector3(x2, height.b, z1);
            Vertices[2] = new Vector3(x2, height.c, z2);
            Vertices[3] = new Vector3(x1, height.d, z2);
            CommitPrimitive(4, GetColor(colorIndex), Playfield.Mode);

            // special case for rendering solid sloped tiles
            // this is done for maximum compatibility with real machine
            if ((TileA & 0x0040) == 0 && Playfield.Mode == Mathbox.RenderMode.Polygon)
                CommitPrimitive(4, GetColor(colorIndex), Mathbox.RenderMode.Vector);

            // draw the bottom of the cube
            Vertices[0] = new Vector3(x1, -ViewPosition.Y + Mathbox.TILE_SIZE_Y, z1);
            Vertices[1] = new Vector3(x1, -ViewPosition.Y + Mathbox.TILE_SIZE_Y, z2);
            Vertices[2] = new Vector3(x2, -ViewPosition.Y + Mathbox.TILE_SIZE_Y, z2);
            Vertices[3] = new Vector3(x2, -ViewPosition.Y + Mathbox.TILE_SIZE_Y, z1);
            CommitPrimitive(4, GetColor(colorIndex), Playfield.Mode);
        }


        /// <summary>
        /// Renders alphanumerics onto the overlay itself in native resolution
        /// </summary>
        /// <param name="graphicsDevice"></param>
        public void Render(GraphicsDevice graphicsDevice)
        {
            // if game has kicked off rendering
            if (EXT_START)
            {
                // it's a one shot
                EXT_START = false;

                viewMatrix = Matrix.CreateLookAt(camPosition, camTarget, Vector3.Up);
                //                viewMatrix = Matrix.CreateTranslation(0, -26f / 128, 0) * viewMatrix;


                graphicsDevice.SetRenderTarget(SceneBuffer);
                graphicsDevice.DepthStencilState = DepthStencilState.Default;
                graphicsDevice.Clear(Color.Transparent);


                basicEffect.Projection = projectionMatrix;
                basicEffect.View = Matrix.CreateScale(1.0f / 128) * viewMatrix;
                basicEffect.World = worldMatrix;

                foreach (DisplayListPrimitive primitive in DisplayList)
                {
                    graphicsDevice.SetVertexBuffer(primitive.VertexBuffer);

                    //Turn off culling so we see both sides of our rendered triangle
                    graphicsDevice.RasterizerState = RasterizerState.CullNone;

                    foreach (EffectPass pass in basicEffect.CurrentTechnique.Passes)
                    {
                        pass.Apply();
                        switch (primitive.RenderMode)
                        {
                            case Mathbox.RenderMode.Dot:
                                graphicsDevice.DrawPrimitives(PrimitiveType.TriangleList, 0, primitive.NumPrimitives);
                                break;
                            case Mathbox.RenderMode.Vector:
                                graphicsDevice.DrawPrimitives(PrimitiveType.LineStrip, 0, primitive.NumPrimitives);
                                break;
                            case Mathbox.RenderMode.Polygon:
                                graphicsDevice.DrawPrimitives(PrimitiveType.TriangleStrip, 0, primitive.NumPrimitives);
                                break;
                        }
                    }
                }

                //EXT_DONE = true;

                // now copy our newly rendered scene ontop of the current screen buffer
                graphicsDevice.SetRenderTarget(ScreenBuffer);
                if (ERASE)
                    graphicsDevice.Clear(Color.Transparent);
                ScreenManager.SpriteBatch.Begin();
                ScreenManager.SpriteBatch.Draw(SceneBuffer, Vector2.Zero, Color.White);
                ScreenManager.SpriteBatch.End();
            }
        }

        public void Draw(GraphicsDevice graphicsDevice)
        {
            ScreenManager.SpriteBatch.Begin();
            ScreenManager.SpriteBatch.Draw(Texture, Vector2.Zero, Color.White);
            ScreenManager.SpriteBatch.End();
        }
    }
}