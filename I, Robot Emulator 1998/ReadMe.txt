///////////////////////////////////////////////////////////////////////////////
I, Robot : THE EMULATOR                                 Version 0.98   06/19/98
(c) 1997,1998 John Manfreda                                 All rights reserved
John.Manfreda@pcm.bosch.com
http://purplesky.simplenet.com/emucamp/softs/irobot/
///////////////////////////////////////////////////////////////////////////////

If you paid for this, you've paid too much!

This emulator is provided freeware and 'as-is'.  You agree to run this program
at your own risk, and accept all responsiblity for its use.  The author accepts
no liability for any loss or damage arising from the use of this emulator.

Permission is grated to upload this emulator to any FTP or WWW side where it
may be publicly accessed, provided:
        - this README file accompanies the executable
        - the original ROMs are NOT distributed with this emulator
Please do not distribute this emulator with the original ROM images.  Doing so
will upset both Atari and myself.  The author will not be held responsible for
any copyright violations by users of the emulator and actively discourages any
such violations.

All other forms of distribution are strictly prohibited.  This restriction
includes, but is not limited to, distribution in Internet compilation disks /
CDs, shareware libraries, magazine cover disks/cds and any other media
(with the exception noted in the previous paragraph).

Do not ask me for original ROM images.  All requests for ROMs will be ignored.
I will not respond to questions already answered in this document.

///////////////////////////////////////////////////////////////////////////////

Netizens,

This is my first emulator, and my first attempt at collaborating over the
internet.  This emulator represents many months of blood, sweat and tears on my
part.  It has been a long, enjoyable and sometimes frustrating road.  I am a
bit older and a bit wiser as a result of this emulator.

I, Robot stands as a milestone in video game history.  The first video game
with 3-D raster graphics generated in real-time.  It is a testament to both the
hardware and software inginuity present at Atari in the early 80's.  I had a
lot of fun getting into the innards of this beast.  Very 'Atari' at heart.
Lots of little tricks and shortcuts used to squeeze more out of what limited
silicon they had.  It will always stand as one of my all time favorite games.

New to this release, what everyone has been bugging me about, sound support!
Please read the requirements section if you are having sound problems, as
it seems that not all DirectSound drivers are created equal.

Enjoy
-- John Manfreda
   John.Manfreda@pcm.bosch.com
   http://purplesky.simplenet.com/emucamp/softs/irobot/

///////////////////////////////////////////////////////////////////////////////

What's new:
-----------

Version 0.98
- SOUND SUPPORT!!!!  I finally figured out what my problem was with the sound
  support.  Basically my cheap sound-blaster 'compatible' card was causing all
  the problems.  I spent 2 weeks trying to figure out why my sound code was
  generating poor audio quality, only to find out afterwards that the code ran
  fine on any machine using a real sound card.  After replacing my sound card
  with a _real_ SoundBlaster 16 _ALL_ the sound problems I was having
  vanished.
- User configuration is saved in Windows registry.  There's no longer any
  reason to hit F1 every time you run the emulator.
- High scores are now saved in Windows registry.
- Now that the user configuration is saved, the default screen size is
  once again 'Normal'.
- Now that the user configuration is saved, the default dip switch settings are
  the same as the manufacturer recommended default settings (from Atari manual)
- Fixed more flickering bugs.  At this point I think I've fixed all of
  them (knock on wood).

Version 0.97
- Fixed flickering problem with welcome text.  Stupid bug on my part.
- Game defaults to free play, since the coin switch seemed a bit sticky.
- Emulator defaults to 'extended' mode screen
- Original crappy icon repaced with much better looking one donated by
  Christian Córdova
- Video hardware timing re-analyzed.  The real game runs at approximately 61
  FPS, not 19 FPS as I had originally calculated.  This was causing major
  timing problems for the emulator.  Video rate updated accordingly.  Game play
  is now _much_ smoother and should be nearly arcade accurate.
- Video 'dots' are now aligned on 2 pixel boundaries.
- Fixed emulated video synchronization problems.  Syncronization problems were
  confusing the 6809, causing the game to run much faster than it should.
