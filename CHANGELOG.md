## New Features

- Restored **_Selective Fuzz Darkening_** setting [by @ceski-1]
- **_Level Stats Format_** settings
  - Removed _Kills % in Stats display_ setting in favor of them
- **Minimap zooming** (keyboard only)
- **NUGHUD:**
  - Text-line stacks
    - Replaced `x == -1` _Messages_ hack with dedicated `nughud_message_defx` toggle [1]
    - Removed `nughud_time_sts` in favor of stacks [1]
  - Status-Bar chunks
  - User-chosen `hud_widget_layout` support
- **Automap color for trigger lines**
- **Key-binding for Computer Area Map cheat**
- Toggle to **disable the Killough-face easter egg**
- Toggle to **make attackers face fuzzy targets straight**
- Toggle to **allow Level Stats icons**

## Changes

- **Merged changes from [Woof! 14.5.0](https://github.com/fabiangreffrath/woof/releases/tag/woof_14.5.0)**, note:
  - Removed Nugget's `all` autoload folder in favor of Woof's `all-all`
  - Changed `nughud_secret_y` default to match default Boom HUD
- **NUGHUD:**
  - **Changed defaults** to make use of stacks and further match the default WOOFHUD [1]
  - **Improved Y-position of standalone Chat**
  - **Made Status Bar elements be drawn before HUD elements**, as was before 3.0.0 [1]
- **Rewind improvements:**
  - Only delete key frames when rewinding within less than 0.6 seconds since the last rewind
  - "Aligned" key-framing time
- **Adjusted spawning height of _Bloodier Gibbing_ blood splats**
- **Support for _Milestone Completion Announcements_ in multiplayer**
- **Lengthened FOV slider**
- **Removed temporary support for "Nugget 2.4.0" saves**

## Bug Fixes

- **Sky being distorted by zoom effect**
- **_Show Stats/Time_ toggle affecting Automap instead of HUD when on Minimap**
- **NUGHUD view offset behaving incorrectly** (should now match pre-3.0.0 behavior) [1]
- **_Cycle Chasecam_ and _Toggle Crosshair_ inputs eating inputs**
- **Crosshair jumping vertically when changing screen size to Status Bar from NUGHUD with view offset**

**[1].** This may affect existing NUGHUDs.
