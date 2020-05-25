// Copyright 2003 by John Manfreda. All Rights Reserved.
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

#include "Emulator.h"
#include "Version.h"

TEmulator Emulator;

//---------------------------------------------------------------------------

// menu functions

static void ClickReset(int i) { Hardware.Boot(); }
static bool CheckPause(int i) { return Emulator.Paused; }
static void ClickPause(int i) { Emulator.TogglePause(); }
static void ClickExit(int i)  { Application->Terminate(); }

static void ClickDifficultyEasy(int i)   { Config.Dipswitch5E &= ~0x40; }
static bool CheckDifficultyEasy(int i)   { return !(Config.Dipswitch5E & 0x40); }
static void ClickDifficultyMedium(int i) { Config.Dipswitch5E |= 0x40; }
static bool CheckDifficultyMedium(int i) { return Config.Dipswitch5E & 0x40; }
static void ClickLives2(int i)           { Config.Dipswitch5E &= ~0x10; Config.Dipswitch5E |= 0x20; }
static bool CheckLives2(int i)           { return (Config.Dipswitch5E & 0x30) == 0x20; }
static void ClickLives3(int i)           { Config.Dipswitch5E |= 0x30;}
static bool CheckLives3(int i)           { return (Config.Dipswitch5E & 0x30) == 0x30; }
static void ClickLives4(int i)           { Config.Dipswitch5E &= ~0x30;}
static bool CheckLives4(int i)           { return (Config.Dipswitch5E & 0x30) == 0x00; }
static void ClickLives5(int i)           { Config.Dipswitch5E |= 0x10; Config.Dipswitch5E &= ~0x20; }
static bool CheckLives5(int i)           { return (Config.Dipswitch5E & 0x30) == 0x10; }
static void ClickBonusLife20000(int i)   { Config.Dipswitch5E |= 0x0C; }
static bool CheckBonusLife20000(int i)   { return (Config.Dipswitch5E & 0x0C) == 0x0C; }
static void ClickBonusLife30000(int i)   { Config.Dipswitch5E &= ~0x0C; }
static bool CheckBonusLife30000(int i)   { return (Config.Dipswitch5E & 0x0C) == 0x00; }
static void ClickBonusLife50000(int i)   { Config.Dipswitch5E |= 0x04; Config.Dipswitch5E &= ~0x08; }
static bool CheckBonusLife50000(int i)   { return (Config.Dipswitch5E & 0x0C) == 0x04; }
static void ClickBonusLifeNone(int i)    { Config.Dipswitch5E &= ~0x04; Config.Dipswitch5E |= 0x08; }
static bool CheckBonusLifeNone(int i)    { return (Config.Dipswitch5E & 0x0C) == 0x08; }
static void ClickFreePlay(int i)         { Config.Dipswitch3J = 0xE0; }
static bool CheckFreePlay(int i)         { return Config.Dipswitch3J == 0xE0; }
static void Click1Coin1Play(int i)       { Config.Dipswitch3J = 0x00; }
static bool Check1Coin1Play(int i)       { return Config.Dipswitch3J == 0x00; }
static void ClickEnglish(int i)          { Config.Dipswitch5E |= 0x01; }
static bool CheckEnglish(int i)          { return Config.Dipswitch5E & 0x01; }
static void ClickGerman(int i)           { Config.Dipswitch5E &= ~0x01; }
static bool CheckGerman(int i)           { return !(Config.Dipswitch5E & 0x01); }
static void ClickMinimumGameTime(int i)  { Config.Dipswitch5E ^= 0x02; }
static bool CheckMinimumGameTime(int i)  { return !(Config.Dipswitch5E & 0x02); }
static void ClickDemoMode(int i)         { Config.Dipswitch5E ^= 0x80; }
static bool CheckDemoMode(int i)         { return !(Config.Dipswitch5E & 0x80); }
static void ClickTestSwitch(int i)       { Hardware.Registers._1000 ^= TESTSWITCH; }
static bool CheckTestSwitch(int i)       { return !(Hardware.Registers._1000 & TESTSWITCH); }


