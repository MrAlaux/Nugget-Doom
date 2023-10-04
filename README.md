# Nugget Doom

Nugget Doom is a fork of [Woof!](https://github.com/fabiangreffrath/woof), simply intended to implement additional features.

**Note:** this README and the rest of documentation are updated on a per-commit basis,
meaning that they may not correspond to the latest release of Nugget Doom.
If you're seeking information on the version you're using, please refer to the README included with it.

### DISCLAIMER
Although the new code has been written with the intention of not breaking demo compatibility, it has not been properly tested yet.
**RECORD DEMOS AT YOUR OWN RISK!**

## Features

Note that this feature list is relative to [Woof!'s](https://github.com/fabiangreffrath/woof/blob/master/README.md#key-features);
read the latter for more details.
Some features were first introduced in Nugget Doom and later merged into Woof!, and so they're not listed here anymore.

Most of Nugget Doom's features come from other sources, like source ports and mods;
the initial implementations for some are **ported from (p.f.)** said sources, while others are just **inspired by (i.b.)** them.
These acknowledgements are included in the feature lists below; be aware that some might be inaccurate or outright missing.

A few settings are labeled as _**CFG-Only**_: they can only be toggled by editing `nugget-doom.cfg`.
For these settings, their CVAR names are provided alongside the _CFG-Only_ label as guidance.

### General

- **Support for higher resolutions:** 4X (800p) and 8X (1600p)
- **Selection of widescreen ratios** in the setup menu itself [i.b. Crispy Doom]
- Toggle to **stretch viewport to fit window** (CFG-Only: `stretch_to_fit`) [i.b. and partially p.f. Crispy Doom; i.b. ZDoom]
- **Gamma Correction slider ranging from 0.50 to 2.0 in steps of 0.05**
- Toggle for **Gamma Correction key cycling**, through either original or extended gamma levels (CFG-Only: `gammacycle`)
- **Menu items for _Air Absorption_ and _Doppler Effect_**, both of which are set to 5 by default
- **Tweaked _Stretch Short Skies_ algorithm**
- _**Screen Wipe Style**_ selection:
  - _"Seizure"_ (**Warning**: this one might legitimately be seizure-inducing, use with caution);
  - _"Fade"_.
- **Fixed screen melt inconsistency across resolutions**
- **Extended _Level Brightness_ range:** [-8, 8]
- **_"Direct + Auto"_ mode for Vertical Aiming**
- _**Things move over/under things**_ setting [p.f. Crispy Doom]
- **Jumping** (default key: <kbd>Alt</kbd>, must be enabled first) [p.f. Crispy Doom]
- **Crouching/ducking** (default key: <kbd>C</kbd>, must be enabled first) [i.b. ZDoom]
- _**Field of View**_ setting [p.f. Doom Retro]
- _**View Height**_ setting, which allows to enter a custom height value between 32 and 56 for the player's POV [i.b. Brutal Doom]
- _**View Bobbing Percentage**_ setting [i.b. Crispy Doom, ZDoom]
- _**Impact Pitch**_ setting, to flinch upon hitting a floor and/or taking damage
- _**Explosion Shake Effect**_ setting [i.b. Doom Retro]
- _**Subtle Idle Bobbing/Breathing**_ setting [p.f. International Doom]
- _**Teleporter Zoom**_ setting [i.b. ZDoom]
- _**Death Camera**_ setting [i.b. ZDoom]
- _**Chasecam**_ [i.b. ZDoom]
- _**Disable palette tint in menus**_ setting [i.b. Crispy Doom]
- _**Disable Berserk Tint**_ setting
- _**Disable Radiation Suit Tint**_ setting
- _**Damage Tint Cap**_ and _**Bonus Tint Cap**_, to attenuate or disable the red and yellow screen tints
- _**Fake Contrast**_ setting
- _**Screen Wipe speed percentage**_ setting
- _**Sound Clipping Distance**_ selection, to optionally double the distance at which SFX become audible
- **_Organize Saves by IWAD_** setting
- _**One-Key Quick Save/Load**_ setting, to skip the confirmation prompt
- _**Play Internal Demos**_ setting
- _**Quick "Quit Game"**_ setting, to skip the confirmation prompt [p.f. Crispy Doom]
- Toggling of _**Weapon Flash Lighting**_ [p.f. Crispy Doom]
- Toggling of _**Weapon Flash Sprite**_ [p.f. Crispy Doom]
- Toggling of _**Invulnerability Colormap**_ [p.f. Crispy Doom]

### Weapons

- Restored _**Weapon Recoil**_ menu item
- _**Disable Horizontal Autoaim**_ setting
- _**Switch [Weapon] on Pickup**_ setting
- Key to **equip last used weapon** [i.b. Cherry Doom]
- **_Horizontal_ Weapon Centering** setting [i.b. DSDA-Doom]
- **Always Bob** setting (CFG-Only: `always_bob`)
- _**Weapon Bobbing Percentage**_ setting [i.b. Crispy Doom, ZDoom]
- _**Bobbing Styles**_ selection [p.f. Zandronum]
- _**Weapon Inertia**_ setting (scale determined by the CFG-Only `weapon_inertia_scale_pct` CVAR)
- _**Squat Weapon Down On Impact**_ setting [p.f. Crispy Doom]
- _**Translucent Flashes**_ setting [i.b. Crispy Doom]
- **Show Berserk availability** setting [partially p.f. Crispy Doom]
- **"Correct" first person sprite centering** setting, to toggle the 1px first person sprite misalignment (CFG-Only: `sx_fix`)

### Status Bar/HUD

- **`NUGHUD` lump support**, making the Crispy HUD (now called Nugget HUD) customizable (see `docs/nughud.md`)
- **SSG is accounted for on the Arms widget** [p.f. Crispy Doom]
- **Blink Missing Keys** setting (CFG-Only: `blink_keys`) [partially p.f. Crispy Doom]
- **Show Powerup Timers** setting
- _**Alternative Arms Display**_ setting, to show the Chainsaw or SSG's availability on the Arms widget in place of the trivial Pistol (CFG-Only: `alt_arms`)
- _**Smart Totals**_ setting [p.f. So Doom]
- _**(In)Complete Milestone Color**_ choices, to customize the Stats text's colors
- **Armor count is colored gray when in God Mode**
- **Support for Berserk (`STBERSRK`) and Infinite Ammo (`STINFNTY`) icons**
- **Event Timers:**
  - _"Use" Button Timer_ [p.f. Crispy Doom]; 
  - _Teleport Timer_ [i.b. the above];
  - _Key Pickup Timer_ [same as above].
- **Crosshair:**
  - Vertical-only target lock-on;
  - Horizontal autoaim indicators;
  - Option to account for fuzzy targets [i.b. From Doom With Love];
  - Dedicated toggle key.

### Automap

- **Automap color for unrevealed secret sectors**
- Key to _**Blink [Automap] Marks**_ (default: <kbd>B</kbd>)
- _**Tag Finder**_: Position the Automap pointer over a sector and press the _Tag Finder_ key to highlight its activator line(s), and vice versa [p.f. PrBoomX]
- Key to _**Teleport to Automap pointer**_

### Enemies

- _**Extra Gibbing**_ setting, to force Berserk Fist/Chainsaw/SSG gibbing (configurable through the CFG-Only `extra_gibbing_#` CVARs) [i.b. Smooth Doom]
- _**Bloodier Gibbing**_ setting [i.b. Smooth Doom]
- _**ZDoom-like item drops**_ setting [of course, i.b. ZDoom]
- **Improved vanilla fuzz effect**
- _**Selective Fuzz Darkening**_ setting

### Messages

- **Secret count in "secret revealed" message** [p.f. Crispy Doom]
- _**Announce Milestone Completion**_ setting, to report completion of milestones (e.g. all items acquired)
- Restored _**Message Listing Scrolls Upwards**_ setting, and enabled it by default

### Doom Compatibility

- Bruiser attack (A_BruisAttack) doesn't face target
- Double Autoaim range
- Fix Lost Soul colliding with items
- Lost Soul forgets target upon impact
- Fuzzy things bleed fuzzy blood [i.b. Crispy Doom]
- Non-bleeders don't bleed when crushed [i.b. Crispy Doom]
- Fix lopsided Icon of Sin explosions
- Permanent IDCHOPPERS invulnerability
- Blazing doors reopen with wrong sound [p.f. Crispy Doom]
- Manually reactivated moving doors are silent [p.f. Crispy Doom]
- Corrected switch sound source [p.f. Crispy Doom]
- Chaingun makes two sounds with one bullet
- Chaingunner uses pistol/chaingun sound
- Arch-Vile fire plays flame start sound [p.f. Crispy Doom]
- Dead players can still play "oof" sound [p.f. Crispy Doom]
- Key pickup resets palette

### Cheats

- _**'FULLCLIP'**_ for infinite ammo
- _**'VALIANT'**_ for fast weapons [i.b. ZDoom]
  - _**'BOBBERS'**_ serves as a shortcut to toggle the two cheats mentioned above, plus IDFA
- _**'GIBBERS'**_ to force gibbing on dying enemies, independently of damage dealt
- _**'IDFLY'**_ to fly (uses jumping/crouching keys) [i.b. PrBoom+, ZDoom]
- _**'SUMMON'**_ to spawn a hostile or friendly actor based on its mobjtype index [i.b. ZDoom]
- _**'RESURRECT' / 'IDRES'**_ to resurrect the player without toggling IDDQD [i.b. ZDoom]
- _**'LINETARGET'**_ to give some info on the player's linetarget [i.b. ZDoom]
- _**'MDK'**_ [i.b. ZDoom]
- _**'SAITAMA'**_ to enable the MDK Fist (replaces A_Punch's melee attack with the MDK attack, featuring an alternate multishot attack when holding down Strafe On)
- _**'BOOMCAN'**_ for explosive hitscan attacks
- _**'NEXTMAP'**_ to exit the level [i.b. ZDoom]
- _**'NEXTSECRET'**_ to exit the level as if using a secret exit [i.b. ZDoom]
- _**'TURBO'**_ to change the player speed in-game
- _**'TNTEM'**_ as an alternative to _'KILLEM'_
- _**'FPS'**_ as a replacement for _'SHOWFPS'_
- **Mid-air control while in noclipping mode** [p.f. Crispy Doom]
- Reenabled _**'NOMOMENTUM'**_ cheat [p.f. Crispy Doom]

### Miscellaneous

- Key to **toggle zoom**
- **Autoload folder for all games** (`autoload/all`)
- **Setting of savegame and screenshot paths in config file** (CFG-Only: `savegame_dir` and `screenshot_dir`)
- **Keep palette changes in screenshots** setting (CFG-Only: `screenshot_palette`)
- **Allowed mouselook while dead**
- **Interactive character cast** (Turn keys to rotate enemy, Run key to gib, Strafe keys to skip) [p.f. Crispy Doom]
- **Support for optional sounds:** [p.f. Crispy Doom]
  - `DSPLJUMP`, `DSPLLAND`;
  - `DSLOCKED`, `DSKEYUP `, `DSKEYBNK`;
  - `DSMNUOPN`, `DSMNUCLS`, `DSMNUACT`, `DSMNUBAK`, `DSMNUMOV`, `DSMNUSLI`, `DSMNUERR`;
  - `DSINTTIC`, `DSINTTOT`, `DSINTNEX`, `DSINTNET`, `DSINTDMS`.
- **Customizable darkening level for dark menu background and Automap overlay** (CFG-Only: `menu_background_darkening` and `automap_overlay_darkening`) [i.b. Cherry Doom]
- The **Chaingun can be given a custom sound effect** by providing a `DSCHGUN` sound effect lump
- Toggle to **allow chat in singleplayer** (CFG-Only: `sp_chat`)
- Restored `-cdrom` command-line parameter

# Releases

Source code and Windows binaries (MSVC builds for Windows 7 and newer) for the latest release can be found on the [Release](https://github.com/MrAlaux/Nugget-Doom/releases/latest) page.

The most recent list of changes can be found in the [Changelog](https://github.com/MrAlaux/Nugget-Doom/blob/master/CHANGELOG.md).

A complete history of changes and releases can be found on the [Releases](https://github.com/MrAlaux/Nugget-Doom/releases) page.

## Versioning

Nugget Doom follows a fairly simple (albeit arbitrary) **X.Y.Z** versioning system:

- **X** is increased in the event of at least one major implementation, such as a new spec like _UMAPINFO_;
- **Y** is increased in the event of at least one minor implementation, such as a new cheat;
- **Z** is increased in the event of at least one bug fix, text/code reformatting, or merging of _Woof!_ updates, even if the changes to the latter are considered minor or major.

Incrementing any of the first values will reset the latter (i.e. a major change to 1.1.2 would shift it to 2.0.0).

# Compiling

As a Woof! fork, its build instructions should also apply here:

The Nugget Doom source code is available at GitHub: <https://github.com/MrAlaux/Nugget-Doom>.

## Linux, and Windows with MSYS2

The following libraries need to be installed:

 * [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2) (>= 2.0.18)
 * [SDL2_net](https://github.com/libsdl-org/SDL_net)
 * [openal-soft](https://github.com/kcat/openal-soft) (>= 1.22.0 for PC Speaker emulation)
 * [libsndfile](https://github.com/libsndfile/libsndfile) (>= 1.1.0 for MPEG support)
 * [fluidsynth](https://github.com/FluidSynth/fluidsynth) (>= 2.2.0, optional)
 * [libxmp](https://github.com/libxmp/libxmp) (optional)
 
Usually your distribution should have the corresponding packages in its repositories, and if your distribution has "dev" versions of those libraries, those are the ones you'll need.

Once installed, compilation should be as simple as:

```
 cd nugget-doom
 mkdir build; cd build
 cmake ..
 make
```

After successful compilation the resulting binary can be found in the `src/` directory.

## Windows with Visual Studio

Visual Studio 2019 and [VSCode](https://code.visualstudio.com/) comes with built-in support for CMake by opening the source tree as a folder.

Install vcpkg <https://github.com/Microsoft/vcpkg#quick-start-windows>. Integrate it into CMake or use toolchain file:
```
 cd nugget-doom
 cmake -B build -DCMAKE_TOOLCHAIN_FILE="[path to vcpkg]/scripts/buildsystems/vcpkg.cmake"
 cmake --build build
```
CMake will automatically download and build all dependencies for you.

# Contact

The homepage for Nugget Doom is <https://github.com/MrAlaux/Nugget-Doom>.

Please report any bugs, glitches or crashes that you encounter to the GitHub [Issue Tracker](https://github.com/MrAlaux/Nugget-Doom/issues).

## Acknowledgement

Help was provided by:

- [_atsb_ (a.k.a. _Gibbon_)](https://github.com/atsb);
- [_Brad Harding_](https://github.com/bradharding);
- [_ceski_](https://github.com/ceski-1);
- [_melak47_](https://github.com/melak47);
- [_Fabian Greffrath_](https://github.com/fabiangreffrath);
- [_Roman Fomin_ (a.k.a. _rfomin_)](https://github.com/rfomin);
- [_Ryan Krafnick_ (a.k.a. _kraflab_)](https://github.com/kraflab).

Thanks to all of you!

# Legalese

Files: `*`  
Copyright:  
 © 1993-1996 Id Software, Inc.;  
 © 1993-2008 Raven Software;  
 © 1999 by id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman;  
 © 2004 James Haley;  
 © 2005-2018 Simon Howard;  
 © 2006 Ben Ryves;  
 © 2017 Christoph Oelckers;  
 © 2019 Fernando Carmona Varo;  
 © 2019 Jonathan Dowland;  
 © 2020 Alex Mayfield;  
 © 2021 Ryan Krafnick;  
 © 2022-2023 ceski;  
 © 2023 liPillON;  
 © 2020-2023 Fabian Greffrath;  
 © 2020-2023 Roman Fomin;  
 © 2021-2023 Alaux.  
License: [GPL-2.0+](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)

Files: `src/beta.h`  
Copyright: © 2001-2019 Contributors to the Freedoom project.  
License: [BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)

Files: `src/dogs.h`  
Copyright:  
 © 2017 Nash Muhandes;  
 © apolloaiello;  
 © TobiasKosmos.  
License: [CC-BY-3.0](https://creativecommons.org/licenses/by/3.0/) and [CC0-1.0](https://creativecommons.org/publicdomain/zero/1.0/)

Files: `src/u_scanner.*`  
Copyright:  
 © 2010 Braden "Blzut3" Obrzut;  
 © 2019 Fernando Carmona Varo.  
License: [BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)

Files: `cmake/FindSDL2.cmake, cmake/FindSDL2_net.cmake`  
Copyright: © 2018 Alex Mayfield.  
License: [BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)

Files: `data/nugget-doom.ico, data/nugget-doom.png, data/nugget-doom8.ico, src/icon.c, data/setup.ico, data/setup8.ico, setup/setup_icon.c, data/nugget-doom-setup.png`  
Copyright: © 2022 Korp.  
License: [CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/)

Files: `src/thermo.h`  
Copyright: © 2020-2022 Julia Nechaevskaya.  
License: [CC-BY-3.0](https://creativecommons.org/licenses/by/3.0/)

Files: `miniz/*`  
Copyright:  
 © 2010-2014 Rich Geldreich and Tenacious Software LLC;  
 © 2013-2014 RAD Game Tools and Valve Software.  
License: [MIT](https://opensource.org/licenses/MIT)

Files: `opl/*`  
Copyright:  
 © 2005-2014 Simon Howard;  
 © 2013-2018 Alexey Khokholov (Nuke.YKT).  
License: [GPL-2.0+](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)

Files: `soundfonts/TimGM6mb.sf2`  
Copyright:  
 © 2004 Tim Brechbill;  
 © 2010 David Bolton.  
License: [GPL-2.0](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)

Files: `textscreen/*`  
Copyright:  
 © 1993-1996 Id Software, Inc.;  
 © 2002-2004 The DOSBox Team;  
 © 2005-2017 Simon Howard.  
License: [GPL-2.0+](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)

Files: `win32/win_opendir.*`  
License: public-domain
