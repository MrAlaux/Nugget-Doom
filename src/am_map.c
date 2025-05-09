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
//   the automap code
//
//-----------------------------------------------------------------------------

#include <limits.h>
#include <string.h>

#include "am_map.h"
#include "d_deh.h"
#include "d_event.h"
#include "d_player.h"
#include "doomdata.h"
#include "doomdef.h"
#include "doomstat.h"
#include "doomtype.h"
#include "i_video.h"
#include "m_config.h"
#include "m_input.h"
#include "mn_menu.h"
#include "m_misc.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_setup.h"
#include "p_spec.h"
#include "r_defs.h"
#include "r_main.h"
#include "r_state.h"
#include "r_things.h"
#include "st_stuff.h"
#include "st_widgets.h"
#include "tables.h"
#include "v_flextran.h"
#include "v_fmt.h"
#include "v_video.h"
#include "ws_stuff.h"
#include "z_zone.h"

// [Nugget]
#include <math.h>
#include "m_random.h"
#include "p_map.h"
#include "s_sound.h"
#include "sounds.h"

// [Nugget] CVARs
boolean fancy_teleport;

//jff 1/7/98 default automap colors added
static int mapcolor_back;    // map background
static int mapcolor_grid;    // grid lines color
static int mapcolor_wall;    // normal 1s wall color
static int mapcolor_fchg;    // line at floor height change color
static int mapcolor_cchg;    // line at ceiling height change color
static int mapcolor_clsd;    // line at sector with floor=ceiling color
static int mapcolor_rkey;    // red key color
static int mapcolor_bkey;    // blue key color
static int mapcolor_ykey;    // yellow key color
static int mapcolor_rdor;    // red door color  (diff from keys to allow option)
static int mapcolor_bdor;    // blue door color (of enabling one but not other )
static int mapcolor_ydor;    // yellow door color
static int mapcolor_tele;    // teleporter line color
static int mapcolor_secr;    // secret sector boundary color
static int mapcolor_revsecr; // revealed secret sector boundary color
static int mapcolor_trig;    // [Nugget] Trigger-line color
static int mapcolor_exit;    // jff 4/23/98 add exit line color
static int mapcolor_unsn;    // computer map unseen line color
static int mapcolor_flat;    // line with no floor/ceiling changes
static int mapcolor_sprt;    // general sprite color
static int mapcolor_hair;    // crosshair color
static int mapcolor_sngl;    // single player arrow color
static int mapcolor_plyr[4]; // colors for player arrows in multiplayer
static int mapcolor_frnd;    // colors for friends of player
static int mapcolor_item;    // item sprite color
static int mapcolor_enemy;   // enemy sprite color
static int mapcolor_hitbox;  // [Nugget] Hitbox color

//jff 3/9/98 add option to not show secret sectors until entered
static boolean map_secret_after;

enum {
  MAP_KEYED_DOOR_OFF,
  MAP_KEYED_DOOR_COLOR,
  MAP_KEYED_DOOR_FLASH
};

static int map_keyed_door; // keyed doors are colored or flashing

static boolean map_smooth_lines;

static boolean map_hitboxes; // [Nugget] Show thing hitboxes

// [Woof!] FRACTOMAPBITS: overflow-safe coordinate system.
// Written by Andrey Budko (entryway), adapted from prboom-plus/src/am_map.*
#define MAPBITS 12
#define MAPUNIT (1<<MAPBITS)
#define FRACTOMAPBITS (FRACBITS-MAPBITS)

// [Woof!] New radius to use with FRACTOMAPBITS, since orginal
// PLAYERRADIUS macro can't be used in this implementation.
#define MAPPLAYERRADIUS (16*(1<<MAPBITS))

// scale on entry
#define INITSCALEMTOF (int)(.2*FRACUNIT)
// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
#define F_PANINC  4
// [crispy] pan faster by holding run button
#define F2_PANINC 12
// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN        ((int) (1.02*FRACUNIT))
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT       ((int) (FRACUNIT/1.02))

// [crispy] zoom faster with the mouse wheel
#define M2_ZOOMIN       ((int) (1.08*FRACUNIT))
#define M2_ZOOMOUT      ((int) (FRACUNIT/1.08))
#define M2_ZOOMINFAST   ((int) (1.5*FRACUNIT))
#define M2_ZOOMOUTFAST  ((int) (FRACUNIT/1.5))

// [crispy] toggleable pan/zoom speed
static int f_paninc = F_PANINC;
static int m_zoomin_kbd = M_ZOOMIN;
static int m_zoomout_kbd = M_ZOOMOUT;
static int m_zoomin_mouse = M2_ZOOMIN;
static int m_zoomout_mouse = M2_ZOOMOUT;
static boolean mousewheelzoom;

// translates between frame-buffer and map distances
// [FG] fix int overflow that causes map and grid lines to disappear
#define FTOM(x) ((((int64_t)(x)<<16)*scale_ftom)>>16)
#define MTOF(x) ((((int64_t)(x)*scale_mtof)>>16)>>16)
// translates between frame-buffer and map coordinates
#define CXMTOF(x)  (f_x + MTOF((x)-m_x))
#define CYMTOF(y)  (f_y + (f_h - MTOF((y)-m_y)))

typedef struct
{
    int x, y;
} fpoint_t;

typedef struct
{
    fpoint_t a, b;
} fline_t;

typedef struct
{
    mpoint_t a, b;
} mline_t;

//
// The vector graphics for the automap.
//  A line drawing of the player pointing right,
//   starting from the middle.
//
#define R ((8*MAPPLAYERRADIUS)/7)
static mline_t player_arrow[] =
{
  { { -R+R/8, 0 }, { R, 0 } }, // -----
  { { R, 0 }, { R-R/2, R/4 } },  // ----->
  { { R, 0 }, { R-R/2, -R/4 } },
  { { -R+R/8, 0 }, { -R-R/8, R/4 } }, // >---->
  { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
  { { -R+3*R/8, 0 }, { -R+R/8, R/4 } }, // >>--->
  { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } }
};
#undef R
#define NUMPLYRLINES (sizeof(player_arrow)/sizeof(mline_t))

#define R ((8*MAPPLAYERRADIUS)/7)
static mline_t cheat_player_arrow[] =
{ // killough 3/22/98: He's alive, Jim :)
  { { -R+R/8, 0 }, { R, 0 } }, // -----
  { { R, 0 }, { R-R/2, R/4 } },  // ----->
  { { R, 0 }, { R-R/2, -R/4 } },
  { { -R+R/8, 0 }, { -R-R/8, R/4 } }, // >---->
  { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
  { { -R+3*R/8, 0 }, { -R+R/8, R/4 } }, // >>--->
  { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } },
  { { -R/10-R/6, R/4}, {-R/10-R/6, -R/4} },  // J
  { { -R/10-R/6, -R/4}, {-R/10-R/6-R/8, -R/4} },
  { { -R/10-R/6-R/8, -R/4}, {-R/10-R/6-R/8, -R/8} },
  { { -R/10, R/4}, {-R/10, -R/4}},           // F
  { { -R/10, R/4}, {-R/10+R/8, R/4}},
  { { -R/10+R/4, R/4}, {-R/10+R/4, -R/4}},   // F
  { { -R/10+R/4, R/4}, {-R/10+R/4+R/8, R/4}},
};
#undef R
#define NUMCHEATPLYRLINES (sizeof(cheat_player_arrow)/sizeof(mline_t))

//jff 1/5/98 new symbol for keys on automap
#define R (FRACUNIT)
static mline_t cross_mark[] =
{
  { { -R, 0 }, { R, 0} },
  { { 0, -R }, { 0, R } },
};
#undef R
#define NUMCROSSMARKLINES (sizeof(cross_mark)/sizeof(mline_t))
//jff 1/5/98 end of new symbol

#define R (FRACUNIT)
#define np5R (int)(-.5*R)
#define np7R (int)(-.7*R)
#define p7R  (int)(.7*R)

static mline_t thintriangle_guy[] =
{
  { { np5R, np7R }, { R,       0 } },
  { { R,       0 }, { np5R,  p7R } },
  { { np5R,  p7R }, { np5R, np7R } }
};
#undef R
#define NUMTHINTRIANGLEGUYLINES (sizeof(thintriangle_guy)/sizeof(mline_t))

// [Nugget] Square hitbox /---------------------------------------------------

#define R (FRACUNIT)

static mline_t square_hitbox[] =
{
  { { -R,  R }, {  R,  R } }, // Top
  { { -R, -R }, {  R, -R } }, // Bottom
  { { -R,  R }, { -R, -R } }, // Left
  { {  R,  R }, {  R, -R } }, // Right
  { { -R,  R }, {  R, -R } }, // Cross line, UL to LR
  { { -R, -R }, {  R,  R } }  // Cross line, LL to UR
};

#undef R

#define NUMSQUAREHITBOXLINES (sizeof(square_hitbox) / sizeof(mline_t))

// [Nugget] -----------------------------------------------------------------/

int ddt_cheating = 0;         // killough 2/7/98: make global, rename to ddt_*

boolean automap_grid = false;

automapmode_t automapactive = AM_OFF;
static boolean automapfirststart = true;

overlay_t automapoverlay = AM_OVERLAY_OFF;

// location of window on screen
static int  f_x;
static int  f_y;

// size of window on screen
static int  f_w;
static int  f_h;

static mpoint_t m_paninc;    // how far the window pans each tic (map coords)
static fixed_t mtof_zoommul; // how far the window zooms each tic (map coords)
static fixed_t ftom_zoommul; // how far the window zooms each tic (fb coords)

static int64_t m_x, m_y;     // LL x,y window location on the map (map coords)
static int64_t m_x2, m_y2;   // UR x,y window location on the map (map coords)
static int64_t prev_m_x, prev_m_y;

//
// width/height of window on map (map coords)
//
static int64_t  m_w;
static int64_t  m_h;

// based on level size
static fixed_t  min_x;
static fixed_t  min_y;
static fixed_t  max_x;
static fixed_t  max_y;

static fixed_t  max_w;          // max_x-min_x,
static fixed_t  max_h;          // max_y-min_y

// based on player size
static const fixed_t min_h = 2 * MAPPLAYERRADIUS; // const? never changed?

static fixed_t  min_scale_mtof; // used to tell when to stop zooming out
static fixed_t  max_scale_mtof; // used to tell when to stop zooming in

// old stuff for recovery later
static int64_t old_m_w, old_m_h;
static int64_t old_m_x, old_m_y;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

static player_t *plr;           // the player represented by an arrow

static patch_t *marknums[10];   // numbers used for marking by the automap

// killough 2/22/98: Remove limit on automap marks,
// and make variables external for use in savegames.

mpoint_t *markpoints = NULL;    // where the points are
int markpointnum = 0; // next point to be assigned (also number of points now)
int markpointnum_max = 0;       // killough 2/22/98
boolean followplayer = true; // specifies whether to follow the player around

static boolean stopped = true;

// Forward declare for AM_LevelInit
static void AM_drawFline_Vanilla(fline_t* fl, int color);
static void AM_drawFline_Smooth(fline_t* fl, int color);
void (*AM_drawFline)(fline_t*, int) = AM_drawFline_Vanilla;

// [crispy/Woof!] automap rotate mode and square aspect ratio need these early on
static boolean automaprotate = false;
static boolean automapsquareaspect = false;
#define ADJUST_ASPECT_RATIO (correct_aspect_ratio && automapsquareaspect)
static void AM_rotate(int64_t *x, int64_t *y, angle_t a);
static void AM_transformPoint(mpoint_t *pt);
static mpoint_t mapcenter;
static angle_t mapangle;

enum
{
  PAN_UP,
  PAN_DOWN,
  PAN_LEFT,
  PAN_RIGHT,
  ZOOM_IN,
  ZOOM_OUT,
  STATE_NUM
};

static int buttons_state[STATE_NUM] = { 0 };

// [Nugget] /=================================================================

// Tag Finder from PrBoomX ---------------------------------------------------

static boolean findtag;

#define MAGIC_SECTOR_COLOR_MIN 168
#define MAGIC_SECTOR_COLOR_MAX 180
#define MAGIC_LINE_COLOR_MIN 112
#define MAGIC_LINE_COLOR_MAX 124
static int magic_sector_color_pos = MAGIC_SECTOR_COLOR_MIN;
static int magic_line_color_pos = MAGIC_LINE_COLOR_MIN;

static sector_t* magic_sector;
static short     magic_tag = -1;

// Minimap -------------------------------------------------------------------

static int mm_x = 8,
           mm_y = 0,
           mm_ws = -2, // Widescreen shift
           mm_w = 80,
           mm_h = 80;

static boolean mm_under_messages = true;

void AM_UpdateMinimap(
  const int x, const int y, const int ws,
  const int w, const int h,
  const boolean under_messages
)
{
  mm_x = x;
  mm_y = y;
  mm_ws = ws;
  mm_w = w;
  mm_h = h;
  mm_under_messages = under_messages;

  if (automapactive == AM_MINI) { AM_Start(); }
}

static boolean reset_older = true;
static int64_t older_m_x, older_m_y, older_m_w, older_m_h;

#define FOLLOW (followplayer || automapactive == AM_MINI)

// Highlight points of interest ----------------------------------------------

static int highlight_timer;

static int highlight_color[4];

// ---------------------------------------------------------------------------

boolean tanz = false;
static boolean tanzen = false;
static int tanzf = 0;
static int tanzc = 0;
#define TANZC 3
static int tanzd = 0;
#define NUMTANZERFL 14
#define NUMTANZERF  125
static mline_t *GetTanzerF(int f);

// [Nugget] =================================================================/

//
// AM_activateNewScale()
//
// Changes the map scale after zooming or translating
//
// Passed nothing, returns nothing
//
static void AM_activateNewScale(void)
{
  m_x += m_w/2;
  m_y += m_h/2;
  m_w = FTOM(f_w);
  m_h = FTOM(f_h);
  m_x -= m_w/2;
  m_y -= m_h/2;
  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;
}

//
// AM_saveScaleAndLoc()
//
// Saves the current center and zoom
// Affects the variables that remember old scale and loc
//
// Passed nothing, returns nothing
//
static void AM_saveScaleAndLoc(void)
{
  old_m_x = m_x;
  old_m_y = m_y;
  old_m_w = m_w;
  old_m_h = m_h;
}

