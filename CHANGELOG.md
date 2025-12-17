## New Features

- **_[Crosshair] Health/Ammo Bars_** setting
- **_Translucent [Weapon] When Invisible_** setting
- **NUGHUD Weapon icon**
- **Pressing _Place [Automap] Mark_ while over a mark now changes that mark's color**

## Changes

- **Made hitscan attacks with a range greater than 192 units spawn hitscan trails** (was >= 128 units)
- **Made the minimap be opened by quickly pressing the automap button twice**, replacing the dedicated button
- **Made the minimap be closed upon dying**
- **Made blood-colored entities transfer their color to `A_SpawnObject` spawnees** (fixes blood coloring in SD21)
- **Made the following cheats repeatable**:
  - `NEXTMAP`, `NEXTSECRET`, `SUMMONR`, `IDDF##`, `IDDET`, `MDK`
- **Made the `IDDF##` cheats display a message when a key is found**

## Bug Fixes

- **_Last Weapon_ button not triggering _[Weapon] Switch Interruption_ correctly**
- **_Animated Health/Armor Counts_ not working on NUGHUDs with bars and no numbers**
- **Wrong message when clearing specific automap marks**
- **_[Crosshair] Horizontal-Autoaim Indicators_ not being cleared when activating freecam**
