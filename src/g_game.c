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
// DESCRIPTION:  none
//
//-----------------------------------------------------------------------------

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "am_map.h"
#include "config.h"
#include "d_deh.h" // Ty 3/27/98 deh declarations
#include "d_event.h"
#include "d_iwad.h"
#include "d_main.h"
#include "d_player.h"
#include "d_ticcmd.h"
#include "doomdata.h"
#include "doomdef.h"
#include "doomkeys.h"
#include "doomstat.h"
#include "doomtype.h"
#include "f_finale.h"
#include "g_game.h"
#include "g_nextweapon.h"
#include "g_umapinfo.h"
#include "hu_command.h"
#include "hu_obituary.h"
#include "i_gamepad.h"
#include "i_gyro.h"
#include "i_input.h"
#include "i_printf.h"
#include "i_rumble.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "info.h"
#include "m_argv.h"
#include "m_array.h"
#include "m_config.h"
#include "m_input.h"
#include "m_io.h"
#include "m_misc.h"
#include "m_random.h"
#include "m_swap.h" // [FG] LONG
#include "memio.h"
#include "mn_menu.h"
#include "mn_snapshot.h"
#include "net_defs.h"
#include "p_enemy.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "p_saveg.h"
#include "p_setup.h"
#include "p_tick.h"
#include "p_user.h"
#include "r_data.h"
#include "r_defs.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_sky.h"
#include "r_state.h"
#include "s_musinfo.h"
#include "s_sound.h"
#include "sounds.h"
#include "st_stuff.h"
#include "st_widgets.h"
#include "statdump.h" // [FG] StatCopy()
#include "tables.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "ws_stuff.h"
#include "z_zone.h"

// [Nugget]
#include "d_items.h"
#include "m_cheat.h"
#include "p_spec.h"

// [Nugget] /=================================================================

// CVARs ---------------------------------------------------------------------

boolean one_key_saveload;
boolean improved_weapon_toggles;
boolean skip_ammoless_weapons;
boolean comp_longautoaim;

boolean nugget_devmode;

// ---------------------------------------------------------------------------

boolean minimap_was_on = false; // Minimap: keep it when advancing through levels

boolean ignore_pistolstart = false; // Custom Skill: ignore pistol-start setting

static float mouse_h_modifier = 1.0f,
             mouse_v_modifier = 1.0f;

static void UpdateMouseModifiers(void)
{
  mouse_h_modifier = mouse_v_modifier = 1.0f;

  if (strictmode) { return; }

  // Decrease the intensity of some movements if zoomed in -------------------

  const int zoom = R_GetFOVFX(FOVFX_ZOOM);

  if (zoom)
  { mouse_h_modifier = MAX(1.0f, (float) custom_fov / MAX(1, custom_fov + zoom)); }

  mouse_v_modifier = mouse_h_modifier;

  // Flip levels -------------------------------------------------------------

  if (STRICTMODE(flip_levels)) { mouse_h_modifier = -mouse_h_modifier; }
}

// Periodic auto save --------------------------------------------------------

int autosave_interval;

static boolean saving_periodic_autosave = false;
static int autosave_countdown = 0;

boolean G_SavingPeriodicAutoSave(void)
{
  return saving_periodic_autosave;
}

void G_SetAutoSaveCountdown(int value)
{
  autosave_countdown = value;
}

// Rewind --------------------------------------------------------------------

int rewind_interval;
static int rewind_depth;
static int rewind_timeout;

static boolean keyframe_rw = false;

static boolean rewind_on = true;
static int rewind_countdown = 0;

typedef struct keyframe_s
{
  struct keyframe_s *prev, *next;
  byte *frame;
  size_t length;
} keyframe_t;

static keyframe_t *keyframe_list_head = NULL, *keyframe_list_tail = NULL;

static int keyframe_index = -1;

// Slow Motion ---------------------------------------------------------------

static boolean slow_motion = false;
static int slow_motion_factor = SLOWMO_FACTOR_NORMAL;

boolean G_GetSlowMotion(void)
{
  return slow_motion;
}

void G_SetSlowMotion(const boolean value)
{
  slow_motion = value;
}

void G_ResetSlowMotion(void)
{
  const boolean reset_scale = slow_motion_factor != SLOWMO_FACTOR_NORMAL;

  slow_motion = false;
  slow_motion_factor = SLOWMO_FACTOR_NORMAL;

  if (reset_scale) { G_SetTimeScale(); }
}

int G_GetSlowMotionFactor(void)
{
  return slow_motion_factor;
}

// Improved weapon toggles ---------------------------------------------------

boolean G_ToggleFistChainsaw(const player_t *const player, boolean vanilla)
{
  const boolean berserk = player->powers[pw_strength];
  const boolean saw_preferred = vanilla || P_WeaponPreferred(wp_chainsaw, wp_fist) || !berserk;
  const weapontype_t pendingweapon = player->pendingweapon,
                       readyweapon = player->readyweapon;

  if (CASUALPLAY(weapswitch_interruption)
      && pendingweapon != wp_nochange
      && !(   pendingweapon == wp_fist
           || pendingweapon == wp_chainsaw))
  {
    if (readyweapon == wp_fist || readyweapon == wp_chainsaw)
    {
      return readyweapon == wp_chainsaw;
    }
    else
    {
      return saw_preferred;
    }
  }
  else if (CASUALPLAY(improved_weapon_toggles))
  {
    if (readyweapon == wp_fist
        || pendingweapon == wp_fist
        || pendingweapon == wp_chainsaw)
    {
      return pendingweapon != wp_chainsaw || (vanilla && !berserk);
    }
    else if (readyweapon == wp_chainsaw)
    {
      return pendingweapon == wp_fist || (vanilla && !berserk);
    }
    else
    {
      return saw_preferred;
    }
  }
  else
  {
    if (vanilla)
    {
      return readyweapon != wp_chainsaw || !berserk;
    }
    else
    {
      return readyweapon != wp_chainsaw
             && (readyweapon == wp_fist
                 || saw_preferred);
    }
  }
}

boolean G_ToggleShotgunSSG(const player_t *const player, boolean vanilla)
{
  const boolean ssg_preferred = vanilla || P_WeaponPreferred(wp_supershotgun, wp_shotgun);
  const weapontype_t pendingweapon = player->pendingweapon,
                       readyweapon = player->readyweapon;

  if (CASUALPLAY(weapswitch_interruption)
      && pendingweapon != wp_nochange
      && !(   pendingweapon == wp_shotgun
           || pendingweapon == wp_supershotgun))
  {
    if (readyweapon == wp_shotgun || readyweapon == wp_supershotgun)
    {
      return readyweapon == wp_supershotgun;
    }
    else
    {
      return ssg_preferred;
    }
  }
  else if (CASUALPLAY(improved_weapon_toggles))
  {
    if (readyweapon == wp_shotgun
        || pendingweapon == wp_shotgun
        || pendingweapon == wp_supershotgun)
    {
      return pendingweapon != wp_supershotgun;
    }
    else if (readyweapon == wp_supershotgun)
    {
      return pendingweapon == wp_shotgun;
    }
    else
    {
      return ssg_preferred;
    }
  }
  else
  {
    if (vanilla)
    {
      return readyweapon != wp_supershotgun;
    }
    else
    {
      return readyweapon == wp_shotgun
             || (readyweapon != wp_supershotgun
                 && ssg_preferred);
    }
  }
}

// Custom Skill --------------------------------------------------------------

int custom_skill_things;
boolean custom_skill_coopspawns;
boolean custom_skill_nomonsters;
boolean custom_skill_doubleammo;
boolean custom_skill_halfdamage;
boolean custom_skill_slowbrain;
boolean custom_skill_fast;
boolean custom_skill_respawn;
boolean custom_skill_aggressive;
boolean custom_skill_x2monsters;

// Actual custom-skill settings, set either by menu or savegames
static struct {
  int     things;
  boolean coopspawns;
  boolean nomonsters;
  boolean doubleammo;
  boolean halfdamage;
  boolean slowbrain;
  boolean fast;
  boolean respawn;
  boolean aggressive;
  boolean x2monsters;
} customskill;

int     thingspawns;
boolean realnomonsters;
boolean doubleammo;
boolean halfdamage;
boolean slowbrain;
boolean fastmonsters;
boolean aggressive;
boolean x2monsters;

static struct {
  int          mohealth;
  int          health;
  int          armorpoints;
  int          armortype;
  boolean      backpack;
  weapontype_t readyweapon;
  weapontype_t lastweapon;
  boolean      weaponowned[NUMWEAPONS];
  int          ammo[NUMAMMO];
  int          maxammo[NUMAMMO];
} initial_loadout;

// [Nugget]
void G_SetSkillParms(const skill_t skill)
{
  if (skill == sk_custom)
  {
    thingspawns     = customskill.things;
    coop_spawns     = customskill.coopspawns;
    realnomonsters  = customskill.nomonsters;
    doubleammo      = customskill.doubleammo;
    halfdamage      = customskill.halfdamage;
    slowbrain       = customskill.slowbrain;
    fastmonsters    = customskill.fast;
    respawnmonsters = customskill.respawn;
    aggressive      = customskill.aggressive;
    x2monsters      = customskill.x2monsters;
  }
  else {
    thingspawns = (skill == sk_baby || skill == sk_easy)      ? THINGSPAWNS_EASY :
                  (skill == sk_hard || skill == sk_nightmare) ? THINGSPAWNS_HARD : THINGSPAWNS_NORMAL;

    coop_spawns     = coopspawnsparm;
    realnomonsters  = nomonsters;
    doubleammo      = skill == sk_baby || skill == sk_nightmare;
    halfdamage      = skill == sk_baby;
    slowbrain       = skill <= sk_easy;
    fastmonsters    = fastparm || skill == sk_nightmare;
    respawnmonsters = skill == sk_nightmare || respawnparm;
    aggressive      = skill == sk_nightmare;

    x2monsters = false;
  }

  G_SetFastParms(fastmonsters);
}

void G_SetUserCustomSkill(void)
{
  customskill.things     = custom_skill_things;
  customskill.coopspawns = custom_skill_coopspawns;
  customskill.nomonsters = custom_skill_nomonsters;
  customskill.doubleammo = custom_skill_doubleammo;
  customskill.halfdamage = custom_skill_halfdamage;
  customskill.slowbrain  = custom_skill_slowbrain;
  customskill.fast       = custom_skill_fast;
  customskill.respawn    = custom_skill_respawn;
  customskill.aggressive = custom_skill_aggressive;
  customskill.x2monsters = custom_skill_x2monsters;
}

static void G_UpdateInitialLoadout(void)
{
  player_t *const player = &players[consoleplayer];

  initial_loadout.mohealth    = player->mo->health;
  initial_loadout.health      = player->health;
  initial_loadout.armorpoints = player->armorpoints;
  initial_loadout.armortype   = player->armortype;
  initial_loadout.backpack    = player->backpack;
  initial_loadout.readyweapon = player->readyweapon;
  initial_loadout.lastweapon  = player->lastweapon;

  memcpy(initial_loadout.weaponowned, player->weaponowned, sizeof(player->weaponowned));
  memcpy(initial_loadout.ammo,        player->ammo,        sizeof(player->ammo));
  memcpy(initial_loadout.maxammo,     player->maxammo,     sizeof(player->maxammo));
}

// [Nugget] =================================================================/

#define SAVEGAMESIZE  0x20000
#define SAVESTRINGSIZE  24

static size_t   savegamesize = SAVEGAMESIZE; // killough
static char     *demoname = NULL;
// the original name of the demo, without "-00000" and file extension
static char *demoname_orig = NULL;
static boolean  netdemo;
static byte     *demobuffer;   // made some static -- killough
static size_t   maxdemosize;
static byte     *demo_p;
static byte     consistancy[MAXPLAYERS][BACKUPTICS];

static int G_GameOptionSize(void);

gameaction_t    gameaction;
gamestate_t     gamestate;
skill_t         gameskill;
boolean         respawnmonsters;
int             gameepisode;
int             gamemap;
mapentry_t*     gamemapinfo;

// If non-zero, exit the level after this number of minutes.
int             timelimit;

int             paused;
boolean         sendpause;     // send a pause event next tic
boolean         sendsave;      // send a save event next tic
boolean         sendreload;    // send a reload level event next tic
boolean         sendjoin;      // send a join demo event next tic
boolean         usergame;      // ok to save / end game
boolean         timingdemo;    // if true, exit with report on completion
boolean         fastdemo;      // if true, run at full speed -- killough
boolean         nodrawers;     // for comparative timing purposes
boolean         noblit;        // for comparative timing purposes
int             starttime;     // for comparative timing purposes
boolean         viewactive;
int             deathmatch;    // only if started as net death
boolean         netgame;       // only true if packets are broadcast
boolean         solonet;
boolean         playeringame[MAXPLAYERS];
player_t        players[MAXPLAYERS];
int             consoleplayer; // player taking events and displaying
int             displayplayer; // view being displayed
int             gametic;
int             levelstarttic; // gametic at level start
int             basetic;       // killough 9/29/98: for demo sync
int             totalkills, totalitems, totalsecret;    // for intermission
int             max_kill_requirement; // DSDA UV Max category requirements
milestone_t     complete_milestones; // [Nugget]
int             totalleveltimes; // [FG] total time for all completed levels
boolean         demorecording;
boolean         longtics;             // cph's doom 1.91 longtics hack
boolean         fake_longtics;        // Fake longtics when using shorttics.
boolean         shorttics;            // Config key for low resolution turning.
boolean         lowres_turn;          // low resolution turning for longtics
boolean         demoplayback;
boolean         singledemo;           // quit after playing a demo from cmdline
boolean         precache = true;      // if true, load all graphics at start
wbstartstruct_t wminfo;               // parms for world map / intermission
boolean         haswolflevels = false;// jff 4/18/98 wolf levels present
byte            *savebuffer;
boolean         autorun = false;      // always running?          // phares
boolean         autostrafe50;
boolean         novert = false;
boolean         mouselook = false;
boolean         padlook = false;
// killough 4/13/98: Make clock rate adjustable by scale factor
int             realtic_clock_rate = 100;
boolean         doom_weapon_toggles; // [Nugget] Global

complevel_t     force_complevel, default_complevel;

static boolean  pistolstart, default_pistolstart;

boolean         strictmode, default_strictmode;
boolean         force_strictmode;
boolean         critical;

// [crispy] store last cmd to track joins
static ticcmd_t* last_cmd = NULL;

//
// controls (have defaults)
//
int     key_escape = KEY_ESCAPE;                           // phares 4/13/98
int     key_help = KEY_F1;                                 // phares 4/13/98

// [FG] double click acts as "use"
static boolean dclick_use;

#define MAXPLMOVE   (forwardmove[1])
#define TURBOTHRESHOLD  0x32
#define SLOWTURNTICS  6
#define QUICKREVERSE 32768 // 180 degree reverse                    // phares

fixed_t forwardmove[2] = {0x19, 0x32};
fixed_t default_sidemove[2] = {0x18, 0x28};
fixed_t *sidemove = default_sidemove;
const fixed_t angleturn[3] = {640, 1280, 320};  // + slow turn

boolean gamekeydown[NUMKEYS];
int     turnheld;       // for accelerative turning

boolean mousebuttons[NUM_MOUSE_BUTTONS];

// mouse values are used once
static int mousex;
static int mousey;
boolean dclick;

static ticcmd_t basecmd;

boolean joybuttons[NUM_GAMEPAD_BUTTONS];

int   savegameslot = -1;
char  savedescription[32];

static boolean save_autosave;
static boolean autosave;

//jff 3/24/98 declare startskill external, define defaultskill here
int default_skill;               //note 1-based

// killough 2/8/98: make corpse queue variable in size
int    bodyqueslot, bodyquesize, default_bodyquesize; // killough 2/8/98, 10/98

static weapontype_t LastWeapon(void)
{
    const weapontype_t weapon = players[consoleplayer].lastweapon;

    if (weapon < wp_fist || weapon >= NUMWEAPONS
        || !G_WeaponSelectable(weapon))
    {
        return wp_nochange;
    }

    if (demo_compatibility && weapon == wp_supershotgun)
    {
        return wp_shotgun;
    }

    return weapon;
}

static weapontype_t WeaponSSG(void)
{
    const player_t *player = &players[consoleplayer];

    if (!ALLOW_SSG || !player->weaponowned[wp_supershotgun])
    {
        return wp_nochange;
    }

    if (!demo_compatibility)
    {
        return wp_supershotgun;
    }

    if (player->pendingweapon != wp_supershotgun
        && player->readyweapon != wp_supershotgun)
    {
        return wp_shotgun;
    }

    return wp_nochange;
}

// [FG] toggle demo warp mode
void G_EnableWarp(boolean warp)
{
  static boolean nodrawers_old;
  static boolean nomusicparm_old, nosfxparm_old;

  if (warp)
  {
    nodrawers_old = nodrawers;
    nomusicparm_old = nomusicparm;
    nosfxparm_old = nosfxparm;

    I_SetFastdemoTimer(true);
    nodrawers = true;
    nomusicparm = true;
    nosfxparm = true;
  }
  else
  {
    I_SetFastdemoTimer(false);
    nodrawers = nodrawers_old;
    nomusicparm = nomusicparm_old;
    nosfxparm = nosfxparm_old;
  }
}

static int playback_levelstarttic;
int playback_skiptics = 0;

static void G_DemoSkipTics(void)
{
  static boolean warp = false;

  if (!playback_skiptics || !playback_totaltics)
    return;

  if (playback_warp >= 0)
    warp = true;

  if (playback_warp == -1)
  {
    int curtic;

    if (playback_skiptics < 0)
    {
      playback_skiptics = playback_totaltics + playback_skiptics;
      warp = false; // ignore -warp
    }

    curtic = (warp ? playback_tic - playback_levelstarttic : playback_tic);

    if (playback_skiptics < curtic)
    {
      playback_skiptics = 0;
      G_EnableWarp(false);
      S_RestartMusic();
    }
  }
}

void G_SetTimeScale(void)
{
    if (strictmode || D_CheckNetConnect())
    {
        I_SetTimeScale(100);
        return;
    }

    int time_scale = realtic_clock_rate;

    //!
    // @arg <n>
    // @category game
    //
    // Increase or decrease game speed, percentage of normal.
    //

    int p = M_CheckParmWithArgs("-speed", 1);

    if (p)
    {
        time_scale = M_ParmArgToInt(p);
        if (time_scale < 10 || time_scale > 1000)
        {
            I_Error(
                "Invalid parameter '%d' for -speed, valid values are 10-1000.",
                time_scale);
        }
    }

    // [Nugget] Slow Motion
    if (casual_play && !menuactive && slow_motion_factor != SLOWMO_FACTOR_NORMAL)
    { time_scale = time_scale * slow_motion_factor / SLOWMO_FACTOR_NORMAL; }

    I_SetTimeScale(time_scale);

    setrefreshneeded = true;
}

static void ClearLocalView(void)
{
  memset(&localview, 0, sizeof(localview));
}

static void UpdateLocalView_FakeLongTics(void)
{
  localview.angle -= localview.angleoffset << FRACBITS;
  localview.rawangle -= localview.angleoffset;
  localview.angleoffset = 0;
  localview.pitch = 0;
  localview.rawpitch = 0.0;
  localview.oldlerpangle = localview.lerpangle;
  localview.lerpangle = localview.angle;
}

static void (*UpdateLocalView)(void);

void G_UpdateLocalViewFunction(void)
{
  if (lowres_turn && fake_longtics && (!netgame || solonet))
  {
    UpdateLocalView = UpdateLocalView_FakeLongTics;
  }
  else
  {
    UpdateLocalView = ClearLocalView;
  }
}

//
// ApplyQuickstartCache
// When recording a demo and the map is reloaded, cached input from a circular
// buffer can be applied prior to the screen wipe. Adapted from DSDA-Doom.
//

static int quickstart_cache_tics;
static boolean quickstart_queued;
static float axis_turn_tic;
static float gyro_turn_tic;
static int mousex_tic;

static void ClearQuickstartTic(void)
{
  axis_turn_tic = 0.0f;
  gyro_turn_tic = 0.0f;
  mousex_tic = 0;
}

static void ApplyQuickstartCache(ticcmd_t *cmd, boolean strafe)
{
  static float axis_turn_cache[TICRATE];
  static float gyro_turn_cache[TICRATE];
  static int mousex_cache[TICRATE];
  static short angleturn_cache[TICRATE];
  static int index;

  if (quickstart_cache_tics < 1)
  {
    return;
  }

  if (quickstart_queued)
  {
    axes[AXIS_TURN] = 0.0f;
    gyro_axes[GYRO_TURN] = 0.0f;
    mousex = 0;

    if (strafe)
    {
      for (int i = 0; i < quickstart_cache_tics; i++)
      {
        axes[AXIS_TURN] += axis_turn_cache[i];
        gyro_axes[GYRO_TURN] += gyro_turn_cache[i];
        mousex += mousex_cache[i];
      }

      cmd->angleturn = 0;
      localview.rawangle = 0.0;
    }
    else
    {
      short result = 0;

      for (int i = 0; i < quickstart_cache_tics; i++)
      {
        result += angleturn_cache[i];
      }

      cmd->angleturn = G_CarryAngleTic(result);
      localview.rawangle = cmd->angleturn;
    }

    memset(axis_turn_cache, 0, sizeof(axis_turn_cache));
    memset(gyro_turn_cache, 0, sizeof(gyro_turn_cache));
    memset(mousex_cache, 0, sizeof(mousex_cache));
    memset(angleturn_cache, 0, sizeof(angleturn_cache));
    index = 0;

    quickstart_queued = false;
  }
  else
  {
    axis_turn_cache[index] = axis_turn_tic;
    gyro_turn_cache[index] = gyro_turn_tic;
    mousex_cache[index] = mousex_tic;
    angleturn_cache[index] = cmd->angleturn;
    index = (index + 1) % quickstart_cache_tics;
  }
}

void G_PrepMouseTiccmd(void)
{
  UpdateMouseModifiers(); // [Nugget]

  if (mousex && !M_InputGameActive(input_strafe))
  {
    localview.rawangle -= G_CalcMouseAngle(mousex) / mouse_h_modifier;
    basecmd.angleturn = G_CarryAngle(localview.rawangle);
    mousex = 0;
  }

  if (mousey && mouselook)
  {
    localview.rawpitch += G_CalcMousePitch(mousey) / mouse_v_modifier;
    basecmd.pitch = G_CarryPitch(localview.rawpitch);
    mousey = 0;
  }
}

void G_PrepGamepadTiccmd(void)
{
  if (I_UseGamepad())
  {
    UpdateMouseModifiers(); // [Nugget]

    const boolean strafe = M_InputGameActive(input_strafe);

    I_CalcGamepadAxes(strafe);
    axis_turn_tic = axes[AXIS_TURN];

    if (axes[AXIS_TURN] && !strafe)
    {
      localview.rawangle -= G_CalcGamepadAngle() / mouse_h_modifier;
      basecmd.angleturn = G_CarryAngle(localview.rawangle);
      axes[AXIS_TURN] = 0.0f;
    }

    if (axes[AXIS_LOOK] && padlook)
    {
      localview.rawpitch -= G_CalcGamepadPitch() / mouse_v_modifier;
      basecmd.pitch = G_CarryPitch(localview.rawpitch);
      axes[AXIS_LOOK] = 0.0f;
    }
  }
}

void G_PrepGyroTiccmd(void)
{
  if (I_UseGamepad())
  {
    UpdateMouseModifiers(); // [Nugget]

    I_CalcGyroAxes(M_InputGameActive(input_strafe));
    gyro_turn_tic = gyro_axes[GYRO_TURN];

    if (gyro_axes[GYRO_TURN])
    {
      localview.rawangle += gyro_axes[GYRO_TURN] / mouse_h_modifier;
      basecmd.angleturn = G_CarryAngle(localview.rawangle);
      gyro_axes[GYRO_TURN] = 0.0f;
    }

    if (gyro_axes[GYRO_LOOK])
    {
      localview.rawpitch += gyro_axes[GYRO_LOOK] / mouse_v_modifier;
      basecmd.pitch = G_CarryPitch(localview.rawpitch);
      gyro_axes[GYRO_LOOK] = 0.0f;
    }
  }
}

