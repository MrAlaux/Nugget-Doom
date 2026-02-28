## New Features

- **_Radial Fog_** setting
- **_[Crosshair] Health/Ammo Bars_** setting
- **_Translucent [Weapon] When Invisible_** setting
- **NUGHUD Weapon icon**
- **Pressing _Place [Automap] Mark_ while over a mark now changes that mark's color**
- Setting to **increase the duration of the "Entering" state in Doom 2's intermission screen**
- RNG override developer cheat

## Changes

- **Made hitscan attacks with a range greater than 192 units spawn hitscan trails** (was >= 128 units)
- **Made the minimap be opened by quickly pressing the automap button twice**, replacing the dedicated button
- **Made the minimap be closed upon dying**
- **Made blood-colored entities transfer their color to `A_SpawnObject` spawnees** (fixes blood coloring in SD21)
- **Made the following cheats repeatable**:
  - `NEXTMAP`, `NEXTSECRET`, `SUMMONR`, `IDDF##`, `IDDET`, `MDK`
- **Optimized drawing of dark minimap background**
- **Made the `IDDF##` cheats display a message when a key is found**

## Bug Fixes

- **Crash when the player becomes a zombie**
- **Crash when loading a save with the minimap enabled right after launch**
- **Desync related to bullet puffs**
- **Weapon becoming completely translucent if translucent flashes were enabled and the base sprite were blank**
- **_Last Weapon_ button not triggering _[Weapon] Switch Interruption_ correctly**
- **_Animated Health/Armor Counts_ not working on NUGHUDs with bars and no numbers**
- **Wrong message when clearing specific automap marks**
- **_[Crosshair] Horizontal-Autoaim Indicators_ not being cleared when activating freecam**
- **_Air Absorption_ menu item not being disabled when the _Sound Module_ weren't set to _OpenAL 3D_**
