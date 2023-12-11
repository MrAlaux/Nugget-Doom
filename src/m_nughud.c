//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//  Copyright(C) 2020-2021 Fabian Greffrath
//  Copyright(C) 2021-2023 Alaux
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
//  Variant of m_misc.c specifically for declaration and loading of NUGHUD
//  variables

#include <errno.h>

#include "hu_stuff.h"
#include "i_video.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_nughud.h"
#include "p_mobj.h"
#include "st_stuff.h"
#include "w_wad.h"

#include "m_io.h"

nughud_t nughud; // Behold!!!

//
// DEFAULTS
//

#define WIDGET(n, m, vx, vy, vw)                                            \
 { n "_x",     (config_t *)&(m).x,     NULL, { vx }, { -1, 320 }, number }, \
 { n "_y",     (config_t *)&(m).y,     NULL, { vy }, {  0, 200 }, number }, \
 { n "_wide",  (config_t *)&(m).wide,  NULL, { vw }, { -2, 2   }, number }

#define WIDGET2(n, m, vx, vy, vw, va)                                      \
 WIDGET(n, m, vx, vy, vw),                                                 \
 { n "_align", (config_t *)&(m).align, NULL, { va }, { -1, 1   }, number }

#define WIDGET3(n, m, vx, vy, vw, va, vv)                                  \
 WIDGET2(n, m, vx, vy, vw, va),                                            \
 { n "_vlign", (config_t *)&(m).vlign, NULL, { vv }, { -1, 1   }, number }

#define TEXTLINE(n, m, vx, vy, vw, va)                                      \
 { n "_x",     (config_t *)&(m).x,     NULL, { vx }, {  0, 320 }, number }, \
 { n "_y",     (config_t *)&(m).y,     NULL, { vy }, {  0, 200 }, number }, \
 { n "_wide",  (config_t *)&(m).wide,  NULL, { vw }, { -2, 2   }, number }, \
 { n "_align", (config_t *)&(m).align, NULL, { va }, { -1, 1   }, number }

#define PATCH(n, i)                                                                             \
 { n "_x",     (config_t *)&nughud.patches[i].x,     NULL, {  0        }, {  0, 320 }, number }, \
 { n "_y",     (config_t *)&nughud.patches[i].y,     NULL, {  0        }, {  0, 200 }, number }, \
 { n "_wide",  (config_t *)&nughud.patches[i].wide,  NULL, {  0        }, { -2, 2   }, number }, \
 { n "_align", (config_t *)&nughud.patches[i].align, NULL, { -1        }, { -1, 1   }, number }, \
 { n "_vlign", (config_t *)&nughud.patches[i].vlign, NULL, {  1        }, { -1, 1   }, number }, \
 { n "_name",  (config_t *)&nughud.patchnames[i],    NULL, { .s = NULL }, {  0      }, string }

#define TOGGLE(n, m, v) { n, (config_t *)&(m), NULL, { v }, { 0, 1 }, number }