static void AdjustWeaponSelection(int *newweapon)
{
    // killough 3/22/98: For network and demo consistency with the
    // new weapons preferences, we must do the weapons switches here
    // instead of in p_user.c. But for old demos we must do it in
    // p_user.c according to the old rules. Therefore demo_compatibility
    // determines where the weapons switch is made.

    // killough 2/8/98:
    // Allow user to switch to fist even if they have chainsaw.
    // Switch to fist or chainsaw based on preferences.
    // Switch to shotgun or SSG based on preferences.
    //
    // killough 10/98: make SG/SSG and Fist/Chainsaw
    // weapon toggles optional

    const player_t *player = &players[consoleplayer];

    // only select chainsaw from '1' if it's owned, it's
    // not already in use, and the player prefers it or
    // the fist is already in use, or the player does not
    // have the berserker strength.

    if (*newweapon == wp_fist
        && player->weaponowned[wp_chainsaw]
        && G_ToggleFistChainsaw(player, false)) // [Nugget] Improved weapon toggles
    {
        *newweapon = wp_chainsaw;
    }

    // Select SSG from '3' only if it's owned and the player
    // does not have a shotgun, or if the shotgun is already
    // in use, or if the SSG is not already in use and the
    // player prefers it.

    if (*newweapon == wp_shotgun && ALLOW_SSG
        && player->weaponowned[wp_supershotgun]
        && (!player->weaponowned[wp_shotgun]
            || G_ToggleShotgunSSG(player, false))) // [Nugget] Improved weapon toggles
    {
        *newweapon = wp_supershotgun;
    }

    // killough 2/8/98, 3/22/98 -- end of weapon selection changes
}

static boolean FilterDeathUseAction(void)
{
    if (players[consoleplayer].playerstate == PST_DEAD && gamestate == GS_LEVEL)
    {
        switch (death_use_action)
        {
            case DEATH_USE_ACTION_NOTHING:
                return true;

            case DEATH_USE_ACTION_LAST_SAVE:
                if (!demoplayback && !demorecording && !netgame
                    && death_use_state == DEATH_USE_STATE_INACTIVE)
                {
                    death_use_state = DEATH_USE_STATE_PENDING;
                }
                return true;

            default:
                break;
        }
    }

    return false;
}

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//

void G_BuildTiccmd(ticcmd_t* cmd)
{
  const boolean strafe = M_InputGameActive(input_strafe);
  const boolean turnleft = M_InputGameActive(input_turnleft);
  const boolean turnright = M_InputGameActive(input_turnright);
  // [FG] speed key inverts autorun
  const int speed = autorun ^ M_InputGameActive(input_speed); // phares
  int angle = 0;
  int forward = 0;
  int side = 0;
  int newweapon;                                          // phares

  static boolean done_autoswitch = false;

  G_DemoSkipTics();

  if (!uncapped || !raw_input)
  {
    G_PrepMouseTiccmd();
    G_PrepGamepadTiccmd();
    G_PrepGyroTiccmd();
  }

  memcpy(cmd, &basecmd, sizeof(*cmd));
  memset(&basecmd, 0, sizeof(basecmd));

  ApplyQuickstartCache(cmd, strafe);

  cmd->consistancy = consistancy[consoleplayer][maketic%BACKUPTICS];

  // Composite input

  // turn 180 degrees in one keystroke?                           // phares
  if (STRICTMODE(M_InputGameActive(input_reverse)))
  {
    angle += QUICKREVERSE;
    M_InputGameDeactivate(input_reverse);
  }

  // let movement keys cancel each other out
  if (turnleft || turnright)
  {
    turnheld += ticdup;

    if (strafe)
    {
      if (!cmd->angleturn)
      {
        if (turnright)
          side += sidemove[speed];
        if (turnleft)
          side -= sidemove[speed];
      }
    }
    else
    {
      // use two stage accelerative turning on the keyboard and joystick
      const int tspeed = ((turnheld < SLOWTURNTICS) ? 2 : speed);

      if (turnright)
        angle -= angleturn[tspeed];
      if (turnleft)
        angle += angleturn[tspeed];
    }
  }
  else
  {
    turnheld = 0;
  }

  if (M_InputGameActive(input_forward))
    forward += forwardmove[speed];
  if (M_InputGameActive(input_backward))
    forward -= forwardmove[speed];
  if (M_InputGameActive(input_straferight))
    side += sidemove[speed];
  if (M_InputGameActive(input_strafeleft))
    side -= sidemove[speed];

  // Gamepad

  if (I_UseGamepad())
  {
    if (axes[AXIS_TURN] && strafe && !cmd->angleturn)
    {
      side += G_CalcGamepadSideTurn(speed);
    }

    if (axes[AXIS_STRAFE])
    {
      side += G_CalcGamepadSideStrafe(speed);
    }

    if (axes[AXIS_FORWARD])
    {
      forward -= G_CalcGamepadForward(speed);
    }
  }

  // Mouse

  if (mousex && strafe && !cmd->angleturn)
  {
    const double mouseside = G_CalcMouseSide(mousex);
    side += G_CarrySide(mouseside);
  }

  if (mousey && !mouselook && !novert)
  {
    const double mousevert = G_CalcMouseVert(mousey);
    forward += G_CarryVert(mousevert);
  }

  // Update/reset

  // [Nugget] Flip levels
  if (STRICTMODE(flip_levels)) {
    angle = -angle;
    side  = -side;
  }

  if (angle)
  {
    const short old_angleturn = cmd->angleturn;
    cmd->angleturn = G_CarryAngleTic(localview.rawangle + angle);
    cmd->ticangleturn = cmd->angleturn - old_angleturn;
  }

  if (forward > MAXPLMOVE)
    forward = MAXPLMOVE;
  else if (forward < -MAXPLMOVE)
    forward = -MAXPLMOVE;
  if (side > MAXPLMOVE)
    side = MAXPLMOVE;
  else if (side < -MAXPLMOVE)
    side = -MAXPLMOVE;

  cmd->forwardmove = forward;
  cmd->sidemove = side;

  ClearQuickstartTic();
  I_ResetGamepadAxes();
  I_ResetGyroAxes();
  mousex = mousey = 0;
  UpdateLocalView();
  G_UpdateCarry();

  // Buttons

  cmd->chatchar = ST_DequeueChatChar();

  if (M_InputGameActive(input_fire))
    cmd->buttons |= BT_ATTACK;

  if (M_InputGameActive(input_use)) // [FG] mouse button for "use"
    {
      if (!FilterDeathUseAction())
        cmd->buttons |= BT_USE;
      // clear double clicks if hit use button
      dclick = false;
    }

  // Toggle between the top 2 favorite weapons.                   // phares
  // If not currently aiming one of these, switch to              // phares
  // the favorite. Only switch if you possess the weapon.         // phares

  // killough 3/22/98:
  //
  // Perform automatic weapons switch here rather than in p_pspr.c,
  // except in demo_compatibility mode.
  //
  // killough 3/26/98, 4/2/98: fix autoswitch when no weapons are left

  if (!players[consoleplayer].attackdown)
  {
    done_autoswitch = false;
  }

  G_NextWeaponResendCmd();
  boolean nextweapon_cmd = false;

  // [Nugget] Tweaked weapon switching /--------------------------------------

  boolean ilw_active = false;

  if (CASUALPLAY(improved_weapon_toggles || weapswitch_interruption))
  {
    static boolean ilw_down = false;

    if (!M_InputGameActive(input_lastweapon))
    {
      ilw_down = false;
    }
    else if (!ilw_down)
    {
      ilw_active = ilw_down = true;
    }
  }
  else { ilw_active = M_InputGameActive(input_lastweapon); }

  // [Nugget] ---------------------------------------------------------------/

  if ((!demo_compatibility && players[consoleplayer].attackdown &&
       !P_CheckAmmo(&players[consoleplayer]) &&
       ((boom_weapon_state_injection && !done_autoswitch) ||
       (cmd->buttons & BT_ATTACK && players[consoleplayer].pendingweapon == wp_nochange))) ||
       M_InputGameActive(input_weapontoggle))
  {
    done_autoswitch = true;
    boom_weapon_state_injection = false;
    newweapon = P_SwitchWeapon(&players[consoleplayer]);           // phares
  }
  else if (WS_SlotSelected())
  {
    newweapon = WS_SlotWeapon();
  }
  else if (ilw_active)
  {
    newweapon = LastWeapon();
  }
  else if (G_NextWeaponDeactivate())
  {
    newweapon = demo_compatibility
                    ? nextweapon_translate[players[consoleplayer].nextweapon]
                    : players[consoleplayer].nextweapon;
    nextweapon_cmd = true;
  }
  else
    {                                 // phares 02/26/98: Added gamemode checks
      // [Nugget] Tweaked switching

      boolean iw_active[9] = {false};

      if (CASUALPLAY(improved_weapon_toggles || weapswitch_interruption))
      {
        static boolean iw_down[9] = {false};

        for (int i = 0;  i < 9;  i++)
        {
          if (!M_InputGameActive(input_weapon1 + i))
          {
            iw_down[i] = false;
          }
          else if (!iw_down[i])
          {
            iw_active[i] = iw_down[i] = true;
          }
        }
      }
      else {
        for (int i = 0;  i < 9;  i++)
        { iw_active[i] = M_InputGameActive(input_weapon1 + i); }
      }

      newweapon =
        iw_active[0] ? wp_fist :    // killough 5/2/98: reformatted
        iw_active[1] ? wp_pistol :
        iw_active[2] ? wp_shotgun :
        iw_active[3] ? wp_chaingun :
        iw_active[4] ? wp_missile :
        iw_active[5] && gamemode != shareware ? wp_plasma :
        iw_active[6] && gamemode != shareware ? wp_bfg :
        iw_active[7] ? wp_chainsaw :
        iw_active[8] ? WeaponSSG() :
        wp_nochange;

      if (!demo_compatibility && doom_weapon_toggles)
        {
          AdjustWeaponSelection(&newweapon);
        }
    }

  if (newweapon != wp_nochange)
    {
      cmd->buttons |= BT_CHANGE;
      cmd->buttons |= newweapon<<BT_WEAPONSHIFT;
      if (!nextweapon_cmd)
        G_NextWeaponReset();
    }

    WS_UpdateStateTic();

  // [FG] double click acts as "use"
  if (dclick)
  {
    dclick = false;
    if (!FilterDeathUseAction())
      cmd->buttons |= BT_USE;
  }

  // special buttons
  if (sendpause && gameaction != ga_newgame)
    {
      sendpause = false;
      cmd->buttons = BT_SPECIAL | (BTS_PAUSE & BT_SPECIALMASK);
    }

  // killough 10/6/98: suppress savegames in demos
  if (sendsave && !demoplayback)
    {
      sendsave = false;
      cmd->buttons = BT_SPECIAL | (BTS_SAVEGAME & BT_SPECIALMASK) | (savegameslot<<BTS_SAVESHIFT);
    }

  if (sendreload)
  {
    sendreload = false;
    cmd->buttons = BT_SPECIAL | (BTS_RELOAD & BT_SPECIALMASK);
  }

  if (sendjoin)
  {
    sendjoin = false;
    cmd->buttons |= BT_JOIN;
  }
}

void G_ClearInput(void)
{
  ClearQuickstartTic();
  I_ResetGamepadState();
  I_FlushGamepadSensorEvents();
  mousex = mousey = 0;
  ClearLocalView();
  G_ClearCarry();
  memset(&basecmd, 0, sizeof(basecmd));
  I_ResetRelativeMouseState();
  I_ResetAllRumbleChannels();
  WS_Reset();
}

//
// G_DoLoadLevel
//

static void G_DoLoadLevel(void)
{
  int i;

  // [Nugget]
  int lastaction = gameaction;
  static int lastepisode = -1, lastmap = -1;

  // Set the sky map.
  // First thing, we have a dummy sky texture name,
  //  a flat. The data is in the WAD only because
  //  we look for an actual index, instead of simply
  //  setting one.

  skyflatnum = R_FlatNumForName ( SKYFLATNAME );

  if (gamemapinfo && gamemapinfo->skytexture[0])
  {
    skytexture = R_TextureNumForName(gamemapinfo->skytexture);
  }
  else
  // DOOM determines the sky texture to be used
  // depending on the current episode, and the game version.
  if (gamemode == commercial)
    // || gamemode == pack_tnt   //jff 3/27/98 sorry guys pack_tnt,pack_plut
    // || gamemode == pack_plut) //aren't gamemodes, this was matching retail
    {
      skytexture = R_TextureNumForName ("SKY3");
      if (gamemap < 12)
        skytexture = R_TextureNumForName ("SKY1");
      else
        if (gamemap < 21)
          skytexture = R_TextureNumForName ("SKY2");
    }
  else //jff 3/27/98 and lets not forget about DOOM and Ultimate DOOM huh?
    switch (gameepisode)
      {
      default:
      case 1:
        skytexture = R_TextureNumForName ("SKY1");
        break;
      case 2:
	// killough 10/98: beta version had different sky orderings
        skytexture = R_TextureNumForName (beta_emulation ? "SKY1" : "SKY2");
        break;
      case 3:
        skytexture = R_TextureNumForName ("SKY3");
        break;
      case 4: // Special Edition sky
        skytexture = R_TextureNumForName ("SKY4");
        break;
      }//jff 3/27/98 end sky setting fix

  R_InitSkyMap(); // [FG] stretch short skies

  levelstarttic = gametic;        // for time calculation

  playback_levelstarttic = playback_tic;

  if (!demo_compatibility && demo_version < DV_MBF)   // killough 9/29/98
    basetic = gametic;

  if (wipegamestate == GS_LEVEL
      && lastaction != ga_rewind) // [Nugget] Rewind
    wipegamestate = -1;             // force a wipe

  gamestate = GS_LEVEL;

  for (i=0 ; i<MAXPLAYERS ; i++)
    {
      if (playeringame[i] && players[i].playerstate == PST_DEAD)
        players[i].playerstate = PST_REBORN;
      memset (players[i].frags,0,sizeof(players[i].frags));
    }

  // initialize the msecnode_t freelist.                     phares 3/25/98
  // any nodes in the freelist are gone by now, cleared
  // by Z_FreeTags() when the previous level ended or player
  // died.

  headsecnode = NULL;

  critical = (gameaction == ga_playdemo || demorecording || demoplayback || D_CheckNetConnect());

  P_UpdateCheckSight();

  // [Nugget] Custom Skill
  if (ignore_pistolstart)
  {
    ignore_pistolstart = false;
  }
  else
  // [crispy] pistol start
  if (CRITICAL(pistolstart) && lastaction != ga_rewind) // [Nugget] Rewind
  {
    G_PlayerReborn(0);
  }

  // [Nugget] Rewind: skip level setup if rewinding within the same map
  if (lastaction == ga_rewind
      && lastepisode == gameepisode && lastmap == gamemap)
  {
    S_Start(); // Stop sounds
  }
  else
    P_SetupLevel (gameepisode, gamemap, 0, gameskill);

  MN_UpdateFreeLook(!mouselook && !padlook);
  HU_UpdateTurnFormat();

  // [Woof!] Do not reset chosen player view across levels in multiplayer
  // demo playback. However, it must be reset when starting a new game.
  if (usergame)
  {
    displayplayer = consoleplayer;    // view the guy you are playing
  }
  gameaction = ga_nothing;

  // Set the initial listener parameters using the player's initial state.
  S_InitListener(players[displayplayer].mo);

  death_use_state = DEATH_USE_STATE_INACTIVE;

  // clear cmd building stuff
  // [Nugget] Rewind: unless we just rewound
  if (lastaction != ga_rewind)
  {
    memset (gamekeydown, 0, sizeof(gamekeydown));
    G_ClearInput();
    sendpause = sendsave = paused = false;
    // [FG] array size!
    memset (mousebuttons, 0, sizeof(mousebuttons));
    memset (joybuttons, 0, sizeof(joybuttons));
  }

  //jff 4/26/98 wake up the status bar in case were coming out of a DM demo
  // killough 5/13/98: in case netdemo has consoleplayer other than green
  ST_Start();

  // killough: make -timedemo work on multilevel demos
  // Move to end of function to minimize noise -- killough 2/22/98:

  if (timingdemo)
    {
      static int first=1;
      if (first)
        {
          starttime = I_GetTime_RealTime();
          first=0;
        }
    }

  // [Nugget] ================================================================

  // Periodic auto save
  G_SetAutoSaveCountdown(autosave_interval * TICRATE);

  // Rewind
  G_SetRewindCountdown(0);

  // Hide messages (but don't delete them outright)
  ST_HideMessages();

  // Minimap
  if (minimap_was_on)
  {
    AM_ChangeMode(AM_MINI);
    minimap_was_on = false;
  }

  // Slow Motion
  G_ResetSlowMotion();

  // Clear visual effects
  R_ClearFOVFX();
  R_ClearShake();

  // Alt. intermission background
  if (WI_AltInterpicOn())
  {
    R_SetViewSize(screenblocks);
    R_ExecuteSetViewSize();

    WI_DisableAltInterpic();
  }

  // Freecam -----------------------------------------------------------------

  R_UpdateFreecamMobj(NULL);

  if (lastepisode != gameepisode || lastmap != gamemap)
  { R_ResetFreecam(true); }

  // -------------------------------------------------------------------------

  lastepisode = gameepisode;
  lastmap     = gamemap;
}

static void G_ReloadLevel(void)
{
  if (demorecording || netgame)
  {
    gamemap = startmap;
    gameepisode = startepisode;
  }

  basetic = gametic;
  rngseed += gametic;

  if (demorecording)
  {
    ddt_cheating = 0;
    G_CheckDemoStatus();
    G_RecordDemo(demoname_orig);
  }

  G_InitNew(gameskill, gameepisode, gamemap);
  gameaction = ga_nothing;

  if (demorecording)
    G_BeginRecording();
}

// [FG] reload current level / go to next level
// adapted from prboom-plus/src/e6y.c:369-449
int G_GotoNextLevel(int *pEpi, int *pMap)
{
  byte doom_next[4][9] = {
    {12, 13, 19, 15, 16, 17, 18, 21, 14},
    {22, 23, 24, 25, 29, 27, 28, 31, 26},
    {32, 33, 34, 35, 36, 39, 38, 41, 37},
    {42, 49, 44, 45, 46, 47, 48, 11, 43}
  };
  byte doom2_next[32] = {
     2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
    12, 13, 14, 15, 31, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30,  1,
    32, 16
  };

  int epsd;
  int map = -1;

  if (gamemapinfo)
  {
    const char *next = NULL;

    if (gamemapinfo->nextsecret[0])
      next = gamemapinfo->nextsecret;
    else if (gamemapinfo->nextmap[0])
      next = gamemapinfo->nextmap;
    else if (gamemapinfo->flags & MapInfo_EndGame)
    {
      epsd = 1;
      map = 1;
    }

    if (next)
      G_ValidateMapName(next, &epsd, &map);
  }

  if (map == -1)
  {
    // secret level
    doom2_next[14] = (haswolflevels ? 31 : 16);

    // shareware doom has only episode 1
    doom_next[0][7] = (gamemode == shareware ? 11 : 21);

    doom_next[2][7] = (gamemode == registered ? 11 : 41);

    //doom2_next and doom_next are 0 based, unlike gameepisode and gamemap
    epsd = gameepisode - 1;
    map = gamemap - 1;

    if (gamemode == commercial)
    {
      epsd = 1;
      if (map >= 0 && map <= 31)
        map = doom2_next[map];
      else
        map = gamemap + 1;
    }
    else
    {
      if (epsd >= 0 && epsd <= 3 && map >= 0 && map <= 8)
      {
        int next = doom_next[epsd][map];
        epsd = next / 10;
        map = next % 10;
      }
      else
      {
        epsd = gameepisode;
        map = gamemap + 1;
      }
    }
  }

  // [FG] report next level without changing
  if (pEpi || pMap)
  {
    if (pEpi)
      *pEpi = epsd;
    if (pMap)
      *pMap = map;
  }
  else if ((gamestate == GS_LEVEL) &&
            !deathmatch && !netgame &&
            !demorecording && !demoplayback &&
            !menuactive)
  {
    char *name = MapName(epsd, map);

    if (W_CheckNumForName(name) == -1)
      displaymsg("Next level not found: %s", name);
    else
    {
      G_DeferedInitNew(gameskill, epsd, map);
      return true;
    }
  }

  return false;
}

static boolean G_StrictModeSkipEvent(event_t *ev)
{
  static boolean enable_mouse = false;
  static boolean enable_gamepad = false;
  static boolean first_event = true;

  if (!strictmode || !demorecording)
    return false;

  switch (ev->type)
  {
    case ev_mouseb_down:
    case ev_mouseb_up:
    case ev_mouse:
        if (first_event)
        {
          first_event = false;
          enable_mouse = true;
        }
        return !enable_mouse;

    case ev_joyb_down:
    case ev_joyb_up:
        if (first_event)
        {
          first_event = false;
          enable_gamepad = true;
        }
        return !enable_gamepad;

    case ev_joystick:
        if (first_event)
        {
          I_UpdateAxesData(ev);
          I_CalcGamepadAxes(M_InputGameActive(input_strafe));
          if (axes[AXIS_STRAFE] || axes[AXIS_FORWARD] || axes[AXIS_TURN] ||
              axes[AXIS_LOOK])
          {
            first_event = false;
            enable_gamepad = true;
          }
          I_ResetGamepadState();
        }
        return !enable_gamepad;

    case ev_gyro:
        if (first_event)
        {
          I_UpdateGyroData(ev);
          I_CalcGyroAxes(M_InputGameActive(input_strafe));
          if (gyro_axes[GYRO_TURN] || gyro_axes[GYRO_LOOK])
          {
            first_event = false;
            enable_gamepad = true;
          }
          I_ResetGamepadState();
        }
        return !enable_gamepad;

    default:
        break;
  }

  return false;
}

boolean G_MovementResponder(event_t *ev)
{
  if (G_StrictModeSkipEvent(ev))
  {
    return true;
  }

  switch (ev->type)
  {
    case ev_mouse:
      mousex_tic += ev->data1.i;
      mousex += ev->data1.i;
      mousey -= ev->data2.i;
      return true;

    case ev_joystick:
      I_UpdateAxesData(ev);
      return true;

    case ev_gyro:
      I_UpdateGyroData(ev);
      return true;

    default:
      break;
  }

  return false;
}

//
// G_Responder
// Get info needed to make ticcmd_ts for the players.
//

boolean G_Responder(event_t* ev)
{
  WS_UpdateState(ev);

  // killough 9/29/98: reformatted
  if (gamestate == GS_LEVEL
      && (ST_Responder(ev) || // status window ate it
          AM_Responder(ev) || // automap ate it
          WS_Responder(ev)))  // weapon slots ate it
  {
    return true;
  }

  if (M_ShortcutResponder(ev))
  {
    return true;
  }

  // allow spy mode changes even during the demo
  // killough 2/22/98: even during DM demo
  //
  // killough 11/98: don't autorepeat spy mode switch

  if (M_InputActivated(input_spy) && netgame && (demoplayback || !deathmatch) &&
      gamestate == GS_LEVEL
      && !chat_on) // [Nugget]
    {
	  do                                          // spy mode
	    if (++displayplayer >= MAXPLAYERS)
	      displayplayer = 0;
	  while (!playeringame[displayplayer] && displayplayer!=consoleplayer);

	  ST_Start();    // killough 3/7/98: switch status bar views too
	  S_UpdateSounds(players[displayplayer].mo);
	  // [crispy] re-init automap variables for correct player arrow angle
	  if (automapactive)
	    AM_initVariables();
      return true;
    }

  if (M_InputActivated(input_menu_reloadlevel) &&
      (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION) &&
      !menuactive
      && !chat_on) // [Nugget]
  {
    sendreload = true;
    return true;
  }

  // any other key pops up menu if in demos
  //
  // killough 8/2/98: enable automap in -timedemo demos
  //
  // killough 9/29/98: make any key pop up menu regardless of
  // which kind of demo, and allow other events during playback

  if (gameaction == ga_nothing && (demoplayback || gamestate == GS_DEMOSCREEN))
    {
      // killough 9/29/98: allow user to pause demos during playback
	if (M_InputActivated(input_pause))
	{
	  if (paused ^= 2)
	    S_PauseSound();
	  else
	    S_ResumeSound();
	  return true;
	}

      if (M_InputActivated(input_demo_join) && !PLAYBACK_SKIP && !fastdemo)
      {
        sendjoin = true;
        return true;
      }

      // killough 10/98:
      // Don't pop up menu, if paused in middle
      // of demo playback, or if automap active.
      // Don't suck up keys, which may be cheats

      // [Nugget] Freecam
      if (!R_FreecamOn())
        return gamestate == GS_DEMOSCREEN &&
	  !(paused & 2) && automapactive != AM_FULL &&
	  ((ev->type == ev_keydown) ||
	   (ev->type == ev_mouseb_down) ||
	   (ev->type == ev_joyb_down)) ?
	  (!menuactive ? S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn) // [Nugget]: [NS] Optional menu sounds.
	               : true),
	  MN_StartControlPanel(), true : false;
    }

  if (gamestate == GS_FINALE && F_Responder(ev))
  {
    return true;  // finale ate the event
  }

  if (dclick_use && ev->type == ev_mouseb_down &&
      (M_InputActivated(input_strafe) || M_InputActivated(input_forward)) &&
      ev->data2.i >= 2 && (ev->data2.i % 2) == 0)
  {
    dclick = true;
  }

  if (M_InputActivated(input_pause))
  {
    sendpause = true;
    return true;
  }

  if (G_MovementResponder(ev))
  {
    return true; // eat events
  }

  if (R_GetFreecamMode() != FREECAM_CAM) // [Nugget] Freecam
  {
    G_NextWeaponUpdate();
  }

  switch (ev->type)
    {
    case ev_keydown:
      if (ev->data1.i < NUMKEYS)
        gamekeydown[ev->data1.i] = true;
      return true;    // eat key down events

    case ev_keyup:
      if (ev->data1.i < NUMKEYS)
        gamekeydown[ev->data1.i] = false;
      return false;   // always let key up events filter down

    case ev_mouseb_down:
      if (ev->data1.i < NUM_MOUSE_BUTTONS)
        mousebuttons[ev->data1.i] = true;
      return true;

    case ev_mouseb_up:
      if (ev->data1.i < NUM_MOUSE_BUTTONS)
        mousebuttons[ev->data1.i] = false;
      return true;

    case ev_joyb_down:
      if (ev->data1.i < NUM_GAMEPAD_BUTTONS)
        joybuttons[ev->data1.i] = true;
      return true;

    case ev_joyb_up:
      if (ev->data1.i < NUM_GAMEPAD_BUTTONS)
        joybuttons[ev->data1.i] = false;
      return true;

    default:
      break;
    }
  return false;
}

