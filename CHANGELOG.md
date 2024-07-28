## New Features

- **Custom skill level**
- **_Freecam_**
- **Autosaving**
- **NUGHUD bars**
- **_Prev/Next [Weapon Buttons] Skip Ammoless Weapons_** setting
- **_Highlight Current/Pending Weapon_** setting
- **_HUD/Menu Shadows_** setting
- **_[Crosshair] Translucency_** setting
- **_Show Thing Hitboxes [in Automap]_** setting
- **_'BABYMODE'_ cheat**, to toggle ITYTD benefits
- **_"Count"_ Level Stats Format**
- **Explosion shake intensity percent** setting
- **Setting to play `DSNOWAY` instead of `DSOOF` when failing to use key-locked triggers**
- **Independent translucency-percentage setting for _Translucent [Weapon] Flashes_**
- **Setting to use improved powerup run-out effect**

## Changes

- **Improved drawing of NUGHUD Face background**:
  - In singleplayer, the background from the status bar itself is now used
    instead of a multiplayer Face background
  - The vertical position of the background is now dependent on the height
    of the multiplayer Face backgrounds
- **_Tag Finder_ radius:**
  - Now adjusted based on automap zoom
  - Capped at a minimum of 24 map units when following the player
- **Made jump and crouch buttons cancel each other out**
- **Chasecam speed effect is now effort-based in complevels MBF and beyond**
- **Changed _"Fancy Teleport [to Pointer]"_ to _"Fancy Warping"_**

## Bug Fixes

- **Blocky _Selective Fuzz Darkening_ overflow** (caused visual glitches and crashes) [thanks @ceski-1]
- **Status bar popping when the _Alternative Intermission Background_ were enabled**
- **1S lines obscuring _Tag Finder_ highlights in automap**
- **Lost souls improperly colliding with the over/underside of other things**
- **Player getting stuck when standing up while over/under other things**
- **Automap position and scale being reset when changing modes**
- **Summoning cheats not properly checking for missing assets**
- **Resurrection cheats misaligning the key-framing time**
- **Crosshair not being drawn if the chasecam mode were set** even if the chasecam itself were disabled
- **Zoom being forcefully disabled if any player were dead**
- **HUD key not updating NUGHUD view offset**
- **Explosion shake and FOV effects not being reset when restarting levels on death**
- **_Selective Fuzz Darkening_ not being disabled in Strict Mode**
