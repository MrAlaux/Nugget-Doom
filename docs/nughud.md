# Nugget Doom's NUGHUD guide

**The `NUGHUD` lump** is a variant of MBF's `OPTIONS` lump, **used specifically by Nugget Doom to customize the Crispy HUD**.

As implied, `NUGHUD` uses the same format as `OPTIONS`. Excerpt from `mbfedit.txt`:

```
The OPTIONS lump has the same format as mbf.cfg: A text file listing option
names and values, optionally separated by blank or comment lines.
```

## Loading NUGHUD lumps

**`NUGHUD` lumps can be loaded just like `OPTIONS` lumps**:

- By loading WADs which contain a lump explicitly named `NUGHUD`;
- By including a file explicitly named `nughud.lmp` (case-insensitive) in the corresponding autoload folder;
- By using the `-file` command-line parameter to load a file explicitly named `nughud.#`, where `#` can be any extension name (`.lmp` is recommended).

## NUGHUD widgets

**`NUGHUD` is composed of widgets**, whose behavior is determined by a set of properties.

The following properties are shared across all widgets:

- `_x`: **X position**, which can be any number between `0` and `320` (inclusive).
- `_y`: **Y position**, which can be any number between `0` and `200` (inclusive).
- `_wide`: **Widescreen shift**, with the following possible values:
  - `-2` to shift the element left forcefully;
  - `-1` to shift the element left only when in widescreen mode;
  - ` 0` to keep the element in place regardless of widescreen mode;
  - ` 1` to shift the element right only when in widescreen mode;
  - ` 2` to shift the element right forcefully.

The following types of widgets support special behavior:

- **Disableables**: Can be disabled by setting `_x` to `-1`.
- **Alignables**: Can be aligned horizontally by means of the `_align` property, with the following possible values:
  - `-1` for left alignment;
  - ` 0` for center alignment;
  - ` 1` for right alignment.

The following widgets are available:

| Widget(s)           | Disableable | Alignable | Description |
| :-----------------: | :---------: | :-------: | :---------- |
| `nughud_ammo`       | Yes         | Yes       | Ammo count for the currently-equipped weapon |
| `nughud_ammoicon`   | Yes         | Yes       | Ammo icon, which changes depending on ammo type of current weapon |
| `nughud_ammobar`    | Yes         | Yes       | Ammo bar |
| `nughud_health`     | Yes         | Yes       | Health count |
| `nughud_healthicon` | Yes         | Yes       | Health icon, which changes depending on whether the player has Berserk |
| `nughud_healthbar`  | Yes         | Yes       | Health bar |
| `nughud_arms#`      | Yes         | No        | Arms (weapon) number, where # is a number between `1` and `9` (inclusive) |
| `nughud_frags`      | Yes         | Yes       | Frags count, only shown during Deathmatch games |
| `nughud_face`       | Yes         | No        | Face (Mugshot) |
| `nughud_armor`      | Yes         | Yes       | Armor count |
| `nughud_armoricon`  | Yes         | Yes       | Armor icon, which changes depending on current armor type |
| `nughud_armorbar`   | Yes         | Yes       | Armor bar |
| `nughud_key#`       | Yes         | No        | Key display, where # is a number between `0` and `2` (in order: Blue Key; Yellow Key; Red Key) |
| `nughud_ammo#`      | Yes         | Yes       | Ammo count for each type, where # is a number between `0` and `3` (in order: Bullets; Shells; Cells; Rockets) |
| `nughud_maxammo#`   | Yes         | Yes       | Same as the above, but for Max Ammo |
| `nughud_time`       | No          | Yes       | Time display, only shown if enabled by the user |
| `nughud_sts`        | No          | Yes       | Stats (Kills/Items/Secrets) display, only shown if enabled by the user |
| `nughud_title`      | No          | Yes       | Level Name display, only shown on the Automap |
| `nughud_powers`     | No          | Yes       | Powerup Timers, only shown if enabled by the user |
| `nughud_coord`      | No          | Yes       | Coordinates display, only shown if enabled by the user |
| `nughud_fps`        | No          | Yes       | FPS display, only shown when the FPS cheat is activated |
| `nughud_message`    | No          | Yes       | Message display |
| `nughud_secret`     | No          | Yes       | Secret Message display |

**The _Ammo_, _Health_ and _Armor icons_ can also be aligned vertically** by means of the `_vlign` property, with the following possible values:
  - ` 1` for top alignment;
  - ` 0` for center alignment;
  - `-1` for bottom alignment.
Additionally, **the offsets of the graphics used by these icons will be ignored, unless a custom font is being used** (see details below).

**The _Ammo_, _Health_ and _Armor bars_ support additional properties:**

- `_ups`: Percentage of units per slice (e.g. a value of `200` would make a slice be drawn every 2 units), which can be any number between `100` and `10000`;
- `_gap`: Additional space between slices (kerning), which can be any number between `-4` and `4`.

**Bars also require slice graphics.** Respectively: `NHAMBAR#`, `NHHLBAR#` and `NHARBAR#`, where `#` stands for either `0` or `1`.
If both slice graphics for a widget are provided, a second bar will be drawn on top of the first when the player has extra units (e.g. health over 100%).
Otherwise, if only the first slice graphic is provided, the bars will continue to grow normally.

**Arms number 1 is lit up when the player has Berserk.**

There are some additional toggles (value of `0` or `1`) for some specific widgets:

- `nughud_percents`: Toggle drawing of percentage signs for the _Health_ and _Armor_ counts.
- `nughud_ammoicon_big`: Toggle usage of big ammo pickup sprites for the _Armor icon_.
- `nughud_face_bg`: Toggle the _Face_ background, whose position is linked to that of the _Face_ itself.
- `nughud_time_sts`: Toggle relocation of the _Time_ widget to the position of the _Stats_ widget when the latter is inactive.
- `nughud_sts_ml`: Toggle three-lined _Stats_ widget.
- `nughud_coord_ml`: Toggle three-lined _Coordinates_ widget.

Lastly, the _Message_ widget supports an X position value of `-1` to forcefully draw it at its original X position, where it'll be affected by the Centered Messages setting.

#### Examples

```
; This is a comment!
; Loading a NUGHUD lump with these contents will restore the Face widget.

nughud_face_x 143

; Move the Frags widget elsewhere, since in the default NUGHUD distribution,
; it is drawn right where the Face is drawn in the traditional Status Bar.
nughud_frags_x    314
nughud_frags_y    155
nughud_frags_wide 1
```

```
; Loading a NUGHUD lump with these contents will draw the Level Name display centered.

nughud_title_x     160
nughud_title_wide  0
nughud_title_align 0
```

### Custom fonts

**Certain widgets support custom fonts.**
Graphics for all characters of a given font must be provided for the font to be used, otherwise the default font will be used instead.

The following fonts are available:

```
Tall Numbers, used for the Health, Armor, current-weapon Ammo and Frags counts:

- NHTNUM# -- Number, where # is a number between 0 and 9 (inclusive)
- NHTMINUS - Minus sign
- NHTPRCNT - Percent sign


Current-weapon Ammo Numbers, which take precedence over Tall Numbers for the Current-weapon Ammo count:

- NHRNUM# -- Number, where # is a number between 0 and 9 (inclusive)
- NHRMINUS - Minus sign


Ammo Numbers, used for the Ammo and Max Ammo counts:

- NHAMNUM# - Number, where # is a number between 0 and 9 (inclusive)


Arms Numbers, used for the weapon numbers:

- NHW0NUM# - Weapon unavailable, where # is a number between 1 and 9 (inclusive)
- NHW1NUM# - Weapon available, where # is a number between 1 and 9 (inclusive)


Keys:

- NHKEYS# -- Key, where # is a number between 0 and 8 (inclusive)


Berserk, drawn in place of the Ammo count when using the Berserk Fist:

- NHBERSRK - Berserk graphic


Ammo graphics, used for the Ammo icon widget:

- NHAMMO# - Graphic, where # is a number between 0 and 3 (in order: Bullets; Shells; Cells; Rockets)


Health graphics, used for the Health icon widget:

- NHEALTH# - Graphic, where # is a number between 0 and 1 (respectively, no Berserk and Berserk)


Armor graphics, used for the Armor icon widget:

- NHARMOR# - Graphic, where # is a number between 0 and 2 (in order: no Armor; Green Armor; Blue Armor)


Infinity, drawn in place of the Ammo count when using weapons with no ammo type (e.g. Fist/Chainsaw):

- NHINFNTY - Infinity graphic
```

### Patches

**Patches are static graphics that can be drawn anywhere on the screen.**
Up to 8 patches can be drawn. They are drawn in increasing order; `patch1` is drawn below `patch2`, which is drawn below `patch3`, and so on.
Patches are mostly drawn behind widgets, with the exception of bars: the first 4 patches are drawn behind bars, while the rest are drawn over them.

Aside from the shared properties, **patches make use of an additional property, `_name`, that determines the name of the graphic lump to be used**, which can either be a sprite (i.e. a lump between `S_START` and `S_END` markers, like `MEDIA0`) or a graphic (like `STBAR`).
**Custom lumps CAN be used** (for example, a graphic called `NEWPATCH`). The names used in the `NUGHUD` lump MUST be enclosed between quotation marks.

**Patches are alignable**, both horizontally and vertically, and can be disabled by simply not providing any graphic.

**There is an additional toggle, `nughud_patch_offsets`, that determines whether or not to apply graphic offsets when drawing Patches.**
Disabling this is useful when using non-exclusive graphics (e.g. ammo pickup sprites), whose offsets may differ across PWADs.
Otherwise, enabling it is useful when using exclusive graphics, whose offsets are determined by the HUD maker, to allow precise positioning.

#### Example

```
; Loading a NUGHUD lump with these contents will draw
; the Status Bar graphic in its traditional position.
; It doesn't look right, but it's just an example.

nughud_patch1_x 0
nughud_patch1_y 168
nughud_patch1_wide 0
nughud_patch1_name "STBAR"

; Also draw the Arms box, which is actually a separate graphic.
nughud_patch2_x 104
nughud_patch2_y 168
nughud_patch2_wide 0
nughud_patch2_name "STARMS"
```

### Additional integer properties

- `nughud_weapheight`: **vertical offset for weapon sprites**, in the [-32, 32] range; greater values shift the sprites downwards.
- `nughud_viewoffset`: **vertical offset for the view window**, in the [-16, 16] range; greater values shift the view downwards.

---

By default and compared to the original Crispy HUD, `NUGHUD` hides the face widget and shows Arms numbers `2-9` instead of `2-7`.

**The default `NUGHUD`**, as defined in the executable, **is available in text format as `nughud.lmp`**, found in the `docs/` folder. Comments were added to it for clarity. Feel free to use it as a base to make new HUDs.

It is advised that you do not include values for variables that you do not wish to modify, as to avoid issues if the handling of any of them is altered in the future.
