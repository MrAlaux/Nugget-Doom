## New Features

- **_Sky Projection_** setting, featuring **_Cylindrical_ projection**
  - Replaces _Linear Sky Scrolling_ setting [^1]
- **_Hold Use to Pick up Items_** setting
- **FPS-counter update time** setting

## Changes

- **Merged changes from Woof! post-15.2.0**, note:
  - Maintained Nugget's implementation of _Custom Skill_
    - Due to UDMF support, changed values of `custom_skill_things` [^2]
  - Maintained _Pistol Start_ menu item
  - Replaced `pitch_bend_range` with `pitched_sounds_range` [^2]
  - Changed _Fake Contrast_ default to _Vanilla_, and changed the order of its values [^2]
  - Removed `comp_switchsource` (now default behavior)
  - Removed `no_page_ticking` in favor of Woof's `menu_pause_demos`
  - Removed `quick_quitgame` in favor of Woof's `quit_prompt`
- **Improved sky stretching**
- **Changed default _Auto Save Interval_ to 300s**, enabling periodic auto-saves by default [^3]
- **Improved _Radial Fog_ for planes**
  - As a result, it no longer overrides the lighting fidelity of planes as set by the _Lighting Mode_ setting,
    so it ought to look rougher with lesser modes, especially with _Vanilla_ lighting
- **Made message shadows fade out when _Message Fadeout_ is enabled**
- **_Vertical Target Lock-on_ improvements**:
  - Made speed configurable through the `vertical_lockon_speed_pct` CVAR
  - Improved support for modded weapons, and made it respect _Smart Autoaim_
  - Improved easing
- **Optimized initialization of generic tranmaps by caching them in files**

## Bug Fixes

- **Voxels overriding sprites regardless of load order**
- **High-resolution sprites overriding normal sprites regardless of load order**
- **_Pain/Pickup/Powerup Flashes_ setting not disabling invulnerability effect and light-amp flashing**
- **_Invulnerability Colormap_ setting affecting light-amp flashing and disabling _Night-Vision Visor Effect_**
- **Lowercase characters not working if they came before uppercase characters in the same WAD**
- **Quick-double-press actions being affected by game speed**
- **Weapon-translucency menu items not generating tranmaps immediately**
- **Key-binding menu items not working when backing from a submenu**
- **Quick-save/load prompts not playing a sound when brought up**
- **Quick-save/load prompts not being replaced by DeHackEd**
- **Quick-save/load prompts sometimes showing the wrong savegame name**
- **UI/HUD updating before game world when changing gamma and color settings**
- **_Tag Finder_ memory leak**

[^1]: This might affect existing config files.
[^2]: This will affect existing config files.
[^3]: This will only affect new config files.