int D_GetPlayersInNetGame(void);

static void CheckPlayersInNetGame(void)
{
  int i;
  int playerscount = 0;
  for (i = 0; i < MAXPLAYERS; ++i)
  {
    if (playeringame[i])
      ++playerscount;
  }

  if (playerscount != D_GetPlayersInNetGame())
    I_Error("Not enough players to continue the demo.");
}

//
// DEMO RECORDING
//

int playback_tic = 0, playback_totaltics = 0;

static const char *defdemoname;

#define DEMOMARKER    0x80

// Stay in the game, hand over controls to the player and continue recording the
// demo under a different name
static void G_JoinDemo(void)
{
  if (netgame)
    CheckPlayersInNetGame();

  if (!demoname_orig)
  {
    byte *actualbuffer = demobuffer;
    size_t actualsize = maxdemosize;

    // [crispy] find a new name for the continued demo
    G_RecordDemo(defdemoname);

    // [crispy] discard the newly allocated demo buffer
    Z_Free(demobuffer);
    demobuffer = actualbuffer;
    maxdemosize = actualsize;
  }

  // [crispy] continue recording
  demoplayback = false;

  // clear progress demo bar
  ST_Start();

  displaymsg("Demo recording: %s", demoname);
}

static void G_ReadDemoTiccmd(ticcmd_t *cmd)
{
  if (*demo_p == DEMOMARKER)
  {
    last_cmd = cmd; // [crispy] remember last cmd to track joins
    G_CheckDemoStatus();      // end of demo data stream
  }
  else
    {
      cmd->forwardmove = ((signed char)*demo_p++);
      cmd->sidemove = ((signed char)*demo_p++);
      if (!longtics)
      {
      cmd->angleturn = ((unsigned char)*demo_p++)<<8;
      }
      else
      {
        cmd->angleturn = *demo_p++;
        cmd->angleturn |= (*demo_p++) << 8;
      }
      cmd->buttons = (unsigned char)*demo_p++;

      // killough 3/26/98, 10/98: Ignore savegames in demos
      if (demoplayback &&
	  cmd->buttons & BT_SPECIAL &&
	  cmd->buttons & BT_SPECIALMASK &&
	  cmd->buttons & BTS_SAVEGAME)
	{
	  cmd->buttons &= ~BT_SPECIALMASK;
	  if (show_save_messages) // [Nugget]
	  { displaymsg("Game Saved (Suppressed)"); }
	}
    }
}

static void CheckDemoBuffer(size_t size)
{
  ptrdiff_t position = demo_p - demobuffer;
  if (position + size > maxdemosize)
  {
    maxdemosize += size + 128 * 1024; // add another 128K
    demobuffer = Z_Realloc(demobuffer, maxdemosize, PU_STATIC, 0);
    demo_p = position + demobuffer;
  }
}

// Demo limits removed -- killough

static void G_WriteDemoTiccmd(ticcmd_t* cmd)
{
  if (M_InputGameActive(input_demo_quit)) // press to end demo recording
    G_CheckDemoStatus();

  demo_p[0] = cmd->forwardmove;
  demo_p[1] = cmd->sidemove;
  if (!longtics)
  {
  demo_p[2] = (cmd->angleturn+128)>>8;
  demo_p[3] = cmd->buttons;
  }
  else
  {
    demo_p[2] = (cmd->angleturn & 0xff);
    demo_p[3] = (cmd->angleturn >> 8) & 0xff;
    demo_p[4] = cmd->buttons;
  }

  CheckDemoBuffer(16);   // killough 8/23/98

  G_ReadDemoTiccmd (cmd);         // make SURE it is exactly the same
}

boolean secretexit;

void G_ExitLevel(void)
{
  secretexit = false;
  gameaction = ga_completed;
}

// Here's for the german edition.
// IF NO WOLF3D LEVELS, NO SECRET EXIT!

void G_SecretExitLevel(void)
{
  secretexit = gamemode != commercial || haswolflevels;
  gameaction = ga_completed;
}

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//

static void G_PlayerFinishLevel(int player)
{
  player_t *p = &players[player];
  memset(p->powers, 0, sizeof p->powers);
  memset(p->cards, 0, sizeof p->cards);
  p->mo->flags &= ~MF_SHADOW;   // cancel invisibility
  p->extralight = 0;      // cancel gun flashes
  p->fixedcolormap = 0;   // cancel ir gogles
  p->damagecount = 0;     // no palette changes
  p->bonuscount = 0;
  // [crispy] reset additional player properties
  p->btuse_tics = 0;
  p->oldpitch = p->pitch = 0;
  p->centering = false;
  p->slope = 0;
  p->recoilpitch = p->oldrecoilpitch = 0;
  p->ticangle = p->oldticangle = 0;

  // [Nugget] Reset more additional player properties ------------------------

  p->jumptics = 0; // Jumping

  // Crouching
  p->mo->height = p->mo->info->height;
  p->mo->intflags &= ~MIF_CROUCHING;
  p->crouchoffset = 0;

  p->oldflinch = p->flinch = 0; // Flinching
}

// [crispy] format time for level statistics
#define TIMESTRSIZE 16
static void FormatLevelStatTime(char *str, int tics, boolean total)
{
    int exitHours, exitMinutes;
    float exitTime, exitSeconds;

    exitTime = (float) tics / TICRATE;
    exitHours = exitTime / 3600;
    exitTime -= exitHours * 3600;
    exitMinutes = exitTime / 60;
    exitTime -= exitMinutes * 60;
    exitSeconds = exitTime;

    if (total)
    {
        M_snprintf(str, TIMESTRSIZE, "%d:%02d",
                tics / TICRATE / 60, (tics % (60 * TICRATE)) / TICRATE);
    }
    else if (exitHours)
    {
        M_snprintf(str, TIMESTRSIZE, "%d:%02d:%05.2f",
                    exitHours, exitMinutes, exitSeconds);
    }
    else
    {
        M_snprintf(str, TIMESTRSIZE, "%01d:%05.2f", exitMinutes, exitSeconds);
    }
}

// [crispy] Write level statistics upon exit
static void G_WriteLevelStat(void)
{
    int playerKills = 0, playerItems = 0, playerSecrets = 0;

    char levelString[9] = {0};
    char levelTimeString[TIMESTRSIZE] = {0};
    char totalTimeString[TIMESTRSIZE] = {0};

    static boolean firsttime = true;

    FILE *fstream = NULL;

    if (firsttime)
    {
        firsttime = false;
        fstream = M_fopen("levelstat.txt", "w");
    }
    else
    {
        fstream = M_fopen("levelstat.txt", "a");
    }

    if (fstream == NULL)
    {
        I_Printf(VB_ERROR,
            "G_WriteLevelStat: Unable to open levelstat.txt for writing!");
        return;
    }

    strcpy(levelString, MapName(gameepisode, gamemap));

    FormatLevelStatTime(levelTimeString, leveltime, false);
    FormatLevelStatTime(totalTimeString, totalleveltimes + leveltime, true);

    for (int i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            playerKills += players[i].killcount - players[i].maxkilldiscount;
            playerItems += players[i].itemcount;
            playerSecrets += players[i].secretcount;
        }
    }

    fprintf(fstream, "%s%s - %s (%s)  K: %d/%d  I: %d/%d  S: %d/%d\n",
            levelString, (secretexit ? "s" : ""),
            levelTimeString, totalTimeString, playerKills, totalkills,
            playerItems, totalitems, playerSecrets, totalsecret);

    fclose(fstream);
}

// [Nugget] Custom Skill
void G_RestartWithLoadout(const boolean current)
{
  G_SetSkillParms(sk_custom);

  gameaction = ga_loadlevel;

  G_PlayerFinishLevel(consoleplayer);

  if (!current)
  {
    player_t *const player = &players[consoleplayer];

    player->playerstate = PST_LIVE;
    player->mo->health  = initial_loadout.mohealth;
    player->health      = initial_loadout.health;
    player->armorpoints = initial_loadout.armorpoints;
    player->armortype   = initial_loadout.armortype;
    player->backpack    = initial_loadout.backpack;
    player->readyweapon = initial_loadout.readyweapon;
    player->lastweapon  = initial_loadout.lastweapon;

    memcpy(player->weaponowned, initial_loadout.weaponowned, sizeof(player->weaponowned));
    memcpy(player->ammo,        initial_loadout.ammo,        sizeof(player->ammo));
    memcpy(player->maxammo,     initial_loadout.maxammo,     sizeof(player->maxammo));
  }

  if (automapactive)
  {
    if (automapactive == AM_MINI) { minimap_was_on = true; }

    AM_ChangeMode(AM_OFF);
  }

  AM_clearMarks();

  ignore_pistolstart = true;
}

//
// G_DoCompleted
//

boolean um_pars = false;

static void G_DoCompleted(void)
{
  int i;

  //!
  // @category demo
  // @help
  //
  // Write level statistics upon exit to levelstat.txt
  //

  if (M_CheckParm("-levelstat"))
  {
      G_WriteLevelStat();
  }

  gameaction = ga_nothing;

  for (i=0; i<MAXPLAYERS; i++)
    if (playeringame[i])
      G_PlayerFinishLevel(i);        // take away cards and stuff

  if (automapactive)
  {
    if (automapactive == AM_MINI) { minimap_was_on = true; }

    AM_ChangeMode(AM_OFF);
  }

  wminfo.nextep = wminfo.epsd = gameepisode -1;
  wminfo.last = gamemap -1;

  wminfo.lastmapinfo = gamemapinfo;
  wminfo.nextmapinfo = NULL;
  um_pars = false;
  if (gamemapinfo)
  {
    const char *next = NULL;
    boolean intermission = false;

    if (gamemapinfo->flags & MapInfo_EndGame)
    {
      if (gamemapinfo->flags & MapInfo_NoIntermission)
      {
        gameaction = ga_victory;
        return;
      }
      else
      {
        intermission = true;
      }
    }

    if (secretexit && gamemapinfo->nextsecret[0])
      next = gamemapinfo->nextsecret;
    else if (gamemapinfo->nextmap[0])
      next = gamemapinfo->nextmap;

    if (next)
    {
      G_ValidateMapName(next, &wminfo.nextep, &wminfo.next);
      wminfo.nextep--;
      wminfo.next--;
      // episode change
      if (wminfo.nextep != wminfo.epsd)
      {
        for (i = 0; i < MAXPLAYERS; i++)
          players[i].didsecret = false;
      }
    }

    if (next || intermission)
    {
      wminfo.didsecret = players[consoleplayer].didsecret;
      wminfo.partime = gamemapinfo->partime * TICRATE;
      if (wminfo.partime > 0)
        um_pars = true;
      goto frommapinfo;	// skip past the default setup.
    }
  }

  if (gamemode != commercial) // kilough 2/7/98
    switch(gamemap)
      {
      case 8:
        gameaction = ga_victory;
        return;
      case 9:
        for (i=0 ; i<MAXPLAYERS ; i++)
          players[i].didsecret = true;
        break;
      }

  wminfo.didsecret = players[consoleplayer].didsecret;

  // wminfo.next is 0 biased, unlike gamemap
  if (gamemode == commercial)
    {
      if (secretexit)
        switch(gamemap)
          {
          case 15:
            wminfo.next = 30; break;
          case 31:
            wminfo.next = 31; break;
          }
      else
        switch(gamemap)
          {
          case 31:
          case 32:
            wminfo.next = 15; break;
          default:
            wminfo.next = gamemap;
          }
    }
  else
    {
      if (secretexit)
        wminfo.next = 8;  // go to secret level
      else
        if (gamemap == 9)
          {
            // returning from secret level
            switch (gameepisode)
              {
              case 1:
                wminfo.next = 3;
                break;
              case 2:
                wminfo.next = 5;
                break;
              case 3:
                wminfo.next = 6;
                break;
              case 4:
                wminfo.next = 2;
                break;
              }
          }
        else
          wminfo.next = gamemap;          // go to next level
    }

  if (gamemode == commercial)
  {
    // MAP33 reads its par time from beyond the cpars[] array.
    if (demo_compatibility && gamemap == 33)
    {
      int cpars32;

      memcpy(&cpars32, s_GAMMALVL0, sizeof(int));
      wminfo.partime = TICRATE*LONG(cpars32);
    }
    else if (gamemap >= 1 && gamemap <= 34)
    {
      wminfo.partime = TICRATE*cpars[gamemap-1];
    }
  }
  else
  {
    // Doom Episode 4 doesn't have a par time, so this overflows into the cpars[] array.
    if (demo_compatibility && gameepisode == 4 && gamemap >= 1 && gamemap <= 9)
    {
      wminfo.partime = TICRATE*cpars[gamemap];
    }
    else if (gameepisode >= 1 && gameepisode <= 3 && gamemap >= 1 && gamemap <= 9)
    {
      wminfo.partime = TICRATE*pars[gameepisode][gamemap];
    }
  }

frommapinfo:

  wminfo.nextmapinfo = G_LookupMapinfo(wminfo.nextep+1, wminfo.next+1);

  wminfo.maxkills = totalkills;
  wminfo.maxitems = totalitems;
  wminfo.maxsecret = totalsecret;
  wminfo.maxfrags = 0;

  wminfo.pnum = consoleplayer;

  for (i=0 ; i<MAXPLAYERS ; i++)
    {
      wminfo.plyr[i].in = playeringame[i];
      wminfo.plyr[i].skills = players[i].killcount;
      wminfo.plyr[i].sitems = players[i].itemcount;
      wminfo.plyr[i].ssecret = players[i].secretcount;
      wminfo.plyr[i].stime = leveltime;
      memcpy (wminfo.plyr[i].frags, players[i].frags,
              sizeof(wminfo.plyr[i].frags));
    }

  // [FG] total time for all completed levels
  wminfo.totaltimes = (totalleveltimes += (leveltime - leveltime % TICRATE));

  gamestate = GS_INTERMISSION;
  viewactive = false;
  automapactive = AM_OFF;

  // [FG] -statdump implementation from Chocolate Doom
  if (gamemode == commercial || gamemap != 8)
  {
    StatCopy(&wminfo);
  }

  for (int i = 0; i < MAXPLAYERS; ++i)
  {
      level_t level = {gameepisode, gamemap};
      array_push(players[i].visitedlevels, level);
      players[i].num_visitedlevels = array_size(players[i].visitedlevels);
  }
  wminfo.visitedlevels = players[consoleplayer].visitedlevels;

  WI_Start (&wminfo);

  // [Nugget] ----------------------------------------------------------------

  // Slow Motion
  G_ResetSlowMotion();

  // Clear visual effects
  R_ClearFOVFX();
  R_ClearShake();
}

static void G_DoWorldDone(void)
{
  idmusnum = -1;             //jff 3/17/98 allow new level's music to be loaded
  musinfo.from_savegame = false;
  gamestate = GS_LEVEL;
  gameepisode = wminfo.nextep + 1;
  gamemap = wminfo.next+1;
  gamemapinfo = G_LookupMapinfo(gameepisode, gamemap);
  G_DoLoadLevel();
  gameaction = ga_nothing;
  viewactive = true;
  AM_clearMarks();           //jff 4/12/98 clear any marks on the automap

  if (autosave && !demorecording && !demoplayback && !netgame)
  {
    M_SaveAutoSave();
  }
  
  G_UpdateInitialLoadout(); // [Nugget] Custom Skill
}

// killough 2/28/98: A ridiculously large number
// of players, the most you'll ever need in a demo
// or savegame. This is used to prevent problems, in
// case more players in a game are supported later.

#define MIN_MAXPLAYERS 32

static void InvalidDemo(void)
{
    gameaction = ga_nothing;
    demoplayback = true;
    G_CheckDemoStatus();
}

static void G_DoPlayDemo(void)
{
  skill_t skill;
  int i, episode, map;
  demo_version_t demover;
  byte *option_p = NULL;      // killough 11/98
  int demolength;

  if (gameaction != ga_loadgame)      // killough 12/98: support -loadgame
    basetic = gametic;  // killough 9/29/98

  // [crispy] in demo continue mode free the obsolete demo buffer
  // of size 'maxdemosize' previously allocated in G_RecordDemo()
  if (demorecording)
  {
      Z_Free(demobuffer);
  }

  char *filename = NULL;
  if (singledemo)
  {
      filename = D_FindLMPByName(defdemoname);
  }

  if (singledemo && filename)
  {
      M_ReadFile(filename, &demobuffer);
      demolength = M_FileLength(filename);
      demo_p = demobuffer;
      I_Printf(VB_DEMO, "G_DoPlayDemo: %s", filename);
  }
  else
  {
      char lumpname[9] = {0};
      W_ExtractFileBase(defdemoname, lumpname);           // killough
      int lumpnum = W_GetNumForName(lumpname);
      demolength = W_LumpLength(lumpnum);
      demobuffer = demo_p = W_CacheLumpNum(lumpnum, PU_STATIC);  // killough
      I_Printf(VB_DEMO, "G_DoPlayDemo: %s (%s)", lumpname, W_WadNameForLump(lumpnum));
  }

  // [FG] ignore too short demo lumps
  if (demolength < 0xd)
  {
    I_Printf(VB_WARNING, "G_DoPlayDemo: Short demo lump %s.", defdemoname);
    InvalidDemo();
    return;
  }

  demover = *demo_p++;

  // skip UMAPINFO demo header
  if (demover == DV_UM)
  {
    // we check for the PR+UM signature.
    // Eternity Engine also uses 255 demover, with other signatures.
    if (memcmp(demo_p, "PR+UM", 5))
    {
      I_Printf(VB_WARNING,
            "Extended demo format %d found, but \"PR+UM\" string not found.",
            demover);
      InvalidDemo();
      return;
    }

    demo_p += 6;

    if (*demo_p++ != 1)
    {
      I_Error("G_DoPlayDemo: Unknown demo format.");
    }

    // the defunct format had only one extension (in two bytes)
    if (*demo_p++ != 1 || *demo_p++ != 0)
    {
      I_Error("G_DoPlayDemo: Unknown demo format.");
    }

    if (*demo_p++ != 8)
    {
      I_Error("G_DoPlayDemo: Unknown demo format.");
    }

    if (memcmp(demo_p, "UMAPINFO", 8))
    {
      I_Error("G_DoPlayDemo: Unknown demo format.");
    }

    demo_p += 8;

    // skip map name
    demo_p += 8;

    // "real" demover
    demover = *demo_p++;
  }

  // killough 2/22/98, 2/28/98: autodetect old demos and act accordingly.
  // Old demos turn on demo_compatibility => compatibility; new demos load
  // compatibility flag, and other flags as well, as a part of the demo.

  demo_version = demover;     // killough 7/19/98: use the version id stored in demo

  // [FG] PrBoom's own demo format starts with demo version 210
  if (demover >= 210 && !mbf21)
  {
    I_Printf(VB_WARNING, "Unknown demo format %d.", demover);
    InvalidDemo();
    return;
  }

  longtics = false;

  if (demover < DV_BOOM200)     // Autodetect old demos
    {
      if (demover == DV_LONGTIC)
      {
        longtics = true;
      }
      compatibility = true;
      memset(comp, 0xff, sizeof comp);  // killough 10/98: a vector now

      // killough 3/2/98: force these variables to be 0 in demo_compatibility

      variable_friction = 0;

      weapon_recoil = 0;

      allow_pushers = 0;

      monster_infighting = 1;           // killough 7/19/98

      classic_bfg = 0;                  // killough 7/19/98
      beta_emulation = 0;               // killough 7/24/98

      dogs = 0;                         // killough 7/19/98
      dog_jumping = 0;                  // killough 10/98

      monster_backing = 0;              // killough 9/8/98

      monster_avoid_hazards = 0;        // killough 9/9/98

      monster_friction = 0;             // killough 10/98
      help_friends = 0;                 // killough 9/9/98
      monkeys = 0;

      // killough 3/6/98: rearrange to fix savegame bugs (moved fastparm,
      // respawnparm, nomonsters flags to G_LoadOptions()/G_SaveOptions())

      if (demover >= 100)         // For demos from versions >= 1.4
        {
          skill = *demo_p++;
          episode = *demo_p++;
          map = *demo_p++;
          deathmatch = *demo_p++;
          respawnparm = *demo_p++;
          fastparm = *demo_p++;
          nomonsters = *demo_p++;
          consoleplayer = *demo_p++;
        }
      else
        {
          skill = (int)demover;
          episode = *demo_p++;
          map = *demo_p++;
          deathmatch = respawnparm = fastparm =
            nomonsters = consoleplayer = 0;
        }
    }
  else    // new versions of demos
    {
      demo_p += 6;               // skip signature;

      if (mbf21)
      {
        longtics = true;
        compatibility = 0;
      }
      else
      {
      compatibility = *demo_p++;       // load old compatibility flag
      }
      skill = *demo_p++;
      episode = *demo_p++;
      map = *demo_p++;
      deathmatch = *demo_p++;
      consoleplayer = *demo_p++;

      // killough 11/98: save option pointer for below
      if (demover >= DV_MBF)
	option_p = demo_p;

      // killough 3/1/98: Read game options
      if (mbf21)
        demo_p = G_ReadOptionsMBF21(demo_p);
      else
        demo_p = G_ReadOptions(demo_p);

      if (demover == DV_BOOM200) // killough 6/3/98: partially fix v2.00 demos
        demo_p += 256-G_GameOptionSize();
    }

  if (demo_compatibility)  // only 4 players can exist in old demos
    {
      for (i=0; i<4; i++)  // intentionally hard-coded 4 -- killough
        playeringame[i] = *demo_p++;
      for (;i < MAXPLAYERS; i++)
        playeringame[i] = 0;
    }
  else
    {
      for (i=0 ; i < MAXPLAYERS; i++)
        playeringame[i] = *demo_p++;
      demo_p += MIN_MAXPLAYERS - MAXPLAYERS;
    }

  if (playeringame[1])
    netgame = netdemo = true;

  // don't spend a lot of time in loadlevel

  if (gameaction != ga_loadgame)      // killough 12/98: support -loadgame
    {
      // killough 2/22/98:
      // Do it anyway for timing demos, to reduce timing noise
      precache = timingdemo;

      G_InitNew(skill, episode, map);

      // killough 11/98: If OPTIONS were loaded from the wad in G_InitNew(),
      // reload any demo sync-critical ones from the demo itself, to be exactly
      // the same as during recording.

      if (option_p)
      {
        if (mbf21)
          G_ReadOptionsMBF21(option_p);
        else
          G_ReadOptions(option_p);
      }
    }

  precache = true;
  usergame = false;
  demoplayback = true;

  for (i=0; i<MAXPLAYERS;i++)         // killough 4/24/98
    players[i].cheats = 0;

  gameaction = ga_nothing;

  maxdemosize = demolength;

  // [crispy] demo progress bar
  {
    int i;
    int playerscount = 0;
    byte *demo_ptr = demo_p;

    playback_totaltics = playback_tic = 0;

    for (i = 0; i < MAXPLAYERS; ++i)
    {
      if (playeringame[i])
        ++playerscount;
    }

    while (*demo_ptr != DEMOMARKER && (demo_ptr - demobuffer) < demolength)
    {
      demo_ptr += playerscount * (longtics ? 5 : 4);
      ++playback_totaltics;
    }
  }

  D_UpdateCasualPlay(); // [Nugget]
}

