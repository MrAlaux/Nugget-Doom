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
//      Cheat sequence checking.
//
//-----------------------------------------------------------------------------

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "am_map.h"
#include "d_deh.h" // Ty 03/27/98 - externalized strings
#include "d_event.h"
#include "d_player.h"
#include "d_think.h"
#include "doomdata.h"
#include "doomdef.h"
#include "doomstat.h"
#include "g_game.h"
#include "g_umapinfo.h"
#include "info.h"
#include "m_cheat.h"
#include "m_array.h"
#include "m_fixed.h"
#include "m_input.h"
#include "m_misc.h"
#include "mn_menu.h"
#include "p_action.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "p_spec.h" // SPECHITS
#include "p_tick.h"
#include "r_defs.h"
#include "r_main.h"
#include "r_state.h"
#include "s_sound.h"
#include "sounds.h"
#include "st_widgets.h"
#include "tables.h"
#include "w_wad.h"
#include "ws_stuff.h"

// [Nugget]
#include "am_map.h"
#include "f_finale.h"
#include "r_main.h"

#define plyr (players+consoleplayer)     /* the console player */

// [Nugget] Testing cheat /------------

//#define NUGMAGIC

#ifdef NUGMAGIC

int nugmagic = 0;

static void cheat_magic(void)
{
  
}

int nugmagic2 = 0;

static void cheat_magic2(void)
{
  
}

#endif

// [Nugget] --------------------------/

//-----------------------------------------------------------------------------
//
// CHEAT SEQUENCE PACKAGE
//
//-----------------------------------------------------------------------------

static void cheat_mus(char *buf);
static void cheat_choppers(void);
static void cheat_god(void);
static void cheat_fa(void);
static void cheat_k(void);
static void cheat_kfa(void);
static void cheat_noclip(void);
static void cheat_pw(int pw);
static void cheat_behold(void);
static void cheat_clev(char *buf);
static void cheat_clev0(void);
static void cheat_mypos(void);
static void cheat_comp(char *buf);
static void cheat_comp0(void);
static void cheat_skill(char *buf);
static void cheat_skill0(void);
static void cheat_friction(void);
static void cheat_pushers(void);
static void cheat_tran(void);
static void cheat_massacre(void);
static void cheat_ddt(void);
static void cheat_hom(void);
static void cheat_fast(void);
static void cheat_key(void);
static void cheat_keyx(void);
static void cheat_keyxx(int key);
static void cheat_weap(void);
static void cheat_weapx(char *buf);
static void cheat_ammo(void);
static void cheat_ammox(char *buf);
static void cheat_smart(void);
static void cheat_pitch(void);
static void cheat_nuke(void);
static void cheat_rate(void);
static void cheat_buddha(void);
static void cheat_spechits(void);
static void cheat_notarget(void);
static void cheat_freeze(void);
static void cheat_health(void);
static void cheat_megaarmour(void);
static void cheat_reveal_secret(void);
static void cheat_reveal_kill(void);
static void cheat_reveal_item(void);

static void cheat_autoaim(void);      // killough 7/19/98
static void cheat_tst(void);
static void cheat_showfps(void); // [FG] FPS counter widget
static void cheat_speed(void);

// [Nugget] /-----------------------------------------------------------------

static void cheat_nomomentum(void);
static void cheat_infammo(void);    // Infinite ammo cheat
static void cheat_fastweaps(void);  // Fast weapons cheat
static void cheat_bobbers(void);    // Shortcut to the two cheats above

boolean gibbers;                 // Used for 'GIBBERS'
static void cheat_gibbers(void); // Everything gibs

static void cheat_riotmode(void);
static void cheat_resurrect(void);
static void cheat_fly(void);
static void cheat_normalexit(void); // Emulate normal level exit
static void cheat_secretexit(void); // Emulate secret level exit
static void cheat_turbo(char *buf);

// Summon a mobj
static void cheat_summon(void);
// Enemy
static void cheat_summone0(void);
static void cheat_summone(char *buf);
// Friend
static void cheat_summonf0(void);
static void cheat_summonf(char *buf);
// Repeat last
static void cheat_summonr(void);
static int spawneetype = -1;
static boolean spawneefriend;

static void cheat_reveal_key(void);
static void cheat_reveal_keyx(void);
static void cheat_reveal_keyxx(int key);

static void cheat_reveal_exit(void);
static void cheat_linetarget(void);  // Give info on the current linetarget
static void cheat_trails(void);      // Show hitscan trails
static void cheat_mdk(void);         // Inspired by ZDoom's console command
static void cheat_saitama(void);     // MDK Fist
static void cheat_boomcan(void);     // Explosive hitscan

static void cheat_fauxdemo(void); // Emulates demo/net-play state, for debugging
static void cheat_dimlight(void);
static void cheat_castcall(void);

static void cheat_cheese(void);
static void cheat_flakes(void);

boolean idgaf;
static void cheat_idgaf(void);

// [Nugget] -----------------------------------------------------------------/

//-----------------------------------------------------------------------------
//
// List of cheat codes, functions, and special argument indicators.
//
// The first argument is the cheat code.
//
// The second argument is its DEH name, or NULL if it's not supported by -deh.
//
// The third argument is a combination of the bitmasks:
// {always, not_dm, not_coop, not_net, not_menu, not_demo, not_deh, beta_only},
// which excludes the cheat during certain modes of play.
//
// The fourth argument is the handler function.
//
// The fifth argument is passed to the handler function if it's non-negative;
// if negative, then its negative indicates the number of extra characters
// expected after the cheat code, which are passed to the handler function
// via a pointer to a buffer (after folding any letters to lowercase).
//
//-----------------------------------------------------------------------------

struct cheat_s cheat[] = {
  {"idmus",      "Change music",      always,
   {.s = cheat_mus}, -2 },

  {"idchoppers", "Chainsaw",          not_net | not_demo,
   {.v = cheat_choppers} },

  {"iddqd",      "God mode",          not_net | not_demo,
   {.v = cheat_god} },

  {"buddha",     "Buddha mode",       not_net | not_demo,
   {.v = cheat_buddha} },

  {"idk",        NULL,                not_net | not_demo | not_deh,
   {.v = cheat_k} }, // The most controversial cheat code in Doom history!!!

  {"idkfa",      "Ammo & Keys",       not_net | not_demo,
   {.v = cheat_kfa} },

  {"idfa",       "Ammo",              not_net | not_demo,
   {.v = cheat_fa} },

  {"idspispopd", "No Clipping 1",     not_net | not_demo,
   {.v = cheat_noclip} },

  {"idclip",     "No Clipping 2",     not_net | not_demo,
   {.v = cheat_noclip} },

  {"idbeholdo",  NULL,                not_net | not_demo | not_deh,
   {.i = cheat_pw}, NUMPOWERS }, // [FG] disable all powerups at once

  {"idbeholdh",  "Health",            not_net | not_demo,
   {.v = cheat_health} },

  {"idbeholdm",  "Mega Armor",        not_net | not_demo,
   {.v = cheat_megaarmour} },

  {"idbeholdv",  "Invincibility",     not_net | not_demo,
   {.i = cheat_pw}, pw_invulnerability },

  {"idbeholds",  "Berserk",           not_net | not_demo,
   {.i = cheat_pw}, pw_strength },

  {"idbeholdi",  "Invisibility",      not_net | not_demo,  
   {.i = cheat_pw}, pw_invisibility },

  {"idbeholdr",  "Radiation Suit",    not_net | not_demo,
   {.i = cheat_pw}, pw_ironfeet },

  {"idbeholda",  "Auto-map",          not_dm,
   {.i = cheat_pw}, pw_allmap },

  {"idbeholdl",  "Lite-Amp Goggles",  not_dm,
   {.i = cheat_pw}, pw_infrared },

  {"idbehold",   "BEHOLD menu",       not_net | not_demo,
   {.v = cheat_behold} },

  {"idclev",     "Level Warp",        not_net | not_demo | not_menu,
   {.s = cheat_clev}, -2 },

  {"idclev",     "Level Warp",        not_net | not_demo | not_menu,
   {.v = cheat_clev0} },

  {"idmypos",    "Player Position",   not_dm, // [FG] not_net | not_demo,
   {.v = cheat_mypos} },

