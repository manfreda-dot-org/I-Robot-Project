using GameManagement;
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
        readonly RenderTarget2D[] VideoBuffer = new RenderTarget2D[2];

        // Pointer to Mathbox memory
        UInt16* Memory;

        public MathboxRenderer(ScreenManager screenManager)
        {
            ScreenManager = screenManager;

            if (!(screenManager.Game is I_Robot.Game game))
                throw new Exception("VideoInterpreter can only be used with I_Robot.Game");
            Game = game;

            // create our render target buffers
            for (int n = 0; n < VideoBuffer.Length; n++)
            {
                VideoBuffer[n] = new RenderTarget2D(
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
        }

        void Mathbox.IInterpreter.EraseVideoBuffer()
        {
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

        public void Draw(GraphicsDevice graphicsDevice)
        {

        }
    }
}