//
// AM_restoreScaleAndLoc()
//
// restores the center and zoom from locally saved values
// Affects global variables for location and scale
//
// Passed nothing, returns nothing
//
static void AM_restoreScaleAndLoc(void)
{
  m_w = old_m_w;
  m_h = old_m_h;
  if (!FOLLOW)
  {
    m_x = old_m_x;
    m_y = old_m_y;
  }
  else
  {
    m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w/2;
    m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h/2;
  }
  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;

  // Change the scaling multipliers
  scale_mtof = FixedDiv(f_w<<FRACBITS, m_w);
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

//
// AM_addMark()
//
// Adds a marker at the current location
// Affects global variables for marked points
//
// Passed nothing, returns nothing
//
static void AM_addMark(void)
{
  // killough 2/22/98:
  // remove limit on automap marks

  if (markpointnum >= markpointnum_max)
    markpoints = Z_Realloc(markpoints,
                           (markpointnum_max = markpointnum_max ?
                            markpointnum_max*2 : 16) * sizeof(*markpoints),
                           PU_STATIC, 0);

  markpoints[markpointnum].x = m_x + m_w/2;
  markpoints[markpointnum].y = m_y + m_h/2;
  markpointnum++;
}

//
// AM_findMinMaxBoundaries()
//
// Determines bounding box of all vertices,
// sets global variables controlling zoom range.
//
// Passed nothing, returns nothing
//

static void AM_setMinMaxScale(void)
{
  const fixed_t a = FixedDiv(f_w << FRACBITS, max_w);
  const fixed_t b = FixedDiv(f_h << FRACBITS, max_h);

  min_scale_mtof = (a < b) ? a : b;
  max_scale_mtof = FixedDiv(f_h << FRACBITS, min_h);

  if (scale_mtof > max_scale_mtof)
    scale_mtof = max_scale_mtof;

  if (scale_mtof < min_scale_mtof)
    scale_mtof = min_scale_mtof;
}

static void AM_findMinMaxBoundaries(void)
{
  int i;

  min_x = min_y =  INT_MAX;
  max_x = max_y = -INT_MAX;

  for (i=0;i<numvertexes;i++)
  {
    if (vertexes[i].x < min_x)
      min_x = vertexes[i].x;
    else if (vertexes[i].x > max_x)
      max_x = vertexes[i].x;

    if (vertexes[i].y < min_y)
      min_y = vertexes[i].y;
    else if (vertexes[i].y > max_y)
      max_y = vertexes[i].y;
  }

  // [FG] cope with huge level dimensions which span the entire INT range
  max_w = (max_x >>= FRACTOMAPBITS) - (min_x >>= FRACTOMAPBITS);
  max_h = (max_y >>= FRACTOMAPBITS) - (min_y >>= FRACTOMAPBITS);

  AM_setMinMaxScale();
}

void AM_SetMapCenter(fixed_t x, fixed_t y)
{
  m_x = (x >> FRACTOMAPBITS) - m_w / 2;
  m_y = (y >> FRACTOMAPBITS) - m_h / 2;
  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;
}

//
// AM_changeWindowLoc()
//
// Moves the map window by the global variables m_paninc.x, m_paninc.y
//
// Passed nothing, returns nothing
//
static void AM_changeWindowLoc(void)
{
  int64_t incx, incy;

  if (m_paninc.x || m_paninc.y)
  {
    followplayer = 0;
  }

  if (uncapped && leveltime > oldleveltime)
  {
    incx = FixedMul(m_paninc.x, fractionaltic);
    incy = FixedMul(m_paninc.y, fractionaltic);
  }
  else
  {
    incx = m_paninc.x;
    incy = m_paninc.y;
  }

  if (automaprotate)
  {
    AM_rotate(&incx, &incy, ANGLE_MAX - mapangle);
  }
  m_x = prev_m_x + incx;
  m_y = prev_m_y + incy;

  if (m_x + m_w/2 > max_x)
    m_x = max_x - m_w/2;
  else if (m_x + m_w/2 < min_x)
    m_x = min_x - m_w/2;

  if (m_y + m_h/2 > max_y)
    m_y = max_y - m_h/2;
  else if (m_y + m_h/2 < min_y)
    m_y = min_y - m_h/2;

  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;
}


//
// AM_initVariables()
//
// Initialize the variables for the automap
//
// Affects the automap global variables
// Status bar is notified that the automap has been entered
// Passed nothing, returns nothing
//
void AM_initVariables(void)
{
  static event_t st_notify = {.type = ev_keyup, .data1.i = AM_MSGENTERED};

  m_paninc.x = m_paninc.y = 0;
  ftom_zoommul = FRACUNIT;
  mtof_zoommul = FRACUNIT;
  mousewheelzoom = false; // [crispy]

  m_w = FTOM(f_w);
  m_h = FTOM(f_h);

  plr = &players[displayplayer];
  // [Alaux] Don't always snap back to player when reopening the Automap
  if (FOLLOW || automapfirststart)
  {
    m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w/2;
    m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h/2;
    automapfirststart = false;
  }
  AM_Ticker(); // initialize variables for interpolation
  AM_changeWindowLoc();

  // for saving & restoring
  old_m_x = m_x;
  old_m_y = m_y;
  old_m_w = m_w;
  old_m_h = m_h;

  // inform the status bar of the change
  ST_Responder(&st_notify);

  // [Nugget] ----------------------------------------------------------------

  byte *const playpal = W_CacheLumpNum(W_GetNumForName("PLAYPAL"), PU_STATIC);

  const int low = 223;

  highlight_color[0] = I_GetNearestColor(playpal, 255, low, low); // Red
  highlight_color[1] = I_GetNearestColor(playpal, low, low, 255); // Blue
  highlight_color[2] = I_GetNearestColor(playpal, 255, 255, low); // Yellow
  highlight_color[3] = I_GetNearestColor(playpal, 255, 255, 255); // Any (white)
}

//
// AM_loadPics()
//
// Load the patches for the mark numbers
//
// Sets the marknums[i] variables to the patches for each digit
// Passed nothing, returns nothing;
//
static void AM_loadPics(void)
{
  int i;
  char namebuf[9];

  for (i=0;i<10;i++)
  {
    M_snprintf(namebuf, sizeof(namebuf), "AMMNUM%d", i);
    marknums[i] = V_CachePatchName(namebuf, PU_STATIC);
  }
}

//
// AM_unloadPics()
//
// Makes the mark patches purgable
//
// Passed nothing, returns nothing
//
static void AM_unloadPics(void)
{
  int i;

  for (i=0;i<10;i++)
    Z_ChangeTag(marknums[i], PU_CACHE);
}

//
// AM_clearMarks()
//
// Sets the number of marks to 0, thereby clearing them from the display
//
// Affects the global variable markpointnum
// Passed nothing, returns nothing
//
void AM_clearMarks(void)
{
  markpointnum = 0;
}

// [Alaux] Clear just the last mark
static void AM_clearLastMark(void)
{
  if (markpointnum)
    markpointnum--;
}

static void AM_EnableSmoothLines(void)
{
  AM_drawFline = map_smooth_lines ? AM_drawFline_Smooth : AM_drawFline_Vanilla;
}

static void AM_initScreenSize(void)
{
  // killough 2/7/98: get rid of finit_ vars
  // to allow runtime setting of width/height
  //
  // killough 11/98: ... finally add hires support :)

  // [Nugget] Minimap
  if (automapactive == AM_MINI)
  {
    int x = mm_x + video.deltaw;

    x += (abs(mm_ws) == 2) ? video.deltaw                  * (mm_ws / 2)
       : (abs(mm_ws) == 1) ? video.deltaw * ST_GetLayout() *  mm_ws
       :                     0;

    f_x = V_ScaleX(x);
    f_y = V_ScaleY(mm_y);

    if (mm_under_messages)
    { f_y += V_ScaleY((ST_GetNumMessageLines() + 1) * ST_GetMessageFontHeight() + 1); }

     // Don't exceed boundaries of screen
    f_w = V_ScaleX(MIN(SCREENWIDTH  - mm_x, mm_w));
    f_h = V_ScaleY(MIN(SCREENHEIGHT - mm_y, mm_h));

    return;
  }

  f_w = video.width;
  if (automapoverlay && scaledviewheight == SCREENHEIGHT)
    f_h = video.height;
  else
    f_h = V_ScaleY(SCREENHEIGHT - ST_HEIGHT);
}

void AM_ResetScreenSize(void)
{
  int old_h = f_h;

  AM_initScreenSize();

  if (f_h != old_h)
  {
    // Change the scaling multipliers
    scale_mtof = FixedDiv(f_w << FRACBITS, m_w);

    AM_setMinMaxScale();

    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
  }

  AM_activateNewScale();
}

//
// AM_LevelInit()
//
// Initialize the automap at the start of a new level
// should be called at the start of every level
//
// Passed nothing, returns nothing
// Affects automap's global variables
//
static void AM_LevelInit(void)
{
  automapfirststart = true;

  f_x = f_y = 0;

  AM_initScreenSize();

  AM_EnableSmoothLines();

  AM_findMinMaxBoundaries();

  // [crispy] initialize zoomlevel on all maps so that a 4096 units
  // square map would just fit in (MAP01 is 3376x3648 units)
  {
    fixed_t a = FixedDiv(f_w, (max_w>>MAPBITS < 2048) ? 2*(max_w>>MAPBITS) : 4096);
    fixed_t b = FixedDiv(f_h, (max_h>>MAPBITS < 2048) ? 2*(max_h>>MAPBITS) : 4096);
    scale_mtof = FixedDiv((a < b ? a : b), (int) (0.7*MAPUNIT));
  }

  if (scale_mtof > max_scale_mtof)
    scale_mtof = max_scale_mtof;

  if (scale_mtof < min_scale_mtof)
    scale_mtof = min_scale_mtof;

  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

//
// AM_Stop()
//
// Cease automap operations, unload patches, notify status bar
//
// Passed nothing, returns nothing
//
void AM_Stop (void)
{
  static event_t st_notify = {.type = 0, .data1.i = ev_keyup, .data2.i = AM_MSGEXITED};

  memset(buttons_state, 0, sizeof(buttons_state));

  AM_unloadPics();

  ST_Responder(&st_notify);
  stopped = true;

  // [Nugget] Tag Finder from PrBoomX
  findtag = false;
  magic_sector = NULL;
  magic_tag = -1;
}

//
// AM_Start()
//
// Start up automap operations,
//  if a new level, or game start, (re)initialize level variables
//  init map variables
//  load mark patches
//
// Passed nothing, returns nothing
//
void AM_Start()
{
  static int lastlevel = -1, lastepisode = -1;

  // [Nugget] Minimap /-------------------------------------------------------

  static int last_automap = -1,
             last_messages = -1,
             last_layout = -1;

  const int messages_height = ST_GetNumMessageLines();
  const boolean layout = ST_GetLayout();

  // [Nugget] ---------------------------------------------------------------/

  if (!stopped)
    AM_Stop();
  stopped = false;
  if (lastlevel != gamemap || lastepisode != gameepisode
      || last_automap != automapactive || last_messages != messages_height
      || last_layout != layout)
  {
    AM_LevelInit();

    // [Nugget] Minimap
    reset_older = reset_older || lastlevel != gamemap || lastepisode != gameepisode;

    lastlevel = gamemap;
    lastepisode = gameepisode;
    last_automap = automapactive;
    last_messages = messages_height;
    last_layout = layout;
  }
  else
  {
    AM_ResetScreenSize();
  }
  AM_initVariables();
  AM_loadPics();
}

//
// AM_minOutWindowScale()
//
// Set the window scale to the maximum size
//
// Passed nothing, returns nothing
//
static void AM_minOutWindowScale()
{
  scale_mtof = min_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
  AM_activateNewScale();
}

//
// AM_maxOutWindowScale(void)
//
// Set the window scale to the minimum size
//
// Passed nothing, returns nothing
//
static void AM_maxOutWindowScale(void)
{
  scale_mtof = max_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
  AM_activateNewScale();
}

// [Nugget]
void AM_ChangeMode(automapmode_t mode)
{
  const boolean modechange = mode && automapactive && mode != automapactive;
  fixed_t rx=0, ry=0, rw=0, rh=0; // Restored values

  automapactive = mode;

  if (automapactive == AM_MINI)
  { memset(buttons_state, 0, sizeof(buttons_state)); }

  if (modechange)
  {
    rx = older_m_x;
    ry = older_m_y;
    rw = older_m_w;
    rh = older_m_h;
    older_m_x = m_x;
    older_m_y = m_y;
    older_m_w = m_w;
    older_m_h = m_h;
  }

  if (!automapactive)
    AM_Stop();
  else
    AM_Start();

  if (modechange)
  {
    if (reset_older)
    {
      reset_older = false;
    }
    else {
      old_m_x = rx;
      old_m_y = ry;
      old_m_w = rw;
      old_m_h = rh;
      AM_restoreScaleAndLoc();
      AM_activateNewScale();
    }
  }

  if (tanz)
  {
    if (automapactive == AM_FULL)
    {
      static int modechangetic = 0;

      if (gametic - modechangetic >= TICRATE)
      {
        if ((tanzen = Woof_Random() == 156)) // Only because the table doesn't have 157
        {
          tanzf = 0;
          tanzc = TANZC;
          tanzd = TICRATE;
        }
      }

      modechangetic = gametic;
    }
    else { tanzen = false; }
  }
}

//
// AM_Responder()
//
// Handle events (user inputs) in automap mode
//
// Passed an input event, returns true if its handled
//
boolean AM_Responder
( event_t*  ev )
{
  int rc;
  static int bigstate=0;

  if (M_InputActivated(input_speed))
  {
    f_paninc = F2_PANINC;
    m_zoomin_kbd = M2_ZOOMIN;
    m_zoomout_kbd = M2_ZOOMOUT;
    m_zoomin_mouse = M2_ZOOMINFAST;
    m_zoomout_mouse = M2_ZOOMOUTFAST;
  }
  else if (M_InputDeactivated(input_speed))
  {
    f_paninc = F_PANINC;
    m_zoomin_kbd = M_ZOOMIN;
    m_zoomout_kbd = M_ZOOMOUT;
    m_zoomin_mouse = M2_ZOOMIN;
    m_zoomout_mouse = M2_ZOOMOUT;
  }

  rc = false;

  if (automapactive != AM_FULL)
  {
    if (M_InputActivated(input_map) && !WS_Override())
    {
      AM_ChangeMode(AM_FULL);
      viewactive = false;
      rc = true;
    }
    // [Nugget] Minimap: allow zooming
    else if (automapactive == AM_MINI)
    {
      if (ev->type == ev_keydown)
      {
        rc = true;

        if (M_InputActivated(input_map_zoomout))
        {
          buttons_state[ZOOM_OUT] = 1;
        }
        else if (M_InputActivated(input_map_zoomin))
        {
          buttons_state[ZOOM_IN] = 1;
        }
        else { rc = false; }
      }
      else if (ev->type == ev_keyup)
      {
        rc = false;

        if (M_InputDeactivated(input_map_zoomout))
        {
          buttons_state[ZOOM_OUT] = 0;
        }
        else if (M_InputDeactivated(input_map_zoomin))
        {
          buttons_state[ZOOM_IN] = 0;
        }
      }
    }
  }
  else if (ev->type == ev_keydown ||
           ev->type == ev_mouseb_down ||
           ev->type == ev_joyb_down)
  {
    rc = true;
                                                                // phares
    if (M_InputActivated(input_map_right))                      //    |
      if (!followplayer && !WS_HoldOverride())                  //    V
        buttons_state[PAN_RIGHT] = 1;
      else
        rc = false;
    else if (M_InputActivated(input_map_left))
      if (!followplayer && !WS_HoldOverride())
        buttons_state[PAN_LEFT] = 1;
      else
        rc = false;
    else if (M_InputActivated(input_map_up))
      if (!followplayer && !WS_HoldOverride())
        buttons_state[PAN_UP] = 1;
      else
        rc = false;
    else if (M_InputActivated(input_map_down))
      if (!followplayer && !WS_HoldOverride())
        buttons_state[PAN_DOWN] = 1;
      else
        rc = false;
    else if (M_InputActivated(input_map_zoomout))
    {
      if (ev->type == ev_mouseb_down && M_IsMouseWheel(ev->data1.i))
      {
        mousewheelzoom = true;
        mtof_zoommul = m_zoomout_mouse;
        ftom_zoommul = m_zoomin_mouse;
      }
      else
        buttons_state[ZOOM_OUT] = 1;
    }
    else if (M_InputActivated(input_map_zoomin))
    {
      if (ev->type == ev_mouseb_down && M_IsMouseWheel(ev->data1.i))
      {
        mousewheelzoom = true;
        mtof_zoommul = m_zoomin_mouse;
        ftom_zoommul = m_zoomout_mouse;
      }
      else
        buttons_state[ZOOM_IN] = 1;
    }
    else if (M_InputActivated(input_map))
    {
      if (!WS_Override() && !tanzd) // [Nugget]
      {
        bigstate = 0;
        viewactive = true;
        AM_ChangeMode(AM_OFF);
      }
      else
      {
        rc = false;
      }
    }
    else if (M_InputActivated(input_map_gobig))
    {
      bigstate = !bigstate;
      if (bigstate)
      {
        AM_saveScaleAndLoc();
        AM_minOutWindowScale();
      }
      else
        AM_restoreScaleAndLoc();
    }
    else if (M_InputActivated(input_map_follow))
    {
      followplayer = !followplayer;
      memset(buttons_state, 0, sizeof(buttons_state));
      // Ty 03/27/98 - externalized
      togglemsg("%s", followplayer ? s_AMSTR_FOLLOWON : s_AMSTR_FOLLOWOFF);
    }
    else if (M_InputActivated(input_map_grid))
    {
      automap_grid = !automap_grid;      // killough 2/28/98
      // Ty 03/27/98 - *not* externalized
      togglemsg("%s", automap_grid ? s_AMSTR_GRIDON : s_AMSTR_GRIDOFF);
    }
    else if (M_InputActivated(input_map_mark))
    {
      // Ty 03/27/98 - *not* externalized     
      displaymsg("%s %d", s_AMSTR_MARKEDSPOT, markpointnum);
      AM_addMark();
    }
    else if (M_InputActivated(input_map_clear))
    {
      // [Alaux] Clear just the last mark
      if (!markpointnum)
        displaymsg("%s", s_AMSTR_MARKSCLEARED);
      else {
        AM_clearLastMark();
        displaymsg("Cleared spot %d", markpointnum);
      }
    }
    else
    if (M_InputActivated(input_map_overlay))
    {
      if (++automapoverlay > AM_OVERLAY_DARK)
        automapoverlay = AM_OVERLAY_OFF;

      switch (automapoverlay)
      {
        case 2:  togglemsg("Dark Overlay On");        break;
        case 1:  togglemsg("%s", s_AMSTR_OVERLAYON);  break;
        default: togglemsg("%s", s_AMSTR_OVERLAYOFF); break;
      }

      AM_initScreenSize();
      AM_activateNewScale();
    }
    else if (M_InputActivated(input_map_rotate))
    {
      automaprotate = !automaprotate;
      togglemsg("%s", automaprotate ? s_AMSTR_ROTATEON : s_AMSTR_ROTATEOFF);
    }

    // [Nugget] /-------------------------------------------------------------

    // Highlight points of interest
    else if (M_InputActivated(input_map_blink))
    {
      highlight_timer = 4*TICRATE;

      if (markpointnum)
      {
        displaymsg("Highlighting points of interest (%i mark%s)...",
                   markpointnum, (markpointnum == 1) ? "" : "s");
      }
      else { displaymsg("Highlighting points of interest..."); }
    }
    // Minimap
    else if (M_InputActivated(input_map_mini))
    {
      AM_ChangeMode(AM_MINI);
    }
    // Tag Finder from PrBoomX
    else if (M_InputActivated(input_map_tagfinder))
    {
      findtag = !strictmode;
    }
    // Teleport to Automap pointer
    else if (M_InputActivated(input_map_teleport) && casual_play)
    {
      const fixed_t x = (m_x + m_w/2) << FRACTOMAPBITS,
                    y = (m_y + m_h/2) << FRACTOMAPBITS;

      if (!followplayer && R_GetFreecamMode() == FREECAM_CAM)
      {
        R_MoveFreecam(
          x, y,
          R_PointInSubsector(x, y)->sector->floorheight + FRACUNIT
        );
      }
      else if (!followplayer || R_GetFreecamMode() == FREECAM_CAM)
      {
        P_MapStart();

        mobj_t *const mo = plr->mo;
      
        P_TeleportMove(mo, x, y, false);
        mo->z = mo->floorz;
        plr->viewz = mo->z + plr->viewheight - plr->crouchoffset;

        if (fancy_teleport)
        {
          R_SetFOVFX(FOVFX_TELEPORT); // Teleporter zoom

          S_StartSound(
            P_SpawnMobj(
              mo->x + 20 * finecosine[mo->angle>>ANGLETOFINESHIFT],
              mo->y + 20 *   finesine[mo->angle>>ANGLETOFINESHIFT],
              mo->z, MT_TFOG
            ),
            sfx_telept
          );
        }

        P_MapEnd();
      }
      
    }

    // [Nugget] -------------------------------------------------------------/

    else
    {
      rc = false;
    }
  }
  else if (ev->type == ev_keyup ||
           ev->type == ev_mouseb_up ||
           ev->type == ev_joyb_up)
  {
    rc = false;

    if (M_InputDeactivated(input_map_right))
    {
      if (!followplayer)
        buttons_state[PAN_RIGHT] = 0;
    }
    else if (M_InputDeactivated(input_map_left))
    {
      if (!followplayer)
        buttons_state[PAN_LEFT] = 0;
    }
    else if (M_InputDeactivated(input_map_up))
    {
      if (!followplayer)
        buttons_state[PAN_UP] = 0;
    }
    else if (M_InputDeactivated(input_map_down))
    {
      if (!followplayer)
        buttons_state[PAN_DOWN] = 0;
    }
    else if (M_InputDeactivated(input_map_zoomout))
    {
      buttons_state[ZOOM_OUT] = 0;
    }
    else if (M_InputDeactivated(input_map_zoomin))
    {
      buttons_state[ZOOM_IN] = 0;
    }
    // [Nugget] Tag Finder from PrBoomX
    else if (M_InputDeactivated(input_map_tagfinder))
    {
      findtag = false;
    }
  }

  m_paninc.x = 0;
  m_paninc.y = 0;

  if (!followplayer)
  {
    int scaled_f_paninc = (f_paninc * video.xscale) >> FRACBITS;
    if (buttons_state[PAN_RIGHT])
      m_paninc.x += FTOM(scaled_f_paninc);
    if (buttons_state[PAN_LEFT])
      m_paninc.x += -FTOM(scaled_f_paninc);

    scaled_f_paninc = (f_paninc * video.yscale) >> FRACBITS;
    if (buttons_state[PAN_UP])
      m_paninc.y += FTOM(scaled_f_paninc);
    if (buttons_state[PAN_DOWN])
      m_paninc.y += -FTOM(scaled_f_paninc);
  }

  if (!mousewheelzoom)
  {
    mtof_zoommul = FRACUNIT;
    ftom_zoommul = FRACUNIT;

    if (buttons_state[ZOOM_OUT] && !buttons_state[ZOOM_IN])
    {
      mtof_zoommul = m_zoomout_kbd;
      ftom_zoommul = m_zoomin_kbd;
    }
    else if (buttons_state[ZOOM_IN] && !buttons_state[ZOOM_OUT])
    {
      mtof_zoommul = m_zoomin_kbd;
      ftom_zoommul = m_zoomout_kbd;
    }
  }

  return rc;
}

//
// AM_changeWindowScale()
//
// Automap zooming
//
// Passed nothing, returns nothing
//
static void AM_changeWindowScale(void)
{
  // Change the scaling multipliers
  scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

  // [crispy] reset after zooming with the mouse wheel
  if (mousewheelzoom)
  {
    mtof_zoommul = FRACUNIT;
    ftom_zoommul = FRACUNIT;
    mousewheelzoom = false;
  }

  if (scale_mtof < min_scale_mtof)
    AM_minOutWindowScale();
  else if (scale_mtof > max_scale_mtof)
    AM_maxOutWindowScale();
  else
    AM_activateNewScale();
}

//
// AM_doFollowPlayer()
//
// Turn on follow mode - the map scrolls opposite to player motion
//
// Passed nothing, returns nothing
//
static void AM_doFollowPlayer(void)
{
  // [Nugget] Prevent Chasecam from shifting the map view
  m_x = ((viewx - chasexofs) >> FRACTOMAPBITS) - m_w/2;
  m_y = ((viewy - chaseyofs) >> FRACTOMAPBITS) - m_h/2;
  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;
}

//
// killough 10/98: return coordinates, to allow use of a non-follow-mode
// pointer. Allows map inspection without moving player to the location.
//

boolean map_point_coord; // [Nugget] Global

void AM_Coordinates(const mobj_t *mo, fixed_t *x, fixed_t *y, fixed_t *z)
{
  *z = FOLLOW || !map_point_coord || !automapactive ? *x = mo->x, *y = mo->y, mo->z :
    R_PointInSubsector(*x = (m_x+m_w/2) << FRACTOMAPBITS, *y = (m_y+m_h/2) << FRACTOMAPBITS)->sector->floorheight;
}

//
// AM_Ticker()
//
// Updates on gametic - enter follow mode, zoom, or change map location
//
// Passed nothing, returns nothing
//
void AM_Ticker (void)
{
  // [Nugget] /===============================================================

  if (highlight_timer) { highlight_timer--; } //  Highlight points of interest

  // Tag Finder from PrBoomX -------------------------------------------------

  if (findtag)
  {
    const fixed_t tmapx = (m_x + m_w/2);
    const fixed_t tmapy = (m_y + m_h/2);
    const subsector_t *const subsec = R_PointInSubsector(tmapx<<FRACTOMAPBITS,
                                                         tmapy<<FRACTOMAPBITS);

    if (subsec && subsec->sector)
    {
      // if we are close to a tagged line in the sector, choose it instead
      float min_distance = MAX(followplayer ? 24 << MAPBITS : 0,
                               8 * FixedMul(scale_ftom, video.xscale));
      short int min_tag = 0;

      magic_sector = (subsec->sector->tag > 0) ? subsec->sector : NULL;
      magic_tag = -1;

      for (int i = 0;  i < subsec->sector->linecount;  i++)
      {
        const line_t *const l = subsec->sector->lines[i];

        if (l && l->tag > 0)
        {
          if (l->v1 && l->v2)
          {
            const float
              x1 = (l->v1->x >> FRACTOMAPBITS),
              x2 = (l->v2->x >> FRACTOMAPBITS),
              y1 = (l->v1->y >> FRACTOMAPBITS),
              y2 = (l->v2->y >> FRACTOMAPBITS),
              dist = fabs((y2 - y1) * tmapx - (x2 - x1) * tmapy + x2*y1 - y2*x1)
                   / sqrtf(powf(y2 - y1, 2) + powf(x2 - x1, 2));

            if (dist < min_distance) {
              min_distance = dist;
              min_tag = l->tag;
            }
          }
        }
      }

      // only pick the line if the crosshair is "close" to it
      if (min_tag > 0) {
        magic_tag = min_tag;
        magic_sector = NULL;
      }
    }
  }

  if (magic_sector || magic_tag > 0)
  {
    if (++magic_sector_color_pos >= MAGIC_SECTOR_COLOR_MAX)
    { magic_sector_color_pos = MAGIC_SECTOR_COLOR_MIN; }
    
    if (++magic_line_color_pos >= MAGIC_LINE_COLOR_MAX)
    { magic_line_color_pos = MAGIC_LINE_COLOR_MIN; }
  }

  // -------------------------------------------------------------------------

  if (tanzen)
  {
    if (!--tanzc)
    {
      tanzc = TANZC;
      tanzf = (tanzf + 1) % NUMTANZERF;
    }

    if (tanzd) { tanzd--; }
  }
  else { tanzd = 0; }

  // [Nugget] ===============================================================/

  // Change the zoom if necessary.
  if (ftom_zoommul != FRACUNIT)
  {
    AM_changeWindowScale();
  }

  prev_m_x = m_x;
  prev_m_y = m_y;
}


//
// Clear automap frame buffer.
//
static void AM_clearFB(int color)
{
  // [Nugget] Minimap: take `f_x` and `f_y` into account
  int h = f_h;
  byte *src = I_VideoBuffer + ((f_y * video.pitch) + f_x);
  while (h--)
  {
    memset(src, color, f_w);
    src += video.pitch;
  }
}

//
// AM_clipMline()
//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes. If the speed is needed,
// use a hash algorithm to handle the common cases.
//
// Passed the line's coordinates on map and in the frame buffer performs
// clipping on them in the lines frame coordinates.
// Returns true if any part of line was not clipped
//
static boolean AM_clipMline
( mline_t*  ml,
  fline_t*  fl )
{
  enum
  {
    LEFT    =1,
    RIGHT   =2,
    BOTTOM  =4,
    TOP     =8
  };

  register int outcode1 = 0;
  register int outcode2 = 0;
  register int outside;

  fpoint_t  tmp;
  int   dx;
  int   dy;


// [Nugget] Minimap: take `f_x` and `f_y` into account
#define DOOUTCODE(oc, mx, my) \
  (oc) = 0; \
  if ((my) < 0) (oc) |= TOP; \
  else if ((my) >= f_y+f_h) (oc) |= BOTTOM; \
  if ((mx) < 0) (oc) |= LEFT; \
  else if ((mx) >= f_x+f_w) (oc) |= RIGHT;


  // do trivial rejects and outcodes
  if (ml->a.y > m_y2)
  outcode1 = TOP;
  else if (ml->a.y < m_y)
  outcode1 = BOTTOM;

  if (ml->b.y > m_y2)
  outcode2 = TOP;
  else if (ml->b.y < m_y)
  outcode2 = BOTTOM;

  if (outcode1 & outcode2)
  return false; // trivially outside

  if (ml->a.x < m_x)
  outcode1 |= LEFT;
  else if (ml->a.x > m_x2)
  outcode1 |= RIGHT;

  if (ml->b.x < m_x)
  outcode2 |= LEFT;
  else if (ml->b.x > m_x2)
  outcode2 |= RIGHT;

  if (outcode1 & outcode2)
  return false; // trivially outside

  // transform to frame-buffer coordinates.
  fl->a.x = CXMTOF(ml->a.x);
  fl->a.y = CYMTOF(ml->a.y);
  fl->b.x = CXMTOF(ml->b.x);
  fl->b.y = CYMTOF(ml->b.y);

  DOOUTCODE(outcode1, fl->a.x, fl->a.y);
  DOOUTCODE(outcode2, fl->b.x, fl->b.y);

  if (outcode1 & outcode2)
  return false;

  while (outcode1 | outcode2)
  {
    // may be partially inside box
    // find an outside point
    if (outcode1)
      outside = outcode1;
    else
      outside = outcode2;

    // clip to each side
    if (outside & TOP)
    {
      dy = fl->a.y - fl->b.y;
      dx = fl->b.x - fl->a.x;
      // [Woof!] 'int64_t' math to avoid overflows on long lines.
      tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-f_y))/dy);
      tmp.y = f_y; // [Nugget] Minimap: take `f_y` into account
    }
    else if (outside & BOTTOM)
    {
      dy = fl->a.y - fl->b.y;
      dx = fl->b.x - fl->a.x;
      tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-(f_y+f_h)))/dy);
      tmp.y = f_y+f_h-1; // [Nugget] Minimap: take `f_y` into account
    }
    else if (outside & RIGHT)
    {
      dy = fl->b.y - fl->a.y;
      dx = fl->b.x - fl->a.x;
      tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x+f_w-1 - fl->a.x))/dx);
      tmp.x = f_x+f_w-1; // [Nugget] Minimap: take `f_x` into account
    }
    else if (outside & LEFT)
    {
      dy = fl->b.y - fl->a.y;
      dx = fl->b.x - fl->a.x;
      tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x-fl->a.x))/dx);
      tmp.x = f_x; // [Nugget] Minimap: take `f_x` into account
    }

    if (outside == outcode1)
    {
      fl->a = tmp;
      DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    }
    else
    {
      fl->b = tmp;
      DOOUTCODE(outcode2, fl->b.x, fl->b.y);
    }

    if (outcode1 & outcode2)
      return false; // trivially outside
  }

  return true;
}
#undef DOOUTCODE

