## New Features

- **_Level Stats Format_** settings
  - Removed _Kills % in Stats display_ setting in favor of them
- **Minimap zooming** (keyboard only)
- **NUGHUD:**
  - Text-line stacks
    - Replaced `x == -1` _Messages_ hack with dedicated `nughud_message_defx` toggle [1]
    - Removed `nughud_time_sts` in favor of stacks [1]
  - Status-Bar chunks
  - User-chosen `hud_widget_layout` support
- Toggle to **disable the Killough-face easter egg**

**[1].** This may affect existing NUGHUDs.

## Changes

- **Rewind improvements:**
  - Only delete key frames when rewinding within less than 0.6 seconds since the last rewind
  - "Aligned" key-framing time
- **Support for _Milestone Completion Announcements_ in multiplayer**
- **Changed NUGHUD defaults** to make use of stacks and further match the default WOOFHUD [1]
- **Improved Y-position of standalone Chat in NUGHUD**

**[1].** This may affect existing NUGHUDs.

## Bug Fixes

- **_Show Stats/Time_ toggle affecting Automap instead of HUD when on Minimap**
- **_Cycle Chasecam_ and _Toggle Crosshair_ inputs eating inputs**
