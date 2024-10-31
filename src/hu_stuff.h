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
// DESCRIPTION:  Head up display
//
//-----------------------------------------------------------------------------

#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "doomdef.h"
#include "doomtype.h"
#include "hu_command.h"

// [Nugget]
#include "doomstat.h"

// [Nugget] CVARs
extern boolean announce_milestones;
extern boolean sp_chat;

struct event_s;
struct mobj_s;

#define HU_BROADCAST    5

#define HU_MSGTIMEOUT   (4*TICRATE)
#define HU_MSGTIMEOUT2  (5*TICRATE/2) // [crispy] 2.5 seconds

//
// Heads up text
//
void HU_Init(void);
void HU_Start(void);
void HU_disable_all_widgets (void);
void HU_widget_rebuild_sttime(void);

// [Nugget]
struct patch_s;
boolean HU_IsSmallFont(const struct patch_s *const patch);
void HU_InitMonSec(void);

void HU_NughudAlignTime(void); // [Nugget] NUGHUD

boolean HU_Responder(struct event_s *ev);

void HU_Ticker(void);
void HU_Drawer(void);
char HU_dequeueChatChar(void);
void HU_Erase(void);

boolean HU_DemoProgressBar(boolean force);

void HU_ResetMessageColors(void);

void WI_DrawWidgets(void);

// killough 5/2/98: moved from m_misc.c:

//jff 2/16/98 hud supported automap colors added
extern int hudcolor_titl;   // color range of automap level title
extern int hudcolor_xyco;   // color range of new coords on automap
//jff 2/23/98 hud is currently displayed
extern boolean hud_displayed;   // hud is displayed
//jff 2/18/98 hud/status control
extern int hud_active;      // hud mode 0=off, 1=small, 2=full

extern int hud_msg_lines; // [Nugget] Global
extern boolean message_list; // [Nugget] Global
extern boolean hud_msg_scrollup; // [Nugget] Restore message scroll direction toggle

// [Nugget]
typedef enum { secretmessage_off, secretmessage_on, secretmessage_count, } secretmessage_t;
extern secretmessage_t hud_secret_message; // "A secret is revealed!" message

extern int hud_player_coords, hud_level_stats, hud_level_time;
extern int hud_power_timers; // [Nugget] Powerup timers
extern boolean hud_widescreen_widgets;

typedef enum {
  TIMER_USE,
  TIMER_TELEPORT,
  TIMER_KEYPICKUP,
  
  NUMTIMERS
} eventtimer_t;

extern boolean hud_time[NUMTIMERS]; // [Nugget] Support more event timers

extern boolean show_messages;
extern boolean show_toggle_messages;
extern boolean show_pickup_messages;

extern boolean chat_on;
extern boolean message_dontfuckwithme;

extern int playback_tic, playback_totaltics;

extern char **player_names[];

enum
{
  HUD_TYPE_CRISPY,
  HUD_TYPE_BOOM_NO_BARS,
  HUD_TYPE_BOOM,

  NUM_HUD_TYPES
};

extern int hud_type;
extern boolean draw_crispy_hud;

enum
{
  HUD_WIDGET_OFF,
  HUD_WIDGET_AUTOMAP,
  HUD_WIDGET_HUD,
  HUD_WIDGET_ALWAYS,
  HUD_WIDGET_ADVANCED,
};

void HU_BindHUDVariables(void);

byte* HU_ColorByHealth(int health, int maxhealth, boolean invul);

extern int speedometer;

#endif

//----------------------------------------------------------------------------
//
// $Log: hu_stuff.h,v $
// Revision 1.6  1998/05/10  19:03:50  jim
// formatted/documented hu_stuff
//
// Revision 1.5  1998/05/03  22:25:03  killough
// add external declarations for hud options
//
// Revision 1.4  1998/02/18  00:59:04  jim
// Addition of HUD
//
// Revision 1.3  1998/02/15  02:48:12  phares
// User-defined keys
//
// Revision 1.2  1998/01/26  19:26:54  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:56  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
