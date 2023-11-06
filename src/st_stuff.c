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
//
// DESCRIPTION:
//      Status bar code.
//      Does the face/direction indicator animatin.
//      Does palette indicators as well (red pain/berserk, bright pickup)
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "doomstat.h"
#include "m_random.h"
#include "i_video.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "hu_stuff.h" // [FG] hud_displayed
#include "st_lib.h"
#include "r_main.h"
#include "am_map.h"
#include "m_cheat.h"
#include "s_sound.h"
#include "sounds.h"
#include "dstrings.h"
#include "m_misc2.h"
#include "m_swap.h"
#include "i_printf.h"
#include "m_nughud.h" // [Nugget]

// [crispy] immediately redraw status bar after help screens have been shown
extern boolean inhelpscreens;

//
// STATUS BAR DATA
//

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4
// Radiation suit, green shift.
#define RADIATIONPAL            13

// Location of status bar
#define ST_X                    0
#define ST_X2                   104

#define ST_FX                   143
#define ST_FY                   169

// Should be set to patch width
//  for tall numbers later on
#define ST_TALLNUMWIDTH         (tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES         5
#define ST_NUMSTRAIGHTFACES     3
#define ST_NUMTURNFACES         2
#define ST_NUMSPECIALFACES      3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES        2
#define ST_NUMXDTHFACES         9

#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES+ST_NUMXDTHFACES)

#define ST_TURNOFFSET           (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET           (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET       (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET        (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE              (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE             (ST_GODFACE+1)
#define ST_XDTHFACE             (ST_DEADFACE+1)

// [Nugget] Moved face position macros to st_stuff.h so m_nughud.c can access them

#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

#define ST_MUCHPAIN             20

// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?
// I dunno, why don't you go and find out!!!  killough

// [Nugget] Moved widget position macros to st_stuff.h so m_nughud.c can access them

// killough 2/8/98: weapon info position macros UNUSED, removed here

// main player in game
static player_t *plyr;

// ST_Start() has just been called
static boolean st_firsttime;

// lump number for PLAYPAL
static int lu_palette;

// whether left-side main status bar is active
static boolean st_statusbaron;

// [crispy] distinguish classic status bar with background and player face from Crispy HUD
int st_crispyhud; // [Nugget] Now an int
static boolean st_classicstatusbar;
static boolean st_statusbarface; // [Nugget] Face may still be drawn in NUGHUD

void ST_createWidgets(); // [Nugget] Prototype this

// !deathmatch
static boolean st_notdeathmatch;

// !deathmatch && st_statusbaron
static boolean st_armson;

// !deathmatch
static boolean st_fragson;

// main bar left
static patch_t *sbar;

// main bar right, for doom 1.0
static patch_t *sbarr;

// 0-9, tall numbers
static patch_t *tallnum[10];

// tall % sign
static patch_t *tallpercent;

// 0-9, short, yellow (,different!) numbers
static patch_t *shortnum[10];

// 3 key-cards, 3 skulls, 3 card/skull combos
// jff 2/24/98 extend number of patches by three skull/card combos
static patch_t *keys[NUMCARDS+3];

// face status patches
static patch_t *faces[ST_NUMFACES];
static int have_xdthfaces;

// face background
static patch_t *faceback[MAXPLAYERS]; // killough 3/7/98: make array

 // main bar right
static patch_t *armsbg;

// weapon ownership patches
static patch_t *arms[6+3][2]; // [Nugget] Extend array to accomodate 9 numbers

// [Nugget] /----------------

static patch_t *stbersrk;
static int lu_berserk;

static patch_t *stinfnty;

static int nughud_patchlump[NUMNUGHUDPATCHES];

// NUGHUD fonts
static patch_t *nhtnum[10];         // NHTNUM#, from 0 to 9
       patch_t *nhtminus;           // NHTMINUS
static patch_t *nhtprcnt;           // NHTPRCNT
static patch_t *nhrnum[10];         // NHRNUM#, from 0 to 9
       patch_t *nhrminus;           // NHRMINUS
static patch_t *nhamnum[10];        // NHAMNUM#, from 0 to 9
static patch_t *nhwpnum[9][2];      // NHW0NUM# and NHW1NUM#, from 1 to 9
static patch_t *nhkeys[NUMCARDS+3]; // NHKEYS
static patch_t *nhbersrk;           // NHBERSRK
static patch_t *nhammo[4];          // NHAMMO#, from 0 to 3
static patch_t *nhealth[2];         // NHEALTH#, from 0 to 1
static patch_t *nharmor[3];         // NHARMOR#, from 0 to 2
static patch_t *nhinfnty;           // NHINFNTY

// [Nugget] ----------------/

// ready-weapon widget
static st_number_t w_ready;

// [Alaux]
int smooth_counts;
int st_health = 100;
int st_armor = 0;

//jff 2/16/98 status color change levels
int ammo_red;      // ammo percent less than which status is red
int ammo_yellow;   // ammo percent less is yellow more green
int health_red;    // health amount less than which status is red
int health_yellow; // health amount less than which status is yellow
int health_green;  // health amount above is blue, below is green
int armor_red;     // armor amount less than which status is red
int armor_yellow;  // armor amount less than which status is yellow
int armor_green;   // armor amount above is blue, below is green

int hud_backpack_thresholds; // backpack changes thresholds
int hud_armor_type; // color of armor depends on type

 // in deathmatch only, summary of frags stats
static st_number_t w_frags;

// health widget
static st_percent_t w_health;

// weapon ownership widgets
static st_multicon_t w_arms[6+3]; // [Nugget] Extend array to accomodate 9 numbers
static int st_berserk;  // [Nugget] Highlight Arms #1 if the player has Berserk
static int st_shotguns; // [Nugget]: [crispy] show SSG availability in the Shotgun slot of the arms widget

// face status widget
static st_multicon_t w_faces;

// keycard widgets
static st_multicon_t w_keyboxes[3];

// armor widget
static st_percent_t  w_armor;

// ammo widgets
static st_number_t   w_ammo[4];

// max ammo widgets
static st_number_t   w_maxammo[4];

 // number of frags so far in deathmatch
static int      st_fragscount;

// used to use appopriately pained face
static int      st_oldhealth = -1;

// used for evil grin
static boolean  oldweaponsowned[NUMWEAPONS];

 // count until face changes
static int      st_facecount = 0;

// current face index, used by w_faces
static int      st_faceindex = 0;

// holds key-type for each key box on bar
static int      keyboxes[3];

// a random number per tick
static int      st_randomnumber;

extern char     *mapnames[];

//
// STATUS BAR CODE
//

void ST_Stop(void);

int st_solidbackground;

