## New Features

- Support for **square-pixel resolutions**, e.g. 1920x1080 instead of 1920x900
- Support for **aspect ratios below 4:3**, e.g. 5:4
- Setting to make **screen-darkening effects fade in**, enabled by default

## Changes

- **Improved accuracy of plane rendering**
- **Improved plane lighting in true-color modes**
- **Improved vertical sprite clipping** (thanks to @JNechaevsky and @pvictress)
- **Improved culling of distant sprite shadows**

## Bug Fixes

- **Crash when assigning brightmaps to states**
- **Crash when loading voxels for renamed sprites**
- **_Tag Finder_ cross marks changing size upon dynamic resolution changes**
- **Rock background and _Melt_ wipe being distorted when draw-to-texture were enabled**
- **Crash when taking screenshots with draw-to-texture enabled**
