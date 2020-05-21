using GameManagement;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Media;
using System;
using System.Collections.Generic;
using System.Net.Security;
using System.Text;

namespace I_Robot
{
    unsafe public class MathboxRenderer : Mathbox.IInterpreter
    {
        readonly Game Game;
        readonly ScreenManager ScreenManager;

        // two video buffers on game hardware
        readonly RenderTarget2D[] Buffers = new RenderTarget2D[2];

        public RenderTarget2D VideoBuffer;
        public Texture2D Texture => Buffers[0];

        // Pointer to Mathbox memory
        UInt16* Memory;

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

        #region MATHBOX INTERFACE

        public unsafe ushort* pMemory { set => Memory = value; }

        void Mathbox.IInterpreter.SetVideoBuffer(int index)
        {
            VideoBuffer = Buffers[index];
        }

        void Mathbox.IInterpreter.EraseVideoBuffer()
        {
            // this is essentially the same as "starting" a new display list
            // so we should clear/cache the old one while we build the new one

        }

        void Mathbox.IInterpreter.RasterizeObject(ushort address)
        {
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