  {"comp",    NULL,                   not_net | not_demo | not_menu,
   {.s = cheat_comp}, -2 },

  {"comp",    NULL,                   not_net | not_demo | not_menu,
   {.v = cheat_comp0} },

  {"skill",    NULL,                  not_net | not_demo | not_menu,
   {.s = cheat_skill}, -1 },

  {"skill",    NULL,                  not_net | not_demo | not_menu,
   {.v = cheat_skill0} },

  {"killem",     NULL,                not_net | not_demo,
   {.v = cheat_massacre} },   // jff 2/01/98 kill all monsters

  {"tntem",     NULL,                not_net | not_demo,
   {cheat_massacre} },     // [Nugget] 'KILLEM' alternative

  {"spechits",     NULL,              not_net | not_demo,
   {.v = cheat_spechits} },

  {"notarget",   "Notarget mode",     not_net | not_demo,
   {.v = cheat_notarget} },

  {"freeze",     "Freeze",            not_net | not_demo,
   {.v = cheat_freeze} },

  {"iddt",       "Map cheat",         not_dm,
   {.v = cheat_ddt} },        // killough 2/07/98: moved from am_map.c

  {"iddst",      NULL,                not_dm,
   {.v = cheat_reveal_secret} },

  {"iddkt",      NULL,                not_dm,
   {.v = cheat_reveal_kill} },

  {"iddit",      NULL,                not_dm,
   {.v = cheat_reveal_item} },

  {"hom",     NULL,                   always,
   {.v = cheat_hom} },        // killough 2/07/98: HOM autodetector

  {"key",     NULL,                   not_net | not_demo, 
   {.v = cheat_key} },     // killough 2/16/98: generalized key cheats

  {"keyr",    NULL,                   not_net | not_demo,
   {.v = cheat_keyx} },

  {"keyy",    NULL,                   not_net | not_demo,
   {.v = cheat_keyx} },

  {"keyb",    NULL,                   not_net | not_demo,
   {.v = cheat_keyx} },

  {"keyrc",   NULL,                   not_net | not_demo, 
   {.i = cheat_keyxx}, it_redcard },

  {"keyyc",   NULL,                   not_net | not_demo,
   {.i = cheat_keyxx}, it_yellowcard },

  {"keybc",   NULL,                   not_net | not_demo, 
   {.i = cheat_keyxx}, it_bluecard },

  {"keyrs",   NULL,                   not_net | not_demo,
   {.i = cheat_keyxx}, it_redskull },

  {"keyys",   NULL,                   not_net | not_demo,
   {.i = cheat_keyxx}, it_yellowskull },

  {"keybs",   NULL,                   not_net | not_demo,
   {.i = cheat_keyxx}, it_blueskull }, // killough 2/16/98: end generalized keys

  {"weap",    NULL,                   not_net | not_demo,
   {.v = cheat_weap} },    // killough 2/16/98: generalized weapon cheats

  {"weap",    NULL,                   not_net | not_demo,
   {.s = cheat_weapx}, -1 },

  {"ammo",    NULL,                   not_net | not_demo,
   {.v = cheat_ammo} },

  {"ammo",    NULL,                   not_net | not_demo,
   {.s = cheat_ammox}, -1 }, // killough 2/16/98: end generalized weapons

  {"tran",    NULL,                   always,
   {.v = cheat_tran} },    // invoke translucency         // phares

  {"smart",   NULL,                   not_net | not_demo,
   {.v = cheat_smart} },      // killough 2/21/98: smart monster toggle

  {"pitch",   NULL,                   always,
   {.v = cheat_pitch} },      // killough 2/21/98: pitched sound toggle

  // killough 2/21/98: reduce RSI injury by adding simpler alias sequences:
  {"mbfran",     NULL,                always, 
   {.v = cheat_tran} },    // killough 2/21/98: same as mbftran

  {"fast",    NULL,                   not_net | not_demo,
   {.v = cheat_fast} },       // killough 3/6/98: -fast toggle

  {"ice",     NULL,                   not_net | not_demo,
   {.v = cheat_friction} },   // phares 3/10/98: toggle variable friction effects

  {"push",    NULL,                   not_net | not_demo, 
   {.v = cheat_pushers} },    // phares 3/10/98: toggle pushers

  {"nuke",    NULL,                   not_net | not_demo,
   {.v = cheat_nuke} },       // killough 12/98: disable nukage damage

  {"rate",    NULL,                   always,
   {.v = cheat_rate} },

  {"aim",        NULL,                not_net | not_demo | beta_only,
   {.v = cheat_autoaim} },

  {"eek",        NULL,                not_dm  | not_demo | beta_only,
   {.v = cheat_ddt} },        // killough 2/07/98: moved from am_map.c

  {"amo",        NULL,                not_net | not_demo | beta_only,
   {.v = cheat_kfa} },

  {"tst",        NULL,                not_net | not_demo | beta_only,
   {.v = cheat_tst} },

  {"nc",         NULL,                not_net | not_demo | beta_only,
   {.v = cheat_noclip} },

// [FG] FPS counter widget
// [Nugget] Change to just "fps"
  {"fps",    NULL,                always,
   {.v = cheat_showfps} },

  {"speed",      NULL,                not_dm,
   {.v = cheat_speed} },

  // [Nugget] /---------------------------------------------------------------

  {"nomomentum", NULL, not_net | not_demo, {.v = cheat_nomomentum}     },
  {"fullclip",   NULL, not_net | not_demo, {.v = cheat_infammo}        }, // Infinite ammo cheat
  {"valiant",    NULL, not_net | not_demo, {.v = cheat_fastweaps}      }, // Fast weapons cheat
  {"bobbers",    NULL, not_net | not_demo, {.v = cheat_bobbers}        }, // Shortcut for the two above cheats
  {"gibbers",    NULL, not_net | not_demo, {.v = cheat_gibbers}        }, // Everything gibs
  {"riotmode",   NULL, not_net | not_demo, {.v = cheat_riotmode}       },
  {"resurrect",  NULL, not_net | not_demo, {.v = cheat_resurrect}      },
  {"idres",      NULL, not_net | not_demo, {.v = cheat_resurrect}      }, // 'RESURRECT' alternative
  {"idfly",      NULL, not_net | not_demo, {.v = cheat_fly}            },
  {"nextmap",    NULL, not_net | not_demo, {.v = cheat_normalexit}     },
  {"nextsecret", NULL, not_net | not_demo, {.v = cheat_secretexit}     },
  {"turbo",      NULL, not_net | not_demo, {.s = cheat_turbo},      -3 },

  {"summon",  NULL, not_net | not_demo, {.v = cheat_summon}      }, // Summon "Menu"
  {"summone", NULL, not_net | not_demo, {.v = cheat_summone0}    }, // Summon Enemy "Menu"
  {"summone", NULL, not_net | not_demo, {.s = cheat_summone}, -3 }, // Summon a hostile mobj
  {"summonf", NULL, not_net | not_demo, {.v = cheat_summonf0}    }, // Summon Friend "Menu"
  {"summonf", NULL, not_net | not_demo, {.s = cheat_summonf}, -3 }, // Summon a friendly mobj
  {"summonr", NULL, not_net | not_demo, {.v = cheat_summonr}     }, // Repeat last summon

  {"iddf",   NULL, not_net | not_demo, {.v = cheat_reveal_key}      },
  {"iddfb",  NULL, not_net | not_demo, {.v = cheat_reveal_keyx}     },
  {"iddfy",  NULL, not_net | not_demo, {.v = cheat_reveal_keyx}     },
  {"iddfr",  NULL, not_net | not_demo, {.v = cheat_reveal_keyx}     },
  {"iddfbc", NULL, not_net | not_demo, {.i = cheat_reveal_keyxx}, 0 },
  {"iddfyc", NULL, not_net | not_demo, {.i = cheat_reveal_keyxx}, 2 },
  {"iddfrc", NULL, not_net | not_demo, {.i = cheat_reveal_keyxx}, 1 },
  {"iddfbs", NULL, not_net | not_demo, {.i = cheat_reveal_keyxx}, 5 },
  {"iddfys", NULL, not_net | not_demo, {.i = cheat_reveal_keyxx}, 3 },
  {"iddfrs", NULL, not_net | not_demo, {.i = cheat_reveal_keyxx}, 4 },

