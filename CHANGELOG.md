## New Features

- **Support for intermediate resolutions** (e.g. 3X, 5X) **and 9X (1800p)**
- **_Background for all menus_** setting
- **_Vanilla Fake Contrast_** setting
- **Minimap mode for Automap**
- **NUGHUD:**
  - Ammo and Health icons;
  - Toggle to apply patch offsets;
  - Vertical view offset setting.
- **Further Extended HUD color customization**
- **_Show Kills Percentage [in Stats display]_** setting
- **Made the following cheats bindable to keys:**
  - Infinite Ammo;
  - Fast Weapons;
  - Resurrect;
  - Flight Mode;
  - Repeat Last Summon;
  - Linetarget Query;
  - MDK Attack;
  - MDK Fist;
  - Explosive Hitscan.
- **Show Save Messages** setting
- **_Direct Vertical Aiming_ for melee attacks**
- **_Disable Melee Snapping_** setting

## Changes

- **NUGHUD:**
  - Let Ammo, Health and Armor icons fall back to vanilla sprites;
  - Made Patches and icons alignable horizontally and vertically;
  - Disabled Armor icon by default;
  - Changed `weapheight` range from [0, 200] to [-32, 32].
- **Speed of non-Melt wipes is now independent of resolution**
- **Implemented Teleporter Zoom for multiplayer respawning**
- **Applied interpolation for Chasecam speed effect**
- **MDK Fist attacks now prioritize enemies over friends**
- **Current resolution is now reported by some video-related menu items**
- **Disabled `input_spy` and `input_menu_reloadlevel` when typing in Chat**

## Bug Fixes

- **Further corrected view pitch as FOV changes**
- **Disabled teleport-to-Automap-pointer during non-Casual Play**
- **Excess speed when airborne with noclip enabled** [thanks @kitchen-ace]
- **Blazing door sound fix not applying to Boom doors**
- **Teleporter Zoom and BFG "explosion" shake affecting all players in multiplayer**
- **Explosion shake being stopped by the menu during demo playback and netgames**
- **Fixed a demo desync** caused by a failed weapon autoswitch when picking up ammo
- **Tweaked dark menu background and Automap overlay algorithm** (fixes very low values)
