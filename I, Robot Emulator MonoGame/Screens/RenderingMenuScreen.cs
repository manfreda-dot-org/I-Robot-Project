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

namespace I_Robot
{
    /// <summary>
    /// The options screen is brought up over the top of the main menu
    /// screen, and gives the user a chance to configure the game
    /// in various hopefully useful ways.
    /// </summary>
    class RenderingMenuScreen : MenuScreen
    {
        public RenderingMenuScreen(ScreenManager screenManager)
            : base(screenManager, "RENDERING")
        {
            // Create our menu items
            MenuItems.Add(new MenuItem(ShowDotsText, ShowDotsMenuSelected));
            MenuItems.Add(new MenuItem(ShowVectorsText, ShowVectorsMenuSelected));
            MenuItems.Add(new MenuItem(ShowPolygonsText, ShowPolygonsMenuSelected));
            MenuItems.Add(new MenuItem(WireframeText, WireframeMenuSelected, 0.5f));
            MenuItems.Add(new MenuItem("BACK", OnCancel, 1));
        }

        string ShowDotsText => $"SHOW    DOTS:    {(Settings.ShowDots ? "YES" : "NO")}";
        string ShowVectorsText => $"SHOW    VECTORS:    {(Settings.ShowVectors ? "YES" : "NO")}";
        string ShowPolygonsText => $"SHOW    POLYGONS:    {(Settings.ShowPolygons ? "YES" : "NO")}";
        string WireframeText => $"RENDERING    MODE:    {(Settings.Wireframe ? "WIREFRAME" : "NORMAL")}";


        void ShowDotsMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.ShowDots = !Settings.ShowDots;
            if (sender is MenuItem item) item.Text = ShowDotsText;
        }

        void ShowVectorsMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.ShowVectors = !Settings.ShowVectors;
            if (sender is MenuItem item) item.Text = ShowVectorsText;
        }

        void ShowPolygonsMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.ShowPolygons = !Settings.ShowPolygons;
            if (sender is MenuItem item) item.Text = ShowPolygonsText;
        }

        void WireframeMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.Wireframe = !Settings.Wireframe;
            if (sender is MenuItem item) item.Text = WireframeText;
        }
    }
}