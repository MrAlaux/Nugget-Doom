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
//  Variant of m_misc.c specifically for declaration and loading of NUGHUD
//  variables

#include "m_menu.h"
#include "w_wad.h"
#include "i_video.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "m_misc.h"
#include "m_nughud.h"

#include "m_io.h"
#include <errno.h>

nughud_t nughud; // Behold!!!

//
// DEFAULTS
//

default_t nughud_defaults[] = {
  { "nughud_ammo_x",        (config_t *)&nughud.ammo.x,           NULL, {ST_AMMOX},       {-1,320}, number },
  { "nughud_ammo_y",        (config_t *)&nughud.ammo.y,           NULL, {ST_AMMOY},       {0,200},  number },
  { "nughud_ammo_wide",     (config_t *)&nughud.ammo.wide,        NULL, {-1},             {-1,1},   number },
  { "nughud_health_x",      (config_t *)&nughud.health.x,         NULL, {ST_HEALTHX},     {-1,320}, number },
  { "nughud_health_y",      (config_t *)&nughud.health.y,         NULL, {ST_HEALTHY},     {0,200},  number },
  { "nughud_health_wide",   (config_t *)&nughud.health.wide,      NULL, {-1},             {-1,1},   number },
  { "nughud_arms2_x",       (config_t *)&nughud.arms[0].x,        NULL, {111},            {-1,320}, number },
  { "nughud_arms2_y",       (config_t *)&nughud.arms[0].y,        NULL, {172},            {0,200},  number },
  { "nughud_arms2_wide",    (config_t *)&nughud.arms[0].wide,     NULL, {-1},             {-1,1},   number },
  { "nughud_arms3_x",       (config_t *)&nughud.arms[1].x,        NULL, {119},            {-1,320}, number },
  { "nughud_arms3_y",       (config_t *)&nughud.arms[1].y,        NULL, {172},            {0,200},  number },
  { "nughud_arms3_wide",    (config_t *)&nughud.arms[1].wide,     NULL, {-1},             {-1,1},   number },
  { "nughud_arms4_x",       (config_t *)&nughud.arms[2].x,        NULL, {127},            {-1,320}, number },
  { "nughud_arms4_y",       (config_t *)&nughud.arms[2].y,        NULL, {172},            {0,200},  number },
  { "nughud_arms4_wide",    (config_t *)&nughud.arms[2].wide,     NULL, {-1},             {-1,1},   number },
  { "nughud_arms5_x",       (config_t *)&nughud.arms[3].x,        NULL, {135},            {-1,320}, number },
  { "nughud_arms5_y",       (config_t *)&nughud.arms[3].y,        NULL, {172},            {0,200},  number },
  { "nughud_arms5_wide",    (config_t *)&nughud.arms[3].wide,     NULL, {-1},             {-1,1},   number },
  { "nughud_arms6_x",       (config_t *)&nughud.arms[4].x,        NULL, {111},            {-1,320}, number },
  { "nughud_arms6_y",       (config_t *)&nughud.arms[4].y,        NULL, {182},            {0,200},  number },
  { "nughud_arms6_wide",    (config_t *)&nughud.arms[4].wide,     NULL, {-1},             {-1,1},   number },
  { "nughud_arms7_x",       (config_t *)&nughud.arms[5].x,        NULL, {119},            {-1,320}, number },
  { "nughud_arms7_y",       (config_t *)&nughud.arms[5].y,        NULL, {182},            {0,200},  number },
  { "nughud_arms7_wide",    (config_t *)&nughud.arms[5].wide,     NULL, {-1},             {-1,1},   number },
  { "nughud_arms8_x",       (config_t *)&nughud.arms[6].x,        NULL, {127},            {-1,320}, number },
  { "nughud_arms8_y",       (config_t *)&nughud.arms[6].y,        NULL, {182},            {0,200},  number },
  { "nughud_arms8_wide",    (config_t *)&nughud.arms[6].wide,     NULL, {-1},             {-1,1},   number },
  { "nughud_arms9_x",       (config_t *)&nughud.arms[7].x,        NULL, {135},            {-1,320}, number },
  { "nughud_arms9_y",       (config_t *)&nughud.arms[7].y,        NULL, {182},            {0,200},  number },
  { "nughud_arms9_wide",    (config_t *)&nughud.arms[7].wide,     NULL, {-1},             {-1,1},   number },
  { "nughud_frags_x",       (config_t *)&nughud.frags.x,          NULL, {174},            {-1,320}, number },
  { "nughud_frags_y",       (config_t *)&nughud.frags.y,          NULL, {171},            {0,200},  number },
  { "nughud_frags_wide",    (config_t *)&nughud.frags.wide,       NULL, {0},              {-1,1},   number },
  { "nughud_face_x",        (config_t *)&nughud.face.x,           NULL, {-1},             {-1,320}, number },
  { "nughud_face_y",        (config_t *)&nughud.face.y,           NULL, {ST_FACESY},      {0,200},  number },
  { "nughud_face_wide",     (config_t *)&nughud.face.wide,        NULL, {0},              {-1,1},   number },
  { "nughud_face_bg",       (config_t *)&nughud.face_bg,          NULL, {1},              {0,1},    number },
  { "nughud_armor_x",       (config_t *)&nughud.armor.x,          NULL, {ST_ARMORX},      {-1,320}, number },
  { "nughud_armor_y",       (config_t *)&nughud.armor.y,          NULL, {ST_ARMORY},      {0,200},  number },
  { "nughud_armor_wide",    (config_t *)&nughud.armor.wide,       NULL, {1},              {-1,1},   number },
  { "nughud_key0_x",        (config_t *)&nughud.keys[0].x,        NULL, {ST_KEY0X},       {-1,320}, number },
  { "nughud_key0_y",        (config_t *)&nughud.keys[0].y,        NULL, {ST_KEY0Y},       {0,200},  number },
  { "nughud_key0_wide",     (config_t *)&nughud.keys[0].wide,     NULL, {1},              {-1,1},   number },
  { "nughud_key1_x",        (config_t *)&nughud.keys[1].x,        NULL, {ST_KEY1X},       {-1,320}, number },
  { "nughud_key1_y",        (config_t *)&nughud.keys[1].y,        NULL, {ST_KEY1Y},       {0,200},  number },
  { "nughud_key1_wide",     (config_t *)&nughud.keys[1].wide,     NULL, {1},              {-1,1},   number },
  { "nughud_key2_x",        (config_t *)&nughud.keys[2].x,        NULL, {ST_KEY2X},       {-1,320}, number },
  { "nughud_key2_y",        (config_t *)&nughud.keys[2].y,        NULL, {ST_KEY2Y},       {0,200},  number },
  { "nughud_key2_wide",     (config_t *)&nughud.keys[2].wide,     NULL, {1},              {-1,1},   number },
  { "nughud_ammo0_x",       (config_t *)&nughud.ammos[0].x,       NULL, {ST_AMMO0X},      {-1,320}, number },
  { "nughud_ammo0_y",       (config_t *)&nughud.ammos[0].y,       NULL, {ST_AMMO0Y},      {0,200},  number },
  { "nughud_ammo0_wide",    (config_t *)&nughud.ammos[0].wide,    NULL, {1},              {-1,1},   number },
  { "nughud_ammo1_x",       (config_t *)&nughud.ammos[1].x,       NULL, {ST_AMMO1X},      {-1,320}, number },
  { "nughud_ammo1_y",       (config_t *)&nughud.ammos[1].y,       NULL, {ST_AMMO1Y},      {0,200},  number },
  { "nughud_ammo1_wide",    (config_t *)&nughud.ammos[1].wide,    NULL, {1},              {-1,1},   number },
  { "nughud_ammo2_x",       (config_t *)&nughud.ammos[2].x,       NULL, {ST_AMMO2X},      {-1,320}, number },
  { "nughud_ammo2_y",       (config_t *)&nughud.ammos[2].y,       NULL, {ST_AMMO2Y},      {0,200},  number },
  { "nughud_ammo2_wide",    (config_t *)&nughud.ammos[2].wide,    NULL, {1},              {-1,1},   number },
  { "nughud_ammo3_x",       (config_t *)&nughud.ammos[3].x,       NULL, {ST_AMMO3X},      {-1,320}, number },
  { "nughud_ammo3_y",       (config_t *)&nughud.ammos[3].y,       NULL, {ST_AMMO3Y},      {0,200},  number },
  { "nughud_ammo3_wide",    (config_t *)&nughud.ammos[3].wide,    NULL, {1},              {-1,1},   number },
  { "nughud_maxammo0_x",    (config_t *)&nughud.maxammos[0].x,    NULL, {ST_MAXAMMO0X},   {-1,320}, number },
  { "nughud_maxammo0_y",    (config_t *)&nughud.maxammos[0].y,    NULL, {ST_MAXAMMO0Y},   {0,200},  number },
  { "nughud_maxammo0_wide", (config_t *)&nughud.maxammos[0].wide, NULL, {1},              {-1,1},   number },
  { "nughud_maxammo1_x",    (config_t *)&nughud.maxammos[1].x,    NULL, {ST_MAXAMMO1X},   {-1,320}, number },
  { "nughud_maxammo1_y",    (config_t *)&nughud.maxammos[1].y,    NULL, {ST_MAXAMMO1Y},   {0,200},  number },
  { "nughud_maxammo1_wide", (config_t *)&nughud.maxammos[1].wide, NULL, {1},              {-1,1},   number },
  { "nughud_maxammo2_x",    (config_t *)&nughud.maxammos[2].x,    NULL, {ST_MAXAMMO2X},   {-1,320}, number },
  { "nughud_maxammo2_y",    (config_t *)&nughud.maxammos[2].y,    NULL, {ST_MAXAMMO2Y},   {0,200},  number },
  { "nughud_maxammo2_wide", (config_t *)&nughud.maxammos[2].wide, NULL, {1},              {-1,1},   number },
  { "nughud_maxammo3_x",    (config_t *)&nughud.maxammos[3].x,    NULL, {ST_MAXAMMO3X},   {-1,320}, number },
  { "nughud_maxammo3_y",    (config_t *)&nughud.maxammos[3].y,    NULL, {ST_MAXAMMO3Y},   {0,200},  number },
  { "nughud_maxammo3_wide", (config_t *)&nughud.maxammos[3].wide, NULL, {1},              {-1,1},   number },
  { "nughud_time_x",        (config_t *)&nughud.time.x,           NULL, {0},              {0,320},  number },
  { "nughud_time_y",        (config_t *)&nughud.time.y,           NULL, {ST_Y-2*HU_GAPY}, {0,200},  number },
  { "nughud_time_wide",     (config_t *)&nughud.time.wide,        NULL, {-1},             {-1,1},   number },
  { "nughud_time_sts",      (config_t *)&nughud.time_sts,         NULL, {1},              {0,1},    number },
  { "nughud_sts_x",         (config_t *)&nughud.sts.x,            NULL, {0},              {0,320},  number },
  { "nughud_sts_y",         (config_t *)&nughud.sts.y,            NULL, {ST_Y-HU_GAPY},   {0,200},  number },
  { "nughud_sts_wide",      (config_t *)&nughud.sts.wide,         NULL, {-1},             {-1,1},   number },
  { "nughud_patch1_x",      (config_t *)&nughud.patches[0].x,     NULL, {0},              {0,320},  number },
  { "nughud_patch1_y",      (config_t *)&nughud.patches[0].y,     NULL, {0},              {0,200},  number },
  { "nughud_patch1_wide",   (config_t *)&nughud.patches[0].wide,  NULL, {0},              {-1,1},   number },
  { "nughud_patch1_name",   (config_t *)&nughud.patches[0].name,  NULL, {.s = NULL},      {0},      string },
  { "nughud_patch2_x",      (config_t *)&nughud.patches[1].x,     NULL, {0},              {0,320},  number },
  { "nughud_patch2_y",      (config_t *)&nughud.patches[1].y,     NULL, {0},              {0,200},  number },
  { "nughud_patch2_wide",   (config_t *)&nughud.patches[1].wide,  NULL, {0},              {-1,1},   number },
  { "nughud_patch2_name",   (config_t *)&nughud.patches[1].name,  NULL, {.s = NULL},      {0},      string },
  { "nughud_patch3_x",      (config_t *)&nughud.patches[2].x,     NULL, {0},              {0,320},  number },
  { "nughud_patch3_y",      (config_t *)&nughud.patches[2].y,     NULL, {0},              {0,200},  number },
  { "nughud_patch3_wide",   (config_t *)&nughud.patches[2].wide,  NULL, {0},              {-1,1},   number },
  { "nughud_patch3_name",   (config_t *)&nughud.patches[2].name,  NULL, {.s = NULL},      {0},      string },
  { "nughud_patch4_x",      (config_t *)&nughud.patches[3].x,     NULL, {0},              {0,320},  number },
  { "nughud_patch4_y",      (config_t *)&nughud.patches[3].y,     NULL, {0},              {0,200},  number },
  { "nughud_patch4_wide",   (config_t *)&nughud.patches[3].wide,  NULL, {0},              {-1,1},   number },
  { "nughud_patch4_name",   (config_t *)&nughud.patches[3].name,  NULL, {.s = NULL},      {0},      string },
  { "nughud_patch5_x",      (config_t *)&nughud.patches[4].x,     NULL, {0},              {0,320},  number },
  { "nughud_patch5_y",      (config_t *)&nughud.patches[4].y,     NULL, {0},              {0,200},  number },
  { "nughud_patch5_wide",   (config_t *)&nughud.patches[4].wide,  NULL, {0},              {-1,1},   number },
  { "nughud_patch5_name",   (config_t *)&nughud.patches[4].name,  NULL, {.s = NULL},      {0},      string },
  { "nughud_patch6_x",      (config_t *)&nughud.patches[5].x,     NULL, {0},              {0,320},  number },
  { "nughud_patch6_y",      (config_t *)&nughud.patches[5].y,     NULL, {0},              {0,200},  number },
  { "nughud_patch6_wide",   (config_t *)&nughud.patches[5].wide,  NULL, {0},              {-1,1},   number },
  { "nughud_patch6_name",   (config_t *)&nughud.patches[5].name,  NULL, {.s = NULL},      {0},      string },
  { "nughud_patch7_x",      (config_t *)&nughud.patches[6].x,     NULL, {0},              {0,320},  number },
  { "nughud_patch7_y",      (config_t *)&nughud.patches[6].y,     NULL, {0},              {0,200},  number },
  { "nughud_patch7_wide",   (config_t *)&nughud.patches[6].wide,  NULL, {0},              {-1,1},   number },
  { "nughud_patch7_name",   (config_t *)&nughud.patches[6].name,  NULL, {.s = NULL},      {0},      string },
  { "nughud_patch8_x",      (config_t *)&nughud.patches[7].x,     NULL, {0},              {0,320},  number },
  { "nughud_patch8_y",      (config_t *)&nughud.patches[7].y,     NULL, {0},              {0,200},  number },
  { "nughud_patch8_wide",   (config_t *)&nughud.patches[7].wide,  NULL, {0},              {-1,1},   number },
  { "nughud_patch8_name",   (config_t *)&nughud.patches[7].name,  NULL, {.s = NULL},      {0},      string },
  { "nughud_weapheight",    (config_t *)&nughud.weapheight,       NULL, {0},              {0,200},  number },

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
    else
      { free(dp->location->s); } // Free old value

    dp->location->s = strdup(strparm+1); // Change default value

    if (dp->current) {                    // Current value
      free(dp->current->s);               // Free old value
      dp->current->s = strdup(strparm+1); // Change current value
    }
  }
  else if (dp->type == number) {
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
    if (dp->type == string)
      { dp->location->s = strdup(dp->defaultvalue.s); }
    else if (dp->type == number)
      { dp->location->i = dp->defaultvalue.i; }
}
