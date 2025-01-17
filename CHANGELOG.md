## New Features

- **_Message Fadeout_** setting
- **_Weapon Bob Speed_** setting
- **_Bob [Weapon] While Switching_** setting
- **_[Color] Contrast_** setting [by @pvictress]

## Changes

- **Merged changes from [Woof! 15.1.0](https://github.com/fabiangreffrath/woof/releases/tag/woof_15.1.0)**, note:
  - Replaced `fullscreen_hud_type` with `use_nughud`, with the NUGHUD now replacing the second-to-last HUD
  - Removed `fuzzdark_mode` in favor of Woof!'s `fuzzmode` (_Refraction_ is equivalent to _Selective Fuzz Darkening_)
  - Removed `comp_blazing2`, as its behavior was integrated into `comp_blazing`
- **Improved interpolation of weapon sprites**
- **Applied weapon inertia when firing** (added `weapon_inertia_fire` CVAR to disable it)
- **Lowered `weapon_inertia_scale_pct` limit to -200**
- Nugget's translucency features now use translucency maps from a shared pool,
  potentially improving program startup time in exchange for stutters
  when enabling said features for the first time since launch

## Bug Fixes

- Broken movement in systems assuming `char`s to be unsigned
- `comp_lsamnesia` inversely affecting collision with entities (i.e. lost souls would remember their target)
- Weapon sprites being raised/lowered excessively when using switch interruption
  with certain bobbing styles and/or reduced weapon bobbing amplitude
- _Flip Levels_ setting not working for side screens in three-screen mode
- Message grouping only checking as many characters as the last message had,
  causing incorrect grouping (e.g. new message "TEST" being grouped with last message "TES")
