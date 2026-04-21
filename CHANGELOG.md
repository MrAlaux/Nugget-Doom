## New Features

- **DeHackEd thing sprite `Scale` property** support

## Changes

- **Optimized initialization of _Interpolated/True-color Lighting_ and _Radial Fog_ through multithreading**
- **Randomized duration of spawn, death, and gib states in _Fancy Cast_**, emulating in-game behavior

## Bug Fixes

- **Crash when toggling _Smooth Palette Tinting_ while the screen were tinted**
- **Damage tint not being reduced when the game were paused if _Smooth Palette Tinting_ were enabled**
- **_Blink Missing Keys_ not working when `screenblocks` were set to 12 and NUGHUD were disabled**
- **Sprite shadows flickering when the crosshair locked onto a target with _Flip Levels_ enabled**
- **Horizontal-autoaim indicators not accounting for _Flip Levels_**
- **_Solid [Status Bar] Background Color_ not respecting color settings with true-color rendering**
