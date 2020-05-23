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


using I_Robot.Emulation;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;

namespace I_Robot
{
    /// <summary>
    /// Represent a display list batching all of the dots, vectors, and polygons together for rendering
    /// </summary>
    class DisplayList : IEnumerable<DisplayList.Primitive>, IDisposable
    {
        /// <summary>
        /// Indiciates whether the target buffer should be erased before rendering
        /// </summary>
        public bool Erase { get; private set; }

        /// <summary>
        /// Batched dot primitives
        /// </summary>
        public readonly Primitive.Dot Dots;

        /// <summary>
        /// Batched vector primitives
        /// </summary>
        public readonly Primitive.Vector Vectors;

        /// <summary>
        /// Batched polygon primitives
        /// </summary>
        public readonly Primitive.Polygon Polygons;

        public DisplayList(MathboxRenderer mathboxRenderer)
        {
            Dots = new Primitive.Dot(mathboxRenderer);
            Vectors = new Primitive.Vector(mathboxRenderer);
            Polygons = new Primitive.Polygon(mathboxRenderer);
        }

        ~DisplayList() { Dispose(); }

        /// <summary>
        /// Resets / empties all of the primitives
        /// </summary>
        public void Reset()
        {
            Dots.Reset();
            Vectors.Reset();
            Polygons.Reset();
        }

        /// <summary>
        /// Commits the primitives for rendering
        /// </summary>
        public void Commit(bool erase)
        {
            Erase = erase;
            Dots.Commit();
            Vectors.Commit();
            Polygons.Commit();
        }

        public void Dispose()
        {
            Dots.Dispose();
            Vectors.Dispose();
            Polygons.Dispose();
        }

        IEnumerator IEnumerable.GetEnumerator() { return GetEnumerator(); }
        public IEnumerator<Primitive> GetEnumerator()
        {
            if (Dots.NumPrimitives > 0)
                yield return Dots;
            if (Vectors.NumPrimitives > 0)
                yield return Vectors;
            if (Polygons.NumPrimitives > 0)
                yield return Polygons;
        }

        /// <summary>
        /// Base class of a primitive batch to render on the display list
        /// </summary>
        public abstract class Primitive : IDisposable
        {
            const int MaxVertices = 8192;

            readonly MathboxRenderer MathboxRenderer;
            int Index;

            /// <summary>
            /// used to assemble vertices
            /// </summary>
            readonly VertexPositionColor[] Buffer = new VertexPositionColor[MaxVertices];

            /// <summary>
            /// The VertexBuffer holding the primitives
            /// </summary>
            public readonly VertexBuffer VertexBuffer;

            /// <summary>
            /// PrimitiveType for rendering
            /// </summary>
            public readonly PrimitiveType Type;

            /// <summary>
            /// The number of primitives to render
            /// </summary>
            public int NumPrimitives { get; private set; }

            public Primitive(MathboxRenderer mathboxRenderer, Mathbox.RenderMode renderMode)
            {
                MathboxRenderer = mathboxRenderer;
                VertexBuffer = new VertexBuffer(mathboxRenderer.Game.GraphicsDevice, typeof(VertexPositionColor), MaxVertices, BufferUsage.WriteOnly);
                Type = (renderMode == Mathbox.RenderMode.Vector) ? PrimitiveType.LineList : PrimitiveType.TriangleList;
            }

            ~Primitive() { Dispose(); }

            public void Dispose() { VertexBuffer.Dispose(); }

            /// <summary>
            /// Resets / empties the batch of primitives
            /// </summary>
            public void Reset()
            {
                Index = 0;
                NumPrimitives = 0;
            }

            /// <summary>
            /// Commits the batch for rendering
            /// </summary>
            public void Commit()
            {
                if (Index > 0)
                    VertexBuffer.SetData<VertexPositionColor>(Buffer, 0, Index);
            }

            /// <summary>
            /// Adds a new set of primitives to the batch
            /// </summary>
            /// <param name="vertices">buffer containing the vertices to add</param>
            /// <param name="numvertices">number of vertices to add</param>
            /// <param name="color">color of the batched primitives</param>
            abstract public void AddPrimitive(Vector3[] vertices, int numvertices, Color color);

            /// <summary>
            /// A primitive batch that renders dots
            /// </summary>
            public class Dot : Primitive
            {
                public Dot(MathboxRenderer mathboxRenderer) : base(mathboxRenderer, Mathbox.RenderMode.Dot) { }

                public override void AddPrimitive(Vector3[] vertices, int numVertices, Color color)
                {
                    System.Diagnostics.Debug.Assert(numVertices > 0);

                    if (Settings.ShowDots)
                    {
                        for (int n = 0; n < numVertices; n++)
                        {
                            float dist = Math.Abs(vertices[n].Z - MathboxRenderer.camPosition.Z) / 256;
                            Buffer[Index].Color = color;
                            Buffer[Index++].Position = vertices[n];
                            Buffer[Index].Color = color;
                            Buffer[Index++].Position = vertices[n] + dist * Vector3.Right;
                            Buffer[Index].Color = color;
                            Buffer[Index++].Position = vertices[n] + dist * Vector3.Down;

                            Buffer[Index].Color = color;
                            Buffer[Index++].Position = vertices[n] + dist * Vector3.Right;
                            Buffer[Index].Color = color;
                            Buffer[Index++].Position = vertices[n] + dist * Vector3.Right + dist * Vector3.Down;
                            Buffer[Index].Color = color;
                            Buffer[Index++].Position = vertices[n] + dist * Vector3.Down;
                        }
                        NumPrimitives += numVertices * 2;
                    }
                }
            }