static bool CheckPolygons(int i)    { return Config.ShowPolygons; }
static void ClickPolygons(int i)    { Config.ShowPolygons = !Config.ShowPolygons; }
static bool CheckVectors(int i)     { return Config.ShowVectors; }
static void ClickVectors(int i)     { Config.ShowVectors = !Config.ShowVectors; }
static bool CheckDots(int i)        { return Config.ShowDots; }
static void ClickDots(int i)        { Config.ShowDots = !Config.ShowDots; }
static bool CheckWireframe(int i)   { return Config.Wireframe; }
static void ClickWireframe(int i)   { Config.Wireframe = !Config.Wireframe; }
static bool Check1x1Scale(int i)    { return Config.Force1x1Scale; }
static void Click1x1Scale(int i)    { Config.Force1x1Scale = !Config.Force1x1Scale; }
static bool CheckTransparent(int i) { return Config.OverlayTransparency == i; }
static void ClickTransparent(int i) { Config.OverlayTransparency = i; }
static bool CheckFPS(int i)         { return Config.ShowFPS; }
static void ClickFPS(int i)         { Config.ShowFPS = !Config.ShowFPS; }

//---------------------------------------------------------------------------

__fastcall TEmulator::TEmulator()
{
}

__fastcall TEmulator::~TEmulator()
{
}


