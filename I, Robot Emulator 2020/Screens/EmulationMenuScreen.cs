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
    class EmulationMenuScreen : MenuScreen
    {
        readonly MenuHeading LeftCounterHeading;
        readonly MenuHeading RightCounterHeading;

        public EmulationMenuScreen(ScreenManager screenManager)
            : base(screenManager, "EMULATION")
        {
            // create our menu entries
            MenuItems.Add(new MenuItem(TestSwitchText, TestSwitchMenuEntrySelected));
            MenuItems.Add(new MenuItem(RomDebugText, RomDebugModeMenuSelected));
            MenuItems.Add(new MenuItem(SpeedThrottleText, SpeedThrottleMenuSelected, 0.5f));
            MenuItems.Add(new MenuItem(ShowFPSText, ShowFPSMenuSelected, 0.5f));
            MenuItems.Add(LeftCounterHeading = new MenuHeading(LeftCounterText, 1));
            MenuItems.Add(RightCounterHeading = new MenuHeading(RightCounterText));
            MenuItems.Add(new MenuItem("CLEAR    COUNTERS", ClearCountersMenuSelected));
            MenuItems.Add(new MenuItem("BACK", OnCancel, 1));
        }

        string TestSwitchText => $"TEST    MODE:    {(Settings.TestSwitch ? "ENABLED" : "DISABLED")}";
        string RomDebugText => $"IROBOT    DEBUG    MODE:    {(Machine.Cheats.RomDebugMode ? "ENABLED" : "DISABLED")}";
        string ShowFPSText => $"FPS:    {(Settings.ShowFPS ? "SHOW" : "HIDE")}";
        string SpeedThrottleText => $"SPEED    THROTTLING:    {(Settings.SpeedThrottle ? "ENABLED" : "DISABLED")}";
        string LeftCounterText => $"LEFT     COIN     COUNTER    {Settings.LeftCoinCounter}";
        string RightCounterText => $"RIGHT    COIN     COUNTER:    {Settings.RightCoinCounter}";

        void TestSwitchMenuEntrySelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.TestSwitch = !Settings.TestSwitch;
            if (sender is MenuItem item) item.Text = TestSwitchText;
        }

        void RomDebugModeMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Machine.Cheats.RomDebugMode = !Machine.Cheats.RomDebugMode;
            if (sender is MenuItem item) item.Text = RomDebugText;
        }

        void SpeedThrottleMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.SpeedThrottle = !Settings.SpeedThrottle;
            if (sender is MenuItem item) item.Text = SpeedThrottleText;
        }

        void ShowFPSMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.ShowFPS = !Settings.ShowFPS;
            if (sender is MenuItem item) item.Text = ShowFPSText;
        }

        void ClearCountersMenuSelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.LeftCoinCounter = 0;
            Settings.RightCoinCounter = 0;
            LeftCounterHeading.Text = LeftCounterText;
            RightCounterHeading.Text = RightCounterText;
        }
    }
}