- Added 'faster but less accurate' technique for video synchronization.  It's
  marginally less accurate (you probably won't be able to tell) but is much
  faster and is great for low end systems.  Feature can be turned on/off from
  the config menu.
- Joystick support added.  Users who own analog joysticks should use the analog
  setting.  For users with digital joysticks, the digital setting works best.
- Hall effect emulation (for keyboard and digital stick users) greatly improved
- Fixed problem that was forcing a joystick self test at boot.  Although many
  machines (mine included) display the test at boot, this behaviour in my
  emulator was in fact related to a bug.  This only affects keyboard and
  digital joystick users.  Depending on the stability of the joystick, users of
  analog joysticks may/may not see the calibration screen at boot.

///////////////////////////////////////////////////////////////////////////////

Requirements:
-------------
Pentium or better system capable of running Windows 95/98 or Windows NT 4.0 or
greater.  DirectX 3.0 or greater must be installed.  I've seen it run on a
variety of different system configurations without a problem.  In general,
users running NT will need more horsepower than users running Windows 95/98.

Low end PCs shouldn't have any problem emulating the game near full speed
without sound support.  To emulate the game at full speed with high quality
sound emulation will require a fairly beefy PC.

If performance is a problem, try using the "MATHBOX" option under "VIDEO SYNC
METHOD" in the menu.  This alternative video update method requires much less
horsepower, and the results are nearly indistinguishable from the full "61 FPS"
method.  Obviously, turning sound emulation off will also improve emulator
performance.

For good sound emulation you will need a newer sound card with a good
DirectSound driver.  If you are using a cheap or older sound card, (like the
one I used to own) then your sound quality may be poor.  Sorry folks, it's a
DirectSound problem, I can't fix it.  Do not bug me with emails regarding
sound quality on your PC!

I have had reports of problems with older video cards.  If you are having video
problems, try getting the latest DirectX drivers from your video card vendor.

///////////////////////////////////////////////////////////////////////////////

ROMs:
-----
The following ROM files are necessary in order for this emulator to function.
They are:
     136029.101
     136029.102
     136029.103
     136029.104
     136029.124
     136029.125
     136029.206
     136029.207
     136029.208
     136029.209
     136029.210
     136029.405

These ROMs are copyright 1983 Atari.  You are required to own an original I,
Robot machine to use this emulator.  I have a copy of the ROMs because I own
an original I, Robot machine.  Do not ask the author for these ROMs, as all
requests for ROMs will be ignored.  Do not ask the author what this all means
if you don't understand.  If you don't understand, you probably shouldn't be
running this program.

///////////////////////////////////////////////////////////////////////////////

Features:

Basic keys:
             1 - Start #1 / view down
             2 - Start #2 / view up
            F1 - menu
            F2 - left coin
            F3 - right coin
            F4 - aux coin
             P - pause
         Arrow - move player
     Space bar - fire
        Ctrl+C - Windows clipboard copy

Menu:
     Hit F1 at any time to enter a basic menu.  Menu allows you to modify
     the dip switch settings (in green), as well as a few other nifty
     program variables (in blue):

     DIP SWITCH SETTINGS
     Typical dip switch settings

     PROGRAM OPTIONS
     Input Device:       KEYBOARD - analog emulation through keyboard arrows
                         DIGITAL JOYSTICK - analog emulation via digital
                                              joystick
                         ANALOG JOYSTICK - straight-through control of
	                                     joystick
     Sound support:      ENABLED or DISABLED if available, N-A otherwise
     Video Sync Method:  "61 FPS" is the default, which gives you video updates
                         at the arcade accurate frame rate.  An alternate
                         method of synchronization, "MATHBOX", is _much_
                         faster, but is marginally less accurate
     Screen Size:        ORIGINAL which is the arcade machine aspect, or
                         EXTENDED which makes more of the virtual playfield
                         visible.
     Show Dots:          turn ON/OFF dot rasterization
     Show Vectors:       turn ON/OFF vector rasterization
     Show Polygons:      turn ON/OFF polygon rasterization
     Display Frame Rate: SHOW or HIDE the emulated FPS
     Speed Throttling:   ON or OFF

///////////////////////////////////////////////////////////////////////////////

Notes:

General:
--------
     I prefer playing on Free Play mode with an extended screen.  For a
     challenge, those of you with fast PCs can turn speed throttling off and
     transport ahead to level 79.  Just for fun, put the game in German mode
     and get a load of those crazy speech balloons.  "Oh Ja? Warum? Darum!"

     If you are seeing performance problems, I recommend using the "MATHBOX"
     setting under "VIDEO SYNC METHOD" in the configuration menu.

     NT support will be dropped in the near future, whenever I get arount to
     porting the rasterization routines into DirectX 5.  Sorry folks, you can
     blame Microsoft :-(

Video hardware:
---------------

     I, Robot is a very different game than most coin-op classics, because all
     the graphics (aside from alphanumerics) are created and rasterized in
     real-time.  The game program creates a 3-D world in which the player
     interacts.  Unfortunately, Atari chose to squeeze this beautiful world
     onto a measely 256x232 display.  Running on the original Atari hardware,
     I'd conservatively estimate that only 70 to 80% of the working playfield
     is visible on screen at any time.

     Because my emulator ends up generating all the display graphics, none of
     the original hardware limitations apply.  I have the freedom to rasterize
     polygons at any scale on any sized video surface.

     Because ModeX is not fully supported under DirectX, I chose to use a
     640x480 screen for game output.  The larger surface allowed me to
     implement the following features :
          - Display the game at twice the original resolution (512x464), so you
            get to see twice as much detail as was originally visible in the
            arcades
          - Display more of the playfield.  The 2x screen (512x464) does not
            completely cover the 640x480 surface.  Since there is nothing in
            the game program limiting the playfield to 256x232, I was able to
            make 'more' of the game playfield visible.

Video synchronization:
----------------------

     I've recently learned quite a bit more about the I, Robot video system.
     The video synchronization code been overhauled to incorporate this
     knowledge.

     Of special interest are the patents that Atari obatined during the
     development of I, Robot.  Many of these patents are related to video
     rendering methods.  Unlike most games, I Robot is not forced to update the
     screen at 61 FPS frame rate the video hardware generates.

     I, Robot presented a challenge to the Atari engineers, because it was
     impossible to render frames fast enough to keep up with the monitor.
     Further, considering that rendering time is directly proportional to the
     number of objects on screen, it is nearly impossible to determine a
     'minimum guaranteed' rendering rate.

     Therefore, instead of forcing 61 frames to be rendered every second, the
     hardware / software is designed to allow the game play, rendering, and
     video generation to take place at different rates.  The rate at which the
     mathbox / video accelerator can rasterize polygons is completely decoupled
     from the speed of game play and the video driving circuitry.

     Based on the above, and alternate method of video synchronization
     (MATHBOX) was added.  Instead of forcing the emulator to generate a full
     61 FPS (rate of the I, Robot video driver), you can instead instruct the
     emulator to synchronize on the mathbox / video accelerator.  This helps
     squeeze out some additional performance on low end PCs, and (as far as I
     can tell) it is marginally less accurate.  The untrained eye should not be
     able to distinguish any difference.  Let me know if you see any
     significant differences in game play while running in this mode.

COOL feature - metafile dump to Window's clipboard:
---------------------------------------------------
     Yes, that's right, a  _metafile_ dump.  After studying the hardware
     graphics functions available on the hardware, it dawned on me that there
     was a 1:1 correspondance between the I, Robot display routines implemented
     in hardware and routines present in the Windows GDI.  With about 15 lines
     of code I added 100% clipboard support for the game.

     So, it is basically possible to generate a screen dump in vector format of
     any portion of the game!  If I'm remembered for contributing anything of
     value to the arcade emulation community, I hope it's this.

     TO USE: hit CTRL+C at any point in the game.  Program will BEEP after
     copying is finished.

     To get the full impact of this feature, paste the metafile into a nice
     vector editing program like PowerPoint or CorelDraw!.  Scale and print to
     a laser printer (color if you have it).  Beautiful!

     Please note that alphanumerics are not copied to the clipboard.  Only
     dots, vectors, and polygons.

///////////////////////////////////////////////////////////////////////////////

Known problems:
---------------

- I am told that this program crashes on some NT installations.  Not sure
  what the root cause is, but it is clearly related to the introduction of
  sound support.  I have myself tested the program on 5 different NT boxes (all
  of them without sound cards) and have not had a problem.  Until I can get
  access to an NT box with a sound card, this problem may persist.
- PCs using older sound cards may find that the sound quality is poor, or
  experience other problem.  This is due to DirectSound support for these cards
  and is out of my control.
- I am told that configuration saving does not work on some systems.  I have
  tested the feature on several PCs, and the only PCs which I have seen problems
  are those PCs whose registry has been disabled by a network administrator.  I
  am working towards a solution, but am unable to reproduce the problem on any
  of my own machines.

///////////////////////////////////////////////////////////////////////////////

Known bugs:
-----------
- Program refuses to run on some NT installations
- Configuration saving does not work on some PCs.
- My polygon drawing routine (written from scratch) has a bug which sometimes
  corrupts the last scan line of the display.  Take a close look at the last
  scan line in Expanded screen mode and you'll see what I'm talking about.

///////////////////////////////////////////////////////////////////////////////

To-do list:
-----------
* Figure out why sound support is preventing execution under NT.
* Figure out why registry saving does not work on all PCs.
* Look into minor polygon bug.  Bug only affects Doodle City game mode.
  Normally a small 3-d object should appear in lower left of screen while in
  this mode, I apparently draw in the wrong location for some reason.
* Rework polygon rasterizer.  Polygons are not _always_ displayed properly.
  The trained eye will notice this while grabbing jewels inside the Pyramid.
  No, I'm not talking about the 'jumping' of the jewels, that was a feature
  present in the arcades as well.  Something else....
* Texture mapping support.  Theoretically it should be very easy to add
  texture mapping support to the polygons in the game.  This can be done
  transparent to the original ROM (added during emulated rasterizing).  Wish I
  knew more about texture mapping, because this would be cool to see.
* Porting the dot/vector/polygon routines to run as primitives in DirectX 5.0
  or better.
* Move mathbox code to another thread, allowing the mathbox to run
  indepantantly of the 6809.
* Improve game timing by reworking emulation loop and allowing the Win32
  subsystem to manage the scheduling of emulation / video updates.

///////////////////////////////////////////////////////////////////////////////

Miscellany:

- Mathbox test fails when in test mode.  Basically, the problem is that on the
  x86 my emulated mathbox is _more_ accurate than the original present on the
  original I, Robot hardware.  You just can't win for losing.
- In test mode, the Dot/Vector/Polygon test and the cross-hatch and centering
  screens don't display.  No plan to fix, since it would MAJORLY slow down
  emulation.  This only affects self test mode, it does not affect normal game
  mode.
- If you look closely enough, you will notice that the "B" in "I, ROBOT" is
  corrupted.  This is from the original ROM, and _was_ corrupted in the
  original coin op.  The lower resolution made it harder to notice.
- If you look closely enough, you will notice that polygon clipping/sorting is
  not performed 100% correctly on more complex objects (most notably Big
  Brother).  Again, this behaviour stems from the original game program and is
  not an emulator bug.  Funny how you don't notice this stuff in the arcade.
- Some flickering occurs in the game that is not related to my artifact bug.
  This is mostly noticed after you have impaced with an object in the space
  wave, and during the Doodle City attract mode.  This flicker is indeed
  present in the actual coin op machine.  Funny how emulation mimics more than
  you'd care to remember.
- There are some other subtle video anomalies that occur during the game.
  They are all cues which speak volumes about how the I, Robot hardware /
  software goes about performing it's job.  See if you can spot them.

///////////////////////////////////////////////////////////////////////////////

Acknowledgments:

- Polygon routines adapted from article in Dr. Dobb's Journal
- 6809 core written by me in assembly
- DirectX routines and Win32 management written by me in straight C
- Thanks to Ron Fries for allowing me the use of his POKEY sound emulation
  routines
- Thanks to Keith Gerdes for his involvement in this project
- Thanks to Paul Kahler talking me through various timing issues, and for
  enlightening me about the Atari patents related to I, Robot
- Thanks to Duncan Brown for helping me get my I, Robot machine working
  (without which I could not have created this emulator)
- Thanks to Christian Córdova for the much better looking program icon.
- Thanks to Emulation Camp for providing space for the emulator webpage
- Thanks to Paul Forgey for the invaluable Win32 technical advice
- Thanks to Dave Theurer and Dave Sherman for this great game and the fond
  memories it's given me.