#define VERSIONSIZE   16

// killough 2/22/98: version id string format for savegames
#define VERSIONID "MBF %d"

#define CURRENT_SAVE_VERSION "Nugget 4.0.0" // [Nugget]

static char *savename = NULL;

//
// killough 5/15/98: add forced loadgames, which allow user to override checks
//

static boolean forced_loadgame = false;
static boolean command_loadgame = false;

void G_ForcedLoadAutoSave(void)
{
  gameaction = ga_loadautosave;
  forced_loadgame = true;
}

void G_ForcedLoadGame(void)
{
  gameaction = ga_loadgame;
  forced_loadgame = true;
}

// killough 3/16/98: add slot info
// killough 5/15/98: add command-line

void G_LoadAutoSave(char *name, boolean command)
{
  free(savename);
  savename = M_StringDuplicate(name);
  gameaction = ga_loadautosave;
  forced_loadgame = false;
  command_loadgame = command;
}

void G_LoadGame(char *name, int slot, boolean command)
{
  if (savename) free(savename);
  savename = M_StringDuplicate(name);
  savegameslot = slot;
  gameaction = ga_loadgame;
  forced_loadgame = false;
  command_loadgame = command;
}

// killough 5/15/98:
// Consistency Error when attempting to load savegame.

static void G_LoadAutoSaveErr(const char *msg)
{
  Z_Free(savebuffer);
  MN_ForcedLoadAutoSave(msg);

  if (command_loadgame)
  {
    G_CheckDemoStatus();
    D_StartTitle();
    gamestate = GS_DEMOSCREEN;
  }
}

static void G_LoadGameErr(const char *msg)
{
  Z_Free(savebuffer);                // Free the savegame buffer
  MN_ForcedLoadGame(msg);             // Print message asking for 'Y' to force
  if (command_loadgame)              // If this was a command-line -loadgame
    {
      G_CheckDemoStatus();           // If there was also a -record
      D_StartTitle();                // Start the title screen
      gamestate = GS_DEMOSCREEN;     // And set the game state accordingly
    }
}

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string
//

void G_SaveAutoSave(char *description)
{
  strcpy(savedescription, description);
  save_autosave = true;
}

void G_SaveGame(int slot, char *description)
{
  savegameslot = slot;
  strcpy(savedescription, description);
  sendsave = true;
}

// Check for overrun and realloc if necessary -- Lee Killough 1/22/98
void CheckSaveGame(size_t size)
{
  size_t pos = save_p - savebuffer;
  size += 1024;  // breathing room
  if (pos+size > savegamesize)
    save_p = (savebuffer = Z_Realloc(savebuffer,
           savegamesize += (size+1023) & ~1023, PU_STATIC, 0)) + pos;
}

// killough 3/22/98: form savegame name in one location
// (previously code was scattered around in multiple places)

// [FG] support up to 8 pages of savegames

static char *SaveGameName(const char *buf)
{
  char *filepath =
    // [Nugget] Restored `-cdrom` parm
#ifdef _WIN32
    M_CheckParm("-cdrom") ? M_StringJoin("c:\\doomdata\\", buf) :
#endif
    M_StringJoin(basesavegame, DIR_SEPARATOR_S, buf);

  char *existing = M_FileCaseExists(filepath);

  if (existing)
  {
    free(filepath);
    return existing;
  }
  else
  {
    char *filename = (char *)M_BaseName(filepath);
    M_StringToLower(filename);
    return filepath;
  }
}

char *G_AutoSaveName(void)
{
  // [Nugget] Periodic auto save /--------------------------------------------

  char buf[16] = {0};
  static int autoslot = -1;
  static boolean last_save_was_periodic = false; // Only changes upon saving

  if (saving_periodic_autosave)
  {
    // We're autosaving (periodic)

    autoslot = (autoslot + 1) % 7; // 8 pages, minus one with the level-end save

    sprintf(buf, "autosav%i.dsg", autoslot + 1);
    last_save_was_periodic = true;
  }
  else if (death_use_state == DEATH_USE_STATE_ACTIVE)
  {
    // We're loading by on-death action

    if (last_save_was_periodic)
    {
      sprintf(buf, "autosav%i.dsg", autoslot + 1);
    }
    else { sprintf(buf, "autosave.dsg"); }
  }
  else if (savepage > 0 && gameaction != ga_saveautosave)
  {
    // We're loading a periodic auto save manually

    sprintf(buf, "autosav%i.dsg", savepage);
  }
  else {
    // We're loading a level-end auto save manually, or autosaving upon level end

    sprintf(buf, "autosave.dsg");

    if (gameaction == ga_saveautosave)
    {
      // We're autosaving upon level end
      last_save_was_periodic = false;
    }
  }

  // [Nugget] ---------------------------------------------------------------/

  return SaveGameName(buf);
}

char *G_SaveGameName(int slot)
{
  // Ty 05/04/98 - use savegamename variable (see d_deh.c)
  // killough 12/98: add .7 to truncate savegamename
  char buf[16] = {0};
  sprintf(buf, "%.7s%d.dsg", savegamename, 10 * savepage + slot);
  return SaveGameName(buf);
}

char* G_MBFSaveGameName(int slot)
{
  char buf[16] = {0};
  sprintf(buf, "MBFSAV%d.dsg", 10*savepage+slot);

  char *filepath = M_StringJoin(basesavegame, DIR_SEPARATOR_S, buf);
  char *existing = M_FileCaseExists(filepath);

  if (existing)
  {
    free(filepath);
    return existing;
  }
  else
  {
    return filepath;
  }
}

// killough 12/98:
// This function returns a signature for the current wad.
// It is used to distinguish between wads, for the purposes
// of savegame compatibility warnings, and options lookups.
//
// killough 12/98: use faster algorithm which has less IO

static uint64_t G_Signature(int sig_epi, int sig_map)
{
  uint64_t s = 0;
  int lump, i;
  char name[9];
  
  strcpy(name, MapName(sig_epi, sig_map));

  lump = W_CheckNumForName(name);

  if (lump != -1 && (i = lump+10) < numlumps)
    do
      s = s*2+W_LumpLength(i);
    while (--i > lump);

  return s;
}

static void DoSaveGame(char *name)
{
  char name2[VERSIONSIZE];
  char *description;
  int  length, i;

  keyframe_rw = false; // [Nugget] Make sure endian-unsafe R/W is disabled

  description = savedescription;

  save_p = savebuffer = Z_Malloc(savegamesize, PU_STATIC, 0);

  CheckSaveGame(SAVESTRINGSIZE+VERSIONSIZE+sizeof(uint64_t));
  memcpy (save_p, description, SAVESTRINGSIZE);
  save_p += SAVESTRINGSIZE;
  memset (name2,0,sizeof(name2));

  // killough 2/22/98: "proprietary" version string :-)
  strcpy(name2, CURRENT_SAVE_VERSION);
  saveg_compat = saveg_current;

  memcpy (save_p, name2, VERSIONSIZE);
  save_p += VERSIONSIZE;

  *save_p++ = demo_version;

  // killough 2/14/98: save old compatibility flag:
  *save_p++ = compatibility;

  *save_p++ = gameskill;
  *save_p++ = gameepisode;
  *save_p++ = gamemap;

  {  // killough 3/16/98, 12/98: store lump name checksum
    uint64_t checksum = G_Signature(gameepisode, gamemap);
    saveg_write64(checksum);
  }

  // killough 3/16/98: store pwad filenames in savegame
  {
    int i;
    for (*save_p = 0, i = 0; i < array_size(wadfiles); i++)
      {
        const char *basename = M_BaseName(wadfiles[i]);
        CheckSaveGame(strlen(basename)+2);
        strcat(strcat((char *) save_p, basename), "\n");
      }
    save_p += strlen((char *) save_p)+1;
  }

  CheckSaveGame(G_GameOptionSize()+MIN_MAXPLAYERS+10);

  for (i=0 ; i<MAXPLAYERS ; i++)
    *save_p++ = playeringame[i];

  for (;i<MIN_MAXPLAYERS;i++)         // killough 2/28/98
    *save_p++ = 0;

  *save_p++ = idmusnum;               // jff 3/17/98 save idmus state

  save_p = G_WriteOptions(save_p);    // killough 3/1/98: save game options

  // [FG] fix copy size and pointer progression
  saveg_write32(leveltime); //killough 11/98: save entire word

  // killough 11/98: save revenant tracer state
  *save_p++ = (gametic-basetic) & 255;

  P_ArchivePlayers();
  P_ArchiveWorld();
  P_ArchiveThinkers();
  P_ArchiveSpecials();
  P_ArchiveRNG();    // killough 1/18/98: save RNG information
  P_ArchiveMap();    // killough 1/22/98: save automap information

  *save_p++ = 0xe6;   // consistancy marker

  // [FG] save total time for all completed levels
  CheckSaveGame(sizeof totalleveltimes);
  saveg_write32(totalleveltimes);

  // save lump name for current MUSINFO item
  CheckSaveGame(8);
  if (musinfo.current_item > 0)
    memcpy(save_p, lumpinfo[musinfo.current_item].name, 8);
  else
    memset(save_p, 0, 8);
  save_p += 8;

  // save max_kill_requirement
  CheckSaveGame(sizeof(max_kill_requirement));
  saveg_write32(max_kill_requirement);

  // [Nugget] /===============================================================

  // Save milestones
  CheckSaveGame(sizeof(complete_milestones));
  saveg_write_enum(complete_milestones);

  // Save custom-skill settings ----------------------------------------------

  CheckSaveGame(sizeof(customskill));

  saveg_write32(customskill.things);
  saveg_write32(customskill.coopspawns);
  saveg_write32(customskill.nomonsters);
  saveg_write32(customskill.doubleammo);
  saveg_write32(customskill.halfdamage);
  saveg_write32(customskill.slowbrain);
  saveg_write32(customskill.fast);
  saveg_write32(customskill.respawn);
  saveg_write32(customskill.aggressive);
  saveg_write32(customskill.x2monsters);

  CheckSaveGame(sizeof(initial_loadout));

  saveg_write32(initial_loadout.mohealth);
  saveg_write32(initial_loadout.health);
  saveg_write32(initial_loadout.armorpoints);
  saveg_write32(initial_loadout.armortype);
  saveg_write32(initial_loadout.backpack);
  saveg_write_enum(initial_loadout.readyweapon);
  saveg_write_enum(initial_loadout.lastweapon);

  for (int i = 0;  i < NUMWEAPONS;  i++)
  { saveg_write32(initial_loadout.weaponowned[i]); }

  for (int i = 0;  i < NUMAMMO;  i++)
  { saveg_write32(initial_loadout.ammo[i]); }

  for (int i = 0;  i < NUMAMMO;  i++)
  { saveg_write32(initial_loadout.maxammo[i]); }

  // [Nugget] ===============================================================/

  // [Nugget] Periodic auto save
  if (!saving_periodic_autosave)
  {
    // [FG] save snapshot
    CheckSaveGame(MN_SnapshotDataSize());
    MN_WriteSnapshot(save_p);
    save_p += MN_SnapshotDataSize();
  }

  length = save_p - savebuffer;

  M_MakeDirectory(basesavegame);

  if (!M_WriteFile(name, savebuffer, length))
    displaymsg("%s", errno ? strerror(errno) : "Could not save game: Error unknown");
  else if (show_save_messages && !saving_periodic_autosave) // [Nugget]
    displaymsg("%s", s_GGSAVED);  // Ty 03/27/98 - externalized

  Z_Free(savebuffer);  // killough
  savebuffer = save_p = NULL;

  gameaction = ga_nothing;
  savedescription[0] = 0;

  if (name) free(name);

  drs_skip_frame = true;

  // [Nugget] Periodic auto save:
  // reset the countdown, even if this was a manual save
  G_SetAutoSaveCountdown(autosave_interval * TICRATE);
}

static void G_DoSaveGame(void)
{
  char *name = G_SaveGameName(savegameslot);
  DoSaveGame(name);
  MN_SetQuickSaveSlot(savegameslot);
}

static void G_DoSaveAutoSave(void)
{
  char *name = G_AutoSaveName();
  DoSaveGame(name);
}

static void CheckSaveVersion(const char *str, saveg_compat_t ver)
{
  if (strncmp((char *) save_p, str, strlen(str)) == 0)
  {
    saveg_compat = ver;
  }
}

static boolean DoLoadGame(boolean do_load_autosave)
{
  int  length, i;
  char vcheck[VERSIONSIZE];
  uint64_t checksum;
  int tmp_compat, tmp_skill, tmp_epi, tmp_map;

  keyframe_rw = false; // [Nugget] Make sure endian-unsafe R/W is disabled

  I_SetFastdemoTimer(false);

  // [crispy] loaded game must always be single player.
  // Needed for ability to use a further game loading, as well as
  // cheat codes and other single player only specifics.
  if (!command_loadgame)
  {
    netdemo = false;
    netgame = false;
    solonet = false;
    deathmatch = false;
  }

  gameaction = ga_nothing;

  length = M_ReadFile(savename, &savebuffer);
  save_p = savebuffer + SAVESTRINGSIZE;

  // skip the description field

  // killough 2/22/98: "proprietary" version string :-)
  sprintf (vcheck,VERSIONID,MBFVERSION);

  CheckSaveVersion(vcheck, saveg_mbf);
  CheckSaveVersion("Woof 6.0.0", saveg_woof600);
  CheckSaveVersion("Nugget 2.0.0", saveg_nugget200);
  CheckSaveVersion("Nugget 2.1.0", saveg_nugget210);
  CheckSaveVersion("Nugget 2.4.0", saveg_nugget300);
  CheckSaveVersion("Nugget 3.2.0", saveg_nugget320);
  CheckSaveVersion("Nugget 3.3.0", saveg_nugget330);
  CheckSaveVersion(CURRENT_SAVE_VERSION, saveg_current);

  // killough 2/22/98: Friendly savegame version difference message
  if (!forced_loadgame && saveg_compat != saveg_mbf && saveg_compat < saveg_woof600)
    {
      const char *msg = "Different Savegame Version!!!\n\nAre you sure?";
      if (do_load_autosave)
        G_LoadAutoSaveErr(msg);
      else
        G_LoadGameErr(msg);
      return false;
    }

  save_p += VERSIONSIZE;

  if (saveg_compat > saveg_woof510)
  {
    demo_version = *save_p++;
  }
  else
  {
    demo_version = DV_MBF;
  }

  // killough 2/14/98: load compatibility mode
  tmp_compat = *save_p++;

  tmp_skill = *save_p++;
  tmp_epi = *save_p++;
  tmp_map = *save_p++;

  checksum = saveg_read64();

  if (!forced_loadgame)
   {  // killough 3/16/98, 12/98: check lump name checksum
     if (checksum != G_Signature(tmp_epi, tmp_map))
       {
	 char *msg = malloc(strlen((char *) save_p) + 128);
	 strcpy(msg,"Incompatible Savegame!!!\n");
	 if (save_p[sizeof checksum])
	   strcat(strcat(msg,"Wads expected:\n\n"), (char *) save_p);
	 strcat(msg, "\nAre you sure?");
	 if (do_load_autosave)
	   G_LoadAutoSaveErr(msg);
	 else
	   G_LoadGameErr(msg);
	 free(msg);
	 return false;
       }
   }

  while (*save_p++);

  compatibility = tmp_compat;
  gameskill = tmp_skill;
  gameepisode = tmp_epi;
  gamemap = tmp_map;
  gamemapinfo = G_LookupMapinfo(gameepisode, gamemap);

  for (i=0 ; i<MAXPLAYERS ; i++)
    playeringame[i] = *save_p++;
  save_p += MIN_MAXPLAYERS-MAXPLAYERS;         // killough 2/28/98

  // jff 3/17/98 restore idmus music
  // jff 3/18/98 account for unsigned byte
  // killough 11/98: simplify
  idmusnum = *(signed char *) save_p++;

  /* cph 2001/05/23 - Must read options before we set up the level */
  if (mbf21)
    G_ReadOptionsMBF21(save_p);
  else
    G_ReadOptions(save_p);

  // load a base level
  G_InitNew(gameskill, gameepisode, gamemap);

  // killough 3/1/98: Read game options
  // killough 11/98: move down to here
  /* cph - MBF needs to reread the savegame options because G_InitNew
   * rereads the WAD options. The demo playback code does this too. */
  if (mbf21)
    save_p = G_ReadOptionsMBF21(save_p);
  else
    save_p = G_ReadOptions(save_p);

  // get the times
  // killough 11/98: save entire word
  // [FG] fix copy size and pointer progression
  leveltime = saveg_read32();

  // killough 11/98: load revenant tracer state
  basetic = gametic - (int) *save_p++;

  // dearchive all the modifications
  P_MapStart();
  P_UnArchivePlayers();
  P_UnArchiveWorld();
  P_UnArchiveThinkers();
  P_UnArchiveSpecials();
  P_UnArchiveRNG();    // killough 1/18/98: load RNG information
  P_UnArchiveMap();    // killough 1/22/98: load automap information
  P_MapEnd();

  if (*save_p != 0xe6)
    I_Error ("Bad savegame");

  // [FG] restore total time for all completed levels
  if (save_p++ - savebuffer < length - sizeof totalleveltimes)
  {
    totalleveltimes = saveg_read32();
  }

  // restore MUSINFO music
  if (save_p - savebuffer <= length - 8)
  {
    char lump[9] = {0};
    int i;

    memcpy(lump, save_p, 8);

    i = W_CheckNumForName(lump);

    if (lump[0] && i > 0)
    {
      musinfo.mapthing = NULL;
      musinfo.lastmapthing = NULL;
      musinfo.tics = 0;
      musinfo.current_item = i;
      musinfo.from_savegame = true;
      S_ChangeMusInfoMusic(i, true);
    }

    save_p += 8;
  }

  // restore max_kill_requirement
  max_kill_requirement = totalkills;
  if (save_p - savebuffer <= length - sizeof(max_kill_requirement))
  {
    if (saveg_compat > saveg_nugget210) // [Nugget]
    {
      max_kill_requirement = saveg_read32();
    }
    // [Nugget]
    else if (saveg_compat > saveg_woof510)
    {
      max_kill_requirement += saveg_read32();
    }
  }

  // [Nugget] /---------------------------------------------------------------

  // Was `extrakills`
  if (saveg_nugget210 >= saveg_compat && saveg_compat > saveg_woof600)
  { saveg_read32(); }

  // Restore milestones
  if (saveg_compat > saveg_nugget200 && (save_p - savebuffer) <= (length - sizeof(complete_milestones)))
  { complete_milestones = saveg_read_enum(); }

  // Restore custom-skill settings
  if (saveg_compat > saveg_nugget300)
  {
    #define READ(x)                                      \
      if ((save_p - savebuffer) <= (length - sizeof(x))) \
        x = saveg_read32()

    READ(customskill.things);
    READ(customskill.coopspawns);
    READ(customskill.nomonsters);
    READ(customskill.doubleammo);
    READ(customskill.halfdamage);
    READ(customskill.slowbrain);
    READ(customskill.fast);
    READ(customskill.respawn);
    READ(customskill.aggressive);

    if (saveg_compat > saveg_nugget320)
    { READ(customskill.x2monsters); }

    if (gameskill == sk_custom) { G_SetSkillParms(sk_custom); }

    READ(initial_loadout.mohealth);
    READ(initial_loadout.health);
    READ(initial_loadout.armorpoints);
    READ(initial_loadout.armortype);
    READ(initial_loadout.backpack);

    if ((save_p - savebuffer) <= (length - sizeof(initial_loadout.readyweapon)))
    { initial_loadout.readyweapon = saveg_read_enum(); }

    if ((save_p - savebuffer) <= (length - sizeof(initial_loadout.lastweapon)))
    { initial_loadout.lastweapon  = saveg_read_enum(); }

    if ((save_p - savebuffer) <= (length - sizeof(initial_loadout.weaponowned)))
    {
      for (int i = 0;  i < NUMWEAPONS;  i++)
      { initial_loadout.weaponowned[i] = saveg_read32(); }
    }

    if ((save_p - savebuffer) <= (length - sizeof(initial_loadout.ammo)))
    {
      for (int i = 0;  i < NUMAMMO;  i++)
      { initial_loadout.ammo[i] = saveg_read32(); }
    }

    if ((save_p - savebuffer) <= (length - sizeof(initial_loadout.maxammo)))
    {
      for (int i = 0;  i < NUMAMMO;  i++)
      { initial_loadout.maxammo[i] = saveg_read32(); }
    }

    #undef READ
  }

  // [Nugget] ---------------------------------------------------------------/
  
  // done
  Z_Free(savebuffer);

  // [Nugget] Periodic auto save:
  // we already have a save (the one we just loaded), so reset the countdown
  G_SetAutoSaveCountdown(autosave_interval * TICRATE);

  // [Nugget] Rewind:
  // Just like with `G_DoRewind`,
  // this is called before the countdown decrement in `G_Ticker()`,
  // so add 1 to keep it aligned
  G_SetRewindCountdown(((rewind_interval * TICRATE) + 1) - ((leveltime - 1) % (rewind_interval * TICRATE)));

  if (setsizeneeded)
    R_ExecuteSetViewSize();

  // draw the pattern into the back screen
  R_FillBackScreen();

  // killough 12/98: support -recordfrom and -loadgame -playdemo
  if (!command_loadgame)
    singledemo = false;         // Clear singledemo flag if loading from menu
  else
    if (singledemo)
      {
	gameaction = ga_loadgame; // Mark that we're loading a game before demo
	G_DoPlayDemo();           // This will detect it and won't reinit level
      }
    else       // Loading games from menu isn't allowed during demo recordings,
      if (demorecording) // So this can only possibly be a -recordfrom command.
	G_BeginRecording();// Start the -recordfrom, since the game was loaded.

  ST_Start();

  return true;
}

static void PrintLevelTimes(void)
{
  if (totalleveltimes)
  {
    I_Printf(VB_DEBUG, "(%d:%02d) ",
             ((totalleveltimes + leveltime) / TICRATE) / 60,
             ((totalleveltimes + leveltime) / TICRATE) % 60);
  }

  I_Printf(VB_DEBUG, "%d:%05.2f", leveltime / TICRATE / 60,
           (float)(leveltime % (60 * TICRATE)) / TICRATE);
}

static void G_DoLoadGame(void)
{
  if (DoLoadGame(false))
  {
    const int slot_num = 10 * savepage + savegameslot;
    I_Printf(VB_DEBUG, "G_DoLoadGame: Slot %02d, Time ", slot_num);
    PrintLevelTimes();
    MN_SetQuickSaveSlot(savegameslot);
  }
}

static void G_DoLoadAutoSave(void)
{
  if (DoLoadGame(true))
  {
    I_Printf(VB_DEBUG, "G_DoLoadGame: Auto Save, Time ");
    PrintLevelTimes();
  }
}

boolean G_AutoSaveEnabled(void)
{
  return autosave;
}

//
// G_LoadAutoSaveDeathUse
// Loads the auto save if it's more recent than the current save slot.
// Returns true if the auto save is loaded.
//
boolean G_LoadAutoSaveDeathUse(void)
{
  struct stat st;
  char *auto_path = G_AutoSaveName();
  time_t auto_time = (M_stat(auto_path, &st) != -1 ? st.st_mtime : 0);
  boolean result = (auto_time > 0);

  if (result)
  {
    if (savegameslot >= 0)
    {
      char *save_path = G_SaveGameName(savegameslot);
      time_t save_time = (M_stat(save_path, &st) != -1 ? st.st_mtime : 0);
      free(save_path);
      result = (auto_time > save_time);
    }

    if (result)
    {
      G_LoadAutoSave(auto_path, false);
    }
  }

  free(auto_path);
  return result;
}

static void CheckSaveAutoSave(void)
{
  if (save_autosave)
  {
    save_autosave = false;
    gameaction = ga_saveautosave;
  }
}

