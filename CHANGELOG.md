## New Features

- **_Hold Use to Pick up Items_** setting
- **FPS-counter update time** setting

## Changes

- **Merged changes from Woof! post-15.2.0**, note:
  - Maintained Nugget's implementation of _Custom Skill_
    - Due to UDMF support, changed values of `custom_skill_things` [^1]
  - Maintained _Pistol Start_ menu item
  - Replaced `pitch_bend_range` with `pitched_sounds_range` [^1]
  - Changed _Fake Contrast_ default to _Vanilla_, and changed the order of its values [^1]
  - Removed `comp_switchsource` (now default behavior)
  - Removed `no_page_ticking` in favor of Woof's `menu_pause_demos`
  - Removed `quick_quitgame` in favor of Woof's `quit_prompt`
- **Improved sky stretching**
- **Made message shadows fade out when _Message Fadeout_ is enabled**
- **Optimized initialization of generic tranmaps by caching them in files**

## Bug Fixes

- **_Pain/Pickup/Powerup Flashes_ setting not disabling invulnerability effect and light-amp flashing**
- **_Invulnerability Colormap_ setting affecting light-amp flashing and disabling _Night-Vision Visor Effect_**
- **Quick-double-press actions being affected by game speed**
- **Weapon-translucency menu items not generating tranmaps immediately**
- **Key-binding menu items not working when backing from a submenu**
- **Quick-save/load prompts not playing a sound when brought up**
- **Quick-save/load prompts not being replaced by DeHackEd**
- **Quick-save/load prompts sometimes showing the wrong savegame name**
- **UI/HUD updating before game world when changing gamma and color settings**
- **_Tag Finder_ memory leak**

[^1]: This will affect existing config files.
