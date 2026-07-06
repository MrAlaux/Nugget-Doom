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

The build corresponding to this documentation is based on [Woof! 15.3.0](https://github.com/fabiangreffrath/woof/releases/tag/woof_15.3.0).

### DISCLAIMER

Although the new code has been written with the intention of not breaking demo compatibility, it has not been thoroughly tested yet.
**RECORD DEMOS AT YOUR OWN RISK!**

## Main Features

- **True-color Lighting**
- **Rewinding**
- **Customizable skill level**
- **_Extra Gibbing_** setting, to force gibbing under certain conditions
- **_Bloodier Gibbing_** setting
- **Support for high-resolution sprites between `HI_START`/`HI_END` markers**
- **_Smart Autoaim_** setting
- **_Move Over/Under Things_** setting
- **_Hitbox-based Hitscan Collision_** setting
- **Minimap:** Quickly press the automap button twice to toggle it
- **_Tag Finder_** from PrBoomX
- **_Sprite Shadows_** setting
- **_Radial Fog_** setting
- **_Thing Lighting Mode_** setting
- **Slow Motion** button
- Button to **_Teleport to automap pointer_**
- **_Freecam_**
- **_Flip Levels_** setting
- **_Milestone-Completion Announcements_** setting
- **_Bobbing Style_** setting
- **_Night-Vision Visor Effect_** setting
- **_Auto Save Interval_** setting, for periodic auto saves
- **_Damage Tint Cap_** and **_Bonus Tint Cap_**, to attenuate or disable said screen tinting
- **Jumping and Crouching**
- **_Message Lines_** settings, allowing a **message list**
- **NUGHUD**, an alternative lump for HUD customization (see `docs/nughud.md`)
- Most of **Crispy Doom's accessibility settings**
- **Support for weapon voxel models**
- **Support for SSG in Doom 1**
- **New cheats**, such as:
  - `FULLCLIP` for infinite ammo
  - `VALIANT` for fast weapons
  - `GIBBERS` to force gibbing on dying enemies
  - `RIOTMODE` cheat, to make enemies attack all sentient entities
  - `SUMMON` to spawn an actor based on its type index
  - `IDDF` to find a key in the automap
  - `MDK` to perform a hitscan attack of 1-million damage
  - `SAITAMA` to enable the MDK Fist
  - `BOOMCAN` for explosive hitscan attacks
  - `TURBO` to change the player speed in-game

See `FEATURES.md` for more information.

# Releases

Source code and Windows binaries (MSVC builds for Windows 7 and newer)
for the latest release can be found on the [Release](https://github.com/MrAlaux/Nugget-Doom/releases/latest) page.

The most recent list of changes can be found in the [Changelog](https://github.com/MrAlaux/Nugget-Doom/blob/master/CHANGELOG.md).

A complete history of changes and releases can be found on the [Releases](https://github.com/MrAlaux/Nugget-Doom/releases) page.

## Versioning

Nugget Doom follows a fairly simple (albeit arbitrary) **X.Y.Z** versioning system:

- **X** is increased in the event of major implementations, as were arbitrary/dynamic resolution and voxel support;
- **Y** is increased in the event of minor implementations, such as a new cheat;
- **Z** is increased in the event of bug fixes or text/code reformatting.

Incrementing any of the first values will reset the latter (i.e. a major change to 1.1.2 would shift it to 2.0.0).

The merging of changes from Woof! releases may affect any of the version values,
but not necessarily in the same way as Woof!'s own version (i.e. `Woof! 11.Y.Z -> 12.Y.Z` doesn't necessarily mean `Nugget 2.Y.Z -> 3.Y.Z`).

# Compiling

The Nugget Doom source code is available at GitHub: <https://github.com/MrAlaux/Nugget-Doom>.

## Linux, and Windows with MSYS2

The following build system and libraries need to be installed:
 
 * [CMake](https://cmake.org) (>= 3.15)
 * [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2) (>= 2.0.18)
 * [SDL2_net](https://github.com/libsdl-org/SDL_net)
 * [openal-soft](https://github.com/kcat/openal-soft) (>= 1.22.0 for PC Speaker emulation)
 * [libsndfile](https://github.com/libsndfile/libsndfile) (>= 1.1.0 for MPEG support)
 * [libebur128](https://github.com/jiixyj/libebur128) (>= 1.2.0)
 * [yyjson](https://github.com/ibireme/yyjson) (>= 0.10.0, optional)
 * [fluidsynth](https://github.com/FluidSynth/fluidsynth) (>= 2.2.0, optional)
 * [libxmp](https://github.com/libxmp/libxmp) (optional)
 
Usually your distribution should have the corresponding packages in its repositories,
and if your distribution has "dev" versions of those libraries, those are the ones you'll need.

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

## Acknowledgements

Help was provided by:

- [_atsb_ (a.k.a. _Gibbon_)](https://github.com/atsb)
- _Ayba_
- [_Brad Harding_](https://github.com/bradharding)
- [_ceski_](https://github.com/ceski-1)
- [_Fabian Greffrath_](https://github.com/fabiangreffrath)
- _Korp_
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
 © 2007-2011 Moritz "Ripper" Kroll;  
 © 2008-2019 Simon Judd;  
 © 2017 Christoph Oelckers;  
 © 2020 Alex Mayfield;  
 © 2020 JadingTsunami;  
 © 2020-2024 Fabian Greffrath;  
 © 2020-2024 Roman Fomin;  
 © 2021-2022 Ryan Krafnick;  
 © 2021-2026 Alaux;  
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

Files: `cmake/FindSDL2.cmake, cmake/FindSDL2_net.cmake`  
Copyright:  
 © 2018 Alex Mayfield.  
License: [BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)

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

Files: `third-party/md5/*`  
License: public-domain

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

Files: `win32/win_opendir.*`  
Copyright:  
 © 2019 win32ports.  
License: [MIT](https://opensource.org/licenses/MIT)
