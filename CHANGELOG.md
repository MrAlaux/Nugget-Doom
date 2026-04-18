## New Features

- **DeHackEd thing sprite `Scale` property** support

## Changes

- **Merged changes from Woof! post-15.2.0**, note:
  - Maintained Nugget's implementation of _Custom Skill_
    - Due to UDMF support, changed values of `custom_skill_things` [^1]
  - Maintained _Pistol Start_ menu item
  - Changed _Fake Contrast_ default to _Vanilla_, and changed the order of its values [^1]
  - Removed `no_page_ticking` in favor of Woof's `menu_pause_demos`
  - Removed `quick_quitgame` in favor of Woof's `quit_prompt`
- **Optimized initialization of _Interpolated/True-color Lighting_ and _Radial Fog_ through multithreading**
- **Improved sky stretching**

## Bug Fixes

- **Sprite shadows flickering when the crosshair locked onto a target with _Flip Levels_ enabled**
- **Horizontal-autoaim indicators not accounting for _Flip Levels_**

[^1]: This will affect existing config files.
