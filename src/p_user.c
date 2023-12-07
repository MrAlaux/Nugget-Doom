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
//      Player related stuff.
//      Bobbing POV/weapon, movement.
//      Pending weapon.
//
//-----------------------------------------------------------------------------

#include "doomstat.h"
#include "d_event.h"
#include "r_main.h"
#include "p_map.h"
#include "p_spec.h"
#include "p_user.h"
#include "g_game.h"
// [Nugget]
#include "m_input.h"
#include "s_sound.h"
#include "sounds.h"

// Index of the special effects (INVUL inverse) map.

#define INVERSECOLORMAP 32

//
// Movement.
//

// 16 pixels of bob

#define MAXBOB  0x100000

boolean onground; // whether player is on ground or in air

// [Nugget]
#define CROUCHUNITS 3*FRACUNIT

//
// P_Thrust
// Moves the given origin along a given angle.
//

void P_Thrust(player_t* player,angle_t angle,fixed_t move)
{
  player->mo->momx += FixedMul(move,finecosine[angle >>= ANGLETOFINESHIFT]);
  player->mo->momy += FixedMul(move,finesine[angle]);
}

//
// P_Bob
// Same as P_Thrust, but only affects bobbing.
//
// killough 10/98: We apply thrust separately between the real physical player
// and the part which affects bobbing. This way, bobbing only comes from player
// motion, nothing external, avoiding many problems, e.g. bobbing should not
// occur on conveyors, unless the player walks on one, and bobbing should be
// reduced at a regular rate, even on ice (where the player coasts).
//

void P_Bob(player_t *player, angle_t angle, fixed_t move)
{
  if (demo_version < 203)
    return;

  player->momx += FixedMul(move,finecosine[angle >>= ANGLETOFINESHIFT]);
  player->momy += FixedMul(move,finesine[angle]);
}

//
// P_CalcHeight
// Calculate the walking / running height adjustment
//

void P_CalcHeight (player_t* player)
{
  int     angle;
  fixed_t bob;
  // [Nugget] Adjustable viewheight
  const fixed_t view = (!strictmode ? viewheight_value*FRACUNIT : VIEWHEIGHT);

  // Regular movement bobbing
  // (needs to be calculated for gun swing
  // even if not on ground)
  // OPTIMIZE: tablify angle
  // Note: a LUT allows for effects
  //  like a ramp with low health.

  // killough 10/98: Make bobbing depend only on player-applied motion.
  //
  // Note: don't reduce bobbing here if on ice: if you reduce bobbing here,
  // it causes bobbing jerkiness when the player moves from ice to non-ice,
  // and vice-versa.

  // [FG] MBF player bobbing rewrite causes demo sync problems
  // http://prboom.sourceforge.net/mbf-bugs.html
  player->bob = (demo_version >= 203 && player_bobbing) ?
      (FixedMul(player->momx,player->momx)
      + FixedMul(player->momy,player->momy))>>2 :
      (demo_compatibility || player_bobbing) ?
      (FixedMul (player->mo->momx, player->mo->momx)
      + FixedMul (player->mo->momy,player->mo->momy))>>2 : 0;

  if ((demo_version == 202 || demo_version == 203) &&
      player->mo->friction > ORIG_FRICTION) // ice?
  {
    if (player->bob > (MAXBOB>>2))
      player->bob = MAXBOB>>2;
  }
  else
  {
  if (player->bob > MAXBOB)                             
    player->bob = MAXBOB;
  }

  if (!onground || player->cheats & CF_NOMOMENTUM)
    {
      // [Nugget] Account for crouching
      player->viewz = player->mo->z + view - player->crouchoffset;

      if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
        player->viewz = player->mo->ceilingz-4*FRACUNIT;

      // phares 2/25/98:
      // The following line was in the Id source and appears
      // to be a bug. player->viewz is checked in a similar
      // manner at a different exit below.

      // player->viewz = player->mo->z + player->viewheight;

      return;
    }

  angle = (FINEANGLES/20*leveltime)&FINEMASK;

  bob = player->bob;

  // [Nugget] View bobbing percentage setting
  if (view_bobbing_percentage != 100)
  { bob = FixedDiv(FixedMul(bob, view_bobbing_percentage), 100); }

  bob = FixedMul(bob / 2, finesine[angle]);

  // move viewheight

  if (player->playerstate == PST_LIVE)
    {
      player->viewheight += player->deltaviewheight;

      // [Nugget]: [JN] Imitate player's breathing.
      if (STRICTMODE(breathing))
      {
        static fixed_t breathing_val = 0;
        static boolean breathing_dir = 0;
        const fixed_t BREATHING_STEP = 32, BREATHING_MAX = 1408;

        if (breathing_dir) { // Inhale (camera up)
          breathing_val += BREATHING_STEP;
          if (breathing_val >= BREATHING_MAX)
          { breathing_dir = false; }
        }
        else { // Exhale (camera down)
          breathing_val -= BREATHING_STEP;
          if (breathing_val <= -BREATHING_MAX)
          { breathing_dir = true; }
        }

        player->viewheight += breathing_val;
      }
            
      if (player->viewheight > view)
        {
          player->viewheight = view;
          player->deltaviewheight = 0;
        }

      if (player->viewheight < view/2)
        {
          player->viewheight = view/2;
          if (player->deltaviewheight <= 0)
            player->deltaviewheight = 1;
        }

      if (player->deltaviewheight)
        {
          player->deltaviewheight += FRACUNIT/4;
          if (!player->deltaviewheight)
            player->deltaviewheight = 1;
        }
    }

  // [Nugget] Account for crouching
  player->viewz = player->mo->z + player->viewheight + bob - player->crouchoffset;
  
  if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
    player->viewz = player->mo->ceilingz-4*FRACUNIT;
}

