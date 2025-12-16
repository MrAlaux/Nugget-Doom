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
//      System specific interface stuff.
//
//-----------------------------------------------------------------------------

#ifndef __R_MAIN__
#define __R_MAIN__

#include "doomtype.h"
#include "m_fixed.h"
#include "tables.h"

struct node_s;
struct player_s;
struct seg_s;

// [Nugget]
struct mobj_s;

//
// POV related.
//

extern fixed_t  viewcos;
extern fixed_t  viewsin;
extern int      viewwindowx;
extern int      viewwindowy;
extern int      centerx;
extern int      centery;
extern fixed_t  centerxfrac;
extern fixed_t  centeryfrac;
extern fixed_t  projection;
extern fixed_t  skyiscale,
                skyiscalediff; // [Nugget] FOV-based sky stretching
extern int      validcount;
extern int      linecount;
extern int      loopcount;
extern fixed_t  viewheightfrac; // [FG] sprite clipping optimizations
extern int      max_project_slope;

//
// Rendering stats
//

extern int rendered_visplanes, rendered_segs, rendered_vissprites, rendered_voxels;

void R_BindRenderVariables(void);

//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants.

extern int LIGHTLEVELS;
extern int LIGHTSEGSHIFT;
extern int LIGHTBRIGHT;
extern int MAXLIGHTSCALE;
extern int LIGHTSCALESHIFT;
extern int MAXLIGHTZ;
extern int LIGHTZSHIFT;

// killough 3/20/98: Allow colormaps to be dynamic (e.g. underwater)
extern lighttable_t **(*scalelight);
extern lighttable_t **(*zlight);
extern int numcolormaps;    // killough 4/4/98: dynamic number of maps
// killough 3/20/98, 4/4/98: end dynamic colormaps

extern boolean setsmoothlight;
void R_SmoothLight(void);

extern int          extralight;
extern lighttable_t *fixedcolormap;

// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.

#define NUMCOLORMAPS 32

//
// Function pointer to switch refresh/drawing functions.
//

extern void (*colfunc)(void);

//
// Utility functions.
//

int R_PointOnSide(fixed_t x, fixed_t y, struct node_s *node);
int R_PointOnSegSide(fixed_t x, fixed_t y, struct seg_s *line);
angle_t R_PointToAngle(fixed_t x, fixed_t y);
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
angle_t R_PointToAngleCrispy(fixed_t x, fixed_t y);
struct subsector_s *R_PointInSubsector(fixed_t x, fixed_t y);

//
// REFRESH - the actual rendering functions.
//

void R_UpdateViewAngleFunction(void);
void R_RenderPlayerView(struct player_s *player);   // Called by G_Drawer.
void R_Init(void);                           // Called by startup code.
void R_SetViewSize(int blocks);              // Called by M_Responder.

// [Nugget] /=================================================================

// CVARs ---------------------------------------------------------------------

typedef enum spriteshadows_s {
  SPRITESHADOWS_OFF,
  SPRITESHADOWS_SIMPLE,
  SPRITESHADOWS_3D,

  NUM_SPRITESHADOWS
} spriteshadows_t;

typedef enum thinglighting_s {
  THINGLIGHTING_ORIGIN,
  THINGLIGHTING_HITBOX,
  THINGLIGHTING_PERCOLUMN,

  NUM_THINGLIGHTING
} thinglighting_t;

typedef enum fakecontrast_s {
  FAKECONTRAST_OFF,
  FAKECONTRAST_SMOOTH,
  FAKECONTRAST_VANILLA,

  NUM_FAKECONTRAST
} fakecontrast_t;

extern boolean vertical_lockon;

extern spriteshadows_t sprite_shadows;
extern int sprite_shadows_tran_pct;
extern thinglighting_t thing_lighting_mode;
extern boolean flip_levels;
extern boolean nightvision_visor;
extern fakecontrast_t fake_contrast;
extern boolean diminishing_lighting;
extern boolean a11y_weapon_pspr;
extern boolean a11y_invul_colormap;
extern int pspr_invis_translucent;
extern int pspr_translucency_pct;
extern int zoom_fov;
extern boolean comp_powerrunout;

// ---------------------------------------------------------------------------

extern boolean have_crouch_sprites;

fixed_t R_GetNughudViewPitch(void);
boolean R_SpriteShadowsOn(void);
int R_GetLightLevelInPoint(fixed_t x, fixed_t y, boolean force_mbf);