            /// <summary>
            /// A primitive batch that renders vectors
            /// </summary>
            public class Vector : Primitive
            {
                public Vector(MathboxRenderer mathboxRenderer) : base(mathboxRenderer, Mathbox.RenderMode.Vector) { }

                public override void AddPrimitive(Vector3[] vertices, int numVertices, Color color)
                {
                    System.Diagnostics.Debug.Assert(numVertices > 1);
                    if (Settings.ShowVectors)
                    {
                        for (int n = 1; n < numVertices; n++)
                        {
                            Buffer[Index].Color = color;
                            Buffer[Index].Position = vertices[n - 1];
                            Buffer[Index++].Position.Z -= 1f; // move lines slightly towards view to help avoid z-buffering issues

                            Buffer[Index].Color = color;
                            Buffer[Index].Position = vertices[n];
                            Buffer[Index++].Position.Z -= 1f; // move lines slightly towards view to help avoid z-buffering issues
                        }
                        NumPrimitives += numVertices - 1;
                    }
                }
            }

            /// <summary>
            /// A primitive batch that renders polygons
            /// </summary>
            public class Polygon : Primitive
            {
                public Polygon(MathboxRenderer mathboxRenderer) : base(mathboxRenderer, Mathbox.RenderMode.Polygon) { }

                public override void AddPrimitive(Vector3[] vertices, int numVertices, Color color)
                {
                    System.Diagnostics.Debug.Assert(numVertices > 2);

                    if (Settings.ShowPolygons)
                    {
                        // turn the polygons into a triangle fan
                        for (int n = 2; n < numVertices; n++)
                        {
                            Buffer[Index].Color = color;
                            Buffer[Index++].Position = vertices[0];

                            Buffer[Index].Color = color;
                            Buffer[Index++].Position = vertices[n - 1];

                            Buffer[Index].Color = color;
                            Buffer[Index++].Position = vertices[n];

                        }
                        NumPrimitives += numVertices - 2;
                    }
                }
            }
        }


        /// <summary>
        /// A resource pool of DisplayLists
        /// </summary>
        class ResourcePool : IDisposable
        {
            readonly MathboxRenderer MathboxRenderer;
            readonly ConcurrentQueue<DisplayList> Queue = new ConcurrentQueue<DisplayList>();

            public ResourcePool(MathboxRenderer mathboxRenderer) { MathboxRenderer = mathboxRenderer; }
            ~ResourcePool() { Dispose(); }

            public DisplayList Get()
            {
                if (Queue.TryDequeue(out DisplayList? result))
                {
                    result.Reset();
                    return result;
                }
                return new DisplayList(MathboxRenderer);
            }

            public void Return(DisplayList displayList)
            {
                Queue.Enqueue(displayList);
            }

            public void Dispose()
            {
                foreach (DisplayList displayList in Queue)
                    displayList.Dispose();
            }
        }

        /// <summary>
        /// A class that manages building of DisplayLists and coordinating the rendering
        /// New DisplayLists are built up by calling AddPrimitive()
        /// Once the DisplayList is complete, Commit() will commit the list and add it to the rendering queue
        /// The renderer should then dequeue them for processing
        /// When the renderer is done it must call Return() so the DisplayList can be re-used
        /// </summary>
        public class Manager
        {
            readonly MathboxRenderer MathboxRenderer;
            readonly ResourcePool Pool;
            readonly ConcurrentQueue<DisplayList> RenderQueue = new ConcurrentQueue<DisplayList>();
            DisplayList WIP;

            public Manager(MathboxRenderer mathboxRenderer)
            {
                MathboxRenderer = mathboxRenderer;
                Pool = new ResourcePool(mathboxRenderer);
                WIP = Pool.Get();
            }

            ~Manager() { Dispose(); }

            public void Dispose()
            {
                foreach (DisplayList displayList in RenderQueue)
                    displayList.Dispose();
                Pool.Dispose();
                WIP.Dispose();
            }

            /// <summary>
            /// Gets the next DisplayList in the Render queue
            /// </summary>
            /// <param name="displayList">DisplayList returned from the queue</param>
            /// <returns>true if DisplayList was returned, false if queue was empty</returns>
            public bool GetNext(out DisplayList? displayList)
            {
                return RenderQueue.TryDequeue(out displayList);
            }

            /// <summary>
            /// Returns a DisplayList so it may be re-used
            /// </summary>
            /// <param name="displayList"></param>
            public void Return(DisplayList displayList)
            {
                Pool.Return(displayList);
            }

            /// <summary>
            /// Commits the assembled display list for rendering and initializes a new batch for assembly
            /// </summary>
            /// <param name="erase">indicates whether the target buffer should be erased before redering this display list</param>
            public void CommitDisplayList(bool erase)
            {
                WIP.Commit(erase);
                RenderQueue.Enqueue(WIP);
                WIP = Pool.Get();
            }

            /// <summary>
            /// Adds a new primitive to the DisplayList being assembled
            /// </summary>
            /// <param name="vertices"></param>
            /// <param name="numVertices"></param>
            /// <param name="color"></param>
            /// <param name="renderMode"></param>
            public void AddPrimitive(Vector3[] vertices, int numVertices, Color color, Mathbox.RenderMode renderMode)
            {
                if (renderMode == Mathbox.RenderMode.Dot || numVertices == 1)
                    WIP.Dots.AddPrimitive(vertices, numVertices, color);
                else if (renderMode == Mathbox.RenderMode.Vector || numVertices == 2)
                    WIP.Vectors.AddPrimitive(vertices, numVertices, color);
                else if (numVertices >= 3)
                    WIP.Polygons.AddPrimitive(vertices, numVertices, color);
            }

        }
    }
}