//
// P_MovePlayer
//
// Adds momentum if the player is not in the air
//
// killough 10/98: simplified

void P_MovePlayer (player_t* player)
{
  ticcmd_t *cmd = &player->cmd;
  mobj_t *mo = player->mo;
  // [Nugget]
  static boolean crouchKeyDown = false;
  int cforwardmove, csidemove;

  mo->angle += cmd->angleturn << 16;
  onground = (mo->z <= mo->floorz
              // [Nugget]
              // On top of a mobj
              || (mo->below_thing && (mo->z == (mo->below_thing->z + mo->below_thing->height)))
              // Mid-air control with noclip or flight cheat enabled
              || (player->mo->flags & MF_NOCLIP) || (player->cheats & CF_FLY));

  // [Nugget]
  if (player->cheats & CF_FLY)
  {
    player->mo->flags |= MF_NOGRAVITY;

    if (!(M_InputGameActive(input_jump) ^ M_InputGameActive(input_crouch)))
    { // Stop moving...
      if (player->cheats & CF_NOMOMENTUM)
      { player->mo->momz = 0; } // ... instantly
      else { // ... slowly
        if (player->mo->momz > 0) {
          player->mo->momz -= 1*FRACUNIT;
          if (player->mo->momz < 0)
          { player->mo->momz = 0; }
        }
        else if (player->mo->momz < 0) {
          player->mo->momz += 1*FRACUNIT;
          if (player->mo->momz > 0)
          { player->mo->momz = 0; }
        }
      }
    }
  }

  // [Nugget] Jumping delay
  if (player->jumptics) { player->jumptics--; }

  // [Nugget] Jump/Fly Up
  if (casual_play && M_InputGameActive(input_jump))
  {
    if (player->cheats & CF_FLY) {
      player->mo->momz += (1 + (autorun ^ M_InputGameActive(input_speed))) * FRACUNIT;
      if (player->mo->momz > 8*FRACUNIT) { player->mo->momz = 8*FRACUNIT; }
    }
    else if (jump_crouch) {
      if (player->mo->intflags & MIF_CROUCHING)
      {
        player->mo->intflags &= ~MIF_CROUCHING; // Stand up first
      }
      else if (onground && !(player->jumptics)
               && (player->mo->height == player->mo->info->height)
               && ((player->mo->ceilingz - player->mo->floorz) > player->mo->height))
      { // Jump
        player->mo->momz = 8*FRACUNIT;
        player->jumptics = 20;

        // [NS] Jump sound.
        S_StartSoundOptional(player->mo, sfx_pljump, -1);

        // [crispy] squat down weapon sprite a bit
        if (STRICTMODE(weaponsquat))
        { player->psprites[ps_weapon].dy = player->mo->momz>>1; }
      }
    }
  }

  // [Nugget] Crouch/Fly Down
  if (!M_InputGameActive(input_crouch))
  {
    crouchKeyDown = false;
  }
  else if (casual_play)
  {
    if (player->cheats & CF_FLY) {
      player->mo->momz -= (1 + (autorun ^ M_InputGameActive(input_speed))) * FRACUNIT;
      if (player->mo->momz < -8*FRACUNIT) { player->mo->momz = -8*FRACUNIT; }
    }
    else if (jump_crouch && crouchKeyDown == false)
    {
      crouchKeyDown = true;
      player->mo->intflags ^= MIF_CROUCHING;
    }
  }

  // [Nugget] Forcefully stand up under certain conditions
  if ((player->mo->intflags & MIF_CROUCHING)
      && (!jump_crouch || player->cheats & CF_FLY || chasecam_mode))
  {
    player->mo->intflags &= ~MIF_CROUCHING;
  }

  // [Nugget] Smooth crouching
  if (   ((player->mo->intflags & MIF_CROUCHING)
          && (player->mo->height > player->mo->info->height/2))
      || (!(player->mo->intflags & MIF_CROUCHING)
          && (player->mo->height < (player->mo->ceilingz - player->mo->floorz))
          && (player->mo->height < player->mo->info->height)))
  {
    const int sign = ((player->mo->intflags & MIF_CROUCHING)
                      && (player->mo->height > player->mo->info->height/2)) ? -1 : 1;
    const fixed_t step = (onground ? CROUCHUNITS : CROUCHUNITS*2) * sign;
    const fixed_t heighttarget = player->mo->info->height / ((sign > 0) ? 1 : 2),
                  offsettarget = (sign > 0) ? 0 : viewheight_value * FRACUNIT / 2;

    player->mo->height += step;

    // Make `crouchoffset` take as many steps as `height` to reach its target
    player->crouchoffset -= (viewheight_value * FRACUNIT / 2)
                            / ((float) (player->mo->info->height / 2) / step);

    if (!onground) { // Crouch jumping!
      player->mo->z -= step;
      if (player->mo->z < player->mo->floorz)
      { player->mo->z = player->mo->floorz; }
    }

    if (player->mo->height > (player->mo->ceilingz - player->mo->floorz))
    {
      player->crouchoffset += player->mo->height
                              - (player->mo->ceilingz - player->mo->floorz);
      player->mo->height = player->mo->ceilingz - player->mo->floorz;
    }

    if ((player->mo->height * sign) >= (heighttarget * sign))
    {
      // Done crouching
      if (!onground && (sign < 0)) // Take away excess Z shifting
      { player->mo->z -= heighttarget - player->mo->height; }
      player->mo->height = heighttarget;
      player->crouchoffset = offsettarget;
    }

    if ((player->crouchoffset * (-sign)) > (offsettarget * (-sign)))
    { player->crouchoffset = offsettarget; }
  }

  // killough 10/98:
  //
  // We must apply thrust to the player and bobbing separately, to avoid
  // anomalies. The thrust applied to bobbing is always the same strength on
  // ice, because the player still "works just as hard" to move, while the
  // thrust applied to the movement varies with 'movefactor'.

  if ((!demo_compatibility && demo_version < 203) ||
      cmd->forwardmove | cmd->sidemove) // killough 10/98
    {
      if (onground || mo->flags & MF_BOUNCES) // killough 8/9/98
        {
          int friction, movefactor = P_GetMoveFactor(mo, &friction);

          // killough 11/98:
          // On sludge, make bobbing depend on efficiency.
          // On ice, make it depend on effort.

          int bobfactor =
            friction < ORIG_FRICTION ? movefactor : ORIG_FRICTION_FACTOR;

          // [Nugget]
          cforwardmove = cmd->forwardmove;
          csidemove = cmd->sidemove;
          // Check for crouching
          if (player->mo->intflags & MIF_CROUCHING)
          { cforwardmove /= 2; csidemove /= 2; }

          if (cmd->forwardmove)
            {
              P_Bob(player,mo->angle,cforwardmove*bobfactor);
              P_Thrust(player,mo->angle,cforwardmove*movefactor);
            }

          if (cmd->sidemove)
            {
              P_Bob(player,mo->angle-ANG90,csidemove*bobfactor);
              P_Thrust(player,mo->angle-ANG90,csidemove*movefactor);
            }
        }
      // [Nugget] Allow minimal mid-air movement if Jumping is enabled
      else if (casual_play && !onground && jump_crouch)
      {
        if (cmd->forwardmove)
        { P_Thrust(player,mo->angle,cmd->forwardmove); }
        if (cmd->sidemove)
        { P_Thrust(player,mo->angle-ANG90,cmd->sidemove); }
      }
      
      // Add (cmd-> forwardmove || cmd-> sidemove) check to prevent the players
      // always in S_PLAY_RUN1 animation in complevel Boom.
      if ((cmd->forwardmove || cmd->sidemove) &&
          (mo->state == states+S_PLAY))
        P_SetMobjState(mo,S_PLAY_RUN1);
    }

  if (!menuactive && !demoplayback)
  {
    player->lookdir = BETWEEN(-LOOKDIRMAX * MLOOKUNIT,
                               LOOKDIRMAX * MLOOKUNIT,
                               player->lookdir + cmd->lookdir);
    player->slope = PLAYER_SLOPE(player);
  }
}