void ST_refreshBackground(boolean force)
{
  if (st_classicstatusbar)
    {
      const int st_x = (SHORT(sbar->width) > ORIGWIDTH && SHORT(sbar->leftoffset) == 0) ?
                       ST_X + (ORIGWIDTH - SHORT(sbar->width)) / 2 :
                       ST_X;

      if (SCREENWIDTH != ST_WIDTH)
      {
        int x, y;
        byte *dest = screens[BG];

        if (st_solidbackground)
        {
          // [FG] calculate average color of the 16px left and right of the status bar
          const int vstep[][2] = {{0, 1}, {1, 2}, {2, ST_HEIGHT}};
          const int hstep = SCREENWIDTH * hires*hires;
          const int lo = MAX(st_x + WIDESCREENDELTA - SHORT(sbar->leftoffset), 0);
          const int w = MIN(SHORT(sbar->width), SCREENWIDTH);
          const int depth = 16;
          byte *pal = W_CacheLumpName("PLAYPAL", PU_STATIC);
          int v;

          // [FG] temporarily draw status bar to background buffer
          V_DrawPatch(st_x, 0, BG, sbar);

          // [FG] separate colors for the top rows
          for (v = 0; v < arrlen(vstep); v++)
          {
            const int v0 = vstep[v][0], v1 = vstep[v][1];
            unsigned r = 0, g = 0, b = 0;
            byte col;

            for (y = v0; y < v1; y++)
            {
              for (x = 0; x < depth; x++)
              {
                byte *c = dest + y * hstep + ((x + lo) * hires);
                r += pal[3 * c[0] + 0];
                g += pal[3 * c[0] + 1];
                b += pal[3 * c[0] + 2];

                c += (w - 2 * x - 1) * hires;
                r += pal[3 * c[0] + 0];
                g += pal[3 * c[0] + 1];
                b += pal[3 * c[0] + 2];
              }
            }

            r /= 2 * depth * (v1 - v0);
            g /= 2 * depth * (v1 - v0);
            b /= 2 * depth * (v1 - v0);

            // [FG] tune down to half saturation (for empiric reasons)
            col = I_GetPaletteIndex(pal, r/2, g/2, b/2);

            // [FG] fill background buffer with average status bar color
            for (y = (v0 * hires); y < (v1 * hires); y++)
            {
              memset(dest + y * HIRESCREENWIDTH, col, HIRESCREENWIDTH);
            }
          }

          Z_ChangeTag (pal, PU_CACHE);
        }
        else
        {
          // [crispy] this is our own local copy of R_FillBackScreen() to
          // fill the entire background of st_backing_screen with the bezel pattern,
          // so it appears to the left and right of the status bar in widescreen mode
          byte *src;
          const char *name = (gamemode == commercial) ? "GRNROCK" : "FLOOR7_2";

          src = W_CacheLumpNum(firstflat + R_FlatNumForName(name), PU_CACHE);

          if (hires > 1)
          {
            int i;
            for (y = ((SCREENHEIGHT - ST_HEIGHT) * hires); y < HIRESCREENHEIGHT; y++)
            {
              for (x = 0; x < HIRESCREENWIDTH; x += hires)
              {
                const byte dot = src[(((y / hires) & 63) << 6) + ((x / hires) & 63)];
                for (i = 0; i < hires; i++)
                {
                  *dest++ = dot;
                }
              }
            }
          }
          else
          {
            for (y = SCREENHEIGHT-ST_HEIGHT; y < SCREENHEIGHT; y++)
              for (x = 0; x < SCREENWIDTH; x++)
              {
                *dest++ = src[((y&63)<<6) + (x&63)];
              }
          }

          // [crispy] preserve bezel bottom edge
          if (scaledviewwidth == SCREENWIDTH)
          {
            patch_t *const patch = W_CacheLumpName("brdr_b", PU_CACHE);

            for (x = 0; x < WIDESCREENDELTA; x += 8)
            {
              V_DrawPatch(x - WIDESCREENDELTA, 0, BG, patch);
              V_DrawPatch(ORIGWIDTH + WIDESCREENDELTA - x - 8, 0, BG, patch);
            }
          }
        }
      }

      // [crispy] center unity rerelease wide status bar
      V_DrawPatch(st_x, 0, BG, sbar);

      // draw right side of bar if needed (Doom 1.0)
      if (sbarr)
        V_DrawPatch(ST_ARMSBGX, 0, BG, sbarr);

      if (st_notdeathmatch)
        V_DrawPatch(ST_ARMSBGX, 0, BG, armsbg);

      // killough 3/7/98: make face background change with displayplayer
      if (netgame)
        V_DrawPatch(ST_FX, 0, BG, faceback[displayplayer]);

      // [crispy] copy entire SCREENWIDTH, to preserve the pattern
      // to the left and right of the status bar in widescren mode
      if (!force)
      {
        V_CopyRect(ST_X, 0, BG, SCREENWIDTH, ST_HEIGHT, ST_X, ST_Y, FG);
      }
      else
      {
        if (WIDESCREENDELTA > 0 && !st_firsttime)
        {
          V_CopyRect(0, 0, BG, WIDESCREENDELTA, ST_HEIGHT, 0, ST_Y, FG);
          V_CopyRect(ORIGWIDTH + WIDESCREENDELTA, 0, BG, WIDESCREENDELTA, ST_HEIGHT, ORIGWIDTH + WIDESCREENDELTA, ST_Y, FG);
        }
      }

    }
}


// Respond to keyboard input events,
//  intercept cheats.
boolean ST_Responder(event_t *ev)
{
  // Filter automap on/off.
  if (ev->type == ev_keyup && (ev->data1 & 0xffff0000) == AM_MSGHEADER)
    {
      if (ev->data1 == AM_MSGENTERED)
      {
        st_firsttime = true;
      }
    }
  else  // if a user keypress...
    return M_CheatResponder(ev);       // Try cheat responder in m_cheat.c
  return false;
}

int ST_calcPainOffset(void)
{
  static int lastcalc;
  static int oldhealth = -1;
  int health = plyr->health > 100 ? 100 : plyr->health;

  if (health != oldhealth)
    {
      lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
      oldhealth = health;
    }
  return lastcalc;
}

//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//

static int ST_DeadFace(void)
{
  const int state = (plyr->mo->state - states) - mobjinfo[plyr->mo->type].xdeathstate;

  // [FG] support face gib animations as in the 3DO/Jaguar/PSX ports
  if (have_xdthfaces && state >= 0)
  {
    return ST_XDTHFACE + MIN(state, have_xdthfaces - 1);
  }

  return ST_DEADFACE;
}

void ST_updateFaceWidget(void)
{
  int         i;
  angle_t     badguyangle;
  angle_t     diffang;
  static int  lastattackdown = -1;
  static int  priority = 0;
  boolean     doevilgrin;

  if (priority < 10)
    {
      // dead
      if (!plyr->health)
        {
          priority = 9;
          st_faceindex = ST_DeadFace();
          st_facecount = 1;
        }
    }

  if (priority < 9)
    {
      if (plyr->bonuscount)
        {
          // picking up bonus
          doevilgrin = false;

          for (i=0;i<NUMWEAPONS;i++)
            {
              if (oldweaponsowned[i] != plyr->weaponowned[i])
                {
                  doevilgrin = true;
                  oldweaponsowned[i] = plyr->weaponowned[i];
                }
            }
          if (doevilgrin)
            {
              // evil grin if just picked up weapon
              priority = 8;
              st_facecount = ST_EVILGRINCOUNT;
              st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
            }
        }

    }

  if (priority < 8)
    {
      if (plyr->damagecount && plyr->attacker && plyr->attacker != plyr->mo)
        {
          // being attacked
          priority = 7;

          // [FG] show "Ouch Face" as intended
          if (st_oldhealth - plyr->health > ST_MUCHPAIN)
            {
              // [FG] raise "Ouch Face" priority
              priority = 8;
              st_facecount = ST_TURNCOUNT;
              st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
            }
          else
            {
              badguyangle = R_PointToAngle2(plyr->mo->x,
                                            plyr->mo->y,
                                            plyr->attacker->x,
                                            plyr->attacker->y);

              if (badguyangle > plyr->mo->angle)
                {
                  // whether right or left
                  diffang = badguyangle - plyr->mo->angle;
                  i = diffang > ANG180;
                }
              else
                {
                  // whether left or right
                  diffang = plyr->mo->angle - badguyangle;
                  i = diffang <= ANG180;
                } // confusing, aint it?


              st_facecount = ST_TURNCOUNT;
              st_faceindex = ST_calcPainOffset();

              if (diffang < ANG45)
                {
                  // head-on
                  st_faceindex += ST_RAMPAGEOFFSET;
                }
              else if (i)
                {
                  // turn face right
                  st_faceindex += ST_TURNOFFSET;
                }
              else
                {
                  // turn face left
                  st_faceindex += ST_TURNOFFSET+1;
                }
            }
        }
    }

  if (priority < 7)
    {
      // getting hurt because of your own damn stupidity
      if (plyr->damagecount)
        {
          // [FG] show "Ouch Face" as intended
          if (st_oldhealth - plyr->health > ST_MUCHPAIN)
            {
              priority = 7;
              st_facecount = ST_TURNCOUNT;
              st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
            }
          else
            {
              priority = 6;
              st_facecount = ST_TURNCOUNT;
              st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
            }

        }

    }

  if (priority < 6)
    {
      // rapid firing
      if (plyr->attackdown)
        {
          if (lastattackdown==-1)
            lastattackdown = ST_RAMPAGEDELAY;
          else if (!--lastattackdown)
            {
              priority = 5;
              st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
              st_facecount = 1;
              lastattackdown = 1;
            }
        }
      else
        lastattackdown = -1;

    }

  if (priority < 5)
    {
      // invulnerability
      if ((plyr->cheats & CF_GODMODE)
          || plyr->powers[pw_invulnerability])
        {
          priority = 4;

          st_faceindex = ST_GODFACE;
          st_facecount = 1;

        }

    }

  // look left or look right if the facecount has timed out
  if (!st_facecount)
    {
      st_faceindex = ST_calcPainOffset() + (st_randomnumber % 3);
      st_facecount = ST_STRAIGHTFACECOUNT;
      priority = 0;
    }

  st_facecount--;

}

