## Changed Doom cheats

`IDBEHOLD`  
Typing IDBEHOLD will bring up a small menu where you can choose to enable one of various powerup effects. To choose a powerup, use the following keys:  
`R`: Toggles radiation suit  
`I`: Toggles partial invisibility  
`V`: Toggles invulnerability (with inverse colormap)  
`A`: Toggles all-map cheat (reveals the automap)  
`L`: Toggles lite-amp goggles  
`S`: Toggles berserk strength  
`H`: Gives 200% health points  
`M`: Gives megaarmor, with 200% armor points  
`O`: Disables all currently active power-ups  

`IDCLEV00` restarts the current level.

`IDDQD` type after death to resurrect.

`IDMUSx` now works across netgames and is remembered by savegame.

`IDKFA` and `IDFA` now includes backpack.

`IDCLIP` now allows mid-air control.
    
## Boom and MBF cheats

In Boom, many of them had a TNT prefix, which was removed in MBF.

`COMPx`  
Show current [complevel](https://doomwiki.org/wiki/Woof!#Compatibility_modes). If you type a number, the engine will switch to this complevel. Not safe, for full compatibility set complevel in the menu and restart the level.

`KILLEM` / `TNTEM`  
This cheat kills all monsters on the map. If there are no enemy monsters on the map and you type this, it will kill all friends on the map (not including the player, of course).

`FAST`  
Toggles fast monsters.

`SMART`  
Toggles smart monsters (the "remember previous enemy" option in the Enemies menu).

`HOM`  
This cheat toggles the Boom [HOM](https://doomwiki.org/wiki/Hall_of_mirrors_effect) detection feature. Depending on your settings, HOM will appear either as solid red or as a red flashing area when this cheat is enabled.

`KEY`  
Type KEY to bring up a menu, then type `R`, `Y`, or `B` to choose a key color (red, yellow, or blue). Then, type `C` or `S` to select card or skull. Once you make the final selection, the key will either be given to you if you don't have it, or it will be taken away. This lets you select the key you need (especially useful in maps with missing keys).

`IDK`  
Gives all keys.

`WEAP`  
Type WEAP, then type a number (`1-9` in Doom 2, `1-8` in Doom). The selected weapon which is associated with that slot will either be added or removed, depending on whether you have it or not.

`AMMO`  
Type AMMO, then either type a number from `1-4`, or `B` for Backpack. The selected ammo will be added or taken away. Ammo types are bullets, shells, rockets, and cells.

`TRAN` or `MBFRAN`  
This code toggles translucency rendering effects.

`PITCH`  
Toggles pitched sounds (the "pitch-shifted sounds" option in General menu).

`ICE`  
Toggle Boom friction effects.

`PUSH`  
Toggle Boom pusher effects.

`NUKE`  
Toggle the effects of damaging sectors.

## New Woof! cheats

Most of them were taken from ZDoom, Crispy Doom and DSDA-Doom.

`SPECHITS`  
Triggers all [Linedef actions](https://doomwiki.org/wiki/Linedef_type) on a map at once, no matter if they are enabled by pushing, walking over or shooting, or if they require a key. It also triggers all boss monster and Commander Keen actions if possible.

`BUDDHA`  
Makes the player's health unable to drop below 1% from damage taken (toggle on/off).

`NOTARGET`  
Toggle deaf and blind monsters that do not act until attacked.

`FREEZE`  
Stops all monsters, projectiles and item animations, but not the player animations. Additionally allows the player to harmlessly walk through frozen projectile attacks (toggle on/off).

`IDDST`  
Centers the automap around the lowest numbered unfound secret sector. If used again, it will center the automap around the next unfound secret sector, and so on.

`IDDKT`  
Centers the automap around the lowest numbered alive monster that counts towards the kill percentage. If used again, it will center the automap around the next alive monster, and so on.

`IDDIT`  
Centers the automap around the lowest numbered uncollected item that counts towards the item percentage. If used again, it will center the automap around the next uncollected item, and so on.

`SKILL`  
Show (or change) game skill level.

`SHOWFPS`  
Toggle printing the FPS in the upper right corner.

`RATE`  
Toggle the display of rendering stats, including frame rate and the current number of segs, visplanes, and sprites.

`SPEED`
Toggle the speedometer. Repeating the cheat cycles through different units: map units per second, kilometers per hour, and miles per hour.

## New Nugget Doom cheats

Most of them were taken from or inspired by other ports -- see `README.md` for credits.

`FULLCLIP`  
Toggle infinite ammo.

`VALIANT`  
Toggle fast weapons.

`BOBBERS`  
Shortcut to toggle the two cheats mentioned above, and IDFA.

`GIBBERS`  
Force gibbing on dying enemies, independently of damage dealt.

`RIOTMODE`  
Make enemies alerted by the player attack all sentient entities (including other enemies),
and allow entities to damage their same species.

Note that when both the no-target cheat and this one are enabled, enemies will still be alerted normally, but they won't target the player.

`IDFLY`  
Toggle Fly Mode (uses jumping/crouching keys).

`SUMMON[X][Y]`  
Spawn an actor based on its mobj type index, where:

- `X` is either "E" to spawn a hostile actor, "F" to spawn a friendly actor, or "R" to repeat the last spawn.
- `Y` is a three-digit number specifying the type.

For example, to spawn a hostile Shotgun Guy (whose index is 2), the full cheat would be `SUMMONE002`.

Actors are always spawned facing the same direction as the player.
If the Automap is enabled and the Follow Mode is off, the actors will be spawned where the pointer is located.
Otherwise, they will be spawned in front of the player.

`IDDF[C][T]`  
Find a key in the Automap.
This cheat functions similarly to the `KEY` cheat, with `C` and `T` specifying the key color and type respectively.

`IDDET`  
Find exits in the Automap.

`RESURRECT` / `IDRES`  
Resurrect the player without toggling IDDQD.

`TRAILS`  
Enable visualization of hitscan trails.

`LINETARGET`  
Toggle Linetarget Query Mode, which gives some info on the player's linetarget (i.e. the shootable thing being aimed at).

`MDK`  
Perform a hitscan attack of 1-million damage.

`SAITAMA`  
Toggle MDK Fist (replaces A_Punch's melee attack with the MDK attack, featuring an alternate multi-shot attack when holding down Strafe On).

`BOOMCAN`  
Toggle explosive hitscan attacks for the player's ranged hitscan weapons

`NEXTMAP`  
Exit the level.

`NEXTSECRET`  
Exit the level as if using a secret exit.

`TURBO`  
Change the player speed in-game.

`NOMOMENTUM`  
Toggle no-momentum mode (re-enabled debugging cheat).

## Beta cheats

These cheats only work in MBF `-beta` emulation mode.

`AIM`  
Toggles projectile autoaiming.

`TST`  
Equivalent to the `IDDQD` cheat (God mode).

`EEK`  
Equivalent to the `IDDT` cheat (displays entire map).

`AMO`  
Equivalent to the `IDKFA` cheat (gives all weapons, ammo and keys).

`NC`  
Equivalent to the `IDSPISPOPD` cheat (no clipping mode).