default_t nughud_defaults[] = {
  WIDGET2(   "nughud_ammo",        nughud.ammo,         ST_AMMOX,     ST_AMMOY,     -1,  1     ),
  WIDGET3(   "nughud_ammoicon",    nughud.ammoicon,    -1,            0,             0, -1,  1 ),
  TOGGLE(    "nughud_ammoicon_big",nughud.ammoicon_big, 0                                      ),
  WIDGET2(   "nughud_health",      nughud.health,       ST_HEALTHX,   ST_HEALTHY,   -1,  1     ),
  WIDGET3(   "nughud_healthicon",  nughud.healthicon,  -1,            0,             0, -1,  1 ),
  WIDGET(    "nughud_arms1",       nughud.arms[0],     -1,            0,             0         ),
  WIDGET(    "nughud_arms2",       nughud.arms[1],      111,          172,          -1         ),
  WIDGET(    "nughud_arms3",       nughud.arms[2],      119,          172,          -1         ),
  WIDGET(    "nughud_arms4",       nughud.arms[3],      127,          172,          -1         ),
  WIDGET(    "nughud_arms5",       nughud.arms[4],      135,          172,          -1         ),
  WIDGET(    "nughud_arms6",       nughud.arms[5],      111,          182,          -1         ),
  WIDGET(    "nughud_arms7",       nughud.arms[6],      119,          182,          -1         ),
  WIDGET(    "nughud_arms8",       nughud.arms[7],      127,          182,          -1         ),
  WIDGET(    "nughud_arms9",       nughud.arms[8],      135,          182,          -1         ),
  WIDGET2(   "nughud_frags",       nughud.frags,        174,          171,           0,  1     ),
  WIDGET(    "nughud_face",        nughud.face,        -1,            ST_FACESY,     0         ),
  TOGGLE(    "nughud_face_bg",     nughud.face_bg,      1                                      ),
  WIDGET2(   "nughud_armor",       nughud.armor,        ST_ARMORX,    ST_ARMORY,     1,  1     ),
  WIDGET3(   "nughud_armoricon",   nughud.armoricon,   -1,            0,             0, -1,  1 ),
  WIDGET(    "nughud_key0",        nughud.keys[0],      ST_KEY0X,     ST_KEY0Y,      1         ),
  WIDGET(    "nughud_key1",        nughud.keys[1],      ST_KEY1X,     ST_KEY1Y,      1         ),
  WIDGET(    "nughud_key2",        nughud.keys[2],      ST_KEY2X,     ST_KEY2Y,      1         ),
  WIDGET2(   "nughud_ammo0",       nughud.ammos[0],     ST_AMMO0X,    ST_AMMO0Y,     1,  1     ),
  WIDGET2(   "nughud_ammo1",       nughud.ammos[1],     ST_AMMO1X,    ST_AMMO1Y,     1,  1     ),
  WIDGET2(   "nughud_ammo2",       nughud.ammos[2],     ST_AMMO2X,    ST_AMMO2Y,     1,  1     ),
  WIDGET2(   "nughud_ammo3",       nughud.ammos[3],     ST_AMMO3X,    ST_AMMO3Y,     1,  1     ),
  WIDGET2(   "nughud_maxammo0",    nughud.maxammos[0],  ST_MAXAMMO0X, ST_MAXAMMO0Y,  1,  1     ),
  WIDGET2(   "nughud_maxammo1",    nughud.maxammos[1],  ST_MAXAMMO1X, ST_MAXAMMO1Y,  1,  1     ),
  WIDGET2(   "nughud_maxammo2",    nughud.maxammos[2],  ST_MAXAMMO2X, ST_MAXAMMO2Y,  1,  1     ),
  WIDGET2(   "nughud_maxammo3",    nughud.maxammos[3],  ST_MAXAMMO3X, ST_MAXAMMO3Y,  1,  1     ),
  TEXTLINE(  "nughud_time",        nughud.time,         2,            151,          -2, -1     ),
  TOGGLE(    "nughud_time_sts",    nughud.time_sts,     1                                      ),
  TEXTLINE(  "nughud_sts",         nughud.sts,          2,            159,          -2, -1     ),
  TOGGLE(    "nughud_sts_ml",      nughud.sts_ml,       0                                      ),
  TEXTLINE(  "nughud_title",       nughud.title,        2,            143,          -2, -1     ),
  TEXTLINE(  "nughud_powers",      nughud.powers,       318,          8,             2,  1     ),
  TEXTLINE(  "nughud_coord",       nughud.coord,        318,          16,            2,  1     ),
  TOGGLE(    "nughud_coord_ml",    nughud.coord_ml,     0                                      ),
  TEXTLINE(  "nughud_fps",         nughud.fps,          318,          24,            2,  1     ),
  {          "nughud_message_x",     (config_t *)&nughud.message.x,     NULL, { -1 }, { -1, 320 }, number },
  {          "nughud_message_y",     (config_t *)&nughud.message.y,     NULL, {  0 }, {  0, 200 }, number },
  {          "nughud_message_wide",  (config_t *)&nughud.message.wide,  NULL, { -2 }, { -2, 2 },   number },
  {          "nughud_message_align", (config_t *)&nughud.message.align, NULL, { -1 }, { -1, 1 },   number },
  TEXTLINE(  "nughud_secret",      nughud.secret,       160,          84,            0,  0     ),
  PATCH(     "nughud_patch1",      0                                                           ),
  PATCH(     "nughud_patch2",      1                                                           ),
  PATCH(     "nughud_patch3",      2                                                           ),
  PATCH(     "nughud_patch4",      3                                                           ),
  PATCH(     "nughud_patch5",      4                                                           ),
  PATCH(     "nughud_patch6",      5                                                           ),
  PATCH(     "nughud_patch7",      6                                                           ),
  PATCH(     "nughud_patch8",      7                                                           ),

  TOGGLE(    "nughud_percents",       nughud.percents,       1                                 ),
  TOGGLE(    "nughud_patch_offsets",  nughud.patch_offsets,  1                                 ),
  {          "nughud_weapheight",     (config_t *)&nughud.weapheight, NULL, {  0 }, { -32, 32 }, number },
  {          "nughud_viewoffset",     (config_t *)&nughud.viewoffset, NULL, {  0 }, { -16, 16 }, number },

  { NULL }         // last entry
};

