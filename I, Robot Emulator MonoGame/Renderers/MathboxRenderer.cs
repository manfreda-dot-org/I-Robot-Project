using GameManagement;
using I_Robot.Emulation;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Color = Microsoft.Xna.Framework.Color;
using Matrix = Microsoft.Xna.Framework.Matrix;
using Matrix3x3 = SharpDX.Matrix3x3;
using Texture2D = Microsoft.Xna.Framework.Graphics.Texture2D;
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
                m.M11 * scale, m.M12 * scale, m.M13 * scale,
                m.M21 * scale, m.M22 * scale, m.M23 * scale,
                m.M31 * scale, m.M32 * scale, m.M33 * scale);
        }

        public static Matrix ToMatrix4(this Mathbox.Matrix m)
        {
            const float scale = 1.0f / 0x4000;
            return new Matrix(
                m.M11 * scale, m.M12 * scale, m.M13 * scale, 0,
                m.M21 * scale, m.M22 * scale, m.M23 * scale, 0,
                m.M31 * scale, m.M32 * scale, m.M33 * scale, 0,
                0, 0, 0, 1);
        }
    }

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

        Vector3[] LockVertexBuffer()
        {
            return Vertices;
        }

        VertexPositionColor[] buf = new VertexPositionColor[2048];
        void UnlockVertexBuffer(int numvertices, Color color, Mathbox.RenderMode renderMode)
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
                    int a = 0;
                    int b = 1;
                    int c = numvertices - 1;

                    buf[i++].Position = Vertices[a];
                    buf[i++].Position = Vertices[b];
                    for (; ; )
                    {
                        buf[i++].Position = Vertices[c];
                        int next = b + 1;
                        a = b; b = c; c = next;
                        if (b == c)
                            break;

                        buf[i++].Position = Vertices[c];
                        next = b - 1;
                        a = b; b = c; c = next;
                        if (b == c)
                            break;

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
            Vector3[] dst = LockVertexBuffer();

            // add points to buffer
            for (int index = 0; ;)
            {
                UInt16 word = *pface++;

                Vector3 pt = GetVertexAt(word);
                pt = Vector3.Transform(pt, WorldRotation) + WorldPosition;
                dst[index++] = pt;

                // is this the last point of the plygon?
                if ((word & 0x8000) != 0)
                {
                    UnlockVertexBuffer(index, color, type);
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
        }

        void IRasterizer.UnknownCommand()
        {
        }
        #endregion

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