  {"iddet",      NULL, not_net | not_demo, {.v = cheat_reveal_exit} }, // Exit finder
  {"linetarget", NULL, not_net | not_demo, {.v = cheat_linetarget}  }, // Give info on the current linetarget
  {"trails",     NULL, not_net | not_demo, {.v = cheat_trails}      }, // Show hitscan trails
  {"mdk",        NULL, not_net | not_demo, {.v = cheat_mdk}         },
  {"saitama",    NULL, not_net | not_demo, {.v = cheat_saitama}     }, // MDK Fist
  {"boomcan",    NULL, not_net | not_demo, {.v = cheat_boomcan}     }, // Explosive hitscan

  {"fauxdemo",   NULL, not_net | not_demo, {.v = cheat_fauxdemo} }, // Emulates demo/net-play state, for debugging
  {"dimlight",   NULL, not_net | not_demo, {.v = cheat_dimlight} },
  {"castcall",   NULL, not_net | not_demo, {.v = cheat_castcall} },

  {"cheese",     NULL, not_net | not_demo, {.v = cheat_cheese} },
  {"flakes",     NULL, not_net | not_demo, {.v = cheat_flakes} },
  {"idgaf",      NULL, not_net | not_demo, {.v = cheat_idgaf}  },

  #ifdef NUGMAGIC

  {"ggg", NULL, 0, {.v = cheat_magic}},
  {"hhh", NULL, 0, {.v = cheat_magic2}},

  #endif

  // [Nugget] ---------------------------------------------------------------/

  {NULL}                 // end-of-list marker
};

//-----------------------------------------------------------------------------

// [Nugget] /=================================================================

static void cheat_nomomentum(void)
{
  plyr->cheats ^= CF_NOMOMENTUM;
  displaymsg("No Momentum Mode %s", (plyr->cheats & CF_NOMOMENTUM) ? "ON" : "OFF");
}

// Infinite ammo
static void cheat_infammo(void)
{
  plyr->cheats ^= CF_INFAMMO;
  displaymsg("Infinite Ammo %s", (plyr->cheats & CF_INFAMMO) ? "ON" : "OFF");
}

// Fast weapons
static void cheat_fastweaps(void)
{
  plyr->cheats ^= CF_FASTWEAPS;
  displaymsg("Fast Weapons %s", (plyr->cheats & CF_FASTWEAPS) ? "ON" : "OFF");
}

// Shortcut for the two above cheats
static void cheat_bobbers(void)
{
  if (!(plyr->cheats & CF_INFAMMO) || !(plyr->cheats & CF_FASTWEAPS))
  {
    cheat_fa();
    plyr->cheats |= CF_INFAMMO;
    plyr->cheats |= CF_FASTWEAPS;
  }
  else {
    plyr->cheats ^= CF_INFAMMO;
    plyr->cheats ^= CF_FASTWEAPS;
  }

  displaymsg("Yippee Ki Yay!");
}

// Everything gibs
static void cheat_gibbers(void)
{
  gibbers = !gibbers;
  displaymsg("%s", gibbers ? "Ludicrous Gibs!" : "Ludicrous Gibs no more.");
}

static void cheat_riotmode(void)
{
  displaymsg("Riot Mode %s", (riotmode = !riotmode) ? "ON" : "OFF");
}

// Resurrection --------------------------------------------------------------

// Factored out from `cheat_god()`
static void DoResurrect(void)
{
  signed int an;
  mapthing_t mt = {0};

  P_MapStart();
  mt.x = plyr->mo->x >> FRACBITS;
  mt.y = plyr->mo->y >> FRACBITS;
  mt.angle = (plyr->mo->angle + ANG45/2)*(uint64_t)45/ANG45;
  mt.type = consoleplayer + 1;
  P_SpawnPlayer(&mt);

  // [crispy] spawn a teleport fog
  an = plyr->mo->angle >> ANGLETOFINESHIFT;
  P_SpawnMobj(plyr->mo->x+20*finecosine[an], plyr->mo->y+20*finesine[an], plyr->mo->z, MT_TFOG);
  S_StartSoundEx(plyr->mo, sfx_slop);
  P_MapEnd();

  // Fix reviving as "zombie" if god mode was already enabled
  if (plyr->mo)
    plyr->mo->health = god_health;  // Ty 03/09/98 - deh
  plyr->health = god_health;

  // [Nugget] Rewind;
  // This is called before the countdown decrement in `G_Ticker()`,
  // so add 1 to keep it aligned
  if (leveltime)
  { G_SetRewindCountdown(((rewind_interval * TICRATE) + 1) - ((leveltime - 1) % (rewind_interval * TICRATE))); }
}

// Resurrection cheat adapted from Crispy's IDDQD
static void cheat_resurrect(void)
{
  // [crispy] dead players are first respawned at the current position
  if (plyr->playerstate == PST_DEAD)
  {
    DoResurrect();

    // [Nugget] Announce
    displaymsg("Resurrected!");
  }
  else { displaymsg("Still alive."); }
}

// ---------------------------------------------------------------------------

static void cheat_fly(void)
{
  plyr->cheats ^= CF_FLY;

  if (plyr->cheats & CF_FLY) { plyr->mo->flags |=  MF_NOGRAVITY; }
  else                       { plyr->mo->flags &= ~MF_NOGRAVITY; }

  displaymsg("Fly Mode %s", (plyr->cheats & CF_FLY) ? "ON" : "OFF");
}

static void cheat_normalexit(void)
{
  G_ExitLevel();
}

static void cheat_secretexit(void)
{
  G_SecretExitLevel();
}

static void cheat_turbo(char *buf)
{
  int scale = 200;

  if (!isdigit(buf[0]) || !isdigit(buf[1]) || !isdigit(buf[2]))
  {
    displaymsg("Turbo: Digits only");
    return;
  }

  scale = (buf[0]-'0')*100 + (buf[1]-'0')*10 + buf[2]-'0';

  // Limit the scale; it gets kinda wonky at 255 already,
  // but going any further outright inverts movement
  scale = BETWEEN(10, 255, scale);

  displaymsg("Turbo Scale: %i%%", scale);
  forwardmove[0] = 0x19 * scale / 100;
  forwardmove[1] = 0x32 * scale / 100;
     sidemove[0] = 0x18 * scale / 100;
     sidemove[1] = 0x28 * scale / 100;
}

// Summoning -----------------------------------------------------------------

static void cheat_summon(void)
{
  if (spawneetype == -1)
  { displaymsg("Summon: Enemy or Friend?"); }
  else
  { displaymsg("Summon: Enemy, Friend or Repeat last (%i)?", spawneetype); }
}

static boolean GetMobjType(char *buf)
{
  int type;

  if (!isdigit(buf[0]) || !isdigit(buf[1]) || !isdigit(buf[2]))
  {
    displaymsg("Summon: Digits only");
    return false;
  }

  type = (buf[0]-'0')*100 + (buf[1]-'0')*10 + buf[2]-'0';

  const spritenum_t spritenum = states[mobjinfo[type].spawnstate].sprite;
  const long            frame = states[mobjinfo[type].spawnstate].frame;

  // Sanity checks
  if (
    // In case it somehow happens
    type < 0
    // May cause issues once spawned
    || type == MT_MUSICSOURCE
    // May be missing assets
    || ((unsigned) spritenum >= num_sprites)
    || ((frame & FF_FRAMEMASK) >= sprites[spritenum].numframes)
    // May be uninitialized
    || num_mobj_types <= type
  ) {
    displaymsg("Summon: Cannot summon mobj %i", type);
    return false;
  }

  spawneetype = type;

  return true;
}

static void SummonMobj(boolean friendly)
{
  fixed_t x, y, z;
  mobj_t *spawnee;

  extern void AM_Coordinates(const mobj_t *, fixed_t *, fixed_t *, fixed_t *);

  if (spawneetype == -1) {
    displaymsg("You must summon a mobj first!");
    return;
  }

  spawneefriend = friendly;

  P_MapStart();

  if (automapactive == AM_FULL && !followplayer)
  {
    const int oldcoords = map_point_coord;

    map_point_coord = true;
    AM_Coordinates(plyr->mo, &x, &y, &z);

    map_point_coord = oldcoords;
  }
  else {
    x = plyr->mo->x + FixedMul((64*FRACUNIT) + mobjinfo[spawneetype].radius,
                               finecosine[plyr->mo->angle >> ANGLETOFINESHIFT]);

    y = plyr->mo->y + FixedMul((64*FRACUNIT) + mobjinfo[spawneetype].radius,
                               finesine[plyr->mo->angle >> ANGLETOFINESHIFT]);

    z = plyr->mo->z + 32*FRACUNIT;
  }

  spawnee = P_SpawnMobj(x, y, z, spawneetype);

  spawnee->angle = plyr->mo->angle;

  if (spawneefriend) { spawnee->flags |= MF_FRIEND; }

  P_MapEnd();

  displaymsg("Mobj summoned! (%s - Type = %i)",
             spawneefriend ? "Friend" : "Enemy", spawneetype);
}