#define PSPR_INVIS_TRANSLUCENCY 50

#define POWER_RUNOUT(power) \
  ((STRICTMODE(comp_powerrunout) ? (power) >= 4*32 : (power) > 4*32) || (power) & 8)

// FOV effects ---------------------------------------------------------------

enum {
  FOVFX_ZOOM,
  FOVFX_SLOWMO,
  FOVFX_TELEPORT,

  NUMFOVFX
};

enum {
  ZOOM_RESET = -1,
  ZOOM_OFF   =  0,
  ZOOM_ON    =  1,
};

extern void R_ClearFOVFX(void);
extern int  R_GetFOVFX(const int fx);
extern void R_SetFOVFX(const int fx);
extern int  R_GetZoom(void);
extern void R_SetZoom(const int state);

// Explosion shake effect ----------------------------------------------------

extern boolean explosion_shake;
extern int explosion_shake_intensity_pct;

extern void R_ClearShake(void);
extern void R_ExplosionShake(fixed_t bombx, fixed_t bomby, int force, int range);

// Chasecam ------------------------------------------------------------------

enum {
  CHASECAMMODE_OFF,
  CHASECAMMODE_BACK,
  CHASECAMMODE_FRONT,

  NUM_CHASECAMMODES
}; extern int chasecam_mode;
extern boolean chasecam_crosshair;

extern boolean R_ChasecamOn(void);
extern void    R_SetChasecamHit(const boolean value);
extern void    R_UpdateChasecam(fixed_t x, fixed_t y, fixed_t z);

// Freecam -------------------------------------------------------------------

typedef enum freecammode_s {
  FREECAM_OFF,
  FREECAM_CAM,
  FREECAM_PLAYER,
  
  NUMFREECAMMODES
} freecammode_t;

extern boolean       R_FreecamOn(void);
extern void          R_SetFreecamOn(const boolean value);
extern freecammode_t R_GetFreecamMode(void);
extern freecammode_t R_CycleFreecamMode(void);
extern angle_t       R_GetFreecamAngle(void);
extern void          R_ResetFreecam(const boolean newmap);
extern void          R_MoveFreecam(fixed_t x, fixed_t y, fixed_t z);

extern void                 R_UpdateFreecamMobj(struct mobj_s *const mobj);
extern const struct mobj_s *R_GetFreecamMobj(void);

extern void R_UpdateFreecam(fixed_t x, fixed_t y, fixed_t z, angle_t angle,
                            angle_t ticangle, fixed_t pitch, boolean center, boolean lock);

// [Nugget] =================================================================/

void R_InitLightTables(void);                // killough 8/9/98
int R_GetLightIndex(fixed_t scale);

extern boolean setsizeneeded;
void R_ExecuteSetViewSize(void);

void R_InitAnyRes(void);

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0).  Used for interpolation.
extern fixed_t fractionaltic;

inline static fixed_t LerpFixed(fixed_t oldvalue, fixed_t newvalue)
{
    return (oldvalue + FixedMul(newvalue - oldvalue, fractionaltic));
}

// [AM] Interpolate between two angles.
inline static angle_t LerpAngle(angle_t oangle, angle_t nangle)
{
    if (nangle == oangle)
        return nangle;
    else if (nangle > oangle)
    {
        if (nangle - oangle < ANG270)
            return oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(fractionaltic));
        else // Wrapped around
            return oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(fractionaltic));
    }
    else // nangle < oangle
    {
        if (oangle - nangle < ANG270)
            return oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(fractionaltic));
        else // Wrapped around
            return oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(fractionaltic));
    }
}

extern boolean raw_input;

extern int autodetect_hom;

#endif

//----------------------------------------------------------------------------
//
// $Log: r_main.h,v $
// Revision 1.7  1998/05/03  23:00:42  killough
// beautification
//
// Revision 1.6  1998/04/06  04:43:17  killough
// Make colormaps fully dynamic
//
// Revision 1.5  1998/03/23  03:37:44  killough
// Add support for arbitrary number of colormaps
//
// Revision 1.4  1998/03/09  07:27:23  killough
// Avoid using FP for point/line queries
//
// Revision 1.3  1998/02/02  13:29:10  killough
// performance tuning
//
// Revision 1.2  1998/01/26  19:27:41  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:08  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
