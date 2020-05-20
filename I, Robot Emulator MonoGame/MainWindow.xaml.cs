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

using SharpDX.Direct3D11;
using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;

namespace I_Robot
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    unsafe public partial class MainWindow : Window
    {
        readonly RomSet? Roms;
        readonly Hardware? Hardware;

        public MainWindow()
        {
            InitializeComponent();
            Style = (Style)FindResource(typeof(Window));

            // read the ROMs
            if (RomSet.ReadRomSetFromZipArchive("irobot.zip", out Roms, out string? errMessage) && Roms != null)
            {
                // create hardware that uses the ROMs
                Hardware = new Hardware(Roms);

                // start our timer
                DispatcherTimer timer = new DispatcherTimer();
                timer.Interval = TimeSpan.FromMilliseconds(33);
                timer.Tick += timer_Tick;
                timer.Start();
            }
            else
            {
                MessageBox.Show(errMessage, "Rom loader");
            }
        }

        protected override void OnClosed(EventArgs e)
        {
            Hardware?.Dispose();
            base.OnClosed(e);
        }

        void timer_Tick(object? sender, EventArgs e)
        {
            FPS.Text = Hardware?.FPS.ToString("FPS: 0.0") ?? "";
            LeftCoinCounter.Text = $"Left coin counter: {String.Format("{0:n0}", Settings.LeftCoinCounter)}";
            RightCoinCounter.Text = $"Right coin counter: {String.Format("{0:n0}", Settings.RightCoinCounter)}";
        }

        private void MenuItem_GameSettingsOpened(object sender, RoutedEventArgs e)
        {
            Menu_TestSwitch.IsChecked = Settings.TestSwitch;

            Menu_DemoMode.IsChecked = Settings.DemoMode;

            Menu_Easy.IsChecked = Settings.GameDifficulty == Settings.Difficulty.Easy;
            Menu_Medium.IsChecked = Settings.GameDifficulty == Settings.Difficulty.Medium;

            Menu_Lives2.IsChecked = Settings.LivesPerCredit == Settings.Lives._2;
            Menu_Lives3.IsChecked = Settings.LivesPerCredit == Settings.Lives._3;
            Menu_Lives4.IsChecked = Settings.LivesPerCredit == Settings.Lives._4;
            Menu_Lives5.IsChecked = Settings.LivesPerCredit == Settings.Lives._5;

            Menu_NoBonus.IsChecked = Settings.BonusLifeInterval == Settings.BonusLives.None;
            Menu_Bonus20000.IsChecked = Settings.BonusLifeInterval == Settings.BonusLives.Every_20000;
            Menu_Bonus30000.IsChecked = Settings.BonusLifeInterval == Settings.BonusLives.Every_30000;
            Menu_Bonus50000.IsChecked = Settings.BonusLifeInterval == Settings.BonusLives.Every_50000;

            Menu_MinGameTime.IsChecked = Settings.MinimumGameTime;

            Menu_English.IsChecked = Settings.GameLanguage == Settings.Language.English;
            Menu_German.IsChecked = Settings.GameLanguage == Settings.Language.German;

            Menu_FreePlay.IsChecked = Settings.FreePlay;
        }

        private void Menu_DemoMode_Click(object sender, RoutedEventArgs e)
        {
            Settings.DemoMode = !Settings.DemoMode;
        }

        private void Menu_Easy_Click(object sender, RoutedEventArgs e)
        {
            Settings.GameDifficulty = Settings.Difficulty.Easy;
        }

        private void Menu_Medium_Click(object sender, RoutedEventArgs e)
        {
            Settings.GameDifficulty = Settings.Difficulty.Medium;
        }

        private void Menu_MinGameTime_Click(object sender, RoutedEventArgs e)
        {
            Settings.MinimumGameTime = !Settings.MinimumGameTime;
        }

        private void Menu_English_Click(object sender, RoutedEventArgs e)
        {
            Settings.GameLanguage = Settings.Language.English;
        }

        private void Menu_German_Click(object sender, RoutedEventArgs e)
        {
            Settings.GameLanguage = Settings.Language.German;
        }

        private void MenuItem_NoBonus_Click(object sender, RoutedEventArgs e)
        {
            Settings.BonusLifeInterval = Settings.BonusLives.None;
        }

        private void Menu_Bonus20000_Click(object sender, RoutedEventArgs e)
        {
            Settings.BonusLifeInterval = Settings.BonusLives.Every_20000;
        }

        private void Menu_Bonus30000_Click(object sender, RoutedEventArgs e)
        {
            Settings.BonusLifeInterval = Settings.BonusLives.Every_30000;
        }

        private void Menu_Bonus50000_Click(object sender, RoutedEventArgs e)
        {
            Settings.BonusLifeInterval = Settings.BonusLives.Every_50000;
        }

        private void Menu_Lives2_Click(object sender, RoutedEventArgs e)
        {
            Settings.LivesPerCredit = Settings.Lives._2;
        }

        private void Menu_Lives3_Click(object sender, RoutedEventArgs e)
        {
            Settings.LivesPerCredit = Settings.Lives._3;
        }

        private void Menu_Lives4_Click(object sender, RoutedEventArgs e)
        {
            Settings.LivesPerCredit = Settings.Lives._4;
        }

        private void Menu_Lives5_Click(object sender, RoutedEventArgs e)
        {
            Settings.LivesPerCredit = Settings.Lives._5;
        }

        private void Menu_TestSwitch_Click(object sender, RoutedEventArgs e)
        {
            Settings.TestSwitch = !Settings.TestSwitch;
        }

        private void Menu_FreePlay_Click(object sender, RoutedEventArgs e)
        {
            Settings.FreePlay = !Settings.FreePlay;
        }

        private void Menu_Exit_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void Menu_EmulationSettings_SubmenuOpened(object sender, RoutedEventArgs e)
        {
            Menu_SpeedThrottle.IsChecked = Settings.SpeedThrottle;

            Menu_SoundEnabled.IsChecked = Settings.SoundEnabled;

            Menu_ShowDots.IsChecked = Settings.ShowDots;
            Menu_ShowDots.IsEnabled = !Settings.Wireframe;
            Menu_ShowVectors.IsChecked = Settings.ShowVectors;
            Menu_ShowVectors.IsEnabled = !Settings.Wireframe;
            Menu_ShowPolygons.IsChecked = Settings.ShowPolygons;
            Menu_ShowPolygons.IsEnabled = !Settings.Wireframe;

            Menu_WireframeMode.IsChecked = Settings.Wireframe;
        }

        private void Menu_SpeedThrottle_Click(object sender, RoutedEventArgs e)
        {
            Settings.SpeedThrottle = !Settings.SpeedThrottle;
        }

        private void Menu_SoundEnabled_Click(object sender, RoutedEventArgs e)
        {
            Settings.SoundEnabled = !Settings.SoundEnabled;
        }

        private void Menu_ShowDots_Click(object sender, RoutedEventArgs e)
        {
            Settings.ShowDots = !Settings.ShowDots;
        }

        private void Menu_ShowVectors_Click(object sender, RoutedEventArgs e)
        {
            Settings.ShowVectors = !Settings.ShowVectors;
        }

        private void Menu_ShowPolygons_Click(object sender, RoutedEventArgs e)
        {
            Settings.ShowPolygons = !Settings.ShowPolygons;
        }

        private void Menu_WireframeMode_Click(object sender, RoutedEventArgs e)
        {
            Settings.Wireframe = !Settings.Wireframe;
        }
    }
}