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
using System.Diagnostics;
using System.Security.Cryptography;
using static I_Robot.Emulation.Mathbox;
using Color = Microsoft.Xna.Framework.Color;
using Matrix = Microsoft.Xna.Framework.Matrix;
using Matrix3x3 = SharpDX.Matrix3x3;
using Texture2D = Microsoft.Xna.Framework.Graphics.Texture2D;
using Vector2 = Microsoft.Xna.Framework.Vector2;
using Vector3 = Microsoft.Xna.Framework.Vector3;

namespace I_Robot
{
    /// <summary>
    /// I, Robot Mathbox interpreter
    /// This class holds the non-portable code related to rasterization of in-game objects
    /// </summary>
    unsafe public class MathboxRenderer : IRasterizer
    {
        public class Factory : IRasterizer.Factory
        {
            readonly ScreenManager ScreenManager;

            public Factory(ScreenManager screenManager)
            {
                ScreenManager = screenManager;
            }

            IRasterizer IRasterizer.Factory.CreateRasterizer(Machine machine)
            {
                // All of our screens have empty constructors so we can just use Activator
                return new MathboxRenderer(machine, ScreenManager);
            }
        }

        // special addresses in mathbox memory
        const UInt16 VIEW_POSITION_ADDRESS = 0x12;
        const UInt16 LIGHT_ADDRESS = 0x44;
        const UInt16 PLAYFIELD_OBJECT_LIST_ADDRESS = 0x6B;
        const UInt16 PLAYFIELD_Z_MAX = 0x6C;
        const UInt16 PLAYFIELD_Z_MIN = 0x6D;
        const UInt16 PLAYFIELD_ROWS_BASE_ADDRESS = 0x70;
        const UInt16 PLAYFIELD_RENDERING_MODE = 0x72;
        const UInt16 PLAYFIELD_X = 0x74;
        const UInt16 PLAYFIELD_Z_FRAC = 0x75;
        const UInt16 PLAYFIELD_X_OFFSET = 0x76;
        const UInt16 PLAYFIELD_Z_OFFSET = 0x77;

        class TimeAccumulator
        {
            Stopwatch Stopwatch = new Stopwatch();

            long Base = 0;

            public TimeAccumulator()
            {
                Restart();
            }

            public void Restart()
            {
                Stopwatch.Restart();
                Base = 0;
            }

            public int ElapsedMilliseconds
            {
                get { return (int)(Stopwatch.ElapsedMilliseconds - Base); }
            }

            public void RemoveTime(int time)
            {
                Base += time;
            }
        }


        public readonly Game Game;
        public readonly ScreenManager ScreenManager;

        readonly Machine Machine;
        readonly UInt16[] Memory; // Pointer to Mathbox memory

        readonly TimeAccumulator Timer = new TimeAccumulator();
        int TotalDots = 0;
        int TotalVectors = 0;
        int TotalPolygons = 0;
        public int DotsPerSecond { get; private set; }
        public int VectorsPerSecond { get; private set; }
        public int PolygonsPerSecond { get; private set; }

        // two video buffers on game hardware
        readonly RenderTarget2D[] ScreenBuffers = new RenderTarget2D[2];

        // a buffer to render our scene into
        // when done, we will paint the scene onto our ScreenBuffer

        readonly RenderTarget2D SceneBuffer;

        // BUFSEL controls which buffer is being rendered to and which is being displayed
        bool BUFSEL = false;
        RenderTarget2D ScreenBuffer => ScreenBuffers[BUFSEL ? 0 : 1];
        public Texture2D Texture => ScreenBuffer;

        bool mERASE = true;
        bool mEXT_DONE = true;

        // emulated vectors and matrices
        Vector3 Light;
        Vector3 ViewPosition;
        Matrix ViewRotation;
        Vector3 WorldPosition;
        Matrix WorldRotation;
        Matrix D3DTS_WORLD;

        UInt16 ObjectVertexTable;

        // must be retained between calls of ParseObjectList()
        UInt16 SurfaceList;

