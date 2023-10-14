## New Features

- **_Organize Saves by IWAD_** setting
- **_Explosion Shake Effect_** setting
- **_Disable Radiation Suit Tint_** setting
- **_Double Autoaim range_** setting
- **Improved fuzz effects** (by @ceski-1)
- **Support for Berserk and Infinity icons in Status Bar**
- **Autoload folder for all games**
- **Setting of savegame and screenshot paths in config file**
- **_Use unused pain/bonus palettes_** setting
- **Keep palette changes in screenshots** setting

## Changes

- **Merged changes from [Woof! 12.0.0](https://github.com/fabiangreffrath/woof/releases/tag/woof_12.0.0)**, note:
  - Removed solid-color crosshairs;
  - Added three-lined widget toggles to NUGHUD;
  - Changed internal values of Nugget's player cheats (may affect existing saves);
  - Maintained `-cdrom` command-line parameter;
  - Added menu items for _Air Absorption_ and _Doppler Effect_, and set both to 5 by default.
- **Made Powerup Timers flash when running out**
- **Disabled Powerup Timers in Strict Mode**
- **Moved Event Timer within Time widget**; it is now displayed after everything else
- **Added Explosive Hitscan support to MDK Fist**
- **Lowered MDK (Fist) attack's autoaim range** from 2048 to 1024 units
- **Added support for Status Bar graphics of non-standard height** (thanks to @ceski-1 and @fabiangreffrath)
- **Added toggle for SSG availability display in Arms widget**
- **SSG availability display now applies to NUGHUD** if prudent
- **`blink_keys` can now be changed by WADs**
- **Moved `comp_bruistarget` menu item to the top of its list**

## Bug Fixes

- **Powerup sound not playing sometimes**
- **Buggy Health/Ammo cheats key bindings**
- **Arbitrary flag setting for _Bloodier Gibbing_ splats** (fixes crashes in `ANTA_REQ.wad`)
- **Incorrect Status Bar/NUGHUD minus sign handling**
- **Explosive Hitscan cheat not being saved in savegames**
- **Event Timer disappearing early with Automap enabled**
- **MDK Fist not being accounted for by crosshair target highlighting**
- **Zoom effect stutter** when not looking straight with widescreen enabled
- **View jittering when teleporting while crouching**
- **Delayed weapon position update when toggling a NUGHUD which changes weapon height**
- **Minor FOV discrepancy between widescreen and non-widescreen with default FOV**