// [Nugget] Factored out
static void PUTDOT(int xx, int yy, int cc)
{
  if (STRICTMODE(flip_levels)) { xx = f_x*2 + f_w - 1 - xx; } // [Nugget] Flip levels

  // [Nugget] Minimap: take `f_x` and `f_y` into account
  if ((f_x <= xx && xx < f_x+f_w) && (f_y <= yy && yy < f_y+f_h))
    I_VideoBuffer[(yy) * video.pitch + (xx)] = (cc);
}


//
// AM_drawFline()
//
// Draw a line in the frame buffer.
// Classic Bresenham w/ whatever optimizations needed for speed
//
// Passed the frame coordinates of line, and the color to be drawn
// Returns nothing
//
static void AM_drawFline_Vanilla(fline_t* fl, int color)
{
  register int x;
  register int y;
  register int dx;
  register int dy;
  register int sx;
  register int sy;
  register int ax;
  register int ay;
  register int d;

#ifdef RANGECHECK         // killough 2/22/98
  // For debugging only
  if
  (
    // [Nugget] Minimap: take `f_x` and `f_y` into account
       fl->a.x < 0 || fl->a.x >= f_x+f_w
    || fl->a.y < 0 || fl->a.y >= f_y+f_h
    || fl->b.x < 0 || fl->b.x >= f_x+f_w
    || fl->b.y < 0 || fl->b.y >= f_y+f_h
  )
  {
    return;
  }
#endif

// [Nugget] Factored PUTDOT() out

  dx = fl->b.x - fl->a.x;
  ax = 2 * (dx<0 ? -dx : dx);
  sx = dx<0 ? -1 : 1;

  dy = fl->b.y - fl->a.y;
  ay = 2 * (dy<0 ? -dy : dy);
  sy = dy<0 ? -1 : 1;

  x = fl->a.x;
  y = fl->a.y;

  if (ax > ay)
  {
    d = ay - ax/2;
    while (1)
    {
      PUTDOT(x,y,color);
      if (x == fl->b.x) return;
      if (d>=0)
      {
        y += sy;
        d -= ax;
      }
      x += sx;
      d += ay;
    }
  }
  else
  {
    d = ax - ay/2;
    while (1)
    {
      PUTDOT(x, y, color);
      if (y == fl->b.y) return;
      if (d >= 0)
      {
        x += sx;
        d -= ay;
      }
      y += sy;
      d += ax;
    }
  }
}