#define ANG5 (ANG90/18)

typedef enum
{
  death_use_default,
  death_use_reload,
  death_use_nothing
} death_use_action_t;

death_use_action_t death_use_action;

//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//

void P_DeathThink (player_t* player)
{
  angle_t angle;
  angle_t delta;

  P_MovePsprites (player);

  // fall to the ground

  if (player->viewheight > 6*FRACUNIT)
    player->viewheight -= FRACUNIT;

  if (player->viewheight < 6*FRACUNIT)
    player->viewheight = 6*FRACUNIT;

  // [Nugget] Gradually decrease the crouching offset;
  // should give a nice arching POV effect, appropriate for the context
  if (player->crouchoffset)
  {
    int step = player->crouchoffset/4;
    if (step < FRACUNIT) { step = FRACUNIT; }

    player->crouchoffset -= step;
    if (player->crouchoffset < 0) { player->crouchoffset = 0; }
  }

  player->deltaviewheight = 0;
  onground = (player->mo->z <= player->mo->floorz);
  P_CalcHeight (player);

  if (player->attacker && player->attacker != player->mo)
    {
      angle = R_PointToAngle2 (player->mo->x,
                               player->mo->y,
                               player->attacker->x,
                               player->attacker->y);

      delta = angle - player->mo->angle;

      if (delta < ANG5 || delta > (unsigned)-ANG5)
        {
          // Looking at killer,
          //  so fade damage flash down.

          player->mo->angle = angle;

          if (player->damagecount)
            player->damagecount--;
        }
      else
        if (delta < ANG180)
          player->mo->angle += ANG5;
        else
          player->mo->angle -= ANG5;
    }
  else
    if (player->damagecount)
      player->damagecount--;

  // [Nugget] Allow some freelook while dead
  if ((player->viewheight == 6*FRACUNIT) && !menuactive && !demoplayback)
  {
    player->lookdir = BETWEEN(-LOOKDIRMAX * MLOOKUNIT / 2,
                               LOOKDIRMAX * MLOOKUNIT / 2,
                               player->lookdir + player->cmd.lookdir);
  }

  if (player->cmd.buttons & BT_USE)
  {
    if (demorecording || demoplayback || netgame)
      player->playerstate = PST_REBORN;
    else switch(death_use_action)
    {
      case death_use_default:
        player->playerstate = PST_REBORN;
        break;
      case death_use_reload:
        if (savegameslot >= 0)
        {
          char *file = G_SaveGameName(savegameslot);
          G_LoadGame(file, savegameslot, false);
          free(file);
        }
        else
          player->playerstate = PST_REBORN;
        break;
      case death_use_nothing:
      default:
        break;
    }
  }
}