int sts_traditional_keys; // killough 2/28/98: traditional status bar keys

// [Nugget]
void ST_blinkKeys(player_t* player, int blue, int yellow, int red)
{
  int i;
  int keys[3] = { blue, yellow, red };
  
  if (!STRICTMODE(blink_keys)) { return; }

  player->keyblinktics = KEYBLINKTICS;

  for (i = 0;  i < 3;  i++) {
    if (   ((keys[i] == KEYBLINK_EITHER) && !(player->cards[i] || player->cards[i+3]))
        || ((keys[i] == KEYBLINK_CARD)   && !(player->cards[i]))
        || ((keys[i] == KEYBLINK_SKULL)  && !(player->cards[i+3]))
        || ((keys[i] == KEYBLINK_BOTH)   && !(player->cards[i] && player->cards[i+3])))
    { player->keyblinkkeys[i] = keys[i]; }
    else
    { player->keyblinkkeys[i] = KEYBLINK_NONE; }
  }
}

void ST_updateWidgets(void)
{
  static int  largeammo = 1994; // means "n/a"
  int         i;

  // must redirect the pointer if the ready weapon has changed.
  //  if (w_ready.data != plyr->readyweapon)
  //  {
  if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
    w_ready.num = &largeammo;
  else
    w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
  //{
  // static int tic=0;
  // static int dir=-1;
  // if (!(tic&15))
  //   plyr->ammo[weaponinfo[plyr->readyweapon].ammo]+=dir;
  // if (plyr->ammo[weaponinfo[plyr->readyweapon].ammo] == -100)
  //   dir = 1;
  // tic++;
  // }
  w_ready.data = plyr->readyweapon;

  // if (*w_ready.on)
  //  STlib_updateNum(&w_ready, true);
  // refresh weapon change
  //  }

  // update keycard multiple widgets
  for (i=0;i<3;i++)
    {
      keyboxes[i] = plyr->cards[i] ? i : -1;

      //jff 2/24/98 select double key
      //killough 2/28/98: preserve traditional keys by config option

      if (plyr->cards[i+3])
        keyboxes[i] = keyboxes[i]==-1 || sts_traditional_keys ? i+3 : i+6;
    }

  // [Nugget]: [crispy] blinking key or skull in the status bar
  if (plyr->keyblinktics)
  {
    if (!(plyr->keyblinktics & ((2*KEYBLINKMASK) - 1)))
    { S_StartSoundOptional(NULL, sfx_keybnk, sfx_itemup); }

    if (st_classicstatusbar && !(plyr->keyblinktics & (KEYBLINKMASK-1)))
    { st_firsttime = true; }

    plyr->keyblinktics--;

    for (i = 0;  i < 3;  i++)
    {
      if (!plyr->keyblinkkeys[i]) { continue; }

      keyboxes[i] = (plyr->keyblinktics & KEYBLINKMASK)
                    ? (plyr->keyblinkkeys[i] == KEYBLINK_BOTH)
                      ? i + 6
                      : (((plyr->keyblinkkeys[i] == KEYBLINK_EITHER)
                          &&  (plyr->keyblinktics & (2*KEYBLINKMASK))
                          && !(plyr->keyblinktics & (4*KEYBLINKMASK)))
                         || (plyr->keyblinkkeys[i] == KEYBLINK_SKULL))
                        ? i + 3
                        : i
                    : -1;

      if (!plyr->keyblinktics)
      { w_keyboxes[i].oldinum = -1; }
    }
  }

  // refresh everything if this is him coming back to life
  ST_updateFaceWidget();

  // used for armbg patch
  st_notdeathmatch = !deathmatch;

  // used by w_arms[] widgets
  st_armson = st_statusbaron && !deathmatch;

  // used by w_frags widget
  st_fragson = deathmatch && st_statusbaron;
  st_fragscount = 0;

  for (i=0 ; i<MAXPLAYERS ; i++)
    {
      if (i != displayplayer)            // killough 3/7/98
        st_fragscount += plyr->frags[i];
      else
        st_fragscount -= plyr->frags[i];
    }

}

// [Alaux]
static int SmoothCount(int shownval, int realval)
{
  int step = realval - shownval;

  // [Nugget] Disallowed in Strict Mode
  if (NOTSTRICTMODE(!smooth_counts || !step))
  {
    return realval;
  }
  else
  {
    int sign = step / abs(step);
    step = BETWEEN(1, 7, abs(step) / 20);
    shownval += (step+1)*sign;
  
    if (  (sign > 0 && shownval > realval)
        ||(sign < 0 && shownval < realval))
    {
      shownval = realval;
    }

    return shownval;
  }
}

boolean st_invul;

void ST_Ticker(void)
{
  st_health = SmoothCount(st_health, plyr->health);
  st_armor  = SmoothCount(st_armor, plyr->armorpoints);
  
  st_randomnumber = M_Random();
  ST_updateWidgets();
  st_oldhealth = plyr->health;

  st_invul = (plyr->powers[pw_invulnerability] > 4*32 ||
              plyr->powers[pw_invulnerability] & 8) ||
              plyr->cheats & CF_GODMODE;
}

static int st_palette = 0;
boolean palette_changes = true;

void ST_doPaletteStuff(void)
{
  int         palette;
  byte*       pal;
  int cnt = plyr->damagecount;

  // killough 7/14/98: beta version did not cause red berserk palette
  if (!beta_emulation)

  if (plyr->powers[pw_strength])
    {
      // slowly fade the berzerk out
      int bzc = 12 - (plyr->powers[pw_strength]>>6);
      if (bzc > cnt && !STRICTMODE(no_berserk_tint)) // [Nugget]
        cnt = bzc;
    }

  // [Nugget] Disable palette tint in menus
  if (STRICTMODE(!palette_changes || (no_menu_tint && menuactive)))
  {
    palette = 0;
  }
  else
  if (cnt)
  {
    // In Chex Quest, the player never sees red. Instead, the radiation suit
    // palette is used to tint the screen green, as though the player is being
    // covered in goo by an attacking flemoid.
    if (gameversion == exe_chex)
    {
      palette = RADIATIONPAL;
    }
    else
    {
      palette = (cnt+7)>>3;
      if (palette >= NUMREDPALS)
        palette = NUMREDPALS-1 + STRICTMODE(comp_unusedpals); // [Nugget]
      // [crispy] tune down a bit so the menu remains legible
      if (menuactive || paused)
        palette >>= 1;
      palette += STARTREDPALS - STRICTMODE(comp_unusedpals); // [Nugget]
    }
  }
  else
    if (plyr->bonuscount)
      {
        palette = (plyr->bonuscount+7)>>3;
        if (palette >= NUMBONUSPALS)
          palette = NUMBONUSPALS-1 + STRICTMODE(comp_unusedpals); // [Nugget]
        palette += STARTBONUSPALS - STRICTMODE(comp_unusedpals); // [Nugget]
      }
    else
      // killough 7/14/98: beta version did not cause green palette
      if (beta_emulation)
        palette = 0;
      else
      if ((plyr->powers[pw_ironfeet] > 4*32 || plyr->powers[pw_ironfeet] & 8)
          && !STRICTMODE(no_radsuit_tint)) // [Nugget]
        palette = RADIATIONPAL;
      else
        palette = 0;

  if (palette != st_palette)
    {
      st_palette = palette;
      // haleyjd: must cast to byte *, arith. on void pointer is
      // a GNU C extension
      pal = (byte *)W_CacheLumpNum(lu_palette, PU_CACHE) + palette*768;
      I_SetPalette (pal);
    }
}

