//
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//  Copyright (C) 2023 Fabian Greffrath
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
// DESCRIPTION:  Heads-up displays
//
//-----------------------------------------------------------------------------

// killough 5/3/98: remove unnecessary headers

#include <stdlib.h>
#include <string.h>

#include "d_deh.h" /* Ty 03/27/98 - externalization of mapnamesx arrays */
#include "d_event.h"
#include "d_items.h"
#include "d_player.h"
#include "doomkeys.h"
#include "doomstat.h"
#include "dstrings.h"
#include "hu_lib.h"
#include "hu_obituary.h"
#include "hu_stuff.h"
#include "i_video.h" // fps
#include "m_fixed.h"
#include "m_input.h"
#include "m_misc.h"
#include "m_swap.h"
#include "p_map.h" // crosshair (linetarget)
#include "p_mobj.h"
#include "r_data.h"
#include "r_defs.h"
#include "r_main.h"
#include "r_state.h"
#include "r_voxel.h"
#include "s_sound.h"
#include "sounds.h"
#include "st_stuff.h" /* jff 2/16/98 need loc of status bar */
#include "tables.h"
#include "u_mapinfo.h"
#include "u_scanner.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

// [Nugget]
#include "am_map.h"
#include "m_nughud.h"

// global heads up display controls

int hud_active;       //jff 2/17/98 controls heads-up display mode 
int hud_displayed;    //jff 2/23/98 turns heads-up display on/off
secretmessage_t hud_secret_message; // "A secret is revealed!" message
int hud_widget_font;
int hud_widget_layout;

int hud_slot; // [Nugget]

int hud_type; // Crispy HUD or Boom variants
boolean draw_crispy_hud;

//
// Locally used constants, shortcuts.
//
// Ty 03/28/98 -
// These four shortcuts modifed to reflect char ** of mapnamesx[]
#define HU_TITLE  (*mapnames[(gameepisode-1)*9+gamemap-1])
#define HU_TITLE2 (*mapnames2[gamemap-1])
#define HU_TITLEP (*mapnamesp[gamemap-1])
#define HU_TITLET (*mapnamest[gamemap-1])

char *chat_macros[] =    // Ty 03/27/98 - *not* externalized
{
  HUSTR_CHATMACRO0,
  HUSTR_CHATMACRO1,
  HUSTR_CHATMACRO2,
  HUSTR_CHATMACRO3,
  HUSTR_CHATMACRO4,
  HUSTR_CHATMACRO5,
  HUSTR_CHATMACRO6,
  HUSTR_CHATMACRO7,
  HUSTR_CHATMACRO8,
  HUSTR_CHATMACRO9
};

char **player_names[] =
{
  &s_HUSTR_PLRGREEN,
  &s_HUSTR_PLRINDIGO,
  &s_HUSTR_PLRBROWN,
  &s_HUSTR_PLRRED
};

//jff 3/17/98 translate player colmap to text color ranges
int plyrcoltran[MAXPLAYERS]={CR_GREEN,CR_GRAY,CR_BROWN,CR_RED};

static player_t *plr;

// font sets
static hu_font_t big_font = {.space_width = 4, .tab_width = 15, .tab_mask = ~15},
                 sml_font = {.space_width = 5, .tab_width =  7, .tab_mask =  ~7};
static hu_font_t *doom_font = &big_font, *boom_font = &sml_font;
patch_t **hu_font = big_font.patches;

static int CR_BLUE = CR_BLUE1;

// widgets

// [FG] Vanilla widgets point to a boolean variable (*on) to determine
//      if they are enabled, always big_font, mostly left-aligned
static hu_multiline_t w_title;
static hu_multiline_t w_message;
static hu_multiline_t w_chat;
static hu_multiline_t w_secret; // [crispy] secret message widget

// [FG] special pony, per-player chat input buffer
static hu_line_t w_inputbuffer[MAXPLAYERS];

// [FG] Boom widgets are built using builder() functions
static hu_multiline_t w_ammo;   //jff 2/16/98 new ammo widget for hud
static hu_multiline_t w_armor;  //jff 2/16/98 new armor widget for hud
static hu_multiline_t w_health; //jff 2/16/98 new health widget for hud
static hu_multiline_t w_keys;   //jff 2/16/98 new keys widget for hud
static hu_multiline_t w_weapon; //jff 2/16/98 new weapon widget for hud

// [FG] extra Boom widgets, that need to be explicitly enabled
static hu_multiline_t w_monsec; //jff 2/16/98 new kill/secret widget for hud
static hu_multiline_t w_sttime; // time above status bar
static hu_multiline_t w_coord;
static hu_multiline_t w_fps;
static hu_multiline_t w_rate;

// [Nugget] 
static hu_multiline_t w_powers; // Powerup timers

#define MAX_HUDS 3
#define MAX_WIDGETS (16 + 1) // [Nugget] Accommodate more widgets

#define NUGHUDSLOT 3 // [Nugget] NUGHUD

// [Nugget] Extra slot for NUGHUD
static hu_widget_t widgets[MAX_HUDS+1][MAX_WIDGETS];

static void HU_ParseHUD (void);

static char       chat_dest[MAXPLAYERS];
boolean           chat_on;
static boolean    message_on;
static boolean    has_message;       // killough 12/98
boolean           message_dontfuckwithme;
static boolean    message_nottobefuckedwith;
static int        message_counter;
static int        message_count;     // killough 11/98
static int        chat_count;        // killough 11/98
static boolean    secret_on;
static int        secret_counter;

boolean           message_centered;
boolean           message_colorized;

extern int        showMessages;
static boolean    headsupactive = false;

//jff 2/16/98 hud supported automap colors added
int hudcolor_titl;  // color range of automap level title
int hudcolor_xyco;  // color range of new coords on automap
//jff 2/16/98 hud text colors, controls added
int hudcolor_mesg;  // color range of scrolling messages
int hudcolor_chat;  // color range of chat lines
int hud_msg_lines;  // number of message lines in window
int message_list;      // killough 11/98: made global

// [Nugget] Restore message scroll direction toggle
int hud_msg_scrollup;  // killough 11/98: allow messages to scroll upwards

int message_timer  = HU_MSGTIMEOUT * (1000/TICRATE);     // killough 11/98
int chat_msg_timer = HU_MSGTIMEOUT * (1000/TICRATE);     // killough 11/98

static void HU_InitCrosshair(void);
void HU_StartCrosshair(void); // [Nugget] Not static anymore
boolean hud_crosshair_on; // [Nugget] Replace with crosshair toggle

//
// Builtin map names.
// The actual names can be found in DStrings.h.
//
// Ty 03/27/98 - externalized map name arrays - now in d_deh.c
// and converted to arrays of pointers to char *
// See modified HUTITLEx macros
extern char **mapnames[];
extern char **mapnames2[];
extern char **mapnamesp[];
extern char **mapnamest[];

// key tables
// jff 5/10/98 french support removed, 
// as it was not being used and couldn't be easily tested
//
const char shiftxform[] =
{
  0,
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
  21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
  31,
  ' ', '!', '"', '#', '$', '%', '&',
  '"', // shift-'
  '(', ')', '*', '+',
  '<', // shift-,
  '_', // shift--
  '>', // shift-.
  '?', // shift-/
  ')', // shift-0
  '!', // shift-1
  '@', // shift-2
  '#', // shift-3
  '$', // shift-4
  '%', // shift-5
  '^', // shift-6
  '&', // shift-7
  '*', // shift-8
  '(', // shift-9
  ':',
  ':', // shift-;
  '<',
  '+', // shift-=
  '>', '?', '@',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '[', // shift-[
  '!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
  ']', // shift-]
  '"', '_',
  '\'', // shift-`
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '{', '|', '}', '~', 127
};

static boolean VANILLAMAP(int e, int m)
{
  if (gamemode == commercial)
    return (e == 1 && m > 0 && m <=32);
  else
    return (e > 0 && e <= 4 && m > 0 && m <= 9);
}

struct {
  char **str;
  const int cr;
  const char *col;
} static const colorize_strings[] = {
  // [Woof!] colorize keycard and skull key messages
  {&s_GOTBLUECARD, CR_BLUE2, " blue "},
  {&s_GOTBLUESKUL, CR_BLUE2, " blue "},
  {&s_GOTREDCARD,  CR_RED,  " red "},
  {&s_GOTREDSKULL, CR_RED,  " red "},
  {&s_GOTYELWCARD, CR_GOLD, " yellow "},
  {&s_GOTYELWSKUL, CR_GOLD, " yellow "},
  {&s_PD_BLUEC,    CR_BLUE2, " blue "},
  {&s_PD_BLUEK,    CR_BLUE2, " blue "},
  {&s_PD_BLUEO,    CR_BLUE2, " blue "},
  {&s_PD_BLUES,    CR_BLUE2, " blue "},
  {&s_PD_REDC,     CR_RED,  " red "},
  {&s_PD_REDK,     CR_RED,  " red "},
  {&s_PD_REDO,     CR_RED,  " red "},
  {&s_PD_REDS,     CR_RED,  " red "},
  {&s_PD_YELLOWC,  CR_GOLD, " yellow "},
  {&s_PD_YELLOWK,  CR_GOLD, " yellow "},
  {&s_PD_YELLOWO,  CR_GOLD, " yellow "},
  {&s_PD_YELLOWS,  CR_GOLD, " yellow "},

  // [Woof!] colorize multi-player messages
  {&s_HUSTR_PLRGREEN,  CR_GREEN, "Green: "},
  {&s_HUSTR_PLRINDIGO, CR_GRAY,  "Indigo: "},
  {&s_HUSTR_PLRBROWN,  CR_BROWN, "Brown: "},
  {&s_HUSTR_PLRRED,    CR_RED,   "Red: "},
};

static char* PrepareColor(const char *str, const char *col)
{
    char *str_replace, col_replace[16];

    M_snprintf(col_replace, sizeof(col_replace),
               "\x1b%c%s\x1b%c", '0'+CR_ORIG, col, '0'+CR_ORIG);
    str_replace = M_StringReplace(str, col, col_replace);

    return str_replace;
}

static void UpdateColor(char *str, int cr)
{
    int i;
    int len = strlen(str);

    if (!message_colorized)
    {
        cr = CR_ORIG;
    }

    for (i = 0; i < len; ++i)
    {
        if (str[i] == '\x1b' && i + 1 < len)
        {
          str[i + 1] = '0'+cr;
          break;
        }
    }
}

void HU_ResetMessageColors(void)
{
    int i;

    for (i = 0; i < arrlen(colorize_strings); i++)
    {
        UpdateColor(*colorize_strings[i].str, colorize_strings[i].cr);
    }
}

extern boolean st_invul;

static byte* ColorByHealth(int health, int maxhealth, boolean invul)
{
  if (invul)
    return colrngs[CR_GRAY];

  health = 100 * health / maxhealth;

  if (health < health_red)
    return colrngs[CR_RED];
  else if (health < health_yellow)
    return colrngs[CR_GOLD];
  else if (health <= health_green)
    return colrngs[CR_GREEN];
  else
    return colrngs[CR_BLUE];
}