static void cheat_summone0(void)
{
  displaymsg("Summon Enemy: Enter mobj index");
}

// Summon a hostile mobj
static void cheat_summone(char *buf)
{
  if (GetMobjType(buf)) { SummonMobj(false); }
}

static void cheat_summonf0(void)
{
  displaymsg("Summon Friend: Enter mobj index");
}

// Summon a friendly mobj
static void cheat_summonf(char *buf)
{
  if (GetMobjType(buf)) { SummonMobj(true); }
}

// Summon the last summoned mobj
static void cheat_summonr(void)
{
  SummonMobj(spawneefriend);
}

// Key finder ----------------------------------------------------------------

static void cheat_reveal_key(void)
{
  if (automapactive != AM_FULL) { return; }

  displaymsg("Key Finder: Red, Yellow or Blue?");
}

static void cheat_reveal_keyx(void)
{
  if (automapactive != AM_FULL) { return; }

  displaymsg("Key Finder: Card or Skull?");
}

static void cheat_reveal_keyxx(int key)
{
  if (automapactive != AM_FULL) { return; }

  static int last_count;
  static mobj_t *last_mobj;

  // If the thinkers have been wiped, addresses are invalid
  if (last_count != init_thinkers_count)
  {
    last_count = init_thinkers_count;
    last_mobj = NULL;
  }

  thinker_t *th, *start_th;

  if (last_mobj) { th = &(last_mobj->thinker); }
  else           { th = &thinkercap; }

  start_th = th;

  boolean found = false;

  do {
    th = th->next;

    if (th->function.p1 == (actionf_p1) P_MobjThinker)
    {
      mobj_t *mobj = (mobj_t *) th;

      if (mobj->type == MT_MISC4 + key)
      {
        found = true;
        followplayer = false;
        AM_SetMapCenter(mobj->x, mobj->y);
        P_SetTarget(&last_mobj, mobj);
        break;
      }
    }
  } while (th != start_th);

  if (!found) { displaymsg("Key Finder: key not found"); }
}

// ---------------------------------------------------------------------------

// Exit finder
static void cheat_reveal_exit(void)
{
  if (automapactive != AM_FULL) { return; }

  static int last_exit_line = -1;
  int i, start_i;

  i = last_exit_line + 1;

  if (i >= numlines) { i = 0; }

  start_i = i;

  // (found_exit & 1) == normal
  // (found_exit & 2) == secret
  // Can be both
  int found_exit = 0;

  do {
    const line_t *const line = &lines[i];
    const short special = line->special;

    if (   special ==  11 || special ==  51 || special ==  52
        || special == 124 || special == 197 || special == 198)
    {
      found_exit |= 1 + (special == 51 || special == 124 || special == 198);
    }

    for (int j = 0;  j < 2;  j++)
    {
      sector_t *const sector = j ? line->backsector : line->frontsector;

      if (sector && P_IsDeathExit(sector))
      {
        found_exit |= 1 + (   (sector->special & DEATH_MASK)
                           && (sector->special & DAMAGE_MASK) == DAMAGE_MASK);
      }
    }

    if (found_exit)
    {
      followplayer = false;
      AM_SetMapCenter(line->v1->x, line->v1->y);

      displaymsg(
        "Exit Finder: found %s",
          (found_exit == 3) ? "normal and secret exits"
        : (found_exit == 2) ? "secret exit"
        :                     "normal exit"
      );

      last_exit_line = i;
      break;
    }

    if (++i >= numlines) { i = 0; }
  } while (i != start_i);

  if (!found_exit) { displaymsg("Exit Finder: no exits found"); }
}

// Give info on the current `linetarget`
static void cheat_linetarget(void)
{
  plyr->cheats ^= CF_LINETARGET;
  displaymsg("Linetarget Query %s", (plyr->cheats & CF_LINETARGET) ? "ON" : "OFF");
}

// Show hitscan trails
static void cheat_trails(void)
{
  const int value = P_CycleShowHitscanTrails();

  displaymsg(
    "Hitscan Trails: %s",
      (value == 2) ? "All"
    : (value == 1) ? "Bullets Only"
    :                "Off"
  );
}

// 1-million-damage hitscan attack
static void cheat_mdk(void)
{
  fixed_t slope;

  P_MapStart();

  if (vertical_aiming == VERTAIM_DIRECT)
  {
    slope = plyr->slope;
  }
  else {
    slope = P_AimLineAttack(plyr->mo, plyr->mo->angle, 16*64*FRACUNIT * (comp_longautoaim+1), 0);

    if (!linetarget && vertical_aiming == VERTAIM_DIRECTAUTO)
    { slope = plyr->slope; }
  }

  P_LineAttack(plyr->mo, plyr->mo->angle, 32*64*FRACUNIT, slope, 1000000);

  P_MapEnd();

  displaymsg("MDK!");
}

// MDK Fist
static void cheat_saitama(void)
{
  plyr->cheats ^= CF_SAITAMA;
  displaymsg("MDK Fist %s", (plyr->cheats & CF_SAITAMA) ? "ON" : "OFF");
}

// Explosive hitscan
static void cheat_boomcan(void)
{
  plyr->cheats ^= CF_BOOMCAN;
  displaymsg("Explosive Hitscan %s", (plyr->cheats & CF_BOOMCAN) ? "ON" : "OFF");
}

// Developer cheats ----------------------------------------------------------

// Emulates demo/net-play state, for debugging
static void cheat_fauxdemo(void)
{
  if (!nugget_devmode) { return; }

  extern void D_UpdateCasualPlay(void);

  fauxdemo = !fauxdemo;
  D_UpdateCasualPlay();

  S_StartSound(plyr->mo, sfx_tink);
  displaymsg("Fauxdemo %s", fauxdemo ? "ON" : "OFF");
}

static void cheat_dimlight(void)
{
  if (!nugget_devmode) { return; }

  diminishing_lighting = !diminishing_lighting;
  displaymsg("Diminishing Lighting %s", diminishing_lighting ? "ON" : "OFF");
}

static void cheat_castcall(void)
{
  if (!nugget_devmode) { return; }

  F_StartFinale();
  F_StartCast();
}

// ---------------------------------------------------------------------------

static void cheat_cheese(void)
{
  cheese = !cheese;
  displaymsg("%s", cheese ? "cheese :)" : "no cheese :(");
}

static void cheat_flakes(void)
{
  if (!allow_flakes) { return; }

  flakes = !flakes;
}

static void cheat_idgaf(void)
{
  idgaf = !idgaf;
  displaymsg("I %s.", idgaf ? "don't" : "do");
}

// [Nugget] =================================================================/

// [FG] FPS counter widget
static void cheat_showfps(void)
{
  plyr->cheats ^= CF_SHOWFPS;
}

static void cheat_speed(void)
{
  speedometer = (speedometer + 1) % 4;
}

// killough 7/19/98: Autoaiming optional in beta emulation mode
static void cheat_autoaim(void)
{
  displaymsg((autoaim=!autoaim) ?
    "Projectile autoaiming on" : 
    "Projectile autoaiming off");
}

