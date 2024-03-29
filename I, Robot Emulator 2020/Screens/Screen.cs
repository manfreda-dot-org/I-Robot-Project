﻿// Copyright 2020 by John Manfreda. All Rights Reserved.
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

namespace I_Robot
{
    /// <summary>
    /// The main menu screen is the first thing displayed when the game starts up.
    /// </summary>
    class Screen : GameManagement.Screen
    {
        new public readonly I_Robot.Game Game;
        public readonly Emulation.Machine Machine;
        
        public Screen(ScreenManager screenManager)
            : base(screenManager)
        {
            if (screenManager.Game is not I_Robot.Game game)
                throw new System.Exception();
            Game = game;
            Machine = Game.Machine;
        }
    }
}