//
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// DESCRIPTION:
//      Status bar code.
//      Does the face/direction indicator animatin.
//      Does palette indicators as well (red pain/berserk, bright pickup)
//
//-----------------------------------------------------------------------------

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"
// [Nugget]
#include "d_player.h"
#include "r_defs.h"

// Size of statusbar.
// Now sensitive for scaling.

#define ST_HEIGHT 32
#define ST_WIDTH  ORIGWIDTH
#define ST_Y      (ORIGHEIGHT - ST_HEIGHT)

// [Nugget] Crispy minimalistic HUD
#define CRISPY_HUD      11
#define CRISPY_HUD_WIDE (CRISPY_HUD+1)

// [Nugget] Macros brought over from st_stuff.c

#define ST_FACESX               143
#define ST_FACESY               168

// AMMO number pos.
#define ST_AMMOWIDTH            3
#define ST_AMMOX                44
#define ST_AMMOY                171

// HEALTH number pos.
#define ST_HEALTHWIDTH          3
#define ST_HEALTHX              90
#define ST_HEALTHY              171

// Weapon pos.
#define ST_ARMSX                111
#define ST_ARMSY                172
#define ST_ARMSBGX              104
#define ST_ARMSBGY              168
#define ST_ARMSXSPACE           12
#define ST_ARMSYSPACE           10

// Frags pos.
#define ST_FRAGSX               138
#define ST_FRAGSY               171
#define ST_FRAGSWIDTH           2

// ARMOR number pos.
#define ST_ARMORWIDTH           3
#define ST_ARMORX               221
#define ST_ARMORY               171

// Key icon positions.
#define ST_KEY0WIDTH            8
#define ST_KEY0HEIGHT           5
#define ST_KEY0X                239
#define ST_KEY0Y                171
#define ST_KEY1WIDTH            ST_KEY0WIDTH
#define ST_KEY1X                239
#define ST_KEY1Y                181
#define ST_KEY2WIDTH            ST_KEY0WIDTH
#define ST_KEY2X                239
#define ST_KEY2Y                191

// Ammunition counter.
#define ST_AMMO0WIDTH           3
#define ST_AMMO0HEIGHT          6
#define ST_AMMO0X               288
#define ST_AMMO0Y               173
#define ST_AMMO1WIDTH           ST_AMMO0WIDTH
#define ST_AMMO1X               288
#define ST_AMMO1Y               179
#define ST_AMMO2WIDTH           ST_AMMO0WIDTH
#define ST_AMMO2X               288
#define ST_AMMO2Y               191
#define ST_AMMO3WIDTH           ST_AMMO0WIDTH
#define ST_AMMO3X               288
#define ST_AMMO3Y               185

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH        3
#define ST_MAXAMMO0HEIGHT       5
#define ST_MAXAMMO0X            314
#define ST_MAXAMMO0Y            173
#define ST_MAXAMMO1WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X            314
#define ST_MAXAMMO1Y            179
#define ST_MAXAMMO2WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X            314
#define ST_MAXAMMO2Y            191
#define ST_MAXAMMO3WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X            314
#define ST_MAXAMMO3Y            185


//
// STATUS BAR
//

// Called by main loop.
boolean ST_Responder(event_t* ev);

// Called by main loop.
void ST_Ticker(void);

// Called by main loop.
void ST_Drawer(boolean fullscreen, boolean refresh);

// Called when the console player is spawned on each level.
void ST_Start(void);

// Called by startup code.
void ST_Init(void);
void ST_Warnings(void);

// [crispy] forcefully initialize the status bar backing screen
extern void ST_refreshBackground(boolean force);

// killough 5/2/98: moved from m_misc.c:

// [Alaux]
extern int smooth_counts;
extern int st_health;
extern int st_armor;

extern int health_red;    // health amount less than which status is red
extern int health_yellow; // health amount less than which status is yellow
extern int health_green;  // health amount above is blue, below is green
extern int armor_red;     // armor amount less than which status is red
extern int armor_yellow;  // armor amount less than which status is yellow
extern int armor_green;   // armor amount above is blue, below is green
extern int ammo_red;      // ammo percent less than which status is red
extern int ammo_yellow;   // ammo percent less is yellow more green
extern int sts_always_red;// status numbers do not change colors
extern int sts_pct_always_gray;// status percents do not change colors
extern int sts_traditional_keys;  // display keys the traditional way

// [Nugget]: [crispy] blinking key or skull in the status bar
#define KEYBLINKMASK 0x8
#define KEYBLINKTICS (7*KEYBLINKMASK)
extern void ST_blinkKeys(player_t* player, int blue, int yellow, int red);

extern int hud_backpack_thresholds; // backpack changes thresholds
extern int hud_armor_type; // color of armor depends on type

extern int st_solidbackground;

extern int st_crispyhud; // [Nugget] Included here; now an int

extern patch_t *nhtminus, *nhrminus; // [Nugget]

#endif

//----------------------------------------------------------------------------
//
// $Log: st_stuff.h,v $
// Revision 1.4  1998/05/03  22:50:55  killough
// beautification, move external declarations, remove cheats
//
// Revision 1.3  1998/04/19  01:10:39  killough
// Generalize cheat engine to add deh support
//
// Revision 1.2  1998/01/26  19:27:56  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:04  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