//
// AM_putWuDot
//
// haleyjd 06/13/09: Pixel plotter for Wu line drawing.
//
static void AM_putWuDot(int x, int y, int color, int weight)
{
  if (STRICTMODE(flip_levels)) { x = f_x*2 + f_w - 1 - x; } // [Nugget] Flip levels

   byte *dest = &I_VideoBuffer[y * video.pitch + x];
   unsigned int *fg2rgb = Col2RGB8[weight];
   unsigned int *bg2rgb = Col2RGB8[64 - weight];
   unsigned int fg, bg;

  // [Nugget] Minimap: take `f_x` and `f_y` into account
  if (!((f_x <= x && x < f_x+f_w) && (f_y <= y && y < f_y+f_h)))
  { return; }

   fg = fg2rgb[color];
   bg = bg2rgb[*dest];
   fg = (fg + bg) | 0x1f07c1f;
   *dest = RGB32k[0][0][fg & (fg >> 15)];
}


// Given 65536, we need 2048; 65536 / 2048 == 32 == 2^5
// Why 2048? ANG90 == 0x40000000 which >> 19 == 0x800 == 2048.
// The trigonometric correction is based on an angle from 0 to 90.
#define wu_fineshift 5

// Given 64 levels in the Col2RGB8 table, 65536 / 64 == 1024 == 2^10
#define wu_fixedshift 10

//
// AM_drawFlineWu
//
// haleyjd 06/12/09: Wu line drawing for the automap, with trigonometric
// brightness correction by SoM. I call this the Wu-McGranahan line drawing
// algorithm.
//
static void AM_drawFline_Smooth(fline_t *fl, int color)
{
   int dx, dy, xdir = 1;
   int x, y;

   // swap end points if necessary
   if(fl->a.y > fl->b.y)
   {
      fpoint_t tmp = fl->a;

      fl->a = fl->b;
      fl->b = tmp;
   }

   // determine change in x, y and direction of travel
   dx = fl->b.x - fl->a.x;
   dy = fl->b.y - fl->a.y;

   if(dx < 0)
   {
      dx   = -dx;
      xdir = -xdir;
   }

   // detect special cases -- horizontal, vertical, and 45 degrees;
   // revert to Bresenham
   if(dx == 0 || dy == 0 || dx == dy)
   {
      AM_drawFline_Vanilla(fl, color);
      return;
   }

   // draw first pixel
   PUTDOT(fl->a.x, fl->a.y, color);

   x = fl->a.x;
   y = fl->a.y;

   if(dy > dx)
   {
      // line is y-axis major.
      uint16_t erroracc = 0,
         erroradj = (uint16_t)(((uint32_t)dx << 16) / (uint32_t)dy);

      while(--dy)
      {
         uint16_t erroracctmp = erroracc;

         erroracc += erroradj;

         // if error has overflown, advance x coordinate
         if(erroracc <= erroracctmp)
            x += xdir;

         y += 1; // advance y

         // the trick is in the trig!
         AM_putWuDot(x, y, color,
                     finecosine[erroracc >> wu_fineshift] >> wu_fixedshift);
         AM_putWuDot(x + xdir, y, color,
                     finesine[erroracc >> wu_fineshift] >> wu_fixedshift);
      }
   }
   else
   {
      // line is x-axis major.
      uint16_t erroracc = 0,
         erroradj = (uint16_t)(((uint32_t)dy << 16) / (uint32_t)dx);

      while(--dx)
      {
         uint16_t erroracctmp = erroracc;

         erroracc += erroradj;

         // if error has overflown, advance y coordinate
         if(erroracc <= erroracctmp)
            y += 1;

         x += xdir; // advance x

         // the trick is in the trig!
         AM_putWuDot(x, y, color,
                     finecosine[erroracc >> wu_fineshift] >> wu_fixedshift);
         AM_putWuDot(x, y + 1, color,
                     finesine[erroracc >> wu_fineshift] >> wu_fixedshift);
      }
   }

   // draw last pixel
   PUTDOT(fl->b.x, fl->b.y, color);
}

//
// AM_drawMline()
//
// Clip lines, draw visible parts of lines.
//
// Passed the map coordinates of the line, and the color to draw it
// Color -1 is special and prevents drawing. Color 247 is special and
// is translated to black, allowing Color 0 to represent feature disable
// in the defaults file.
// Returns nothing.
//
static void AM_drawMline
( mline_t*  ml,
  int   color )
{
  static fline_t fl;

  if (color==-1)  // jff 4/3/98 allow not drawing any sort of line
    return;       // by setting its color to -1
  if (color==247) // jff 4/3/98 if color is 247 (xparent), use black
    color=0;

  if (AM_clipMline(ml, &fl))
    AM_drawFline(&fl, color); // draws it on frame buffer using fb coords
}

//
// AM_drawGrid()
//
// Draws blockmap aligned grid lines.
//
// Passed the color to draw the grid lines
// Returns nothing
//
static void AM_drawGrid(int color)
{
  int64_t x, y;
  int64_t start, end;
  const fixed_t gridsize = MAPBLOCKUNITS << MAPBITS;
  mline_t ml;

  // Figure out start of vertical gridlines
  start = m_x;
  if (automaprotate)
  {
    start -= m_h / 2;
  }
  // [crispy] fix losing grid lines near the automap boundary
  if ((start-(bmaporgx>>FRACTOMAPBITS))%gridsize)
    start += // (MAPBLOCKUNITS<<FRACBITS)
      - ((start-(bmaporgx>>FRACTOMAPBITS))%gridsize);
  end = m_x + m_w;
  if (automaprotate)
  {
    end += m_h / 2;
  }

  // draw vertical gridlines
  for (x=start; x<end; x+=gridsize)
  {
    ml.a.x = x;
    ml.b.x = x;
    // [crispy] moved here
    ml.a.y = m_y;
    ml.b.y = m_y+m_h;
    if (automaprotate || ADJUST_ASPECT_RATIO)
    {
      ml.a.y -= m_w / 2;
      ml.b.y += m_w / 2;
    }
    AM_transformPoint(&ml.a);
    AM_transformPoint(&ml.b);
    AM_drawMline(&ml, color);
  }

  // Figure out start of horizontal gridlines
  start = m_y;
  if (automaprotate || ADJUST_ASPECT_RATIO)
  {
    start -= m_w / 2;
  }
  // [crispy] fix losing grid lines near the automap boundary
  if ((start-(bmaporgy>>FRACTOMAPBITS))%gridsize)
    start += // (MAPBLOCKUNITS<<FRACBITS)
      - ((start-(bmaporgy>>FRACTOMAPBITS))%gridsize);
  end = m_y + m_h;
  if (automaprotate || ADJUST_ASPECT_RATIO)
  {
    end += m_w / 2;
  }

  // draw horizontal gridlines
  for (y=start; y<end; y+=gridsize)
  {
    ml.a.y = y;
    ml.b.y = y;
    // [crispy] moved here
    ml.a.x = m_x;
    ml.b.x = m_x + m_w;
    if (automaprotate)
    {
      ml.a.x -= m_h / 2;
      ml.b.x += m_h / 2;
    }
    AM_transformPoint(&ml.a);
    AM_transformPoint(&ml.b);
    AM_drawMline(&ml, color);
  }
}

//
// AM_DoorColor()
//
// Returns the 'color' or key needed for a door linedef type
//
// Passed the type of linedef, returns:
//   -1 if not a keyed door
//    0 if a red key required
//    1 if a blue key required
//    2 if a yellow key required
//    3 if a multiple keys required
//
// jff 4/3/98 add routine to get color of generalized keyed door
//
static int AM_DoorColor(int type)
{
  if (map_keyed_door == MAP_KEYED_DOOR_OFF)
  {
    return -1;
  }

  if (GenLockedBase <= type && type< GenDoorBase)
  {
    type -= GenLockedBase;
    type = (type & LockedKey) >> LockedKeyShift;
    if (!type || type==7)
      return 3;  //any or all keys
    else return (type-1)%3;
  }
  switch (type)  // closed keyed door
  {
    case 26: case 32: case 99: case 133:
      /*bluekey*/
      return 1;
    case 27: case 34: case 136: case 137:
      /*yellowkey*/
      return 2;
    case 28: case 33: case 134: case 135:
      /*redkey*/
      return 0;
    default:
      return -1; //not a keyed door
  }
  return -1;     //not a keyed door
}

//
// Determines visible lines, draws them.
// This is LineDef based, not LineSeg based.
//
// jff 1/5/98 many changes in this routine
// backward compatibility not needed, so just changes, no ifs
// addition of clauses for:
//    doors opening, keyed door id, secret sectors,
//    teleports, exit lines, key things
// ability to suppress any of added features or lines with no height changes
//
// support for gamma correction in automap abandoned
//
// jff 4/3/98 changed mapcolor_xxxx=0 as control to disable feature
// jff 4/3/98 changed mapcolor_xxxx=-1 to disable drawing line completely
//

#define M_ARRAY_INIT_CAPACITY 500
#include "m_array.h"

typedef struct
{
  mline_t l;
  int color;
} am_line_t;

static am_line_t *lines_1S = NULL;

// [Nugget] Tag Finder from PrBoomX /-----------------------------------------

// Prototype this function
static void AM_drawLineCharacter(mline_t*, int, fixed_t, angle_t, int, fixed_t, fixed_t);

static int AM_isTagFinderLine(const line_t *const line)
{
  int ret = 0;

  if (magic_sector || magic_tag > 0)
  {
    if (line->frontsector && (   (magic_sector  && line->frontsector->tag == magic_sector->tag)
                              || (magic_tag > 0 && line->frontsector->tag == magic_tag)))
    {
      ret |= 0x1;
    }
    else 
    if (line->backsector && (   (magic_sector  && line->backsector->tag == magic_sector->tag)
                             || (magic_tag > 0 && line->backsector->tag == magic_tag)))
    {
      ret |= 0x1;
    }

    if (line->tag > 0 && (line->tag == magic_tag || (magic_sector && (line->tag == magic_sector->tag))))
    {
      ret |= 0x2;
    }
  }

  return ret;
}

// [Nugget] -----------------------------------------------------------------/