#define NUMNUGHUDDEFAULTS ((unsigned)(sizeof nughud_defaults / sizeof *nughud_defaults - 1))

// killough 11/98: hash function for name lookup
static unsigned nughud_default_hash(const char *name)
{
  unsigned hash = 0;
  while (*name)
    hash = hash*2 + toupper(*name++);
  return hash % NUMNUGHUDDEFAULTS;
}


static default_t *M_NughudLookupDefault(const char *name)
{
  static int hash_init;
  register default_t *dp;

  // Initialize hash table if not initialized already
  if (!hash_init)
    for (hash_init = 1, dp = nughud_defaults; dp->name; dp++)
    {
      unsigned h = nughud_default_hash(dp->name);
      dp->next = nughud_defaults[h].first;
      nughud_defaults[h].first = dp;
    }

  // Look up name in hash table
  for (dp = nughud_defaults[nughud_default_hash(name)].first;
       dp && strcasecmp(name, dp->name);
       dp = dp->next);

  return dp;
}


static boolean M_NughudParseOption(const char *p, boolean wad)
{
  char name[80], strparm[1024];
  default_t *dp;
  int parm;

  while (isspace(*p)) { p++; } // killough 10/98: skip leading whitespace

  //jff 3/3/98 skip lines not starting with an alphanum
  // killough 10/98: move to be made part of main test, add comment-handling

  if (sscanf(p, "%79s %1023[^\n]", name, strparm) != 2 || !isalnum(*name)
      || !(dp = M_NughudLookupDefault(name))
      || (*strparm == '"') == (dp->type != string))
  { return 1; }

  if (dp->type == string)     // get a string default
  {
    int len = strlen(strparm)-1;

    while (isspace(strparm[len])) { len--; }

    if (strparm[len] == '"') { len--; }

    strparm[len+1] = 0;

    if (wad && !dp->modified) {             // Modified by wad
      dp->modified = 1;                     // Mark it as modified
      dp->orig_default.s = dp->location->s; // Save original default
    }
    else { free(dp->location->s); } // Free old value

    dp->location->s = strdup(strparm+1); // Change default value

    if (dp->current) {                    // Current value
      free(dp->current->s);               // Free old value
      dp->current->s = strdup(strparm+1); // Change current value
    }
  }
  else if (dp->type == number) {
    if (sscanf(strparm, "%i", &parm) != 1) { return 1; } // Not A Number

    //jff 3/4/98 range check numeric parameters
    if (   (dp->limit.min == UL || dp->limit.min <= parm)
        && (dp->limit.max == UL || dp->limit.max >= parm))
    {
      if (wad)
      {
        if (!dp->modified) { // First time it's modified by wad
          dp->modified = 1;                     // Mark it as modified
          dp->orig_default.i = dp->location->i; // Save original default
        }

        if (dp->current) // Change current value
        { dp->current->i = parm; }
      }

      dp->location->i = parm;          // Change default
    }
  }

  if (wad && dp->setup_menu) { dp->setup_menu->m_flags |= S_DISABLE; }

  return 0; // Success
}


void M_NughudLoadOptions(void)
{
  int lump;

  if ((lump = W_CheckNumForName("NUGHUD")) != -1)
  {
    int size = W_LumpLength(lump), buflen = 0;
    char *buf = NULL, *p, *options = p = W_CacheLumpNum(lump, PU_STATIC);

    while (size > 0)
    {
      int len = 0;
      
      while (len < size && p[len++] && p[len-1] != '\n');

      if (len >= buflen) { buf = I_Realloc(buf, buflen = len+1); }
      
      strncpy(buf, p, len)[len] = 0;
      p += len;
      size -= len;

      M_NughudParseOption(buf, true);
    }
    
    free(buf);
    Z_ChangeTag(options, PU_CACHE);
  }
}


void M_NughudLoadDefaults (void)
{
  register default_t *dp;

  for (dp = nughud_defaults; dp->name; dp++)
    if (dp->type == string && dp->defaultvalue.s)
    { dp->location->s = strdup(dp->defaultvalue.s); }
    else if (dp->type == number)
    { dp->location->i = dp->defaultvalue.i; }
}
