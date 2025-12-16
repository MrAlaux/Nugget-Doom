# Nugget Doom's NUGHUD guide

**The `NUGHUD` lump** is a variant of MBF's `OPTIONS` lump, **used exclusively by Nugget Doom to customize the fullscreen HUD**
used when the `screenblocks` CVAR is set to the maximum size minus one (usually 12) and the _Use NUGHUD_ setting is enabled.

From this point onwards, _NUGHUD_ refers to the customization system in general, whereas _`NUGHUD`_ refers to a NUGHUD lump.

As implied, NUGHUD uses the same format as `OPTIONS`. Excerpt from `mbfedit.txt`:

```
The OPTIONS lump has the same format as mbf.cfg: A text file listing option
names and values, optionally separated by blank or comment lines.
```

NUGHUD mostly serves as an interface for SBARDEF, with simpler (albeit verbose) syntax and some exclusive features.
Unlike SBARDEF, however, **NUGHUD works by applying modifications to a built-in `NUGHUD`**,
meaning that **it is not necessary to redefine all properties in new `NUGHUD`s for them to work**.

## Loading `NUGHUD` lumps

**`NUGHUD` lumps can be loaded just like `OPTIONS` lumps**:

- By loading WADs which contain a lump explicitly named `NUGHUD`.
- By including a file explicitly named `nughud.lmp` (case-insensitive) in the corresponding autoload folder.
- By using the `-file` command-line parameter to load a file explicitly named `nughud.#`,
  where `#` can be any extension name (`.lmp` is recommended).

## NUGHUD widgets

**NUGHUD is composed of widgets**, whose behavior is determined by a set of properties.

**The following properties are shared across all widgets:**

- `_x`: **X position**, which can be any integer in the [0, 320] range.
- `_y`: **Y position**, which can be any integer in the [0, 200] range.
- `_wide`: **Widescreen shift**, with the following possible values:
  - `-2` to shift the element left forcefully.
  - `-1` to shift the element left only when in widescreen mode.
  - ` 0` to keep the element in place regardless of widescreen mode.
  - ` 1` to shift the element right only when in widescreen mode.
  - ` 2` to shift the element right forcefully.

**Some widgets are alignable**, by means of the following properties:

- `_align`: horizontal alignment, with the following possible values:
  - `-1` for left alignment.
  - ` 0` for center alignment.
  - ` 1` for right alignment.
- `_vlign`: vertical alignment, with the following possible values:
  - ` 1` for top alignment.
  - ` 0` for center alignment.
  - `-1` for bottom alignment.
  

## Status-bar widgets

The following status-bar widgets are available:

| Widget(s)           | Alignable | Description |
| :-----------------: | :-------: | :---------- |
| `nughud_ammo`       | Yes       | Ammo count for the currently equipped weapon |
| `nughud_ammoicon`   | Yes       | Ammo icon, which changes depending on the ammo type of the current weapon |
| `nughud_ammobar`    | Yes       | Ammo bar |
| `nughud_weaponicon` | Yes       | Weapon icon, which changes depending on the current weapon |
| `nughud_health`     | Yes       | Health count |
| `nughud_healthicon` | Yes       | Health icon, which changes depending on whether the player has Berserk |
| `nughud_healthbar`  | Yes       | Health bar |
| `nughud_arms#`      | No        | Arms (weapon) number, where `#` is an integer in the [1, 9] range |
| `nughud_frags`      | Yes       | Frag count, only shown during Deathmatch games |
| `nughud_face`       | No        | Face (mugshot) |
| `nughud_armor`      | Yes       | Armor count |
| `nughud_armoricon`  | Yes       | Armor icon, which changes depending on the current armor type |
| `nughud_armorbar`   | Yes       | Armor bar |
| `nughud_key#`       | No        | Key display, where `#` is an integer in the [0, 2] range (in order: Blue Key; Yellow Key; Red Key) |
| `nughud_ammo#`      | Yes       | Ammo count for each type, where `#` is an integer in the [0, 3] range (in order: Bullets; Shells; Cells; Rockets) |
| `nughud_maxammo#`   | Yes       | Same as the above, but for Max. Ammo |

**All status-bar widgets can be disabled** by setting `_x` to `-1`.

**The _Ammo_, _Weapon_, _Health_ and _Armor_ icons are vertically alignable.**
Additionally, **the offsets of the graphics used by these icons will be ignored, unless a custom font is being used** (see details below).

**The _Ammo_, _Health_ and _Armor bars_ support additional properties:**

- `_xstep` and `_ystep`: Horizontal and vertical spacing between slices, which can be any integer in the [0, 64] range;
- `_ups`: Percentage of units per slice (e.g. a value of `200` would make a slice be drawn every 2 units), which can be any integer in the [10, 10000] range.