static void AM_drawWalls(void)
{
  int i;
  static mline_t l;

  const boolean keyed_door_flash = (map_keyed_door == MAP_KEYED_DOOR_FLASH) && (leveltime & 16);

  // [Nugget] Tag Finder from PrBoomX /---------------------------------------

  typedef struct {
    fixed_t x, y;
    int color;
  } crossmark_t;

  crossmark_t *crossmarks = NULL;

  // [Nugget] ---------------------------------------------------------------/

  // draw the unclipped visible portions of all lines
  for (i=0;i<numlines;i++)
  {
    l.a.x = lines[i].v1->x >> FRACTOMAPBITS;
    l.a.y = lines[i].v1->y >> FRACTOMAPBITS;
    l.b.x = lines[i].v2->x >> FRACTOMAPBITS;
    l.b.y = lines[i].v2->y >> FRACTOMAPBITS;
    AM_transformPoint(&l.a);
    AM_transformPoint(&l.b);

    // [Nugget] Tag Finder from PrBoomX: Highlight sectors and lines /--------

    const int is_tf_line = AM_isTagFinderLine(&lines[i]);

    if (is_tf_line & 0x1)
    {
      if (!lines[i].backsector)
      { array_push(lines_1S, ((am_line_t) {l, magic_sector_color_pos})); }
      else
      { AM_drawMline(&l, magic_sector_color_pos); }

      if (magic_sector_color_pos <= MAGIC_SECTOR_COLOR_MIN+1)
      { array_push(crossmarks, ((crossmark_t) { l.a.x, l.a.y, 229 })); }
    }
    
    if (is_tf_line & 0x2)
    {
      if (!lines[i].backsector)
      { array_push(lines_1S, ((am_line_t) {l, magic_line_color_pos})); }
      else
      { AM_drawMline(&l, magic_line_color_pos); }

      if (magic_line_color_pos <= MAGIC_LINE_COLOR_MIN+1)
      { array_push(crossmarks, ((crossmark_t) { l.a.x, l.a.y, 251 })); }
    }

    if (is_tf_line) { continue; }

    // [Nugget] -------------------------------------------------------------/

    // if line has been seen or IDDT has been used
    if (ddt_cheating || (lines[i].flags & ML_MAPPED))
    {
      if ((lines[i].flags & ML_DONTDRAW) && !ddt_cheating)
        continue;
      {
        /* cph - show keyed doors and lines */
        const int amd = AM_DoorColor(lines[i].special);

        // [Nugget] Highlight keyed lines
        if (amd != -1 && !(lines[i].flags & ML_SECRET)
            && highlight_timer && (highlight_timer % 12) < 2)
        {
            array_push(crossmarks, ((crossmark_t) { l.a.x, l.a.y, highlight_color[amd] })); 
        }

        if ((mapcolor_bdor || mapcolor_ydor || mapcolor_rdor) &&
            !(lines[i].flags & ML_SECRET) &&    /* non-secret */
            (amd != -1)
        )
        {
            if (keyed_door_flash)
            {
               AM_drawMline(&l, mapcolor_grid);
            }
            else switch (amd) // closed keyed door
            {
              case 1:
                /*bluekey*/
                AM_drawMline(&l,
                  mapcolor_bdor? mapcolor_bdor : mapcolor_cchg);
                break;
              case 2:
                /*yellowkey*/
                AM_drawMline(&l,
                  mapcolor_ydor? mapcolor_ydor : mapcolor_cchg);
                break;
              case 0:
                /*redkey*/
                AM_drawMline(&l,
                  mapcolor_rdor? mapcolor_rdor : mapcolor_cchg);
                break;
              case 3:
                /*any or all*/
                AM_drawMline(&l,
                  mapcolor_clsd? mapcolor_clsd : mapcolor_cchg);
                break;
            }
            continue;
        }
      }
      if //jff 4/23/98 add exit lines to automap
      (
        mapcolor_exit &&
        (
          lines[i].special==11 ||
          lines[i].special==52 ||
          lines[i].special==197 ||
          lines[i].special==51  ||
          lines[i].special==124 ||
          lines[i].special==198
        )
      )
      {
        AM_drawMline(&l, keyed_door_flash ? mapcolor_grid : mapcolor_exit); // exit line
        continue;
      }

      // [Nugget] Trigger lines
      #define IsTrigger(l) (        \
        (l).special                 \
        && !(   (l).special == 48   \
             || (l).special == 85   \
             || (l).special == 255) \
      )

      if (!lines[i].backsector)
      {
        if (mapcolor_exit && P_IsDeathExit(lines[i].frontsector))
        {
          array_push(lines_1S, ((am_line_t){l, keyed_door_flash ? mapcolor_grid : mapcolor_exit}));
        }
        // [Nugget] Trigger lines
        else if (ddt_cheating && mapcolor_trig && IsTrigger(lines[i]))
        {
          array_push(lines_1S, ((am_line_t){l, mapcolor_trig}));
        }
        // jff 1/10/98 add new color for 1S secret sector boundary
        else if (mapcolor_secr && //jff 4/3/98 0 is disable
            (
             !map_secret_after &&
             P_IsSecret(lines[i].frontsector)
            )
          )
        {
          // line bounding secret sector
          array_push(lines_1S, ((am_line_t){l, mapcolor_secr}));
        }
        else if (mapcolor_revsecr &&
            (
             P_WasSecret(lines[i].frontsector) &&
             !P_IsSecret(lines[i].frontsector)
            )
          )
        {
          // line bounding revealed secret sector
          array_push(lines_1S, ((am_line_t){l, mapcolor_revsecr}));
        }
        else                               //jff 2/16/98 fixed bug
        {
          // special was cleared
          array_push(lines_1S, ((am_line_t){l, mapcolor_wall}));
        }
      }
      else
      {
        // jff 1/10/98 add color change for all teleporter types
        if
        (
            mapcolor_tele && !(lines[i].flags & ML_SECRET) && 
            (lines[i].special == 39 || lines[i].special == 97 /* ||
            lines[i].special == 125 || lines[i].special == 126 */ )
        )
        { // teleporters
          AM_drawMline(&l, mapcolor_tele);
        }
        // [Nugget] Trigger lines
        else if (ddt_cheating && mapcolor_trig && IsTrigger(lines[i]))
        {
          AM_drawMline(&l, mapcolor_trig);
        }
        else if (lines[i].flags & ML_SECRET)    // secret door
        {
          AM_drawMline(&l, mapcolor_wall);      // wall color
        }
        else if
        (
            mapcolor_clsd &&  
            !(lines[i].flags & ML_SECRET) &&    // non-secret closed door
            ((lines[i].backsector->floorheight==lines[i].backsector->ceilingheight) ||
            (lines[i].frontsector->floorheight==lines[i].frontsector->ceilingheight))
        )
        {
          AM_drawMline(&l, mapcolor_clsd);      // non-secret closed door
        } //jff 1/6/98 show secret sector 2S lines
        else if (mapcolor_exit &&
            (P_IsDeathExit(lines[i].frontsector) ||
             P_IsDeathExit(lines[i].backsector))
        )
        {
          AM_drawMline(&l, keyed_door_flash ? mapcolor_grid : mapcolor_exit);
        }
        else if
        (
            mapcolor_secr && //jff 2/16/98 fixed bug
            (                    // special was cleared after getting it
              !map_secret_after &&
               (
                P_IsSecret(lines[i].frontsector) ||
                P_IsSecret(lines[i].backsector)
               )
            )
        )
        {
          AM_drawMline(&l, mapcolor_secr); // line bounding secret sector
        } //jff 1/6/98 end secret sector line change
        else if
        (
            mapcolor_revsecr &&
            (
              (P_WasSecret(lines[i].frontsector)
               && !P_IsSecret(lines[i].frontsector)) ||
              (P_WasSecret(lines[i].backsector)
               && !P_IsSecret(lines[i].backsector))
            )
        )
        {
          AM_drawMline(&l, mapcolor_revsecr); // line bounding revealed secret sector
        }
        else if (lines[i].backsector->floorheight !=
                  lines[i].frontsector->floorheight)
        {
          AM_drawMline(&l, mapcolor_fchg); // floor level change
        }
        else if (lines[i].backsector->ceilingheight !=
                  lines[i].frontsector->ceilingheight)
        {
          AM_drawMline(&l, mapcolor_cchg); // ceiling level change
        }
        else if (mapcolor_flat && ddt_cheating)
        { 
          AM_drawMline(&l, mapcolor_flat); //2S lines that appear only in IDDT  
        }
      }
    } // now draw the lines only visible because the player has computermap
    else if (plr->powers[pw_allmap]) // computermap visible lines
    {
      if (!(lines[i].flags & ML_DONTDRAW)) // invisible flag lines do not show
      {
        if
        (
          mapcolor_flat
          ||
          !lines[i].backsector
          ||
          lines[i].backsector->floorheight
          != lines[i].frontsector->floorheight
          ||
          lines[i].backsector->ceilingheight
          != lines[i].frontsector->ceilingheight
        )
          AM_drawMline(&l, mapcolor_unsn);
      }
    }
  }

  for (int i = 0; i < array_size(lines_1S); ++i)
  {
    AM_drawMline(&lines_1S[i].l, lines_1S[i].color);
  }
  array_clear(lines_1S);

  // [Nugget] Tag Finder from PrBoomX
  for (int i = 0;  i < array_size(crossmarks);  i++)
  {
    AM_drawLineCharacter(cross_mark, NUMCROSSMARKLINES, 128<<MAPBITS, 0,
                         crossmarks[i].color, crossmarks[i].x, crossmarks[i].y);
  }
  array_clear(crossmarks);
}

//
// AM_rotate()
//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
// Passed the coordinates of a point, and an angle
// Returns the coordinates rotated by the angle
//
static void AM_rotate
( int64_t*  x,
  int64_t*  y,
  angle_t a )
{
  int64_t tmpx;

  a >>= ANGLETOFINESHIFT;

  tmpx =
    FixedMul(*x,finecosine[a])
      - FixedMul(*y,finesine[a]);

  *y   =
    FixedMul(*x,finesine[a])
      + FixedMul(*y,finecosine[a]);

  *x = tmpx;
}

// [crispy] rotate point around map center
// adapted from prboom-plus/src/am_map.c:898-920
// [Woof!] Also, scale y coordinate of point for square aspect ratio
static void AM_transformPoint(mpoint_t *pt)
{
  if (automaprotate)
  {
    int64_t tmpx;
    // [crispy] smooth automap rotation
    angle_t smoothangle = FOLLOW ? ANG90 - viewangle : mapangle;

    pt->x -= mapcenter.x;
    pt->y -= mapcenter.y;

    smoothangle >>= ANGLETOFINESHIFT;

    tmpx = (int64_t)FixedMul(pt->x, finecosine[smoothangle])
        - (int64_t)FixedMul(pt->y, finesine[smoothangle])
        + mapcenter.x;

    pt->y = (int64_t)FixedMul(pt->x, finesine[smoothangle])
          + (int64_t)FixedMul(pt->y, finecosine[smoothangle])
          + mapcenter.y;

    pt->x = tmpx;
  }
  if (ADJUST_ASPECT_RATIO)
  {
    int64_t diff = pt->y - mapcenter.y;
    diff = 5 * diff / 6;
    pt->y = mapcenter.y + diff;
  }
}

//
// AM_drawLineCharacter()
//
// Draws a vector graphic according to numerous parameters
//
// Passed the structure defining the vector graphic shape, the number
// of vectors in it, the scale to draw it at, the angle to draw it at,
// the color to draw it with, and the map coordinates to draw it at.
// Returns nothing
//
static void AM_drawLineCharacter
( mline_t*  lineguy,
  int   lineguylines,
  fixed_t scale,
  angle_t angle,
  int   color,
  fixed_t x,
  fixed_t y )
{
  int   i;
  mline_t l;

  if (automaprotate)
  {
    angle += mapangle;
  }

  for (i=0;i<lineguylines;i++)
  {
    l.a.x = lineguy[i].a.x;
    l.a.y = lineguy[i].a.y;

    if (scale)
    {
      l.a.x = FixedMul(scale, l.a.x);
      l.a.y = FixedMul(scale, l.a.y);
    }

    if (angle)
      AM_rotate(&l.a.x, &l.a.y, angle);

    if (ADJUST_ASPECT_RATIO)
      l.a.y = 5 * l.a.y / 6;

    l.a.x += x;
    l.a.y += y;

    l.b.x = lineguy[i].b.x;
    l.b.y = lineguy[i].b.y;

    if (scale)
    {
      l.b.x = FixedMul(scale, l.b.x);
      l.b.y = FixedMul(scale, l.b.y);
    }

    if (angle)
      AM_rotate(&l.b.x, &l.b.y, angle);

    if (ADJUST_ASPECT_RATIO)
      l.b.y = 5 * l.b.y / 6;

    l.b.x += x;
    l.b.y += y;

    AM_drawMline(&l, color);
  }
}

//
// AM_drawPlayers()
//
// Draws the player arrow in single player,
// or all the player arrows in a netgame.
//
// Passed nothing, returns nothing
//
static void AM_drawPlayers(void)
{
  int   i;
  player_t* p;
  //jff 1/6/98   static int   their_colors[] = { GREENS, GRAYS, BROWNS, REDS };
  int   their_color = -1;
  int   color;
  mpoint_t pt;

  if (!netgame || R_GetFreecamOn()) // [Nugget] Freecam
  {
    // [crispy] smooth player arrow rotation
    const angle_t smoothangle = (automaprotate ? plr->mo->angle : viewangle) - chaseaofs; // [Nugget]

    // interpolate player arrow
    if ((uncapped && leveltime > oldleveltime) || R_GetFreecamOn()) // [Nugget]
    {
        // [Nugget] Prevent Chasecam from shifting the map view
        pt.x = (viewx - chasexofs) >> FRACTOMAPBITS;
        pt.y = (viewy - chaseyofs) >> FRACTOMAPBITS;
    }
    else
    {
        pt.x = plr->mo->x >> FRACTOMAPBITS;
        pt.y = plr->mo->y >> FRACTOMAPBITS;
    }
    AM_transformPoint(&pt);

    if (ddt_cheating)
      AM_drawLineCharacter
      (
        cheat_player_arrow,
        NUMCHEATPLYRLINES,
        0,
        smoothangle,
        mapcolor_sngl,      //jff color
        pt.x,
        pt.y
      );
    else
      AM_drawLineCharacter
      (
        player_arrow,
        NUMPLYRLINES,
        0,
        smoothangle,
        mapcolor_sngl,      //jff color
        pt.x,
        pt.y);

    // [Nugget] Freecam: draw the other arrows
    if (!R_GetFreecamOn())
      return;
  }

  for (i=0;i<MAXPLAYERS;i++)
  {
    angle_t smoothangle;

    their_color++;
    p = &players[i];

    // killough 9/29/98: use !demoplayback so internal demos are no different
    if ( (deathmatch && !demoplayback) && p != plr)
      continue;

    if (!playeringame[i])
      continue;

    if (p->powers[pw_invisibility])
      color = 246; // *close* to black
    else
      color = mapcolor_plyr[their_color];   //jff 1/6/98 use default color

    // [crispy] interpolate other player arrows
    if (uncapped && leveltime > oldleveltime && p->mo->interp)
    {
        pt.x = LerpFixed(p->mo->oldx, p->mo->x) >> FRACTOMAPBITS;
        pt.y = LerpFixed(p->mo->oldy, p->mo->y) >> FRACTOMAPBITS;
    }
    else
    {
        pt.x = p->mo->x >> FRACTOMAPBITS;
        pt.y = p->mo->y >> FRACTOMAPBITS;
    }

    AM_transformPoint(&pt);
    if (automaprotate)
    {
      smoothangle = p->mo->angle;
    }
    else
    {
      smoothangle = LerpAngle(p->mo->oldangle, p->mo->angle);
    }

    AM_drawLineCharacter
    (
      player_arrow,
      NUMPLYRLINES,
      0,
      smoothangle,
      color,
      pt.x,
      pt.y
    );
  }
}

//
// AM_drawThings()
//
// Draws the things on the automap in double IDDT cheat mode
//
// Passed colors and colorrange, no longer used
// Returns nothing
//
static void AM_drawThings
( int colors,
  int  colorrange)
{
  int   i;
  mobj_t* t;
  mpoint_t pt;

  // for all sectors
  for (i=0;i<numsectors;i++)
  {
    t = sectors[i].thinglist;
    while (t) // for all things in that sector
    {
      // [Nugget] Moved player check below, since we want to draw its hitbox

      // [crispy] interpolate thing triangles movement
      if (uncapped && leveltime > oldleveltime && t->interp)
      {
        pt.x = LerpFixed(t->oldx, t->x) >> FRACTOMAPBITS;
        pt.y = LerpFixed(t->oldy, t->y) >> FRACTOMAPBITS;
      }
      else
      {
        pt.x = t->x >> FRACTOMAPBITS;
        pt.y = t->y >> FRACTOMAPBITS;
      }
      AM_transformPoint(&pt);

      // [Nugget] Show hitbox
      if (map_hitboxes)
      {
        AM_drawLineCharacter(
          square_hitbox,
          NUMSQUAREHITBOXLINES,
          t->radius >> FRACTOMAPBITS,
          0,
          mapcolor_hitbox,
          pt.x,
          pt.y
        );
      }

      // [Nugget] Brought here
      // [crispy] do not draw an extra triangle for the player
      if (t == plr->mo)
      {
          t = t->snext;
          continue;
      }

      //jff 1/5/98 case over doomednum of thing being drawn
      if (mapcolor_rkey || mapcolor_ykey || mapcolor_bkey)
      {
        // [Nugget] Make keys flash too
        const boolean key_flash = (map_keyed_door == MAP_KEYED_DOOR_FLASH)
                                  && (leveltime & 16);

        switch(t->info->doomednum)
        {
          //jff 1/5/98 treat keys special
          case 38: case 13: //jff  red key
            AM_drawLineCharacter
            (
              cross_mark,
              NUMCROSSMARKLINES,
              16<<MAPBITS,
              t->angle,
              key_flash ? mapcolor_grid : (mapcolor_rkey != -1 ? mapcolor_rkey : mapcolor_sprt),
              pt.x,
              pt.y
            );
            t = t->snext;
            continue;
          case 39: case 6: //jff yellow key
            AM_drawLineCharacter
            (
              cross_mark,
              NUMCROSSMARKLINES,
              16<<MAPBITS,
              t->angle,
              key_flash ? mapcolor_grid : (mapcolor_ykey != -1 ? mapcolor_ykey : mapcolor_sprt),
              pt.x,
              pt.y
            );
            t = t->snext;
            continue;
          case 40: case 5: //jff blue key
            AM_drawLineCharacter
            (
              cross_mark,
              NUMCROSSMARKLINES,
              16<<MAPBITS,
              t->angle,
              key_flash ? mapcolor_grid : (mapcolor_bkey != -1 ? mapcolor_bkey : mapcolor_sprt),
              pt.x,
              pt.y
            );
            t = t->snext;
            continue;
          default:
            break;
        }
      }
      //jff 1/5/98 end added code for keys

      //jff previously entire code
      AM_drawLineCharacter
      (
        thintriangle_guy,
        NUMTHINTRIANGLEGUYLINES,
        t->radius >> FRACTOMAPBITS, // [crispy] triangle size represents actual thing size
        t->angle,
        // killough 8/8/98: mark friends specially
        ((t->flags & MF_FRIEND) && !t->player) ? mapcolor_frnd :
        /* cph 2006/07/30 - Show count-as-kills in red. */
        ((t->flags & (MF_COUNTKILL | MF_CORPSE)) == MF_COUNTKILL) ? mapcolor_enemy :
        /* bbm 2/28/03 Show countable items in yellow. */
        (t->flags & MF_COUNTITEM) ? mapcolor_item :
        mapcolor_sprt,
        pt.x,
        pt.y
      );
      t = t->snext;
    }
  }
}