        class PlayfieldRenderer
        {
            readonly MathboxRenderer Parent;
            readonly DisplayList.Manager DisplayListManager;

            readonly Vector3[] Vertices = new Vector3[10];

            // Pointer to Mathbox memory
            readonly UInt16[] Memory;

            class RowInfo
            {
                public int Count;
                public int ObjectCount;
                public UInt16 Prev;
                public UInt16 This;
                public UInt16 Next;
            }

            Mathbox.RenderMode Mode;
            Matrix Rotation;
            UInt16 ObjectList;
            int TileTableBaseAddress;

            readonly RowInfo Row = new RowInfo();

            public PlayfieldRenderer(MathboxRenderer mathboxRenderer)
            {
                Parent = mathboxRenderer;
                Memory = Parent.Memory;
                DisplayListManager = Parent.DisplayListManager;
            }

            public void Rasterize()
            {
                // locate the tile table in memory
                System.Diagnostics.Debug.Assert(Memory[PLAYFIELD_ROWS_BASE_ADDRESS] == 0xE00);
                TileTableBaseAddress = Memory[PLAYFIELD_ROWS_BASE_ADDRESS]; // always 0x0E00

                Parent.StartRender();

                Parent.LoadLightVector();
                Parent.LoadViewPosition();
                Parent.LoadViewMatrix(0x15);
                Rotation = Parent.ViewRotation;
                Parent.SetWorldMatrix(ref Parent.Playfield.Rotation);

                // determine rendering method (dot/vector/polygon)
                switch (Memory[PLAYFIELD_RENDERING_MODE])
                {
                    case 0x0000: Mode = Mathbox.RenderMode.Polygon; break;
                    case 0x0100: Mode = Mathbox.RenderMode.Vector; break;
                    default: Mode = Mathbox.RenderMode.Dot; break;
                }

                // get address of playfield object buffer
                ObjectList = Memory[PLAYFIELD_OBJECT_LIST_ADDRESS];

                Int16 z_max = (Int16)Memory[PLAYFIELD_Z_MAX];
                Int16 z_min = (Int16)Memory[PLAYFIELD_Z_MIN];
                Int16 x = (Int16)Memory[PLAYFIELD_X];
                Int16 z_frac = (Int16)Memory[PLAYFIELD_Z_FRAC];
                Int16 x_offset = (Int16)Memory[PLAYFIELD_X_OFFSET];
                Int16 z_offset = (Int16)Memory[PLAYFIELD_Z_OFFSET];

                // locate left front tile corner (x1,y1,z1)
                //         +-------+
                //        /       /|
                //       /       / |           x2 = x1 + Mathbox.Tile.SIZE_X
                // y1-- +-------+  |           y2 = y1 + Mathbox.Tile.SIZE_Y
                //      |       |  + --z2      z2 = z1 + Mathbox.Tile.SIZE_Z
                //      |       | /
                //      |       |/
                // y2-- +-------+ --z1
                //      |       |
                //      x1      x2
                Vector3 corner = new Vector3(
                    Tile.WIDTH_X - x_offset * 128 - x,
                    -Parent.ViewPosition.Y,
                    z_max * 128 - z_frac);

                // render each row in the playfield
                int row = z_offset / 16 + z_max; // first absolute row to be rendered
                Row.Count = z_max - z_min + 1; // number of rows to display
                while (Row.Count-- > 0)
                {
                    DrawPlayfieldRow(row-- & 31, corner);
                    corner.Z -= Tile.DEPTH_Z; // move to next row along the Z axis
                }

                Parent.EndRender();
            }

