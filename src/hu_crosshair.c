//
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//  Copyright (C) 2023-2024 Fabian Greffrath
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
// DESCRIPTION:  Crosshair HUD component
//
//-----------------------------------------------------------------------------

#include "hu_crosshair.h"
#include "d_items.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "m_swap.h"
#include "p_map.h"
#include "p_mobj.h"
#include "r_data.h"
#include "r_main.h"
#include "r_state.h"
#include "st_stuff.h"
#include "v_fmt.h"
#include "v_video.h"

// [Nugget]
#include "am_map.h"
#include "g_game.h"
#include "m_nughud.h"

static player_t *plr = players;

int hud_crosshair;

crosstarget_t hud_crosshair_target;
boolean hud_crosshair_health;
crosslockon_t hud_crosshair_lockon; // [Alaux] Crosshair locks on target
int hud_crosshair_color;
int hud_crosshair_target_color;

// [Nugget]
boolean hud_crosshair_on; // Crosshair toggle
int     hud_crosshair_tran_pct; // Translucent crosshair
boolean hud_crosshair_indicators; // Horizontal-autoaim indicators
boolean hud_crosshair_fuzzy; // Account for fuzzy targets

typedef struct
{
    patch_t *patch;
    int w, h, x, y;
    byte *cr;

    // [Nugget] Horizontal-autoaim indicators
    patch_t *lpatch, *rpatch;
    int lw, lh, rw, rh;
    int side;
} crosshair_t;

static crosshair_t crosshair;

const char *crosshair_lumps[HU_CROSSHAIRS] = {
    NULL,      "CROSS00", "CROSS01", "CROSS02", "CROSS03",
    "CROSS04", "CROSS05", "CROSS06", "CROSS07", "CROSS08"};

const char *crosshair_strings[HU_CROSSHAIRS] = {
    "Off",    "Cross",      "Angle",   "Dot",      "Big Cross",
    "Circle", "Big Circle", "Chevron", "Chevrons", "Arcs"};

void HU_InitCrosshair(void)
{
    for (int i = 1; i < HU_CROSSHAIRS; i++)
    {
        int lump = W_CheckNumForName(crosshair_lumps[i]);

        if (W_IsWADLump(lump))
        {
            if (R_IsPatchLump(lump))
            {
                crosshair_strings[i] = crosshair_lumps[i];
            }
            else
            {
                crosshair_lumps[i] = NULL;
            }
        }
    }
}

void HU_StartCrosshair(void)
{
    if (crosshair.patch)
    {
        Z_ChangeTag(crosshair.patch, PU_CACHE);
    }

    if (crosshair_lumps[hud_crosshair])
    {
        crosshair.patch =
            V_CachePatchName(crosshair_lumps[hud_crosshair], PU_STATIC);

        crosshair.w = SHORT(crosshair.patch->width) / 2;
        crosshair.h = SHORT(crosshair.patch->height) / 2;
    }
    else
    {
        crosshair.patch = NULL;
    }

    // [Nugget] Horizontal-autoaim indicators --------------------------------

    if (crosshair.lpatch) { Z_ChangeTag(crosshair.lpatch, PU_CACHE); }
    if (crosshair.rpatch) { Z_ChangeTag(crosshair.rpatch, PU_CACHE); }

    crosshair.lpatch = V_CachePatchName("CROSSIL", PU_STATIC);
    crosshair.lw = SHORT(crosshair.lpatch->width);
    crosshair.lh = SHORT(crosshair.lpatch->height)/2;

    crosshair.rpatch = V_CachePatchName("CROSSIR", PU_STATIC);
    crosshair.rw = SHORT(crosshair.rpatch->width);
    crosshair.rh = SHORT(crosshair.rpatch->height)/2;
}

mobj_t *crosshair_target; // [Alaux] Lock crosshair on target