//
// AM_drawMarks()
//
// Draw the marked locations on the automap
//
// Passed nothing, returns nothing
//
// killough 2/22/98:
// Rewrote AM_drawMarks(). Removed limit on marks.
//
// killough 11/98: added hires support

static void AM_drawMarks(void)
{
  int i;
  mpoint_t pt;

  for (i=0;i<markpointnum;i++) // killough 2/22/98: remove automap mark limit
    if (markpoints[i].x != -1)
      {
	int w = (5 * video.xscale) >> FRACBITS;
	int h = (6 * video.yscale) >> FRACBITS;
	int fx;
	int fy;
	int j = i;

	// [crispy] center marks around player
	pt.x = markpoints[i].x;
	pt.y = markpoints[i].y;
	AM_transformPoint(&pt);
	fx = CXMTOF(pt.x);
	fy = CYMTOF(pt.y);

	if (STRICTMODE(flip_levels)) { fx = f_x*2 + f_w - 1 - fx; } // [Nugget] Flip levels

	do
	  {
	    int d = j % 10;

	    if (d == 1)           // killough 2/22/98: less spacing for '1'
	      fx += (video.xscale >> FRACBITS);

	    // [Nugget] Minimap: take `f_x` and `f_y` into account
	    if (fx >= f_x && fx < f_x+f_w - w && fy >= f_y && fy < f_y+f_h - h)
	      // [Nugget] Highlight marks
	      V_DrawPatchTranslated(((fx << FRACBITS) / video.xscale) - video.deltaw,
                              (fy << FRACBITS) / video.yscale,
                              marknums[d], (highlight_timer & 8) ? cr_dark : NULL);

	    fx -= w - (video.yscale >> FRACBITS); // killough 2/22/98: 1 space backwards

	    j /= 10;
	  }
	while (j>0);
      }
}

//
// AM_drawCrosshair()
//
// Draw the single point crosshair representing map center
//
// Passed the color to draw the pixel with
// Returns nothing
//
static void AM_drawCrosshair(int color)
{
  // [crispy] do not draw the useless dot on the player arrow
  if (!FOLLOW)
  {
    PUTDOT((f_w + 1) / 2, (f_h + 1) / 2, color); // single point for now
  }
}

// [Nugget] /-----------------------------------------------------------------

void AM_shadeScreen(void)
{
  // Minimap
  if (automapactive == AM_MINI)
  {
    for (int x = f_x;  x < f_x+f_w;  x++)
    {
      for (int y = f_y;  y < f_y+f_h;  y++)
      {
        const int pixel = y * video.pitch + x;
        I_VideoBuffer[pixel] = colormaps[0][automap_overlay_darkening * 256 + I_VideoBuffer[pixel]];
      }
    }
  }
  else if (!MN_MenuIsShaded())
    V_ShadeScreen(automap_overlay_darkening); // [Nugget] Parameterized
}

// [Nugget] -----------------------------------------------------------------/

//
// AM_Drawer()
//
// Draws the entire automap
//
// Passed nothing, returns nothing
//
void AM_Drawer (void)
{
  if (!automapactive) return;

  // move AM_doFollowPlayer and AM_changeWindowLoc from AM_Ticker for
  // interpolation

  if (FOLLOW)
  {
    AM_doFollowPlayer();
  }

  // Change X and Y location.
  if (m_paninc.x || m_paninc.y)
  {
    AM_changeWindowLoc();
  }

  // [crispy/Woof!] required for AM_transformPoint()
  if (automaprotate || ADJUST_ASPECT_RATIO)
  {
    mapcenter.x = m_x + m_w / 2;
    mapcenter.y = m_y + m_h / 2;
    // [crispy] keep the map static if not following the player
    if (automaprotate && FOLLOW)
    {
      mapangle = ANG90 - plr->mo->angle;
    }
  }

  if (automapoverlay == AM_OVERLAY_OFF)
    AM_clearFB(mapcolor_back);       //jff 1/5/98 background default color
  // [Alaux] Dark automap overlay
  else if (automapoverlay == AM_OVERLAY_DARK)
    AM_shadeScreen();

  // [Nugget]
  if (tanzen)
  {
    AM_drawLineCharacter(
      GetTanzerF(tanzf),
      NUMTANZERFL,
      scale_ftom * current_video_height / SCREENHEIGHT,
      automaprotate ? (FOLLOW ? plr->mo->angle - ANG90 : ANGLE_MAX - mapangle) : 0,
      v_lightest_color,
      (m_x + m_x2) / 2,
      (m_y + m_y2) / 2
    );

    return;
  }

  if (automap_grid)                  // killough 2/28/98: change var name
    AM_drawGrid(mapcolor_grid);      //jff 1/7/98 grid default color
  AM_drawWalls();
  AM_drawPlayers();
  if (ddt_cheating==2)
    AM_drawThings(mapcolor_sprt, 0); //jff 1/5/98 default double IDDT sprite
  AM_drawCrosshair(mapcolor_hair);   //jff 1/7/98 default crosshair color

  AM_drawMarks();
}

typedef enum {
  AM_PRESET_VANILLA,
  AM_PRESET_CRISPY,
  AM_PRESET_BOOM,
  AM_PRESET_ZDOOM,
  NUM_AM_PRESETS
} am_preset_t;

static am_preset_t mapcolor_preset;

void AM_ColorPreset(void)
{
  struct
  {
    int *var;
    int color[NUM_AM_PRESETS]; // Vanilla Doom, Crispy, Boom, ZDoom
  } mapcolors[] =
  {                                            // ZDoom CVAR name
    {&mapcolor_back,    {  0,   0, 247, 139}}, // am_backcolor
    {&mapcolor_grid,    {104, 104, 104,  70}}, // am_gridcolor
    {&mapcolor_wall,    {176, 180,  23, 239}}, // am_wallcolor
    {&mapcolor_fchg,    { 64,  70,  55, 135}}, // am_fdwallcolor
    {&mapcolor_cchg,    {231, 163, 215,  76}}, // am_cdwallcolor
    {&mapcolor_clsd,    {  0,   0, 208,   0}},
    {&mapcolor_rkey,    {176, 176, 175, 176}}, // P_GetMapColorForLock()
    {&mapcolor_bkey,    {200, 200, 204, 200}}, // P_GetMapColorForLock()
    {&mapcolor_ykey,    {231, 231, 231, 231}}, // P_GetMapColorForLock()
    {&mapcolor_rdor,    {176, 174, 175, 176}}, // P_GetMapColorForLock()
    {&mapcolor_bdor,    {200, 200, 204, 200}}, // P_GetMapColorForLock()
    {&mapcolor_ydor,    {231, 229, 231, 231}}, // P_GetMapColorForLock()
    {&mapcolor_tele,    {  0, 120, 119, 200}}, // am_intralevelcolor
    {&mapcolor_secr,    {  0,  -1, 252, 251}}, // am_unexploredsecretcolor
    {&mapcolor_revsecr, {  0,  -1, 112, 251}}, // am_secretsectorcolor
    {&mapcolor_exit,    {  0, 209, 208, 176}}, // am_interlevelcolor
    {&mapcolor_unsn,    { 99,  99, 104, 100}}, // am_notseencolor
    {&mapcolor_flat,    { 96,  96,  88,  95}}, // am_tswallcolor
    {&mapcolor_sprt,    {112, 112, 112,   4}}, // am_thingcolor
    {&mapcolor_hair,    { 96,  96, 208,  97}}, // am_xhaircolor
    {&mapcolor_sngl,    {209, 209, 208, 209}}, // am_yourcolor
    {&mapcolor_plyr[0], {112, 112, 112, 112}},
    {&mapcolor_plyr[1], { 96,  96,  88,  88}},
    {&mapcolor_plyr[2], { 64,  64,  64,  64}},
    {&mapcolor_plyr[3], {176, 176, 176, 176}},
    {&mapcolor_frnd,    {252, 252, 252,   4}}, // am_thingcolor_friend
    {&mapcolor_enemy,   {112, 176, 177,   4}}, // am_thingcolor_monster
    {&mapcolor_item,    {112, 231, 231,   4}}, // am_thingcolor_item

    {&hudcolor_titl,    {CR_NONE, CR_GOLD, CR_GOLD, CR_GRAY}}, // DrawAutomapHUD()
  };

  for (int i = 0; i < arrlen(mapcolors); i++)
  {
    *mapcolors[i].var = mapcolors[i].color[mapcolor_preset];
  }

  // [crispy] Make secret wall colors independent from PLAYPAL color indexes
  if (mapcolor_preset == AM_PRESET_CRISPY)
  {
    byte *playpal = W_CacheLumpName("PLAYPAL", PU_CACHE);
    mapcolor_secr = I_GetNearestColor(playpal, 255, 0, 255);
    mapcolor_revsecr = I_GetNearestColor(playpal, 119, 255, 111);
  }

  ST_ResetTitle();
}