            void DrawPlayfieldRow(int row, Vector3 corner)
            {
                // Get address of the three rows that determine how tiles
                // in the current row are drawn
                Row.Prev = (UInt16)( TileTableBaseAddress + 16 * ((row - 1) & 31));
                Row.This = (UInt16)(TileTableBaseAddress + 16 * row);
                Row.Next = (UInt16)(TileTableBaseAddress + 16 * ((row + 1) & 31));

                // there are 15 tiles per row
                // some are to the left of the camera, some to the right
                // we render from outside -> in, stopping once camera is reached

                int TilesLeft = 15;

                // reset row object counter
                Row.ObjectCount = 0;

                // left side (not including center)
                if (corner.X < -Mathbox.Tile.WIDTH_X)
                {
                    int n = 1;
                    while (corner.X < -Mathbox.Tile.WIDTH_X && TilesLeft > 0)
                    {
                        DrawPlayfieldTile(n++, corner);
                        corner.X += Mathbox.Tile.WIDTH_X;
                        TilesLeft--;
                    }
                }

                // right side (includes center)
                if (TilesLeft > 0)
                {
                    corner.X += (TilesLeft - 1) * Mathbox.Tile.WIDTH_X;

                    int n = 15;
                    while (TilesLeft-- > 0)
                    {
                        DrawPlayfieldTile(n--, corner);
                        corner.X -= Mathbox.Tile.WIDTH_X;
                    }
                }

                // any object lists to deal with?
                if (Row.ObjectCount > 0)
                {
                    // render the objects above this row
                    do
                    {
                        int count = Memory[ObjectList++] + 1;
                        while (count-- > 0)
                            Parent.ParseObjectList(Memory[ObjectList++]);
                    } while (--Row.ObjectCount > 0);

                    // reset the world matrix to what it was before
                    Parent.SetWorldMatrix(ref Rotation);
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

            TILE_HEIGHT GetTileHeight(Tile a, Tile b, Tile c, Tile d)
            {
                if (a.IsEmpty)
                {
                    // no tile
                    float h = Mathbox.Tile.DEFAULT_HEIGHT_Y - Parent.ViewPosition.Y;
                    return new TILE_HEIGHT(h, h, h, h);
                }
                else if (a.IsFlat)
                {
                    // surface is flat
                    float h = a.Height - Parent.ViewPosition.Y;
                    return new TILE_HEIGHT(h, h, h, h);
                }
                else
                {
                    // surface is sloped
                    return new TILE_HEIGHT(
                        a.Height - Parent.ViewPosition.Y,
                        b.Height - Parent.ViewPosition.Y,
                        c.Height - Parent.ViewPosition.Y,
                        d.Height - Parent.ViewPosition.Y);
                }
            }

            void DrawPlayfieldTile(int index, Vector3 corner)
            {
                // local tile map (we are rendering tile A)
                //  TileF   TileD   TileC
                //  TileE   TileA   TileB
                //          TileG   TileH

                // get this tile
                Tile TileA = Memory[Row.This + ((index + 0) & 15)];

                // check if object must be rendered with this tile
                if (TileA.HoldsObject)
                    Row.ObjectCount++;

                // check if tile is empty
                if (TileA.IsEmpty)
                    return; // nothing to draw

                // get base tile color
                int colorIndex = TileA.ColorIndex;

                // locate tile corners
                //       d +-------+ c
                //        /       /|
                //       /       / |          x2 = x1 + Tile.WIDTH_X
                // y1-- +-------+  |          y2 = y1 + Tile.DEFAULT_HEIGHT_Y
                //      |a     b| g+ --z2     z2 = z1 + Tile.DEPTH_Z
                //      |       | /
                //      |e     f|/
                // y2-- +-------+ --z1
                //      |       |
                //      x1      x2
                float x1 = corner.X;
                float x2 = x1 + Tile.WIDTH_X;
                float z1 = corner.Z;
                float z2 = corner.Z + Tile.DEPTH_Z;

                // determine tile height offset
                Tile TileB = Memory[Row.This + ((index + 1) & 15)];
                Tile TileC = Memory[Row.Next + ((index + 1) & 15)];
                Tile TileD = Memory[Row.Next + ((index + 0) & 15)];
                TILE_HEIGHT height = GetTileHeight(TileA, TileB, TileC, TileD);

                // draw left or right side of cube
                if (x1 > 0)
                {
                    // left side of the cube
                    TILE_HEIGHT side;

                    if (index == 1)
                        // leftmost tile
                        side.b = side.c = -Parent.ViewPosition.Y + Tile.DEFAULT_HEIGHT_Y;
                    else
                    {
                        Tile TileE = Memory[Row.This + ((index - 1) & 15)];
                        Tile TileF = Memory[Row.Next + ((index - 1) & 15)];
                        side = GetTileHeight(TileE, TileA, TileD, TileF);
                    }

                    // draw if tile to the left is lower than current tile
                    if (height.a < side.b || height.d < side.c)
                    {
                        Vertices[0] = new Vector3(x1, height.a, z1);
                        Vertices[1] = new Vector3(x1, height.d, z2);
                        Vertices[2] = new Vector3(x1, side.c, z2);
                        Vertices[3] = new Vector3(x1, side.b, z1);
                        DisplayListManager.AddPrimitive(Vertices, 4, Parent.GetColor(colorIndex - 1), Mode);
                    }
                }
                else if (x2 < 0)
                {
                    // right side of the cube
                    TILE_HEIGHT side;

                    if (index == 15)
                        // rightmost tile
                        side.a = side.d = -Parent.ViewPosition.Y + Tile.DEFAULT_HEIGHT_Y;
                    else
                        side = GetTileHeight(TileB, TileB, TileC, TileC);

                    // draw if tile to the right is lower than current tile
                    if (height.b < side.a || height.c < side.d)
                    {
                        Vertices[0] = new Vector3(x2, height.b, z1);
                        Vertices[1] = new Vector3(x2, side.a, z1);
                        Vertices[2] = new Vector3(x2, side.d, z2);
                        Vertices[3] = new Vector3(x2, height.c, z2);
                        DisplayListManager.AddPrimitive(Vertices, 4, Parent.GetColor(colorIndex - 1), Mode);
                    }
                }

                // if tile is flat
                if (TileA.IsFlat)
                {
                    // Draw the front of the cube if:
                    //   - This is the last row to render
                    // OR
                    //   - Tile in front is lower than current tile (or empty)
                    TILE_HEIGHT side;
                    if (Row.Count == 0)
                        side.c = side.d = -Parent.ViewPosition.Y + Tile.DEFAULT_HEIGHT_Y;
                    else
                    {
                        Tile TileG = Memory[Row.Prev + ((index + 0) & 15)];
                        Tile TileH = Memory[Row.Prev + ((index + 10) & 15)];
                        side = GetTileHeight(TileG, TileH, TileB, TileA);
                    }
                    if (height.a < side.d || height.b < side.c)
                    {
                        Vertices[0] = new Vector3(x1, height.a, z1);
                        Vertices[1] = new Vector3(x1, side.d, z1);
                        Vertices[2] = new Vector3(x2, side.c, z1);
                        Vertices[3] = new Vector3(x2, height.b, z1);
                        DisplayListManager.AddPrimitive(Vertices, 4, Parent.GetColor(colorIndex - 2), Mode);
                    }
                }

                // draw the top of the cube
                Vertices[0] = new Vector3(x1, height.a, z1);
                Vertices[1] = new Vector3(x2, height.b, z1);
                Vertices[2] = new Vector3(x2, height.c, z2);
                Vertices[3] = new Vector3(x1, height.d, z2);
                DisplayListManager.AddPrimitive(Vertices, 4, Parent.GetColor(colorIndex), Mode);

                // special case for rendering solid sloped tiles
                // this is done for maximum compatibility with real machine
                if (!TileA.IsFlat && Mode == Mathbox.RenderMode.Polygon)
                    DisplayListManager.AddPrimitive(Vertices, 4, Parent.GetColor(colorIndex), Mathbox.RenderMode.Vector);

                // draw the bottom of the cube
                Vertices[0] = new Vector3(x1, -Parent.ViewPosition.Y + Tile.DEFAULT_HEIGHT_Y, z1);
                Vertices[1] = new Vector3(x1, -Parent.ViewPosition.Y + Tile.DEFAULT_HEIGHT_Y, z2);
                Vertices[2] = new Vector3(x2, -Parent.ViewPosition.Y + Tile.DEFAULT_HEIGHT_Y, z2);
                Vertices[3] = new Vector3(x2, -Parent.ViewPosition.Y + Tile.DEFAULT_HEIGHT_Y, z1);
                DisplayListManager.AddPrimitive(Vertices, 4, Parent.GetColor(colorIndex), Mode);
            }
        }
        readonly PlayfieldRenderer Playfield;


        readonly DisplayList.Manager DisplayListManager;
        readonly Vector3[] Vertices = new Vector3[2048];
        Vector3 camTarget;
        public Vector3 camPosition { get; private set; }
        Matrix projectionMatrix;
        Matrix viewMatrix;
        Matrix worldMatrix;
        BasicEffect basicEffect;

        MathboxRenderer(Machine machine, ScreenManager screenManager)
        {
            Machine = machine;
            Memory = Machine.Mathbox.Memory16;
            ScreenManager = screenManager;

            if (!(screenManager.Game is I_Robot.Game game))
                throw new Exception("VideoInterpreter can only be used with I_Robot.Game");
            Game = game;

            DisplayListManager = new DisplayList.Manager(this);
            Playfield = new PlayfieldRenderer(this);

            // create our scene buffer
            // this buffer has a z-buffer
            SceneBuffer = new RenderTarget2D(
                Game.GraphicsDevice,
                Game.GraphicsDevice.Viewport.Width,
                Game.GraphicsDevice.Viewport.Height,
                false,
                Game.GraphicsDevice.PresentationParameters.BackBufferFormat,
                DepthFormat.Depth24,
                8,
                RenderTargetUsage.DiscardContents);

            // create our two screen buffers
            // these buffers do not require depth sorting, they are simply raw bitmaps
            // however the contents need to be preserved when rendering context is reset
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
            DisplayListManager.Dispose();
            foreach (var b in ScreenBuffers)
                b.Dispose();
        }

        #region HELPER
        Vector3 GetVectorAt(UInt16 address)
        {
            System.Diagnostics.Debug.Assert(address <= (0x8000 - 3));
            return new Vector3((Int16)Memory[address], (Int16)Memory[address + 1], (Int16)Memory[address + 2]);
        }

        Vector3 GetVertexFromTable(UInt16 word)
        {
            return GetVectorAt((UInt16)(ObjectVertexTable + (word & 0x3FFF)));
        }

        Matrix GetMatrix4At(UInt16 address)
        {
            System.Diagnostics.Debug.Assert(address <= (0x8000 - 18));

            const float scale = 1.0f / 0x4000;
            return new Matrix(
                (Int16)Memory[address + 0] * scale, (Int16)Memory[address + 3] * scale, (Int16)Memory[address + 6] * scale, 0,
                (Int16)Memory[address + 1] * scale, (Int16)Memory[address + 4] * scale, (Int16)Memory[address + 7] * scale, 0,
                (Int16)Memory[address + 2] * scale, (Int16)Memory[address + 5] * scale, (Int16)Memory[address + 8] * scale, 0,
                0, 0, 0, 1);
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
            D3DTS_WORLD = rotation;
        }

        void SetWorldMatrix(ref Vector3 position, ref Matrix rotation)
        {
            D3DTS_WORLD = rotation * Matrix.CreateTranslation(position);
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
                Vector3 normal = GetVertexFromTable(Memory[address]);
                normal = Vector3.Transform(normal, WorldRotation);

                // get the first coordinate
                Vector3 pt = GetVertexFromTable(Memory[address + 1]);
                pt = Vector3.Transform(pt, D3DTS_WORLD);

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
            BuildNewPrimitive(address, color, (Mathbox.RenderMode)flags & Mathbox.RenderMode.Mask);

            return true;
        }

        void BuildNewPrimitive(UInt16 address, Color color, Mathbox.RenderMode type)
        {
            // add points to buffer
            for (int index = 0; ;)
            {
                UInt16 word = Memory[++address];

                Vector3 pt = GetVertexFromTable(word);
                pt = Vector3.Transform(pt, D3DTS_WORLD);
                Vertices[index++] = pt;

                // is this the last point of the plygon?
                if ((word & 0x8000) != 0)
                {
                    DisplayListManager.AddPrimitive(Vertices, index, color, type);
                    return;
                }
            }
        }



        #endregion

        #region EMULATOR INTERFACE

        bool IRasterizer.EXT_DONE => mEXT_DONE;

        void IRasterizer.ERASE(bool bufsel)
        {
            mERASE = true;
        }

        void IRasterizer.EXT_START(bool bufsel)
        {
            // simulate rasterizer being busy
            mEXT_DONE = false;

            // select buffer as necessary
            //BUFSEL = bufsel

            // commit the new display list
            DisplayListManager.CommitDisplayList(mERASE); // commit the display list
            Machine.EmulatorTrace($"CommitDisplayList({mERASE})");
            mERASE = false;

            // simulate rasterizer being done
            // technically this should take place after rendering is complete
            // however this might cause frames to be dropped
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
            Playfield.Rasterize();
        }

        void IRasterizer.UnknownCommand()
        {
        }
        #endregion

        /// <summary>
        /// Renders any queued display lists
        /// </summary>
        /// <param name="graphicsDevice"></param>
        public void Render(GraphicsDevice graphicsDevice)
        {
            CheckPolysPerSecond();

            // is there a new display list to render?
            while (DisplayListManager.GetNext(out DisplayList? displayList) && displayList != null)
            {
                viewMatrix = Matrix.CreateLookAt(camPosition, camTarget, Vector3.Up);
                //                viewMatrix = Matrix.CreateTranslation(0, -26f / 128, 0) * viewMatrix;


                graphicsDevice.SetRenderTarget(SceneBuffer);
                graphicsDevice.DepthStencilState = DepthStencilState.Default;
                graphicsDevice.Clear(Color.Transparent);

                basicEffect.Projection = projectionMatrix;
                basicEffect.View = Matrix.CreateScale(1.0f / 128) * viewMatrix;
                basicEffect.World = worldMatrix;

                if (Settings.Wireframe)
                {
                    RasterizerState prevRasterizerState = graphicsDevice.RasterizerState;
                    graphicsDevice.RasterizerState = new RasterizerState()
                    {
                        FillMode = FillMode.WireFrame,
                        CullMode = CullMode.None,
                        MultiSampleAntiAlias = true,
                    };
                }
                else
                {
                    graphicsDevice.RasterizerState = new RasterizerState()
                    {
                        CullMode = CullMode.CullClockwiseFace,
                        MultiSampleAntiAlias = true,
                    };
                }

                foreach (DisplayList.Primitive primitive in displayList)
                {
                    System.Diagnostics.Debug.Assert(primitive.NumPrimitives > 0);

                    graphicsDevice.SetVertexBuffer(primitive.VertexBuffer);

                    foreach (EffectPass pass in basicEffect.CurrentTechnique.Passes)
                    {
                        pass.Apply();
                        graphicsDevice.DrawPrimitives(primitive.Type, 0, primitive.NumPrimitives);
                    }
                }

                mEXT_DONE = true;

                // now copy our newly rendered scene ontop of the current screen buffer
                graphicsDevice.SetRenderTarget(ScreenBuffer);
                if (displayList.Erase)
                    graphicsDevice.Clear(Color.Transparent);
                ScreenManager.SpriteBatch.Begin();
                ScreenManager.SpriteBatch.Draw(SceneBuffer, Vector2.Zero, Color.White);
                ScreenManager.SpriteBatch.End();

                TotalDots += displayList.NumDots;
                TotalVectors += displayList.NumVectors;
                TotalPolygons += displayList.NumPolygons;
                DisplayListManager.Return(displayList);
            }

            CheckPolysPerSecond();
        }

        void CheckPolysPerSecond()
        {
            int ms = Timer.ElapsedMilliseconds;
            if (ms >= 1000)
            {
                if (ms < 1100)
                {
                    DotsPerSecond = TotalDots;
                    VectorsPerSecond = TotalVectors;
                    PolygonsPerSecond = TotalPolygons;
                }

                Timer.RemoveTime(ms);

                TotalDots = 0;
                TotalVectors = 0;
                TotalPolygons = 0;
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