// [Nugget] Event Timers
void P_SetPlayerEvent(player_t* player, eventtimer_t type)
{
  if (!STRICTMODE(event_timers[type] == 2
                  || (event_timers[type] && (demorecording||demoplayback))))
  {
    return;
  }

  player->eventtype = type;
  player->eventtime = leveltime;
  player->eventtics = 5*TICRATE/2; // [crispy] 2.5 seconds
}

//
// P_PlayerThink
//

void P_PlayerThink (player_t* player)
{
  ticcmd_t*    cmd;
  weapontype_t newweapon;
  static boolean zoomKeyDown = false; // [Nugget]

  // [AM] Assume we can interpolate at the beginning
  //      of the tic.
  player->mo->interp = true;

  // [AM] Store starting position for player interpolation.
  player->mo->oldx = player->mo->x;
  player->mo->oldy = player->mo->y;
  player->mo->oldz = player->mo->z;
  player->mo->oldangle = player->mo->angle;
  player->oldviewz = player->viewz;
  player->oldlookdir = player->lookdir;
  player->oldrecoilpitch = player->recoilpitch;
  player->oldimpactpitch = player->impactpitch; // [Nugget] Impact pitch

  // killough 2/8/98, 3/21/98:
  // (this code is necessary despite questions raised elsewhere in a comment)

  if (player->cheats & CF_NOCLIP)
    player->mo->flags |= MF_NOCLIP;
  else
    player->mo->flags &= ~MF_NOCLIP;

  // chain saw run forward

  cmd = &player->cmd;
  if (player->mo->flags & MF_JUSTATTACKED)
    {
      cmd->angleturn = 0;
      cmd->forwardmove = 0xc800/512;
      cmd->sidemove = 0;
      player->mo->flags &= ~MF_JUSTATTACKED;
    }

  // [crispy] center view
  if (player->centering)
  {
    if (player->lookdir > 0)
    {
      player->lookdir -= 8 * MLOOKUNIT;
    }
    else if (player->lookdir < 0)
    {
      player->lookdir += 8 * MLOOKUNIT;
    }
    if (abs(player->lookdir) < 8 * MLOOKUNIT)
    {
      player->lookdir = 0;
      player->centering = false;
    }
  }

  // [crispy] weapon recoil pitch
  if (player->recoilpitch)
  {
    if (player->recoilpitch > 0)
    {
      player->recoilpitch -= 1;
    }
    else if (player->recoilpitch < 0)
    {
      player->recoilpitch += 1;
    }
  }

  // [Nugget] Impact pitch
  if (player->impactpitch) {
         if (player->impactpitch > 0) { player->impactpitch--; }
    else if (player->impactpitch < 0) { player->impactpitch++; }
  }

  if (player->playerstate == PST_DEAD)
    {
      // [Nugget] Disable zoom upon death
      if (R_GetZoom() == 1) { R_SetZoom(ZOOM_OFF); }

      player->slope = PLAYER_SLOPE(player); // For 3D audio pitch angle.
      P_DeathThink (player);
      return;
    }

  if (!M_InputGameActive(input_zoom))
  {
    zoomKeyDown = false;
  }
  else if (STRICTMODE(zoomKeyDown == false))
  {
    zoomKeyDown = true;
    R_SetZoom(!R_GetZoom());
  }

  // Move around.
  // Reactiontime is used to prevent movement
  //  for a bit after a teleport.

  if (player->mo->reactiontime)
    player->mo->reactiontime--;
  else
    P_MovePlayer (player);

  P_CalcHeight (player); // Determines view height and bobbing

  // Determine if there's anything about the sector you're in that's
  // going to affect you, like painful floors.
  if (player->mo->subsector->sector->special)
    P_PlayerInSpecialSector (player);

  // Sprite Height problem...                                         // phares
  // Future code:                                                     //  |
  // It's possible that at this point the player is standing on top   //  V
  // of a Thing that could cause him some damage, like a torch or
  // burning barrel. We need a way to generalize Thing damage by
  // grabbing a bit in the Thing's options to indicate damage. Since
  // this is competing with other attributes we may want to add,
  // we'll put this off for future consideration when more is
  // known.

  // Future Code:                                                     //  ^
  // Check to see if the object you've been standing on has moved     //  |
  // out from underneath you.                                         // phares

  // Check for weapon change.

  // A special event has no other buttons.

  if (cmd->buttons & BT_SPECIAL)
    cmd->buttons = 0;

  if (cmd->buttons & BT_CHANGE
      || (casual_play && M_InputGameActive(input_lastweapon))) // [Nugget] Last weapon key
    {
      // [Nugget] Last weapon key
      const weapontype_t lastweapon = ((casual_play && M_InputGameActive(input_lastweapon))
                                       ? player->lastweapon : wp_nochange);

      // The actual changing of the weapon is done
      //  when the weapon psprite can do it
      //  (read: not in the middle of an attack).

      // [Nugget]
      if (lastweapon != wp_nochange)
        newweapon = lastweapon;
      else
        newweapon = (cmd->buttons & BT_WEAPONMASK)>>BT_WEAPONSHIFT;

      // killough 3/22/98: For demo compatibility we must perform the fist
      // and SSG weapons switches here, rather than in G_BuildTiccmd(). For
      // other games which rely on user preferences, we must use the latter.

      if (demo_compatibility)
        { // compatibility mode -- required for old demos -- killough
          if (newweapon == wp_fist && player->weaponowned[wp_chainsaw] &&
              ((player->readyweapon != wp_chainsaw
                && lastweapon != wp_fist) || // [Nugget]
               !player->powers[pw_strength]))
            newweapon = wp_chainsaw;
          if (have_ssg &&
              newweapon == wp_shotgun &&
              player->weaponowned[wp_supershotgun] &&
              player->readyweapon != wp_supershotgun &&
              lastweapon != wp_shotgun) // [Nugget]
            newweapon = wp_supershotgun;
        }

      // killough 2/8/98, 3/22/98 -- end of weapon selection changes

      if (player->weaponowned[newweapon] && newweapon != player->readyweapon)

        // Do not go to plasma or BFG in shareware,
        //  even if cheated.

        if ((newweapon != wp_plasma && newweapon != wp_bfg)
            || (gamemode != shareware) )
          player->pendingweapon = newweapon;
    }

  // check for use

  if (cmd->buttons & BT_USE)
    {
      if (!player->usedown)
        {
          P_UseLines (player);
          player->usedown = true;
          
          P_SetPlayerEvent(player, TIMER_USE); // [Nugget] "Use" button timer
        }
    }
  else
    player->usedown = false;

  // cycle psprites

  P_MovePsprites (player);

  // Counters, time dependent power ups.

  // Strength counts up to diminish fade.

  if (player->powers[pw_strength])
    player->powers[pw_strength]++;

  // killough 1/98: Make idbeholdx toggle:

  if (player->powers[pw_invulnerability] > 0) // killough
    player->powers[pw_invulnerability]--;

  if (player->powers[pw_invisibility] > 0)    // killough
    if (! --player->powers[pw_invisibility] )
      player->mo->flags &= ~MF_SHADOW;

  if (player->powers[pw_infrared] > 0)        // killough
    player->powers[pw_infrared]--;

  if (player->powers[pw_ironfeet] > 0)        // killough
    player->powers[pw_ironfeet]--;

  // [Nugget] Fast weapons cheat
  if (player->cheats & CF_FASTWEAPS)
  {
    if (player->psprites[ps_weapon].tics >= 1)
    { player->psprites[ps_weapon].tics = 1; }

    if (player->psprites[ps_flash].tics >= 1)
    { player->psprites[ps_flash].tics = 1; }
  }

  // [Nugget] Linetarget Query cheat
  if (player->cheats & CF_LINETARGET)
  {
    boolean intercepts_overflow_enabled = overflow[emu_intercepts].enabled;

    overflow[emu_intercepts].enabled = false;
    P_AimLineAttack(player->mo, player->mo->angle, 16*64*FRACUNIT * (comp_longautoaim+1), 0);
    overflow[emu_intercepts].enabled = intercepts_overflow_enabled;

    if (linetarget)
    {
      // Give some info on the thing
      displaymsg("Type: %i - Health: %i/%i", linetarget->type,
                 linetarget->health, linetarget->info->spawnhealth);
    }
  }

  if (player->cheats & CF_RENDERSTATS)
  {
    extern void R_ShowRenderingStats();
    R_ShowRenderingStats();
  }

  if (player->cheats & CF_MAPCOORDS)
  {
    extern void cheat_mypos_print();
    cheat_mypos_print();
  }

  if (player->damagecount)
    player->damagecount--;

  if (player->bonuscount)
    player->bonuscount--;

  // Handling colormaps.
  // killough 3/20/98: reformat to terse C syntax

  // killough 7/11/98: beta version had invisibility, instead of
  // invulernability, and the light amp visor used the last colormap.
  // But white flashes occurred when invulnerability wore off.

  // [Nugget]: [crispy] A11Y
  if (STRICTMODE(!a11y_invul_colormap))
  {
    if (player->powers[pw_invulnerability] || player->powers[pw_infrared])
    { player->fixedcolormap = 1; }
    else
    { player->fixedcolormap = 0; }
  }
  else
  player->fixedcolormap = 

    beta_emulation ?    /* Beta Emulation */
    player->powers[pw_infrared] > 4*32 ||
    player->powers[pw_infrared] & 8 ? 32 :
    player->powers[pw_invisibility] > 4*32 ||
    player->powers[pw_invisibility] & 8 ||
    (player->powers[pw_invulnerability] < 4*32 &&
     player->powers[pw_invulnerability] > 0 &&
     player->powers[pw_invulnerability] & 8) ? 33 : 0 :

    player->powers[pw_invulnerability] > 4*32 ||    /* Regular Doom */
    player->powers[pw_invulnerability] & 8 ? INVERSECOLORMAP :
    player->powers[pw_infrared] > 4*32 || player->powers[pw_infrared] & 8;
}