// [Nugget] NUGHUD
static void NughudDrawPatch(nughud_vlignable_t *widget, patch_t *patch, boolean no_offsets)
{
  int x, y;

  x = widget->x + NUGHUDWIDESHIFT(widget->wide)
      - ((widget->align == 1) ? SHORT(patch->width)   :
         (widget->align == 0) ? SHORT(patch->width)/2 : 0);

  y = widget->y
      - ((widget->vlign == -1) ? SHORT(patch->height)   :
         (widget->vlign ==  0) ? SHORT(patch->height)/2 : 0);

  if (no_offsets) {
    x += SHORT(patch->leftoffset);
    y += SHORT(patch->topoffset);
  }

  V_DrawPatch(x, y, FG, patch);
}

void ST_drawWidgets(void)
{
  int i;
  int maxammo = plyr->maxammo[weaponinfo[w_ready.data].ammo];
  
  // [Alaux] Used to color health and armor counts based on
  // the real values, only ever relevant when using smooth counts
  const int health = plyr->health,  armor = plyr->armorpoints;

  // clear area
  if (!st_crispyhud && st_statusbaron)
  {
    V_CopyRect(WIDESCREENDELTA, 0, BG, ST_WIDTH, ST_HEIGHT, WIDESCREENDELTA, ST_Y, FG);
  }

  // [Nugget] Draw some NUGHUD graphics
  if (st_crispyhud)
  {
    for (i = 0;  i < NUMNUGHUDPATCHES;  i++)
    {
      if (nughud_patchlump[i] >= 0)
      {
        NughudDrawPatch(
          &nughud.patches[i],
          W_CacheLumpNum(nughud_patchlump[i], PU_STATIC),
          !nughud.patch_offsets
        );
      }
    }

    if (nughud.ammoicon.x > -1 && weaponinfo[w_ready.data].ammo != am_noammo)
    {
      patch_t *patch;
      int lump;
      boolean no_offsets = false;

      if (nhammo[0])
      { patch = nhammo[BETWEEN(0, 3, weaponinfo[w_ready.data].ammo)]; }
      else {
        char namebuf[32];
        boolean big = nughud.ammoicon_big;

        no_offsets = true;

        switch (BETWEEN(0, 3, weaponinfo[w_ready.data].ammo))
        {
          case 0: sprintf(namebuf, big ? "AMMOA0" : "CLIPA0"); break;
          case 1: sprintf(namebuf, big ? "SBOXA0" : "SHELA0"); break;
          case 2: sprintf(namebuf, big ? "CELPA0" : "CELLA0"); break;
          case 3: sprintf(namebuf, big ? "BROKA0" : "ROCKA0"); break;
        }

        if ((lump = (W_CheckNumForName)(namebuf, ns_sprites)) >= 0)
        { patch = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
        else
        { patch = NULL; }
      }

      if (patch) { NughudDrawPatch(&nughud.ammoicon, patch, no_offsets); }
    }

    if (nughud.healthicon.x > -1)
    {
      patch_t *patch;
      int lump;
      boolean no_offsets = false;

      if (nhealth[0])
      { patch = nhealth[plyr->powers[pw_strength] ? 1 : 0]; }
      else {
        char namebuf[32];

        no_offsets = true;

        switch (plyr->powers[pw_strength] ? 1 : 0)
        {
          case 0: sprintf(namebuf, "MEDIA0"); break;
          case 1: sprintf(namebuf, "PSTRA0"); break;
        }

        if ((lump = (W_CheckNumForName)(namebuf, ns_sprites)) >= 0)
        { patch = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
        else
        { patch = NULL; }
      }

      if (patch) { NughudDrawPatch(&nughud.healthicon, patch, no_offsets); }
    }

    if (nughud.armoricon.x > -1)
    {
      patch_t *patch;
      int lump;
      boolean no_offsets = false;

      if (nharmor[0])
      { patch = nharmor[BETWEEN(0, 2, plyr->armortype)]; }
      else {
        char namebuf[32];

        no_offsets = true;

        switch (BETWEEN(0, 2, plyr->armortype))
        {
          case 0: sprintf(namebuf, "BON2A0"); break;
          case 1: sprintf(namebuf, "ARM1A0"); break;
          case 2: sprintf(namebuf, "ARM2A0"); break;
        }

        if ((lump = (W_CheckNumForName)(namebuf, ns_sprites)) >= 0)
        { patch = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
        else
        { patch = NULL; }
      }

      if (patch) { NughudDrawPatch(&nughud.armoricon, patch, no_offsets); }
    }
  }

  // used by w_arms[] widgets
  // [Nugget] Draw both Arms and Frags in NUGHUD
  st_armson = st_statusbaron && (!deathmatch || st_crispyhud);

  // used by w_frags widget
  st_fragson = deathmatch && st_statusbaron;

  // backpack changes thresholds
  if (plyr->backpack && !hud_backpack_thresholds)
    maxammo /= 2;

  if (!st_crispyhud || nughud.ammo.x > -1) { // [Nugget] NUGHUD
    //jff 2/16/98 make color of ammo depend on amount
    // [Nugget] Make it gray if the player has infinite ammo
    if (plyr->cheats & CF_INFAMMO)
    { STlib_updateNum(&w_ready, cr_gray); }
    else
    if (*w_ready.num*100 < ammo_red*maxammo)
      STlib_updateNum(&w_ready, cr_red);
    else
      if (*w_ready.num*100 <
          ammo_yellow*maxammo)
        STlib_updateNum(&w_ready, cr_gold);
      else if (*w_ready.num > maxammo)
        STlib_updateNum(&w_ready, cr_blue2);
      else
        STlib_updateNum(&w_ready, cr_green);
  }

  // [Nugget] In case of `am_noammo`
  if ((screenblocks < CRISPY_HUD || (st_crispyhud && nughud.ammo.x > -1))
      && weaponinfo[plyr->readyweapon].ammo == am_noammo)
  {
    const int ammox = (st_crispyhud ? nughud.ammo.x : ST_AMMOX) + NUGHUDWIDESHIFT(nughud.ammo.wide),
              ammoy = (st_crispyhud ? nughud.ammo.y : ST_AMMOY);

    // [Nugget]: [crispy] draw berserk pack instead of no ammo if appropriate
    if (show_berserk && plyr->readyweapon == wp_fist && plyr->powers[pw_strength])
    {
      // NUGHUD Berserk
      if (st_crispyhud && nhbersrk)
      { V_DrawPatch(ammox, ammoy, FG, nhbersrk); }
      // Status Bar Berserk
      else if (stbersrk)
      { V_DrawPatch(ammox, ammoy, FG, stbersrk); }
      // Berserk or Medkit sprite
      else if (lu_berserk >= 0) {
        patch_t *const patch = W_CacheLumpNum(lu_berserk, PU_STATIC);
        
        // [crispy] (23,179) is the center of the Ammo widget
        V_DrawPatch(ammox - 21 - SHORT(patch->width)/2 + SHORT(patch->leftoffset),
                    ammoy + 8 - SHORT(patch->height)/2 + SHORT(patch->topoffset),
                    FG, patch);
      }
    }
    // NUGHUD Infinity
    else if (st_crispyhud && nhinfnty)
    { V_DrawPatch(ammox, ammoy, FG, nhinfnty); }
    // Status Bar Infinity
    else if (stinfnty)
    { V_DrawPatch(ammox, ammoy, FG, stinfnty); }
  }

  // [Nugget] NUGHUD
  if (st_crispyhud) {
    for (i=0;i<4;i++) {
      if (nughud.ammos[i].x > -1)     { STlib_updateNum(&w_ammo[i], NULL); }
      if (nughud.maxammos[i].x > -1)  { STlib_updateNum(&w_maxammo[i], NULL); }
    }
  }
  else
    for (i=0;i<4;i++)
    {
      STlib_updateNum(&w_ammo[i], NULL); //jff 2/16/98 no xlation
      STlib_updateNum(&w_maxammo[i], NULL);
    }

  if (!st_crispyhud || nughud.health.x > -1) { // [Nugget] NUGHUD
    // [Alaux] Make color of health gray when invulnerable
    if (st_invul)
      STlib_updatePercent(&w_health, cr_gray);
    else
    //jff 2/16/98 make color of health depend on amount
    if (health<health_red)
      STlib_updatePercent(&w_health, cr_red);
    else if (health<health_yellow)
      STlib_updatePercent(&w_health, cr_gold);
    else if (health<=health_green)
      STlib_updatePercent(&w_health, cr_green);
    else
      STlib_updatePercent(&w_health, cr_blue2); //killough 2/28/98
  }

  if (!st_crispyhud || nughud.armor.x > -1) { // [Nugget] NUGHUD
    // color of armor depends on type
    if (hud_armor_type)
    {
      // [Nugget] Make it gray ONLY if the player is in God Mode
      if (plyr->cheats & CF_GODMODE)
        STlib_updatePercent(&w_armor, cr_gray);
      else if (!plyr->armortype)
        STlib_updatePercent(&w_armor, cr_red);
      else if (plyr->armortype == 1)
        STlib_updatePercent(&w_armor, cr_green);
      else
        STlib_updatePercent(&w_armor, cr_blue2);
    }
    else
    {
    // [Nugget] Make it gray ONLY if the player is in God Mode
    if (plyr->cheats & CF_GODMODE)
      STlib_updatePercent(&w_armor, cr_gray);
    else
    //jff 2/16/98 make color of armor depend on amount
    if (armor<armor_red)
      STlib_updatePercent(&w_armor, cr_red);
    else if (armor<armor_yellow)
      STlib_updatePercent(&w_armor, cr_gold);
    else if (armor<=armor_green)
      STlib_updatePercent(&w_armor, cr_green);
    else
      STlib_updatePercent(&w_armor, cr_blue2); //killough 2/28/98
    }
  }

  // [Nugget] Highlight Arms #1 only if the player has Berserk
  st_berserk = plyr->powers[pw_strength] ? true : false;
  // [Nugget]: [crispy] show SSG availability in the Shotgun slot of the arms widget
  st_shotguns = plyr->weaponowned[wp_shotgun] | plyr->weaponowned[wp_supershotgun];

  // [Nugget] NUGHUD
  if (st_crispyhud) {
    for (i = 0;  i < 9;  i++)
      if (nughud.arms[i].x > -1)
      { STlib_updateMultIcon(&w_arms[i]); }
  }
  else
    for (i=0; i<6; i++)
    { STlib_updateMultIcon(&w_arms[i]); }

  // [Nugget] This probably shouldn't go here, but it works
  if (st_crispyhud && (nughud.face.x > -1) && nughud.face_bg)
  {
    V_DrawPatch(nughud.face.x + NUGHUDWIDESHIFT(nughud.face.wide), nughud.face.y+1,
                FG, faceback[netgame ? displayplayer : 1]);
  }

  if (!st_crispyhud || nughud.face.x > -1) // [Nugget] NUGHUD
  { STlib_updateMultIcon(&w_faces); }

  // [Nugget] NUGHUD
  if (st_crispyhud) {
    for (i = 0;  i < 3;  i++)
      if (nughud.keys[i].x > -1) { STlib_updateMultIcon(&w_keyboxes[i]); }
  }
  else
    for (i=0;i<3;i++)
    { STlib_updateMultIcon(&w_keyboxes[i]); }

  if (!st_crispyhud || nughud.frags.x > -1) // [Nugget] NUGHUD
  { STlib_updateNum(&w_frags, NULL); }
}

void ST_Drawer(boolean fullscreen, boolean refresh)
{
  static boolean oldcrispy = 0; // [Nugget] NUGHUD

  st_statusbaron = !fullscreen || automap_on;
  // [crispy] immediately redraw status bar after help screens have been shown
  st_firsttime = st_firsttime || refresh || inhelpscreens;

  st_classicstatusbar = st_statusbaron && !st_crispyhud;
  st_statusbarface = st_classicstatusbar || (st_crispyhud && nughud.face.x > -1);

  // [Nugget] NUGHUD /--------------------------------------------------------

  if (oldcrispy != st_crispyhud)
  {
    ST_createWidgets();
    ST_updateWidgets();
    HU_Start();
  }
  
  oldcrispy = st_crispyhud;

  // [Nugget] ---------------------------------------------------------------/

  ST_doPaletteStuff();  // Do red-/gold-shifts from damage/items

  if (st_firsttime)     // If just after ST_Start(), refresh all
  {
    st_firsttime = false;

    // draw status bar background to off-screen buff
    ST_refreshBackground(false);
  }
  
  ST_drawWidgets();
}

void ST_loadGraphics(void)
{
  int  i, facenum;
  char namebuf[32];

  // Load the numbers, tall and short
  for (i=0;i<10;i++)
    {
      sprintf(namebuf, "STTNUM%d", i);
      tallnum[i] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);
      M_snprintf(namebuf, sizeof(namebuf), "STYSNUM%d", i);
      shortnum[i] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);
    }

  // Load percent key.
  //Note: why not load STMINUS here, too?
  tallpercent = (patch_t *) W_CacheLumpName("STTPRCNT", PU_STATIC);

  // key cards
  for (i=0;i<NUMCARDS+3;i++)  //jff 2/23/98 show both keys too
    {
      sprintf(namebuf, "STKEYS%d", i);
      keys[i] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);
    }

  // arms background
  armsbg = (patch_t *) W_CacheLumpName("STARMS", PU_STATIC);

  // arms ownership widgets
  // [Nugget] Load all 9 numbers
  for (i=0;i<6+3;i++)
    {
      sprintf(namebuf, "STGNUM%d", i+1);

      // gray #
      arms[i][0] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);

      // yellow #
      arms[i][1] = shortnum[i+1];
    }

  // face backgrounds for different color players
  // killough 3/7/98: add better support for spy mode by loading all
  // player face backgrounds and using displayplayer to choose them:
  for (i=0; i<MAXPLAYERS; i++)
    {
      sprintf(namebuf, "STFB%d", i);
      faceback[i] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);
    }

  // status bar background bits
  if (W_CheckNumForName("STBAR") >= 0)
  {
    sbar  = (patch_t *) W_CacheLumpName("STBAR", PU_STATIC);
    sbarr = NULL;
  }
  else
  {
    sbar  = (patch_t *) W_CacheLumpName("STMBARL", PU_STATIC);
    sbarr = (patch_t *) W_CacheLumpName("STMBARR", PU_STATIC);
  }

  // face states
  facenum = 0;
  for (i=0;i<ST_NUMPAINFACES;i++)
    {
      int j;
      for (j=0;j<ST_NUMSTRAIGHTFACES;j++)
        {
          sprintf(namebuf, "STFST%d%d", i, j);
          faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
        }
      sprintf(namebuf, "STFTR%d0", i);        // turn right
      faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
      sprintf(namebuf, "STFTL%d0", i);        // turn left
      faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
      sprintf(namebuf, "STFOUCH%d", i);       // ouch!
      faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
      sprintf(namebuf, "STFEVL%d", i);        // evil grin ;)
      faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
      sprintf(namebuf, "STFKILL%d", i);       // pissed off
      faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
    }
  faces[facenum++] = W_CacheLumpName("STFGOD0", PU_STATIC);
  faces[facenum++] = W_CacheLumpName("STFDEAD0", PU_STATIC);

  // [FG] support face gib animations as in the 3DO/Jaguar/PSX ports
  for (i = 0; i < ST_NUMXDTHFACES; i++)
  {
    sprintf(namebuf, "STFXDTH%d", i);

    if (W_CheckNumForName(namebuf) != -1)
      faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
    else
      break;
  }
  have_xdthfaces = i;

  { // [Nugget] --------------------------------------------------------------
    int lump;

    // Find Status Bar Berserk patch
    if ((lump = (W_CheckNumForName)("STBERSRK", ns_global)) >= 0)
    { stbersrk = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
    else {
      stbersrk = NULL;

      lu_berserk = (W_CheckNumForName)("PSTRA0", ns_sprites);
      if (lu_berserk == -1)
      { lu_berserk = (W_CheckNumForName)("MEDIA0", ns_sprites); }
    }

    // Find Status Bar Infinity patch
    if ((lump = (W_CheckNumForName)("STINFNTY", ns_global)) >= 0)
    { stinfnty = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
    else
    { stinfnty = NULL; }

    // Find NUGHUD patches
    for (i = 0;  i < NUMNUGHUDPATCHES;  i++)
    {
      if (nughud.patchnames[i] != NULL)
      {
        nughud_patchlump[i] = (W_CheckNumForName)(nughud.patchnames[i], ns_sprites);
        if (nughud_patchlump[i] == -1)
        { nughud_patchlump[i] = (W_CheckNumForName)(nughud.patchnames[i], ns_global); }
      }
      else
      { nughud_patchlump[i] = -1; }
    }

    // Load NUGHUD fonts -----------------------------------------------------

    // Tall Numbers -------------------

    // Load NHTNUM0 to NHTNUM9
    for (i = 0;  i < 10;  i++) {
      sprintf(namebuf, "NHTNUM%d", i);

      if ((lump = (W_CheckNumForName)(namebuf, ns_global)) >= 0)
      { nhtnum[i] = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
      else {
        nhtnum[0] = NULL;
        break;
      }
    }

    // Load NHTMINUS
    if (nhtnum[0] && (lump = (W_CheckNumForName)("NHTMINUS", ns_global)) >= 0)
    { nhtminus = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
    else
    { nhtnum[0] = nhtminus = NULL; }

    // Load NHTPRCNT
    if (nhtnum[0] && (lump = (W_CheckNumForName)("NHTPRCNT", ns_global)) >= 0)
    { nhtprcnt = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
    else
    { nhtnum[0] = nhtminus = nhtprcnt = NULL; }

    // Ready Ammo Numbers -------------

    // Load NHRNUM0 to NHRNUM9
    for (i = 0;  i < 10;  i++) {
      sprintf(namebuf, "NHRNUM%d", i);

      if ((lump = (W_CheckNumForName)(namebuf, ns_global)) >= 0)
      { nhrnum[i] = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
      else {
        nhrnum[0] = NULL;
        break;
      }
    }

    // Load NHRMINUS
    if (nhrnum[0] && (lump = (W_CheckNumForName)("NHRMINUS", ns_global)) >= 0)
    { nhrminus = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
    else
    { nhrnum[0] = nhrminus = NULL; }

    // Ammo numbers -------------------

    // Load NHAMNUM0 to NHAMNUM9
    for (i = 0;  i < 10;  i++) {
      sprintf(namebuf, "NHAMNUM%d", i);

      if ((lump = (W_CheckNumForName)(namebuf, ns_global)) >= 0)
      { nhamnum[i] = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
      else {
        nhamnum[0] = NULL;
        break;
      }
    }

    // Arms numbers -------------------

    for (i = 0;  i < 9;  i++) {
      // Load NHW0NUM1 to NHW0NUM9
      sprintf(namebuf, "NHW0NUM%d", i+1);

      if ((lump = (W_CheckNumForName)(namebuf, ns_global)) >= 0)
      { nhwpnum[i][0] = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
      else {
        nhwpnum[0][0] = NULL;
        break;
      }

      // Load NHW1NUM1 to NHW1NUM9
      sprintf(namebuf, "NHW1NUM%d", i+1);

      if ((lump = (W_CheckNumForName)(namebuf, ns_global)) >= 0)
      { nhwpnum[i][1] = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
      else {
        nhwpnum[0][0] = NULL;
        break;
      }
    }

    // Keys ---------------------------

    // Load NHKEYS
    for (i = 0;  i < NUMCARDS+3;  i++) {
      sprintf(namebuf, "NHKEYS%d", i);

      if ((lump = (W_CheckNumForName)(namebuf, ns_global)) >= 0)
      { nhkeys[i] = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
      else {
        nhkeys[0] = NULL;
        break;
      }
    }

    // Berserk ------------------------

    // Load NHBERSRK
    if ((lump = (W_CheckNumForName)("NHBERSRK", ns_global)) >= 0)
    { nhbersrk = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
    else
    { nhbersrk = NULL; }

    // Ammo icons ---------------------

    // Load NHAMMO0 to NHAMMO3 if available
    for (i = 0;  i < 4;  i++) {
      sprintf(namebuf, "NHAMMO%d", i);

      if ((lump = (W_CheckNumForName)(namebuf, ns_global)) >= 0)
      { nhammo[i] = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
      else {
        nhammo[0] = NULL;
        break;
      }
    }

    // Health icons -------------------

    // Load NHEALTH0 to NHEALTH1 if available
    for (i = 0;  i < 2;  i++) {
      sprintf(namebuf, "NHEALTH%d", i);

      if ((lump = (W_CheckNumForName)(namebuf, ns_global)) >= 0)
      { nhealth[i] = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
      else {
        nhealth[0] = NULL;
        break;
      }
    }

    // Armor icons --------------------

    // Load NHARMOR0 to NHARMOR2 if available
    for (i = 0;  i < 3;  i++) {
      sprintf(namebuf, "NHARMOR%d", i);

      if ((lump = (W_CheckNumForName)(namebuf, ns_global)) >= 0)
      { nharmor[i] = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
      else {
        nharmor[0] = NULL;
        break;
      }
    }

    // Infinity -----------------------

    // Load NHINFNTY
    if ((lump = (W_CheckNumForName)("NHINFNTY", ns_global)) >= 0)
    { nhinfnty = (patch_t *) W_CacheLumpNum(lump, PU_STATIC); }
    else
    { nhinfnty = NULL; }
  }
}

void ST_loadData(void)
{
  lu_palette = W_GetNumForName ("PLAYPAL");
  ST_loadGraphics();
}

void ST_initData(void)
{
  int i;

  st_firsttime = true;
  plyr = &players[displayplayer];            // killough 3/7/98

  st_statusbaron = true;

  st_faceindex = 0;
  st_palette = -1;

  st_oldhealth = -1;

  for (i=0;i<NUMWEAPONS;i++)
    oldweaponsowned[i] = plyr->weaponowned[i];

  for (i=0;i<3;i++)
    keyboxes[i] = -1;

  STlib_init();
}

void ST_createWidgets(void)
{
  int i;

  STlib_init(); // [Nugget] Reload minus sign

  // [Nugget] NUGHUD

  #define NUGHUDALIGN(a) (st_crispyhud ? (a) : 1)

  // ready weapon ammo
  STlib_initNum(&w_ready,
                (st_crispyhud ? nughud.ammo.x : ST_AMMOX) + NUGHUDWIDESHIFT(nughud.ammo.wide),
                (st_crispyhud ? nughud.ammo.y : ST_AMMOY),
                (st_crispyhud ? (nhrnum[0] ? nhrnum :
                                 nhtnum[0] ? nhtnum : tallnum) : tallnum),
                &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
                &st_statusbaron,
                ST_AMMOWIDTH,
                NUGHUDALIGN(nughud.ammo.align));

  w_ready.isready = true;
  /*
  // the last weapon type
  w_ready.data = plyr->readyweapon;
  */

  // health percentage
  STlib_initPercent(&w_health,
                    (st_crispyhud ? nughud.health.x : ST_HEALTHX) + NUGHUDWIDESHIFT(nughud.health.wide),
                    (st_crispyhud ? nughud.health.y : ST_HEALTHY),
                    ((st_crispyhud && nhtnum[0]) ? nhtnum : tallnum),
                    &st_health,
                    &st_statusbaron,
                    ((st_crispyhud && nhtprcnt) ? nhtprcnt : tallpercent),
                    NUGHUDALIGN(nughud.health.align));

  // weapons owned
  if (st_crispyhud) {
    for (i = 0;  i < 9;  i++)
      STlib_initMultIcon(&w_arms[i],
                         nughud.arms[i].x + NUGHUDWIDESHIFT(nughud.arms[i].wide),
                         nughud.arms[i].y,
                         (nhwpnum[0][0] ? nhwpnum[i] : arms[i]),
                         // [Nugget] Highlight Arms #1 only if the player has Berserk
                         ((i == wp_fist) ? &st_berserk : (int *) &plyr->weaponowned[i]),
                         &st_armson);

    // [crispy] show SSG availability in the Shotgun slot of the arms widget
    if (show_ssg && !(nughud.arms[8].x > -1)) { w_arms[2].inum = &st_shotguns; }
  }
  else {
    for(i=0;i<6;i++) {
      // [Nugget] Alternative Arms display (Saw/SSG instead of Pistol)
      int alt = (alt_arms ? ((i==5 && have_ssg) ? 2 : 1) : 0);
      STlib_initMultIcon(&w_arms[i],
                         ST_ARMSX+(i%3)*ST_ARMSXSPACE,
                         ST_ARMSY+(i/3)*ST_ARMSYSPACE,
                         arms[i+1+alt], (int *) &plyr->weaponowned[i+1+alt],
                         &st_armson);
    }
    // [Nugget]: [crispy] show SSG availability in the Shotgun slot of the arms widget
    if (show_ssg && !alt_arms) { w_arms[1].inum = &st_shotguns; }
  }

  // frags sum
  STlib_initNum(&w_frags,
                (st_crispyhud ? nughud.frags.x : ST_FRAGSX) + NUGHUDWIDESHIFT(nughud.frags.wide),
                (st_crispyhud ? nughud.frags.y : ST_FRAGSY),
                ((st_crispyhud && nhtnum[0]) ? nhtnum : tallnum),
                &st_fragscount,
                &st_fragson,
                ST_FRAGSWIDTH,
                NUGHUDALIGN(nughud.frags.align));

  // faces
  STlib_initMultIcon(&w_faces,
                     (st_crispyhud ? nughud.face.x : ST_FACESX) + NUGHUDWIDESHIFT(nughud.face.wide),
                     (st_crispyhud ? nughud.face.y : ST_FACESY),
                     faces,
                     &st_faceindex,
                     &st_statusbarface); // [Nugget]

  // armor percentage - should be colored later
  STlib_initPercent(&w_armor,
                    (st_crispyhud ? nughud.armor.x : ST_ARMORX) + NUGHUDWIDESHIFT(nughud.armor.wide),
                    (st_crispyhud ? nughud.armor.y : ST_ARMORY),
                    ((st_crispyhud && nhtnum[0]) ? nhtnum : tallnum),
                    &st_armor,
                    &st_statusbaron,
                    ((st_crispyhud && nhtprcnt) ? nhtprcnt : tallpercent),
                    NUGHUDALIGN(nughud.armor.align));

  // keyboxes 0-2
  STlib_initMultIcon(&w_keyboxes[0],
                     (st_crispyhud ? nughud.keys[0].x : ST_KEY0X) + NUGHUDWIDESHIFT(nughud.keys[0].wide),
                     (st_crispyhud ? nughud.keys[0].y : ST_KEY0Y),
                     ((st_crispyhud && nhkeys[0]) ? nhkeys : keys),
                     &keyboxes[0],
                     &st_statusbaron);

  STlib_initMultIcon(&w_keyboxes[1],
                     (st_crispyhud ? nughud.keys[1].x : ST_KEY1X) + NUGHUDWIDESHIFT(nughud.keys[1].wide),
                     (st_crispyhud ? nughud.keys[1].y : ST_KEY1Y),
                     ((st_crispyhud && nhkeys[0]) ? nhkeys : keys),
                     &keyboxes[1],
                     &st_statusbaron);

  STlib_initMultIcon(&w_keyboxes[2],
                     (st_crispyhud ? nughud.keys[2].x : ST_KEY2X) + NUGHUDWIDESHIFT(nughud.keys[2].wide),
                     (st_crispyhud ? nughud.keys[2].y : ST_KEY2Y),
                     ((st_crispyhud && nhkeys[0]) ? nhkeys : keys),
                     &keyboxes[2],
                     &st_statusbaron);

  // ammo count (all four kinds)
  STlib_initNum(&w_ammo[0],
                (st_crispyhud ? nughud.ammos[0].x : ST_AMMO0X) + NUGHUDWIDESHIFT(nughud.ammos[0].wide),
                (st_crispyhud ? nughud.ammos[0].y : ST_AMMO0Y),
                ((st_crispyhud && nhamnum[0]) ? nhamnum : shortnum),
                &plyr->ammo[0],
                &st_statusbaron,
                ST_AMMO0WIDTH,
                NUGHUDALIGN(nughud.ammos[0].align));

  STlib_initNum(&w_ammo[1],
                (st_crispyhud ? nughud.ammos[1].x : ST_AMMO1X) + NUGHUDWIDESHIFT(nughud.ammos[1].wide),
                (st_crispyhud ? nughud.ammos[1].y : ST_AMMO1Y),
                ((st_crispyhud && nhamnum[0]) ? nhamnum : shortnum),
                &plyr->ammo[1],
                &st_statusbaron,
                ST_AMMO1WIDTH,
                NUGHUDALIGN(nughud.ammos[1].align));

  STlib_initNum(&w_ammo[2],
                (st_crispyhud ? nughud.ammos[2].x : ST_AMMO2X) + NUGHUDWIDESHIFT(nughud.ammos[2].wide),
                (st_crispyhud ? nughud.ammos[2].y : ST_AMMO2Y),
                ((st_crispyhud && nhamnum[0]) ? nhamnum : shortnum),
                &plyr->ammo[2],
                &st_statusbaron,
                ST_AMMO2WIDTH,
                NUGHUDALIGN(nughud.ammos[2].align));

  STlib_initNum(&w_ammo[3],
                (st_crispyhud ? nughud.ammos[3].x : ST_AMMO3X) + NUGHUDWIDESHIFT(nughud.ammos[3].wide),
                (st_crispyhud ? nughud.ammos[3].y : ST_AMMO3Y),
                ((st_crispyhud && nhamnum[0]) ? nhamnum : shortnum),
                &plyr->ammo[3],
                &st_statusbaron,
                ST_AMMO3WIDTH,
                NUGHUDALIGN(nughud.ammos[3].align));

  // max ammo count (all four kinds)
  STlib_initNum(&w_maxammo[0],
                (st_crispyhud ? nughud.maxammos[0].x : ST_MAXAMMO0X) + NUGHUDWIDESHIFT(nughud.maxammos[0].wide),
                (st_crispyhud ? nughud.maxammos[0].y : ST_MAXAMMO0Y),
                ((st_crispyhud && nhamnum[0]) ? nhamnum : shortnum),
                &plyr->maxammo[0],
                &st_statusbaron,
                ST_MAXAMMO0WIDTH,
                NUGHUDALIGN(nughud.maxammos[0].align));

  STlib_initNum(&w_maxammo[1],
                (st_crispyhud ? nughud.maxammos[1].x : ST_MAXAMMO1X) + NUGHUDWIDESHIFT(nughud.maxammos[1].wide),
                (st_crispyhud ? nughud.maxammos[1].y : ST_MAXAMMO1Y),
                ((st_crispyhud && nhamnum[0]) ? nhamnum : shortnum),
                &plyr->maxammo[1],
                &st_statusbaron,
                ST_MAXAMMO1WIDTH,
                NUGHUDALIGN(nughud.maxammos[1].align));

  STlib_initNum(&w_maxammo[2],
                (st_crispyhud ? nughud.maxammos[2].x : ST_MAXAMMO2X) + NUGHUDWIDESHIFT(nughud.maxammos[2].wide),
                (st_crispyhud ? nughud.maxammos[2].y : ST_MAXAMMO2Y),
                ((st_crispyhud && nhamnum[0]) ? nhamnum : shortnum),
                &plyr->maxammo[2],
                &st_statusbaron,
                ST_MAXAMMO2WIDTH,
                NUGHUDALIGN(nughud.maxammos[2].align));

  STlib_initNum(&w_maxammo[3],
                (st_crispyhud ? nughud.maxammos[3].x : ST_MAXAMMO3X) + NUGHUDWIDESHIFT(nughud.maxammos[3].wide),
                (st_crispyhud ? nughud.maxammos[3].y : ST_MAXAMMO3Y),
                ((st_crispyhud && nhamnum[0]) ? nhamnum : shortnum),
                &plyr->maxammo[3],
                &st_statusbaron,
                ST_MAXAMMO3WIDTH,
                NUGHUDALIGN(nughud.maxammos[3].align));

  #undef NUGHUDALIGN
}

// [Nugget] Removed ST_MoveHud(), as we don't need it

static boolean st_stopped = true;

void ST_Start(void)
{
  if (!st_stopped)
    ST_Stop();
  ST_initData();
  ST_createWidgets();
  st_stopped = false;
}

void ST_Stop(void)
{
  if (st_stopped)
    return;
  I_SetPalette (W_CacheLumpNum (lu_palette, PU_CACHE));
  st_stopped = true;
}

static int StatusBarBufferHeight(void)
{
  int i;
  int st_height = ST_HEIGHT;
  patch_t *const patch = W_CacheLumpName("brdr_b", PU_CACHE);

  if (patch && patch->height > st_height)
    st_height = patch->height;

  if (sbar && sbar->height > st_height)
    st_height = sbar->height;

  if (armsbg && armsbg->height > st_height)
    st_height = armsbg->height;

  for (i = 0; i < MAXPLAYERS; i++)
  {
    if (faceback[i] && faceback[i]->height > st_height)
      st_height = faceback[i]->height;
  }

  return st_height;
}

void ST_Init(void)
{
  int st_height, size;

  ST_loadData();

  st_height = StatusBarBufferHeight();
  size = SCREENWIDTH * (st_height * hires*hires);

  if (screens[4])
  {
    Z_Free(screens[4]);
  }

  // killough 11/98: allocate enough for hires
  screens[4] = Z_Malloc(size, PU_STATIC, 0);
}

void ST_Warnings(void)
{
  int i;
  patch_t *const patch = W_CacheLumpName("brdr_b", PU_CACHE);

  if (patch && patch->height > ST_HEIGHT)
  {
    I_Printf(VB_WARNING, "ST_Init: Non-standard BRDR_B height of %d. "
                         "Expected <= %d.", patch->height, ST_HEIGHT);
  }

  if (sbar && sbar->height != ST_HEIGHT)
  {
    I_Printf(VB_WARNING, "ST_Init: Non-standard STBAR height of %d. "
                         "Expected %d.", sbar->height, ST_HEIGHT);
  }

  if (armsbg && armsbg->height > ST_HEIGHT)
  {
    I_Printf(VB_WARNING, "ST_Init: Non-standard STARMS height of %d. "
                         "Expected <= %d.", armsbg->height, ST_HEIGHT);
  }

  for (i = 0; i < MAXPLAYERS; i++)
  {
    if (faceback[i] && faceback[i]->height > ST_HEIGHT)
    {
      I_Printf(VB_WARNING, "ST_Init: Non-standard STFB%d height of %d. "
                           "Expected <= %d.", i, faceback[i]->height, ST_HEIGHT);
    }
  }
}

void ST_ResetPalette(void)
{
  st_palette = -1;
  I_SetPalette(W_CacheLumpNum(lu_palette, PU_CACHE));
}

//----------------------------------------------------------------------------
//
// $Log: st_stuff.c,v $
// Revision 1.46  1998/05/06  16:05:40  jim
// formatting and documenting
//
// Revision 1.45  1998/05/03  22:50:58  killough
// beautification, move external declarations, remove cheats
//
// Revision 1.44  1998/04/27  17:30:39  jim
// Fix DM demo/newgame status, remove IDK (again)
//
// Revision 1.43  1998/04/27  02:30:12  killough
// fuck you
//
// Revision 1.42  1998/04/24  23:52:31  thldrmn
// Removed idk cheat
//
// Revision 1.41  1998/04/24  11:39:23  killough
// Fix cheats while demo is played back
//
// Revision 1.40  1998/04/19  01:10:19  killough
// Generalize cheat engine to add deh support
//
// Revision 1.39  1998/04/16  06:26:06  killough
// Prevent cheats from working inside menu
//
// Revision 1.38  1998/04/12  10:58:24  jim
// IDMUSxy for DOOM 1 fix
//
// Revision 1.37  1998/04/12  10:23:52  jim
// IDMUS00 ok in DOOM 1
//
// Revision 1.36  1998/04/12  02:00:39  killough
// Change tranmap to main_tranmap
//
// Revision 1.35  1998/04/12  01:08:51  jim
// Fixed IDMUS00 crash
//
// Revision 1.34  1998/04/11  14:48:11  thldrmn
// Replaced IDK with TNTKA cheat
//
// Revision 1.33  1998/04/10  06:36:45  killough
// Fix -fast parameter bugs
//
// Revision 1.32  1998/03/31  10:37:17  killough
// comment clarification
//
// Revision 1.31  1998/03/28  18:09:19  killough
// Fix deh-cheat self-annihilation bug, make iddt closer to Doom
//
// Revision 1.30  1998/03/28  05:33:02  jim
// Text enabling changes for DEH
//
// Revision 1.29  1998/03/23  15:24:54  phares
// Changed pushers to linedef control
//
// Revision 1.28  1998/03/23  06:43:26  jim
// linedefs reference initial version
//
// Revision 1.27  1998/03/23  03:40:46  killough
// Fix idclip bug, make monster kills message smart
//
// Revision 1.26  1998/03/20  00:30:37  phares
// Changed friction to linedef control
//
// Revision 1.25  1998/03/17  20:44:32  jim
// fixed idmus non-restore, space bug
//
// Revision 1.24  1998/03/12  14:35:01  phares
// New cheat codes
//
// Revision 1.23  1998/03/10  07:14:38  jim
// Initial DEH support added, minus text
//
// Revision 1.22  1998/03/09  07:31:48  killough
// Fix spy mode to display player correctly, add TNTFAST
//
// Revision 1.21  1998/03/06  05:31:02  killough
// PEst control, from the TNT'EM man
//
// Revision 1.20  1998/03/02  15:35:03  jim
// Enabled Lee's status changes, added new types to common.cfg
//
// Revision 1.19  1998/03/02  12:09:18  killough
// blue status bar color, monsters_remember, traditional_keys
//
// Revision 1.18  1998/02/27  11:00:58  phares
// Can't own weapons that don't exist
//
// Revision 1.17  1998/02/26  22:57:45  jim
// Added message review display to HUD
//
// Revision 1.16  1998/02/24  08:46:45  phares
// Pushers, recoil, new friction, and over/under work
//
// Revision 1.15  1998/02/24  04:14:19  jim
// Added double keys to status
//
// Revision 1.14  1998/02/23  04:57:29  killough
// Fix TNTEM cheat again, add new cheats
//
// Revision 1.13  1998/02/20  21:57:07  phares
// Preliminarey sprite translucency
//
// Revision 1.12  1998/02/19  23:15:52  killough
// Add TNTAMMO in addition to TNTAMO
//
// Revision 1.11  1998/02/19  16:55:22  jim
// Optimized HUD and made more configurable
//
// Revision 1.10  1998/02/18  00:59:20  jim
// Addition of HUD
//
// Revision 1.9  1998/02/17  06:15:48  killough
// Add TNTKEYxx, TNTAMOx, TNTWEAPx cheats, and cheat engine support for them.
//
// Revision 1.8  1998/02/15  02:48:01  phares
// User-defined keys
//
// Revision 1.7  1998/02/09  03:19:04  killough
// Rewrite cheat code engine, add IDK and TNTHOM
//
// Revision 1.6  1998/02/02  22:19:01  jim
// Added TNTEM cheat to kill every monster alive
//
// Revision 1.5  1998/01/30  18:48:10  phares
// Changed textspeed and textwait to functions
//
// Revision 1.4  1998/01/30  16:09:03  phares
// Faster end-mission text display
//
// Revision 1.3  1998/01/28  12:23:05  phares
// TNTCOMP cheat code added
//
// Revision 1.2  1998/01/26  19:24:58  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:03  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
