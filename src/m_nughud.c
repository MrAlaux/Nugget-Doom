//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//  Copyright(C) 2020-2021 Fabian Greffrath
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
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
//
// DESCRIPTION:
//  Variant of m_misc.c specifically for loading of NUGHUD variables

#include "doomstat.h"
#include "doomkeys.h"
#include "m_argv.h"
#include "g_game.h"
#include "m_menu.h"
#include "am_map.h"
#include "w_wad.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "dstrings.h"
#include "m_misc.h"
#include "m_misc2.h"
#include "m_swap.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_main.h"
#include "r_draw.h" // [FG] fuzzcolumn_mode
#include "r_sky.h" // [FG] stretchsky
#include "hu_lib.h" // HU_MAXMESSAGES
#include "m_nughud.h"

#include "m_io.h"
#include <errno.h>

//
// DEFAULTS
//

default_t nughud_defaults[] = {
  { "nughud_ammo_x",        (config_t *)&nughud_ammo_x,         NULL, {ST_AMMOX},     {-1,320},   number, ss_none, wad_yes },
  { "nughud_ammo_y",        (config_t *)&nughud_ammo_y,         NULL, {ST_AMMOY},     {0,200},    number, ss_none, wad_yes },
  { "nughud_ammo_wide",     (config_t *)&nughud_ammo_wide,      NULL, {-1},           {-1,1},     number, ss_none, wad_yes },
  { "nughud_health_x",      (config_t *)&nughud_health_x,       NULL, {ST_HEALTHX},   {-1,320},   number, ss_none, wad_yes },
  { "nughud_health_y",      (config_t *)&nughud_health_y,       NULL, {ST_HEALTHY},   {0,200},    number, ss_none, wad_yes },
  { "nughud_health_wide",   (config_t *)&nughud_health_wide,    NULL, {-1},           {-1,1},     number, ss_none, wad_yes },
  { "nughud_arms2_x",       (config_t *)&nughud_arms2_x,        NULL, {111},          {-1,320},   number, ss_none, wad_yes },
  { "nughud_arms2_y",       (config_t *)&nughud_arms2_y,        NULL, {172},          {0,200},    number, ss_none, wad_yes },
  { "nughud_arms2_wide",    (config_t *)&nughud_arms2_wide,     NULL, {-1},           {-1,1},     number, ss_none, wad_yes },
  { "nughud_arms3_x",       (config_t *)&nughud_arms3_x,        NULL, {119},          {-1,320},   number, ss_none, wad_yes },
  { "nughud_arms3_y",       (config_t *)&nughud_arms3_y,        NULL, {172},          {0,200},    number, ss_none, wad_yes },
  { "nughud_arms3_wide",    (config_t *)&nughud_arms3_wide,     NULL, {-1},           {-1,1},     number, ss_none, wad_yes },
  { "nughud_arms4_x",       (config_t *)&nughud_arms4_x,        NULL, {127},          {-1,320},   number, ss_none, wad_yes },
  { "nughud_arms4_y",       (config_t *)&nughud_arms4_y,        NULL, {172},          {0,200},    number, ss_none, wad_yes },
  { "nughud_arms4_wide",    (config_t *)&nughud_arms4_wide,     NULL, {-1},           {-1,1},     number, ss_none, wad_yes },
  { "nughud_arms5_x",       (config_t *)&nughud_arms5_x,        NULL, {135},          {-1,320},   number, ss_none, wad_yes },
  { "nughud_arms5_y",       (config_t *)&nughud_arms5_y,        NULL, {172},          {0,200},    number, ss_none, wad_yes },
  { "nughud_arms5_wide",    (config_t *)&nughud_arms5_wide,     NULL, {-1},           {-1,1},     number, ss_none, wad_yes },
  { "nughud_arms6_x",       (config_t *)&nughud_arms6_x,        NULL, {111},          {-1,320},   number, ss_none, wad_yes },
  { "nughud_arms6_y",       (config_t *)&nughud_arms6_y,        NULL, {182},          {0,200},    number, ss_none, wad_yes },
  { "nughud_arms6_wide",    (config_t *)&nughud_arms6_wide,     NULL, {-1},           {-1,1},     number, ss_none, wad_yes },
  { "nughud_arms7_x",       (config_t *)&nughud_arms7_x,        NULL, {119},          {-1,320},   number, ss_none, wad_yes },
  { "nughud_arms7_y",       (config_t *)&nughud_arms7_y,        NULL, {182},          {0,200},    number, ss_none, wad_yes },
  { "nughud_arms7_wide",    (config_t *)&nughud_arms7_wide,     NULL, {-1},           {-1,1},     number, ss_none, wad_yes },
  { "nughud_arms8_x",       (config_t *)&nughud_arms8_x,        NULL, {127},          {-1,320},   number, ss_none, wad_yes },
  { "nughud_arms8_y",       (config_t *)&nughud_arms8_y,        NULL, {182},          {0,200},    number, ss_none, wad_yes },
  { "nughud_arms8_wide",    (config_t *)&nughud_arms8_wide,     NULL, {-1},           {-1,1},     number, ss_none, wad_yes },
  { "nughud_arms9_x",       (config_t *)&nughud_arms9_x,        NULL, {135},          {-1,320},   number, ss_none, wad_yes },
  { "nughud_arms9_y",       (config_t *)&nughud_arms9_y,        NULL, {182},          {0,200},    number, ss_none, wad_yes },
  { "nughud_arms9_wide",    (config_t *)&nughud_arms9_wide,     NULL, {-1},           {-1,1},     number, ss_none, wad_yes },
  { "nughud_frags_x",       (config_t *)&nughud_frags_x,        NULL, {ST_FRAGSX},    {-1,320},   number, ss_none, wad_yes },
  { "nughud_frags_y",       (config_t *)&nughud_frags_y,        NULL, {ST_FRAGSY},    {0,200},    number, ss_none, wad_yes },
  { "nughud_frags_wide",    (config_t *)&nughud_frags_wide,     NULL, {-1},           {-1,1},     number, ss_none, wad_yes },
  { "nughud_face_x",        (config_t *)&nughud_face_x,         NULL, {-1},           {-1,320},   number, ss_none, wad_yes },
  { "nughud_face_y",        (config_t *)&nughud_face_y,         NULL, {0},            {0,200},    number, ss_none, wad_yes },
  { "nughud_face_wide",     (config_t *)&nughud_face_wide,      NULL, {0},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_face_bg",       (config_t *)&nughud_face_bg,        NULL, {1},            {0,1},      number, ss_none, wad_yes },
  { "nughud_armor_x",       (config_t *)&nughud_armor_x,        NULL, {ST_ARMORX},    {-1,320},   number, ss_none, wad_yes },
  { "nughud_armor_y",       (config_t *)&nughud_armor_y,        NULL, {ST_ARMORY},    {0,200},    number, ss_none, wad_yes },
  { "nughud_armor_wide",    (config_t *)&nughud_armor_wide,     NULL, {1},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_key0_x",        (config_t *)&nughud_key0_x,         NULL, {ST_KEY0X},     {-1,320},   number, ss_none, wad_yes },
  { "nughud_key0_y",        (config_t *)&nughud_key0_y,         NULL, {ST_KEY0Y},     {0,200},    number, ss_none, wad_yes },
  { "nughud_key0_wide",     (config_t *)&nughud_key0_wide,      NULL, {1},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_key1_x",        (config_t *)&nughud_key1_x,         NULL, {ST_KEY1X},     {-1,320},   number, ss_none, wad_yes },
  { "nughud_key1_y",        (config_t *)&nughud_key1_y,         NULL, {ST_KEY1Y},     {0,200},    number, ss_none, wad_yes },
  { "nughud_key1_wide",     (config_t *)&nughud_key1_wide,      NULL, {1},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_key2_x",        (config_t *)&nughud_key2_x,         NULL, {ST_KEY2X},     {-1,320},   number, ss_none, wad_yes },
  { "nughud_key2_y",        (config_t *)&nughud_key2_y,         NULL, {ST_KEY2Y},     {0,200},    number, ss_none, wad_yes },
  { "nughud_key2_wide",     (config_t *)&nughud_key2_wide,      NULL, {1},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_ammo0_x",       (config_t *)&nughud_ammo0_x,        NULL, {ST_AMMO0X},    {-1,320},   number, ss_none, wad_yes },
  { "nughud_ammo0_y",       (config_t *)&nughud_ammo0_y,        NULL, {ST_AMMO0Y},    {0,200},    number, ss_none, wad_yes },
  { "nughud_ammo0_wide",    (config_t *)&nughud_ammo0_wide,     NULL, {1},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_ammo1_x",       (config_t *)&nughud_ammo1_x,        NULL, {ST_AMMO1X},    {-1,320},   number, ss_none, wad_yes },
  { "nughud_ammo1_y",       (config_t *)&nughud_ammo1_y,        NULL, {ST_AMMO1Y},    {0,200},    number, ss_none, wad_yes },
  { "nughud_ammo1_wide",    (config_t *)&nughud_ammo1_wide,     NULL, {1},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_ammo2_x",       (config_t *)&nughud_ammo2_x,        NULL, {ST_AMMO2X},    {-1,320},   number, ss_none, wad_yes },
  { "nughud_ammo2_y",       (config_t *)&nughud_ammo2_y,        NULL, {ST_AMMO2Y},    {0,200},    number, ss_none, wad_yes },
  { "nughud_ammo2_wide",    (config_t *)&nughud_ammo2_wide,     NULL, {1},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_ammo3_x",       (config_t *)&nughud_ammo3_x,        NULL, {ST_AMMO3X},    {-1,320},   number, ss_none, wad_yes },
  { "nughud_ammo3_y",       (config_t *)&nughud_ammo3_y,        NULL, {ST_AMMO3Y},    {0,200},    number, ss_none, wad_yes },
  { "nughud_ammo3_wide",    (config_t *)&nughud_ammo3_wide,     NULL, {1},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_maxammo0_x",    (config_t *)&nughud_maxammo0_x,     NULL, {ST_MAXAMMO0X}, {-1,320},   number, ss_none, wad_yes },
  { "nughud_maxammo0_y",    (config_t *)&nughud_maxammo0_y,     NULL, {ST_MAXAMMO0Y}, {0,200},    number, ss_none, wad_yes },
  { "nughud_maxammo0_wide", (config_t *)&nughud_maxammo0_wide,  NULL, {1},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_maxammo1_x",    (config_t *)&nughud_maxammo1_x,     NULL, {ST_MAXAMMO1X}, {-1,320},   number, ss_none, wad_yes },
  { "nughud_maxammo1_y",    (config_t *)&nughud_maxammo1_y,     NULL, {ST_MAXAMMO1Y}, {0,200},    number, ss_none, wad_yes },
  { "nughud_maxammo1_wide", (config_t *)&nughud_maxammo1_wide,  NULL, {1},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_maxammo2_x",    (config_t *)&nughud_maxammo2_x,     NULL, {ST_MAXAMMO2X}, {-1,320},   number, ss_none, wad_yes },
  { "nughud_maxammo2_y",    (config_t *)&nughud_maxammo2_y,     NULL, {ST_MAXAMMO2Y}, {0,200},    number, ss_none, wad_yes },
  { "nughud_maxammo2_wide", (config_t *)&nughud_maxammo2_wide,  NULL, {1},            {-1,1},     number, ss_none, wad_yes },
  { "nughud_maxammo3_x",    (config_t *)&nughud_maxammo3_x,     NULL, {ST_MAXAMMO3X}, {-1,320},   number, ss_none, wad_yes },
  { "nughud_maxammo3_y",    (config_t *)&nughud_maxammo3_y,     NULL, {ST_MAXAMMO3Y}, {0,200},    number, ss_none, wad_yes },
  { "nughud_maxammo3_wide", (config_t *)&nughud_maxammo3_wide,  NULL, {1},            {-1,1},     number, ss_none, wad_yes },

  {NULL}         // last entry
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


default_t *M_NughudLookupDefault(const char *name)
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


boolean M_NughudParseOption(const char *p, boolean wad)
{
  char name[80], strparm[100];
  default_t *dp;
  int parm;

  while (isspace(*p)) { p++; } // killough 10/98: skip leading whitespace

  //jff 3/3/98 skip lines not starting with an alphanum
  // killough 10/98: move to be made part of main test, add comment-handling

  if (sscanf(p, "%79s %99[^\n]", name, strparm) != 2 || !isalnum(*name)
      || !(dp = M_NughudLookupDefault(name))
      || (*strparm == '"') == (dp->type != string)
      || (wad && !dp->wad_allowed))
    { return 1; }

  if (dp->type == number) {
    if (sscanf(strparm, "%i", &parm) != 1) { return 1; } // Not A Number

    if (!strncmp(name, "key_", 4)) { parm = I_ScanCode2DoomCode(parm); } // killough

    //jff 3/4/98 range check numeric parameters
    if ((dp->limit.min == UL || dp->limit.min <= parm)
        && (dp->limit.max == UL || dp->limit.max >= parm))
    {
      if (wad) {
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
      if (len >= buflen)
        buf = realloc(buf, buflen = len+1);
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
    if (dp->type == number)
      { dp->location->i = dp->defaultvalue.i; }
}