void AM_BindAutomapVariables(void)
{
  M_BindBool("followplayer", &followplayer, NULL, true, ss_auto, wad_no,
             "Automap follows the player");
  M_BindNum("automapoverlay", &automapoverlay, NULL, AM_OVERLAY_OFF,
            AM_OVERLAY_OFF, AM_OVERLAY_DARK, ss_auto, wad_no,
            "Automap overlay mode (0 = Off; 1 = On; 2 = Dark)");

  // [Nugget] (CFG-only)
  M_BindNum("automap_overlay_darkening", &automap_overlay_darkening, NULL,
            20, 0, 31, ss_none, wad_no,
            "Darkening level of dark automap overlay");

  M_BindBool("automaprotate", &automaprotate, NULL, false, ss_auto, wad_no,
             "Automap rotation");

  M_BindBool("map_point_coord", &map_point_coord, NULL, true, ss_none, wad_no,
             "Show automap pointer coordinates in non-follow mode");
  M_BindBool("map_secret_after", &map_secret_after, NULL, false, ss_auto, wad_no,
             "Don't highlight secret sectors on the automap before they're revealed");
  M_BindNum("map_keyed_door", &map_keyed_door, NULL,
            MAP_KEYED_DOOR_COLOR, MAP_KEYED_DOOR_OFF, MAP_KEYED_DOOR_FLASH,
            ss_auto, wad_no,
            "Color key-locked doors on the automap (1 = Static; 2 = Flashing)");
  M_BindBool("map_smooth_lines", &map_smooth_lines, NULL, true, ss_none,
             wad_no, "Smooth automap lines");

  // [Nugget]
  M_BindBool("map_hitboxes", &map_hitboxes, NULL,
             false, ss_auto, wad_no,
             "Show thing hitboxes on automap");

  M_BindNum("mapcolor_preset", &mapcolor_preset, NULL, AM_PRESET_BOOM,
            AM_PRESET_VANILLA, AM_PRESET_ZDOOM, ss_auto, wad_no,
            "Automap color preset (0 = Vanilla Doom; 1 = Crispy Doom; 2 = Boom; 3 = ZDoom)");

  M_BindBool("automapsquareaspect", &automapsquareaspect, NULL, true, ss_none, wad_no,
             "Use square aspect ratio in automap");

#define BIND_CR(name, v, help) \
  M_BindNum(#name, &name, NULL, (v), 0, 255, ss_none, wad_yes, help)

  BIND_CR(mapcolor_back, 247, "Color used for the automap background");
  BIND_CR(mapcolor_grid, 104, "Color used for grid lines");
  BIND_CR(mapcolor_wall, 23, "Color used for one-sided walls");
  BIND_CR(mapcolor_fchg, 55, "Color used for lines with floor height changes");
  BIND_CR(mapcolor_cchg, 215, "Color used for lines with ceiling height changes");
  BIND_CR(mapcolor_clsd, 208, "Color used for lines denoting closed doors, objects");
  BIND_CR(mapcolor_rkey, 175, "Color used for red-key sprites");
  BIND_CR(mapcolor_bkey, 204, "Color used for blue-key sprites");
  BIND_CR(mapcolor_ykey, 231, "Color used for yellow-key sprites");
  BIND_CR(mapcolor_rdor, 175, "Color used for closed red doors");
  BIND_CR(mapcolor_bdor, 204, "Color used for closed blue doors");
  BIND_CR(mapcolor_ydor, 231, "Color used for closed yellow doors");
  BIND_CR(mapcolor_tele, 119, "Color used for teleporter lines");
  BIND_CR(mapcolor_secr, 252, "Color used for lines around secret sectors");
  BIND_CR(mapcolor_revsecr, 112, "Color used for lines around revealed secret sectors");
  BIND_CR(mapcolor_trig, 0, "Color used for trigger lines (lines with actions)"); // [Nugget]
  BIND_CR(mapcolor_exit, 0, "Color used for exit lines");
  BIND_CR(mapcolor_unsn, 104, "Color used for lines not seen without computer map");
  BIND_CR(mapcolor_flat, 88, "Color used for lines with no height changes");
  BIND_CR(mapcolor_sprt, 112, "Color used for things");
  BIND_CR(mapcolor_hair, 208, "Color used for the automap pointer/crosshair");
  BIND_CR(mapcolor_sngl, 208, "Color used for the player's arrow (single-player only)");

#define BIND_PLR_CR(num, v, help)                                           \
  M_BindNum("mapcolor_ply"#num, &mapcolor_plyr[(num)-1], NULL, (v), 0, 255, \
            ss_none, wad_yes, help)

  BIND_PLR_CR(1, 112, "Color used for the green player's arrow");
  BIND_PLR_CR(2, 88, "Color used for the gray player's arrow");
  BIND_PLR_CR(3, 64, "Color used for the brown player's arrow");
  BIND_PLR_CR(4, 176, "Color used for the red player's arrow");

  BIND_CR(mapcolor_frnd, 252, "Color used for friends");
  BIND_CR(mapcolor_enemy, 177, "Color used for enemies");
  BIND_CR(mapcolor_item, 231, "Color used for countable items");
  BIND_CR(mapcolor_hitbox, 96, "Color used for thing hitboxes");
}

static mline_t *GetTanzerF(int f)
{
  #define R (FRACUNIT*96)
  #define RX(x) (R * ((x) - 48) / 64)
  #define RY(y) (R * (64 - (y)) / 64)
  #define RL(x1, y1, x2, y2) { { RX(x1), RY(y1) }, { RX(x2), RY(y2) } }
  #define RF( \
    hlx, hly, htx, hty, hrx, hry, tux, tuy, tbx, tby, aux, auy, almx, almy, \
    albx, alby, armx, army, arbx, arby, llmx, llmy, llbx, llby, lrmx, lrmy, lrbx, lrby \
  ) \
    { RL( tux,  tuy,  hlx,  hly), RL( hlx,  hly,  htx,  hty), RL( htx,  hty,  hrx,  hry), RL( hrx,  hry,  tux,  tuy), \
      RL( tux,  tuy,  aux,  auy), RL( aux,  auy,  tbx,  tby), RL( aux,  auy, almx, almy), RL(almx, almy, albx, alby), RL( aux,  auy, armx, army), \
      RL(armx, army, arbx, arby), RL( tbx,  tby, llmx, llmy), RL(llmx, llmy, llbx, llby), RL( tbx,  tby, lrmx, lrmy), RL(lrmx, lrmy, lrbx, lrby), }

  // Thanks Ayba!
  static mline_t tanzer[NUMTANZERF][NUMTANZERFL] =
  {
    RF( 41,  31,  47,  26,  52,  31,  47,  38,  47,  63,  47,  41,  47,  41,  39,  69,  50,  55,  54,  69,  47,  63,  44, 103,  47,  63,  54, 104),
    RF( 41,  32,  46,  26,  52,  31,  46,  38,  47,  62,  46,  41,  43,  53,  39,  69,  46,  41,  54,  69,  47,  62,  44, 103,  50,  82,  54, 104),
    RF( 40,  32,  45,  26,  51,  32,  45,  38,  46,  63,  45,  41,  45,  41,  38,  69,  45,  41,  54,  69,  46,  63,  43, 103,  50,  82,  54, 104),
    RF( 39,  32,  45,  26,  50,  32,  45,  38,  46,  63,  45,  42,  45,  42,  36,  70,  45,  42,  53,  69,  46,  63,  43, 104,  49,  81,  54, 104),
    RF( 38,  33,  44,  27,  49,  33,  44,  38,  45,  63,  44,  42,  44,  42,  34,  70,  44,  42,  53,  69,  43,  82,  43, 104,  45,  63,  54, 104),
    RF( 36,  34,  42,  29,  47,  34,  42,  41,  43,  65,  42,  44,  33,  56,  31,  69,  53,  56,  52,  70,  41,  84,  41, 104,  50,  84,  54, 104),
    RF( 38,  35,  44,  30,  49,  35,  44,  41,  42,  66,  44,  44,  31,  51,  34,  64,  58,  53,  51,  65,  43,  84,  43, 104,  55,  76,  55,  99),
    RF( 40,  34,  45,  28,  51,  34,  45,  40,  43,  64,  45,  43,  34,  46,  43,  53,  57,  48,  50,  58,  43,  83,  43, 103,  58,  68,  56,  91),
    RF( 39,  31,  45,  26,  50,  32,  44,  38,  44,  62,  44,  41,  37,  51,  49,  48,  56,  50,  45,  54,  45,  81,  44, 101,  59,  69,  56,  90),
    RF( 38,  33,  44,  27,  50,  32,  44,  39,  46,  63,  44,  42,  41,  55,  50,  50,  52,  55,  42,  55,  49,  82,  49, 102,  57,  77,  58,  96),
    RF( 39,  37,  45,  31,  50,  36,  45,  43,  48,  67,  45,  46,  38,  58,  47,  58,  51,  59,  40,  61,  45,  83,  48, 102,  53,  84,  59, 102),
    RF( 39,  39,  45,  33,  50,  39,  45,  45,  48,  68,  45,  48,  34,  53,  43,  57,  47,  57,  34,  59,  39,  74,  43,  92,  48,  68,  59, 103),
    RF( 38,  35,  43,  30,  50,  35,  44,  41,  45,  65,  44,  45,  35,  48,  47,  50,  50,  55,  37,  53,  30,  70,  34,  88,  45,  65,  59, 101),
    RF( 37,  33,  42,  28,  49,  34,  43,  40,  40,  64,  43,  43,  36,  53,  46,  49,  46,  57,  37,  52,  30,  77,  26,  97,  48,  79,  47, 100),
    RF( 35,  35,  41,  29,  46,  36,  40,  41,  38,  66,  40,  45,  35,  58,  45,  57,  45,  60,  34,  60,  38,  66,  25, 103,  42,  83,  38, 103),
    RF( 33,  38,  39,  32,  44,  38,  38,  44,  35,  68,  38,  47,  34,  56,  44,  64,  45,  58,  35,  66,  28,  84,  26, 103,  46,  79,  41,  97),
    RF( 33,  37,  39,  31,  45,  37,  39,  43,  37,  65,  39,  46,  32,  52,  44,  58,  46,  51,  34,  59,  28,  83,  26, 102,  52,  68,  44,  89),
    RF( 33,  34,  39,  28,  45,  34,  39,  40,  39,  63,  39,  43,  30,  51,  41,  51,  47,  48,  34,  52,  30,  81,  27, 101,  54,  67,  48,  89),
    RF( 33,  32,  38,  26,  44,  32,  39,  38,  42,  63,  40,  42,  36,  53,  45,  49,  46,  55,  37,  54,  35,  79,  36, 100,  52,  79,  55,  99),
    RF( 34,  35,  40,  29,  46,  35,  40,  41,  44,  66,  41,  44,  38,  56,  48,  53,  47,  58,  38,  60,  39,  84,  42, 104,  52,  82,  55, 104),
    RF( 36,  36,  42,  31,  48,  36,  43,  42,  45,  67,  43,  46,  38,  55,  49,  55,  52,  56,  42,  60,  40,  85,  43, 105,  59,  75,  58,  96),
    RF( 38,  35,  44,  29,  50,  35,  44,  41,  46,  65,  45,  45,  37,  51,  48,  49,  53,  49,  41,  52,  40,  83,  43, 104,  60,  66,  59,  88),
    RF( 38,  31,  43,  25,  50,  31,  44,  37,  49,  61,  45,  40,  41,  51,  50,  42,  53,  48,  42,  48,  44,  81,  45, 102,  60,  73,  61,  92),
    RF( 40,  33,  45,  27,  51,  33,  46,  39,  51,  63,  47,  42,  44,  52,  52,  49,  53,  52,  43,  56,  46,  82,  49, 103,  57,  81,  61, 102),
    RF( 41,  36,  46,  31,  52,  36,  47,  42,  53,  67,  48,  46,  44,  54,  51,  61,  53,  56,  44,  66,  40,  80,  46,  99,  57,  85,  60, 104),
    RF( 42,  38,  47,  33,  53,  38,  48,  44,  52,  68,  48,  48,  42,  55,  48,  60,  51,  58,  39,  65,  36,  69,  42,  89,  52,  68,  61, 104),
    RF( 40,  33,  46,  27,  52,  32,  46,  39,  49,  63,  47,  43,  40,  50,  45,  54,  48,  53,  36,  55,  32,  68,  35,  88,  49,  63,  60, 102),
    RF( 39,  31,  45,  25,  50,  31,  45,  37,  45,  62,  45,  41,  38,  49,  42,  48,  46,  49,  34,  51,  33,  78,  27,  98,  48,  81,  47, 100),
    RF( 37,  33,  42,  27,  48,  33,  43,  39,  42,  64,  43,  43,  38,  53,  42,  55,  46,  52,  35,  58,  33,  81,  28, 103,  46,  83,  43, 103),
    RF( 35,  37,  41,  31,  46,  37,  41,  43,  39,  67,  41,  47,  37,  57,  47,  60,  48,  59,  38,  64,  31,  82,  27, 104,  53,  77,  48,  94),
    RF( 35,  36,  41,  30,  47,  36,  42,  42,  41,  66,  42,  46,  34,  54,  46,  53,  50,  54,  37,  57,  32,  83,  28, 103,  56,  69,  54,  89),
    RF( 36,  32,  41,  27,  47,  33,  42,  39,  45,  63,  42,  43,  34,  52,  44,  46,  50,  51,  38,  49,  36,  81,  33, 101,  58,  74,  60,  92),
    RF( 37,  33,  42,  27,  48,  33,  43,  39,  48,  63,  43,  43,  40,  55,  47,  48,  51,  54,  40,  53,  42,  82,  41, 103,  48,  63,  63, 100),
    RF( 39,  36,  45,  30,  50,  36,  45,  42,  51,  66,  46,  46,  42,  58,  52,  57,  54,  58,  43,  63,  41,  82,  40, 102,  51,  66,  62, 104),
    RF( 40,  37,  46,  32,  52,  38,  46,  44,  51,  67,  47,  47,  38,  54,  47,  57,  53,  57,  40,  61,  36,  75,  40,  94,  51,  67,  63, 104),
    RF( 40,  34,  46,  28,  52,  34,  46,  40,  48,  64,  46,  44,  34,  49,  44,  49,  50,  52,  36,  53,  32,  71,  36,  90,  48,  64,  61, 102),
    RF( 40,  31,  45,  26,  51,  31,  46,  38,  45,  63,  46,  41,  37,  51,  43,  47,  47,  51,  34,  50,  35,  78,  29,  97,  52,  80,  53, 100),
    RF( 39,  34,  44,  28,  50,  34,  45,  40,  41,  65,  44,  44,  38,  57,  45,  55,  45,  55,  33,  58,  41,  65,  26, 104,  45,  84,  42, 103),
    RF( 37,  36,  43,  31,  49,  37,  43,  43,  40,  67,  43,  46,  31,  52,  39,  54,  45,  56,  31,  57,  30,  80,  25, 102,  44,  85,  41, 104),
    RF( 34,  36,  39,  30,  45,  36,  39,  42,  38,  66,  39,  45,  25,  47,  37,  51,  40,  51,  24,  47,  25,  72,  24,  93,  42,  86,  40, 104),
    RF( 31,  32,  37,  26,  43,  32,  38,  39,  37,  63,  38,  42,  24,  48,  32,  41,  37,  49,  24,  44,  24,  73,  22,  93,  40,  83,  39, 103),
    RF( 28,  31,  34,  26,  40,  32,  34,  38,  34,  64,  34,  42,  24,  55,  33,  52,  36,  51,  23,  56,  27,  81,  19, 102,  35,  82,  32, 104),
    RF( 27,  36,  32,  31,  38,  36,  32,  43,  33,  68,  32,  47,  16,  57,  26,  56,  40,  56,  28,  61,  33,  68,  19, 105,  40,  82,  36, 100),
    RF( 27,  37,  33,  32,  39,  37,  34,  43,  34,  68,  34,  47,  14,  44,  22,  37,  44,  50,  30,  54,  34,  68,  19, 105,  49,  74,  44,  93),
    RF( 30,  33,  36,  28,  42,  33,  37,  39,  38,  64,  37,  43,  18,  38,  25,  26,  46,  41,  33,  47,  30,  82,  20, 104,  54,  71,  53,  90),
    RF( 34,  31,  39,  25,  45,  31,  40,  37,  45,  62,  41,  41,  26,  34,  16,  22,  47,  41,  35,  48,  39,  79,  34, 100,  45,  62,  63,  96),
    RF( 39,  35,  44,  29,  50,  35,  45,  41,  50,  66,  45,  45,  32,  38,  25,  31,  52,  48,  39,  54,  46,  83,  41, 104,  50,  66,  66, 102),
    RF( 41,  38,  47,  33,  53,  39,  47,  45,  52,  69,  48,  48,  31,  43,  37,  35,  54,  54,  41,  55,  45,  82,  42, 101,  58,  85,  67, 103),
    RF( 42,  37,  48,  31,  53,  37,  48,  43,  51,  67,  48,  47,  31,  42,  36,  31,  53,  53,  40,  50,  40,  79,  42,  94,  51,  67,  67, 102),
    RF( 41,  32,  47,  26,  53,  32,  47,  38,  48,  63,  47,  42,  32,  36,  21,  24,  50,  48,  36,  47,  37,  76,  35,  94,  48,  63,  61, 101),
    RF( 38,  32,  43,  27,  49,  33,  44,  38,  42,  64,  44,  42,  31,  36,  16,  29,  48,  49,  34,  51,  42,  64,  26, 103,  45,  82,  43, 103),
    RF( 33,  36,  39,  31,  45,  36,  39,  42,  39,  68,  39,  46,  22,  41,  22,  38,  48,  55,  34,  54,  33,  85,  26, 105,  39,  68,  42, 102),
    RF( 31,  38,  37,  33,  43,  38,  37,  45,  38,  69,  37,  48,  19,  45,  27,  37,  47,  53,  34,  52,  30,  86,  26, 105,  45,  77,  46,  95),
    RF( 31,  35,  37,  30,  43,  36,  38,  42,  40,  66,  38,  45,  20,  43,  23,  31,  49,  47,  34,  45,  31,  84,  26, 104,  50,  71,  51,  90),
    RF( 32,  31,  37,  25,  43,  31,  38,  37,  43,  63,  39,  41,  23,  37,  13,  25,  49,  40,  34,  41,  36,  79,  32, 102,  52,  78,  58,  97),
    RF( 33,  33,  39,  28,  45,  33,  40,  39,  46,  65,  41,  43,  41,  43,  21,  30,  50,  42,  37,  47,  41,  84,  43, 102,  46,  65,  61, 104),
    RF( 35,  36,  40,  30,  46,  36,  41,  42,  47,  67,  42,  46,  25,  41,  31,  32,  53,  45,  40,  49,  44,  84,  45, 104,  57,  81,  62, 102),
    RF( 37,  37,  43,  31,  48,  37,  43,  43,  48,  67,  44,  46,  27,  42,  31,  31,  55,  44,  41,  47,  44,  86,  45, 104,  62,  72,  62,  93),
    RF( 38,  33,  44,  27,  50,  33,  45,  39,  50,  64,  45,  42,  28,  39,  22,  27,  55,  41,  41,  43,  47,  81,  46, 103,  62,  72,  63,  94),
    RF( 40,  30,  46,  25,  52,  30,  47,  37,  53,  62,  48,  40,  48,  40,  19,  28,  57,  40,  44,  44,  51,  81,  51, 103,  53,  62,  66, 102),
    RF( 44,  34,  49,  28,  55,  34,  50,  40,  54,  65,  51,  44,  33,  44,  32,  38,  58,  48,  45,  50,  49,  83,  51, 102,  54,  65,  65, 105),
    RF( 46,  38,  52,  32,  57,  38,  52,  44,  55,  69,  52,  48,  36,  45,  42,  37,  57,  53,  43,  51,  44,  76,  48,  93,  62,  86,  66, 106),
    RF( 45,  36,  51,  30,  57,  36,  51,  42,  53,  68,  51,  46,  34,  43,  38,  32,  54,  50,  41,  47,  38,  72,  43,  88,  60,  85,  66, 105),
    RF( 43,  30,  49,  25,  54,  31,  50,  37,  48,  63,  50,  40,  34,  33,  24,  21,  53,  43,  38,  42,  36,  78,  33,  97,  54,  81,  58, 103),
    RF( 40,  31,  46,  25,  51,  31,  46,  37,  44,  63,  46,  41,  46,  41,  18,  26,  52,  45,  38,  44,  44,  63,  29, 103,  48,  82,  48, 104),
    RF( 36,  35,  42,  29,  47,  35,  42,  41,  41,  67,  42,  45,  25,  41,  22,  38,  50,  51,  37,  50,  34,  85,  29, 105,  47,  83,  48, 102),
    RF( 33,  38,  39,  32,  45,  38,  39,  44,  40,  69,  39,  48,  20,  46,  30,  39,  47,  50,  34,  50,  31,  85,  28, 105,  49,  77,  50,  94),
    RF( 34,  32,  40,  26,  46,  33,  40,  38,  42,  64,  40,  41,  22,  40,  28,  30,  48,  41,  34,  42,  35,  82,  29, 104,  53,  72,  55,  91),
    RF( 35,  29,  41,  23,  47,  29,  41,  35,  45,  61,  42,  38,  23,  35,  15,  23,  48,  38,  34,  39,  45,  61,  37, 102,  54,  77,  60,  99),
    RF( 38,  31,  43,  25,  49,  31,  44,  37,  48,  63,  45,  40,  45,  40,  18,  27,  50,  41,  35,  42,  48,  63,  45, 104,  48,  63,  61, 104),
    RF( 40,  35,  46,  29,  52,  35,  46,  41,  49,  66,  46,  45,  29,  42,  31,  35,  51,  48,  36,  45,  44,  82,  43, 100,  49,  66,  61, 105),
    RF( 40,  34,  46,  28,  52,  35,  46,  41,  48,  66,  46,  45,  29,  44,  34,  34,  49,  48,  35,  45,  37,  77,  42,  91,  48,  66,  61, 105),
    RF( 39,  30,  45,  24,  51,  30,  45,  36,  46,  63,  45,  40,  27,  39,  22,  26,  48,  44,  34,  42,  35,  76,  36,  94,  46,  63,  58, 103),
    RF( 37,  30,  43,  24,  49,  30,  44,  36,  42,  62,  44,  40,  44,  40,  15,  23,  47,  42,  33,  42,  42,  62,  29, 104,  48,  81,  48, 101),
    RF( 35,  32,  41,  26,  47,  32,  42,  38,  41,  64,  42,  42,  27,  35,  21,  34,  46,  46,  32,  44,  34,  82,  29, 105,  43,  83,  42, 104),
    RF( 33,  34,  39,  28,  45,  34,  40,  40,  38,  66,  39,  44,  24,  38,  29,  32,  43,  45,  28,  43,  27,  79,  28,  99,  42,  86,  41, 105),
    RF( 30,  32,  36,  26,  42,  32,  36,  38,  37,  64,  36,  42,  19,  37,  19,  23,  40,  43,  24,  41,  23,  75,  27,  93,  40,  83,  41, 104),
    RF( 28,  29,  34,  23,  40,  29,  35,  35,  33,  61,  35,  39,  35,  39,   7,  23,  40,  41,  26,  41,  24,  79,  20, 100,  33,  61,  37, 102),
    RF( 25,  30,  31,  24,  36,  31,  31,  36,  31,  63,  31,  40,  15,  39,   7,  47,  45,  48,  34,  52,  31,  63,  18, 103,  31,  63,  29, 104),
    RF( 21,  34,  27,  29,  33,  35,  27,  41,  29,  67,  28,  45,  16,  53,  18,  64,  42,  54,  39,  64,  22,  86,  18, 106,  31,  87,  29, 105),
    RF( 23,  37,  29,  31,  36,  37,  29,  43,  29,  69,  29,  47,  18,  55,  20,  66,  42,  56,  38,  66,  21,  86,  19, 106,  37,  84,  36, 101),
    RF( 30,  32,  36,  26,  42,  32,  36,  38,  31,  64,  35,  42,  22,  51,  22,  62,  46,  51,  41,  61,  20,  82,  11, 101,  41,  80,  44, 100),
    RF( 33,  31,  39,  25,  45,  31,  38,  37,  36,  63,  38,  41,  25,  48,  26,  59,  49,  50,  46,  60,  36,  63,   9,  95,  42,  81,  44, 104),
    RF( 36,  31,  43,  26,  48,  32,  41,  37,  39,  63,  41,  41,  29,  48,  30,  60,  51,  51,  49,  61,  29,  80,  20,  98,  44,  84,  43, 104),
    RF( 41,  31,  47,  26,  52,  31,  45,  38,  41,  63,  44,  41,  31,  47,  32,  60,  56,  51,  51,  60,  37,  81,  35, 104,  50,  81,  50, 101),
    RF( 46,  32,  52,  27,  57,  32,  50,  39,  45,  64,  49,  43,  36,  48,  36,  61,  59,  51,  55,  61,  45,  64,  37, 105,  55,  80,  60, 100),
    RF( 50,  31,  56,  25,  62,  32,  55,  38,  51,  63,  54,  41,  40,  48,  42,  60,  65,  50,  61,  60,  51,  63,  29,  99,  60,  82,  61, 102),
    RF( 53,  32,  59,  27,  65,  33,  58,  38,  58,  64,  57,  43,  44,  51,  48,  61,  69,  50,  67,  61,  58,  64,  31,  95,  62,  84,  59, 105),
    RF( 59,  32,  65,  26,  70,  32,  64,  38,  62,  63,  62,  42,  50,  50,  52,  60,  73,  49,  70,  60,  58,  82,  52,  99,  67,  82,  63, 102),
    RF( 64,  33,  70,  27,  76,  32,  69,  39,  65,  64,  67,  43,  55,  50,  56,  62,  78,  50,  74,  60,  66,  84,  63, 104,  75,  80,  77, 100),
    RF( 68,  33,  74,  27,  80,  32,  73,  39,  70,  64,  73,  42,  60,  51,  62,  61,  82,  51,  80,  61,  70,  64,  61, 102,  80,  81,  84, 102),
    RF( 73,  34,  79,  28,  85,  34,  78,  40,  78,  65,  78,  44,  65,  52,  69,  62,  90,  51,  87,  62,  67,  81,  51,  96,  85,  84,  85, 104),
    RF( 76,  34,  82,  29,  88,  34,  81,  40,  82,  65,  82,  44,  70,  53,  73,  62,  93,  51,  91,  62,  76,  81,  65,  97,  88,  84,  85, 103),
    RF( 80,  34,  86,  28,  91,  34,  85,  39,  85,  64,  85,  43,  73,  52,  75,  62,  96,  50,  94,  60,  86,  83,  84, 103,  94,  82,  91, 101),
    RF( 85,  34,  91,  28,  96,  34,  90,  40,  88,  65,  90,  43,  78,  52,  80,  63, 100,  51,  97,  61,  89,  85,  85, 103,  99,  82, 102, 102),
    RF( 91,  33,  97,  27, 102,  33,  96,  39,  94,  64,  95,  43,  82,  51,  85,  61, 106,  51, 103,  61,  94,  64,  78, 101, 104,  82, 111, 104),
    RF( 95,  35, 101,  29, 107,  34, 100,  40,  99,  66, 100,  44,  86,  53,  90,  62, 111,  52, 108,  63,  99,  66,  68,  95, 108,  83, 112, 105),
    RF( 95,  36, 102,  31, 107,  36, 102,  42, 100,  68, 101,  46,  88,  55,  91,  64, 113,  54, 110,  65,  89,  82,  70,  93, 111,  85, 112, 105),
    RF( 93,  36,  99,  30, 105,  35,  99,  42, 100,  67,  99,  45,  87,  55,  91,  63, 111,  53, 109,  64,  93,  83,  77,  96, 110,  84, 111, 105),
    RF( 84,  33,  90,  27,  96,  32,  91,  39,  97,  64,  92,  42,  82,  52,  87,  62, 105,  47, 105,  59,  96,  83,  91, 102, 109,  81, 114, 101),
    RF( 81,  33,  87,  28,  93,  33,  89,  39,  96,  64,  90,  43,  78,  50,  85,  60, 103,  48, 104,  61,  96,  64,  95, 104, 112,  80, 123,  97),
    RF( 78,  34,  84,  28,  90,  34,  86,  40,  92,  65,  87,  43,  75,  52,  82,  63,  99,  48, 101,  60,  92,  65,  95, 104, 108,  78, 119,  97),
    RF( 75,  32,  81,  26,  87,  32,  83,  38,  89,  63,  85,  41,  72,  51,  79,  61,  96,  46,  98,  59,  88,  83,  91, 102,  99,  80, 100, 101),
    RF( 71,  35,  77,  28,  82,  34,  79,  40,  87,  64,  81,  43,  69,  52,  76,  61,  93,  49,  95,  61,  82,  81,  81,  99,  93,  84,  93, 105),
    RF( 65,  34,  71,  29,  77,  34,  73,  40,  81,  64,  75,  43,  65,  54,  72,  63,  88,  50,  90,  61,  81,  64,  71, 101,  90,  82,  94, 104),
    RF( 63,  34,  68,  29,  74,  35,  70,  40,  75,  65,  71,  44,  60,  52,  66,  62,  84,  51,  84,  63,  73,  83,  70, 104,  90,  79, 106,  96),
    RF( 58,  34,  64,  28,  70,  34,  65,  40,  70,  65,  66,  44,  56,  53,  60,  63,  80,  51,  78,  62,  69,  85,  69, 103,  83,  80,  93,  99),
    RF( 54,  33,  60,  27,  66,  33,  62,  39,  68,  64,  64,  42,  52,  50,  58,  61,  77,  49,  77,  60,  64,  83,  66, 101,  76,  83,  75, 103),
    RF( 49,  34,  54,  28,  60,  34,  57,  40,  64,  64,  58,  43,  47,  52,  53,  62,  72,  49,  72,  61,  59,  81,  56, 100,  69,  84,  69, 105),
    RF( 46,  34,  51,  28,  57,  34,  53,  40,  59,  64,  54,  43,  45,  52,  50,  62,  68,  49,  67,  61,  59,  64,  52, 101,  70,  81,  77, 102),
    RF( 43,  35,  49,  29,  54,  35,  50,  41,  53,  66,  50,  44,  40,  53,  43,  64,  64,  50,  61,  61,  53,  66,  51, 103,  53,  66,  82,  96),
    RF( 41,  34,  47,  29,  52,  35,  48,  40,  52,  65,  48,  44,  37,  52,  42,  63,  62,  50,  60,  61,  51,  83,  51, 102,  52,  65,  72,  97),
    RF( 35,  34,  41,  29,  46,  35,  43,  40,  50,  64,  44,  43,  33,  51,  39,  61,  58,  49,  58,  61,  44,  81,  41,  99,  54,  83,  54, 103),
    RF( 30,  35,  35,  29,  43,  35,  38,  41,  47,  64,  40,  44,  30,  51,  36,  61,  54,  49,  55,  61,  47,  64,  34,  99,  53,  82,  56, 102),
    RF( 25,  35,  31,  30,  36,  35,  34,  41,  42,  65,  36,  44,  26,  52,  32,  62,  50,  48,  50,  61,  42,  65,  32, 103,  42,  65,  66,  97),
    RF( 25,  37,  30,  31,  36,  36,  33,  42,  39,  66,  35,  45,  25,  54,  30,  65,  48,  51,  47,  62,  39,  66,  31, 104,  39,  66,  69,  91),
    RF( 27,  35,  33,  30,  38,  35,  34,  41,  40,  66,  36,  45,  26,  53,  31,  64,  50,  51,  48,  62,  40,  66,  32, 103,  40,  66,  66,  92),
    RF( 31,  33,  37,  27,  43,  32,  38,  39,  42,  64,  38,  43,  29,  54,  33,  66,  51,  51,  49,  62,  42,  64,  32, 103,  52,  80,  60,  98),
    RF( 35,  33,  41,  27,  46,  33,  42,  39,  44,  64,  42,  42,  33,  55,  30,  69,  52,  50,  54,  64,  44,  64,  33, 103,  44,  64,  54, 103),
    RF( 38,  33,  43,  27,  49,  33,  44,  39,  46,  64,  45,  42,  38,  54,  32,  70,  54,  52,  59,  65,  40,  82,  35, 102,  49,  82,  54, 104),
    RF( 40,  33,  46,  28,  51,  33,  46,  39,  48,  64,  47,  42,  47,  42,  36,  70,  55,  54,  60,  68,  43,  81,  42, 103,  50,  84,  54, 104),
    RF( 41,  33,  47,  28,  53,  33,  47,  39,  49,  64,  48,  43,  48,  43,  39,  71,  48,  43,  57,  70,  44,  84,  44, 103,  51,  85,  54, 104),
    RF( 42,  33,  47,  27,  53,  32,  48,  39,  48,  64,  48,  42,  48,  42,  40,  71,  48,  42,  56,  71,  44,  84,  44, 103,  51,  85,  54, 104),
    RF( 42,  32,  47,  27,  52,  32,  47,  38,  48,  63,  48,  42,  48,  42,  39,  70,  48,  42,  55,  70,  44,  83,  44, 103,  51,  84,  54, 104),
    RF( 41,  32,  46,  26,  52,  32,  47,  37,  47,  62,  47,  41,  47,  41,  39,  69,  47,  41,  54,  69,  47,  62,  44, 103,  47,  62,  54, 104),
  };

  #undef RF
  #undef RL
  #undef RY
  #undef RX
  #undef R

  return f[tanzer];
}

