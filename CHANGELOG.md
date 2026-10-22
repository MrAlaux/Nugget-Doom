**Nugget Doom 6.1.0**, released October 17th, 2026.

## New Features

- **Frametime mode for the `FPS` cheat**
- Setting to **draw to SDL texture directly**
- Setting to **clear SDL renderer every frame**

## Changes

- **Merged changes from Woof! post-16.0.0**
- **Made the NUGHUD have its own slot**
- **Made _HUD Anchoring_ fine-tunable** [^1]
- **Improved and optimized clipping of _Sprite Shadows_**
- **Made messages scroll as they fade out when _Message Fadeout_ is enabled**
- **Made automap marks flash brighter when using _Highlight P.O.I.'s_**
- **Improved how the NUGHUD's minimap interacts with the message list**
- **Improved carousel fadeout**
- **Improved low-resolution post-process**
- **Allowed _Fast-Forward_ outside of demo playback**

## Bug Fixes

- **Lighting modes beyond _Vanilla_ being darker than intended**
- **Custom skill not taking effect when starting a new game with it during demo playback**
- **Shadows of right-offset sprites being cut off**
- **Forced weapon carousel malfunctioning during demo playback**
- **_Slow Motion_ affecting freecam speed**
- **_Slow Motion_ persisting when pausing with the <kbd>Pause</kbd> key**

[^1]: This will affect existing config files.