static void cheat_mus(char *buf)
{
  int musnum;
  mapentry_t* entry;
  
  //jff 3/20/98 note: this cheat allowed in netgame/demorecord

  //jff 3/17/98 avoid musnum being negative and crashing
  if (!isdigit(buf[0]) || !isdigit(buf[1]))
    return;

  displaymsg("%s", s_STSTR_MUS); // Ty 03/27/98 - externalized
  
  // First check if we have a mapinfo entry for the requested level.
  if (gamemode == commercial)
    entry = G_LookupMapinfo(1, 10*(buf[0]-'0') + (buf[1]-'0'));
  else
    entry = G_LookupMapinfo(buf[0]-'0', buf[1]-'0');

  if (entry && entry->music[0])
  {
     musnum = W_CheckNumForName(entry->music);

     if (musnum == -1)
        displaymsg("%s", s_STSTR_NOMUS);
     else
     {
        S_ChangeMusInfoMusic(musnum, 1);
        idmusnum = -1;
     }
     return;
  }

  if (gamemode == commercial)
    {
      musnum = mus_runnin + (buf[0]-'0')*10 + buf[1]-'0' - 1;

      //jff 4/11/98 prevent IDMUS00 in DOOMII and IDMUS36 or greater
      if (musnum < mus_runnin || musnum >= NUMMUSIC)
        displaymsg("%s", s_STSTR_NOMUS); // Ty 03/27/98 - externalized
      else
        {
          S_ChangeMusic(musnum, 1);
          idmusnum = musnum; //jff 3/17/98 remember idmus number for restore
        }
    }
  else
    {
      musnum = mus_e1m1 + (buf[0]-'1')*9 + (buf[1]-'1');

      //jff 4/11/98 prevent IDMUS0x IDMUSx0 in DOOMI and greater than introa
      if (musnum < mus_e1m1 || musnum >= mus_runnin)
        displaymsg("%s", s_STSTR_NOMUS); // Ty 03/27/98 - externalized
      else
        {
          S_ChangeMusic(musnum, 1);
          idmusnum = musnum; //jff 3/17/98 remember idmus number for restore
        }
    }
}

boolean comp_choppers; // [Nugget]

// 'choppers' invulnerability & chainsaw
static void cheat_choppers(void)
{
  plyr->weaponowned[wp_chainsaw] = true;

  // [Nugget]
  if (casual_play && comp_choppers)
    P_GivePower(plyr, pw_invulnerability);
  else
    plyr->powers[pw_invulnerability] = true;

  plyr->bonuscount += 2; // trigger evil grin now
  displaymsg("%s", s_STSTR_CHOPPERS); // Ty 03/27/98 - externalized
}

static void cheat_god(void)
{                                    // 'dqd' cheat for toggleable god mode
  // [crispy] dead players are first respawned at the current position
  if (plyr->playerstate == PST_DEAD)
  {
    DoResurrect(); // [Nugget] Factored out
  }

  plyr->cheats ^= CF_GODMODE;
  if (plyr->cheats & CF_GODMODE)
    {
      if (plyr->mo)
        plyr->mo->health = god_health;  // Ty 03/09/98 - deh

      plyr->health = god_health;
      displaymsg("%s", s_STSTR_DQDON); // Ty 03/27/98 - externalized
    }
  else 
    displaymsg("%s", s_STSTR_DQDOFF); // Ty 03/27/98 - externalized
}

static void cheat_buddha(void)
{
  plyr->cheats ^= CF_BUDDHA;
  if (plyr->cheats & CF_BUDDHA)
    displaymsg("Buddha Mode ON");
  else
    displaymsg("Buddha Mode OFF");
}

static void cheat_notarget(void)
{
  plyr->cheats ^= CF_NOTARGET;

  // [Nugget]: [crispy]
  if (plyr->cheats & CF_NOTARGET)
  {
    // [crispy] let mobjs forget their target and tracer
    for (thinker_t *th = thinkercap.next;  th != &thinkercap;  th = th->next)
    {
      if (th->function.p1 == (actionf_p1) P_MobjThinker)
      {
        mobj_t *const mo = (mobj_t *) th;

        if (mo->target && mo->target->player) { mo->target = NULL; }
        if (mo->tracer && mo->tracer->player) { mo->tracer = NULL; }
      }
    }

    // [crispy] let sectors forget their soundtarget
    for (int i = 0;  i < numsectors;  i++) { sectors[i].soundtarget = NULL; }
  }

  if (plyr->cheats & CF_NOTARGET)
    displaymsg("Notarget ON");
  else
    displaymsg("Notarget OFF");
}

boolean frozen_mode;

static void cheat_freeze(void)
{
  frozen_mode = !frozen_mode;
  if (frozen_mode)
    displaymsg("Freeze ON");
  else
    displaymsg("Freeze OFF");
}

static void cheat_avj(void)
{
  void A_VileJump(mobj_t *mo);

  if (plyr->mo)
    A_VileJump(plyr->mo);
}

// CPhipps - new health and armour cheat codes
static void cheat_health(void)
{
  if (!(plyr->cheats & CF_GODMODE))
  {
    if (plyr->mo)
      plyr->mo->health = mega_health;
    plyr->health = mega_health;
    displaymsg("%s", s_STSTR_BEHOLDX); // Ty 03/27/98 - externalized
  }
}

static void cheat_megaarmour(void)
{
  plyr->armorpoints = idfa_armor;      // Ty 03/09/98 - deh
  plyr->armortype = idfa_armor_class;  // Ty 03/09/98 - deh
  displaymsg("%s", s_STSTR_BEHOLDX); // Ty 03/27/98 - externalized
}

static void cheat_tst(void)
{ // killough 10/98: same as iddqd except for message
  cheat_god();
  displaymsg(plyr->cheats & CF_GODMODE ? "God Mode On" : "God Mode Off");
}

static void cheat_fa(void)
{
  int i;

  if (!plyr->backpack)
    {
      for (i=0 ; i<NUMAMMO ; i++)
        plyr->maxammo[i] *= 2;
      plyr->backpack = true;
    }

  plyr->armorpoints = idfa_armor;      // Ty 03/09/98 - deh
  plyr->armortype = idfa_armor_class;  // Ty 03/09/98 - deh

  // You can't own weapons that aren't in the game // phares 02/27/98
  for (i=0;i<NUMWEAPONS;i++)
    if (!(((i == wp_plasma || i == wp_bfg) && gamemode == shareware) ||
          (i == wp_supershotgun && !ALLOW_SSG)))
      plyr->weaponowned[i] = true;

  for (i=0;i<NUMAMMO;i++)
    if (i!=am_cell || gamemode!=shareware)
      plyr->ammo[i] = plyr->maxammo[i];

  plyr->bonuscount += 2; // trigger evil grin now
  displaymsg("%s", s_STSTR_FAADDED);
}

static void cheat_k(void)
{
  int i;
  for (i=0;i<NUMCARDS;i++)
    if (!plyr->cards[i])     // only print message if at least one key added
      {                      // however, caller may overwrite message anyway
        plyr->cards[i] = true;
        displaymsg("Keys Added");
      }
}

static void cheat_kfa(void)
{
  cheat_k();
  cheat_fa();
  displaymsg("%s", s_STSTR_KFAADDED);
}

static void cheat_noclip(void)
{
  // Simplified, accepting both "noclip" and "idspispopd".
  // no clipping mode cheat

  displaymsg("%s", (plyr->cheats ^= CF_NOCLIP) & CF_NOCLIP ? 
    s_STSTR_NCON : s_STSTR_NCOFF); // Ty 03/27/98 - externalized
}

// 'behold?' power-up cheats (modified for infinite duration -- killough)
static void cheat_pw(int pw)
{
  // [Nugget] Freecam
  if (pw == pw_infrared && R_GetFreecamOn())
  {
    viewplayer->fixedcolormap = !viewplayer->fixedcolormap;
    return;
  }

  if (pw == NUMPOWERS)
  {
    memset(plyr->powers, 0, sizeof(plyr->powers));
    plyr->mo->flags &= ~MF_SHADOW; // [crispy] cancel invisibility
  }
  else
  if (plyr->powers[pw])
    plyr->powers[pw] = pw!=pw_strength && pw!=pw_allmap;  // killough
  else
    {
      P_GivePower(plyr, pw);
      if (pw != pw_strength && !comp[comp_infcheat])
        plyr->powers[pw] = -1;      // infinite duration -- killough
    }
  displaymsg("%s", s_STSTR_BEHOLDX); // Ty 03/27/98 - externalized
}

// 'behold' power-up menu
static void cheat_behold(void)
{
  displaymsg("%s", s_STSTR_BEHOLD); // Ty 03/27/98 - externalized
}

