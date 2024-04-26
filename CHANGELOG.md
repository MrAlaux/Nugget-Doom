## New Features

- **_Level Stats Format_** settings
  - Removed _Kills % in Stats display_ setting in favor of them
- **Minimap zooming** (keyboard only)
- **NUGHUD:**
  - Text-line stacks
    - Removed `nughud_time_sts` in favor of them [1]
  - Status-Bar chunks
  - User-chosen `hud_widget_layout` support

**[1].** This may affect existing NUGHUDs.

## Changes

- **Rewind improvements:**
  - Only delete key frames when rewinding within less than 0.6 seconds since the last rewind
  - "Aligned" key-framing time
- **Support for _Milestone Completion Announcements_ in multiplayer**
- **Changed NUGHUD defaults** to make use of stacks and further match the default WOOFHUD [1]

**[1].** This may affect existing NUGHUDs.

## Bug Fixes

- **_Show Stats/Time_ toggle affecting Automap instead of HUD when on Minimap**
- **_Cycle Chasecam_ and _Toggle Crosshair_ inputs eating inputs**
