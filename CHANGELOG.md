## New Features

- **Support for high-resolution sprites between `HI_START`/`HI_END` markers**
- **DeHackEd thing `Scale` property** support
- **Weapon Carousel fadeout** setting
- **NUGHUD:**
  - Toggle to hide armor-related widgets when the player has no armor
  - Toggle to resize the ammo bar when the player has the backpack
- Toggle to **organize screenshots**
- **Support for lowercase characters in console font**
- **Toggle to disable minimap double-press**
- **Toggles to disable specific milestone-completion announcements**
- Toggle to **use palette colors exactly when gamma correction is disabled**
- Developer cheats to toggle netgame and deathmatch states
- Version cheat

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
- **Optimized initialization of _Interpolated/True-color Lighting_ and _Radial Fog_ through multithreading**
- **Optimized initialization of generic tranmaps by caching them in files**
- **Made bloodier crushing apply only to shootable things**
- **Improved crosshair target detection for modded weapons**
  - Fixed _Smart Autoaim_ disabling target detection for projectile weapons
- **Replaced _Explosion Shake Effect_ with _Screen-Shake Effects_**
  - The setting now governs the effects in general, with each effect being toggleable
  - Renamed `explosion_shake` and `explosion_shake_intensity_pct`
    to `screen_shake_explosions` and `screen_shake_intensity_pct` respectively [^1]
  - Added shake effect upon impact of high-damage projectiles (e.g. BFG projectile)
  - Added shake effect for hitscan attacks whose damage exceeds their range (e.g. berserk fist)
- **Raised upper limits of color _Intensity_ and _Saturation_ settings to 200%**
- **Made automap-mark coloring take the <kbd>Shift</kbd> key into account**
- **Randomized duration of spawn, death, and gib states in _Fancy Cast_**, emulating in-game behavior
- **Improved _Sprite Shadows_ casting threshold**

## Bug Fixes

- **Crash when toggling _Smooth Palette Tinting_ while the screen were tinted**
- **_Sprite Shadows_ relying on the first palette color being black**
- **Partially fixed continuous area-of-effect attacks (e.g. fire, gas) triggering the explosion shake effect excessively**
- **_Pain/Pickup/Powerup Flashes_ setting not disabling invulnerability effect and light-amp flashing**
- **_Invulnerability Colormap_ setting affecting light-amp flashing and disabling _Night-Vision Visor Effect_**
- **Wrong _Tag Finder_ colors with different palettes**
- **Damage tint not being reduced when the game were paused if _Smooth Palette Tinting_ were enabled**
- **_Blink Missing Keys_ not working when `screenblocks` were set to 12 and NUGHUD were disabled**
- **Sprite shadows flickering when the crosshair locked onto a target with _Flip Levels_ enabled**
- **Horizontal-autoaim indicators not accounting for _Flip Levels_**
- **Quick-double-press actions being affected by game speed**
- **True-color rendering breaking invisibility colormap in beta-emulation mode**
- **_Solid [Status Bar] Background Color_ not respecting color settings with true-color rendering**
- **UI/HUD updating before game world when changing gamma and color settings**
- **_Tag Finder_ memory leak**

[^1]: This will affect existing config files.
