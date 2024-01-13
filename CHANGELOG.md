## New Features

- **_Alternative Intermission Background_** setting
- **Rewinding**
- **Support for optional health-based player pain sounds**
- **_Higher god-mode face priority_** setting

## Changes

- **Reimplemented _Move Over/Under Things_ feature**¹, making it much less bug-prone
- **Improved Automap line thickening when the window is downscaled**
- **Tweaked zooming effect**
- **Changed internal values of Nugget's internal mobj flags**²

**1\.** Among other changes, the setting itself has been extended: a value of `1` enables the feature only for players,
while a value of `2` enables it for all Things. This differs from the previous implementation, wherein `1` would enable
the feature for all Things.

**2\.** This change may affect existing saves.

## Bug Fixes

- **FOV effects not being cleared thoroughly upon loading levels**
