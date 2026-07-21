## New Features

- **_Sky Projection_** setting, featuring **_Cylindrical_ projection**
  - Replaces _Linear Sky Scrolling_ setting [^1]
- **_Hold Use to Pick up Items_** setting
- **FPS-counter update time** setting

## Changes

- **Changed default _Auto Save Interval_ to 300s**, enabling periodic auto-saves by default [^2]
- **Improved _Radial Fog_ for planes**
  - As a result, it no longer overrides the lighting fidelity of planes as set by the _Lighting Mode_ setting,
    so it ought to look rougher with lesser modes, especially with _Vanilla_ lighting
- **_Vertical Target Lock-on_ improvements**:
  - Made speed configurable through the `vertical_lockon_speed_pct` CVAR
  - Improved support for modded weapons, and made it respect _Smart Autoaim_
  - Improved easing

## Bug Fixes

- **Key-binding menu items not working when backing from a submenu**
- **Quick-save/load prompts not playing a sound when brought up**
- **Quick-save/load prompts not being replaced by DeHackEd**
- **Quick-save/load prompts sometimes showing the wrong savegame name**

[^1]: This might affect existing config files.
[^2]: This will only affect new config files.