// [FG] support centered player messages

static void HU_set_centered_message(void)
{
  int i, j;

  for (i = 0; i < MAX_HUDS+1; i++) // [Nugget] Extra slot for NUGHUD
  {
    hu_widget_t *const w = widgets[i];

    for (j = 0; w[j].multiline; j++)
    {
      if (w[j].multiline == &w_message)
      {
        w[j].h_align = message_centered ? align_center : w[j].h_align_orig;
      }
    }
  }
}

//
// HU_Init()
//
// Initialize the heads-up display, text that overwrites the primary display
//
// Passed nothing, returns nothing
//
void HU_Init(void)
{
  int i, j;
  char buffer[9];

  // load the heads-up font
  for (i = 0, j = HU_FONTSTART; i < HU_FONTSIZE; i++, j++)
  {
    M_snprintf(buffer, sizeof(buffer), "STCFN%.3d", j);
    if (W_CheckNumForName(buffer) != -1)
      big_font.patches[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);

    if ('0' <= j && j <= '9')
    {
      M_snprintf(buffer, sizeof(buffer), "DIG%.1d", j - 48);
      sml_font.patches[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }
    else if ('A' <= j && j <= 'Z')
    {
      M_snprintf(buffer, sizeof(buffer), "DIG%c", j);
      sml_font.patches[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }
    else if (j > 122)
    {
      M_snprintf(buffer, sizeof(buffer), "STBR%.3d", j);
      sml_font.patches[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }
    else
    {
      M_snprintf(buffer, sizeof(buffer), "DIG%.2d", j);
      if (W_CheckNumForName(buffer) != -1)
        sml_font.patches[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

    // [FG] small font available, big font unavailable
    if (big_font.patches[i] == NULL && sml_font.patches[i] != NULL)
    {
      big_font.patches[i] = sml_font.patches[i];
    }
    // [FG] big font available, small font unavailable
    else if (big_font.patches[i] != NULL && sml_font.patches[i] == NULL)
    {
      sml_font.patches[i] = big_font.patches[i];
    }
    // [FG] both fonts unavailable, fall back to '!'
    else if (big_font.patches[i] == NULL && sml_font.patches[i] == NULL)
    {
      sml_font.patches[i] =
      big_font.patches[i] = big_font.patches[0];
    }
  }

  //jff 2/26/98 load patches for keys and double keys
  for (i = HU_FONTSIZE, j = 0; j < 6; i++, j++)
  {
    M_snprintf(buffer, sizeof(buffer), "STKEYS%.1d", j);
    sml_font.patches[i] =
    big_font.patches[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
  }

  // [Nugget] Load Stats icons
  for (i = HU_FONTSIZE + 6, j = 0; j < 3; i++, j++)
  {
    static const char *names[] = { "HUDKILLS", "HUDITEMS", "HUDSCRTS" };
    const char *icon = names[j];

    if (W_CheckNumForName(icon) != -1)
    {
      sml_font.patches[i] =
      big_font.patches[i] = (patch_t *) W_CacheLumpName(icon, PU_STATIC);
    }
    else { sml_font.patches[i] = big_font.patches[i] = NULL; }
  }

  // [FG] calculate font height once right here
  sml_font.line_height = SHORT(sml_font.patches['A'-HU_FONTSTART]->height) + 1;
  big_font.line_height = SHORT(big_font.patches['A'-HU_FONTSTART]->height) + 1;

  // [FG] support crosshair patches from extras.wad
  HU_InitCrosshair();

  HU_InitObituaries();

  HU_ParseHUD();
  HU_set_centered_message();

  // [Woof!] prepare player messages for colorization
  for (i = 0; i < arrlen(colorize_strings); i++)
  {
    *colorize_strings[i].str = PrepareColor(*colorize_strings[i].str, colorize_strings[i].col);
  }

  HU_ResetMessageColors();

  // [Nugget] ----------------------------------------------------------------

  hu_widget_t nughud_widgets[] = {
    {&w_title,   align_direct, align_direct},
    {&w_message, align_direct, align_direct},
    {&w_chat,    align_direct, align_direct},
    {&w_secret,  align_direct, align_direct},
    {&w_keys,    align_direct, align_direct},
    {&w_monsec,  align_direct, align_direct},
    {&w_sttime,  align_direct, align_direct},
    {&w_powers,  align_direct, align_direct}, // Powerup timers
    {&w_coord,   align_direct, align_direct},
    {&w_fps,     align_direct, align_direct},
    {&w_rate,    align_direct, align_direct},
  };

  for (i = 0;  i < (sizeof(nughud_widgets) / sizeof(*nughud_widgets));  i++)
  {
    widgets[NUGHUDSLOT][i] = nughud_widgets[i];
  }
}

static inline void HU_cond_build_widget (hu_multiline_t *const multiline, boolean cond)
{
  if (cond && multiline->built == false)
  {
    multiline->builder();
    multiline->built = true;
  }
}

static boolean hud_pending;

void HU_disable_all_widgets (void)
{
  hu_widget_t *w = widgets[hud_slot];

  while (w->multiline)
  {
    w->multiline->built = false;
    w++;
  }

  hud_pending = true;
}

//
// HU_Stop()
//
// Make the heads-up displays inactive
//
// Passed nothing, returns nothing
//
void HU_Stop(void)
{
  headsupactive = false;
}

//
// HU_Start(void)
//
// Create and initialize the heads-up widgets, software machines to
// maintain, update, and display information over the primary display
//
// This routine must be called after any change to the heads up configuration
// in order for the changes to take effect in the actual displays
//
// Passed nothing, returns nothing
//

static void HU_widget_build_ammo (void);
static void HU_widget_build_armor (void);
static void HU_widget_build_powers(void); // [Nugget] Powerup timers
static void HU_widget_build_coord (void);
static void HU_widget_build_fps (void);
static void HU_widget_build_rate (void);
static void HU_widget_build_health (void);
static void HU_widget_build_keys (void);
static void HU_widget_build_frag (void);
static void HU_widget_build_monsec(void);
static void HU_widget_build_sttime(void);
static void HU_widget_build_title (void);
static void HU_widget_build_weapon (void);

static hu_multiline_t *w_stats;

// [Nugget] /-----------------------------------------------------------------

#define NUMSQWIDGETS 10

typedef struct {
  nughud_textline_t *ntl;
  hu_widget_t *wid;
  int order;
} widgetpair_t;

typedef struct {
  int offset;
  widgetpair_t pairs[NUMSQWIDGETS];
} stackqueue_t;

static stackqueue_t nughud_stackqueues[NUMNUGHUDSTACKS];

static void NughudAlignWidget(nughud_textline_t *aligner, hu_widget_t *alignee)
{
  // Chat hack
  if (alignee->multiline == &w_chat) { return; }

  alignee->y = MAX(0, aligner->y);

  // Messages hack
  if (alignee->multiline == &w_message && nughud.message_defx) { return; }

  switch (aligner->align) {
    case -1:  alignee->h_align = align_left;    break;
    case  0:  alignee->h_align = align_center;  break;
    case  1:  alignee->h_align = align_right;   break;
    default:                                    break;
  }

  alignee->x = MAX(0, aligner->x) + NUGHUDWIDESHIFT(aligner->wide);
}

static boolean NughudAddToStack(nughud_textline_t *ntl, hu_widget_t *wid, int stack)
{
  if (ntl->x != -1 || ntl->y != -1) { return false; }

  for (int i = 0;  i < NUMSQWIDGETS;  i++)
  {
    widgetpair_t *pair = &nughud_stackqueues[stack - 1].pairs[i];

    if (!pair->ntl) {
      pair->ntl = ntl;
      pair->wid = wid;
      pair->order = (wid->multiline == &w_message) ? -2 :
                    (wid->multiline == &w_chat)    ? -1 : ntl->order;
      break;
    }
  }

  return true;
}

static int NughudSortWidgets(const void *_p1, const void *_p2)
{
  const widgetpair_t *p1 = (widgetpair_t *) _p1,
                     *p2 = (widgetpair_t *) _p2;

  if (!p1->ntl) { return 0; }

  int ret;

       if (p1->order < p2->order) { ret = -1; }
  else if (p1->order > p2->order) { ret =  1; }
  else                            { ret =  0; }

  if (nughud.stacks[p1->ntl->stack - 1].vlign == -1) { ret = -ret; }

  return ret;
}

#define MULTILINE(a) (                                 \
   (hud_widget_layout && (!st_crispyhud || (a) == -1)) \
    || (st_crispyhud && (a) == 1)                      \
)

boolean hud_automap;  // [Nugget] Condition used for level title

// [Nugget] -----------------------------------------------------------------/

void HU_Start(void)
{
  int i;

  if (headsupactive)                    // stop before starting
    HU_Stop();

  plr = &players[displayplayer];        // killough 3/7/98
  message_on = false;
  message_dontfuckwithme = false;
  message_nottobefuckedwith = false;
  chat_on = false;
  secret_on = false;

  // killough 11/98:
  message_counter = 0;
  message_count = (message_timer  * TICRATE) / 1000 + 1;
  chat_count    = (chat_msg_timer * TICRATE) / 1000 + 1;

  // create the message widget
  HUlib_init_multiline(&w_message, message_list ? hud_msg_lines : 1,
                       &doom_font, colrngs[hudcolor_mesg],
                       &message_on, NULL);

  // create the secret message widget
  HUlib_init_multiline(&w_secret, 1,
                       &doom_font, colrngs[CR_GOLD],
                       &secret_on, NULL);

  // create the chat widget
  HUlib_init_multiline(&w_chat, 1,
                       &doom_font, colrngs[hudcolor_chat],
                       &chat_on, NULL);
  // [FG] only the chat widget draws a cursor
  w_chat.drawcursor = true;

  // create the inputbuffer widgets, one per player
  for (i = 0; i < MAXPLAYERS; i++)
  {
    HUlib_clear_line(&w_inputbuffer[i]);
  }

  //jff 2/16/98 added some HUD widgets
  // create the map title widget
  HUlib_init_multiline(&w_title, 1,
                       &doom_font, colrngs[hudcolor_titl],
                       &hud_automap, HU_widget_build_title);
  // [FG] built only once right here
  w_title.builder();

  // create the hud health widget
  HUlib_init_multiline(&w_health, 1,
                       &boom_font, colrngs[CR_GREEN],
                       NULL, HU_widget_build_health);

  // create the hud armor widget
  HUlib_init_multiline(&w_armor, 1,
                       &boom_font, colrngs[CR_GREEN],
                       NULL, HU_widget_build_armor);

  // create the hud ammo widget
  HUlib_init_multiline(&w_ammo, 1,
                       &boom_font, colrngs[CR_GOLD],
                       NULL, HU_widget_build_ammo);

  // create the hud weapons widget
  HUlib_init_multiline(&w_weapon, 1,
                       &boom_font, colrngs[CR_GRAY],
                       NULL, HU_widget_build_weapon);

  // create the hud keys widget
  HUlib_init_multiline(&w_keys, 1,
                       &boom_font, colrngs[CR_GRAY],
                       NULL, deathmatch ? HU_widget_build_frag : HU_widget_build_keys);

  // create the hud monster/secret widget
  HUlib_init_multiline(&w_monsec,
                       MULTILINE(nughud.sts_ml) ? 3 : 1, // [Nugget] NUGHUD
                       &boom_font, colrngs[CR_GRAY],
                       NULL, HU_widget_build_monsec);
  // [FG] in deathmatch: w_keys.builder = HU_widget_build_frag()
  w_stats = deathmatch ? &w_keys : &w_monsec;

  HUlib_init_multiline(&w_sttime, 1,
                       &boom_font, colrngs[CR_GRAY],
                       NULL, HU_widget_build_sttime);

  // [Nugget] Powerup timers
  HUlib_init_multiline(&w_powers, 1, // TO-DO: Multi-lined?
                       &boom_font, colrngs[CR_GRAY],
                       NULL, HU_widget_build_powers);

  // create the automaps coordinate widget
  HUlib_init_multiline(&w_coord,
                       MULTILINE(nughud.coord_ml) ? 3 : 1, // [Nugget] NUGHUD
                       &boom_font, colrngs[hudcolor_xyco],
                       NULL, HU_widget_build_coord);

  HUlib_init_multiline(&w_fps, 1,
                       &boom_font, colrngs[hudcolor_xyco],
                       NULL, HU_widget_build_fps);

  HUlib_init_multiline(&w_rate, (voxels_rendering ? 2 : 1),
                       &boom_font, colrngs[hudcolor_xyco],
                       NULL, HU_widget_build_rate);
  // [FG] draw the IDRATE widget exclusively
  w_rate.exclusive = true;

  HU_set_centered_message();

  // [Nugget] NUGHUD
  if (st_crispyhud)
  {
    hu_widget_t    *w = widgets[NUGHUDSLOT];
    hu_multiline_t *m;
    nughud_textline_t *ntl;

    for (i = 0;  i < NUMNUGHUDSTACKS;  i++)
    {
      stackqueue_t *sq = &nughud_stackqueues[i];

      sq->offset = 0;

      for (int j = 0;  j < NUMSQWIDGETS;  j++)
      {
        sq->pairs[j].ntl = NULL;
        sq->pairs[j].wid = NULL;
        sq->pairs[j].order = 0;
      }
    }

    while ((m = w->multiline)) 
    {
      if      (m == &w_title)   { ntl = &nughud.title;   } 
      else if (m == &w_message) { ntl = &nughud.message; }
      else if (m == &w_chat)    { ntl = &nughud.message; }
      else if (m == &w_secret)  { ntl = &nughud.secret;  }
      else if (m ==  w_stats)   { ntl = &nughud.sts;     }
      else if (m == &w_sttime)  { ntl = &nughud.time;    }
      else if (m == &w_powers)  { ntl = &nughud.powers;  }
      else if (m == &w_coord)   { ntl = &nughud.coord;   }
      else if (m == &w_fps)     { ntl = &nughud.fps;     }
      else if (m == &w_rate)    { ntl = &nughud.rate;    }
      else                      { ntl = NULL;            }

      if (ntl)
      {
        // Messages hack
        if (m == &w_message && nughud.message_defx)
        {
          w->h_align = message_centered ? align_center : align_left;
          w->x = (w->h_align == align_center) ? SCREENWIDTH/2 : -video.deltaw * (hud_active == 2);
        }

        // Chat hack
        if (m == &w_chat)
        {
          w->x = nughud.message_defx ? -video.deltaw * (hud_active == 2)
                                     : -abs(NUGHUDWIDESHIFT(ntl->wide));
        }

        if (!NughudAddToStack(ntl, w, ntl->stack))
        {
          NughudAlignWidget(ntl, w);
        }
        else if ((m == &w_message || m == &w_chat)
                 && ntl->stack < NUMNUGHUDSTACKS)
        {
          NughudAddToStack(ntl, w, ntl->stack + 1);
        }
      }

      w++;
    }

    for (i = 0;  i < NUMNUGHUDSTACKS;  i++)
    {
      qsort(
        nughud_stackqueues[i].pairs,
        NUMSQWIDGETS,
        sizeof(widgetpair_t),
        NughudSortWidgets
      );
    }
  }

  HU_disable_all_widgets();
  HUlib_set_margins();

  // init crosshair
  if (hud_crosshair_on) // [Nugget] Use crosshair toggle
    HU_StartCrosshair();

  // now allow the heads-up display to run
  headsupactive = true;

  // [Nugget] Minimap: height of Messages widget may've changed,
  // so relocate the minimap if prudent
  if (automapactive == AM_MINI) { AM_Start(); }
}

static void HU_widget_build_title (void)
{
  char hud_titlestr[HU_MAXLINELENGTH] = "";
  char *s, *n;

  if (gamemapinfo && gamemapinfo->levelname)
  {
    if (gamemapinfo->label)
      s = gamemapinfo->label;
    else
      s = gamemapinfo->mapname;

    if (s == gamemapinfo->mapname || U_CheckField(s))
    {
      M_snprintf(hud_titlestr, sizeof(hud_titlestr), "%s: ", s);
    }
    s = gamemapinfo->levelname;
  }
  else if (gamestate == GS_LEVEL)
  {
    if (VANILLAMAP(gameepisode, gamemap))
    {
      s = (gamemode != commercial) ? HU_TITLE :
          (gamemission == pack_tnt) ? HU_TITLET :
          (gamemission == pack_plut) ? HU_TITLEP :
          HU_TITLE2;
    }
    // WADs like pl2.wad have a MAP33, and rely on the layout in the
    // Vanilla executable, where it is possible to overflow the end of one
    // array into the next.
    else if (gamemode == commercial && gamemap >= 33 && gamemap <= 35)
    {
      s = (gamemission == doom2) ? (*mapnamesp[gamemap-33]) :
          (gamemission == pack_plut) ? (*mapnamest[gamemap-33]) : "";
    }
    else
    {
      // initialize the map title widget with the generic map lump name
      s = MAPNAME(gameepisode, gamemap);
    }
  }
  else
  {
    s = "";
  }

  // [FG] cap at line break
  if ((n = strchr(s, '\n')))
  {
    *n = '\0';
  }

  M_StringConcat(hud_titlestr, s, sizeof(hud_titlestr));

  HUlib_add_string_to_cur_line(&w_title, hud_titlestr);
}

// do the hud ammo display
static void HU_widget_build_ammo (void)
{
  char hud_ammostr[HU_MAXLINELENGTH] = "AMM ";
  int fullammo = plr->maxammo[weaponinfo[plr->readyweapon].ammo];
  int i = 4;

  // special case for weapon with no ammo selected - blank bargraph + N/A
  if (weaponinfo[plr->readyweapon].ammo == am_noammo || fullammo == 0)
  {
    if (hud_type == HUD_TYPE_BOOM)
    {
      strcat(hud_ammostr, "\x7f\x7f\x7f\x7f\x7f\x7f\x7f");
    }
    strcat(hud_ammostr, "N/A");
    w_ammo.cr = colrngs[CR_GRAY];
  }
  else
  {
    int ammo = plr->ammo[weaponinfo[plr->readyweapon].ammo];
    int ammopct = (100 * ammo) / fullammo;
    int ammobars = ammopct / 4;

    // build the bargraph string
    if (hud_type == HUD_TYPE_BOOM)
    {
      // full bargraph chars
      for (i = 4; i < 4 + ammobars / 4;)
        hud_ammostr[i++] = 123;

      // plus one last character with 0, 1, 2, 3 bars
      switch (ammobars % 4)
      {
        case 0:
          break;
        case 1:
          hud_ammostr[i++] = 126;
          break;
        case 2:
          hud_ammostr[i++] = 125;
          break;
        case 3:
          hud_ammostr[i++] = 124;
          break;
      }

      // pad string with blank bar characters
      while (i < 4 + 7)
        hud_ammostr[i++] = 127;
      hud_ammostr[i] = '\0';
    }

    // build the numeric amount init string
    M_snprintf(hud_ammostr + i, sizeof(hud_ammostr), "%3d/%3d", ammo, fullammo);

    // backpack changes thresholds (ammo widget)
    if (plr->backpack && !hud_backpack_thresholds && fullammo)
      ammopct = (100 * ammo) / (fullammo / 2);

    // set the display color from the percentage of total ammo held

    // [Nugget] Make it gray if the player has infinite ammo /----------------
    if (plr->cheats & CF_INFAMMO)
      w_ammo.cr = colrngs[CR_GRAY];
    else
    // [Nugget] -------------------------------------------------------------/

    if (ammopct < ammo_red)
      w_ammo.cr = colrngs[CR_RED];
    else if (ammopct < ammo_yellow)
      w_ammo.cr = colrngs[CR_GOLD];
    else if (ammopct > 100) // more than max threshold w/o backpack
      w_ammo.cr = colrngs[CR_BLUE];
    else
      w_ammo.cr = colrngs[CR_GREEN];
  }

  // transfer the init string to the widget
  HUlib_add_string_to_cur_line(&w_ammo, hud_ammostr);
}

// do the hud health display
static void HU_widget_build_health (void)
{
  char hud_healthstr[HU_MAXLINELENGTH] = "HEL ";
  int i = 4;
  int healthbars = (st_health > 100) ? 25 : (st_health / 4);

  // build the bargraph string
  if (hud_type == HUD_TYPE_BOOM)
  {
    // full bargraph chars
    for (i = 4; i < 4 + healthbars / 4;)
      hud_healthstr[i++] = 123;

    // plus one last character with 0, 1, 2, 3 bars
    switch (healthbars % 4)
    {
      case 0:
        break;
      case 1:
        hud_healthstr[i++] = 126;
        break;
      case 2:
        hud_healthstr[i++] = 125;
        break;
      case 3:
        hud_healthstr[i++] = 124;
        break;
    }

    // pad string with blank bar characters
    while (i < 4 + 7)
      hud_healthstr[i++] = 127;
    hud_healthstr[i] = '\0';
  }

  // build the numeric amount init string
  M_snprintf(hud_healthstr + i, sizeof(hud_healthstr), "%3d", st_health);

  // set the display color from the amount of health posessed
  w_health.cr = ColorByHealth(plr->health, 100, st_invul);

  // transfer the init string to the widget
  HUlib_add_string_to_cur_line(&w_health, hud_healthstr);
}

// do the hud armor display
static void HU_widget_build_armor (void)
{
  char hud_armorstr[HU_MAXLINELENGTH] = "ARM ";
  int i = 4;
  int armorbars = (st_armor > 100) ? 25 : (st_armor / 4);

  // build the bargraph string
  if (hud_type == HUD_TYPE_BOOM)
  {
    // full bargraph chars
    for (i = 4; i < 4 + armorbars / 4;)
      hud_armorstr[i++] = 123;

    // plus one last character with 0, 1, 2, 3 bars
    switch (armorbars % 4)
    {
      case 0:
        break;
      case 1:
        hud_armorstr[i++] = 126;
        break;
      case 2:
        hud_armorstr[i++] = 125;
        break;
      case 3:
        hud_armorstr[i++] = 124;
        break;
    }

    // pad string with blank bar characters
    while (i < 4 + 7)
      hud_armorstr[i++] = 127;
    hud_armorstr[i] = '\0';
  }

  // build the numeric amount init string
  M_snprintf(hud_armorstr + i, sizeof(hud_armorstr), "%3d", st_armor);

  // color of armor depends on type
  if (hud_armor_type)
  {
    w_armor.cr =
      // [Nugget] Make it gray ONLY if the player is in God Mode
      (plr->cheats & CF_GODMODE) ? colrngs[CR_GRAY] :
      (plr->armortype == 0) ? colrngs[CR_RED] :
      (plr->armortype == 1) ? colrngs[CR_GREEN] :
      colrngs[CR_BLUE];
  }
  else
  {
    int armor = plr->armorpoints;
    
    // set the display color from the amount of armor posessed
    w_armor.cr =
      st_invul ? colrngs[CR_GRAY] :
      (armor < armor_red) ? colrngs[CR_RED] :
      (armor < armor_yellow) ? colrngs[CR_GOLD] :
      (armor <= armor_green) ? colrngs[CR_GREEN] :
      colrngs[CR_BLUE];
  }

  // transfer the init string to the widget
  HUlib_add_string_to_cur_line(&w_armor, hud_armorstr);
}

// do the hud weapon display
static void HU_widget_build_weapon (void)
{
  char hud_weapstr[HU_MAXLINELENGTH] = "WEA ";
  int i = 4, w, ammo, fullammo, ammopct;

  // do each weapon that exists in current gamemode
  for (w = 0; w <= wp_supershotgun; w++) //jff 3/4/98 show fists too, why not?
  {
    int ok = 1;

    //jff avoid executing for weapons that do not exist
    switch (gamemode)
    {
      case shareware:
        if (w >= wp_plasma && w != wp_chainsaw)
          ok = 0;
        break;
      case retail:
      case registered:
        if (w >= wp_supershotgun && !have_ssg)
          ok = 0;
        break;
      default:
      case commercial:
        break;
    }
    if (!ok)
      continue;

    ammo = plr->ammo[weaponinfo[w].ammo];
    fullammo = plr->maxammo[weaponinfo[w].ammo];
    ammopct = 0;

    // skip weapons not currently posessed
    if (!plr->weaponowned[w])
      continue;

    // backpack changes thresholds (weapon widget)
    if (plr->backpack && !hud_backpack_thresholds)
      fullammo /= 2;

    ammopct = fullammo ? (100 * ammo) / fullammo : 100;

    // display each weapon number in a color related to the ammo for it
    hud_weapstr[i++] = '\x1b'; //jff 3/26/98 use ESC not '\' for paths
    if (weaponinfo[w].ammo == am_noammo) //jff 3/14/98 show berserk on HUD
      hud_weapstr[i++] = (w == wp_fist && !plr->powers[pw_strength]) ? '0'+CR_GRAY : '0'+CR_GREEN;
    else if (ammopct < ammo_red)
      hud_weapstr[i++] = '0'+CR_RED;
    else if (ammopct < ammo_yellow)
      hud_weapstr[i++] = '0'+CR_GOLD;
    else if (ammopct > 100) // more than max threshold w/o backpack
      hud_weapstr[i++] = '0'+CR_BLUE;
    else
      hud_weapstr[i++] = '0'+CR_GREEN;

    hud_weapstr[i++] = '0'+w+1;
    hud_weapstr[i++] = ' ';
    hud_weapstr[i] = '\0';
  }

  // transfer the init string to the widget
  HUlib_add_string_to_cur_line(&w_weapon, hud_weapstr);
}

static void HU_widget_build_keys (void)
{
  char hud_keysstr[HU_MAXLINELENGTH] = { 'K', 'E', 'Y', '\x1b', '0'+CR_NONE, ' ', '\0' };
  int i = 6, k;

  // build text string whose characters call out graphic keys
  for (k = 0; k < 6; k++)
  {
    // skip keys not possessed
    if (!plr->cards[k])
      continue;

    hud_keysstr[i++] = HU_FONTEND + k + 1; // key number plus HU_FONTEND is char for key
    hud_keysstr[i++] = ' ';   // spacing
    hud_keysstr[i++] = ' ';
  }

  // [Alaux] Blink missing keys *after* possessed keys
  for (k = 0; k < 6; k++)
  {
    if (plr->cards[k])
      continue;

    switch (ST_BlinkKey(plr, k % 3))
    {
      case KEYBLINK_CARD:
        if (k >= 3)
          continue;
        break;

      case KEYBLINK_SKULL:
        if (k < 3)
          continue;
        break;

      case KEYBLINK_BOTH:
        break;

      default:
        continue;
    }

    hud_keysstr[i++] = HU_FONTEND + k + 1;
    hud_keysstr[i++] = ' ';
    hud_keysstr[i++] = ' ';
  }

  hud_keysstr[i] = '\0';

  // transfer the built string (frags or key title) to the widget
  HUlib_add_string_to_cur_line(&w_keys, hud_keysstr);
}

static inline int HU_top (char *const fragstr, int i, const int idx1, const int top1)
{
  if (idx1 > -1)
  {
    char numbuf[32], *s;

    M_snprintf(numbuf, sizeof(numbuf), "%5d", top1);
    // make frag count in player's color via escape code

    fragstr[i++] = '\x1b'; //jff 3/26/98 use ESC not '\' for paths
    fragstr[i++] = '0' + plyrcoltran[idx1 & 3];
    s = numbuf;
    while (*s)
      fragstr[i++] = *s++;
  }
  return i;
}

static void HU_widget_build_frag (void)
{
  char hud_fragstr[HU_MAXLINELENGTH] = { 'F', 'R', 'G', '\x1b', '0'+CR_ORIG, ' ', '\0' };
  int i = 6, k;

  int top1 = -999, top2 = -999, top3 = -999, top4 = -999;
  int idx1 = -1, idx2 = -1, idx3 = -1, idx4 = -1;
  int fragcount, m;

  // scan thru players
  for (k = 0; k < MAXPLAYERS; k++)
  {
    // skip players not in game
    if (!playeringame[k])
      continue;

    fragcount = 0;

    // compute number of times they've fragged each player
    // minus number of times they've been fragged by them
    for (m = 0; m < MAXPLAYERS; m++)
    {
      if (!playeringame[m])
        continue;
      fragcount += (m != k) ? players[k].frags[m] : -players[k].frags[m];
    }

    // very primitive sort of frags to find top four
    if (fragcount > top1)
    {
      top4 = top3; top3 = top2; top2 = top1; top1 = fragcount;
      idx4 = idx3; idx3 = idx2; idx2 = idx1; idx1 = k;
    }
    else if (fragcount > top2)
    {
      top4 = top3; top3 = top2; top2 = fragcount;
      idx4 = idx3; idx3 = idx2; idx2 = k;
    }
    else if (fragcount > top3)
    {
      top4 = top3; top3 = fragcount;
      idx4 = idx3; idx3 = k;
    }
    else if (fragcount > top4)
    {
      top4 = fragcount;
      idx4 = k;
    }
  }

  // killough 11/98: replaced cut-and-pasted code with function

  // if the biggest number exists,
  // put it in the init string
  i = HU_top(hud_fragstr, i, idx1, top1);

  // if the second biggest number exists,
  // put it in the init string
  i = HU_top(hud_fragstr, i, idx2, top2);

  // if the third biggest number exists,
  // put it in the init string
  i = HU_top(hud_fragstr, i, idx3, top3);

  // if the fourth biggest number exists,
  // put it in the init string
  i = HU_top(hud_fragstr, i, idx4, top4);

  hud_fragstr[i] = '\0';

  // transfer the built string (frags or key title) to the widget
  HUlib_add_string_to_cur_line(&w_keys, hud_fragstr);
}

static void HU_widget_build_monsec(void)
{
  char hud_monsecstr[HU_MAXLINELENGTH];
  int i;
  int fullkillcount, fullitemcount, fullsecretcount;
  int killcolor, itemcolor, secretcolor;
  int kill_percent_count;

  fullkillcount = 0;
  fullitemcount = 0;
  fullsecretcount = 0;
  kill_percent_count = 0;

  for (i = 0; i < MAXPLAYERS; ++i)
  {
    if (playeringame[i])
    {
      fullkillcount += players[i].killcount - players[i].maxkilldiscount;
      fullitemcount += players[i].itemcount;
      fullsecretcount += players[i].secretcount;
      kill_percent_count += players[i].killcount;
    }
  }

  if (respawnmonsters)
  {
    fullkillcount = kill_percent_count;
    max_kill_requirement = totalkills;
  }

  // [Nugget] Customizable Stats colors

  char killlabelcolor, itemlabelcolor, secretlabelcolor;

  killcolor   = (fullkillcount >= max_kill_requirement) ? '0'+hudcolor_ms_comp : '0'+hudcolor_ms_incomp;
  itemcolor   = (fullitemcount >= totalitems)           ? '0'+hudcolor_ms_comp : '0'+hudcolor_ms_incomp;
  secretcolor = (fullsecretcount >= totalsecret)        ? '0'+hudcolor_ms_comp : '0'+hudcolor_ms_incomp;

  // [Nugget] Stats formats from Crispy /-------------------------------------

  char kill_str[32], item_str[32], secret_str[32];

  switch ((automapactive == AM_FULL && hud_stats_format_map) ? hud_stats_format_map : hud_stats_format)
  {
    case STATSFORMAT_RATIO:
      M_snprintf(kill_str,   sizeof(kill_str),   "%d/%d", fullkillcount,   max_kill_requirement);
      M_snprintf(item_str,   sizeof(item_str),   "%d/%d", fullitemcount,   totalitems);
      M_snprintf(secret_str, sizeof(secret_str), "%d/%d", fullsecretcount, totalsecret);
      break;

    case STATSFORMAT_BOOLEAN:
      M_snprintf(kill_str,   sizeof(kill_str),   "%s", (fullkillcount   >= max_kill_requirement) ? "YES" : "NO");
      M_snprintf(item_str,   sizeof(item_str),   "%s", (fullitemcount   >= totalitems)           ? "YES" : "NO");
      M_snprintf(secret_str, sizeof(secret_str), "%s", (fullsecretcount >= totalsecret)          ? "YES" : "NO");
      break;

    case STATSFORMAT_PERCENTAGE:
      M_snprintf(kill_str,   sizeof(kill_str),   "%d%%", (!max_kill_requirement) ? 100 : fullkillcount   * 100 / max_kill_requirement);
      M_snprintf(item_str,   sizeof(item_str),   "%d%%", (!totalitems)           ? 100 : fullitemcount   * 100 / totalitems);
      M_snprintf(secret_str, sizeof(secret_str), "%d%%", (!totalsecret)          ? 100 : fullsecretcount * 100 / totalsecret);
      break;

    case STATSFORMAT_REMAINING:
      M_snprintf(kill_str,   sizeof(kill_str),   "%d", max_kill_requirement - fullkillcount);
      M_snprintf(item_str,   sizeof(item_str),   "%d", totalitems           - fullitemcount);
      M_snprintf(secret_str, sizeof(secret_str), "%d", totalsecret          - fullsecretcount);
      break;
  }

  // [Nugget] ---------------------------------------------------------------/

  // [Nugget] Stats icons /---------------------------------------------------

  char killlabel, itemlabel, secretlabel;

  if (sml_font.patches[HU_FONTSIZE + 6 + 0])
  {
    killlabel = (char) (HU_FONTEND + 7 + 0);
    killlabelcolor = '0'+CR_NONE;
  }
  else {
    killlabel = 'K';
    killlabelcolor = '0'+hudcolor_kills;
  }

  if (sml_font.patches[HU_FONTSIZE + 6 + 1])
  {
    itemlabel = (char) (HU_FONTEND + 7 + 1);
    itemlabelcolor = '0'+CR_NONE;
  }
  else {
    itemlabel = 'I';
    itemlabelcolor = '0'+hudcolor_items;
  }

  if (sml_font.patches[HU_FONTSIZE + 6 + 2])
  {
    secretlabel = (char) (HU_FONTEND + 7 + 2);
    secretlabelcolor = '0'+CR_NONE;
  }
  else {
    secretlabel = 'S';
    secretlabelcolor = '0'+hudcolor_secrets;
  }

  // [Nugget] ---------------------------------------------------------------/

  if (MULTILINE(nughud.sts_ml)) // [Nugget] NUGHUD
  {
    // [Nugget] Stats icons
    // [Nugget] Stats formats from Crispy

    M_snprintf(hud_monsecstr, sizeof(hud_monsecstr),
      "\x1b%c%c\t\x1b%c%s", killlabelcolor, killlabel, killcolor, kill_str);
    HUlib_add_string_to_cur_line(&w_monsec, hud_monsecstr);

    M_snprintf(hud_monsecstr, sizeof(hud_monsecstr),
      "\x1b%c%c\t\x1b%c%s", itemlabelcolor, itemlabel, itemcolor, item_str);
    HUlib_add_string_to_cur_line(&w_monsec, hud_monsecstr);

    M_snprintf(hud_monsecstr, sizeof(hud_monsecstr),
      "\x1b%c%c\t\x1b%c%s", secretlabelcolor, secretlabel, secretcolor, secret_str);
    HUlib_add_string_to_cur_line(&w_monsec, hud_monsecstr);
  }
  else
  {
    // [Nugget] Stats icons
    // [Nugget] Stats formats from Crispy
    M_snprintf(hud_monsecstr, sizeof(hud_monsecstr),
      "\x1b%c%c \x1b%c%s \x1b%c%c \x1b%c%s \x1b%c%c \x1b%c%s",
      killlabelcolor, killlabel, killcolor, kill_str,
      itemlabelcolor, itemlabel, itemcolor, item_str,
      secretlabelcolor, secretlabel, secretcolor, secret_str);

    HUlib_add_string_to_cur_line(&w_monsec, hud_monsecstr);
  }
}

static void HU_widget_build_sttime(void)
{
  char hud_timestr[HU_MAXLINELENGTH/2];
  int offset = 0;
  extern int time_scale;

  if ((hud_level_time & HUD_WIDGET_HUD     && !automapactive) ||
      (hud_level_time & HUD_WIDGET_AUTOMAP &&  automapactive))
  {
    if (time_scale != 100)
    {
      offset += M_snprintf(hud_timestr, sizeof(hud_timestr), "\x1b%c%d%% ",
                           '0'+hudcolor_time_scale, time_scale);
    }

    if (totalleveltimes)
    {
      const int time = (totalleveltimes + leveltime) / TICRATE;

      offset += M_snprintf(hud_timestr + offset, sizeof(hud_timestr),
                           "\x1b%c%d:%02d ",
                           '0'+hudcolor_total_time, time/60, time%60);
    }

    if (!plr->btuse_tics)
    {
      M_snprintf(hud_timestr + offset, sizeof(hud_timestr), "\x1b%c%d:%05.2f\t",
                 '0'+hudcolor_time, leveltime / TICRATE / 60,
                 (float)(leveltime % (60 * TICRATE)) / TICRATE);
    }
  }

  if (plr->btuse_tics)
  {
    const int type = plr->eventtype;

    // [Nugget] Support other events
    M_snprintf(hud_timestr + offset, sizeof(hud_timestr), "\x1b%c%c %d:%05.2f\t",
               '0'+hudcolor_event_timer,
               type == TIMER_KEYPICKUP ? 'K' : type == TIMER_TELEPORT ? 'T' : 'U',
               plr->btuse / TICRATE / 60, 
               (float)(plr->btuse % (60 * TICRATE)) / TICRATE);
  }

  HUlib_add_string_to_cur_line(&w_sttime, hud_timestr);
}

void HU_widget_rebuild_sttime(void)
{
  HU_widget_build_sttime();
}

// [Nugget] Powerup timers
static void HU_widget_build_powers(void)
{
  char hud_powerstr[48];
  int offset = 0;

  // TO-DO: Multi-lined?

  if (plr->powers[pw_invisibility] > 0) {
    offset += M_snprintf(hud_powerstr, sizeof(hud_powerstr), "\x1b%cINVIS %i\" ",
                         '0' + ((plr->powers[pw_invisibility] > 4*32 || plr->powers[pw_invisibility] & 8) ? CR_RED : CR_BLACK),
                         MIN(INVISTICS/TICRATE, 1 + (plr->powers[pw_invisibility] / TICRATE)));
  }

  if (plr->powers[pw_invulnerability] > 0) {
    offset += M_snprintf(hud_powerstr + offset, sizeof(hud_powerstr), "\x1b%cINVUL %i\" ",
                         '0' + ((plr->powers[pw_invulnerability] > 4*32 || plr->powers[pw_invulnerability] & 8) ? CR_GREEN : CR_BLACK),
                         MIN(INVULNTICS/TICRATE, 1 + (plr->powers[pw_invulnerability] / TICRATE)));
  }

  if (plr->powers[pw_infrared] > 0) {
    offset += M_snprintf(hud_powerstr + offset, sizeof(hud_powerstr), "\x1b%cLIGHT %i\" ",
                         '0' + ((plr->powers[pw_infrared] > 4*32 || plr->powers[pw_infrared] & 8) ? CR_BRICK : CR_BLACK),
                         MIN(INFRATICS/TICRATE, 1 + (plr->powers[pw_infrared] / TICRATE)));
  }

  if (plr->powers[pw_ironfeet] > 0) {
    offset += M_snprintf(hud_powerstr + offset, sizeof(hud_powerstr), "\x1b%cSUIT %i\"",
                         '0' + ((plr->powers[pw_ironfeet] > 4*32 || plr->powers[pw_ironfeet] & 8) ? CR_GRAY : CR_BLACK),
                         MIN(IRONTICS/TICRATE, 1 + (plr->powers[pw_ironfeet] / TICRATE)));
  }

  
  if (offset) { HUlib_add_string_to_cur_line(&w_powers, hud_powerstr); }
  else        { HUlib_clear_cur_line(&w_powers); }
}

static void HU_widget_build_coord (void)
{
  char hud_coordstr[HU_MAXLINELENGTH];
  fixed_t x,y,z; // killough 10/98:
  void AM_Coordinates(const mobj_t *, fixed_t *, fixed_t *, fixed_t *);

  // killough 10/98: allow coordinates to display non-following pointer
  AM_Coordinates(plr->mo, &x, &y, &z);

  //jff 2/16/98 output new coord display
  if (MULTILINE(nughud.coord_ml)) // [Nugget] NUGHUD
  {
    M_snprintf(hud_coordstr, sizeof(hud_coordstr), "X\t\x1b%c%d", '0'+CR_GRAY, x >> FRACBITS);
    HUlib_add_string_to_cur_line(&w_coord, hud_coordstr);

    M_snprintf(hud_coordstr, sizeof(hud_coordstr), "Y\t\x1b%c%d", '0'+CR_GRAY, y >> FRACBITS);
    HUlib_add_string_to_cur_line(&w_coord, hud_coordstr);

    M_snprintf(hud_coordstr, sizeof(hud_coordstr), "Z\t\x1b%c%d", '0'+CR_GRAY, z >> FRACBITS);
    HUlib_add_string_to_cur_line(&w_coord, hud_coordstr);
  }
  else
  {
    M_snprintf(hud_coordstr, sizeof(hud_coordstr), "X \x1b%c%d \x1b%cY \x1b%c%d \x1b%cZ \x1b%c%d",
            '0'+CR_GRAY, x >> FRACBITS, '0'+hudcolor_xyco,
            '0'+CR_GRAY, y >> FRACBITS, '0'+hudcolor_xyco,
            '0'+CR_GRAY, z >> FRACBITS);

    HUlib_add_string_to_cur_line(&w_coord, hud_coordstr);
  }
}

static void HU_widget_build_fps (void)
{
  char hud_fpsstr[HU_MAXLINELENGTH/4];

  M_snprintf(hud_fpsstr, sizeof(hud_fpsstr), "\x1b%c%d \x1b%cFPS",
             '0'+CR_GRAY, fps, '0'+CR_ORIG);
  HUlib_add_string_to_cur_line(&w_fps, hud_fpsstr);
}

static void HU_widget_build_rate (void)
{
  char hud_ratestr[HU_MAXLINELENGTH];

  M_snprintf(hud_ratestr, sizeof(hud_ratestr),
             "Sprites %4d Segs %4d Visplanes %4d   \x1b%cFPS %3d %dx%d\x1b%c",
             rendered_vissprites, rendered_segs, rendered_visplanes,
             '0'+CR_GRAY, fps, video.width, video.height, '0'+CR_ORIG);
  HUlib_add_string_to_cur_line(&w_rate, hud_ratestr);

  if (voxels_rendering)
  {
    M_snprintf(hud_ratestr, sizeof(hud_ratestr), " Voxels %4d", rendered_voxels);
    HUlib_add_string_to_cur_line(&w_rate, hud_ratestr);
  }
}

// Crosshair

int hud_crosshair; // [Nugget] Crosshair type to be used
boolean hud_crosshair_health;
crosstarget_t hud_crosshair_target;
crosslockon_t hud_crosshair_lockon; // [Alaux] Crosshair locks on target
boolean hud_crosshair_indicators; // [Nugget] Horizontal-autoaim indicators
boolean hud_crosshair_fuzzy; // [Nugget] Account for fuzzy targets
int hud_crosshair_color;
int hud_crosshair_target_color;

typedef struct
{
  patch_t *patch;
  int w, h, x, y;
  byte *cr;

  // [Nugget] Horizontal-autoaim indicators
  patch_t *patchl, *patchr;
  int lw, lh, rw, rh;
  int side;
} crosshair_t;

static crosshair_t crosshair;

const char *crosshair_lumps[HU_CROSSHAIRS] =
{
  NULL,
  "CROSS00", "CROSS01", "CROSS02", "CROSS03",
  "CROSS04", "CROSS05", "CROSS06", "CROSS07",
  "CROSS08"
};

const char *crosshair_strings[HU_CROSSHAIRS] =
{
  "Off",
  "Cross", "Angle", "Dot", "Big Cross",
  "Circle", "Big Circle", "Chevron", "Chevrons",
  "Arcs"
};

static void HU_InitCrosshair(void)
{
  int i, j;

  for (i = 1; i < HU_CROSSHAIRS; i++)
  {
    j = W_CheckNumForName(crosshair_lumps[i]);
    if (j >= num_predefined_lumps)
    {
      if (R_IsPatchLump(j))
        crosshair_strings[i] = crosshair_lumps[i];
      else
        crosshair_lumps[i] = NULL;
    }
  }
}

void HU_StartCrosshair(void) // [Nugget] Not static anymore
{
  if (crosshair.patch)
    Z_ChangeTag(crosshair.patch, PU_CACHE);

  // [Nugget] Horizontal-autoaim indicators
  if (crosshair.patchl) { Z_ChangeTag(crosshair.patchl, PU_CACHE); }
  if (crosshair.patchr) { Z_ChangeTag(crosshair.patchr, PU_CACHE); }

  if (crosshair_lumps[hud_crosshair])
  {
    crosshair.patch = W_CacheLumpName(crosshair_lumps[hud_crosshair], PU_STATIC);

    crosshair.w = SHORT(crosshair.patch->width)/2;
    crosshair.h = SHORT(crosshair.patch->height)/2;
  }
  else
    crosshair.patch = NULL;

  // [Nugget] Horizontal-autoaim indicators ----------------------------------

  crosshair.patchl = W_CacheLumpName("CROSSIL", PU_STATIC);
  crosshair.lw = SHORT(crosshair.patchl->width);
  crosshair.lh = SHORT(crosshair.patchl->height)/2;

  crosshair.patchr = W_CacheLumpName("CROSSIR", PU_STATIC);
  crosshair.rw = SHORT(crosshair.patchr->width);
  crosshair.rh = SHORT(crosshair.patchr->height)/2;
}

mobj_t *crosshair_target; // [Alaux] Lock crosshair on target

static void HU_UpdateCrosshair(void)
{
  crosshair.x = SCREENWIDTH/2;
  crosshair.y = (screenblocks <= 10) ? (SCREENHEIGHT-ST_HEIGHT)/2 : SCREENHEIGHT/2;

  crosshair.side = 0; // [Nugget] Horizontal-autoaim indicators

  if (hud_crosshair_health)
    crosshair.cr = ColorByHealth(plr->health, 100, st_invul);
  else
    crosshair.cr = colrngs[hud_crosshair_color];

  if (STRICTMODE(hud_crosshair_target || hud_crosshair_lockon))
  {
    angle_t an = plr->mo->angle;
    ammotype_t ammo = weaponinfo[plr->readyweapon].ammo;
    fixed_t range = (ammo == am_noammo
                     && !(plr->readyweapon == wp_fist && plr->cheats & CF_SAITAMA)) // [Nugget]
                    ? MELEERANGE : 16*64*FRACUNIT * NOTCASUALPLAY(comp_longautoaim+1); // [Nugget]
    boolean intercepts_overflow_enabled = overflow[emu_intercepts].enabled;

    crosshair_target = linetarget = NULL;

    overflow[emu_intercepts].enabled = false;
    P_AimLineAttack(plr->mo, an, range, CROSSHAIR_AIM);
    if (!vertical_aiming && (ammo == am_misl || ammo == am_cell) // [Nugget] Vertical aiming
        && (!no_hor_autoaim || !casual_play)) // [Nugget]
    {
      if (!linetarget) {
        P_AimLineAttack(plr->mo, an += 1<<26, range, CROSSHAIR_AIM);
        if (linetarget && hud_crosshair_indicators) { crosshair.side = -1; } // [Nugget]
      }
      if (!linetarget) {
        P_AimLineAttack(plr->mo, an -= 2<<26, range, CROSSHAIR_AIM);
        if (linetarget && hud_crosshair_indicators) { crosshair.side = 1; } // [Nugget]
      }
    }
    overflow[emu_intercepts].enabled = intercepts_overflow_enabled;

    if (linetarget
        && (!(linetarget->flags & MF_SHADOW) || hud_crosshair_fuzzy)) // [Nugget]
      crosshair_target = linetarget;

    if (hud_crosshair_target && crosshair_target)
    {
      // [Alaux] Color crosshair by target health
      if (hud_crosshair_target == crosstarget_health)
      {
        crosshair.cr = ColorByHealth(crosshair_target->health, crosshair_target->info->spawnhealth, false);
      }
      else
      {
        crosshair.cr = colrngs[hud_crosshair_target_color];
      }
    }
  }
}

void HU_UpdateCrosshairLock(int x, int y)
{
  int w = (crosshair.w * video.xscale) >> FRACBITS;
  int h = (crosshair.h * video.yscale) >> FRACBITS;

  x = viewwindowx + BETWEEN(w, viewwidth  - w - 1, x);
  y = viewwindowy + BETWEEN(h, viewheight - h - 1, y);

  if (hud_crosshair_lockon == crosslockon_full) // [Nugget]
    crosshair.x = (x << FRACBITS) / video.xscale - video.deltaw;

  crosshair.y = (y << FRACBITS) / video.yscale;
}

void HU_DrawCrosshair(void)
{
  if (plr->playerstate != PST_LIVE ||
      automapactive == AM_FULL || // [Nugget] Changed condition
      menuactive ||
      paused ||
      // [Nugget] New conditions
      !crosshair.cr || // Crash fix
      (chasecam_mode && !chasecam_crosshair) || // Chasecam
      (gamestate == GS_INTERMISSION)) // Alt. intermission background
  {
    return;
  }

  // [Nugget] NUGHUD
  const int y = crosshair.y + (nughud.viewoffset * STRICTMODE(st_crispyhud));

  if (crosshair.patch)
    V_DrawPatchTranslated(crosshair.x - crosshair.w,
                                    y - crosshair.h,
                          crosshair.patch, crosshair.cr);

  // [Nugget] Horizontal-autoaim indicators ----------------------------------

  if (crosshair.side == -1)
  {
    V_DrawPatchTranslated(crosshair.x - crosshair.w - crosshair.lw,
                                    y - crosshair.lh,
                          crosshair.patchl, crosshair.cr);
  }
  else if (crosshair.side == 1)
  {
    V_DrawPatchTranslated(crosshair.x + crosshair.w,
                                    y - crosshair.rh,
                          crosshair.patchr, crosshair.cr);
  }
}

// [crispy] print a bar indicating demo progress at the bottom of the screen
boolean HU_DemoProgressBar(boolean force)
{
  const int progress = video.unscaledw * playback_tic / playback_totaltics;
  static int old_progress = 0;

  if (old_progress < progress)
  {
    old_progress = progress;
  }
  else if (!force)
  {
    return false;
  }

  V_FillRect(0, SCREENHEIGHT - 2, progress, 1, v_darkest_color);
  V_FillRect(0, SCREENHEIGHT - 1, progress, 1, v_lightest_color);

  return true;
}

// [FG] level stats and level time widgets
int hud_player_coords, hud_level_stats, hud_level_time;

// [Nugget]
int hud_power_timers; // Powerup timers

int hud_time[NUMTIMERS]; // [Nugget] Support more event timers

//
// HU_Drawer()
//
// Draw all the pieces of the heads-up display
//
// Passed nothing, returns nothing
//
void HU_Drawer(void)
{
  hu_widget_t *w = widgets[hud_slot];

  if (hud_pending)
    return;

  // [Nugget] Draw Status Bar *before* HUD
  if (draw_crispy_hud)
  {
    ST_Drawer (false, true);
  }

  HUlib_reset_align_offsets();

  while (w->multiline)
  {
    if ((w->multiline->on && *w->multiline->on) || w->multiline->built)
    {
      HUlib_draw_widget(w);
    }
    w++;
  }
}

// [FG] draw Time widget on intermission screen
void WI_DrawTimeWidget(void)
{
  const hu_widget_t w = {&w_sttime, align_left, align_top};

  if (hud_level_time & HUD_WIDGET_HUD)
  {
    HUlib_reset_align_offsets();
    // leveltime is already added to totalleveltimes before WI_Start()
    //HU_widget_build_sttime();
    HUlib_draw_widget(&w);
  }
}

//
// HU_Erase()
//
// Erase hud display lines that can be trashed by small screen display
//
// Passed nothing, returns nothing
//

void HU_Erase(void)
{
  hu_widget_t *w = widgets[hud_slot];

  if (automapactive || !scaledviewx)
    return;

  HUlib_reset_align_offsets();

  while (w->multiline)
  {
    if (w->multiline->on || w->multiline->built)
    {
      HUlib_erase_widget(w);
    }
    w++;
  }
}

//
// HU_Ticker()
//
// Update the hud displays once per frame
//
// Passed nothing, returns nothing
//

static boolean bsdown; // Is backspace down?
static int bscounter;

void HU_Ticker(void)
{
  hud_slot = st_crispyhud ? NUGHUDSLOT : hud_active; // [Nugget] NUGHUD

  plr = &players[displayplayer];         // killough 3/7/98

  hud_automap = (automapactive == AM_FULL); // [Nugget] Minimap

  HU_disable_all_widgets();
  draw_crispy_hud = false;

  // [Nugget] Minimap
  if ((automapactive == AM_FULL && hud_widget_font == 1) || 
      (automapactive != AM_FULL && hud_widget_font == 2) ||
      hud_widget_font == 3)
  {
    boom_font = &big_font;
    CR_BLUE = CR_BLUE2;
  }
  else
  {
    boom_font = &sml_font;
    CR_BLUE = CR_BLUE1;
  }

  // wait a few tics before sending a backspace character
  if (bsdown && bscounter++ > 9)
  {
    HUlib_add_key_to_cur_line(&w_chat, KEY_BACKSPACE);
    bscounter = 8;
  }

  // tick down message counter if message is up
  if (message_counter && !--message_counter)
    message_on = message_nottobefuckedwith = false;

  if (secret_counter && !--secret_counter)
    secret_on = false;

  // [Woof!] "A secret is revealed!" message
  if (plr->secretmessage)
  {
    HUlib_add_string_to_cur_line(&w_secret, plr->secretmessage);
    plr->secretmessage = NULL;
    secret_on = true;
    secret_counter = 5*TICRATE/2; // [crispy] 2.5 seconds
  }

  // if messages on, or "Messages Off" is being displayed
  // this allows the notification of turning messages off to be seen
  // display message if necessary

  if ((showMessages || message_dontfuckwithme) && plr->message &&
      (!message_nottobefuckedwith || message_dontfuckwithme))
  {
    //post the message to the message widget
    HUlib_add_string_to_cur_line(&w_message, plr->message);

    // [FG] empty messages clear the whole widget
    if (plr->message[0] == '\0')
      HUlib_clear_all_lines(&w_message);

    // clear the message to avoid posting multiple times
    plr->message = 0;

    message_on = true;       // note a message is displayed
    // start the message persistence counter	      
    message_counter = message_count;

    has_message = true;        // killough 12/98

    // transfer "Messages Off" exception to the "being displayed" variable
    message_nottobefuckedwith = message_dontfuckwithme;

    // clear the flag that "Messages Off" is being posted
    message_dontfuckwithme = 0;
  }

  // check for incoming chat characters
  if (netgame)
  {
    int i, rc;
    char c;

    for (i = 0; i < MAXPLAYERS; i++)
    {
      if (!playeringame[i])
        continue;

      if (i != consoleplayer &&
          (c = players[i].cmd.chatchar))
      {
        if (c <= HU_BROADCAST)
          chat_dest[i] = c;
        else
        {
          if (c >= 'a' && c <= 'z')
            c = (char) shiftxform[(unsigned char) c];

          rc = HUlib_add_key_to_line(&w_inputbuffer[i], c);
          if (rc && c == KEY_ENTER)
          {
            if (w_inputbuffer[i].len &&
                (chat_dest[i] == consoleplayer + 1 ||
                chat_dest[i] == HU_BROADCAST))
            {
              HUlib_add_strings_to_cur_line(&w_message,
                                            *player_names[i],
                                            w_inputbuffer[i].line);

              has_message = true; // killough 12/98
              message_nottobefuckedwith = true;
              message_on = true;
              message_counter = chat_count; // killough 11/98
              S_StartSoundPitch(0, gamemode == commercial ?
                              sfx_radio : sfx_tink, PITCH_NONE);
            }
            HUlib_clear_line(&w_inputbuffer[i]);
          }
        }
        players[i].cmd.chatchar = 0;
      }
    }
  }

  // draw the automap widgets if automap is displayed

  // [Nugget] Powerup timers
  #define SHOWPOWERS (                     \
       plr->powers[pw_infrared] > 0        \
    || plr->powers[pw_invisibility] > 0    \
    || plr->powers[pw_invulnerability] > 0 \
    || plr->powers[pw_ironfeet] > 0        \
  )

  if (automapactive == AM_FULL)
  {
    HU_cond_build_widget(w_stats, hud_level_stats & HUD_WIDGET_AUTOMAP);
    HU_cond_build_widget(&w_sttime, hud_level_time & HUD_WIDGET_AUTOMAP || plr->btuse_tics);
    HU_cond_build_widget(&w_powers, STRICTMODE(hud_power_timers) & HUD_WIDGET_AUTOMAP && SHOWPOWERS); // [Nugget] Powerup timers
    HU_cond_build_widget(&w_coord, STRICTMODE(hud_player_coords) & HUD_WIDGET_AUTOMAP);
  }
  else
  {
    HU_cond_build_widget(w_stats, hud_level_stats & HUD_WIDGET_HUD);
    HU_cond_build_widget(&w_sttime, hud_level_time & HUD_WIDGET_HUD || plr->btuse_tics);
    HU_cond_build_widget(&w_powers, STRICTMODE(hud_power_timers) & HUD_WIDGET_HUD && SHOWPOWERS); // [Nugget] Powerup timers
    HU_cond_build_widget(&w_coord, STRICTMODE(hud_player_coords) & HUD_WIDGET_HUD);
  }

  #undef SHOWPOWERS

  HU_cond_build_widget(&w_fps, plr->cheats & CF_SHOWFPS);
  HU_cond_build_widget(&w_rate, plr->cheats & CF_RENDERSTATS);

  if (hud_displayed &&
      scaledviewheight == SCREENHEIGHT &&
      automap_off)
  {
    if (hud_type == HUD_TYPE_CRISPY)
    {
      if (hud_active > 0)
        draw_crispy_hud = true;
    }
    else
    {
      HU_cond_build_widget(&w_weapon, true);
      HU_cond_build_widget(&w_armor, true);
      HU_cond_build_widget(&w_health, true);
      HU_cond_build_widget(&w_ammo, true);
      HU_cond_build_widget(&w_keys, true);
    }
  }

  // [Nugget] NUGHUD
  if (st_crispyhud)
  {
    /* Loose-chat hack */ {
      hu_widget_t *w = widgets[NUGHUDSLOT];

      while (w->multiline)
      {
        if (w->multiline == &w_chat)
        {
          w->y = nughud.message.y;

          if (*w_message.on)
          {
            for (int i = 0;  i < w_message.numlines;  i++)
            {
              if (w_message.lines[i]->width)
              {
                w->y += (*w_message.font)->line_height;
              }
            }
          }

          break;
        }

        w++;
      }
    }

    // Stacks
    for (int i = 0;  i < NUMNUGHUDSTACKS;  i++)
    {
      nughud_stackqueues[i].offset = 0;

      int secondtime = 0;

      do
      {
        for (int j = 0;  j < NUMSQWIDGETS;  j++)
        {
          widgetpair_t *pair = &nughud_stackqueues[i].pairs[j];

          if (!pair->ntl) { continue; }

          hu_multiline_t *ml = pair->wid->multiline;

          if (ml->built || (ml->on && *ml->on))
          {
            nughud_vlignable_t *stack = &nughud.stacks[i];
            const int line_height = (*ml->font)->line_height;

            if (secondtime && i == pair->ntl->stack - 1)
            {
                pair->wid->y = stack->y - nughud_stackqueues[i].offset;

                if (!((ml == &w_message && nughud.message_defx) || ml == &w_chat)) // Messages and chat hacks
                {
                  switch (stack->align) {
                    case -1:  pair->wid->h_align = align_left;    break;
                    case  0:  pair->wid->h_align = align_center;  break;
                    case  1:  pair->wid->h_align = align_right;   break;
                    default:                                      break;
                  }

                  pair->wid->x = stack->x + NUGHUDWIDESHIFT(stack->wide);
                }
            }

            for (int k = 0;  k < ml->numlines;  k++)
            {
              // Don't account for empty lines beyond the first one;
              // message lines can be empty, and the chat's line-width is always zero
              if (k && !ml->lines[k]->width) { continue; }

              if (!secondtime)
              {
                switch (stack->vlign) {
                  case -1:  nughud_stackqueues[i].offset += line_height;      break;
                  case  0:  nughud_stackqueues[i].offset += line_height / 2;  break;
                  case  1:  default:                                          break;
                }
              }
              else { nughud_stackqueues[i].offset -= line_height; }
            }
          }
        }
      } while (!secondtime++);
    }
  }

  if (plr->btuse_tics)
    plr->btuse_tics--;

  // update crosshair properties
  if (hud_crosshair_on) // [Nugget] Use crosshair toggle
    HU_UpdateCrosshair();

  hud_pending = false;
}

#define QUEUESIZE   128

static char chatchars[QUEUESIZE];
static int  head = 0;
static int  tail = 0;

//
// HU_queueChatChar()
//
// Add an incoming character to the circular chat queue
//
// Passed the character to queue, returns nothing
//
void HU_queueChatChar(char c)
{
  if (((head + 1) & (QUEUESIZE-1)) == tail)
    displaymsg("%s", HUSTR_MSGU);
  else
    {
      chatchars[head++] = c;
      head &= QUEUESIZE-1;
    }
}

//
// HU_dequeueChatChar()
//
// Remove the earliest added character from the circular chat queue
//
// Passed nothing, returns the character dequeued
//
char HU_dequeueChatChar(void)
{
  char c;

  if (head != tail)
    {
      c = chatchars[tail++];
      tail &= QUEUESIZE-1;
    }
  else
    c = 0;
  return c;
}

//
// HU_Responder()
//
// Responds to input events that affect the heads up displays
//
// Passed the event to respond to, returns true if the event was handled
//

boolean HU_Responder(event_t *ev)
{
  static char   lastmessage[HU_MAXLINELENGTH+1];
  char*   macromessage;
  boolean   eatkey = false;
  static boolean  shiftdown = false;
  static boolean  altdown = false;
  int     c;
  int     i;
  int     numplayers;

  static int    num_nobrainers = 0;

  c = (ev->type == ev_keydown) ? ev->data1 : 0;

  numplayers = 0;
  for (i=0 ; i<MAXPLAYERS ; i++)
    numplayers += playeringame[i];

  if (ev->data1 == KEY_RSHIFT)
    {
      shiftdown = ev->type == ev_keydown;
      return false;
    }

  if (ev->data1 == KEY_RALT)
    {
      altdown = ev->type == ev_keydown;
      return false;
    }

  if (M_InputActivated(input_chat_backspace))
  {
    bsdown = true;
    bscounter = 0;
    c = KEY_BACKSPACE;
  }
  else if (M_InputDeactivated(input_chat_backspace))
  {
    bsdown = false;
    bscounter = 0;
  }

  if (ev->type == ev_keyup)
    return false;

  if (!chat_on)
    {
      if (M_InputActivated(input_chat_enter))                         // phares
        {
	  //jff 2/26/98 toggle list of messages

	  if (has_message)
	    {
		  message_counter = message_count;
		  message_on = true;
	    }
          eatkey = true;
        }  //jff 2/26/98 no chat if message review is displayed
      else // killough 10/02/98: no chat if demo playback
        if (!demoplayback)
          {
	    if ((netgame || STRICTMODE(sp_chat)) // [Nugget] Singleplayer chat
		&& M_InputActivated(input_chat))
	      {
		eatkey = chat_on = true;
		HUlib_clear_cur_line(&w_chat);
		HU_queueChatChar(HU_BROADCAST);
	      }//jff 2/26/98
	    else    // killough 11/98: simplify
	      if (netgame && numplayers > 2)
		for (i=0; i<MAXPLAYERS ; i++)
		  if (M_InputActivated(input_chat_dest0 + i))
		  {
		    if (i == consoleplayer)
		      displaymsg("%s", 
			++num_nobrainers <  3 ? HUSTR_TALKTOSELF1 :
	                  num_nobrainers <  6 ? HUSTR_TALKTOSELF2 :
	                  num_nobrainers <  9 ? HUSTR_TALKTOSELF3 :
	                  num_nobrainers < 32 ? HUSTR_TALKTOSELF4 :
                                                HUSTR_TALKTOSELF5 );
                  else
                    if (playeringame[i])
                      {
                        eatkey = chat_on = true;
                        HUlib_clear_cur_line(&w_chat);
                        HU_queueChatChar((char)(i+1));
                        break;
                      }
		  }
          }
    }//jff 2/26/98 no chat functions if message review is displayed
  else
      {
        if (M_InputActivated(input_chat_enter))
        {
          c = KEY_ENTER;
        }

        // send a macro
        if (altdown)
          {
            c = c - '0';
            if (c < 0 || c > 9)
              return false;
            // fprintf(stderr, "got here\n");
            macromessage = chat_macros[c];
      
            // kill last message with a '\n'
            HU_queueChatChar(KEY_ENTER); // DEBUG!!!                // phares
      
            // send the macro message
            while (*macromessage)
              HU_queueChatChar(*macromessage++);
            HU_queueChatChar(KEY_ENTER);                            // phares
      
            // leave chat mode and notify that it was sent
            chat_on = false;
            strcpy(lastmessage, chat_macros[c]);
            displaymsg("%s", lastmessage);
            eatkey = true;
          }
        else
          {
            if (shiftdown || (c >= 'a' && c <= 'z'))
              c = shiftxform[c];
            eatkey = HUlib_add_key_to_cur_line(&w_chat, c);
            if (eatkey)
              HU_queueChatChar(c);

            if (c == KEY_ENTER)                                     // phares
              {
                chat_on = false;
                if (w_chat.lines[0]->len)
                  {
                    strcpy(lastmessage, w_chat.lines[0]->line);
                    displaymsg("%s", lastmessage);
                  }
              }
            else
              if (c == KEY_ESCAPE)                               // phares
                chat_on = false;
          }
      }
  return eatkey;
}

// [FG] dynamic HUD alignment

static const struct {
  const char *name, *altname;
  hu_multiline_t *const multiline;
} multiline_names[] = {
    {"title",   NULL,     &w_title},
    {"message", NULL,     &w_message},
// [FG] TODO due to its variable width and the trailing cursor,
//      the w_chat widget *must* currently remain left-aligned
//  {"chat",    NULL,     &w_chat},
    {"secret",  NULL,     &w_secret},

    {"ammo",    NULL,     &w_ammo},
    {"armor",   NULL,     &w_armor},
    {"health",  NULL,     &w_health},
    {"keys",    NULL,     &w_keys},
    {"weapon", "weapons", &w_weapon},

    {"monsec", "stats",   &w_monsec},
    {"sttime", "time",    &w_sttime},
    {"powers",  NULL,     &w_powers}, // [Nugget] Powerup Timers
    {"coord",  "coords",  &w_coord},
    {"fps",     NULL,     &w_fps},
    {"rate",    NULL,     &w_rate},
    {NULL},
};

static boolean HU_ReplaceInWidgets (hu_multiline_t *multiline, int hud, align_t h_align, align_t v_align, int x, int y)
{
  int i;

  if (hud < 0 || hud >= MAX_HUDS)
  {
    return false;
  }

  for (i = 0; i < MAX_WIDGETS - 1; i++)
  {
    if (widgets[hud][i].multiline == NULL)
    {
      break;
    }

    if (widgets[hud][i].multiline == multiline)
    {
      widgets[hud][i].h_align = h_align;
      widgets[hud][i].v_align = v_align;
      widgets[hud][i].x = x;
      widgets[hud][i].y = y;

      // [FG] save original alignment
      widgets[hud][i].h_align_orig = widgets[hud][i].h_align;

      return true;
    }
  }

  return false;
}

static boolean HU_AppendToWidgets (hu_multiline_t *multiline, int hud, align_t h_align, align_t v_align, int x, int y)
{
  int i;

  if (hud < 0 || hud >= MAX_HUDS)
  {
    return false;
  }

  for (i = 0; i < MAX_WIDGETS - 1; i++)
  {
    if (widgets[hud][i].multiline == NULL)
    {
      break;
    }
  }

  if (i + 1 >= MAX_WIDGETS)
  {
    return false;
  }

  widgets[hud][i].multiline = multiline;
  widgets[hud][i].h_align = h_align;
  widgets[hud][i].v_align = v_align;
  widgets[hud][i].x = x;
  widgets[hud][i].y = y;

  // [FG] save original alignment
  widgets[hud][i].h_align_orig = widgets[hud][i].h_align;

  widgets[hud][i + 1].multiline = NULL;

  return true;
}

static boolean HU_AddToWidgets (hu_multiline_t *multiline, int hud, align_t h_align, align_t v_align, int x, int y)
{
  if (HU_ReplaceInWidgets(multiline, hud, h_align, v_align, x, y))
  {
    return true;
  }
  else if (HU_AppendToWidgets(multiline, hud, h_align, v_align, x, y))
  {
    return true;
  }

  return false;
}

static hu_multiline_t *HU_MultilineByName (const char *name)
{
  int i;

  for (i = 0; multiline_names[i].name; i++)
  {
    if (strcasecmp(name, multiline_names[i].name) == 0 ||
       (multiline_names[i].altname && strcasecmp(name, multiline_names[i].altname) == 0))
    {
      return multiline_names[i].multiline;
    }
  }

  return NULL;
}

static boolean HU_AddHUDCoords (char *name, int hud, int x, int y)
{
  hu_multiline_t *multiline = HU_MultilineByName(name);

  if (multiline == NULL)
  {
    return false;
  }

  // [FG] relative alignment to the edges
  if (x < 0)
  {
    x += SCREENWIDTH;
  }
  if (y < 0)
  {
    y += SCREENHEIGHT;
  }

  if (x < 0 || x >= SCREENWIDTH || y < 0 || y >= SCREENHEIGHT)
  {
    return false;
  }

  return HU_AddToWidgets(multiline, hud, align_direct, align_direct, x, y);
}

static boolean HU_AddHUDAlignment (char *name, int hud, char *alignstr)
{
  hu_multiline_t *multiline = HU_MultilineByName(name);

  if (multiline == NULL)
  {
    return false;
  }

  if (!strcasecmp(alignstr, "topleft")          || !strcasecmp(alignstr, "upperleft"))
  {
    return HU_AddToWidgets(multiline, hud, align_left, align_top, 0, 0);
  }
  else if (!strcasecmp(alignstr, "topright")    || !strcasecmp(alignstr, "upperright"))
  {
    return HU_AddToWidgets(multiline, hud, align_right, align_top, 0, 0);
  }
  else if (!strcasecmp(alignstr, "topcenter")   || !strcasecmp(alignstr, "uppercenter"))
  {
    return HU_AddToWidgets(multiline, hud, align_center, align_top, 0, 0);
  }
  else if (!strcasecmp(alignstr, "bottomleft")  || !strcasecmp(alignstr, "lowerleft"))
  {
    return HU_AddToWidgets(multiline, hud, align_left, align_bottom, 0, 0);
  }
  else if (!strcasecmp(alignstr, "bottomright") || !strcasecmp(alignstr, "lowerright"))
  {
    return HU_AddToWidgets(multiline, hud, align_right, align_bottom, 0, 0);
  }
  else if (!strcasecmp(alignstr, "bottomcenter")|| !strcasecmp(alignstr, "lowercenter"))
  {
    return HU_AddToWidgets(multiline, hud, align_center, align_bottom, 0, 0);
  }

  return false;
}

static void HU_ParseHUD (void)
{
  u_scanner_t *s;
  int hud;
  int lumpnum;
  const char *data;
  int length;

  // [FG] initialize HUDs with Vanilla Doom widgets
  for (hud = 0; hud < MAX_HUDS; hud++)
  {
    HU_AddToWidgets(&w_title,   hud, align_direct, align_bottom, 0, 0);
    HU_AddToWidgets(&w_message, hud, align_direct, align_top,    0, 0);
    HU_AddToWidgets(&w_chat,    hud, align_direct, align_top,    0, 0);
    HU_AddToWidgets(&w_secret , hud, align_center, align_direct, 0, (SCREENHEIGHT - ST_HEIGHT) / 4);
  }

  if ((lumpnum = W_CheckNumForName("WOOFHUD")) == -1)
  {
    return;
  }

  data = W_CacheLumpNum(lumpnum, PU_CACHE);
  length = W_LumpLength(lumpnum);

  s = U_ScanOpen(data, length, "WOOFHUD");

  while (U_HasTokensLeft(s))
  {
    char *name;

    if (!U_CheckToken(s, TK_Identifier))
    {
      U_GetNextToken(s, true);
      continue;
    }

    if (!strcasecmp("HUD", s->string))
    {
      U_MustGetInteger(s);
      hud = s->number;

      if (hud < 0 || hud >= MAX_HUDS)
      {
        U_Error(s, "HUD (%d) must be between 0 and %d", hud, MAX_HUDS - 1);
      }

      continue;
    }

    name = M_StringDuplicate(s->string);

    if (U_CheckToken(s, TK_IntConst))
    {
      int x, y;

      x = s->number;
      U_MustGetInteger(s);
      y = s->number;

      if (!HU_AddHUDCoords(name, hud, x, y))
      {
        U_Error(s, "Cannot set coordinates for widget (%s)", name);
      }
    }
    else
    {
      char *align;

      U_MustGetToken(s, TK_Identifier);
      align = M_StringDuplicate(s->string);

      if (!HU_AddHUDAlignment(name, hud, align))
      {
        U_Error(s, "Cannot set alignment for widget (%s)", name);
      }

      free(align);
    }

    free(name);
  }

  U_ScanClose(s);
}

//----------------------------------------------------------------------------
//
// $Log: hu_stuff.c,v $
// Revision 1.27  1998/05/10  19:03:41  jim
// formatted/documented hu_stuff
//
// Revision 1.26  1998/05/03  22:25:24  killough
// Provide minimal headers at top; nothing else
//
// Revision 1.25  1998/04/28  15:53:58  jim
// Fix message list bug in small screen mode
//
// Revision 1.24  1998/04/22  12:50:14  jim
// Fix lockout from dynamic message change
//
// Revision 1.23  1998/04/05  10:09:51  jim
// added STCFN096 lump
//
// Revision 1.22  1998/03/28  05:32:12  jim
// Text enabling changes for DEH
//
// Revision 1.19  1998/03/17  20:45:23  jim
// added frags to HUD
//
// Revision 1.18  1998/03/15  14:42:16  jim
// added green fist/chainsaw in HUD when berserk
//
// Revision 1.17  1998/03/10  07:07:15  jim
// Fixed display glitch in HUD cycle
//
// Revision 1.16  1998/03/09  11:01:48  jim
// fixed string overflow for DEH, added graphic keys
//
// Revision 1.15  1998/03/09  07:10:09  killough
// Use displayplayer instead of consoleplayer
//
// Revision 1.14  1998/03/05  00:57:37  jim
// Scattered HUD
//
// Revision 1.13  1998/03/04  11:50:48  jim
// Change automap coord display
//
// Revision 1.12  1998/02/26  22:58:26  jim
// Added message review display to HUD
//
// Revision 1.11  1998/02/23  14:20:51  jim
// Merged HUD stuff, fixed p_plats.c to support elevators again
//
// Revision 1.10  1998/02/23  04:26:07  killough
// really allow new hud stuff to be turned off COMPLETELY
//
// Revision 1.9  1998/02/22  12:51:26  jim
// HUD control on F5, z coord, spacing change
//
// Revision 1.7  1998/02/20  18:46:51  jim
// cleanup of HUD control
//
// Revision 1.6  1998/02/19  16:54:53  jim
// Optimized HUD and made more configurable
//
// Revision 1.5  1998/02/18  11:55:55  jim
// Fixed issues with HUD and reduced screen size
//
// Revision 1.3  1998/02/15  02:47:47  phares
// User-defined keys
//
// Revision 1.2  1998/01/26  19:23:22  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:55  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
