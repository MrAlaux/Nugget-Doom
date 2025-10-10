# Nugget Doom

[![Nugget Doom Icon](https://raw.githubusercontent.com/MrAlaux/Nugget-Doom/master/data/nugget-doom.png)](https://github.com/MrAlaux/Nugget-Doom)

[![Release](https://img.shields.io/github/release/MrAlaux/Nugget-Doom.svg)](https://github.com/MrAlaux/Nugget-Doom/releases/latest)
[![Release Date](https://img.shields.io/github/release-date/MrAlaux/Nugget-Doom.svg)](https://github.com/MrAlaux/Nugget-Doom/releases/latest)
[![Downloads (total)](https://img.shields.io/github/downloads/MrAlaux/Nugget-Doom/total)](https://github.com/MrAlaux/Nugget-Doom/releases)
[![Downloads (latest)](https://img.shields.io/github/downloads/MrAlaux/Nugget-Doom/latest/total.svg)](https://github.com/MrAlaux/Nugget-Doom/releases/latest)

Nugget Doom is a source port of Doom forked from [Woof!](https://github.com/fabiangreffrath/woof), simply intended to implement additional features.

**Note:** this README and the rest of documentation are updated on a per-commit basis,
meaning that they may not correspond to the latest release of Nugget Doom.
If you're seeking information on the version you're using, please refer to the documentation included with it.

### DISCLAIMER
Although the new code has been written with the intention of not breaking demo compatibility, it has not been properly tested yet.
**RECORD DEMOS AT YOUR OWN RISK!**

## Features

Note that this feature list is relative to [Woof!'s](https://github.com/fabiangreffrath/woof/blob/master/README.md#key-features);
read the latter for more details.
Some features were first implemented in Nugget Doom and later in Woof!, so they're not listed here anymore.

The build corresponding to this README is based on [Woof! 15.2.0](https://github.com/fabiangreffrath/woof/releases/tag/woof_15.2.0).

Most of Nugget Doom's features come from other sources, like source ports and mods;
the initial implementations for some are **ported from (p.f.)** said sources, while others are just **inspired by (i.b.)** them.
These acknowledgements are included in the feature lists below; be aware that some might be inaccurate or outright missing.

A few settings are labeled as **_CFG-only_**: they can only be toggled by editing `nugget-doom.cfg`.
For these settings, their CVAR names are provided alongside the _CFG-only_ label as guidance.

### General

- **Extended FOV range:** [20, 140]
- **Stretch viewport to fit window** setting (CFG-only: `stretch_to_fit`) [i.b. and partially p.f. Crispy Doom; i.b. ZDoom]
- **Set _Air Absorption_ and _Doppler Effect_ to 5 by default**
- **_Bounded Voxel Rendering_** setting, to draw each voxel as a rectangular sprite
- **FOV-based sky stretching** setting (CFG-only: `fov_stretchsky`)
- **Tweaked _Stretch Short Skies_ algorithm**
- **_Black Fade_ screen wipe**
- **Extended _Level Brightness_ range:** [-8, 8]
- **Support for SSG in Doom 1** [p.f. Woof! 15.2.0]
- **_Hitbox-based Hitscan Collision_** setting
- **_"Direct + Auto"_ mode for _Free Look_**
- **Direct vertical aiming for melee attacks**
- **_Move Over/Under Things_** setting [partially p.f. Crispy Doom, DSDA-Doom]
- **Jumping** (default key: <kbd>Alt</kbd>) [p.f. Crispy Doom]
- **Crouching/ducking** (default key: <kbd>C</kbd>) [i.b. ZDoom]
  - Includes support for crouching-player sprites, named `PLYC`; must be provided by the user
- **_View Height_** setting, which allows to enter a custom POV height value in the [32, 56] range [i.b. Brutal Doom]
- **_Vertical Target Lock-on_** setting, to make the camera automatically lock onto targets vertically [i.b. Rise of the Triad]
- **Flinching** setting, to flinch upon landing and/or taking damage
- **_Explosion Shake Effect_** setting [i.b. Doom Retro]
  - Shake intensity determined by the CFG-only `explosion_shake_intensity_pct` CVAR
- **_Subtle Idle Bobbing/Breathing_** setting [p.f. International Doom]
- **_Teleporter Zoom_** setting [i.b. ZDoom]
- **_Death Camera_** setting [i.b. ZDoom]
- **_Freecam_**, which repurposes the following inputs:
  - _Cycle Chasecam_ to toggle control between the camera and the player
  - _Use_ to reset the camera to the player's POV
  - _Fire_ to lock onto and let go of a sentient thing's POV
  - _Previous/Next Weapon_ to adjust the camera's movement speed
  - Rapid _Strafe On_ double-press to center the camera vertically
- **_Chasecam_** [i.b. ZDoom]
- **Slow Motion** button
- **Zoom** button
- **_Backdrop for all menus_** setting
- **_No Palette Tint in Menus_** setting [i.b. Crispy Doom]
- **_HUD/Menu Shadows_** setting [i.b. CRL]
  - Opacity level determined by the CFG-only `hud_menu_shadows_filter_pct` CVAR
- **_Sprite Shadows_** setting [i.b. Doom Retro]
  - Opacity level determined by the CFG-only `sprite_shadows_tran_pct` CVAR
- **_Thing Lighting Mode_** setting
  - _Hitbox_ suggested by [@fragglet](https://github.com/fragglet)
  - _Per-column_ inspired by PSX Hexen
- **_Flip Levels_** setting
- **Low-resolution pixel width/height** settings, to enlarge pixels when using 100% resolution (CFG-only: `lowres_pixel_width`, `lowres_pixel_height`) [i.b. Doom Retro]
- **_No Berserk Tint_** setting
- **_No Radiation Suit Tint_** setting
- **_Night-Vision Visor Effect_** setting [i.b. International Doom]
- **_Damage Tint Cap_** and **_Bonus Tint Cap_**, to attenuate or disable said screen tinting
- **_Fake Contrast_** setting
- Toggle for **diminishing lighting** (CFG-only: `diminishing_lighting`)
  - _Diminishing lighting_ refers to geometry and entities becoming brighter as they come closer to the camera
- **_Screen Wipe speed percentage_** setting
- **_Alternative Intermission Background_** setting, to replace the intermission graphic with a darkened rotating camera view
- **Intermission ratio stats** setting, to use ratios for the stats on the intermission screen (CFG-only: `inter_ratio_stats`) [i.b. Heretic]
- **Color settings** [p.f. International Doom]
  - _Contrast_ by [@pvictress](https://github.com/pvictress)
- **_Sound Clipping Distance_** selection, to optionally double the distance at which sound effects become audible
- **_One-Key Quick Save/Load_** setting, to skip the confirmation prompt
- **_Auto Save Interval_** setting, for periodic auto saves
- Toggle for **_Weapon Flash Lighting_** [p.f. Crispy Doom]
- Toggle for **_Weapon Flash Sprite_** [p.f. Crispy Doom]
- Toggle for **_Invulnerability Colormap_** [p.f. Crispy Doom]

### Weapons

- **Extended _View/Weapon Bob_ percentages**
- **Recoil-pitch scale percent** setting
- **_Smart Autoaim_** setting
- **_No Horizontal Autoaim_** setting
- **_Switch [Weapon] on Pickup_** setting
- **_Improved Weapon Toggles_** setting
  - Normally, when using the weapon toggles (that is, the same key for both the fist and chainsaw or shotgun and SSG),
    one must wait until the first weapon in the toggle is equipped before being able to select the other weapon in it;
    this setting changes that, allowing to select either weapon in the toggle without waiting
- **_Allow [Weapon] Switch Interruption_** setting
- **_Prev/Next Skip Ammoless Weapons_** setting, to make the previous/next-weapon buttons skip weapons with insufficient ammo
- **_Horizontal_ Weapon Centering** setting [i.b. DSDA-Doom]
- **Always Bob** setting (CFG-only: `always_bob`)
  - This setting forces the weapon to bob in every tic of its ready state,
    whether or not the `A_WeaponReady` action is called in said tic,
    fixing choppy bobbing for weapons like the chainsaw
- **_Bobbing Styles_** selection [p.f. Zandronum]
- **_Weapon Bob Speed_** setting
- **_Bob While Switching_** setting
- **_Weapon Inertia_** settings [by _ceski_]
- **_Weapon Squat Upon Landing_** setting [p.f. Crispy Doom]
- **_[Weapon] Flash Opacity_** setting [i.b. Crispy Doom]
- **Support for weapon voxel models**
- **"Correct" first-person-sprite centering** setting (CFG-only: `sx_fix`)
  - In vanilla Doom, weapon sprites are centered with an offset of one unit to the right;
    this setting changes that

### Status Bar/HUD

- **NUGHUD**, an alternative lump for HUD customization (see `docs/nughud.md`)
- **SBARDEF:**
  - Chat hack to move it vertically based on the height of the message list
- **Crosshair:**
  - Opacity setting
  - Vertical-only target lock-on
  - Horizontal-autoaim indicators
  - Option to account for fuzzy targets [i.b. From Doom With Love]
  - Dedicated toggle button
- **_Sound only_ option for "secret revealed" message**
- **Show Save Messages** setting (CFG-only: `show_save_messages`)
- **_Milestone-Completion Announcements_** setting, to report completion of milestones (e.g. all items acquired)
- **_Message Fadeout_** setting
- **_Message Flash_** setting
- **_Message Lines_** settings
  - Scrolling direction determined by the CFG-only `hud_msg_scrollup` CVAR
  - The CFG-only `hud_msg_total_lines` CVAR determines how many messages will be shown during message review;
    if lesser than `hud_msg_lines`, that will be used instead
- **_Group Repeated Messages_** setting [i.b. Doom Retro]
- **_(Chat) Message Duration_** settings
- **_Show Powerup Timers_** setting
  - The CFG-only `hud_power_timers_notime` CVAR can be enabled to show only the powerup names/icons
- **_Blink Missing Keys_** setting [partially p.f. Crispy Doom]
- **_Animated Health/Armor Count_** setting
- **_Berserk display when using Fist_** setting [partially p.f. Crispy Doom]
- **_Automap Level Stats Format_** setting
- **Level-Stats Selection** settings (CFG-only: `hud_stats_#[_map]`)
- **Event Timers:**
  - _Teleport Timer_ [i.b. Crispy Doom];
  - _Key-Pickup Timer_ [same as above].
- **Extended HUD color customization**
- **Armor count is colored gray when in God Mode**
- **Support for Berserk (`STBERSRK`) and Infinite Ammo (`STINFNTY`) icons**
- **Support for HUD icons**:
  - Level-stats widget: `HUDKILLS`, `HUDITEMS`, `HUDSCRTS`
  - Powerup-timers widget: `HUDINVIS`, `HUDINVUL`, `HUDLIGHT`, `HUDSUIT`

### Automap

- **Minimap mode** [i.b. DSDA-Doom]
- Button to **_Highlight Points of Interest_**; marks and keyed lines (default: <kbd>B</kbd>)
- **_Tag Finder_** button [p.f. PrBoomX]
  - Position the automap pointer over a sector and press this button
    to highlight its activator line(s), and vice versa
- Button to **_Teleport to Automap pointer_**
- **_Show Thing Hitboxes_** setting
- **Keys now flash when flashing of keyed doors is enabled**
- **Color for trigger lines**, used with IDDT (CFG-only: `mapcolor_trig`)

### Enemies

- **_Extra Gibbing_** setting, to force gibbing under certain conditions [i.b. Smooth Doom]
  - Configurable through the CFG-only `extra_gibbing_#` CVARs
- **_Bloodier Gibbing_** setting [i.b. Smooth Doom]
  - Number of splats determined by the CFG-only `bloodier_gibbing_splats` CVAR
- **_Toss Items Upon Death_** setting [i.b. ZDoom, Doom Retro]

### Doom Compatibility settings

All of these are CFG-only, so their CVAR names are included.

- Bruiser attack doesn't face target (`comp_bruistarget`)
- Disable melee snapping (`comp_nomeleesnap`)
- Double autoaim range (`comp_longautoaim`)
- Fix Lost Soul colliding with items (`comp_lscollision`)
- Lost Soul forgets target upon impact (`comp_lsamnesia`)
- Fuzzy things bleed fuzzy blood (`comp_fuzzyblood`) [i.b. Crispy Doom]
- Non-bleeders don't bleed when crushed (`comp_nonbleeders`) [i.b. Crispy Doom]
- Attackers face fuzzy targets straight (`comp_faceshadow`)
- Fix lopsided Icon of Sin explosions (`comp_iosdeath`)
- Permanent IDCHOPPERS invulnerability (`comp_choppers`)
- Manually toggled moving doors are silent (`comp_manualdoor`) [p.f. Crispy Doom]
- Corrected switch sound source (`comp_switchsource`) [p.f. Crispy Doom]
- Chaingun makes two sounds with one bullet (`comp_cgundblsnd`)
- Chaingunner uses pistol/chaingun sound (`comp_cgunnersfx`)
- Arch-Vile fire plays flame start sound (`comp_flamst`) [p.f. Crispy Doom]
- Play `DSNOWAY` instead of `DSOOF` when failing to use key-locked triggers (`comp_keynoway`)
- Higher god-mode face priority (`comp_godface`) [p.f. International Doom]
- Dead players can still play "oof" sound (`comp_deadoof`) [p.f. Crispy Doom]
- Use improved powerup run-out effect (`comp_powerrunout`)
- Use unused pain/bonus palettes (`comp_unusedpals`)
- Key pickup resets palette (`comp_keypal`)

### Cheats

- **`FULLCLIP`** for infinite ammo
- **`VALIANT`** for fast weapons [i.b. ZDoom]
  - **`BOBBERS`** serves as a shortcut to toggle the two above cheats, and `IDFA`
- **`GIBBERS`** to force gibbing on dying enemies, independently of damage dealt
- **`RIOTMODE`** cheat, to make enemies attack all sentient entities
- **`IDFLY`** to fly (uses jumping/crouching buttons) [i.b. PrBoom+, ZDoom]
- **`SUMMON`** to spawn an actor based on its type index [i.b. ZDoom, PrBoomX]
- **`IDDF`** to find a key in the automap
- **`IDDET`** to find exits in the automap
- **`RESURRECT / IDRES`** to resurrect the player without toggling `IDDQD` [i.b. ZDoom]
- **`LINETARGET`** to give some info on the player's linetarget [i.b. ZDoom]
- **`TRAILS`** to show hitscan trails
- **`MDK`** to perform a hitscan attack of 1-million damage [i.b. ZDoom]
- **`SAITAMA`** to enable the MDK Fist
- **`BOOMCAN`** for explosive hitscan attacks
- **`NEXTMAP`** to exit the level [i.b. ZDoom]
- **`NEXTSECRET`** to exit the level as if using a secret exit [i.b. ZDoom]
- **`TURBO`** to change the player speed in-game
- **`TNTEM`** as an alternative to `KILLEM`
- **`FPS`** as a replacement for `SHOWFPS`
- **Mid-air control while in noclipping mode** [p.f. Crispy Doom]
- **Key-binding for Computer Area Map cheat**
- Reenabled **`NOMOMENTUM`** cheat [p.f. Crispy Doom]

For a complete list with more details, see the _New Nugget Doom cheats_ section of `docs/cheats.md`.

### Miscellaneous

- **Reworked custom skill**:
  - Considered skill #6, settable through the `SKILL` cheat
  - _Thing Spawns_ setting
  - _Duplicate Monsters_ setting
  - _Slow Spawn-Cube Spitter_ setting
  - _Restart [Current] Level_ options
  - Settings are saved in the config file
  - Its menu item uses the `M_CSTSKL` graphic if found
- **SDL render driver** setting (CFG-only: `sdl_renderdriver`) [p.f. Woof! 14.0.0]
- **Setting of savegame and screenshot paths in config file** (CFG-only: `savegame_dir` and `screenshot_dir`)
- **Keep palette changes in screenshots** setting (CFG-only: `screenshot_palette`)
- **When dying with _Free Look_ enabled, the camera is pitched towards the killer**
- **Extended character cast** [partially p.f. Crispy Doom]
  - _Turn_ buttons to rotate
  - _Run_ button to gib
  - _Strafe_ buttons to skip
  - Press `0` to toggle fancy mode
- **Support for optional sounds:** [partially p.f. Crispy Doom]
  - Jumping: `DSPLJUMP`
  - Landing: `DSPLLAND`
  - Key-locked door: `DSLOCKED`
  - Key pickup: `DSKEYUP`
  - Key blinking on HUD: `DSKEYBNK`
  - Menus: `DSMNUOPN`, `DSMNUCLS`, `DSMNUACT`, `DSMNUBAK`, `DSMNUMOV`, `DSMNUSLI`, `DSMNUERR`
  - Intermission: `DSINTTIC`, `DSINTTOT`, `DSINTNEX`, `DSINTNET`, `DSINTDMS` (last three unused)
  - Health-based player-pain sounds: `DSPPAI25`, `DSPPAI50`, `DSPPAI75`, `DSPPA100` [i.b. ZDoom]
  - Slow motion: `DSNGSLON`, `DSNGSLOF`
- **Customizable darkening level for dark menu background and Automap overlay** (CFG-only: `menu_background_darkening` and `automap_overlay_darkening`) [i.b. Cherry Doom]
- The **Chaingun can be given a custom sound effect** by providing a `DSCHGUN` sound effect lump
- Toggle to **disable the Killough-face easter egg** (CFG-only: `no_killough_face`)
- Toggle to **allow chat in singleplayer** (CFG-only: `sp_chat`)
- Made `force_flip_pan` affect the _OpenAL 3D_ sound module [by _ceski_]
- Restored menu items for various Woof! settings
- Restored `-cdrom` command-line parameter
- Decreased minimum window size to 200p (240p with aspect-ratio correction)
- Setting to enable developer features (CFG-only: `nugget_devmode`)

# Releases

Source code and Windows binaries (MSVC builds for Windows 7 and newer) for the latest release can be found on the [Release](https://github.com/MrAlaux/Nugget-Doom/releases/latest) page.

The most recent list of changes can be found in the [Changelog](https://github.com/MrAlaux/Nugget-Doom/blob/master/CHANGELOG.md).

A complete history of changes and releases can be found on the [Releases](https://github.com/MrAlaux/Nugget-Doom/releases) page.

## Versioning

Nugget Doom follows a fairly simple (albeit arbitrary) **X.Y.Z** versioning system:

- **X** is increased in the event of major implementations, as were arbitrary/dynamic resolution and voxel support;
- **Y** is increased in the event of minor implementations, such as a new cheat;
- **Z** is increased in the event of bug fixes or text/code reformatting.

Incrementing any of the first values will reset the latter (i.e. a major change to 1.1.2 would shift it to 2.0.0).

The merging of changes from Woof! releases may affect any of the version values, but not necessarily in the same way as Woof!'s own version (i.e. `Woof! 11.Y.Z -> 12.Y.Z` doesn't necessarily mean `Nugget 2.Y.Z -> 3.Y.Z`).

# Compiling

As a Woof! fork, its build instructions should also apply here:

The Nugget Doom source code is available at GitHub: <https://github.com/MrAlaux/Nugget-Doom>.

## Linux, and Windows with MSYS2

The following build system and libraries need to be installed:
 
 * [CMake](https://cmake.org) (>= 3.15)
 * [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2) (>= 2.0.18)
 * [openal-soft](https://github.com/kcat/openal-soft) (>= 1.22.0 for PC Speaker emulation)
 * [libsndfile](https://github.com/libsndfile/libsndfile) (>= 1.1.0 for MPEG support)
 * [libebur128](https://github.com/jiixyj/libebur128) (>= 1.2.0)
 * [yyjson](https://github.com/ibireme/yyjson) (>= 0.10.0, optional)
 * [fluidsynth](https://github.com/FluidSynth/fluidsynth) (>= 2.2.0, optional)
 * [libxmp](https://github.com/libxmp/libxmp) (optional)
 * [discord-rpc](https://github.com/discord/discord-rpc) (optional)
 
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

Install vcpkg <https://github.com/Microsoft/vcpkg?tab=readme-ov-file#get-started>. 

Run the CMake configuration:
```
 cd nugget-doom
 cmake -B build -DCMAKE_TOOLCHAIN_FILE="[path to vcpkg]/scripts/buildsystems/vcpkg.cmake"
```
During this step, vcpkg will build all the dependencies.

Build the project:
```
 cmake --build build
```

# Contact

The homepage for Nugget Doom is <https://github.com/MrAlaux/Nugget-Doom>.

Please report any bugs, glitches or crashes that you encounter to the GitHub [Issue Tracker](https://github.com/MrAlaux/Nugget-Doom/issues).

## Acknowledgements

Help was provided by:

- [_atsb_ (a.k.a. _Gibbon_)](https://github.com/atsb)
- _Ayba_
- [_Brad Harding_](https://github.com/bradharding)
- [_ceski_](https://github.com/ceski-1)
- [_Fabian Greffrath_](https://github.com/fabiangreffrath)
- [_melak47_](https://github.com/melak47)
- [_Mikolah_](https://github.com/mykola-ambar)
- [_Roman Fomin_ (a.k.a. _rfomin_)](https://github.com/rfomin)
- [_Ryan Krafnick_ (a.k.a. _kraflab_)](https://github.com/kraflab)

Thanks to all of you!

# Legalese

Files: `*`  
Copyright:  
 © 1993-1996 Id Software, Inc.;  
 © 1993-2008 Raven Software;  
 © 1999 by id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman;  
 © 1999-2004 by Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze;  
 © 2004 James Haley;  
 © 2005-2006 by Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko;  
 © 2005-2018 Simon Howard;  
 © 2006 Ben Ryves;  
 © 2006-2025 by The Odamex Team;  
 © 2007-2011 Moritz "Ripper" Kroll;  
 © 2008-2019 Simon Judd;  
 © 2013-2025 Brad Harding;  
 © 2017 Christoph Oelckers;  
 © 2020 Alex Mayfield;  
 © 2020 Ethan Watson;  
 © 2020 JadingTsunami;  
 © 2020-2024 Fabian Greffrath;  
 © 2020-2024 Roman Fomin;  
 © 2021-2022 Ryan Krafnick;  
 © 2021-2025 Alaux;  
 © 2022 Julia Nechaevskaya;  
 © 2022-2024 ceski;  
 © 2023 Andrew Apted;  
 © 2023 liPillON;  
 © 2024 pvictress;  
 © 2025 Guilherme Miranda.  
License: [GPL-2.0+](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)

Files: `src/i_flickstick.*, src/i_gyro.*`  
Copyright:  
 © 2018-2021 Julian "Jibb" Smart;  
 © 2021-2024 Nicolas Lessard;  
 © 2024 ceski.  
License: [MIT](https://opensource.org/licenses/MIT)

Files: `src/nano_bsp.*`  
Copyright:  
 © 2023 Andrew Apted.  
License: [MIT](https://opensource.org/licenses/MIT)

Files: `src/m_scanner.*`  
Copyright:  
 © 2015 Braden "Blzut3" Obrzut.  
License: [BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)

Files: `src/v_flextran.*`  
Copyright:  
 © 2013 James Haley et al.;  
 © 1998-2012 Marisa Heit.  
License: [GPL-3.0+](https://www.gnu.org/licenses/gpl-3.0)

Files: `src/v_video.*`  
Copyright:  
 © 1999 by id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman;  
 © 2013 James Haley et al.  
License: [GPL-3.0+](https://www.gnu.org/licenses/gpl-3.0)

Files: `base/all-all/sprites/pls*, man/simplecpp`  
Copyright:  
 © 2001-2019 Contributors to the Freedoom project.  
License: [BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)

Files: `base/all-all/sprites/ngcha0.png, base/all-all/sprites/ngcla0.png, base/all-all/sprites/ngtr*0.png`  
Copyright:  
 © 2023-2024 Korp.  
License: [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/)

Files: `base/all-all/dsdg*, base/all-all/sprites/dog*`  
Copyright:  
 © 2017 Nash Muhandes;  
 © apolloaiello;  
 © TobiasKosmos.  
License: [CC-BY-3.0](https://creativecommons.org/licenses/by/3.0/) and [CC0-1.0](https://creativecommons.org/publicdomain/zero/1.0/)

Files: `base/all-all/sm*.png`  
Copyright:  
 © 2024 Julia Nechaevskaya.  
License: [CC-BY-3.0](https://creativecommons.org/licenses/by/3.0/)

Files: `base/all-all/sbardef.lmp`  
Copyright:  
 © 2024 Ethan Watson.  
License: [CC0-1.0](https://creativecommons.org/publicdomain/zero/1.0/)

Files: `base/all-all/dmxopl.op2`  
Copyright:  
 © 2017 Shannon Freeman.  
License: [MIT](https://github.com/sneakernets/DMXOPL/blob/DMXOPL3/LICENSE)

Files: `data/nugget-doom.ico, data/nugget-doom.png, src/icon.c, data/setup.ico, data/nugget-doom-setup.png, setup/setup_icon.c`  
Copyright:  
 © 2022 Korp.  
License: [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/)

Files: `data/io.github.MrAlaux.Nugget-Doom.metainfo.*`  
Copyright:  
 © 2023-2024 Fabian Greffrath.  
License: [CC0-1.0](https://creativecommons.org/publicdomain/zero/1.0/)

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

Files: `netlib/*`  
Copyright:  
 © 1997-2025 Sam Lantinga;  
 © 2012 Simeon Maxein.    
License: [zlib](https://opensource.org/license/zlib)

Files: `third-party/md5/*`  
License: public-domain

Files: `third-party/minimp3/*`  
Copyright:  
 © 2021 lief.      
License: [CC0-1.0](https://creativecommons.org/publicdomain/zero/1.0/)

Files: `third-party/miniz/*`  
Copyright:  
 © 2010-2014 Rich Geldreich and Tenacious Software LLC;  
 © 2013-2014 RAD Game Tools and Valve Software.  
License: [MIT](https://opensource.org/licenses/MIT)

Files: `third-party/pffft/*`  
Copyright:  
 © 2004 The University Corporation for Atmospheric Research ("UCAR");  
 © 2013 Julien Pommier.  
License: [FFTPACK License](https://bitbucket.org/jpommier/pffft/src/master/pffft.h)

Files: `third-party/sha1/*`  
Copyright:  
 © 1998-2001 Free Software Foundation, Inc.;  
 © 2005-2014 Simon Howard.  
License: [GPL-2.0+](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)

Files: `third-party/spng/*`  
Copyright:  
 © 2018-2023 Randy.  
License: [BSD-2-Clause](https://opensource.org/license/bsd-2-clause)

Files: `third-party/yyjson/*`  
Copyright:  
 © 2020 YaoYuan.  
License: [MIT](https://opensource.org/licenses/MIT)
