## New Features

- **_Organize Saves by IWAD_** setting
- **_Explosion Shake Effect_** setting
- **_Disable Radiation Suit Tint_** setting
- **_Double Autoaim range_** setting
- **Improved fuzz effects**
- **Support for Berserk and Infinity icons in Status Bar**
- **Autoload folder for all games**
- **Setting of savegame and screenshot paths in config file**
- **Keep palette changes in screenshots** setting

## Changes

- **Merged changes from [Woof! 12.0.0](https://github.com/fabiangreffrath/woof/releases/tag/woof_12.0.0]**, note:
  - Removed solid-color crosshairs;
  - Added three-lined widget toggles to NUGHUD;
  - Changed internal values of Nugget's player cheats;
  - Maintained `-cdrom` command-line parameter;
  - Added menu items for _Air Absorption_ and _Doppler Effect_, and set both to 5 by default.
- **Made Powerup Timers flash when running out**
- **Disabled Powerup Timers in Strict Mode**
- **Added Explosive Hitscan support to MDK Fist**
- **Lowered MDK (Fist) attack's autoaim range** from 2048 to 1024 units
- **Moved `comp_bruistarget` menu item to the top of its list**

## Bug Fixes

- **Powerup sound not playing sometimes**
- **Incorrect Status Bar/NUGHUD minus sign handling**
- **Explosive Hitscan cheat not being saved in savegames**
- **MDK Fist not being accounted for by crosshair target highlighting**
- **View jittering when teleporting while crouching**
