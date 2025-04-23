## New Features

- **_Hitbox-based Hitscan Collision_** setting
- **_Bounded Voxel Rendering_** setting
- **Fancy mode for character cast**
- **Intermission ratio stats** setting
- **Recoil-pitch scale percent** setting
- **_'IDDET'_ cheat**, to find exits in the automap
- **Number of _Bloodier Gibbing_ splats** setting
- Setting to **show only the powerup names/icons for powerup timers**
- **_IWAD only_ setting for _Alt. Intermission Background_** [1]
- **Number of total message lines** setting, to let the message review show more messages
- **Support for slow-motion toggling sounds**
- Setting to enable developer features
- _`DIMLIGHT`_ developer cheat to toggle diminishing lighting

## Changes

- **Applied _Hitbox Thing Lighting_ to weapon sprites**
- **When dying with freelook enabled, the camera is now pitched towards the killer**
  - Consequently, freelook can't be controlled manually while dead anymore
- **Keys now flash on the automap when flashing of keyed doors is enabled**
- **Added rough distance checks to explosive-hitscan cheat**
- **Tweaked _Bloodier Gibbing_ effect**
- Made _`FAUXDEMO`_ a developer cheat

## Bug Fixes

- **Rare crash involving PNG graphics and weapon inertia** [thanks @rfomin]
- **Automap zoom being reset when enabling automap with overlay disabled from a NUGHUD**
- **Direct vertical aiming for melee provoking hit effect without actually hitting an entity**
- **Specific switching bug with weapon toggles with switch interruption enabled**
- **Incorrect vanilla fake contrast with smooth diminishing lighting enabled**
- **Freecam interpolation issues**
- **Freecam not being forcefully disabled when enabling strict mode**
- **Potential desyncs by `comp_fuzzyblood` and `comp_nonbleeders`**
- **Message review overriding message durations even when the new one were shorter**
- **Message review ignoring the forced-duration setting**
- **`DSMNUBAK` being unused**

**[1].** This might affect existing config files.