//----------------------------------------------------------------------------
//
// $Log: p_user.c,v $
// Revision 1.14  1998/05/12  12:47:25  phares
// Removed OVER_UNDER code
//
// Revision 1.13  1998/05/10  23:38:04  killough
// Add #include p_user.h to ensure consistent prototypes
//
// Revision 1.12  1998/05/05  15:35:20  phares
// Documentation and Reformatting changes
//
// Revision 1.11  1998/05/03  23:21:04  killough
// Fix #includes and remove unnecessary decls at the top, nothing else
//
// Revision 1.10  1998/03/23  15:24:50  phares
// Changed pushers to linedef control
//
// Revision 1.9  1998/03/23  03:35:24  killough
// Move weapons changes to G_BuildTiccmd, fix idclip
//
// Revision 1.8  1998/03/12  14:28:50  phares
// friction and IDCLIP changes
//
// Revision 1.7  1998/03/09  18:26:55  phares
// Fixed bug in neighboring variable friction sectors
//
// Revision 1.6  1998/02/27  08:10:08  phares
// Added optional player bobbing
//
// Revision 1.5  1998/02/24  08:46:42  phares
// Pushers, recoil, new friction, and over/under work
//
// Revision 1.4  1998/02/15  02:47:57  phares
// User-defined keys
//
// Revision 1.3  1998/02/09  03:13:20  killough
// Improve weapon control and add preferences
//
// Revision 1.2  1998/01/26  19:24:34  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:01  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
