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
//      Put all global state variables here.
//
//-----------------------------------------------------------------------------

#include "doomstat.h"
#include "m_misc.h"

// Game Mode - identify IWAD as shareware, retail etc.
GameMode_t gamemode = indetermined;
GameMission_t gamemission = doom;

// [FG] emulate a specific version of Doom
GameVersion_t gameversion = exe_doom_1_9;

GameVersions_t gameversions[] = {
    {"Doom 1.9",      "1.9",      exe_doom_1_9},
    {"Ultimate Doom", "ultimate", exe_ultimate},
    {"Final Doom",    "final",    exe_final},
    {"Chex Quest",    "chex",     exe_chex},
    { NULL,           NULL,       0},
};

// Language.
Language_t   language = english;

// Set if homebrew PWAD stuff has been added.
boolean modifiedgame;

boolean have_ssg;

//-----------------------------------------------------------------------------

// compatibility with old engines (monster behavior, metrics, etc.)
int compatibility, default_compatibility;          // killough 1/31/98

int comp[COMP_TOTAL], default_comp[COMP_TOTAL];    // killough 10/98

invul_mode_t invul_mode;

// [FG] overflow emulation
overflow_t overflow[EMU_TOTAL] = {
  { true, false, "spechits_overflow"},
  { true, false, "reject_overflow"},
  { true, false, "intercepts_overflow"},
  { true, false, "missedbackside_overflow"},
  { true, false, "donut_overflow"}
};

demo_version_t demo_version;        // killough 7/19/98: Boom version of demo

// v1.1-like pitched sounds
int pitched_sounds;  // killough 10/98

int translucency;    // killough 10/98

int  allow_pushers = 1;      // MT_PUSH Things              // phares 3/10/98
int  default_allow_pushers;  // killough 3/1/98: make local to each game

int  variable_friction = 1;      // ice & mud               // phares 3/10/98
int  default_variable_friction;  // killough 3/1/98: make local to each game

int  weapon_recoil;              // weapon recoil                   // phares
int  default_weapon_recoil;      // killough 3/1/98: make local to each game

int player_bobbing;  // whether player bobs or not          // phares 2/25/98
int default_player_bobbing;      // killough 3/1/98: make local to each game

int monsters_remember=1;        // killough 3/1/98
int default_monsters_remember=1;

int monster_infighting=1;       // killough 7/19/98: monster<=>monster attacks
int default_monster_infighting=1;

int monster_friction=1;       // killough 10/98: monsters affected by friction
int default_monster_friction=1;

// killough 7/19/98: classic Doom BFG
int classic_bfg, default_classic_bfg;

// killough 7/24/98: Emulation of Press Release version of Doom
int beta_emulation;

int dogs, default_dogs;         // killough 7/19/98: Marine's best friend :)
int dog_jumping, default_dog_jumping;   // killough 10/98

// killough 8/8/98: distance friends tend to move towards players
int distfriend = 128, default_distfriend = 128;

// killough 9/8/98: whether monsters are allowed to strafe or retreat
int monster_backing, default_monster_backing;

// killough 9/9/98: whether monsters are able to avoid hazards (e.g. crushers)
int monster_avoid_hazards, default_monster_avoid_hazards;

// killough 9/9/98: whether monsters help friends
int help_friends, default_help_friends;

int flashing_hom;     // killough 10/98

int doom_weapon_toggles; // killough 10/98

int monkeys, default_monkeys;

boolean hide_weapon;

// [FG] centered weapon sprite
int center_weapon;

int view_bobbing_pct;
int weapon_bobbing_pct;

char *MAPNAME(int e, int m)
{
  static char name[9];

  if (gamemode == commercial)
    M_snprintf(name, sizeof(name), "MAP%02d", m);
  else
    M_snprintf(name, sizeof(name), "E%dM%d", e, m);

  return name;
}


// [Nugget] /=================================================================

boolean fauxdemo = false;    // Checked for in `casual_play`, for debugging
boolean casual_play = false; // Like `critical`, with different checks and functionality

// General -------------------------------------------------------------------