**Bars also require slice graphics.** Respectively: `NHAMBAR#`, `NHHLBAR#` and `NHARBAR#`, where `#` stands for either `0` or `1`.
If both slice graphics for a widget are provided, a second bar will be drawn on top of the first when the player has extra units (e.g. health over 100%).
Otherwise, if only the first slice graphic is provided, the bars will continue to grow normally.
**If both `_xstep` and `_ystep` are 0, the slice graphic's width will be used as the horizontal spacing.**

**Arms #1 is highlighted when the player has Berserk.**
**If Arms #8 is disabled, Arms #3 is highlighted when the player owns either the Shotgun or the Super Shotgun.**

There are some additional toggles (value of `0` or `1`) for some specific widgets:

- `nughud_percents`: Toggle drawing of percentage signs for the _Health_ and _Armor_ counts.
- `nughud_ammoicon_big`: Toggle usage of big-ammo-pickup sprites for the _Ammo_ icon.
- `nughud_face_bg`: Toggle the _Face_ background, whose position is linked to that of the _Face_ itself.

## Text lines

The following text lines are available:

| Widget(s)        | Description |
| :--------------: | :---------- |
| `nughud_time`    | Time display, only shown if enabled by the user |
| `nughud_sts`     | Stats (Kills/Items/Secrets) display, only shown if enabled by the user |
| `nughud_title`   | Level Name display, only shown in the Automap |
| `nughud_powers`  | Powerup Timers, only shown if enabled by the user |
| `nughud_coord`   | Coordinates display, only shown if enabled by the user |
| `nughud_fps`     | FPS display, only shown when the `FPS` cheat is activated |
| `nughud_rate`    | Rendering-statistics display, only shown when the `IDRATE` cheat is activated |
| `nughud_cmd`     | Command-history display, only shown if enabled by the user |
| `nughud_speed`   | Speedometer, only shown when the `SPEED` cheat is activated |
| `nughud_message` | Message and Chat display |
| `nughud_secret`  | Announcements display (secret revealed, level name, etc.) |

**All text lines are horizontally alignable.**

There are some additional toggles -- `nughud_sts_ml` and `nughud_coord_ml` --
that respectively determine whether to draw the Stats and Coordinates display as multiple lines or a single one.

There is an additional toggle, `nughud_message_defx`, to forcefully draw the _Message_ display at its original X position,
where it'll be affected by the _Centered Messages_ setting.

### Stacks

**Text lines can be drawn as part of stacks** by setting both `_x` and `_y` to `-1`.
The following additional properties are used:

- `_stack`: **Index of the stack** that the text line belongs to.
- `_order`: **Order of the text line** within the stack, which can be any integer in the [0, 64] range.

Text lines will inherit the properties of the stack they belong to.

**8 stacks are available**, numbered 1 through 8. **They're referred to as `nughud_stack#`**, where `#` is the stack number.
**Stacks make use of all the shared properties, and are alignable both horizontally and vertically.**

The _Message_ and _Chat_ displays will always be drawn first in whichever stack they're assigned to,
and will also be assigned to the next stack, which is assumed to be on the other side of the screen at the same height,
so as to leave space to prevent overlapping issues due to those displays' potential length.

#### Examples

```
; This is a comment!
; Loading a NUGHUD lump with these contents will restore the Face widget.

nughud_face_x 143

; Move the Frags widget elsewhere, since in the default NUGHUD lump,
; it is drawn right where the Face is drawn in the traditional Status Bar.
nughud_frags_x     314
nughud_frags_y     155
nughud_frags_wide  1
nughud_frags_align 1
```

```
; Loading a NUGHUD lump with these contents will create a bottom-aligned stack on the right side of the screen
; and assign the Level Name and "Secret Revealed" message displays to it.

nughud_stack5_x     318
nughud_stack5_y     168
nughud_stack5_wide  1
nughud_stack5_align 1
nughud_stack5_vlign -1

nughud_title_x     -1
nughud_title_y     -1
nughud_title_stack 5
nughud_title_order 1

nughud_secret_x     -1
nughud_secret_y     -1
nughud_secret_stack 5
nughud_secret_order 0
```

### Custom fonts

**Certain widgets support custom fonts.**
Graphics for all characters of a given font must be provided for the font to be used,
otherwise the default font will be used instead.

The following fonts are available:

```
Tall Numbers, used for the Health, Armor, current-weapon Ammo and Frag counts:

- NHTNUM# -- Number, where # is a number between 0 and 9 (inclusive)
- NHTMINUS - Minus sign
- NHTPRCNT - Percent sign


Current-weapon Ammo Numbers, which take precedence over Tall Numbers for the current-weapon Ammo count:

- NHRNUM# -- Number, where # is a number between 0 and 9 (inclusive)
- NHRMINUS - Minus sign


Ammo Numbers, used for the Ammo and Max. Ammo counts:

- NHAMNUM# - Number, where # is a number between 0 and 9 (inclusive)


Arms Numbers, used for the weapon numbers:

- NHW0NUM# - Weapon unavailable, where # is a number between 1 and 9 (inclusive)
- NHW1NUM# - Weapon available, where # is a number between 1 and 9 (inclusive)


Keys:

- NHKEYS# - Key, where # is a number between 0 and 8 (inclusive)


Berserk, drawn in place of the Ammo count when using the Berserk Fist:

- NHBERSRK - Berserk graphic


Ammo graphics, used for the Ammo icon widget:

- NHAMMO# - Graphic, where # is a number between 0 and 3 (in order: Bullets; Shells; Cells; Rockets)


Weapon graphics, used for the Weapon icon widget:

- NHWEAP# - Graphic, where # is a number between 0 and 8
  (in order: Fist; Pistol; Shotgun; Chaingun; Rocket Launcher; Plasma Rifle; BFG; Chainsaw; SSG)


Health graphics, used for the Health icon widget:

- NHEALTH# - Graphic, where # is a number between 0 and 1 (respectively, no Berserk and Berserk)


Armor graphics, used for the Armor icon widget:

- NHARMOR# - Graphic, where # is a number between 0 and 2 (in order: no Armor; Green Armor; Blue Armor)


Infinity, drawn in place of the Ammo count when using weapons with no ammo type (e.g. Fist/Chainsaw):

- NHINFNTY - Infinity graphic
```

### Patches

**Patches are static graphics that can be drawn anywhere on the screen**, behind the rest of widgets.

Up to 8 patches can be drawn; `patch1` is drawn behind `patch2`, which is drawn behind `patch3`, and so on.
Most widgets are drawn over patches, with the exception of bars: the first 4 patches are drawn behind bars, while the rest are drawn over them.

Aside from the shared properties, **patches make use of an additional property, `_name`, that determines the name of the graphic lump to be used**,
which can be either a sprite (i.e. a lump between `S_START` and `S_END` markers, like `MEDIA0`) or a graphic (like `STBAR`).

**Custom lumps can be used** (for example, a graphic called `NEWPATCH`). The names used in `NUGHUD` must be enclosed between quotation marks.

**Patches are alignable**, both horizontally and vertically, and can be disabled by simply not providing any graphic.

**There is an additional toggle, `nughud_patch_offsets`, that determines whether or not to apply graphic offsets when drawing Patches.**
Disabling this is useful when using non-exclusive graphics (e.g. ammo-pickup sprites), whose offsets may differ across PWADs.
Otherwise, enabling it is useful when using exclusive graphics, whose offsets are determined by the HUD maker, to allow precise positioning.

#### Example

```
; Loading a NUGHUD lump with these contents will draw
; the status-bar graphic in its traditional position.
; It doesn't look right, but it's just an example.

nughud_patch1_x 0
nughud_patch1_y 168
nughud_patch1_wide 0
nughud_patch1_name "STBAR"

; Also draw the Arms box, which is a separate graphic.
nughud_patch2_x 104
nughud_patch2_y 168
nughud_patch2_wide 0
nughud_patch2_name "STARMS"
```

### Status-bar chunks

**Status-bar chunks** are square regions of the status bar, taken from the currently loaded `STBAR`.
Up to 8 chunks can be drawn; they are drawn behind patches, following the same order (`chunk1` before `chunk2`, etc.).

Aside from the shared properties, **chunks make use of the following additional properties**:

- `_sx`: **X origin** (left end, absolute) of the chunk, which can be any integer in the [0, 319] range.
- `_sy`: **Y origin** (top end, absolute) of the chunk, which can be any integer in the [0, 31] range.
- `_sw`: **Width** (right end, relative to left end) of the chunk, which can be any integer in the [1, 320] range.
- `_sh`: **Height** (bottom end, relative to top end) of the chunk, which can be any integer in the [1, 32] range.

Chunks can only be taken from the original 320x200 region of the status bar;
this keeps the feature functional with wide status bars, whose extended borders are ignored.

**Note:** The `STBAR` graphic itself features the _Frags_ widget instead of the _Arms_ widget.
The latter can be drawn as a patch.

### Minimap

Just like with SBARDEF, **the minimap can be customized through NUGHUD**.
Aside from the shared properties, **the minimap makes use of the following additional properties**:

- `_w`: **Width**, which can be any integer in the [32, 96] range.
- `_h`: **Height**, which can be any integer in the [32, 96] range.
- `_undmess`: A boolean which determines if the minimap is **under messages**, in which case its Y position will be affected by the height of the message list.

### Additional integer properties

- `nughud_weapheight`: **vertical offset for weapon sprites**, in the [-32, 32] range; greater values shift the sprites downwards.
- `nughud_viewoffset`: **vertical offset for the view window**, in the [-32, 32] range; greater values shift the view downwards.

---

**The default `NUGHUD` is based on the traditional status bar**, with the addition of showing Arms numbers `2-9` instead of `2-7`.

**Said default `NUGHUD`**, as built into the executable, **is available in text format as `nughud.lmp`**, found in the `docs/` folder.
Comments were added to it for clarity. Feel free to use it as a base to make new `NUGHUD`s.
