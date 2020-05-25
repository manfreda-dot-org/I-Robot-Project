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

using Microsoft.Xna.Framework.Input;
using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Input;

namespace I_Robot
{
    static public class Joystick
    {
        const int HALL_JOY_MID = 128;
        const int MAX_HALL_DELTA = 40;

        class KeyboardJoystick
        {
            Axis X = new Axis(Keys.Left, Keys.Right);
            Axis Y = new Axis(Keys.Down, Keys.Up);

            class Axis
            {
                readonly Keys Positive;
                readonly Keys Negative;
                double Delta = 0;

                public Axis(Keys positive, Keys negative)
                {
                    Positive = positive;
                    Negative = negative;
                }

                int GetKeyboardDirection()
                {
                    if (Keyboard.IsPressed(Positive))
                    {
                        if (!Keyboard.IsPressed(Negative))
                            return 1;
                    }
                    else if (Keyboard.IsPressed(Negative))
                        return -1;
                    return 0;
                }

                public byte Read()
                {
                    switch (GetKeyboardDirection())
                    {
                        default:
                            Delta = 0;
                            break;
                        case 1:
                            if (Delta < 0)
                                Delta = 0;
                            Delta += 2.0 / 3.0;
                            if (Delta > MAX_HALL_DELTA)
                                Delta = MAX_HALL_DELTA;
                            break;
                        case -1:
                            if (Delta > 0)
                                Delta = 0;
                            Delta -= 2.0 / 3.0;
                            if (Delta < -MAX_HALL_DELTA)
                                Delta = -MAX_HALL_DELTA;
                            break;
                    }

                    return (byte)Math.Round(HALL_JOY_MID + Delta);
                }
            }

            public byte ADC_X => X.Read();
            public byte ADC_Y => Y.Read();
        }

        static readonly KeyboardJoystick KeyJoy = new KeyboardJoystick();

        static public byte ADC_X => KeyJoy.ADC_X;
        static public byte ADC_Y => KeyJoy.ADC_Y;
    }
}