void HU_UpdateCrosshair(void)
{
    plr = &players[displayplayer];

    crosshair.x = SCREENWIDTH / 2;
    crosshair.y = (screenblocks <= 10) ? (SCREENHEIGHT - ST_HEIGHT) / 2
                                       : SCREENHEIGHT / 2;

    // [Nugget] Freecam
    if (R_GetFreecamOn()) {
      crosshair.cr = colrngs[hud_crosshair_color];
      return;
    }

    crosshair.side = 0; // [Nugget] Horizontal-autoaim indicators

    if (hud_crosshair_health)
    {
        crosshair.cr = HU_ColorByHealth(plr->health, 100, st_invul);
    }
    else
    {
        crosshair.cr = colrngs[hud_crosshair_color];
    }

    if (STRICTMODE(hud_crosshair_target || hud_crosshair_lockon))
    {
        angle_t an = plr->mo->angle;
        ammotype_t ammo = weaponinfo[plr->readyweapon].ammo;
        fixed_t range = (ammo == am_noammo
                         && !(plr->readyweapon == wp_fist && plr->cheats & CF_SAITAMA)) // [Nugget]
                        ? MELEERANGE : 16 * 64 * FRACUNIT * NOTCASUALPLAY(comp_longautoaim+1); // [Nugget]
        boolean intercepts_overflow_enabled = overflow[emu_intercepts].enabled;
        int side = 0; // [Nugget]

        crosshair_target = linetarget = NULL;

        overflow[emu_intercepts].enabled = false;
        P_AimLineAttack(plr->mo, an, range, CROSSHAIR_AIM);
        if (!vertical_aiming && (ammo == am_misl || ammo == am_cell)
            && (!no_hor_autoaim || !casual_play)) // [Nugget]
        {
            if (!linetarget)
            {
                P_AimLineAttack(plr->mo, an + (1 << 26), range, CROSSHAIR_AIM);

                if (linetarget && hud_crosshair_indicators) { side = -1; } // [Nugget]
            }
            if (!linetarget)
            {
                P_AimLineAttack(plr->mo, an - (1 << 26), range, CROSSHAIR_AIM);

                if (linetarget && hud_crosshair_indicators) { side = 1; } // [Nugget]
            }
        }
        overflow[emu_intercepts].enabled = intercepts_overflow_enabled;

        if (linetarget && (!(linetarget->flags & MF_SHADOW)
                           || hud_crosshair_fuzzy)) // [Nugget]
        {
            crosshair_target = linetarget;
            crosshair.side = side; // [Nugget]
        }

        if (hud_crosshair_target && crosshair_target)
        {
            // [Alaux] Color crosshair by target health
            if (hud_crosshair_target == crosstarget_health)
            {
                crosshair.cr = HU_ColorByHealth(
                    crosshair_target->health,
                    crosshair_target->info->spawnhealth, false);
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

    x = viewwindowx + BETWEEN(w, viewwidth - w - 1, x);
    y = viewwindowy + BETWEEN(h, viewheight - h - 1, y);

    // [Nugget] Vertical-only lock-on
    if (hud_crosshair_lockon == crosslockon_full)
      crosshair.x = (x << FRACBITS) / video.xscale - video.deltaw;

    crosshair.y = (y << FRACBITS) / video.yscale;
}

void HU_DrawCrosshair(void)
{
    if (plr->playerstate != PST_LIVE || automapactive == AM_FULL || menuactive || paused
      // [Nugget] New conditions
      // Crash fix
      || !crosshair.cr
      // Chasecam
      || (R_GetChasecamOn() && !chasecam_crosshair)
      // Freecam
      || (R_GetFreecamOn() && (R_GetFreecamMode() != FREECAM_CAM || R_GetFreecamMobj()))
      // Alt. intermission background
      || (gamestate == GS_INTERMISSION))
    {
        return;
    }

    // [Nugget] NUGHUD
    const int y = crosshair.y + (nughud.viewoffset * STRICTMODE(st_crispyhud));

    if (crosshair.patch)
    {
        // [Nugget] Translucent crosshair
        V_DrawPatchTranslatedTL(crosshair.x - crosshair.w,
                                          y - crosshair.h, crosshair.patch,
                                crosshair.cr, xhair_tranmap);
    }

    // [Nugget] Horizontal-autoaim indicators --------------------------------

    if (crosshair.side == -1)
    {
        V_DrawPatchTranslatedTL(crosshair.x - crosshair.w - crosshair.lw,
                                          y - crosshair.lh,
                                crosshair.lpatch, crosshair.cr, xhair_tranmap);
    }
    else if (crosshair.side == 1)
    {
        V_DrawPatchTranslatedTL(crosshair.x + crosshair.w,
                                          y - crosshair.rh,
                                crosshair.rpatch, crosshair.cr, xhair_tranmap);
    }
}
