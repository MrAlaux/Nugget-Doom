## New Features

- **Dedicated _Weapon Inertia_ scale value for firing**
- `FOVSKY` developer cheat

## Changes

- **Merged changes from Woof! post-15.2.0**, note:
  - Maintained Nugget's implementation of _Custom Skill_
  - Maintained support for SSG in Doom 1, now governed by the `doom1_ssg` CVAR
  - Maintained _Animated Health/Armor Count_ setting
  - Maintained _Pistol Start_ menu item
  - Removed `no_page_ticking` in favor of Woof's `menu_pause_demos`
  - Removed `quick_quitgame` in favor of Woof's `quit_prompt`
- **Improved sky stretching**
- **Improved _Hitbox Thing Lighting_**
- **Smoothened the transitions between idling and firing with _Weapon Inertia_ enabled**
- **Reduced sensitivity of _Vertical Target Lock-on_**
- **Reduced opacity of hitscan trails**
- **Replaced _Weapon Inertia_ toggle with the scale values themselves**
- **Made `TURBO` cheat display a help message**
- **Replaced "translucency" with "opacity" in setting descriptions where prudent**

## Bug Fixes

- **Crash when changing _[HUD] Wide Shift_ without loading into a map since launching the game**
- **Translucent weapon flashes and hitscan trails not being translucent if _Sprite Translucency_ were disabled**
- **Sprite shadows not interpolating on moving floors**
- **Hitscan trails becoming opaque when loading saves**
- **_Hitbox Thing Lighting_ not applying to weapon voxels**
- **_Vertical Target Lock-on_ menu item not being disabled when _Free Look_ were enabled**
