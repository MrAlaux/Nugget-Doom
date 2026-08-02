## New Features

- **_Dithered Lighting_** setting
- **_Sky Projection_** setting, featuring **_Cylindrical_ projection**
  - Replaces _Linear Sky Scrolling_ setting [^1]
- **_Hold Use to Pick up Items_** setting
- **FPS-counter update time** setting
- **Customizable pronouns**

## Changes

- **Improved _Radial Fog_ for planes**
  - As a result, it no longer overrides the lighting fidelity of planes as set by the _Lighting Mode_ setting,
    so it ought to look rougher with lesser modes, especially with _Vanilla_ lighting
- **_Vertical Target Lock-on_ improvements**:
  - Made speed configurable through the `vertical_lockon_speed_pct` CVAR
  - Improved support for modded weapons, and made it respect _Smart Autoaim_
  - Improved easing
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
- **Lowercase characters not working if they came before uppercase characters in the same WAD**
- **Key-binding menu items not working when backing from a submenu**
- **Quick-save/load prompts not playing a sound when brought up**
- **Quick-save/load prompts not being replaced by DeHackEd**
- **Quick-save/load prompts sometimes showing the wrong savegame name**
- **Made increased intermission "Entering" delay not apply when the intermission becomes blank**
- **Chaingun not firing when out of ammo while the Infinite Ammo cheat were enabled**
- **Fancy Cast:**
  - Gibbing state not being flippable
  - Monster disappearing when trying to display its gibbing state if it didn't have one

[^1]: This might affect existing config files.
[^2]: This will only affect new config files.