// 'clev' change-level cheat
static void cheat_clev0(void)
{
  int epsd, map;
  char *cur, *next;

  cur = M_StringDuplicate(MapName(gameepisode, gamemap));

  G_GotoNextLevel(&epsd, &map);
  next = MapName(epsd, map);

  if (W_CheckNumForName(next) != -1)
    displaymsg("Current: %s, Next: %s", cur, next);
  else
    displaymsg("Current: %s", cur);

  free(cur);
}

static void cheat_clev(char *buf)
{
  int epsd, map;
  mapentry_t* entry;

  if (gamemode == commercial)
  {
    epsd = 1; //jff was 0, but espd is 1-based
    map = (buf[0] - '0')*10 + buf[1] - '0';
  }
  else
  {
    epsd = buf[0] - '0';
    map = buf[1] - '0';
  }

  // catch non-numerical input
  if (epsd < 0 || epsd > 9 || map < 0 || map > 99)
    return;

  // First check if we have a mapinfo entry for the requested level.
  // If this is present the remaining checks should be skipped.
  entry = G_LookupMapinfo(epsd, map);
  if (!entry)
  {
    char *next;

    // Chex.exe always warps to episode 1.
    if (gameversion == exe_chex)
    {
      epsd = 1;
    }

    next = MapName(epsd, map);

    if (W_CheckNumForName(next) == -1)
    {
      // [Alaux] Restart map with IDCLEV00
      if ((epsd == 0 && map == 0) || (gamemode == commercial && map == 0))
      {
        epsd = gameepisode;
        map = gamemap;
      }
      else
      {
        displaymsg("IDCLEV target not found: %s", next);
        return;
      }
    }
  }

  // So be it.

  idmusnum = -1; //jff 3/17/98 revert to normal level music on IDCLEV

  displaymsg("%s", s_STSTR_CLEV); // Ty 03/27/98 - externalized

  G_DeferedInitNew(gameskill, epsd, map);
}

// 'mypos' for player position
// killough 2/7/98: simplified using dprintf and made output more user-friendly
static void cheat_mypos(void)
{
  plyr->cheats ^= CF_MAPCOORDS;
  if ((plyr->cheats & CF_MAPCOORDS) == 0)
    plyr->message = "";
}

void cheat_mypos_print(void)
{
  displaymsg("X=%.10f Y=%.10f A=%-.0f",
          (double)players[consoleplayer].mo->x / FRACUNIT,
          (double)players[consoleplayer].mo->y / FRACUNIT,
          players[consoleplayer].mo->angle * (90.0/ANG90));
}

// compatibility cheat

static void cheat_comp0(void)
{
  displaymsg("Complevel: %s", G_GetCurrentComplevelName());
}

static void cheat_comp(char *buf)
{
  int new_demover;

  buf[2] = '\0';

  if (buf[0] == '0')
    buf++;

  new_demover = G_GetNamedComplevel(buf);

  if (new_demover != -1)
  {
    demo_version = new_demover;
    G_ReloadDefaults(true);
    displaymsg("New Complevel: %s", G_GetCurrentComplevelName());
  }
}

// variable friction cheat
static void cheat_friction(void)
{
  displaymsg(            // Ty 03/27/98 - *not* externalized
    (variable_friction = !variable_friction) ? "Variable Friction enabled" : 
                                               "Variable Friction disabled");
}

static void cheat_skill0(void)
{
  displaymsg("Skill: %s", default_skill_strings[gameskill + 1]);
}

static void cheat_skill(char *buf)
{
  int skill = buf[0] - '0';

  if (skill >= 1 && skill <= 5 + casual_play) // [Nugget] Custom Skill
  {
    gameskill = skill - 1;
    displaymsg("Next Level Skill: %s", default_skill_strings[gameskill + 1]);

    G_SetSkillParms(gameskill); // [Nugget]
  }
}

// Pusher cheat
// phares 3/10/98
static void cheat_pushers(void)
{
  displaymsg(           // Ty 03/27/98 - *not* externalized
    (allow_pushers = !allow_pushers) ? "Pushers enabled" : "Pushers disabled");
}

// translucency cheat
static void cheat_tran(void)
{
  displaymsg(            // Ty 03/27/98 - *not* externalized
    (translucency = !translucency) ? "Translucency enabled" : "Translucency disabled");
}

static void cheat_massacre(void)    // jff 2/01/98 kill all monsters
{
  // jff 02/01/98 'em' cheat - kill all monsters
  // partially taken from Chi's .46 port
  //
  // killough 2/7/98: cleaned up code and changed to use dprintf;
  // fixed lost soul bug (LSs left behind when PEs are killed)

  int killcount=0;
  thinker_t *currentthinker=&thinkercap;
  // killough 7/20/98: kill friendly monsters only if no others to kill
  int mask = MF_FRIEND;

  // [Nugget] /---------------------------------------------------------------

  // Temporarily disable Bloodier Gibbing if enabled;
  // it's too much to handle on maps with many monsters
  const int oldgibbing = bloodier_gibbing;
  bloodier_gibbing = false;

  complete_milestones |= MILESTONE_KILLS; // Don't announce

  // [Nugget] ---------------------------------------------------------------/

  P_MapStart();
  do
    while ((currentthinker=currentthinker->next)!=&thinkercap)
      if (currentthinker->function.p1 == (actionf_p1)P_MobjThinker &&
	  !(((mobj_t *) currentthinker)->flags & mask) && // killough 7/20/98
	  (((mobj_t *) currentthinker)->flags & MF_COUNTKILL ||
	   ((mobj_t *) currentthinker)->type == MT_SKULL))
	{ // killough 3/6/98: kill even if PE is dead
	  if (((mobj_t *) currentthinker)->health > 0)
	    {
	      killcount++;
	      P_DamageMobj((mobj_t *) currentthinker, NULL, NULL, 10000);
	    }
	  if (((mobj_t *) currentthinker)->type == MT_PAIN)
	    {
	      A_PainDie((mobj_t *) currentthinker);    // killough 2/8/98
	      P_SetMobjState((mobj_t *) currentthinker, S_PAIN_DIE6);
	    }
	}
  while (!killcount && mask ? mask=0, 1 : 0);  // killough 7/20/98
  P_MapEnd();
  // killough 3/22/98: make more intelligent about plural
  // Ty 03/27/98 - string(s) *not* externalized
  displaymsg("%d Monster%s Killed", killcount, killcount==1 ? "" : "s");

  bloodier_gibbing = oldgibbing; // [Nugget]
}