//----------------------------------------------------------------------------
//
// $Log: am_map.c,v $
// Revision 1.24  1998/05/10  12:05:24  jim
// formatted/documented am_map
//
// Revision 1.23  1998/05/03  22:13:49  killough
// Provide minimal headers at top; no other changes
//
// Revision 1.22  1998/04/23  13:06:53  jim
// Add exit line to automap
//
// Revision 1.21  1998/04/16  16:16:56  jim
// Fixed disappearing marks after new level
//
// Revision 1.20  1998/04/03  14:45:17  jim
// Fixed automap disables at 0, mouse sens unbounded
//
// Revision 1.19  1998/03/28  05:31:40  jim
// Text enabling changes for DEH
//
// Revision 1.18  1998/03/23  03:06:22  killough
// I wonder
//
// Revision 1.17  1998/03/15  14:36:46  jim
// fixed secrets transfer bug in automap
//
// Revision 1.16  1998/03/10  07:06:21  jim
// Added secrets on automap after found only option
//
// Revision 1.15  1998/03/09  18:29:22  phares
// Created separately bound automap and menu keys
//
// Revision 1.14  1998/03/02  11:22:30  killough
// change grid to automap_grid and make external
//
// Revision 1.13  1998/02/23  04:08:11  killough
// Remove limit on automap marks, save them in savegame
//
// Revision 1.12  1998/02/17  22:58:40  jim
// Fixed bug of vanishinb secret sectors in automap
//
// Revision 1.11  1998/02/15  03:12:42  phares
// Jim's previous comment: Fixed bug in automap from mistaking framebuffer index for mark color
//
// Revision 1.10  1998/02/15  02:47:33  phares
// User-defined keys
//
// Revision 1.8  1998/02/09  02:50:13  killough
// move ddt cheat to st_stuff.c and some cleanup
//
// Revision 1.7  1998/02/02  22:16:31  jim
// Fixed bug in automap that showed secret lines
//
// Revision 1.6  1998/01/26  20:57:54  phares
// Second test of checkin/checkout
//
// Revision 1.5  1998/01/26  20:28:15  phares
// First checkin/checkout script test
//
// Revision 1.4  1998/01/26  19:23:00  phares
// First rev with no ^Ms
//
// Revision 1.3  1998/01/24  11:21:25  jim
// Changed disables in automap to -1 and -2 (nodraw)
//
// Revision 1.1.1.1  1998/01/19  14:02:53  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------

