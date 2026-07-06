# Nugget Doom Features

Note that this feature list is relative to [Woof!'s](https://github.com/fabiangreffrath/woof/blob/woof_15.3.0/README.md#key-features);
read the latter for more details.
Some features were first implemented in Nugget Doom and later in Woof!, so they're not listed here anymore.

Most of Nugget Doom's features come from other sources, like source ports and mods;
the initial implementations for some are **ported from (p.f.)** said sources, while others are just **inspired by (i.b.)** them.
These acknowledgements are included in the feature lists below; be aware that some might be inaccurate or outright missing.

A few settings are labeled as **_CFG-only_**: they can only be toggled by editing `nugget-doom.cfg`.
For these settings, their CVAR names are provided alongside the _CFG-only_ label as guidance.

## General

- **_Lighting Mode_** settings:
  - _Vanilla_:
    - Supports 16 distinct light levels for sectors
  - _Smooth_ (a.k.a. _Smooth Diminishing Lighting_):
    - Smoother lighting for floors and ceilings
    - Supports 31 distinct light levels for sectors
  - _Interpolated_:
    - Partial true-color lighting
    - Smoother lighting in general
    - Theoretically compatible with all colormap effects
    - Supports 241 distinct light levels for sectors
  - _True-color_:
    - Full true-color lighting
    - Better color quality
    - Smoothest lighting in general
    - Does not support some colormap effects (colormap brightmaps and fog are still supported)
    - Supports 241 distinct light levels for sectors
  - Notes on the true-color modes:
    - Compatible with custom tinted palettes
    - No support for true-color (32-bit PNG) graphics; those are still palettized at startup
    - May significantly increase memory consumption and loading times of program startup and changes to color settings (e.g. gamma correction),
      especially with _Smooth Palette Tinting_ enabled
- **Extended FOV range:** [20, 140]
- **Stretch viewport to fit window** setting (CFG-only: `stretch_to_fit`) [i.b. and partially p.f. Crispy Doom; i.b. ZDoom]
- **Set _Air Absorption_ and _Doppler Effect_ to 5 by default**
- **_Bounded Voxel Rendering_** setting, to draw each voxel as a rectangular sprite
- **FOV-based sky stretching** setting (CFG-only: `fov_stretchsky`)
- **Tweaked _Stretch Short Skies_ algorithm**
- **_Black Fade_ screen wipe**
- **Extended _Level Brightness_ range:** [-8, 8]
- **Support for SSG in Doom 1** [p.f. Woof! 15.2.0]
- **_Hitbox-based Hitscan Collision_** setting
- **_"Direct + Auto"_ mode for Vertical Aiming**
- **_Direct Vertical Aiming_ for melee attacks**
- **_Move Over/Under Things_** setting [partially p.f. Crispy Doom, DSDA-Doom]
- **Jumping** (default key: <kbd>Alt</kbd>) [p.f. Crispy Doom]
- **Crouching/ducking** (default key: <kbd>C</kbd>) [i.b. ZDoom]
  - Includes support for crouching-player sprites, named `PLYC`; must be provided by the user
- **_Hold Use to Pick up Items_** setting
- **_View Height_** setting, which allows to enter a custom POV height value in the [32, 56] range [i.b. Brutal Doom]
- **_Vertical Target Lock-on_** setting, to make the camera automatically lock onto targets vertically [i.b. Rise of the Triad]
- **Flinching** setting, to flinch upon landing and/or taking damage
- **_Screen-Shake Effects_** setting [i.b. Doom Retro]
  - Configurable through the CFG-only `screen_shake_#` CVARs
- **_Subtle Idle Bobbing/Breathing_** setting [p.f. International Doom]
- **_Teleporter Zoom_** setting [i.b. ZDoom]
- **_Death Camera_** setting [i.b. ZDoom]
- **_Freecam_**, which repurposes the following inputs:
  - _Cycle Chasecam_ to toggle control between the camera and the player
  - _Use_ to reset the camera to the player's POV
  - _Fire_ to lock onto and let go of a sentient thing's POV
  - _Previous/Next Weapon_ to adjust the camera's movement speed
  - Quick _Strafe On_ double-press to center the camera vertically
- **_Chasecam_** [i.b. ZDoom]
- **Slow Motion** button
- **Zoom** button
- **_Backdrop for all menus_** setting
- **_No Palette Tint in Menus_** setting [i.b. Crispy Doom]
- **_HUD/Menu Shadows_** setting [i.b. CRL]
  - Opacity level determined by the CFG-only `hud_menu_shadows_filter_pct` CVAR
- **_Sprite Shadows_** setting [i.b. Doom Retro]
  - Opacity level determined by the CFG-only `sprite_shadows_tran_pct` CVAR
- **_Thing Lighting Mode_** setting
  - _Hitbox_ suggested by [@fragglet](https://github.com/fragglet)
  - _Per-column_ inspired by PSX Hexen
- **_Radial Fog_** setting
  - The fidelity of the effect on floors and ceilings can be tuned through the CFG-only `radial_plane_fog_fidelity` CVAR;
    greater values increase fidelity but cause more stutter
- **_Flip Levels_** setting
- **Low-resolution pixel width/height** settings, to enlarge pixels when using 100% resolution
  (CFG-only: `lowres_pixel_width`, `lowres_pixel_height`) [i.b. Doom Retro]
- **_No Berserk Tint_** setting
- **_No Radiation Suit Tint_** setting
- **_Night-Vision Visor Effect_** setting [i.b. International Doom]
- **_Smooth Palette Tinting_** setting
- **_Damage Tint Cap_** and **_Bonus Tint Cap_**, to attenuate or disable said screen tinting
- **_Fake Contrast_** setting
- Toggle for **diminishing lighting** (CFG-only: `diminishing_lighting`)
  - _Diminishing lighting_ (a.k.a. fog) refers to geometry and entities becoming brighter as they come closer to the camera
- **_Screen Wipe speed percentage_** setting
- **_Alternative Intermission Background_** setting, to replace the intermission graphic with a darkened rotating camera view
- **Color settings** [p.f. International Doom]
  - _Contrast_ by [@pvictress](https://github.com/pvictress)
- **_Sound Clipping Distance_** selection, to optionally double the distance at which sound effects become audible
- **_One-Key Quick Save/Load_** setting, to skip the confirmation prompt
- **_Auto Save Interval_** setting, for periodic auto saves
- **Rewinding** [i.b. DSDA-Doom]
  - _Rewind Interval_ : number of seconds between key frames
  - _Rewind Depth_: number of maximum key frames to store; when exceeded, the oldest key frame is deleted to make room for the new one;
    set to 0 to disable rewinding
  - _Rewind Frame Timeout_: number of maximum milliseconds that the game can spend storing a single key frame;
    if exceeded, storing of further key frames is stopped
  - _Rewind 4-Frame Timeout_: number of maximum milliseconds that the game can have spent storing the last 4 key frames;
    if exceeded, storing of further key frames is stopped
  - The _Frame Timeout_ can be set to a higher value to allow flukes in key-frame storage time,
    while the _4-Frame Timeout_ stops storing if it is consistently slow
  - If stopped, storing can be restarted by attempting to rewind or by changing any of the settings above
- **_Play Internal Demos_** setting, to control whether or not to play demos built into WADs
- **_Quick "Quit Game"_** setting, to skip the confirmation prompt [p.f. Crispy Doom]
- Toggle for **_Weapon Flash Lighting_** [p.f. Crispy Doom]
- Toggle for **_Weapon Flash Sprite_** [p.f. Crispy Doom]
- Toggle for **_Invulnerability Colormap_** [p.f. Crispy Doom]
- **Fixed bullet puffs not spawning when firing at planes far away from lines**
- **Fixed bullet puffs not spawning when "hitting" the sky behind a line if said sky were the ceiling of the line's front sector**
- **Made blood-colored entities transfer their color to `A_SpawnObject` spawnees**

## Weapons

- **Extended _View/Weapon Bob_ percentages**
- **Recoil-pitch scale percent** setting
- **_Smart Autoaim_** setting
- **_No Horizontal Autoaim_** setting
- **_Switch [Weapon] on Pickup_** setting
- **_Improved Weapon Toggles_** setting
  - Normally, when using the weapon toggles (that is, the same key for both the fist and chainsaw or shotgun and SSG),
    one must wait until the first weapon in the toggle is equipped before being able to select the other weapon in it;
    this setting changes that, allowing to select either weapon in the toggle without waiting
- **_Allow [Weapon] Switch Interruption_** setting
- **_Prev/Next Skip Ammoless Weapons_** setting, to make the previous/next-weapon buttons skip weapons with insufficient ammo
- **_Horizontal_ Weapon Centering** setting [i.b. DSDA-Doom]
- **Made _Bobbing Weapon Alignment_ respect DeHackEd-set sprite offsets**
- **Always Bob** setting (CFG-only: `always_bob`)
  - This setting forces the weapon to bob in every tic of its ready state,
    whether or not the `A_WeaponReady` action is called in said tic,
    fixing choppy bobbing for weapons like the chainsaw
- **_Bobbing Style_** setting [p.f. Zandronum]
- **_Weapon Bob Speed_** setting
- **_Bob While Switching_** setting
- **_Weapon Inertia_** settings [by _ceski_]
- **_Weapon Squat Upon Landing_** setting [p.f. Crispy Doom]
- **_Translucent [Weapon] When Invisible_** setting
- **_Force Weapon Carousel_** setting
- **Weapon Carousel fadeout** setting (CFG-only: `carousel_fadeout`)
- **_[Weapon] Flash Opacity_** setting [i.b. Crispy Doom]
- **Support for weapon voxel models**
- **"Correct" first-person-sprite centering** setting (CFG-only: `sx_fix`)
  - In vanilla Doom, weapon sprites are centered with an offset of one unit to the right;
    this setting changes that

## Status Bar/HUD

- **NUGHUD**, an alternative lump for HUD customization (see `docs/nughud.md`)
- **SBARDEF:**
  - Chat hack to move it vertically based on the height of the message list
- **Crosshair:**
  - Opacity setting
  - Vertical-only target lock-on
  - Health/ammo bars
  - Improved target detection
  - Horizontal-autoaim indicators
  - Option to account for fuzzy targets [i.b. From Doom With Love]
  - Dedicated toggle button
- **_Sound only_ option for "secret revealed" message**
- **Show Save Messages** setting (CFG-only: `show_save_messages`)
- **_Milestone-Completion Announcements_** setting, to report completion of milestones (e.g. all items acquired)
  - Each announcement can be toggled through the CFG-only `announce_milestone_#` CVARs
- **_Message Fadeout_** setting
- **_Message Flash_** setting
- **_Message Lines_** settings
  - Scrolling direction determined by the CFG-only `hud_msg_scrollup` CVAR
  - The CFG-only `hud_msg_total_lines` CVAR determines how many messages will be shown during message review;
    if lesser than `hud_msg_lines`, that will be used instead
- **_Group Repeated Messages_** setting [i.b. Doom Retro]
- **_(Chat) Message Duration_** settings
- **_Show Powerup Timers_** setting
  - The CFG-only `hud_power_timers_notime` CVAR can be enabled to show only the powerup names/icons
- **_Blink Missing Keys_** setting [partially p.f. Crispy Doom]
- **_Animated Health/Armor Counts_** setting
- **_Berserk display when using Fist_** setting [partially p.f. Crispy Doom]
- **_Automap Level Stats Format_** setting
- **Level-Stats Selection** settings (CFG-only: `hud_stats_#[_map]`)
- **Event Timers:**
  - _Teleport Timer_ [i.b. Crispy Doom];
  - _Key-Pickup Timer_ [same as above].
- **Extended HUD color customization**
- **Armor count is colored gray only when in God Mode** when _Colored Numbers_ are enabled
- **Support for Berserk (`STBERSRK`) and Infinite Ammo (`STINFNTY`) icons**
- **Support for HUD icons**:
  - Level-stats widget: `HUDKILLS`, `HUDITEMS`, `HUDSCRTS`
  - Powerup-timers widget: `HUDINVIS`, `HUDINVUL`, `HUDLIGHT`, `HUDSUIT`

## Automap

- **Minimap mode:** Quickly press the automap button twice to toggle it [i.b. DSDA-Doom]
  - The CFG-only `minimap_double_press` CVAR controls this functionality
- Button to **_Highlight Points of Interest_**; marks and keyed lines (default: <kbd>B</kbd>)
- **_Tag Finder_** button [p.f. PrBoomX]
  - Position the automap pointer over a sector and press this button
    to highlight its activator line(s), and vice versa
- Button to **_Teleport to automap pointer_**
- **The _Clear [Automap] Mark_ button now clears the mark under the crosshair when over one**
- **Pressing _Place Mark_ while over a mark changes that mark's color**
- **_Show Thing Hitboxes_** setting
- **Keys now flash when flashing of keyed doors is enabled**
- **Color for trigger lines**, used with IDDT (CFG-only: `mapcolor_trig`)

## Enemies

- **_Extra Gibbing_** setting, to force gibbing under certain conditions [i.b. Smooth Doom]
  - Configurable through the CFG-only `extra_gibbing_#` CVARs
- **_Bloodier Gibbing_** setting [i.b. Smooth Doom]
  - Number of splats determined by the CFG-only `bloodier_gibbing_splats` CVAR
- **_Toss Items Upon Death_** setting [i.b. ZDoom, Doom Retro]

## Doom Compatibility settings

All of these are CFG-only, so their CVAR names are included.

- Bruiser attack doesn't face target (`comp_bruistarget`)
- Disable melee snapping (`comp_nomeleesnap`)
- Double autoaim range (`comp_longautoaim`)
- Fix Lost Soul colliding with items (`comp_lscollision`)
- Lost Soul forgets target upon impact (`comp_lsamnesia`)
- Fuzzy things bleed fuzzy blood (`comp_fuzzyblood`) [i.b. Crispy Doom]
- Non-bleeders don't bleed when crushed (`comp_nonbleeders`) [i.b. Crispy Doom]
- Attackers face fuzzy targets straight (`comp_faceshadow`)
- Fix lopsided Icon of Sin explosions (`comp_iosdeath`)
- Permanent IDCHOPPERS invulnerability (`comp_choppers`)
- Manually toggled moving doors are silent (`comp_manualdoor`) [p.f. Crispy Doom]
- Corrected switch sound source (`comp_switchsource`) [p.f. Crispy Doom]
- Chaingun makes two sounds with one bullet (`comp_cgundblsnd`)
- Chaingunner uses pistol/chaingun sound (`comp_cgunnersfx`)
- Arch-Vile fire plays flame start sound (`comp_flamst`) [p.f. Crispy Doom]
- Play `DSNOWAY` instead of `DSOOF` when failing to use key-locked triggers (`comp_keynoway`)
- Higher god-mode face priority (`comp_godface`) [p.f. International Doom]
- Dead players can still play "oof" sound (`comp_deadoof`) [p.f. Crispy Doom]
- Use improved powerup run-out effect (`comp_powerrunout`)
- Use unused pain/bonus palettes (`comp_unusedpals`)
- Key pickup resets palette (`comp_keypal`)

## Cheats

- **`FULLCLIP`** for infinite ammo
- **`VALIANT`** for fast weapons [i.b. ZDoom]
  - **`BOBBERS`** serves as a shortcut to toggle the two above cheats, and `IDFA`
- **`GIBBERS`** to force gibbing on dying enemies, independently of damage dealt
- **`RIOTMODE`** cheat, to make enemies attack all sentient entities
- **`IDFLY`** to fly (uses jumping/crouching buttons) [i.b. PrBoom+, ZDoom]
- **`SUMMON`** to spawn an actor based on its type index [i.b. ZDoom, PrBoomX]
- **`IDDF`** to find a key in the automap
- **`IDDET`** to find exits in the automap
- **`RESURRECT / IDRES`** to resurrect the player without toggling `IDDQD` [i.b. ZDoom]
- **`LINETARGET`** to give some info on the player's linetarget [i.b. ZDoom]
- **`TRAILS`** to show hitscan trails
- **`MDK`** to perform a hitscan attack of 1-million damage [i.b. ZDoom]
- **`SAITAMA`** to enable the MDK Fist
- **`BOOMCAN`** for explosive hitscan attacks
- **`NEXTMAP`** to exit the level [i.b. ZDoom]
- **`NEXTSECRET`** to exit the level as if using a secret exit [i.b. ZDoom]
- **`TURBO`** to change the player speed in-game
- **`TNTEM`** as an alternative to `KILLEM`
- **`FPS`** as a replacement for `SHOWFPS`
- **Mid-air control while in noclipping mode** [p.f. Crispy Doom]
- **Key-binding for Computer Area Map cheat**
- Reenabled **`NOMOMENTUM`** cheat [p.f. Crispy Doom]

For a complete list with more details, see the _New Nugget Doom cheats_ section of `docs/cheats.md`.

## Miscellaneous

- **Customizable skill level**, supporting all vanilla settings and a new one for duplicate monster spawns
  - Its menu item uses the `M_CSTSKL` graphic if found
- **SDL render driver** setting (CFG-only: `sdl_renderdriver`) [p.f. Woof! 14.0.0]
- **Setting of savegame and screenshot paths in config file** (CFG-only: `savegame_dir` and `screenshot_dir`)
- **Keep palette changes in screenshots** setting (CFG-only: `screenshot_palette`)
- **Organize screenshots** setting (CFG-only: `organize_screenshots`)
- **Intermission ratio stats** setting, to use ratios for the stats on the intermission screen (CFG-only: `inter_ratio_stats`) [i.b. Heretic]
- Setting to **increase the duration of the "Entering" state in Doom 2's intermission screen** (CFG-only: `inter_entering_delay`)
  - When enabled, said state can also be accelerated like previous states by pressing the _Fire_ or _Use_ buttons
- **When dying with freelook enabled, the camera is pitched towards the killer**
- **Extended character cast** [partially p.f. Crispy Doom]
  - _Turn_ buttons to rotate
  - _Run_ button to gib
  - _Strafe_ buttons to skip
  - Press `0` to toggle fancy mode
- **Support for high-resolution sprites between `HI_START`/`HI_END` markers**
- **Support for the DeHackEd thing `Scale` property**, as featured in ZDoom and derivatives
  - Does not work with voxel models
- **Support for optional sounds:** [partially p.f. Crispy Doom]
  - Jumping: `DSPLJUMP`
  - Landing: `DSPLLAND`
  - Key-locked door: `DSLOCKED`
  - Key pickup: `DSKEYUP`
  - Key blinking on HUD: `DSKEYBNK`
  - Menus: `DSMNUOPN`, `DSMNUCLS`, `DSMNUACT`, `DSMNUBAK`, `DSMNUMOV`, `DSMNUSLI`, `DSMNUERR`
  - Intermission: `DSINTTIC`, `DSINTTOT`, `DSINTNEX`, `DSINTNET`, `DSINTDMS` (last three unused)
  - Health-based player-pain sounds: `DSPPAI25`, `DSPPAI50`, `DSPPAI75`, `DSPPA100` [i.b. ZDoom]
  - Slow motion: `DSNGSLON`, `DSNGSLOF`
- **Support for lowercase characters in console font** (CFG-only: `hud_menu_allow_lowercase`)
- **Customizable darkening level for dark menu background and Automap overlay**
  (CFG-only: `menu_background_darkening` and `automap_overlay_darkening`) [i.b. Cherry Doom]
- The **Chaingun and Chaingunner can be given a custom sound effect** by providing a `DSCHGUN` sound effect lump
  - This does not apply to HacX, as it provides a dummy, incongruous lump for the sound
- Toggle to **disable the Killough-face easter egg** (CFG-only: `no_killough_face`)
- Toggle to **allow chat in singleplayer** (CFG-only: `sp_chat`)
- Toggle to **use palette colors exactly when gamma correction is disabled** (CFG-only: `gamma_off_fix`)
  - In vanilla Doom, RGB channels are always passed through a gamma table, regardless of the gamma correction setting;
    the table used when gamma correction is disabled makes channels with a value below 128 be incremented by one,
    so e.g. `rgb(0, 64, 128)` from a `PLAYPAL` palette is rendered as `rgb(1, 65, 128)` in-game;
    many source ports, including this one, retain this behavior, but this setting allows to disable it
- **FPS-counter update time** setting (CFG-only: `fps_counter_update_time`)
- Made `force_flip_pan` affect the _OpenAL 3D_ sound module [by _ceski_]
- Restored menu items for various Woof! settings
- Restored `-cdrom` command-line parameter
- Decreased minimum window size to 200p (240p with aspect-ratio correction)
- Setting to enable developer features (CFG-only: `nugget_devmode`)
