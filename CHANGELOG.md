## New Features

- **_Background for all menus_** setting
- **Minimap mode for Automap**
- **NUGHUD:**
  - Ammo and Health icons;
  - Toggle to apply patch offsets.
- **Further Extended HUD color customization**
- **_Show Kills Percentage [in Stats display]_** setting
- **Show Save Messages** setting
- **_Direct Vertical Aiming_ for melee attacks**
- **_Disable Melee Snapping_** setting

## Changes

- **NUGHUD:**
  - Let Ammo, Health and Armor icons fall back to vanilla sprites;
  - Made Patches and icons alignable horizontally and vertically;
  - Disabled Armor icon by default.
- **Implemented Teleporter Zoom for multiplayer respawning**
- **MDK Fist attacks now prioritize enemies over friends**
- **Current resolution is now reported by some video-related menu items**
- **Disabled `input_spy` and `input_menu_reloadlevel` when typing in Chat**

## Bug Fixes

- **Disabled teleport-to-Automap-pointer during non-Casual Play**
- **Excess speed when airborne with noclip enabled** [thanks @kitchen-ace]
- **Teleporter Zoom and BFG "explosion" shake affecting all players in multiplayer**
- **Fixed a demo desync** caused by a failed weapon autoswitch when picking up ammo
- **Tweaked dark menu background and Automap overlay algorithm** (fixes very low values)