int over_under;
int jump_crouch;
int viewheight_value;
int flinching;
int explosion_shake;
int explosion_shake_intensity_pct; // CFG-only
int breathing;
int teleporter_zoom;
int death_camera;

int chasecam_mode;
int chasecam_distance;
int chasecam_height;
int chasecam_crosshair; // CFG-only

int menu_background_all;
int no_menu_tint;
int hud_menu_shadows;
int no_berserk_tint;
int no_radsuit_tint;
int nightvision_visor;
int damagecount_cap;
int bonuscount_cap;
int fake_contrast;
int diminished_lighting; // CFG-only
int wipe_speed_percentage;
int alt_interpic;
int s_clipping_dist_x2;
int one_key_saveload;
int autosave_interval; // CFG-only
int rewind_interval;
int rewind_depth;
int rewind_timeout;
int no_page_ticking;
int quick_quitgame;

//int a11y_sector_lighting;
int a11y_weapon_flash;
int a11y_weapon_pspr;
int a11y_invul_colormap;

// Weapons -------------------------------------------------------------------

int no_hor_autoaim;
int switch_on_pickup;
int skip_ammoless_weapons;
int always_bob; // CFG-only
int bobbing_style;
int weapon_inertia;
int weapon_inertia_scale_pct; // CFG-only
int weaponsquat;
int translucent_pspr;
int translucent_pspr_pct; // CFG-only
int show_berserk;
int sx_fix; // CFG-only

// Status Bar/HUD ------------------------------------------------------------

int announce_milestones;
int show_save_messages; // CFG-only
int show_ssg; // CFG-only
int hud_stats_format;
int hud_stats_format_map;
int hud_allow_icons;
int hud_highlight_weapon;
int alt_arms;
int hud_time_teleport;
int hud_time_keypickup;

int hudcolor_time_scale;
int hudcolor_total_time;
int hudcolor_time;
int hudcolor_event_timer;
int hudcolor_kills;
int hudcolor_items;
int hudcolor_secrets;
int hudcolor_ms_incomp;
int hudcolor_ms_comp;

// Enemies -------------------------------------------------------------------

int extra_gibbing_on;
int extra_gibbing[NUMEXGIBS]; // CFG-only
int bloodier_gibbing;
int zdoom_item_drops;

// Key Bindings --------------------------------------------------------------

int zoom_fov;
int fancy_teleport;

// Miscellaneous (CFG-only) --------------------------------------------------

int screenshot_palette;
int fov_stretchsky;
int hud_menu_shadows_filter_pct;
int menu_backdrop_darkening;
int automap_overlay_darkening;
int no_killough_face;
int sp_chat;

int fail_safe;

// Doom Compatibility (CFG-only) ---------------------------------------------

int comp_bruistarget;
int comp_nomeleesnap;
int comp_longautoaim;
int comp_lscollision;
int comp_lsamnesia;
int comp_fuzzyblood;
int comp_nonbleeders;
int comp_faceshadow;
int comp_iosdeath;
int comp_choppers;

int comp_blazing2;
int comp_manualdoor;
int comp_switchsource;
int comp_cgundblsnd;
int comp_cgunnersfx;
int comp_flamst;
int comp_keynoway;
int comp_godface;
int comp_deadoof;
int comp_powerrunout;
int comp_unusedpals;
int comp_keypal;

// Custom Skill --------------------------------------------------------------

int custom_skill_things;
int custom_skill_coopspawns;
int custom_skill_nomonsters;
int custom_skill_doubleammo;
int custom_skill_halfdamage;
int custom_skill_slowbrain;
int custom_skill_fast;
int custom_skill_respawn;
int custom_skill_aggressive;
int custom_skill_x2monsters;

// [Nugget] =================================================================/


//----------------------------------------------------------------------------
//
// $Log: doomstat.c,v $
// Revision 1.5  1998/05/12  12:46:12  phares
// Removed OVER_UNDER code
//
// Revision 1.4  1998/05/05  16:29:01  phares
// Removed RECOIL and OPT_BOBBING defines
//
// Revision 1.3  1998/05/03  23:12:13  killough
// beautify, move most global switch variables here
//
// Revision 1.2  1998/01/26  19:23:10  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:06  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
