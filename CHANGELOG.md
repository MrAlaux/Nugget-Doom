## New Features

- **_Dithered Lighting_** setting
- **_Sky Projection_** setting, featuring **_Cylindrical_ projection**
  - Replaces _Linear Sky Scrolling_ setting [^1]
- **_Hold Use to Pick up Items_** setting
- **FPS-counter update time** setting
- **Customizable pronouns**

## Changes

- **Merged changes from Woof! post-15.2.0**, note:
  - Maintained Nugget's implementation of _Custom Skill_
    - Due to UDMF support, changed values of `custom_skill_things` [^2]
  - Maintained _Pistol Start_ menu item
  - Replaced `pitch_bend_range` with `pitched_sounds_range` [^2]
  - Changed _Fake Contrast_ default to _Vanilla_, and changed the order of its values [^2]
  - Removed `comp_switchsource` (now default behavior)
  - Removed `no_page_ticking` in favor of Woof's `menu_pause_demos`
  - Removed `quick_quitgame` in favor of Woof's `quit_prompt`
- **Improved sky stretching**
- **Improved _Radial Fog_ for planes**
  - As a result, it no longer overrides the lighting fidelity of planes as set by the _Lighting Mode_ setting,
    so it ought to look rougher with lesser modes, especially with _Vanilla_ lighting
- **Made message shadows fade out when _Message Fadeout_ is enabled**
- **_Vertical Target Lock-on_ improvements**:
  - Made speed configurable through the `vertical_lockon_speed_pct` CVAR
  - Improved support for modded weapons, and made it respect _Smart Autoaim_
  - Improved easing
- **Optimized initialization of generic tranmaps by caching them in files**
- **Changed the default values of the following settings**: [^2]
  - Set _Auto Save Interval_ to 300s, enabling periodic auto-saves
  - Enabled _Extra Gibbing_
  - Enabled _HUD/Menu Shadows_
  - Enabled _Message Fadeout_
  - Enabled _Blink Missing Keys [on Status Bar]_
  - Disabled `comp_manualdoor` (_Manually toggled moving doors are silent_)
  - Disabled `comp_cgundblsnd` (_Chaingun makes two sounds with one bullet_)
  - Enabled `comp_powerrunout` (_Use improved powerup run-out effect_)
  - Enabled `comp_unusedpals` (_Use unused pain/bonus palettes_)
  - Enabled `inter_entering_delay` (Increased duration of the "Entering" state in Doom 2's intermission screen)

## Bug Fixes

- **Voxels overriding sprites regardless of load order**
- **High-resolution sprites overriding normal sprites regardless of load order**
- **_Pain/Pickup/Powerup Flashes_ setting not disabling invulnerability effect and light-amp flashing**
- **_Invulnerability Colormap_ setting affecting light-amp flashing and disabling _Night-Vision Visor Effect_**
- **Lowercase characters not working if they came before uppercase characters in the same WAD**
- **Wrong player controls when using freecam during netgames**
- **Quick-double-press actions being affected by game speed**
- **Weapon-translucency menu items not generating tranmaps immediately**
- **Key-binding menu items not working when backing from a submenu**
- **Weapon-switch interruption allowing to switch to an already-raising weapon**
- **Quick-save/load prompts not playing a sound when brought up**
- **Quick-save/load prompts not being replaced by DeHackEd**
- **Quick-save/load prompts sometimes showing the wrong savegame name**
- **Made increased intermission "Entering" delay not apply when the intermission becomes blank**
- **Chaingun not firing when out of ammo while the Infinite Ammo cheat were enabled**
- **Fancy Cast:**
  - Gibbing state not being flippable
  - Monster disappearing when trying to display its gibbing state if it didn't have one
- **UI/HUD updating before game world when changing gamma and color settings**
- **_Tag Finder_ memory leak**

[^1]: This might affect existing config files.
[^2]: This will affect existing config files.
[^3]: This will only affect new config files.