// [Nugget] Rewind /----------------------------------------------------------

void G_SetRewindCountdown(int value)
{
  rewind_countdown = value;
}

static void G_SaveKeyFrame(void)
{
  int length, i;
  const int start_time = I_GetTimeMS();

  save_p = savebuffer = Z_Malloc(savegamesize, PU_STATIC, NULL);

  saveg_compat = saveg_current;

  keyframe_rw = true;

  *save_p++ = demo_version;

  // killough 2/14/98: save old compatibility flag:
  *save_p++ = compatibility;

  *save_p++ = gameskill;
  *save_p++ = gameepisode;
  *save_p++ = gamemap;

  CheckSaveGame(G_GameOptionSize()+MIN_MAXPLAYERS+10);

  for (i=0 ; i<MAXPLAYERS ; i++)
    *save_p++ = playeringame[i];

  for (;i<MIN_MAXPLAYERS;i++)         // killough 2/28/98
    *save_p++ = 0;

  *save_p++ = idmusnum;               // jff 3/17/98 save idmus state

  save_p = G_WriteOptions(save_p);    // killough 3/1/98: save game options

  // [FG] fix copy size and pointer progression
  saveg_write32(leveltime); //killough 11/98: save entire word

  // killough 11/98: save revenant tracer state
  *save_p++ = (gametic-basetic) & 255;

  P_ArchivePlayers();
  P_ArchiveWorld();
  P_ArchiveThinkers();
  P_ArchiveSpecials();
  P_ArchiveRNG();    // killough 1/18/98: save RNG information
  P_ArchiveMap();    // killough 1/22/98: save automap information

  *save_p++ = 0xe6;   // consistancy marker

  // [FG] save total time for all completed levels
  CheckSaveGame(sizeof totalleveltimes);
  saveg_write32(totalleveltimes);

  // save lump name for current MUSINFO item
  CheckSaveGame(8);
  if (musinfo.current_item > 0)
    memcpy(save_p, lumpinfo[musinfo.current_item].name, 8);
  else
    memset(save_p, 0, 8);
  save_p += 8;

  // save max_kill_requirement
  CheckSaveGame(sizeof(max_kill_requirement));
  saveg_write32(max_kill_requirement);

  // [Nugget] /===============================================================

  // Save milestones
  CheckSaveGame(sizeof(complete_milestones));
  saveg_write_enum(complete_milestones);

  // Save custom-skill settings ----------------------------------------------

  CheckSaveGame(sizeof(customskill));

  saveg_write32(customskill.things);
  saveg_write32(customskill.coopspawns);
  saveg_write32(customskill.nomonsters);
  saveg_write32(customskill.doubleammo);
  saveg_write32(customskill.halfdamage);
  saveg_write32(customskill.slowbrain);
  saveg_write32(customskill.fast);
  saveg_write32(customskill.respawn);
  saveg_write32(customskill.aggressive);
  saveg_write32(customskill.x2monsters);

  CheckSaveGame(sizeof(initial_loadout));

  saveg_write32(initial_loadout.mohealth);
  saveg_write32(initial_loadout.health);
  saveg_write32(initial_loadout.armorpoints);
  saveg_write32(initial_loadout.armortype);
  saveg_write32(initial_loadout.backpack);
  saveg_write_enum(initial_loadout.readyweapon);
  saveg_write_enum(initial_loadout.lastweapon);

  for (int i = 0;  i < NUMWEAPONS;  i++)
  { saveg_write32(initial_loadout.weaponowned[i]); }

  for (int i = 0;  i < NUMAMMO;  i++)
  { saveg_write32(initial_loadout.ammo[i]); }

  for (int i = 0;  i < NUMAMMO;  i++)
  { saveg_write32(initial_loadout.maxammo[i]); }

  // [Nugget] ===============================================================/

  keyframe_rw = false;

  length = save_p - savebuffer;

  if (!keyframe_list_head)
  {
    keyframe_list_head =
    keyframe_list_tail = Z_Malloc(sizeof(keyframe_t), PU_STATIC, NULL);

    keyframe_list_tail->prev = NULL;
  }
  else
  {
    keyframe_list_tail->next = Z_Malloc(sizeof(keyframe_t), PU_STATIC, NULL);

    keyframe_list_tail->next->prev = keyframe_list_tail;
    keyframe_list_tail = keyframe_list_tail->next;
  }

  keyframe_list_tail->next = NULL;

  keyframe_list_tail->frame = Z_Malloc(length, PU_STATIC, NULL);

  memcpy(keyframe_list_tail->frame, savebuffer, length);

  keyframe_list_tail->length = length;

  if (rewind_depth == ++keyframe_index)
  {
    Z_Free(keyframe_list_head->frame);

    keyframe_list_head = keyframe_list_head->next;
    Z_Free(keyframe_list_head->prev);

    keyframe_list_head->prev = NULL;

    keyframe_index--;
  }

  if (rewind_timeout && (rewind_timeout < (I_GetTimeMS() - start_time)))
  {
    displaymsg("Slow key-framing: storing stopped");
    rewind_on = false;
  }

  G_SetRewindCountdown(rewind_interval * TICRATE);

  Z_Free(savebuffer);
  savebuffer = save_p = NULL;
}

static void G_DoRewind(void)
{
  static int last_rewind_time = 0;

  if ((0 <= keyframe_index - 1)
      && (gametic - last_rewind_time <= 21)) // 0.6 seconds
  {
    keyframe_index--;

    Z_Free(keyframe_list_tail->frame);

    keyframe_list_tail = keyframe_list_tail->prev;
    Z_Free(keyframe_list_tail->next);
    keyframe_list_tail->next = NULL;
  }

  last_rewind_time = gametic;

  int length, i;

  I_SetFastdemoTimer(false);

  // [crispy] loaded game must always be single player.
  // Needed for ability to use a further game loading, as well as
  // cheat codes and other single player only specifics.
  netdemo = false;
  netgame = false;
  deathmatch = false;

  length = keyframe_list_tail->length;
  save_p = savebuffer = keyframe_list_tail->frame;

  saveg_compat = saveg_current;

  keyframe_rw = true;

  demo_version = *save_p++; // saveg_woof510 < saveg_compat

  compatibility = *save_p++; // killough 2/14/98: load compatibility mode
  gameskill = *save_p++;
  gameepisode = *save_p++;
  gamemap = *save_p++;

  gamemapinfo = G_LookupMapinfo(gameepisode, gamemap);

  for (i=0 ; i<MAXPLAYERS ; i++)
    playeringame[i] = *save_p++;

  save_p += MIN_MAXPLAYERS-MAXPLAYERS;         // killough 2/28/98

  // jff 3/17/98 restore idmus music
  // jff 3/18/98 account for unsigned byte
  // killough 11/98: simplify
  idmusnum = *(signed char *) save_p++;

  /* cph 2001/05/23 - Must read options before we set up the level */
  if (mbf21)
    G_ReadOptionsMBF21(save_p);
  else
    G_ReadOptions(save_p);

  G_InitNew(gameskill, gameepisode, gamemap); // load a base level

  // killough 3/1/98: Read game options
  // killough 11/98: move down to here
  /* cph - MBF needs to reread the savegame options because G_InitNew
   * rereads the WAD options. The demo playback code does this too. */
  if (mbf21)
    save_p = G_ReadOptionsMBF21(save_p);
  else
    save_p = G_ReadOptions(save_p);

  // get the times
  // killough 11/98: save entire word
  // [FG] fix copy size and pointer progression
  leveltime = saveg_read32();

  // killough 11/98: load revenant tracer state
  basetic = gametic - (int) *save_p++;

  // dearchive all the modifications
  P_MapStart();
  P_UnArchivePlayers();
  P_UnArchiveWorld();
  P_UnArchiveThinkers();
  P_UnArchiveSpecials();
  P_UnArchiveRNG();    // killough 1/18/98: load RNG information
  P_UnArchiveMap();    // killough 1/22/98: load automap information
  P_MapEnd();

  if (*save_p != 0xe6) { I_Error("G_DoRewind: Bad key frame."); }

  // [FG] restore total time for all completed levels
  if (save_p++ - savebuffer < length - sizeof totalleveltimes)
  { totalleveltimes = saveg_read32(); }

  // restore MUSINFO music
  if (save_p - savebuffer <= length - 8)
  {
    char lump[9] = {0};
    int i;

    memcpy(lump, save_p, 8);

    i = W_CheckNumForName(lump);

    if (lump[0] && i > 0) {
      musinfo.mapthing = NULL;
      musinfo.lastmapthing = NULL;
      musinfo.tics = 0;
      musinfo.current_item = i;
      musinfo.from_savegame = true;
      S_ChangeMusInfoMusic(i, true);
    }

    save_p += 8;
  }

  if (save_p - savebuffer <= length - sizeof(max_kill_requirement))
  {
    max_kill_requirement = saveg_read32();
  }

  // [Nugget] Restore milestones
  if ((save_p - savebuffer) <= (length - sizeof(complete_milestones)))
  { complete_milestones = saveg_read_enum(); }

  // [Nugget] Restore custom-skill settings
  if ((save_p - savebuffer) <= (length - sizeof(customskill)))
  {
    customskill.things     = saveg_read32();
    customskill.coopspawns = saveg_read32();
    customskill.nomonsters = saveg_read32();
    customskill.doubleammo = saveg_read32();
    customskill.halfdamage = saveg_read32();
    customskill.slowbrain  = saveg_read32();
    customskill.fast       = saveg_read32();
    customskill.respawn    = saveg_read32();
    customskill.aggressive = saveg_read32();
    customskill.x2monsters = saveg_read32();

    if (gameskill == sk_custom) { G_SetSkillParms(sk_custom); }

    if ((save_p - savebuffer) <= (length - sizeof(initial_loadout)))
    {
      initial_loadout.mohealth    = saveg_read32();
      initial_loadout.health      = saveg_read32();
      initial_loadout.armorpoints = saveg_read32();
      initial_loadout.armortype   = saveg_read32();
      initial_loadout.backpack    = saveg_read32();
      initial_loadout.readyweapon = saveg_read_enum();
      initial_loadout.lastweapon  = saveg_read_enum();

      for (int i = 0;  i < NUMWEAPONS;  i++)
      { initial_loadout.weaponowned[i] = saveg_read32(); }

      for (int i = 0;  i < NUMAMMO;  i++)
      { initial_loadout.ammo[i] = saveg_read32(); }

      for (int i = 0;  i < NUMAMMO;  i++)
      { initial_loadout.maxammo[i] = saveg_read32(); }
    }
  }

  keyframe_rw = false;

  if (setsizeneeded) { R_ExecuteSetViewSize(); }

  R_FillBackScreen(); // draw the pattern into the back screen

  displaymsg("Restored key frame %i", keyframe_index);

  // This is called before the countdown decrement in `G_Ticker()`,
  // so add 1 to keep it aligned
  G_SetRewindCountdown((rewind_interval * TICRATE) + 1);

  ST_Start();
}

void G_EnableRewind(void)
{
  rewind_on = true;
}

void G_Rewind(void)
{
  if (!casual_play) { return; }

  G_EnableRewind();

  if (0 <= keyframe_index)
  { gameaction = ga_rewind; }
  else
  { displaymsg("No key frame found"); }
}

void G_ClearExcessKeyFrames(void)
{
  while (rewind_depth <= keyframe_index)
  {
    Z_Free(keyframe_list_head->frame);

    if (keyframe_list_head->next)
    {
      keyframe_list_head = keyframe_list_head->next;
      Z_Free(keyframe_list_head->prev);

      keyframe_list_head->prev = NULL;
    }
    else {
      Z_Free(keyframe_list_head);
      keyframe_list_head = keyframe_list_tail = NULL;
    }

    keyframe_index--;
  }
}

boolean G_KeyFrameRW(void)
{
  return keyframe_rw;
}

// [Nugget] -----------------------------------------------------------------/

boolean clean_screenshot;

screenshotpalette_t screenshot_palette; // [Nugget]

