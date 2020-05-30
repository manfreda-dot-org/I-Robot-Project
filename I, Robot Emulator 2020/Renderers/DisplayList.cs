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
using System.Diagnostics;

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
        public readonly Primitive Primitives;

        public DisplayList(MathboxRenderer mathboxRenderer)
        {
            Primitives = new Primitive(mathboxRenderer);
        }

        ~DisplayList() { Dispose(); }

        public int NumDots => Primitives.NumDots;
        public int NumVectors => Primitives.NumVectors;
        public int NumPolygons => Primitives.NumPolygons;

        /// <summary>
        /// Resets / empties all of the primitives
        /// </summary>
        public void Reset()
        {
            Primitives.Reset();
        }

        public void AddPrimitive(Mathbox.RenderMode renderMode, Vector3[] vertices, int numVertices, Color color)
        {
            Primitives.AddPrimitive(renderMode, vertices, numVertices, color);
        }

        /// <summary>
        /// Commits the primitives for rendering
        /// </summary>
        public void Commit(bool erase)
        {
            Erase = erase;
            Primitives.Commit();
        }

        public void Dispose()
        {
            Primitives.Dispose();
        }

        IEnumerator IEnumerable.GetEnumerator() { return GetEnumerator(); }
        public IEnumerator<Primitive> GetEnumerator()
        {
            if (Primitives.NumPrimitives > 0)
                yield return Primitives;
        }

        /// <summary>
        /// Base class of a primitive batch to render on the display list
        /// </summary>
        public class Primitive : IDisposable
        {
            // at least 10,000 are needed, not sure of max
            const int MaxVertices = 1000000;

            const int CylinderSides = 6;
            const float CylinderDiameter = 1.5f;
            readonly Vector3[] Radials1 = new Vector3[CylinderSides];
            readonly Vector3[] Radials2 = new Vector3[CylinderSides];

#if DEBUG
            static int LargestPrimitive = 0;
#endif

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
            public PrimitiveType Type => PrimitiveType.TriangleList;

            /// <summary>
            /// The number of primitives to render
            /// </summary>
            public int NumPrimitives { get; private set; }

            public int NumDots { get; private set; }
            public int NumVectors { get; private set; }
            public int NumPolygons { get; private set; }

            public Primitive(MathboxRenderer mathboxRenderer)
            {
                MathboxRenderer = mathboxRenderer;
                VertexBuffer = new VertexBuffer(mathboxRenderer.Game.GraphicsDevice, typeof(VertexPositionColor), MaxVertices, BufferUsage.WriteOnly);
            }

            ~Primitive() { Dispose(); }

            public void Dispose() { VertexBuffer.Dispose(); }

            /// <summary>
            /// Resets / empties the batch of primitives
            /// </summary>
            public virtual void Reset()
            {
                Index = 0;
                NumPrimitives = 0;
                NumDots = 0;
                NumVectors = 0;
                NumPolygons = 0;
            }

            /// <summary>
            /// Commits the batch for rendering
            /// </summary>
            public void Commit()
            {
#if DEBUG
                if (LargestPrimitive < Index)
                {
                    LargestPrimitive = Index;
                    Debug.WriteLine($"Largest DisplayList.Primitive = {Index} vertices ");
                }
#endif

                if (Index > 0)
                    VertexBuffer.SetData<VertexPositionColor>(Buffer, 0, Index);
            }

            /// <summary>
            /// Adds a new set of primitives to the batch
            /// </summary>
            /// <param name="renderMode">type of primitive to render</param>
            /// <param name="vertices">buffer containing the vertices to add</param>
            /// <param name="numVertices">number of vertices to add</param>
            /// <param name="color">color of the batched primitives</param>
            public void AddPrimitive(Mathbox.RenderMode renderMode, Vector3[] vertices, int numVertices, Color color)
            {
                if (renderMode == Mathbox.RenderMode.Dot || numVertices == 1)
                    AddDotPrimitive(vertices, numVertices, color);
                else if (renderMode == Mathbox.RenderMode.Vector || numVertices == 2)
                    AddVectorPrimitive(vertices, numVertices, color);
                else if (numVertices >= 3)
                    AddPolygonPrimitive(vertices, numVertices, color);

            }

            void AddDotPrimitive(Vector3[] vertices, int numVertices, Color color)
            {
#if DEBUG
                try
                {
#endif
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
                        NumDots += numVertices;
                    }
#if DEBUG
                }
                catch
                {
                    throw new Exception("Primitive buffer is too small");
                }
#endif
            }


            void AddCylinder(Vector3 point1, Vector3 point2, Color color)
            {
                float radius1 = CylinderDiameter / 2 * point1.Z / 512;
                float radius2 = CylinderDiameter / 2 * point2.Z / 512;

                Vector3 axis = point2 - point1;

                // Get two vectors perpendicular to the axis.
                Vector3 v1;
                if ((axis.Z < -0.01) || (axis.Z > 0.01))
                    v1 = new Vector3(axis.Z, axis.Z, -axis.X - axis.Y);
                else
                    v1 = new Vector3(-axis.Y - axis.Z, axis.X, axis.X);
                v1.Normalize();
                Vector3 v2 = Vector3.Cross(v1, axis); v2.Normalize();
                Vector3 axis_norm = axis; axis_norm.Normalize();

                // Make the vectors have length radius.
                Vector3 v3 = axis_norm * radius1;
                Vector3 v4 = axis_norm * radius2;

                for (int i = 0; i < CylinderSides; i++)
                {
                    double theta = i * (2 * Math.PI / CylinderSides);
                    Radials1[i] = (float)Math.Cos(theta) * v1 * radius1 + (float)Math.Sin(theta) * v2 * radius1;
                    Radials2[i] = (float)Math.Cos(theta) * v1 * radius2 + (float)Math.Sin(theta) * v2 * radius2;
                }

                for (int prev = CylinderSides - 1, i = 0; i < CylinderSides; prev = i++)
                {
                    //Trace.WriteLine($"{Index}, {NumVectors}");
                    // end caps
                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point1 - v3;
                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point1 + Radials1[prev];
                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point1 + Radials1[i];

                    // end caps
                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point2 + v4;
                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point2 + Radials2[prev];
                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point2 + Radials2[i];

                    // sides
                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point1 + Radials1[prev];
                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point1 + Radials1[i];
                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point2 + Radials2[prev];

                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point2 + Radials2[prev];
                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point1 + Radials1[i];
                    Buffer[Index].Color = color;
                    Buffer[Index++].Position = point2 + Radials2[i];
                }
            }

            void AddVectorPrimitive(Vector3[] vertices, int numVertices, Color color)
            {
                System.Diagnostics.Debug.Assert(numVertices > 1);
                if (Settings.ShowVectors)
                {
                    // close the object by drawing a line between the first and last points
                    Vector3 prev = vertices[numVertices - 1];

                    // is this a single line segment (not polyline)
                    if (numVertices == 2)
                        numVertices--;

                    for (int n = 0; n < numVertices; n++)
                    {
                        AddCylinder(prev, vertices[n], color);
                        prev = vertices[n];
                    }
                    NumVectors += numVertices;
                    NumPrimitives += 4 * CylinderSides * numVertices;
                }
            }

            void AddPolygonPrimitive(Vector3[] vertices, int numVertices, Color color)
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
                    NumPolygons++;
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
            /// <param name="renderMode"></param>
            /// <param name="vertices"></param>
            /// <param name="numVertices"></param>
            /// <param name="color"></param>
            public void AddPrimitive(Mathbox.RenderMode renderMode, Vector3[] vertices, int numVertices, Color color)
            {
                WIP.AddPrimitive(renderMode, vertices, numVertices, color);
            }
        }
    }
}