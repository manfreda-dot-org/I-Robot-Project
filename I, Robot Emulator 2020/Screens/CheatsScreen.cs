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
using SharpDX.DXGI;
using System.Runtime.InteropServices;

namespace I_Robot
{
    /// <summary>
    /// The options screen is brought up over the top of the main menu
    /// screen, and gives the user a chance to configure the game
    /// in various hopefully useful ways.
    /// </summary>
    class CheatsScreen : MenuScreen
    {
        readonly Machine Machine;

        readonly string[] RenderingType = new string[] { "POLYGON", "WIREFRAME", "DOTS" };

        public CheatsScreen(ScreenManager screenManager)
            : base(screenManager, "CHEATS")
        {
            Machine? m = (Game as I_Robot.Game)?.Machine;
            if (m == null)
                throw new System.Exception();
            Machine = m;

            // Create our menu items
            MenuItems.Add(new MenuItem(RapidFireText, RapidFireMenuSelected));
            MenuItems.Add(new MenuItem(JumpsCreateBridgesText, JumpsCreateBridgesMenuSelected));
            MenuItems.Add(new MenuItem(NoRedTilesRemainText, NoRedTilesRemainMenuSelected));
            MenuItems.Add(new MenuItem(UnlimitedTransportersText, UnlimitedTransportersMenuSelected));
            MenuItems.Add(new MenuItem(UnlimitedLivesText, UnlimitedLivesMenuSelected));
            MenuItems.Add(new MenuItem(UnlimitedDoodleCityText, UnlimitedDoodleCityMenuSelected));
            MenuItems.Add(new MenuItem(PlayfieldRenderModeText, PlayfieldRenderModeMenuSelected));
            MenuItems.Add(new MenuItem("BACK", OnCancel, 1));
        }

        string JumpsCreateBridgesText => $"JUMPS    CREATE    BRIDGES:    {(Machine.Cheats.JumpsCreateBridges ? "YES" : "NO")}";
        string NoRedTilesRemainText => $"NO    RED    TILES    REMAIN:    {(Machine.Cheats.NoRedTilesRemain ? "YES" : "NO")}";
        string UnlimitedTransportersText => $"TRANSPORTERS:    {(Machine.Cheats.UnlimitedTransporters ? "UNLIMITED" : "NORMAL")}";
        string UnlimitedLivesText => $"LIVES:    {(Machine.Cheats.UnlimitedLives ? "UNLIMITED" : "NORMAL")}";
        string UnlimitedDoodleCityText => $"DOODLE    TIME:    {(Machine.Cheats.UnlimitedDoodleCity ? "FOREVER" : "NORMAL")}";
        string PlayfieldRenderModeText => $"PLAYFIELD    RENDERING:    {RenderingType[Machine.Cheats.PlayfieldRenderMode]}";
        string RapidFireText => $"RAPID    FIRE:    {(Machine.Cheats.RapidFire ? "FAST" : "NORMAL")}";



        void JumpsCreateBridgesMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Machine.Cheats.JumpsCreateBridges = !Machine.Cheats.JumpsCreateBridges;
            if (sender is MenuItem item) item.Text = JumpsCreateBridgesText;
        }

        void NoRedTilesRemainMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Machine.Cheats.NoRedTilesRemain = !Machine.Cheats.NoRedTilesRemain;
            if (sender is MenuItem item) item.Text = NoRedTilesRemainText;
        }

        void UnlimitedTransportersMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Machine.Cheats.UnlimitedTransporters = !Machine.Cheats.UnlimitedTransporters;
            if (sender is MenuItem item) item.Text = UnlimitedTransportersText;
        }

        void UnlimitedLivesMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Machine.Cheats.UnlimitedLives = !Machine.Cheats.UnlimitedLives;
            if (sender is MenuItem item) item.Text = UnlimitedLivesText;
        }

        void UnlimitedDoodleCityMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Machine.Cheats.UnlimitedDoodleCity = !Machine.Cheats.UnlimitedDoodleCity;
            if (sender is MenuItem item) item.Text = UnlimitedDoodleCityText;
        }

        void PlayfieldRenderModeMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            int m = Machine.Cheats.PlayfieldRenderMode + 1;
            if (m > 2)
                m = 0;
            Machine.Cheats.PlayfieldRenderMode = (byte)m;
            if (sender is MenuItem item) item.Text = PlayfieldRenderModeText;
        }

        void RapidFireMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Machine.Cheats.RapidFire = !Machine.Cheats.RapidFire;
            if (sender is MenuItem item) item.Text = RapidFireText;
        }
    }
}