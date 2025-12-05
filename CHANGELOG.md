## New Features

- **Dedicated _Weapon Inertia_ scale value for firing**
- `FOVSKY` developer cheat

## Changes

- **Merged changes from [Woof! 15.3.0](https://github.com/fabiangreffrath/woof/releases/tag/woof_15.3.0)**, note:
  - Maintained support for SSG in Doom 1, now governed by the `doom1_ssg` CVAR
  - Maintained _Animated Health/Armor Count_ setting, and made it settable in WADs
- **Improved _Hitbox Thing Lighting_**
- **Optimized _Bounded Voxel Rendering_**
- **Optimized _Flip Levels_ post-process**
- **Smoothened the transitions between idling and firing with _Weapon Inertia_ enabled**
- **Reduced sensitivity of _Vertical Target Lock-on_**
- **Reduced opacity of hitscan trails**
- **Replaced _Weapon Inertia_ toggle with the scale values themselves**
- **Made `TURBO` cheat display a help message**
- **Replaced "translucency" with "opacity" in setting descriptions where prudent**

## Bug Fixes

- **Crash when changing _[HUD] Layout_ without loading into a map since launching the game**
- **_Bobbing Weapon Alignment_ misbehaving with DeHackEd-set sprite offsets**
- **Entities casting huge shadows when far above the floor**
- **`A_FireCGun` and `A_CPosAttack` using the wrong sound in HacX (fixes Uzi and Monstruct)**
- **Bullet puffs not spawning and hitscan trails misbehaving when firing at planes far away from lines**
- **Bullet puffs not spawning when "hitting" the sky behind a line if said sky were the ceiling of the line's front sector**
- **Length of hitscan trails becoming inaccurate at diagonal angles**
- **Translucent weapon flashes and hitscan trails not being translucent if _Sprite Translucency_ were disabled**
- **Sprite shadows not interpolating on moving floors**
- **Hitscan trails becoming opaque when loading saves**
- **Keyframe time becoming misaligned when rewinding**
- **_Hitbox Thing Lighting_ not applying to weapon voxels**
- **_Vertical Target Lock-on_ menu item not being disabled when _Free Look_ were enabled**
