## New Features

- **DeHackEd thing sprite `Scale` property** support
- **Weapon Carousel fadeout** setting
- Toggle to **use palette colors exactly when gamma correction is disabled**

## Changes

- **Merged changes from Woof! post-15.2.0**, note:
  - Maintained Nugget's implementation of _Custom Skill_
    - Due to UDMF support, changed values of `custom_skill_things` [^1]
  - Maintained _Pistol Start_ menu item
  - Replaced `pitch_bend_range` with `pitched_sounds_range` [^1]
  - Changed _Fake Contrast_ default to _Vanilla_, and changed the order of its values [^1]
  - Removed `no_page_ticking` in favor of Woof's `menu_pause_demos`
  - Removed `quick_quitgame` in favor of Woof's `quit_prompt`
- **Improved sky stretching**
- **Optimized initialization of _Interpolated/True-color Lighting_ and _Radial Fog_ through multithreading**
- **Optimized initialization of generic tranmaps by caching them in files**
- **Raised upper limits of color _Intensity_ and _Saturation_ settings to 200%**
- **Randomized duration of spawn, death, and gib states in _Fancy Cast_**, emulating in-game behavior

## Bug Fixes

- **Crash when toggling _Smooth Palette Tinting_ while the screen were tinted**
- **_Sprite Shadows_ relying on the first palette color being black**
- **_Pain/Pickup/Powerup Flashes_ setting not disabling invulnerability effect and light-amp flashing**
- **_Invulnerability Colormap_ setting affecting light-amp flashing and disabling _Night-Vision Visor Effect_**
- **Damage tint not being reduced when the game were paused if _Smooth Palette Tinting_ were enabled**
- **_Blink Missing Keys_ not working when `screenblocks` were set to 12 and NUGHUD were disabled**
- **Sprite shadows flickering when the crosshair locked onto a target with _Flip Levels_ enabled**
- **Horizontal-autoaim indicators not accounting for _Flip Levels_**
- **Quick-double-press actions being affected by game speed**
- **_Solid [Status Bar] Background Color_ not respecting gamma and color settings with true-color rendering**
- **UI/HUD updating before game world when changing gamma and color settings**

[^1]: This will affect existing config files.