static void cheat_spechits(void)
{
  int i, speciallines = 0;
  boolean origcards[NUMCARDS];
  line_t dummy;
  boolean trigger_keen = true;

  // [crispy] temporarily give all keys
  for (i = 0; i < NUMCARDS; i++)
  {
    origcards[i] = plyr->cards[i];
    plyr->cards[i] = true;
  }

  P_MapStart();

  for (i = 0; i < numlines; i++)
  {
    if (lines[i].special)
    {
      switch (lines[i].special)
        // [crispy] do not trigger level exit switches/lines
        case 11:
        case 51:
        case 52:
        case 124:
        case 197:
        case 198:
        // [crispy] do not trigger teleporters switches/lines
        case 39:
        case 97:
        case 125:
        case 126:
        case 174:
        case 195:
        // [FG] do not trigger silent teleporters
        case 207:
        case 208:
        case 209:
        case 210:
        case 268:
        case 269:
        {
          continue;
        }

      // [crispy] special without tag --> DR linedef type
      // do not change door direction if it is already moving
      if (lines[i].tag == 0 &&
          lines[i].sidenum[1] != NO_INDEX &&
         (sides[lines[i].sidenum[1]].sector->floordata ||
          sides[lines[i].sidenum[1]].sector->ceilingdata))
      {
        continue;
      }

      P_CrossSpecialLine(&lines[i], 0, plyr->mo, false);
      P_ShootSpecialLine(plyr->mo, &lines[i]);
      P_UseSpecialLine(plyr->mo, &lines[i], 0, false);

      speciallines++;
    }
  }

  for (i = 0; i < NUMCARDS; i++)
  {
    plyr->cards[i] = origcards[i];
  }

  if (gamemapinfo && array_size(gamemapinfo->bossactions))
  {
    thinker_t *th;

    for (th = thinkercap.next ; th != &thinkercap ; th = th->next)
    {
      if (th->function.p1 == (actionf_p1)P_MobjThinker)
      {
        mobj_t *mo = (mobj_t *) th;

        bossaction_t *bossaction;
        array_foreach(bossaction, gamemapinfo->bossactions)
        {
          if (bossaction->type == mo->type)
          {
            dummy = *lines;
            dummy.special = (short)bossaction->special;
            dummy.tag = (short)bossaction->tag;
            // use special semantics for line activation to block problem types.
            if (!P_UseSpecialLine(mo, &dummy, 0, true))
              P_CrossSpecialLine(&dummy, 0, mo, true);

            speciallines++;

            if (dummy.tag == 666)
              trigger_keen = false;
          }
        }
      }
    }
  }
  else
  {
    // [crispy] trigger tag 666/667 events
    if (gamemode == commercial)
    {
      if (gamemap == 7)
      {
        // Mancubi
        dummy.tag = 666;
        speciallines += EV_DoFloor(&dummy, lowerFloorToLowest);
        trigger_keen = false;

        // Arachnotrons
        dummy.tag = 667;
        speciallines += EV_DoFloor(&dummy, raiseToTexture);
      }
    }
    else
    {
      if (gameepisode == 1)
      {
        // Barons of Hell
        dummy.tag = 666;
        speciallines += EV_DoFloor(&dummy, lowerFloorToLowest);
        trigger_keen = false;
      }
      else if (gameepisode == 4)
      {
        if (gamemap == 6)
        {
          // Cyberdemons
          dummy.tag = 666;
          speciallines += EV_DoDoor(&dummy, blazeOpen);
          trigger_keen = false;
        }
        else if (gamemap == 8)
        {
          // Spider Masterminds
          dummy.tag = 666;
          speciallines += EV_DoFloor(&dummy, lowerFloorToLowest);
          trigger_keen = false;
        }
      }
    }
  }

  // Keens (no matter which level they are on)
  if (trigger_keen)
  {
    dummy.tag = 666;
    speciallines += EV_DoDoor(&dummy, doorOpen);
  }

  P_MapEnd();

  displaymsg("%d Special Action%s Triggered", speciallines, speciallines == 1 ? "" : "s");
}

// killough 2/7/98: move iddt cheat from am_map.c to here
// killough 3/26/98: emulate Doom better
static void cheat_ddt(void)
{
  if (automapactive)
    ddt_cheating = (ddt_cheating+1) % 3;
}

static void cheat_reveal_secret(void)
{
  static int last_secret = -1;

  if (automapactive == AM_FULL)
  {
    int i, start_i;

    i = last_secret + 1;
    if (i >= numsectors)
      i = 0;
    start_i = i;

    do
    {
      sector_t *sec = &sectors[i];

      if (P_IsSecret(sec))
      {
        followplayer = false;

        // This is probably not necessary
        if (sec->lines && sec->lines[0] && sec->lines[0]->v1)
        {
          AM_SetMapCenter(sec->lines[0]->v1->x, sec->lines[0]->v1->y);
          last_secret = i;
          break;
        }
      }

      i++;
      if (i >= numsectors)
        i = 0;
    } while (i != start_i);
  }
}

static void cheat_cycle_mobj(mobj_t **last_mobj, int *last_count,
                             int flags, int alive)
{
  thinker_t *th, *start_th;

  // If the thinkers have been wiped, addresses are invalid
  if (*last_count != init_thinkers_count)
  {
    *last_count = init_thinkers_count;
    *last_mobj = NULL;
  }

  if (*last_mobj)
    th = &(*last_mobj)->thinker;
  else
    th = &thinkercap;

  start_th = th;

  do
  {
    th = th->next;
    if (th->function.p1 == (actionf_p1)P_MobjThinker)
    {
      mobj_t *mobj;

      mobj = (mobj_t *) th;

      if ((!alive || mobj->health > 0) && mobj->flags & flags)
      {
        followplayer = false;
        AM_SetMapCenter(mobj->x, mobj->y);
        P_SetTarget(last_mobj, mobj);
        break;
      }
    }
  } while (th != start_th);
}

static void cheat_reveal_kill(void)
{
  if (automapactive == AM_FULL)
  {
    static int last_count;
    static mobj_t *last_mobj;

    cheat_cycle_mobj(&last_mobj, &last_count, MF_COUNTKILL, true);
  }
}

static void cheat_reveal_item(void)
{
  if (automapactive == AM_FULL)
  {
    static int last_count;
    static mobj_t *last_mobj;

    cheat_cycle_mobj(&last_mobj, &last_count, MF_COUNTITEM, false);
  }
}

// killough 2/7/98: HOM autodetection
static void cheat_hom(void)
{
  displaymsg((autodetect_hom = !autodetect_hom) ? "HOM Detection On" :
    "HOM Detection Off");
}

// killough 3/6/98: -fast parameter toggle
static void cheat_fast(void)
{
  // [Nugget] Custom Skill
  if (gameskill == sk_custom)
  {
    displaymsg((fastmonsters = !fastmonsters) ? "Fast Monsters On" : "Fast Monsters Off");
    G_SetFastParms(fastmonsters);
    return;
  }

  displaymsg((fastparm = !fastparm) ? "Fast Monsters On" : 
    "Fast Monsters Off");  // Ty 03/27/98 - *not* externalized
  G_SetFastParms(fastparm); // killough 4/10/98: set -fast parameter correctly

  fastmonsters = fastparm; // [Nugget]
}

// killough 2/16/98: keycard/skullkey cheat functions
static void cheat_key(void)
{
  displaymsg("Red, Yellow, Blue");  // Ty 03/27/98 - *not* externalized
}

static void cheat_keyx(void)
{
  displaymsg("Card, Skull");        // Ty 03/27/98 - *not* externalized
}

static void cheat_keyxx(int key)
{
  displaymsg((plyr->cards[key] = !plyr->cards[key]) ? 
    "Key Added" : "Key Removed");  // Ty 03/27/98 - *not* externalized
}

// killough 2/16/98: generalized weapon cheats

static void cheat_weap(void)
{                                   // Ty 03/27/98 - *not* externalized
  displaymsg(ALLOW_SSG ? // killough 2/28/98
    "Weapon number 1-9" : "Weapon number 1-8");
}

static void cheat_weapx(char *buf)
{
  int w = *buf - '1';

  if ((w==wp_supershotgun && !ALLOW_SSG) ||      // killough 2/28/98
      ((w==wp_bfg || w==wp_plasma) && gamemode==shareware))
    return;

  if (w==wp_fist)           // make '1' apply beserker strength toggle
    cheat_pw(pw_strength);
  else
    if (w >= 0 && w < NUMWEAPONS)
    {
      if ((plyr->weaponowned[w] = !plyr->weaponowned[w]))
      {
        plyr->bonuscount += 2; // trigger evil grin now
        displaymsg("Weapon Added");  // Ty 03/27/98 - *not* externalized
      }
      else 
        {
          displaymsg("Weapon Removed"); // Ty 03/27/98 - *not* externalized
          if (w==plyr->readyweapon)         // maybe switch if weapon removed
            plyr->pendingweapon = P_SwitchWeapon(plyr);
        }
    }
}

// killough 2/16/98: generalized ammo cheats
static void cheat_ammo(void)
{
  displaymsg("Ammo 1-4, Backpack");  // Ty 03/27/98 - *not* externalized
}

static void cheat_ammox(char *buf)
{
  int a = *buf - '1';
  if (*buf == 'b')  // Ty 03/27/98 - strings *not* externalized
    if ((plyr->backpack = !plyr->backpack))
      for (displaymsg("Backpack Added"),   a=0 ; a<NUMAMMO ; a++)
        plyr->maxammo[a] <<= 1;
    else
      for (displaymsg("Backpack Removed"), a=0 ; a<NUMAMMO ; a++)
        {
          if (plyr->ammo[a] > (plyr->maxammo[a] >>= 1))
            plyr->ammo[a] = plyr->maxammo[a];
        }
  else
    if (a>=0 && a<NUMAMMO)  // Ty 03/27/98 - *not* externalized
      { // killough 5/5/98: switch plasma and rockets for now -- KLUDGE
        a = a==am_cell ? am_misl : a==am_misl ? am_cell : a;  // HACK
        if ((plyr->ammo[a] = !plyr->ammo[a]))
        {
          plyr->ammo[a] = plyr->maxammo[a];
          displaymsg("Ammo Added");
        }
        else
          displaymsg("Ammo Removed");
      }
}