void G_CleanScreenshot(void)
{
  int old_screenblocks;
  boolean old_hide_weapon;

  if (!(screenshot_palette & SHOTPAL_CLEAN)) // [Nugget]
    ST_ResetPalette();

  if (gamestate != GS_LEVEL)
      return;

  old_screenblocks = screenblocks;
  old_hide_weapon = hide_weapon;
  hide_weapon = true;
  R_SetViewSize(11);
  R_ExecuteSetViewSize();
  R_RenderPlayerView(&players[displayplayer]);
  R_SetViewSize(old_screenblocks);
  hide_weapon = old_hide_weapon;
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//

void G_Ticker(void)
{
  int i;

  // do player reborns if needed
  P_MapStart();
  for (i=0 ; i<MAXPLAYERS ; i++)
    if (playeringame[i] && players[i].playerstate == PST_REBORN)
      G_DoReborn (i);
  P_MapEnd();

  // do things to change the game state
  while (gameaction != ga_nothing)
    switch (gameaction)
      {
      case ga_loadlevel:
	G_DoLoadLevel();
	break;
      case ga_newgame:
	G_DoNewGame();
	break;
      case ga_loadgame:
	G_DoLoadGame();
	break;
      case ga_savegame:
	G_DoSaveGame();
	break;
      case ga_playdemo:
	G_DoPlayDemo();
	break;
      case ga_completed:
	G_DoCompleted();
	break;
      case ga_victory:
	F_StartFinale();
	break;
      case ga_worlddone:
	G_DoWorldDone();
	break;
      case ga_screenshot:
	if (clean_screenshot)
	{
	  G_CleanScreenshot();
	  clean_screenshot = false;
	}
	// [Nugget]
	else if (!(screenshot_palette & SHOTPAL_NORMAL))
	{
	  ST_ResetPalette();
	}

	V_ScreenShot();
	gameaction = ga_nothing;
	break;
      case ga_reloadlevel:
	G_ReloadLevel();
	break;
      case ga_loadautosave:
	G_DoLoadAutoSave();
	break;
      case ga_saveautosave:
	G_DoSaveAutoSave();
	break;
      // [Nugget] Rewind
      case ga_rewind:
	G_DoRewind();
	break;
      default:  // killough 9/29/98
	gameaction = ga_nothing;
	break;
    }

  // [Nugget] Periodic auto save
  if (CASUALPLAY(autosave && autosave_interval)
      && gamestate == GS_LEVEL && oldleveltime < leveltime
      && players[consoleplayer].playerstate != PST_DEAD)
  {
    if (--autosave_countdown <= 0)
    {
      saving_periodic_autosave = true;

      M_SaveAutoSave();
      save_autosave = false;
      G_DoSaveAutoSave();

      saving_periodic_autosave = false;
    }
  }

  CheckSaveAutoSave();

  // [Nugget] Rewind
  if (CASUALPLAY(rewind_depth && rewind_on)
      && gamestate == GS_LEVEL && oldleveltime < leveltime
      && players[consoleplayer].playerstate != PST_DEAD)
  {
    if (--rewind_countdown <= 0)
    { G_SaveKeyFrame(); }
  }
  else if (!CASUALPLAY(rewind_depth) || gamestate != GS_LEVEL)
  {
    rewind_countdown = 0;
  }

  // killough 10/6/98: allow games to be saved during demo
  // playback, by the playback user (not by demo itself)

  if (demoplayback && sendsave)
    {
      sendsave = false;
      gameaction = ga_savegame;
    }

  // killough 9/29/98: Skip some commands while pausing during demo
  // playback, or while menu is active.
  //
  // We increment basetic and skip processing if a demo being played
  // back is paused or if the menu is active while a non-net game is
  // being played, to maintain sync while allowing pauses.
  //
  // P_Ticker() does not stop netgames if a menu is activated, so
  // we do not need to stop if a menu is pulled up during netgames.

  if (paused & 2 || (!demoplayback && menuactive && !netgame))
    basetic++;  // For revenant tracers and RNG -- we must maintain sync
  else
    {
      // get commands, check consistancy, and build new consistancy check
      int buf = (gametic/ticdup)%BACKUPTICS;

      for (i=0 ; i<MAXPLAYERS ; i++)
	{
	  if (playeringame[i])
	    {
	      ticcmd_t *cmd = &players[i].cmd;

	      memcpy(cmd, &netcmds[i], sizeof *cmd);

	      // catch BT_JOIN before G_ReadDemoTiccmd overwrites it
	      if (demoplayback && cmd->buttons & BT_JOIN)
		G_JoinDemo();

	      // catch BTS_RELOAD for demo playback restart
	      if (demoplayback &&
	          cmd->buttons & BT_SPECIAL && cmd->buttons & BT_SPECIALMASK &&
	          cmd->buttons & BTS_RELOAD)
	      {
	        playback_tic = 0;
	        gameaction = ga_playdemo;
	      }

	      if (demoplayback)
		G_ReadDemoTiccmd(cmd);

	      // [crispy] do not record tics while still playing back in demo
	      // continue mode
	      if (demorecording && !demoplayback)
		G_WriteDemoTiccmd(cmd);

	      // check for turbo cheats
	      // killough 2/14/98, 2/20/98 -- only warn in netgames and demos

	      if ((netgame || demoplayback) &&
		  cmd->forwardmove > TURBOTHRESHOLD &&
		  !(gametic&31) && ((gametic>>5)&3) == i )
		{
		  displaymsg("%s is turbo!", *player_names[i]); // killough 9/29/98
		}

	      if (netgame && !netdemo && !(gametic%ticdup) )
		{
		  if (gametic > BACKUPTICS
		      && consistancy[i][buf] != cmd->consistancy)
		    I_Error ("consistency failure (%i should be %i)",
			     cmd->consistancy, consistancy[i][buf]);
		  if (players[i].mo)
		    consistancy[i][buf] = players[i].mo->x;
		  else
		    consistancy[i][buf] = 0; // killough 2/14/98
		}
	    }
	}

      if (demoplayback)
        ++playback_tic;

      HU_UpdateCommandHistory(&players[displayplayer].cmd);

      // check for special buttons
      for (i=0; i<MAXPLAYERS; i++)
	if (playeringame[i] && players[i].cmd.buttons & BT_SPECIAL)
	  switch (players[i].cmd.buttons & BT_SPECIALMASK)
	  {
	    case BTS_RELOAD:
	      if (!demoplayback) // ignore in demos
	      {
	        gameaction = ga_reloadlevel;
	        if (demorecording)
	        {
	          quickstart_queued = true;
	        }
	      }
	      break;

	    case BTS_PAUSE:
	      if ((paused ^= 1))
		S_PauseSound();
	      else
		S_ResumeSound();
	      break;

	    case BTS_SAVEGAME:
		if (!savedescription[0])
		  strcpy(savedescription, "NET GAME");
		savegameslot =
		  (players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT;
		gameaction = ga_savegame;
	      break;

	    default:
	      break;
	  }
    }

  // [Nugget] /===============================================================

  // Zoom --------------------------------------------------------------------

  static boolean zoomKeyDown = false;

  if (!M_InputGameActive(input_zoom)
      || !(gamestate == GS_LEVEL || gamestate == GS_DEMOSCREEN))
  {
    zoomKeyDown = false;
  }
  else if (STRICTMODE(!zoomKeyDown))
  {
    zoomKeyDown = true;
    R_SetZoom(!R_GetZoom());
  }

  // Slow Motion -------------------------------------------------------------

  boolean change = false;

  if (slow_motion && slow_motion_factor != SLOWMO_FACTOR_TARGET)
  {
    slow_motion_factor -= MAX(MIN_SLOWMO_STEP, (slow_motion_factor - SLOWMO_FACTOR_TARGET) / 4);
    slow_motion_factor  = MAX(SLOWMO_FACTOR_TARGET, slow_motion_factor);
    change = true;
  }
  else if (!slow_motion && slow_motion_factor != SLOWMO_FACTOR_NORMAL)
  {
    slow_motion_factor += MAX(MIN_SLOWMO_STEP, (SLOWMO_FACTOR_NORMAL - slow_motion_factor) / 4);
    slow_motion_factor  = MIN(SLOWMO_FACTOR_NORMAL, slow_motion_factor);
    change = true;
  }

  static boolean oldmenuactive = false;

  if (oldmenuactive != menuactive)
  {
    oldmenuactive = menuactive;
    change = true;
  }

  if (change) {
    G_SetTimeScale();
    setrefreshneeded = false;
  }

  // Freecam -----------------------------------------------------------------

  if (casual_play && R_GetFreecamMode() == FREECAM_CAM && gamestate == GS_LEVEL)
  { memset(&players[consoleplayer].cmd, 0, sizeof(ticcmd_t)); }

  // [Nugget] ===============================================================/

  oldleveltime = leveltime;

  // do main actions

  // killough 9/29/98: split up switch statement
  // into pauseable and unpauseable parts.

  gamestate == GS_LEVEL ? P_Ticker(), ST_Ticker(), AM_Ticker() :
    paused & 2 ? (void) 0 :
      gamestate == GS_INTERMISSION ? WI_Ticker() :
	gamestate == GS_FINALE ? F_Ticker() :
	  gamestate == GS_DEMOSCREEN ? D_PageTicker() : (void) 0;

  // [Nugget] Freecam
  if (R_FreecamOn())
  {
    fixed_t x = 0,
            y = 0,
            z = 0;
    angle_t angle = 0,
            ticangle = 0;
    fixed_t pitch = 0;
    boolean center = false,
            lock = false;

    if (R_GetFreecamMode() == FREECAM_CAM && gamestate == GS_LEVEL && !menuactive)
    {
      const ticcmd_t *const cmd = &netcmds[consoleplayer];

      #define INPUT(input) M_InputGameActive(input)

      static boolean usedown = false, firedown = false;

      if (!INPUT(input_use))  {  usedown = false; }
      if (!INPUT(input_fire)) { firedown = false; }

      if (INPUT(input_use) && !usedown)
      {
        usedown = true;
        R_ResetFreecam(false);
      }
      else if (INPUT(input_fire) && !firedown)
      {
        firedown = true;
        lock = true;
      }
      else {
        if (!R_GetFreecamMobj() || R_ChasecamOn())
        {
          angle = cmd->angleturn << 16;
          ticangle = cmd->ticangleturn << FRACBITS;
        }

        if (!R_GetFreecamMobj())
        {
          static fixed_t basespeed = 8*FRACUNIT;
          const int speedchange = INPUT(input_nextweapon) - INPUT(input_prevweapon);

          if (speedchange)
          {
            basespeed = BETWEEN(FRACUNIT, 40*FRACUNIT, basespeed + (FRACUNIT * speedchange));

            const int scaledspeed = basespeed / FRACUNIT;
            displaymsg("Freecam Speed: %i unit%s", scaledspeed, (scaledspeed == 1) ? "" : "s");
          }

          fixed_t speed = basespeed * (1 + (autorun ^ INPUT(input_speed))) * 100 / realtic_clock_rate;

          fixed_t forwardmove = speed * (INPUT(input_forward)     - INPUT(input_backward)),
                  sidemove    = speed * (INPUT(input_straferight) - INPUT(input_strafeleft));

          if (flip_levels) { sidemove = -sidemove; } // Flip levels

          angle_t fangle = R_GetFreecamAngle() + angle;

          x = FixedMul(forwardmove, finecosine[ fangle          >> ANGLETOFINESHIFT])
            + FixedMul(sidemove,    finecosine[(fangle - ANG90) >> ANGLETOFINESHIFT]);

          y = FixedMul(forwardmove, finesine[ fangle          >> ANGLETOFINESHIFT])
            + FixedMul(sidemove,    finesine[(fangle - ANG90) >> ANGLETOFINESHIFT]);

          z = speed * (INPUT(input_jump) - INPUT(input_crouch));
        }

        pitch = cmd->pitch << FRACBITS;

        static int strafetic = -10;
        static boolean strafedown = false;

        if (!INPUT(input_strafe))
        {
          strafedown = false;
        }
        else if (!strafedown)
        {
          strafedown = true;

          if (gametic - strafetic < 10)
          {
            center = true;
          }

          strafetic = gametic;
        }
      }

      #undef INPUT
    }

    R_UpdateFreecam(x, y, z, angle, ticangle, pitch, center, lock);
  }
}

//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//

void G_PlayerReborn(int player)
{
  player_t *p;
  int i;
  int frags[MAXPLAYERS];
  int killcount;
  int itemcount;
  int secretcount;
  int maxkilldiscount;

  memcpy (frags, players[player].frags, sizeof frags);
  killcount = players[player].killcount;
  itemcount = players[player].itemcount;
  secretcount = players[player].secretcount;
  maxkilldiscount = players[player].maxkilldiscount;

  p = &players[player];

  // killough 3/10/98,3/21/98: preserve cheats across idclev
  {
    int cheats = p->cheats;
    memset (p, 0, sizeof(*p));
    p->cheats = cheats;
  }

  memcpy(players[player].frags, frags, sizeof(players[player].frags));
  players[player].killcount = killcount;
  players[player].itemcount = itemcount;
  players[player].secretcount = secretcount;
  players[player].maxkilldiscount = maxkilldiscount;

  p->usedown = p->attackdown = true;  // don't do anything immediately
  p->playerstate = PST_LIVE;
  p->health = initial_health;  // Ty 03/12/98 - use dehacked values
  p->lastweapon = wp_fist;
  p->nextweapon = p->readyweapon = p->pendingweapon = wp_pistol;
  p->weaponowned[wp_fist] = true;
  p->weaponowned[wp_pistol] = true;
  p->ammo[am_clip] = initial_bullets; // Ty 03/12/98 - use dehacked values

  for (i=0 ; i<NUMAMMO ; i++)
    p->maxammo[i] = maxammo[i];
}

//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot
// because something is occupying it
//

static boolean G_CheckSpot(int playernum, mapthing_t *mthing)
{
  fixed_t     x,y;
  subsector_t *ss;
  mobj_t      *mo;
  int         i;

  if (!players[playernum].mo)
    {
      // first spawn of level, before corpses
      for (i=0 ; i<playernum ; i++)
        if (players[i].mo->x == mthing->x << FRACBITS
            && players[i].mo->y == mthing->y << FRACBITS)
          return false;
      return true;
    }

  x = mthing->x << FRACBITS;
  y = mthing->y << FRACBITS;

  // killough 4/2/98: fix bug where P_CheckPosition() uses a non-solid
  // corpse to detect collisions with other players in DM starts
  //
  // Old code:
  // if (!P_CheckPosition (players[playernum].mo, x, y))
  //    return false;

  players[playernum].mo->flags |=  MF_SOLID;
  i = P_CheckPosition(players[playernum].mo, x, y);
  players[playernum].mo->flags &= ~MF_SOLID;
  if (!i)
    return false;

  // flush an old corpse if needed
  // killough 2/8/98: make corpse queue have an adjustable limit
  // killough 8/1/98: Fix bugs causing strange crashes

  if (bodyquesize > 0)
    {
      static mobj_t **bodyque;
      static size_t queuesize;
      if (queuesize < bodyquesize)
	{
	  bodyque = Z_Realloc(bodyque, bodyquesize*sizeof*bodyque, PU_STATIC, 0);
	  memset(bodyque+queuesize, 0,
		 (bodyquesize-queuesize)*sizeof*bodyque);
	  queuesize = bodyquesize;
	}
      if (bodyqueslot >= bodyquesize)
	P_RemoveMobj(bodyque[bodyqueslot % bodyquesize]);
      bodyque[bodyqueslot++ % bodyquesize] = players[playernum].mo;
    }
  else
    if (!bodyquesize)
      P_RemoveMobj(players[playernum].mo);

  // spawn a teleport fog
  ss = R_PointInSubsector (x,y);

  // The code in the released source looks like this:
  //
  //    an = ( ANG45 * (((unsigned int) mthing->angle)/45) )
  //         >> ANGLETOFINESHIFT;
  //    mo = P_SpawnMobj (x+20*finecosine[an], y+20*finesine[an]
  //                     , ss->sector->floorheight
  //                     , MT_TFOG);
  //
  // But 'an' can be a signed value in the DOS version. This means that
  // we get a negative index and the lookups into finecosine/finesine
  // end up dereferencing values in finetangent[].
  // A player spawning on a deathmatch start facing directly west spawns
  // "silently" with no spawn fog. Emulate this.
  //
  // This code is imported from PrBoom+.

  {
    fixed_t xa, ya;
    signed int an;

    // This calculation overflows in Vanilla Doom, but here we deliberately
    // avoid integer overflow as it is undefined behavior, so the value of
    // 'an' will always be positive.
    an = (ANG45 >> ANGLETOFINESHIFT) * ((signed int) mthing->angle / 45);

    if (demo_compatibility)
      switch (an)
      {
        case 4096:  // -4096:
            xa = finetangent[2048];    // finecosine[-4096]
            ya = finetangent[0];       // finesine[-4096]
            break;
        case 5120:  // -3072:
            xa = finetangent[3072];    // finecosine[-3072]
            ya = finetangent[1024];    // finesine[-3072]
            break;
        case 6144:  // -2048:
            xa = finesine[0];          // finecosine[-2048]
            ya = finetangent[2048];    // finesine[-2048]
            break;
        case 7168:  // -1024:
            xa = finesine[1024];       // finecosine[-1024]
            ya = finetangent[3072];    // finesine[-1024]
            break;
        case 0:
        case 1024:
        case 2048:
        case 3072:
            xa = finecosine[an];
            ya = finesine[an];
            break;
        default:
            I_Error("G_CheckSpot: unexpected angle %d\n", an);
            xa = ya = 0;
            break;
      }
    else
    {
      xa = finecosine[an];
      ya = finesine[an];
    }
    mo = P_SpawnMobj(x + 20 * xa, y + 20 * ya,
                     ss->sector->floorheight, MT_TFOG);
  }

  // [Nugget] Teleporter zoom
  if (&players[playernum] == &players[displayplayer])
  { R_SetFOVFX(FOVFX_TELEPORT); }

  if (players[consoleplayer].viewz != 1) // don't start sound on first frame
    S_StartSoundSource(players[consoleplayer].mo, mo, sfx_telept);

  return true;
}

//
// G_DeathMatchSpawnPlayer
// Spawns a player at one of the random death match spots
// called at level load and each death
//

void G_DeathMatchSpawnPlayer(int playernum)
{
  int j, selections = deathmatch_p - deathmatchstarts;

  if (selections < MAXPLAYERS)
    I_Error("Only %i deathmatch spots, %d required", selections, MAXPLAYERS);

  for (j=0 ; j<20 ; j++)
    {
      int i = P_Random(pr_dmspawn) % selections;
      if (G_CheckSpot(playernum, &deathmatchstarts[i]) )
        {
          deathmatchstarts[i].type = playernum+1;
          P_SpawnPlayer (&deathmatchstarts[i]);
          return;
        }
    }

  // no good spot, so the player will probably get stuck
  P_SpawnPlayer (&playerstarts[playernum]);
}

//
// G_DoReborn
//

void G_DoReborn(int playernum)
{
  if (!netgame)
  {
    if (gameaction != ga_reloadlevel)
      gameaction = ga_loadlevel;      // reload the level from scratch
  }
  else
    { // respawn at the start
      int i;

      // first dissasociate the corpse
      players[playernum].mo->player = NULL;

      // spawn at random spot if in death match
      if (deathmatch)
        {
          G_DeathMatchSpawnPlayer (playernum);
          return;
        }

      if (G_CheckSpot (playernum, &playerstarts[playernum]) )
        {
          P_SpawnPlayer (&playerstarts[playernum]);
          return;
        }

      // try to spawn at one of the other players spots
      for (i=0 ; i<MAXPLAYERS ; i++)
        {
          if (G_CheckSpot (playernum, &playerstarts[i]) )
            {
              playerstarts[i].type = playernum+1; // fake as other player
              P_SpawnPlayer (&playerstarts[i]);
              playerstarts[i].type = i+1;   // restore
              return;
            }
          // he's going to be inside something.  Too bad.
        }
      P_SpawnPlayer (&playerstarts[playernum]);
    }
}

void G_ScreenShot(void)
{
  gameaction = ga_screenshot;
}

// DOOM Par Times
int pars[4][10] = {
  {0},
  {0,30,75,120,90,165,180,180,30,165},
  {0,90,90,90,120,90,360,240,30,170},
  {0,90,45,90,150,90,90,165,30,135}
};

// DOOM II Par Times
int cpars[34] = {
  30,90,120,120,90,150,120,120,270,90,  //  1-10
  210,150,150,150,210,150,420,150,210,150,  // 11-20
  240,150,180,150,150,300,330,420,300,180,  // 21-30
  120,30,30,30          // 31-34
};

//
// G_WorldDone
//

void G_WorldDone(void)
{
  gameaction = ga_worlddone;

  if (secretexit)
    players[consoleplayer].didsecret = true;

  if (gamemapinfo)
  {
      if (gamemapinfo->flags & MapInfo_InterTextClear
          && gamemapinfo->flags & MapInfo_EndGame)
      {
          I_Printf(VB_DEBUG,
              "UMAPINFO: 'intertext = clear' with one of the end game keys.");
      }

      if (secretexit)
      {
          if (gamemapinfo->flags & MapInfo_InterTextSecretClear)
          {
              return;
          }
          if (gamemapinfo->intertextsecret)
          {
              F_StartFinale();
              return;
          }
      }
      else
      {
          if (gamemapinfo->flags & MapInfo_EndGame)
          {
              // game ends without a status screen.
              gameaction = ga_victory;
              return;
          }
          else if (gamemapinfo->flags & MapInfo_InterTextClear)
          {
              return;
          }
          else if (gamemapinfo->intertext)
          {
              F_StartFinale();
              return;
          }
      }
      // if nothing applied, use the defaults.
  }

  if (gamemode == commercial)
    {
      switch (gamemap)
        {
        case 15:
        case 31:
          if (!secretexit)
            break;
        case 6:
        case 11:
        case 20:
        case 30:
          F_StartFinale();
          break;
        }
    }
}

static skill_t d_skill;
static int     d_episode;
static int     d_map;

void G_DeferedInitNew(skill_t skill, int episode, int map)
{
  d_skill = skill;
  d_episode = episode;
  d_map = map;
  gameaction = ga_newgame;
  musinfo.from_savegame = false;

  if (demorecording)
  {
    ddt_cheating = 0;
    G_CheckDemoStatus();
    G_RecordDemo(demoname_orig);
  }
}

// killough 7/19/98: Marine's best friend :)
static int G_GetHelpers(void)
{
  //!
  // @category game
  //
  // Enables a single helper dog.
  //

  int j = M_CheckParm ("-dog");

  if (!j)

    //!
    // @arg <n>
    // @category game
    //
    // Overrides the current number of helper dogs, setting it to n.
    //

    j = M_CheckParm ("-dogs");
  return j ? (j+1 < myargc && myargv[j+1][0] != '-') ? M_ParmArgToInt(j) : 1 : default_dogs;
}

// [FG] support named complevels on the command line, e.g. "-complevel boom"

demo_version_t G_GetNamedComplevel(const char *arg)
{
    const struct
    {
        const char *const name;
        demo_version_t demover;
        int exe;
    } named_complevel[] = {
        {"vanilla",  DV_VANILLA, exe_indetermined},
        {"doom2",    DV_VANILLA, exe_doom_1_9    },
        {"1.9",      DV_VANILLA, exe_doom_1_9    },
        {"2",        DV_VANILLA, exe_doom_1_9    },
        {"ultimate", DV_VANILLA, exe_ultimate    },
        {"3",        DV_VANILLA, exe_ultimate    },
        {"final",    DV_VANILLA, exe_final       },
        {"tnt",      DV_VANILLA, exe_final       },
        {"plutonia", DV_VANILLA, exe_final       },
        {"4",        DV_VANILLA, exe_final       },
        {"boom",     DV_BOOM,    exe_indetermined},
        {"9",        DV_BOOM,    exe_indetermined},
        {"mbf",      DV_MBF,     exe_indetermined},
        {"11",       DV_MBF,     exe_indetermined},
        {"mbf21",    DV_MBF21,   exe_indetermined},
        {"21",       DV_MBF21,   exe_indetermined},
    };

    for (int i = 0; i < arrlen(named_complevel); i++)
    {
        if (!strcasecmp(arg, named_complevel[i].name))
        {
            if (named_complevel[i].exe != exe_indetermined)
            {
                gameversion = named_complevel[i].exe;
            }

            return named_complevel[i].demover;
        }
    }

    return DV_NONE;
}

static struct
{
    demo_version_t demover;
    complevel_t complevel;
} demover_complevel[] = {
    {DV_VANILLA, CL_VANILLA},
    {DV_BOOM,    CL_BOOM   },
    {DV_MBF,     CL_MBF    },
    {DV_MBF21,   CL_MBF21  }
};

static complevel_t GetComplevel(demo_version_t demover)
{
    for (int i = 0; i < arrlen(demover_complevel); ++i)
    {
        if (demover == demover_complevel[i].demover)
        {
            return demover_complevel[i].complevel;
        }
    }

    return CL_NONE;
}

static demo_version_t GetDemover(complevel_t complevel)
{
    for (int i = 0; i < arrlen(demover_complevel); ++i)
    {
        if (complevel == demover_complevel[i].complevel)
        {
            return demover_complevel[i].demover;
        }
    }

    return DV_NONE;
}

const char *G_GetCurrentComplevelName(void)
{
    switch (demo_version)
    {
        case DV_VANILLA:
            return gameversions[gameversion].description;
        case DV_BOOM:
            return "Boom";
        case DV_MBF:
            return "MBF";
        case DV_MBF21:
            return "MBF21";
        default:
            return "Unknown";
    }
}

static demo_version_t GetWadDemover(void)
{
    int lumpnum = W_CheckNumForName("COMPLVL");

    if (lumpnum < 0)
    {
        return DV_NONE;
    }

    int length = W_LumpLength(lumpnum);
    char *data = W_CacheLumpNum(lumpnum, PU_CACHE);

    if (length == 7 && !strncasecmp("vanilla", data, 7))
    {
        return DV_VANILLA;
    }
    else if (length == 4 && !strncasecmp("boom", data, 4))
    {
        return DV_BOOM;
    }
    else if (length == 3 && !strncasecmp("mbf", data, 3))
    {
        return DV_MBF;
    }
    else if (length == 5 && !strncasecmp("mbf21", data, 5))
    {
        return DV_MBF21;
    }

    return DV_NONE;
}

static void G_MBFDefaults(void)
{
  weapon_recoil = 0;
  monsters_remember = 1;
  monster_infighting = 1;
  monster_backing = 0;
  monster_avoid_hazards = 1;
  monkeys = 0;
  monster_friction = 1;
  help_friends = 0;
  dogs = 0;
  distfriend = 128;
  dog_jumping = 1;

  memset(comp, 0, sizeof comp);

  comp[comp_zombie] = 1;
}

static void G_MBF21Defaults(void)
{
  G_MBFDefaults();

  comp[comp_pursuit] = 1;

  comp[comp_respawn] = 0;
  comp[comp_soul] = 0;
  comp[comp_ledgeblock] = 1;
  comp[comp_friendlyspawn] = 1;
  comp[comp_voodooscroller] = 0;
  comp[comp_reservedlineflag] = 1;
}

static void G_MBFComp()
{
  comp[comp_respawn] = 1;
  comp[comp_soul] = 1;
  comp[comp_ledgeblock] = 0;
  comp[comp_friendlyspawn] = 1;
  comp[comp_voodooscroller] = 1;
  comp[comp_reservedlineflag] = 0;
}

static void G_BoomComp()
{
  comp[comp_telefrag] = 1;
  comp[comp_dropoff]  = 0;
  comp[comp_falloff]  = 1;
  comp[comp_skymap]   = 1;
  comp[comp_pursuit]  = 1;
  comp[comp_staylift] = 1;
  comp[comp_zombie]   = 1;
  comp[comp_infcheat] = 1;

  comp[comp_respawn] = 1;
  comp[comp_soul] = 1;
  comp[comp_ledgeblock] = 0;
  comp[comp_friendlyspawn] = 1;
  comp[comp_voodooscroller] = 0;
  comp[comp_reservedlineflag] = 0;
}

// killough 3/1/98: function to reload all the default parameter
// settings before a new game begins

void G_ReloadDefaults(boolean keep_demover)
{
  // killough 3/1/98: Initialize options based on config file
  // (allows functions above to load different values for demos
  // and savegames without messing up defaults).

  weapon_recoil = default_weapon_recoil;    // weapon recoil

  player_bobbing = default_player_bobbing;  // whether player bobs or not

  variable_friction = allow_pushers = true;

  monsters_remember = default_monsters_remember;   // remember former enemies

  monster_infighting = default_monster_infighting; // killough 7/19/98

  dogs = netgame ? 0 : G_GetHelpers();             // killough 7/19/98
  dog_jumping = default_dog_jumping;

  distfriend = default_distfriend;                 // killough 8/8/98

  monster_backing = default_monster_backing;     // killough 9/8/98

  monster_avoid_hazards = default_monster_avoid_hazards; // killough 9/9/98

  monster_friction = default_monster_friction;     // killough 10/98

  help_friends = default_help_friends;             // killough 9/9/98

  monkeys = default_monkeys;

  classic_bfg = default_classic_bfg;               // killough 7/19/98
  beta_emulation = !!M_CheckParm("-beta");         // killough 7/24/98

  // jff 1/24/98 reset play mode to command line spec'd version
  // killough 3/1/98: moved to here
  respawnparm = clrespawnparm;
  fastparm = clfastparm;
  nomonsters = clnomonsters;

  //jff 3/24/98 set startskill from defaultskill in config file, unless
  // it has already been set by a -skill parameter
  if (startskill == sk_default)
    startskill = (skill_t)(default_skill - 1);

  demoplayback = false;
  singledemo = false;            // killough 9/29/98: don't stop after 1 demo
  netdemo = false;

  // killough 2/21/98:
  memset(playeringame+1, 0, sizeof(*playeringame)*(MAXPLAYERS-1));

  consoleplayer = 0;

  compatibility = false;     // killough 10/98: replaced by comp[] vector
  memcpy(comp, default_comp, sizeof comp);

  if (!keep_demover)
  {
    demo_version_t demover = DV_NONE;

    //!
    // @arg <version>
    // @category compat
    // @help
    //
    // Emulate a specific version of Doom/Boom/MBF. Valid values are
    // "vanilla", "boom", "mbf", "mbf21".
    //

    int p = M_CheckParmWithArgs("-complevel", 1);

    if (!p)
    {
    //!
    // @arg <version>
    // @category compat
    // @help
    //
    // Alias to -complevel.
    //
      p = M_CheckParmWithArgs("-cl", 1);
    }

    if (p > 0)
    {
      demover = G_GetNamedComplevel(myargv[p + 1]);
      if (demover == DV_NONE)
      {
        I_Error("Invalid parameter '%s' for -complevel, "
                "valid values are vanilla, boom, mbf, mbf21.", myargv[p + 1]);
      }
    }

    if (demover == DV_NONE)
    {
      demover = GetWadDemover();
    }

    if (demover == DV_NONE)
    {
      demo_version = GetDemover(default_complevel);
      force_complevel = CL_NONE;
    }
    else
    {
      demo_version = demover;
      force_complevel = GetComplevel(demo_version);
    }
  }

  strictmode = default_strictmode;

  //!
  // @category demo
  // @help
  //
  // Sets compatibility and cosmetic settings according to DSDA rules.
  //

  if (M_CheckParm("-strict"))
  {
    strictmode = true;
    force_strictmode = true;
  }

  // [Nugget] /---------------------------------------------------------------

  static int old_strictmode = -1;

  if (old_strictmode == -1) { old_strictmode = strictmode; }

  if (old_strictmode != strictmode)
  {
    P_SegLengths(true); // Fake contrast

    old_strictmode = strictmode;
  }

  // [Nugget] ---------------------------------------------------------------/

  G_UpdateSideMove();

  pistolstart = default_pistolstart;

  //!
  // @category game
  // @help
  //
  // Enables automatic pistol starts on each level.
  //

  if (M_CheckParm("-pistolstart"))
    pistolstart = true;

  // Reset MBF compatibility options in strict mode
  if (strictmode)
  {
    if (demo_version == DV_MBF)
      G_MBFDefaults();
    else if (mbf21)
      G_MBF21Defaults();
  }

  D_SetMaxHealth();

  D_SetBloodColor();

  R_InvulMode();

  if (!mbf21)
  {
    // Set new compatibility options
    G_MBFComp();
  }

  // killough 3/31/98, 4/5/98: demo sync insurance
  demo_insurance = 0;

  // haleyjd
  rngseed = time(NULL);

  if (beta_emulation && demo_version != DV_MBF)
    I_Error("G_ReloadDefaults: Beta emulation requires complevel MBF.");

  if ((M_CheckParm("-dog") || M_CheckParm("-dogs")) && demo_version < DV_MBF)
    I_Error("G_ReloadDefaults: Helper dogs require complevel MBF or MBF21.");

  if (M_CheckParm("-skill") && startskill == sk_none && !demo_compatibility)
    I_Error("G_ReloadDefaults: '-skill 0' requires complevel Vanilla.");

  if (demo_version < DV_MBF)
  {
    monster_infighting = 1;
    monster_backing = 0;
    monster_avoid_hazards = 0;
    monster_friction = 0;
    help_friends = 0;

    dogs = 0;
    dog_jumping = 0;

    monkeys = 0;

    if (demo_version == DV_VANILLA)
    {
      compatibility = true;
      memset(comp, 0xff, sizeof comp);
    }
    else if (demo_version == DV_BOOM)
    {
      memset(comp, 0, sizeof comp);
      G_BoomComp();
    }
  }
  else if (mbf21)
  {
    // These are not configurable
    variable_friction = 1;
    allow_pushers = 1;
    classic_bfg = 0;
  }
}

void G_DoNewGame (void)
{
  I_SetFastdemoTimer(false);
  G_ReloadDefaults(false); // killough 3/1/98
  netgame = false;               // killough 3/29/98
  solonet = false;
  deathmatch = false;
  basetic = gametic;             // killough 9/29/98

  G_InitNew(d_skill, d_episode, d_map);
  gameaction = ga_nothing;

  if (demorecording)
    G_BeginRecording();
}

// killough 4/10/98: New function to fix bug which caused Doom
// lockups when idclev was used in conjunction with -fast.

void G_SetFastParms(int fast_pending)
{
  static int fast = 0;            // remembers fast state
  int i;

  if (fast != fast_pending)       // only change if necessary
  {
    for (i = 0; i < num_mobj_types; ++i)
      if (mobjinfo[i].altspeed != NO_ALTSPEED)
      {
        int swap = mobjinfo[i].speed;
        mobjinfo[i].speed = mobjinfo[i].altspeed;
        mobjinfo[i].altspeed = swap;
      }

    if ((fast = fast_pending))
    {
      for (i = 0; i < num_states; i++)
        if (states[i].flags & STATEF_SKILL5FAST && (states[i].tics != 1 || demo_compatibility))
          states[i].tics >>= 1;  // don't change 1->0 since it causes cycles
    }
    else
    {
      for (i = 0; i < num_states; i++)
        if (states[i].flags & STATEF_SKILL5FAST)
          states[i].tics <<= 1;
    }
  }
}

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set.

void G_InitNew(skill_t skill, int episode, int map)
{
  int i;

  if (paused)
    {
      paused = false;
      S_ResumeSound();
    }

  if (skill > sk_nightmare && skill != sk_custom) // [Nugget] Custom Skill
    skill = sk_nightmare;

  // [Nugget] Custom Skill
  if (skill == sk_custom && strictmode)
  { skill = (default_skill - 1 < sk_custom) ? default_skill - 1 : sk_hard; }

  if (episode < 1)
    episode = 1;

  // Disable all sanity checks if there are custom episode definitions. They do not make sense in this case.
  if (!EpiCustom && W_CheckNumForName(MapName(episode, map)) == -1)
  {

  if (gamemode == retail)
    {
      if (episode > 4)
        episode = 4;
    }
  else
    if (gamemode == shareware)
      {
        if (episode > 1)
          episode = 1; // only start episode 1 on shareware
      }
    else
      if (episode > 3)
        episode = 3;

  if (map < 1)
    map = 1;
  if (map > 9 && gamemode != commercial)
    map = 9;
  }

  M_ClearRandom();

  G_SetSkillParms(skill); // [Nugget]

  // force players to be initialized upon first level load
  for (i=0 ; i<MAXPLAYERS ; i++)
    players[i].playerstate = PST_REBORN;

  usergame = true;                // will be set false if a demo
  paused = false;
  demoplayback = false;
  automapactive = AM_OFF;
  viewactive = true;
  gameepisode = episode;
  gamemap = map;
  gameskill = skill;
  gamemapinfo = G_LookupMapinfo(gameepisode, gamemap);

  // [FG] total time for all completed levels
  totalleveltimes = 0;
  playback_tic = 0;

  HU_ResetCommandHistory();

  //jff 4/16/98 force marks on automap cleared every new level start
  AM_clearMarks();

  M_LoadOptions();     // killough 11/98: read OPTIONS lump from wad

  if (demo_version == DV_MBF)
    G_MBFComp();

  D_UpdateCasualPlay(); // [Nugget]

  G_DoLoadLevel();

  G_UpdateInitialLoadout(); // [Nugget] Custom Skill
}

//
// G_RecordDemo
//

void G_RecordDemo(const char *name)
{
  int i;

  // the original name of the demo, without "-00000" and file extension
  if (!demoname_orig)
  {
    demoname_orig = M_StringDuplicate(name);
    if (M_StringCaseEndsWith(demoname_orig, ".lmp"))
    {
      demoname_orig[strlen(demoname_orig) - 4] = '\0';
    }
  }

  demo_insurance = 0;
  usergame = false;

  // + 11 for "-00000.lmp\0"
  size_t demoname_size = strlen(demoname_orig) + 11;
  demoname = I_Realloc(demoname, demoname_size);
  M_snprintf(demoname, demoname_size, "%s.lmp", demoname_orig);

  // demo file name suffix counter
  static int j;
  while (M_access(demoname, F_OK) == 0)
  {
    M_snprintf(demoname, demoname_size, "%s-%05d.lmp", demoname_orig, j++);
  }

  //!
  // @arg <size>
  // @category demo
  // @vanilla
  //
  // Sets the initial size of the demo recording buffer (KiB). This is no longer a
  // hard limit, and the buffer will expand if the given limit is exceeded.
  //

  i = M_CheckParm ("-maxdemo");
  if (i && i<myargc-1)
    maxdemosize = M_ParmArgToInt(i) * 1024;
  if (maxdemosize < 0x20000)  // killough
    maxdemosize = 0x20000;
  demobuffer = Z_Malloc(maxdemosize, PU_STATIC, 0); // killough
  demorecording = true;
}

// These functions are used to read and write game-specific options in demos
// and savegames so that demo sync is preserved and savegame restoration is
// complete. Not all options (for example "compatibility"), however, should
// be loaded and saved here. It is extremely important to use the same
// positions as before for the variables, so if one becomes obsolete, the
// byte(s) should still be skipped over or padded with 0's.
// Lee Killough 3/1/98

static int G_GameOptionSize(void) {
  return mbf21 ? MBF21_GAME_OPTION_SIZE : GAME_OPTION_SIZE;
}

static byte* G_WriteOptionsMBF21(byte* demo_p)
{
  int i;
  byte *target = demo_p + MBF21_GAME_OPTION_SIZE;

  *demo_p++ = monsters_remember;
  *demo_p++ = weapon_recoil;
  *demo_p++ = player_bobbing;

  *demo_p++ = respawnparm;
  *demo_p++ = fastparm;
  *demo_p++ = nomonsters;

  *demo_p++ = (byte)((rngseed >> 24) & 0xff);
  *demo_p++ = (byte)((rngseed >> 16) & 0xff);
  *demo_p++ = (byte)((rngseed >>  8) & 0xff);
  *demo_p++ = (byte)( rngseed        & 0xff);

  *demo_p++ = monster_infighting;
  *demo_p++ = dogs;

  *demo_p++ = (distfriend >> 8) & 0xff;
  *demo_p++ =  distfriend       & 0xff;

  *demo_p++ = monster_backing;
  *demo_p++ = monster_avoid_hazards;
  *demo_p++ = monster_friction;
  *demo_p++ = help_friends;
  *demo_p++ = dog_jumping;
  *demo_p++ = monkeys;

  *demo_p++ = MBF21_COMP_TOTAL;

  for (i = 0; i < MBF21_COMP_TOTAL; i++)
    *demo_p++ = comp[i] != 0;

  if (demo_p != target)
    I_Error("mbf21_WriteOptions: MBF21_GAME_OPTION_SIZE is too small");

  return demo_p;
}

byte *G_WriteOptions(byte *demo_p)
{
  byte *target = demo_p + GAME_OPTION_SIZE;

  if (mbf21)
  {
    return G_WriteOptionsMBF21(demo_p);
  }

  *demo_p++ = monsters_remember;  // part of monster AI

  *demo_p++ = variable_friction;  // ice & mud

  *demo_p++ = weapon_recoil;      // weapon recoil

  *demo_p++ = allow_pushers;      // MT_PUSH Things

  *demo_p++ = 0;

  *demo_p++ = player_bobbing;  // whether player bobs or not

  // killough 3/6/98: add parameters to savegame, move around some in demos
  *demo_p++ = respawnparm;
  *demo_p++ = fastparm;
  *demo_p++ = nomonsters;

  *demo_p++ = demo_insurance;        // killough 3/31/98

  // killough 3/26/98: Added rngseed. 3/31/98: moved here
  *demo_p++ = (byte)((rngseed >> 24) & 0xff);
  *demo_p++ = (byte)((rngseed >> 16) & 0xff);
  *demo_p++ = (byte)((rngseed >>  8) & 0xff);
  *demo_p++ = (byte) (rngseed        & 0xff);

  // Options new to v2.03 begin here

  *demo_p++ = monster_infighting;   // killough 7/19/98

  *demo_p++ = dogs;                 // killough 7/19/98

  *demo_p++ = classic_bfg;          // killough 7/19/98
  *demo_p++ = beta_emulation;       // killough 7/24/98

  *demo_p++ = (distfriend >> 8) & 0xff;  // killough 8/8/98
  *demo_p++ =  distfriend       & 0xff;  // killough 8/8/98

  *demo_p++ = monster_backing;         // killough 9/8/98

  *demo_p++ = monster_avoid_hazards;    // killough 9/9/98

  *demo_p++ = monster_friction;         // killough 10/98

  *demo_p++ = help_friends;             // killough 9/9/98

  *demo_p++ = dog_jumping;

  *demo_p++ = monkeys;

  {   // killough 10/98: a compatibility vector now
    int i;
    for (i=0; i < COMP_TOTAL; i++)
      *demo_p++ = comp[i] != 0;
  }

  //----------------
  // Padding at end
  //----------------
  while (demo_p < target)
    *demo_p++ = 0;

  if (demo_p != target)
    I_Error("G_WriteOptions: GAME_OPTION_SIZE is too small");

  return target;
}

// Same, but read instead of write

byte *G_ReadOptionsMBF21(byte *demo_p)
{
  int i, count;

  // not configurable in mbf21
  variable_friction = 1;
  allow_pushers = 1;
  demo_insurance = 0;
  classic_bfg = 0;
  beta_emulation = 0;

  monsters_remember = *demo_p++;
  weapon_recoil = *demo_p++;
  player_bobbing = *demo_p++;

  respawnparm = *demo_p++;
  fastparm = *demo_p++;
  nomonsters = *demo_p++;

  rngseed  = *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;

  monster_infighting = *demo_p++;
  dogs = *demo_p++;

  distfriend  = *demo_p++ << 8;
  distfriend += *demo_p++;

  monster_backing = *demo_p++;
  monster_avoid_hazards = *demo_p++;
  monster_friction = *demo_p++;
  help_friends = *demo_p++;
  dog_jumping = *demo_p++;
  monkeys = *demo_p++;

  count = *demo_p++;

  if (count > MBF21_COMP_TOTAL)
    I_Error("Encountered unknown mbf21 compatibility options!");

  for (i = 0; i < count; i++)
    comp[i] = *demo_p++;

  // comp_voodooscroller
  if (count < MBF21_COMP_TOTAL - 2)
    comp[comp_voodooscroller] = 1;

  // comp_reservedlineflag
  if (count < MBF21_COMP_TOTAL - 1)
    comp[comp_reservedlineflag] = 0;

  return demo_p;
}

byte *G_ReadOptions(byte *demo_p)
{
  byte *target = demo_p + GAME_OPTION_SIZE;

  monsters_remember = *demo_p++;

  variable_friction = *demo_p;  // ice & mud
  demo_p++;

  weapon_recoil = *demo_p;       // weapon recoil
  demo_p++;

  allow_pushers = *demo_p;      // MT_PUSH Things
  demo_p++;

  demo_p++;

  player_bobbing = *demo_p;     // whether player bobs or not
  demo_p++;

  // killough 3/6/98: add parameters to savegame, move from demo
  respawnparm = *demo_p++;
  fastparm = *demo_p++;
  nomonsters = *demo_p++;

  demo_insurance = *demo_p++;              // killough 3/31/98

  // killough 3/26/98: Added rngseed to demos; 3/31/98: moved here

  rngseed  = *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;

  // Options new to v2.03
  if (demo_version >= DV_MBF)
    {
      monster_infighting = *demo_p++;   // killough 7/19/98

      dogs = *demo_p++;                 // killough 7/19/98

      classic_bfg = *demo_p++;          // killough 7/19/98
      beta_emulation = *demo_p++;       // killough 7/24/98

      if (beta_emulation && !M_CheckParm("-beta"))
	I_Error("The -beta option is required to play "
		"back beta emulation demos");

      distfriend = *demo_p++ << 8;      // killough 8/8/98
      distfriend+= *demo_p++;

      monster_backing = *demo_p++;     // killough 9/8/98

      monster_avoid_hazards = *demo_p++; // killough 9/9/98

      monster_friction = *demo_p++;      // killough 10/98

      help_friends = *demo_p++;          // killough 9/9/98

      dog_jumping = *demo_p++;           // killough 10/98

      monkeys = *demo_p++;

      {   // killough 10/98: a compatibility vector now
	int i;
	for (i=0; i < COMP_TOTAL; i++)
	  comp[i] = *demo_p++;
      }

      G_MBFComp();
    }
  else  // defaults for versions < 2.02
    {
      int i;  // killough 10/98: a compatibility vector now
      for (i=0; i < COMP_TOTAL; i++)
	comp[i] = compatibility;

      if (demo_version == DV_BOOM || demo_version == DV_BOOM201)
        G_BoomComp();

      monster_infighting = 1;           // killough 7/19/98

      monster_backing = 0;              // killough 9/8/98

      monster_avoid_hazards = 0;        // killough 9/9/98

      monster_friction = 0;             // killough 10/98

      help_friends = 0;                 // killough 9/9/98

      classic_bfg = 0;                  // killough 7/19/98
      beta_emulation = 0;               // killough 7/24/98

      dogs = 0;                         // killough 7/19/98
      dog_jumping = 0;                  // killough 10/98
      monkeys = 0;
    }

  return target;
}

void G_BeginRecording(void)
{
  int i;

  demo_p = demobuffer;

  if (demo_version == DV_MBF || mbf21)
  {
  *demo_p++ = demo_version;

  // signature
  *demo_p++ = 0x1d;
  *demo_p++ = 'M';
  *demo_p++ = 'B';
  *demo_p++ = 'F';
  *demo_p++ = 0xe6;
  *demo_p++ = '\0';

  if (!mbf21)
  {
  // killough 2/22/98: save compatibility flag in new demos
  *demo_p++ = compatibility;       // killough 2/22/98
  }

  *demo_p++ = gameskill;
  *demo_p++ = gameepisode;
  *demo_p++ = gamemap;
  *demo_p++ = deathmatch;
  *demo_p++ = consoleplayer;

  demo_p = G_WriteOptions(demo_p); // killough 3/1/98: Save game options

  for (i=0 ; i<MAXPLAYERS ; i++)
    *demo_p++ = playeringame[i];

  // killough 2/28/98:
  // We always store at least MIN_MAXPLAYERS bytes in demo, to
  // support enhancements later w/o losing demo compatibility

  for (; i<MIN_MAXPLAYERS; i++)
    *demo_p++ = 0;
  }
  else if (demo_version == DV_BOOM)
  {
    *demo_p++ = demo_version;

    // signature
    *demo_p++ = 0x1d;
    *demo_p++ = 'B';
    *demo_p++ = 'o';
    *demo_p++ = 'o';
    *demo_p++ = 'm';
    *demo_p++ = 0xe6;

    /* CPhipps - save compatibility level in demos */
    *demo_p++ = 0;

    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = deathmatch;
    *demo_p++ = consoleplayer;

    demo_p = G_WriteOptions(demo_p); // killough 3/1/98: Save game options

    for (i=0 ; i<MAXPLAYERS ; i++)
      *demo_p++ = playeringame[i];

    // killough 2/28/98:
    // We always store at least MIN_MAXPLAYERS bytes in demo, to
    // support enhancements later w/o losing demo compatibility

    for (; i<MIN_MAXPLAYERS; i++)
      *demo_p++ = 0;
  }
  else if (demo_version == DV_VANILLA)
  {
    if (longtics)
    {
      *demo_p++ = DV_LONGTIC;
    }
    else
    {
      *demo_p++ = demo_version;
    }
    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = deathmatch;
    *demo_p++ = respawnparm;
    *demo_p++ = fastparm;
    *demo_p++ = nomonsters;
    *demo_p++ = consoleplayer;
    for (i=0; i<4; i++)  // intentionally hard-coded 4 -- killough
      *demo_p++ = playeringame[i];
  }

  displaymsg("Demo Recording: %s", M_BaseName(demoname));
}

//
// G_PlayDemo
//

void D_CheckNetPlaybackSkip(void);

void G_DeferedPlayDemo(const char* name)
{
  defdemoname = name;
  gameaction = ga_playdemo;

  D_CheckNetPlaybackSkip();

  // [FG] fast-forward demo to the desired map
  if (playback_warp >= 0 || playback_skiptics)
  {
    G_EnableWarp(true);
  }
}

#define DEMO_FOOTER_SEPARATOR "\n"
#define NUM_DEMO_FOOTER_LUMPS 4

static size_t WriteCmdLineLump(MEMFILE *stream)
{
  int i;
  char *tmp;
  boolean has_files = false;

  long pos = mem_ftell(stream);

  tmp = M_StringJoin("-iwad \"", M_BaseName(wadfiles[0]), "\"");
  mem_fputs(tmp, stream);
  free(tmp);

  for (i = 1; i < array_size(wadfiles); i++)
  {
    const char *basename = M_BaseName(wadfiles[i]);

    if (!strcasecmp("brghtmps.lmp", basename) ||
        !strcasecmp("woofhud.lmp", basename))
    {
      continue;
    }

    if (!has_files)
    {
      mem_fputs(" -file", stream);
      has_files = true;
    }

    tmp = M_StringJoin(" \"", basename, "\"");
    mem_fputs(tmp, stream);
    free(tmp);
  }

  if (dehfiles)
  {
    mem_fputs(" -deh", stream);
    for (i = 0; i < array_size(dehfiles); ++i)
    {
      tmp = M_StringJoin(" \"", M_BaseName(dehfiles[i]), "\"");
      mem_fputs(tmp, stream);
      free(tmp);
    }
  }

  if (demo_compatibility)
  {
    if (gameversion == exe_doom_1_9)
      mem_fputs(" -complevel 2", stream);
    else if (gameversion == exe_ultimate)
      mem_fputs(" -complevel 3", stream);
    else if (gameversion == exe_final)
      mem_fputs(" -complevel 4", stream);
  }

  if (coopspawnsparm)
  {
    mem_fputs(" -coop_spawns", stream);
  }

  if (M_CheckParm("-solo-net"))
  {
    mem_fputs(" -solo-net", stream);
  }

  for (i = 0; i < EMU_TOTAL; ++i)
  {
     if (overflow[i].triggered)
     {
        mem_fputs(" -", stream);
        mem_fputs(overflow[i].str, stream);
     }
  }

  return mem_ftell(stream) - pos;
}

static long WriteFileInfo(const char *name, size_t size, long filepos,
                          MEMFILE *stream)
{
  filelump_t fileinfo = { 0 };

  fileinfo.filepos = LONG(filepos);
  fileinfo.size = LONG(size);

  if (name)
    M_CopyLumpName(fileinfo.name, name);

  mem_fwrite(&fileinfo, 1, sizeof(fileinfo), stream);

  filepos += size;
  return filepos;
}

static void G_AddDemoFooter(void)
{
  byte *data;
  size_t size;

  MEMFILE *stream = mem_fopen_write();

  wadinfo_t header = { "PWAD" };
  header.numlumps = LONG(NUM_DEMO_FOOTER_LUMPS);
  mem_fwrite(&header, 1, sizeof(header), stream);

  mem_fputs(PROJECT_STRING, stream);
  mem_fputs(DEMO_FOOTER_SEPARATOR, stream);
  size = WriteCmdLineLump(stream);
  mem_fputs(DEMO_FOOTER_SEPARATOR, stream);

  header.infotableofs = LONG(mem_ftell(stream));
  mem_fseek(stream, 0, MEM_SEEK_SET);
  mem_fwrite(&header, 1, sizeof(header), stream);
  mem_fseek(stream, 0, MEM_SEEK_END);

  long filepos = sizeof(wadinfo_t);
  filepos = WriteFileInfo("PORTNAME", strlen(PROJECT_STRING), filepos, stream);
  filepos = WriteFileInfo(NULL, strlen(DEMO_FOOTER_SEPARATOR), filepos, stream);
  filepos = WriteFileInfo("CMDLINE", size, filepos, stream);
  WriteFileInfo(NULL, strlen(DEMO_FOOTER_SEPARATOR), filepos, stream);

  mem_get_buf(stream, (void **)&data, &size);

  CheckDemoBuffer(size);

  memcpy(demo_p, data, size);
  demo_p += size;

  mem_fclose(stream);
}

//===================
//=
//= G_CheckDemoStatus
//=
//= Called after a death or level completion to allow demos to be cleaned up
//= Returns true if a new demo loop action will take place
//===================

boolean G_CheckDemoStatus(void)
{
  // [crispy] catch the last cmd to track joins
  ticcmd_t* cmd = last_cmd;
  last_cmd = NULL;

  if (timingdemo)
    {
      int endtime = I_GetTime_RealTime();
      // killough -- added fps information and made it work for longer demos:
      unsigned realtics = endtime-starttime;
      I_Success("Timed %u gametics in %u realtics = %-.1f frames per second",
               (unsigned) gametic,realtics,
               (unsigned) gametic * (double) TICRATE / realtics);
    }

  if (demoplayback)
    {
      if (demorecording)
      {
        if (netgame)
          CheckPlayersInNetGame();

        demoplayback = false;

        // clear progress demo bar
        ST_Start();

        // [crispy] record demo join
        if (cmd != NULL)
        {
          cmd->buttons |= BT_JOIN;
        }

        displaymsg("Demo Recording: %s", M_BaseName(demoname));

        return true;
      }

      if (singledemo)
        I_SafeExit(0);  // killough

      // [FG] ignore empty demo lumps
      if (demobuffer)
      {
        Z_ChangeTag(demobuffer, PU_CACHE);
      }

      G_ReloadDefaults(false); // killough 3/1/98
      netgame = false;       // killough 3/29/98
      solonet = false;
      deathmatch = false;
      D_AdvanceDemo();
      return true;
    }

  if (demorecording)
    {
      demorecording = false;

      if (!demo_p)
        return false;

      *demo_p++ = DEMOMARKER;

      G_AddDemoFooter();

      if (!M_WriteFile(demoname, demobuffer, demo_p - demobuffer))
	I_Error("Error recording demo %s: %s", demoname,  // killough 11/98
		errno ? strerror(errno) : "(Unknown Error)");

      Z_Free(demobuffer);
      demobuffer = NULL;  // killough
      I_Printf(VB_ALWAYS, "Demo %s recorded", demoname);
      // [crispy] if a new game is started during demo recording, start a new demo
      if (gameaction != ga_newgame && gameaction != ga_reloadlevel)
      {
        I_SafeExit(0);
      }
      return false;  // killough
    }

  return false;
}

// killough 1/22/98: this is a "Doom printf" for messages. I've gotten
// tired of using players->message=... and so I've added this dprintf.
//
// killough 3/6/98: Made limit static to allow z_zone functions to call
// this function, without calling realloc(), which seems to cause problems.

#define MAX_MESSAGE_SIZE 1024

void doomprintf(player_t *player, msg_category_t category, const char *s, ...)
{
  static char msg[MAX_MESSAGE_SIZE];
  va_list v;

  if ((category == MESSAGES_TOGGLE && !show_toggle_messages) ||
      (category == MESSAGES_PICKUP && !show_pickup_messages) ||
      (category == MESSAGES_OBITUARY && !show_obituary_messages))
    return;

  va_start(v,s);
  vsprintf(msg,s,v);                  // print message in buffer
  va_end(v);

  if (player)
    player->message = msg;
  else
    players[displayplayer].message = msg;  // set new message
}

void G_BindGameInputVariables(void)
{
  BIND_BOOL(autorun, true, "Always run");
  BIND_BOOL_GENERAL(mouselook, false, "Mouselook");
  BIND_BOOL_GENERAL(dclick_use, true, "Double-click acts as use-button");
  BIND_BOOL(novert, true, "Disable vertical mouse movement");
  BIND_BOOL_GENERAL(padlook, false, "Padlook");
}

void G_BindGameVariables(void)
{
  BIND_BOOL(raw_input, true,
    "Raw gamepad/mouse input for turning/looking (0 = Interpolate; 1 = Raw)");
  BIND_BOOL(fake_longtics, true,
    "Fake high-resolution turning when using low-resolution turning");
  BIND_BOOL(shorttics, false, "Always use low-resolution turning");
  BIND_NUM(quickstart_cache_tics, 0, 0, TICRATE, "Quickstart cache tics");

  // [Nugget] Account for custom skill
  BIND_NUM_GENERAL(default_skill, 3, 1, 6,
    "Default skill level (1 = ITYTD; 2 = HNTR; 3 = HMP; 4 = UV; 5 = NM; 6 = Custom)");

  // [Nugget] Custom Skill /--------------------------------------------------

  M_BindNum("custom_skill_things", &custom_skill_things, NULL,
            2, 0, 2, ss_skill, wad_yes,
            "Custom Skill: thing spawns (0 = Easy; 1 = Normal; 2 = Hard)");

  M_BindBool("custom_skill_coopspawns", &custom_skill_coopspawns, NULL,
            0, ss_skill, wad_yes,
            "Custom Skill: spawn multiplayer things");

  M_BindBool("custom_skill_nomonsters", &custom_skill_nomonsters, NULL,
            0, ss_skill, wad_yes,
            "Custom Skill: don't spawn monsters");

  M_BindBool("custom_skill_doubleammo", &custom_skill_doubleammo, NULL,
            0, ss_skill, wad_yes,
            "Custom Skill: receive double ammo from pickups");

  M_BindBool("custom_skill_halfdamage", &custom_skill_halfdamage, NULL,
            0, ss_skill, wad_yes,
            "Custom Skill: player takes half the damage");

  M_BindBool("custom_skill_slowbrain", &custom_skill_slowbrain, NULL,
            0, ss_skill, wad_yes,
            "Custom Skill: Icon of Sin shoots cubes half the time");

  M_BindBool("custom_skill_fast", &custom_skill_fast, NULL,
            0, ss_skill, wad_yes,
            "Custom Skill: fast monsters");

  M_BindBool("custom_skill_respawn", &custom_skill_respawn, NULL,
            0, ss_skill, wad_yes,
            "Custom Skill: respawning monsters");

  M_BindBool("custom_skill_aggressive", &custom_skill_aggressive, NULL,
            0, ss_skill, wad_yes,
            "Custom Skill: aggressive monsters (instant reaction time; continuous attacks)");

  M_BindBool("custom_skill_x2monsters", &custom_skill_x2monsters, NULL,
            0, ss_skill, wad_yes,
            "Custom Skill: duplicate monster spawns");

  // [Nugget] ---------------------------------------------------------------/

  BIND_NUM_GENERAL(realtic_clock_rate, 100, 10, 1000,
    "Game speed percent");
  M_BindNum("max_player_corpse", &default_bodyquesize, NULL,
    32, UL, UL, ss_none, wad_no,
    "Maximum number of player corpses (< 0 = No limit)");
  BIND_NUM_GENERAL(death_use_action, 0, 0, 2,
    "Use-button action upon death (0 = Default; 1 = Last Save; 2 = Nothing)");
  BIND_BOOL_GENERAL(autosave, true,
    "Auto save at the beginning of a map, after completing the previous one");

  // [Nugget] ----------------------------------------------------------------

  M_BindNum("autosave_interval", &autosave_interval, NULL,
            0, 0, 600, ss_misc, wad_no,
            "Interval between periodic auto saves, in seconds (0 = Off)");

  M_BindBool("one_key_saveload", &one_key_saveload, NULL,
             false, ss_misc, wad_no,
             "One-key quick-saving/loading");

  M_BindNum("rewind_interval", &rewind_interval, NULL,
            1, 1, 600, ss_misc, wad_no,
            "Interval between rewind key-frames, in seconds");

  M_BindNum("rewind_depth", &rewind_depth, NULL,
            60, 0, 3000, ss_misc, wad_no,
            "Number of rewind key-frames to be stored (0 = No rewinding)");

  M_BindNum("rewind_timeout", &rewind_timeout, NULL,
            10, 0, 25, ss_misc, wad_no,
            "Max. time to store a key frame, in milliseconds; if exceeded, storing will stop (0 = No limit)");
}

void G_BindEnemVariables(void)
{
  M_BindNum("player_helpers", &default_dogs, &dogs, 0, 0, 3, ss_enem, wad_yes,
    "Number of helper dogs to spawn at map start");
  M_BindBool("ghost_monsters", &ghost_monsters, NULL, true, ss_enem, wad_no,
             "Make ghost monsters (resurrected pools of gore) translucent");

  M_BindBool("monsters_remember", &default_monsters_remember, &monsters_remember,
             true, ss_none, wad_yes,
             "Monsters return to their previous target after losing their current one");
  M_BindBool("monster_infighting", &default_monster_infighting, &monster_infighting,
             true, ss_none, wad_yes,
             "Monsters fight each other when provoked");
  M_BindBool("monster_backing", &default_monster_backing, &monster_backing,
             false, ss_none, wad_yes,
             "Ranged monsters back away from melee targets");
  M_BindBool("monster_avoid_hazards", &default_monster_avoid_hazards, &monster_avoid_hazards,
             true, ss_none, wad_yes,
             "Monsters avoid hazards such as crushing ceilings");
  M_BindBool("monkeys", &default_monkeys, &monkeys, false, ss_none, wad_yes,
             "Monsters move up/down steep stairs");
  M_BindBool("monster_friction", &default_monster_friction, &monster_friction,
             true, ss_none, wad_yes,
             "Monsters are affected by friction modifiers");
  M_BindBool("help_friends", &default_help_friends, &help_friends,
             false, ss_none, wad_yes, "Monsters prefer targets of injured allies");
  M_BindNum("friend_distance", &default_distfriend, &distfriend,
            128, 0, 999, ss_none, wad_yes, "Minimum distance that friends keep between each other");
  M_BindBool("dog_jumping", &default_dog_jumping, &dog_jumping,
             true, ss_none, wad_yes, "Dogs are able to jump down from high ledges");

  // [Nugget] ----------------------------------------------------------------

  M_BindBool("extra_gibbing", &extra_gibbing_on, NULL,
             false, ss_enem, wad_yes,
             "Enable extra gibbing in general (affected by CVARs below)");

  // (CFG-only)
  M_BindBool("extra_gibbing_fist", &extra_gibbing[EXGIB_FIST], NULL,
             true, ss_none, wad_yes,
             "Extra gibbing for Berserk Fist and similar MBF21 attacks");

  // (CFG-only)
  M_BindBool("extra_gibbing_csaw", &extra_gibbing[EXGIB_CSAW], NULL,
             true, ss_none, wad_yes,
             "Extra gibbing for Chainsaw");

  // (CFG-only)
  M_BindBool("extra_gibbing_ssg", &extra_gibbing[EXGIB_SSG], NULL,
             true, ss_none, wad_yes,
             "Extra gibbing for SSG and similar MBF21 attacks");

  // (CFG-only)
  M_BindBool("extra_gibbing_bfg", &extra_gibbing[EXGIB_BFG], NULL,
             true, ss_none, wad_yes,
             "Extra gibbing for BFG tracers");

  // (CFG-only)
  M_BindBool("extra_gibbing_proj", &extra_gibbing[EXGIB_PROJ], NULL,
             true, ss_none, wad_yes,
             "Extra gibbing for projectiles dealing 20+ base damage");

  M_BindBool("bloodier_gibbing", &bloodier_gibbing, NULL,
             false, ss_enem, wad_yes,
             "Bloodier gibbing");

  // (CFG-only)
  M_BindNum("bloodier_gibbing_splats", &bloodier_gibbing_splats, &bloodier_gibbing_splats,
            40, 10, 256, ss_none, wad_yes,
            "Max. number of bloodier-gibbing splats");

  M_BindBool("tossdrop", &tossdrop, NULL,
             false, ss_enem, wad_yes,
             "Enemies toss their items dropped upon death");
}

void G_BindCompVariables(void)
{
  M_BindNum("default_complevel", &default_complevel, NULL,
            CL_MBF21, CL_VANILLA, CL_MBF21, ss_comp, wad_no,
            "Default compatibility level (0 = Vanilla; 1 = Boom; 2 = MBF; 3 = MBF21)");
  M_BindBool("autostrafe50", &autostrafe50, NULL, false, ss_comp, wad_no,
             "Automatic strafe50 (SR50)");
  M_BindBool("strictmode", &default_strictmode, &strictmode,
             false, ss_comp, wad_no, "Strict mode");
  M_BindBool("hangsolid", &hangsolid, NULL, false, ss_comp, wad_no,
             "Enable walking under solid hanging bodies");

  // [Nugget] /---------------------------------------------------------------

  M_BindNum("over_under", &over_under, NULL,
            0, 0, 2, ss_gen, wad_yes,
            "Allow movement over/under things (0 = Off; 1 = Player only; 2 = All things)");

  M_BindBool("jump_crouch", &jump_crouch, NULL,
             false, ss_gen, wad_yes,
             "Jumping/crouching");

  // [Nugget] ---------------------------------------------------------------/

  M_BindBool("blockmapfix", &blockmapfix, NULL, false, ss_comp, wad_no,
             "Fix blockmap bug (improves hit detection)");

  // [Nugget] Hitbox-based hitscan collision
  M_BindBool("hitbox_hitscan", &hitbox_hitscan, NULL, false, ss_comp, wad_no,
             "Hitbox-based hitscan collision (fixes melee against large things)");

  M_BindBool("checksight12", &checksight12, NULL, false, ss_comp, wad_no,
             "Fast blockmap-based line-of-sight calculation");

  // [Nugget] Replaces `direct_vertical_aiming`
  M_BindNum("vertical_aiming", &default_vertical_aiming, &vertical_aiming,
            0, 0, 2, ss_comp, wad_no, "Vertical aiming (0 = Auto; 1 = Direct; 2 = Direct + Auto)");

  M_BindBool("pistolstart", &default_pistolstart, &pistolstart,
             false, ss_comp, wad_no, "Pistol start");

#define BIND_COMP(id, v, help) \
  M_BindNum(#id, &default_comp[(id)], &comp[(id)], (v), 0, 1, ss_none, wad_yes, help)

  BIND_COMP(comp_zombie,    1, "Dead players can trigger linedef actions");
  BIND_COMP(comp_infcheat,  0, "Powerup cheats don't last forever");
  BIND_COMP(comp_stairs,    0, "Build stairs exactly the same way that Doom does");
  BIND_COMP(comp_telefrag,  0, "Monsters can only telefrag on MAP30");
  BIND_COMP(comp_dropoff,   0, "Some objects never move over tall ledges");
  BIND_COMP(comp_falloff,   0, "Objects don't fall off ledges under their own weight");
  BIND_COMP(comp_staylift,  0, "Monsters randomly walk off of moving lifts");
  BIND_COMP(comp_doorstuck, 0, "Monsters get stuck in door tracks");
  BIND_COMP(comp_pursuit,   1, "Monsters can infight immediately when alerted");
  BIND_COMP(comp_vile,      0, "Arch-viles can create ghost monsters");
  BIND_COMP(comp_pain,      0, "Pain elementals are limited to 20 lost souls");
  BIND_COMP(comp_skull,     0, "Lost souls can spawn past impassable lines");
  BIND_COMP(comp_blazing,   0, "Incorrect sound behavior for blazing doors");
  BIND_COMP(comp_doorlight, 0, "Door lighting changes are immediate");
  BIND_COMP(comp_god,       0, "God mode isn't absolute");
  BIND_COMP(comp_skymap,    0, "Don't apply invulnerability palette to skies");
  BIND_COMP(comp_floors,    0, "Use exactly Doom's floor motion behavior");
  BIND_COMP(comp_model,     0, "Use exactly Doom's linedef trigger model");
  BIND_COMP(comp_zerotags,  0, "Linedef actions work on sectors with tag 0");
  BIND_COMP(comp_soul,      0, "Lost souls bounce on floors and ceilings");
  BIND_COMP(comp_respawn,   0, "Monsters not spawned at level start respawn at map origin");
  BIND_COMP(comp_ledgeblock, 1, "Ledges block monsters");
  BIND_COMP(comp_friendlyspawn, 1, "Things spawned by A_Spawn inherit friendliness of spawner");
  BIND_COMP(comp_voodooscroller, 0, "Voodoo dolls on slow scrollers move too slowly");
  BIND_COMP(comp_reservedlineflag, 1, "ML_RESERVED clears extended flags");

#define BIND_EMU(id, v, help) \
  M_BindBool(#id, &overflow[(id)].enabled, NULL, (v), ss_none, wad_no, help)

  BIND_EMU(emu_spechits, true, "Emulate SPECHITS overflow");
  BIND_EMU(emu_reject, true, "Emulate REJECT overflow");
  M_BindBool("emu_intercepts", &overflow[emu_intercepts].enabled, NULL, true,
    ss_comp, wad_no, "Emulate INTERCEPTS overflow");
  BIND_EMU(emu_missedbackside, false, "Emulate overflow caused by two-sided lines with missing backsides");
  BIND_EMU(emu_donut, true, "Emulate donut overflow");

  // [Nugget] (CFG-only) -----------------------------------------------------

  #define BIND_COMP_NUG(name, v, help) \
    M_BindBool(#name, &name, NULL, (v), ss_none, wad_yes, help)

  BIND_COMP_NUG(comp_bruistarget,  true,  "Bruiser attack doesn't face target");
  BIND_COMP_NUG(comp_nomeleesnap,  false, "Disable snapping to target when using melee");
  BIND_COMP_NUG(comp_longautoaim,  false, "Double autoaim range");
  BIND_COMP_NUG(comp_lscollision,  false, "Fix Lost Souls colliding with items");
  BIND_COMP_NUG(comp_lsamnesia,    true,  "Lost Souls forget target upon impact");
  BIND_COMP_NUG(comp_fuzzyblood,   false, "Fuzzy things bleed fuzzy blood");
  BIND_COMP_NUG(comp_faceshadow,   false, "Attackers face fuzzy targets straight");
  BIND_COMP_NUG(comp_nonbleeders,  false, "Non-bleeders don't bleed when crushed");
  BIND_COMP_NUG(comp_iosdeath,     false, "Fix lopsided Icon of Sin explosions");
  BIND_COMP_NUG(comp_choppers,     false, "Permanent IDCHOPPERS invulnerability");
  BIND_COMP_NUG(comp_manualdoor,   true,  "Manually toggled moving doors are silent");
  BIND_COMP_NUG(comp_switchsource, false, "Corrected switch sound source");
  BIND_COMP_NUG(comp_cgundblsnd,   true,  "Chaingun makes two sounds with one bullet");
  BIND_COMP_NUG(comp_cgunnersfx,   false, "Chaingunner uses pistol/chaingun sound");
  BIND_COMP_NUG(comp_flamst,       false, "Arch-Vile fire plays flame-start sound");
  BIND_COMP_NUG(comp_keynoway,     false, "Play DSNOWAY instead of DSOOF when failing to use key-locked triggers");
  BIND_COMP_NUG(comp_godface,      false, "Higher god-mode face priority");
  BIND_COMP_NUG(comp_deadoof,      true,  "Dead players can still play oof sound");
  BIND_COMP_NUG(comp_powerrunout,  false, "Use improved powerup run-out effect");
  BIND_COMP_NUG(comp_unusedpals,   false, "Use unused pain/bonus palettes");
  BIND_COMP_NUG(comp_keypal,       true,  "Key pickup resets palette");
}

void G_BindWeapVariables(void)
{
  // [Nugget] Extended, tweaked descriptions
  M_BindNum("view_bobbing_pct", &view_bobbing_pct, NULL,
            100, 0, 100, ss_weap, wad_no,
            "Player view bobbing amplitude percent");
  M_BindNum("weapon_bobbing_pct", &weapon_bobbing_pct, NULL,
            100, 0, 100, ss_weap, wad_no,
            "Player weapon bobbing amplitude percent");

  M_BindBool("hide_weapon", &hide_weapon, NULL, false, ss_weap, wad_no,
             "Disable rendering of weapon sprites");

  // [Nugget] Horizontal weapon centering
  M_BindNum("center_weapon", &center_weapon, NULL, 0, 0, 3, ss_weap, wad_no,
            "Weapon alignment while attacking (1 = Centered; 2 = Bobbing; 3 = Horizontal)");

  M_BindBool("weapon_recoilpitch", &weapon_recoilpitch, NULL,
             false, ss_weap, wad_no,
             "Recoil pitch from weapon fire");

  // [Nugget] (CFG-only)
  M_BindNum("weapon_recoilpitch_scale_pct", &weapon_recoilpitch_scale_pct, NULL,
            100, 10, 100, ss_none, wad_no,
            "Recoil-pitch scale percent");

  M_BindBool("weapon_recoil", &default_weapon_recoil, &weapon_recoil,
             false, ss_weap, wad_yes, // [Nugget] Restored menu item
             "Physical recoil from weapon fire (affects compatibility)");
  M_BindBool("doom_weapon_toggles", &doom_weapon_toggles, NULL,
             true, ss_weap, wad_no,
             "Allow toggling between SG/SSG and Fist/Chainsaw");
  BIND_BOOL(doom_weapon_cycle, false,
            "Next weapon skips lower priority weapon slots (SG/SSG and Fist/Chainsaw)");
  M_BindBool("player_bobbing", &default_player_bobbing, &player_bobbing,
             true, ss_none, wad_no, "Physical player bobbing (affects compatibility)");

  // [Nugget] /---------------------------------------------------------------

  M_BindBool("smart_autoaim", &smart_autoaim, NULL,
             false, ss_weap, wad_yes,
             "Smart projectile autoaim (checks if projectile will hit a living entity)");

  M_BindBool("no_hor_autoaim", &no_hor_autoaim, NULL,
             false, ss_weap, wad_yes,
             "Disable horizontal projectile autoaim");

  M_BindBool("switch_on_pickup", &switch_on_pickup, NULL,
             true, ss_weap, wad_no,
             "Switch weapons when acquiring new ones or ammo for them");

  M_BindBool("improved_weapon_toggles", &improved_weapon_toggles, NULL,
             false, ss_weap, wad_no,
             "Improved Fist/Chainsaw and Shotgun/SSG toggles");

  M_BindBool("weapswitch_interruption", &weapswitch_interruption, NULL,
             false, ss_weap, wad_yes,
             "Allow interruption of weapon switches");

  M_BindBool("skip_ammoless_weapons", &skip_ammoless_weapons, NULL,
             false, ss_weap, wad_no,
             "Previous/next-weapon buttons skip weapons with insufficient ammo");

  // (CFG-only)
  M_BindBool("always_bob", &always_bob, NULL,
             true, ss_none, wad_yes,
             "Bob weapon every tic (fixes choppy Chainsaw bobbing)");

  M_BindNum("bobbing_style", &bobbing_style, NULL,
            BOBSTYLE_VANILLA, BOBSTYLE_VANILLA, NUM_BOBSTYLES-1, ss_weap, wad_yes,
            "Weapon bobbing style");

  M_BindNum("weapon_bobbing_speed_pct", &weapon_bobbing_speed_pct, NULL,
            100, 50, 300, ss_weap, wad_yes,
            "Player weapon bobbing speed percent");

  M_BindBool("switch_bob", &switch_bob, NULL,
             false, ss_weap, wad_yes,
             "Bob weapon while switching it");

  M_BindBool("weapon_inertia", &weapon_inertia, NULL,
             false, ss_weap, wad_yes,
             "Weapon inertia");

  M_BindNum("weapon_inertia_scale_pct", &weapon_inertia_scale_pct, NULL,
            100, -200, 200, ss_weap, wad_yes,
            "Weapon-inertia scale percent");

  // (CFG-only)
  M_BindBool("weapon_inertia_fire", &weapon_inertia_fire, NULL,
             true, ss_none, wad_yes,
             "Apply weapon inertia while firing");

  M_BindBool("weaponsquat", &weaponsquat, NULL,
             false, ss_weap, wad_yes,
             "Squat weapon down upon landing/jumping");

  M_BindNum("pspr_translucency_pct", &pspr_translucency_pct, NULL,
            100, 0, 100, ss_weap, wad_yes,
            "Weapon-flash translucency percent");

  // (CFG-only)
  M_BindBool("sx_fix", &sx_fix, NULL,
             false, ss_none, wad_yes,
             "Correct centering of first-person sprites");

  // (CFG-only)
  M_BindNum("hitscan_trail_interval", &hitscan_trail_interval, NULL,
            8, 1, 16, ss_none, wad_yes,
            "Distance between particles of hitscan trails, in units");

  // [Nugget] ---------------------------------------------------------------/

#define BIND_WEAP(num, v, help) \
  M_BindNum("weapon_choice_"#num, &weapon_preferences[0][(num) - 1], NULL, \
      (v), 1, 9, ss_weap, wad_yes, help)

  BIND_WEAP(1, 6, "First choice for weapon (best)");
  BIND_WEAP(2, 9, "Second choice for weapon");
  BIND_WEAP(3, 4, "Third choice for weapon");
  BIND_WEAP(4, 3, "Fourth choice for weapon");
  BIND_WEAP(5, 2, "Fifth choice for weapon");
  BIND_WEAP(6, 8, "Sixth choice for weapon");
  BIND_WEAP(7, 5, "Seventh choice for weapon");
  BIND_WEAP(8, 7, "Eighth choice for weapon");
  BIND_WEAP(9, 1, "Ninth choice for weapon (worst)");

  M_BindBool("classic_bfg", &default_classic_bfg, &classic_bfg,
             false, ss_weap, wad_yes, "Use pre-beta BFG2704");
}

//----------------------------------------------------------------------------
//
// $Log: g_game.c,v $
// Revision 1.59  1998/06/03  20:23:10  killough
// fix v2.00 demos
//
// Revision 1.58  1998/05/16  09:16:57  killough
// Make loadgame checksum friendlier
//
// Revision 1.57  1998/05/15  00:32:28  killough
// Remove unnecessary crash hack
//
// Revision 1.56  1998/05/13  22:59:23  killough
// Restore Doom bug compatibility for demos, fix multiplayer status bar
//
// Revision 1.55  1998/05/12  12:46:16  phares
// Removed OVER_UNDER code
//
// Revision 1.54  1998/05/07  00:48:51  killough
// Avoid displaying uncalled for precision
//
// Revision 1.53  1998/05/06  15:32:24  jim
// document g_game.c, change externals
//
// Revision 1.52  1998/05/05  16:29:06  phares
// Removed RECOIL and OPT_BOBBING defines
//
// Revision 1.51  1998/05/04  22:00:33  thldrmn
// savegamename globalization
//
// Revision 1.50  1998/05/03  22:15:19  killough
// beautification, decls & headers, net consistency fix
//
// Revision 1.49  1998/04/27  17:30:12  jim
// Fix DM demo/newgame status, remove IDK (again)
//
// Revision 1.48  1998/04/25  12:03:44  jim
// Fix secret level fix
//
// Revision 1.46  1998/04/24  12:09:01  killough
// Clear player cheats before demo starts
//
// Revision 1.45  1998/04/19  19:24:19  jim
// Improved IWAD search
//
// Revision 1.44  1998/04/16  16:17:09  jim
// Fixed disappearing marks after new level
//
// Revision 1.43  1998/04/14  10:55:13  phares
// Recoil, Bobbing, Monsters Remember changes in Setup now take effect immediately
//
// Revision 1.42  1998/04/13  21:36:12  phares
// Cemented ESC and F1 in place
//
// Revision 1.41  1998/04/13  10:40:58  stan
// Now synch up all items identified by Lee Killough as essential to
// game synch (including bobbing, recoil, rngseed).  Commented out
// code in g_game.c so rndseed is always set even in netgame.
//
// Revision 1.40  1998/04/13  00:39:29  jim
// Fix automap marks carrying over thru levels
//
// Revision 1.39  1998/04/10  06:33:00  killough
// Fix -fast parameter bugs
//
// Revision 1.38  1998/04/06  04:51:32  killough
// Allow demo_insurance=2
//
// Revision 1.37  1998/04/05  00:50:48  phares
// Joystick support, Main Menu re-ordering
//
// Revision 1.36  1998/04/02  16:15:24  killough
// Fix weapons switch
//
// Revision 1.35  1998/04/02  04:04:27  killough
// Fix DM respawn sticking problem
//
// Revision 1.34  1998/04/02  00:47:19  killough
// Fix net consistency errors
//
// Revision 1.33  1998/03/31  10:36:41  killough
// Fix crash caused by last change, add new RNG options
//
// Revision 1.32  1998/03/28  19:15:48  killough
// fix DM spawn bug (Stan's fix)
//
// Revision 1.31  1998/03/28  17:55:06  killough
// Fix weapons switch bug, improve RNG while maintaining sync
//
// Revision 1.30  1998/03/28  15:49:47  jim
// Fixed merge glitches in d_main.c and g_game.c
//
// Revision 1.29  1998/03/28  05:32:00  jim
// Text enabling changes for DEH
//
// Revision 1.28  1998/03/27  21:27:00  jim
// Fixed sky bug for Ultimate DOOM
//
// Revision 1.27  1998/03/27  16:11:43  stan
// (SG) Commented out lines in G_ReloadDefaults that reset netgame and
//      deathmatch to zero.
//
// Revision 1.26  1998/03/25  22:51:25  phares
// Fixed headsecnode bug trashing memory
//
// Revision 1.25  1998/03/24  15:59:17  jim
// Added default_skill parameter to config file
//
// Revision 1.24  1998/03/23  15:23:39  phares
// Changed pushers to linedef control
//
// Revision 1.23  1998/03/23  03:14:27  killough
// Fix savegame checksum, net/demo consistency w.r.t. weapon switch
//
// Revision 1.22  1998/03/20  00:29:39  phares
// Changed friction to linedef control
//
// Revision 1.21  1998/03/18  16:16:47  jim
// Fix to idmusnum handling
//
// Revision 1.20  1998/03/17  20:44:14  jim
// fixed idmus non-restore, space bug
//
// Revision 1.19  1998/03/16  12:29:14  killough
// Add savegame checksum test
//
// Revision 1.18  1998/03/14  17:17:24  jim
// Fixes to deh
//
// Revision 1.17  1998/03/11  17:48:01  phares
// New cheats, clean help code, friction fix
//
// Revision 1.16  1998/03/09  18:29:17  phares
// Created separately bound automap and menu keys
//
// Revision 1.15  1998/03/09  07:09:20  killough
// Avoid realloc() in dprintf(), fix savegame -nomonsters bug
//
// Revision 1.14  1998/03/02  11:27:45  killough
// Forward and backward demo sync compatibility
//
// Revision 1.13  1998/02/27  08:09:22  phares
// Added gamemode checks to weapon selection
//
// Revision 1.12  1998/02/24  08:45:35  phares
// Pushers, recoil, new friction, and over/under work
//
// Revision 1.11  1998/02/23  04:19:35  killough
// Fix Internal and v1.9 Demo sync problems
//
// Revision 1.10  1998/02/20  22:50:51  killough
// Fix dprintf for multiplayer games
//
// Revision 1.9  1998/02/20  06:15:08  killough
// Turn turbo messages on in demo playbacks
//
// Revision 1.8  1998/02/17  05:53:41  killough
// Suppress "green is turbo" in non-net games
// Remove dependence on RNG for net consistency (intereferes with RNG)
// Use new RNG calling method, with keys assigned to blocks
// Friendlier savegame version difference message (instead of nothing)
// Remove futile attempt to make Boom v1.9-savegame-compatibile
//
// Revision 1.7  1998/02/15  02:47:41  phares
// User-defined keys
//
// Revision 1.6  1998/02/09  02:57:08  killough
// Make player corpse limit user-configurable
// Fix ExM8 level endings
// Stop 'q' from ending demo recordings
//
// Revision 1.5  1998/02/02  13:44:45  killough
// Fix dprintf and CheckSaveGame realloc bugs
//
// Revision 1.4  1998/01/26  19:23:18  phares
// First rev with no ^Ms
//
// Revision 1.3  1998/01/24  21:03:07  jim
// Fixed disappearence of nomonsters, respawn, or fast mode after demo play or IDCLEV
//
// Revision 1.1.1.1  1998/01/19  14:02:54  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
