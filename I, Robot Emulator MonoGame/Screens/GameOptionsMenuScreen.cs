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
    class GameOptionsMenuScreen : MenuScreen
    {
        static string[] sLives = { "4", "5", "2", "3" };

        static string[] sExtraLives = { "EVERY    30,000", "EVERY    50,000", "NONE", "EVERY    20,000" };

        /// <summary>
        /// Constructor.
        /// </summary>
        public GameOptionsMenuScreen(ScreenManager screenManager)
            : base(screenManager, "GAME OPTIONS")
        {
            // create our menu entries
            MenuItems.Add(new MenuItem(DifficultyText, DifficultyMenuEntrySelected));
            MenuItems.Add(new MenuItem(LivesText, LivesMenuEntrySelected, 0.5f));
            MenuItems.Add(new MenuItem(BonusText, BonusLivesMenuEntrySelected));
            MenuItems.Add(new MenuItem(FreePlayText, FreePlayMenuEntrySelected, 0.5f));
            MenuItems.Add(new MenuItem(MinGameTimeText, MinGameTimeMenuEntrySelected, 0.5f));
            MenuItems.Add(new MenuItem(DemoModeText, DemoModeMenuEntrySelected, 0.5f));
            MenuItems.Add(new MenuItem(LanguageText, LanguageMenuEntrySelected, 0.5f));
            MenuItems.Add(new MenuItem("BACK", OnCancel, 1));
        }

        string DifficultyText => $"DIFFICULTY:    {(Settings.GameDifficulty == Settings.Difficulty.Easy ? "EASY" : "MEDIUM")}";
        string DemoModeText => $"DEMO    MODE:    {(Settings.DemoMode ? "ENABLED" : "DISABLED")}";
        string FreePlayText => $"COIN    SETTINGS:    {(Settings.FreePlay ? "FREE    PLAY" : "1    COIN    1    CREDIT")}";
        string LivesText=> $"LIVES    PER    COIN:    {sLives[(int)Settings.LivesPerCredit]}";
        string BonusText => $"BONUS    LIVES:    {sExtraLives[(int)Settings.BonusLifeInterval]}";
        string MinGameTimeText => $"MINIMUM    GAME    TIME:    {(Settings.MinimumGameTime ? "ENABLED" : "DISABLED")}";
        string LanguageText => $"LANGUAGE:    {(Settings.GameLanguage == Settings.Language.English ? "ENGLISH" : "GERMAN")}";

        void DifficultyMenuEntrySelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.GameDifficulty = (Settings.GameDifficulty == Settings.Difficulty.Easy) ? Settings.Difficulty.Medium : Settings.Difficulty.Easy;
            if (sender is MenuItem item) item.Text = DifficultyText;
        }

        void LivesMenuEntrySelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.LivesPerCredit++;
            if (Settings.LivesPerCredit > Settings.Lives._3)
                Settings.LivesPerCredit = 0;
            if (sender is MenuItem item) item.Text = LivesText;
        }

        void BonusLivesMenuEntrySelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.BonusLifeInterval++;
            if (Settings.BonusLifeInterval > Settings.BonusLives.Every_20000)
                Settings.BonusLifeInterval = 0;
            if (sender is MenuItem item) item.Text = BonusText;
        }

        void FreePlayMenuEntrySelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.FreePlay = !Settings.FreePlay;
            if (sender is MenuItem item) item.Text = FreePlayText;
        }

        void DemoModeMenuEntrySelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.DemoMode = !Settings.DemoMode;
            if (sender is MenuItem item) item.Text = DemoModeText;
        }

        void MinGameTimeMenuEntrySelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.MinimumGameTime= !Settings.MinimumGameTime;
            if (sender is MenuItem item) item.Text = MinGameTimeText;
        }

        void LanguageMenuEntrySelected(object? sender, PlayerIndexEventArgs e)
        {
            Settings.GameLanguage = (Settings.GameLanguage == Settings.Language.English) ? Settings.Language.German : Settings.Language.English;
            if (sender is MenuItem item) item.Text = LanguageText;
        }
    }
}
