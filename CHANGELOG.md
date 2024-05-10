## New Features

- Restored **_Selective Fuzz Darkening_** setting [by @ceski-1]
- **_Level Stats Format_** settings
  - Removed _Kills % in Stats display_ setting in favor of them
- **Minimap zooming** (keyboard only)
- Toggle to **disable the Killough-face easter egg**
- Toggle to **make attackers face fuzzy targets straight**

## Changes

- **Merged changes from [Woof! 14.5.0](https://github.com/fabiangreffrath/woof/releases/tag/woof_14.5.0)**, note:
  - Removed Nugget's `all` autoload folder in favor of Woof's `all-all`
  - Changed `nughud_secret_y` default to match default Boom HUD
- **Rewind improvements:**
  - Only delete key frames when rewinding within less than 0.6 seconds since the last rewind
  - "Aligned" key-framing time
- **Adjusted spawning height of _Bloodier Gibbing_ blood splats**
- **Support for _Milestone Completion Announcements_ in multiplayer**
- **Lengthened FOV slider**

## Bug Fixes

- **_Show Stats/Time_ toggle affecting Automap instead of HUD when on Minimap**
- **_Cycle Chasecam_ and _Toggle Crosshair_ inputs eating inputs**
