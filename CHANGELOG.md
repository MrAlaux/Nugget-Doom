## New Features

- **Custom Skill: duplicate monster spawns** setting
- **FOV-based sky stretching** setting
- **_Allow [Weapon] Switch Interruption_** setting
- **_Message Flash_** setting
- **Support for powerup-timer icons**
  - Replaced `hud_stats_icons` with `hud_allow_icons` [1]
- **Quit Sound** setting, enabled by default

## Changes

- **Merged changes from [Woof! post-14.5.0]**, note:
  - Moved `show_berserk` menu item to _Status Bar/HUD_ setup menu
  - Removed `hudcolor_chat` and `chat_msg_timer` menu items
  - Renamed `#_bobbing_percentage` to `#_bobbing_pct` [1]
  - Gave an `sts_` prefix to `show_berserk`, `show_ssg`, and `alt_arms` [1]
  - Revised the descriptions of many of Nugget's new CVARs
- **Improved loading speed when rewinding**
- **Autosave improvements:**
  - Separated level-end autosaves and periodic autosaves into two settings,
    enabled the former by default, and gave menu items to both
  - Autosaves are now prefixed as per the executable's name
- **_Bloodier Gibbing_ setting now adds crushing effects**
- **Freecam speed is now mostly independent of game speed**
- **Renamed _ZDoom-like Item Drops_ (`zdoom_item_drops`) to _Toss Items Upon Death_ (`tossdrop`)** [1]
- **Removed _Upward Message Scrolling_ menu item**

## Bug Fixes

- **Crash when loading status bars taller than 32px**
- **_'FAST'_ cheat not fully toggling fast monsters outside of custom skill**
- **Fallback status-bar Berserk graphic not taking NUGHUD Ammo alignment into account**
- **Tag Finder not highlighting hidden lines**
- **Last-weapon button being affected by _Skip Ammoless Weapons_ setting**
- **Horizontal-autoaim indicators reacting to fuzzy targets regardless of detection setting**
- **_[Crosshair] Translucency_ menu item not being disabled when the crosshair were disabled**

**[1].** This will affect existing config files.
