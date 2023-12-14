## New Features

- **Support for intermediate resolutions** (e.g. 3X, 5X) **and 9X (1800p)**
- **_Background for all menus_** setting
- **_Vanilla Fake Contrast_** setting
- Toggle for **Diminished Lighting**
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

- **Merged changes from the following Woof! releases:**
  - [**12.0.1**](https://github.com/fabiangreffrath/woof/releases/tag/woof_12.0.1);
  - [**12.0.2**](https://github.com/fabiangreffrath/woof/releases/tag/woof_12.0.2).
- **NUGHUD:**
  - Let Ammo, Health and Armor icons fall back to vanilla sprites;
  - Made Patches and icons alignable horizontally and vertically;
  - Disabled Armor icon by default;
  - Changed `weapheight` range from [0, 200] to [-32, 32].
  - Inverted effect of `weapheight` (greater values now shift the sprites downwards)¹ for consistency with other properties;
  - Changed `secret_y` default to 84, to match Woof!.
- **Speed of non-Melt wipes is now independent of resolution**
- **Implemented Teleporter Zoom for multiplayer respawning**
- **_Blink [Automap] Marks_ message now includes number of marks**
- **Applied interpolation for Chasecam speed effect**
- **MDK Fist attacks now prioritize enemies over friends**
- **Current resolution is now reported by some video-related menu items**
- **Disabled `input_spy` and `input_menu_reloadlevel` when typing in Chat**

**1\.** This change will affect existing NUGHUDs. Negating the value of `weapheight` will restore the intended effect.

## Bug Fixes

- **Further corrected view pitch as FOV changes**
- **Disabled teleport-to-Automap-pointer during non-Casual Play**
- **Corrupted screenshots with integer scaling enabled** [by @ceski-1]
- **Excess speed when airborne with noclip enabled** [thanks @kitchen-ace]
- **Blazing door sound fix not applying to Boom doors**
- **Teleporter Zoom and BFG "explosion" shake affecting all players in multiplayer**
- **Explosion shake being stopped by the menu during demo playback and netgames**
- **Choppy Chasecam speed effect when looking up or down**
- **View snapping when teleporting to Automap pointer while crouching**
- **View clipping through floor when landing while crouching**
- **Fixed a demo desync** caused by a failed weapon autoswitch when picking up ammo
- **_View Height_ increments not being applied immediately**
- **Tweaked dark menu background and Automap overlay algorithm** (fixes very low values)