bool __fastcall TEmulator::Create(TForm* owner)
{
        TVersion Version;

        Owner = owner;
        Paused = false;

        // add welcome text
        Log.Add("I, Robot emulator (Mark 2)", RGB(255,0,0), 1000);
        Log.Add("Version " + Version.GetVersionString(), RGB(255,0,0));
        Log.Add(Version.GetDateTime(), RGB(255,0,0));
        Log.Add("© 1997, 1998, 2003 John Manfreda", RGB(255,0,0));
        Log.Add("http://mywebpages.comcast.net/lordfrito/", RGB(255,0,0));
        Log.Add("mailto:lordfrito@comcast.net", RGB(255,0,0));
        Log.Add("");

        // create game menu
        TGameMenuItem *game = Menu.AddMenu("Game");
                Menu.AddMenu("Reset", game, ClickReset);
                Menu.AddMenu("Pause", game, ClickPause, CheckPause);
                Menu.AddSeparator(game);
                Menu.AddMenu("Save state", game, NULL);
                Menu.AddMenu("Load state", game, NULL);
                Menu.AddSeparator(game);
                Menu.AddMenu("Exit", game, ClickExit);
        TGameMenuItem *config = Menu.AddMenu("Config");
                TGameMenuItem *difficulty = Menu.AddMenu("Difficulty", config);
                        Menu.AddMenu("Easy", difficulty, ClickDifficultyEasy, CheckDifficultyEasy);
                        Menu.AddMenu("Medium", difficulty, ClickDifficultyMedium, CheckDifficultyMedium);
                TGameMenuItem *lives = Menu.AddMenu("Lives", config);
                        Menu.AddMenu("2", lives, ClickLives2, CheckLives2);
                        Menu.AddMenu("3", lives, ClickLives3, CheckLives3);
                        Menu.AddMenu("4", lives, ClickLives4, CheckLives4);
                        Menu.AddMenu("5", lives, ClickLives5, CheckLives5);
                TGameMenuItem *bonus = Menu.AddMenu("Bonus life", config);
                        Menu.AddMenu("Every 20000", bonus, ClickBonusLife20000, CheckBonusLife20000);
                        Menu.AddMenu("Every 30000", bonus, ClickBonusLife30000, CheckBonusLife30000);
                        Menu.AddMenu("Every 50000", bonus, ClickBonusLife50000, CheckBonusLife50000);
                        Menu.AddMenu("None", bonus, ClickBonusLifeNone, CheckBonusLifeNone);
                TGameMenuItem *coin = Menu.AddMenu("Coin settings", config);
                        Menu.AddMenu("Free play", coin, ClickFreePlay, CheckFreePlay);
                        Menu.AddMenu("1 coin 1 play", coin, Click1Coin1Play, Check1Coin1Play);
                TGameMenuItem *language = Menu.AddMenu("Language", config);
                        Menu.AddMenu("English", language, ClickEnglish, CheckEnglish);
                        Menu.AddMenu("German", language, ClickGerman, CheckGerman);
                Menu.AddMenu("Minimum gametime", config, ClickMinimumGameTime, CheckMinimumGameTime);
                Menu.AddMenu("Demo mode", config, ClickDemoMode, CheckDemoMode);
                Menu.AddMenu("Test switch", config, ClickTestSwitch, CheckTestSwitch);
        TGameMenuItem *video = Menu.AddMenu("Video");
                Video.SetResolutionMenu(Menu.AddMenu("Resolution", video));
                Video.SetRefreshRateMenu(Menu.AddMenu("Refresh rate", video));
                Menu.AddSeparator(video);
                Menu.AddMenu("Show polygons", video, ClickPolygons, CheckPolygons);
                Menu.AddMenu("Show vectors", video, ClickVectors, CheckVectors);
                Menu.AddMenu("Show dots", video, ClickDots, CheckDots);
                Menu.AddMenu("Wireframe", video, ClickWireframe, CheckWireframe);
                Menu.AddSeparator(video);
                TGameMenuItem *trans = Menu.AddMenu("Overlay transparency", video);
                        Menu.AddMenu("None", trans, ClickTransparent, CheckTransparent, 0xFF);
                        Menu.AddMenu("25%", trans, ClickTransparent, CheckTransparent, 0xC0);
                        Menu.AddMenu("50%", trans, ClickTransparent, CheckTransparent, 0x80);
                        Menu.AddMenu("75%", trans, ClickTransparent, CheckTransparent, 0x40);
                        Menu.AddMenu("Full", trans, ClickTransparent, CheckTransparent, 0x00);
                Menu.AddMenu("Scale 1:1", video, Click1x1Scale, Check1x1Scale);
                Menu.AddMenu("Show FPS", video, ClickFPS, CheckFPS);
        TGameMenuItem *audio = Menu.AddMenu("Audio");
                Menu.AddMenu("TBD", audio);
        TGameMenuItem *misc = Menu.AddMenu("Miscellaneous");
                Menu.AddMenu("TBD", misc);

        // initialize input object
        if (!GameInput.Create(Owner))
        {
                Application->MessageBox("Failed to initialize input system.", "Problem", MB_OK | MB_ICONSTOP);
                return false;
        }

        // initialize sound object
        // ???

        // initialize video
        if (!Video.Create(Owner))
        {
                Application->MessageBox("Failed to initialize video system.", "Problem", MB_OK | MB_ICONSTOP);
                return false;
        }

        // boot hardware, we're ready to go!
        Hardware.Boot();

        return true;
}

bool __fastcall TEmulator::Run()
{
        // sample game inputs
        GameInput.Update();

        // check pause button
        if (GameInput.GetKeyPress(DIK_PAUSE))
                TogglePause();

        if (!Paused && !Menu.Visible())
                Hardware.CPU.Execute();

        // process the current sound buffer
//        Sound.Process();

                // display screen and perform speed throttling once per frame
/*                if (Video.Scanline == VBLANK_SCANLINE)
                {
                        // display new screen
                        if (RenderEveryFrame)
                                DisplayScreenBuffer();

                        // Perform speed throttling
                        if (SpeedThrottling && CounterExists)
                        {
                                do
                                {
                                        QueryPerformanceCounter( &Delta );
                                        Delta.QuadPart -= Timer.QuadPart;
                                } while (Delta.QuadPart < Timeout.QuadPart);
                                QueryPerformanceCounter( &Timer );
                        }
                }
*/

        // render the game video
        Video.Render();

        return true;
}

void __fastcall TEmulator::Resize()
{
        Video.Create(Owner);
}

void __fastcall TEmulator::Repaint()
{
        Video.Repaint();
}

void __fastcall TEmulator::TogglePause()
{
        Paused = !Paused;
        Log.Add(Paused ? "Paused" : "Unpaused");
}