static void cheat_smart(void)
{
  displaymsg((monsters_remember = !monsters_remember) ? 
    "Smart Monsters Enabled" : "Smart Monsters Disabled");
}

static void cheat_pitch(void)
{
  displaymsg((pitched_sounds = !pitched_sounds) ? "Pitch Effects Enabled" :
    "Pitch Effects Disabled");
}

static void cheat_nuke(void)
{
  displaymsg((disable_nuke = !disable_nuke) ? "Nukage Disabled" :
    "Nukage Enabled");
}

static void cheat_rate(void)
{
  plyr->cheats ^= CF_RENDERSTATS;
}

//-----------------------------------------------------------------------------
// 2/7/98: Cheat detection rewritten by Lee Killough, to avoid
// scrambling and to use a more general table-driven approach.
//-----------------------------------------------------------------------------

static boolean M_CheatAllowed(cheat_when_t when)
{
  return
    !(when & not_dm   && deathmatch && !demoplayback) &&
    !(when & not_coop && netgame && !deathmatch) &&
    !(when & not_demo && (demorecording || demoplayback)) &&
    !(when & not_menu && menuactive) &&
    !(when & beta_only && !beta_emulation);
}

#define CHEAT_ARGS_MAX 8  /* Maximum number of args at end of cheats */

boolean M_FindCheats(int key)
{
  static uint64_t sr;
  static char argbuf[CHEAT_ARGS_MAX+1], *arg;
  static int init, argsleft, cht;
  int i, ret, matchedbefore;

  // If we are expecting arguments to a cheat
  // (e.g. idclev), put them in the arg buffer

  if (argsleft)
    {
      *arg++ = M_ToLower(key);             // store key in arg buffer
      if (!--argsleft)                   // if last key in arg list,
        cheat[cht].func.s(argbuf);       // process the arg buffer
      return 1;                          // affirmative response
    }

  key = M_ToLower(key) - 'a';
  if (key < 0 || key >= 32)              // ignore most non-alpha cheat letters
    {
      sr = 0;        // clear shift register
      return 0;
    }

  if (!init)                             // initialize aux entries of table
    {
      init = 1;
      for (i=0;cheat[i].cheat;i++)
        {
          uint64_t c=0, m=0;
          const char *p; // [FG] char!
          for (p=cheat[i].cheat; *p; p++)
            {
              unsigned key = M_ToLower(*p)-'a';  // convert to 0-31
              if (key >= 32)            // ignore most non-alpha cheat letters
                continue;
              c = (c<<5) + key;         // shift key into code
              m = (m<<5) + 31;          // shift 1's into mask
            }
          cheat[i].code = c;            // code for this cheat key
          cheat[i].mask = m;            // mask for this cheat key
        }
    }

  sr = (sr<<5) + key;                   // shift this key into shift register

#if 0
  {signed/*long*/volatile/*double *x,*y;*/static/*const*/int/*double*/i;/**/char/*(*)*/*D_DoomExeName/*(int)*/(void)/*?*/;(void/*)^x*/)((/*sr|1024*/32767/*|8%key*/&sr)-19891||/*isupper(c*/strcasecmp/*)*/("b"/*"'%2d!"*/"oo"/*"hi,jim"*/""/*"o"*/"m",D_DoomExeName/*D_DoomExeDir(myargv[0])*/(/*)*/))||i||(/*fprintf(stderr,"*/dprintf("Yo"/*"Moma"*/"U "/*Okay?*/"mUSt"/*for(you;read;tHis){/_*/" be a "/*MAN! Re-*/"member"/*That.*/" TO uSe"/*x++*/" t"/*(x%y)+5*/"HiS "/*"Life"*/"cHe"/*"eze"**/"aT"),i/*+--*/++/*;&^*/));}
#endif

  for (matchedbefore = ret = i = 0; cheat[i].cheat; i++)
    if ((sr & cheat[i].mask) == cheat[i].code &&  // if match found & allowed
        M_CheatAllowed(cheat[i].when) &&
        !(cheat[i].when & not_deh  && cheat[i].deh_modified))
    {
      if (cheat[i].arg < 0)               // if additional args are required
        {
          cht = i;                        // remember this cheat code
          arg = argbuf;                   // point to start of arg buffer
          argsleft = -cheat[i].arg;       // number of args expected
          ret = 1;                        // responder has eaten key
        }
      else
        if (!matchedbefore)               // allow only one cheat at a time
          {
            matchedbefore = ret = 1;      // responder has eaten key
            cheat[i].func.i(cheat[i].arg); // call cheat handler
          }
    }
  return ret;
}

static const struct {
  int input;
  const cheat_when_t when;
  const cheatf_t func;
  const int arg;
} cheat_input[] = {
  { input_iddqd,     not_net|not_demo, {.v = cheat_god},      0 },
  { input_idkfa,     not_net|not_demo, {.v = cheat_kfa},      0 },
  { input_idfa,      not_net|not_demo, {.v = cheat_fa},       0 },
  { input_idclip,    not_net|not_demo, {.v = cheat_noclip},   0 },
  { input_idbeholdh, not_net|not_demo, {.v = cheat_health},   0 },
  { input_idbeholdm, not_net|not_demo, {.v = cheat_megaarmour}, 0 },
  { input_idbeholdv, not_net|not_demo, {.i = cheat_pw},       pw_invulnerability },
  { input_idbeholds, not_net|not_demo, {.i = cheat_pw},       pw_strength },
  { input_idbeholdi, not_net|not_demo, {.i = cheat_pw},       pw_invisibility },
  { input_idbeholdr, not_net|not_demo, {.i = cheat_pw},       pw_ironfeet },
  { input_idbeholdl, not_dm,           {.i = cheat_pw},       pw_infrared },
  { input_iddt,      not_dm,           {.v = cheat_ddt},      0 },
  { input_notarget,  not_net|not_demo, {.v = cheat_notarget}, 0 },
  { input_freeze,    not_net|not_demo, {.v = cheat_freeze},   0 },
  { input_avj,       not_net|not_demo, {.v = cheat_avj},      0 },

  // [Nugget] ----------------------------------------------------------------

  { input_idbeholda,  not_net|not_demo, {.i = cheat_pw},         pw_allmap },
  { input_infammo,    not_net|not_demo, {.v = cheat_infammo},    0 },
  { input_fastweaps,  not_net|not_demo, {.v = cheat_fastweaps},  0 },
  { input_resurrect,  not_net|not_demo, {.v = cheat_resurrect},  0 },
  { input_fly,        not_net|not_demo, {.v = cheat_fly},        0 },
  { input_summonr,    not_net|not_demo, {.v = cheat_summonr},    0 },
  { input_linetarget, not_net|not_demo, {.v = cheat_linetarget}, 0 },
  { input_mdk,        not_net|not_demo, {.v = cheat_mdk},        0 },
  { input_saitama,    not_net|not_demo, {.v = cheat_saitama},    0 },
  { input_boomcan,    not_net|not_demo, {.v = cheat_boomcan},    0 },
};

boolean M_CheatResponder(event_t *ev)
{
  int i;

  if (strictmode && demorecording)
    return false;

  if (ev->type == ev_keydown && M_FindCheats(ev->data2.i))
    return true;

  if (WS_Override())
    return false;

  for (i = 0; i < arrlen(cheat_input); ++i)
  {
    if (M_InputActivated(cheat_input[i].input))
    {
      if (M_CheatAllowed(cheat_input[i].when))
        cheat_input[i].func.i(cheat_input[i].arg);

      return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------
//
// $Log: m_cheat.c,v $
// Revision 1.7  1998/05/12  12:47:00  phares
// Removed OVER_UNDER code
//
// Revision 1.6  1998/05/07  01:08:11  killough
// Make TNTAMMO ammo ordering more natural
//
// Revision 1.5  1998/05/03  22:10:53  killough
// Cheat engine, moved from st_stuff
//
// Revision 1.4  1998/05/01  14:38:06  killough
// beautification
//
// Revision 1.3  1998/02/09  03:03:05  killough
// Rendered obsolete by st_stuff.c
//
// Revision 1.2  1998/01/26  19:23:44  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:58  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
