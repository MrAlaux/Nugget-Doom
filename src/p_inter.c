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
//      Handling interactions (i.e., collisions).
//
//-----------------------------------------------------------------------------

#include "p_inter.h"

#include "am_map.h"
#include "d_deh.h" // Ty 03/22/98 - externalized strings
#include "d_items.h"
#include "d_player.h"
#include "d_think.h"
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "info.h"
#include "m_fixed.h"
#include "m_random.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "p_tick.h"
#include "r_defs.h"
#include "r_main.h"
#include "s_sound.h"
#include "sounds.h"
#include "tables.h"

// [Nugget]
#include "p_maputl.h"
#include "p_user.h"
#include "v_video.h"

// [Nugget] cheese :)
extern boolean cheese;

#define BONUSADD        6

// Ty 03/07/98 - add deh externals
// Maximums and such were hardcoded values.  Need to externalize those for
// dehacked support (and future flexibility).  Most var names came from the key
// strings used in dehacked.

int initial_health = 100;
int initial_bullets = 50;
int maxhealth = 100; // was MAXHEALTH as a #define, used only in this module
int maxhealthbonus = 200;
int max_armor = 200;
int green_armor_class = 1;  // these are involved with armortype below
int blue_armor_class = 2;
int max_soul = 200;
int soul_health = 100;
int mega_health = 200;
int god_health = 100;   // these are used in cheats (see st_stuff.c)
int idfa_armor = 200;
int idfa_armor_class = 2;
// not actually used due to pairing of cheat_k and cheat_fa
int idkfa_armor = 200;
int idkfa_armor_class = 2;

int bfgcells = 40;      // used in p_pspr.c
int deh_species_infighting = 0;
// Ty 03/07/98 - end deh externals

// a weapon is found with two clip loads,
// a big item has five clip loads
int maxammo[NUMAMMO]  = {200, 50, 300, 50};
int clipammo[NUMAMMO] = { 10,  4,  20,  1};

//
// GET STUFF
//

// [Nugget]
static boolean P_AutoswitchWeapon(void)
{
  if (!casual_play || switch_on_pickup)
  { return true; }
  
  return false;
}

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

// mbf21: take into account new weapon autoswitch flags
static boolean P_GiveAmmoAutoSwitch(player_t *player, ammotype_t ammo, int oldammo)
{
  int i;

  if (
    weaponinfo[player->readyweapon].flags & WPF_AUTOSWITCHFROM &&
    weaponinfo[player->readyweapon].ammo != ammo
  )
  {
    for (i = NUMWEAPONS - 1; i > player->readyweapon; --i)
    {
      if (
        player->weaponowned[i] &&
        !(weaponinfo[i].flags & WPF_NOAUTOSWITCHTO) &&
        weaponinfo[i].ammo == ammo &&
        weaponinfo[i].ammopershot > oldammo &&
        weaponinfo[i].ammopershot <= player->ammo[ammo]
        && P_AutoswitchWeapon() // [Nugget]
      )
      {
        player->pendingweapon = i;
        break;
      }
    }
  }

  return true;
}

boolean P_GiveAmmo(player_t *player, ammotype_t ammo, int num)
{
  int oldammo;

  if (ammo == am_noammo)
    return false;

  if ((unsigned) ammo > NUMAMMO)
    I_Error ("P_GiveAmmo: bad type %i", ammo);

  if ( player->ammo[ammo] == player->maxammo[ammo]  )
    return false;

  if (num)
    num *= clipammo[ammo];
  else
    num = clipammo[ammo]/2;

  // give double ammo in trainer mode, you'll need in nightmare
  if (doubleammo) // [Nugget]
    num <<= 1;

  oldammo = player->ammo[ammo];
  player->ammo[ammo] += num;

  if (player->ammo[ammo] > player->maxammo[ammo])
    player->ammo[ammo] = player->maxammo[ammo];

  if (mbf21)
    return P_GiveAmmoAutoSwitch(player, ammo, oldammo);

  // If non zero ammo, don't change up weapons, player was lower on purpose.
  if (oldammo || !P_AutoswitchWeapon()) // [Nugget]
    return true;

  // We were down to zero, so select a new weapon.
  // Preferences are not user selectable.

  switch (ammo)
    {
    case am_clip:
      if (player->readyweapon == wp_fist)
      {
        if (player->weaponowned[wp_chaingun])
          player->pendingweapon = wp_chaingun;
        else
          player->pendingweapon = wp_pistol;
      }
      break;

    case am_shell:
      if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
      {
        if (player->weaponowned[wp_shotgun])
          player->pendingweapon = wp_shotgun;
      }
        break;

      case am_cell:
        if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
          if (player->weaponowned[wp_plasma])
            player->pendingweapon = wp_plasma;
        break;

      case am_misl:
        if (player->readyweapon == wp_fist)
          if (player->weaponowned[wp_missile])
            player->pendingweapon = wp_missile;
    default:
      break;
    }
  return true;
}

//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//

boolean P_GiveWeapon(player_t *player, weapontype_t weapon, boolean dropped)
{
  boolean gaveammo;

  if (netgame && deathmatch!=2 && !dropped)
    {
      // leave placed weapons forever on net games
      if (player->weaponowned[weapon])
        return false;

      player->bonuscount += BONUSADD;
      player->weaponowned[weapon] = true;

      P_GiveAmmo(player, weaponinfo[weapon].ammo, deathmatch ? 5 : 2);

      player->pendingweapon = weapon;
      S_StartSound(player->mo, sfx_wpnup); // killough 4/25/98, 12/98
      return false;
    }

  // give one clip with a dropped weapon, two clips with a found weapon
  gaveammo = weaponinfo[weapon].ammo != am_noammo &&
    P_GiveAmmo(player, weaponinfo[weapon].ammo, dropped ? 1 : 2);

  // [Nugget]
  if (!player->weaponowned[weapon] && P_AutoswitchWeapon())
  { player->pendingweapon = weapon; }

  return !player->weaponowned[weapon] ? player->weaponowned[weapon] = true : gaveammo;
}

//
// P_GiveBody
// Returns false if the body isn't needed at all
//

boolean P_GiveBody(player_t *player, int num)
{
  if (player->health >= maxhealth)
    return false; // Ty 03/09/98 externalized MAXHEALTH to maxhealth
  player->health += num;
  if (player->health > maxhealth)
    player->health = maxhealth;
  player->mo->health = player->health;
  return true;
}

//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//

boolean P_GiveArmor(player_t *player, int armortype)
{
  int hits = armortype*100;
  if (player->armorpoints >= hits)
    return false;   // don't pick up
  player->armortype = armortype;
  player->armorpoints = hits;
  return true;
}

//
// P_GiveCard
//

void P_GiveCard(player_t *player, card_t card)
{
  if (player->cards[card])
    return;
    
  // [Nugget] Fix for "key pickup resets palette"
  if (STRICTMODE(!comp_keypal))
  { player->bonuscount += BONUSADD; }
  else
  { player->bonuscount = BONUSADD; }
  
  player->cards[card] = 1;
  
  P_SetPlayerEvent(player, TIMER_KEYPICKUP); // [Nugget] Key pickup timer
}

//
// P_GivePower
//
// Rewritten by Lee Killough
//

boolean P_GivePower(player_t *player, int power)
{
  static const int tics[NUMPOWERS] = {
    INVULNTICS, 1 /* strength */, INVISTICS,
    IRONTICS, 1 /* allmap */, INFRATICS,
   };

  switch (power)
    {
      case pw_invisibility:
        player->mo->flags |= MF_SHADOW;
        break;
      case pw_allmap:
        if (player->powers[pw_allmap])
          return false;
        break;
      case pw_strength:
        P_GiveBody(player,100);
        break;
    }

  // Unless player has infinite duration cheat, set duration (killough)

  if (player->powers[power] >= 0)
    player->powers[power] = tics[power];
  return true;
}

//
// P_TouchSpecialThing
//

void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher)
{
  player_t *player;
  int      i;
  int      sound;
  fixed_t  delta = special->z - toucher->z;

  if (delta > toucher->height || delta < -8*FRACUNIT)
    return;        // out of reach

  sound = sfx_itemup;
  player = toucher->player;

  // Dead thing touching.
  // Can happen with a sliding player corpse.
  if (toucher->health <= 0)
    return;

    // Identify by sprite.
  switch (special->sprite)
    {
      // armor
    case SPR_ARM1:
      if (!P_GiveArmor (player, green_armor_class))
        return;
      pickupmsg(player, "%s", s_GOTARMOR); // Ty 03/22/98 - externalized
      break;

    case SPR_ARM2:
      if (!P_GiveArmor (player, blue_armor_class))
        return;
      pickupmsg(player, "%s", s_GOTMEGA); // Ty 03/22/98 - externalized
      break;

      // bonus items
    // [Nugget] cheese :)
    case SPR_TNT1:
      if (!cheese) { return; }
    case SPR_BON1:

      if (beta_emulation)
	{   // killough 7/11/98: beta version items did not have any effect
	  pickupmsg(player, "You pick up a demonic dagger.");
	  break;
	}

      player->health++;               // can go over 100%
      if (player->health > maxhealthbonus)
        player->health = maxhealthbonus;
      player->mo->health = player->health;
      // [Nugget] cheese :)
      pickupmsg(player, "%s", cheese ? "Picked up a cheese." : s_GOTHTHBONUS); // Ty 03/22/98 - externalized
      break;

    case SPR_BON2:

      if (beta_emulation)
	{ // killough 7/11/98: beta version items did not have any effect
	  pickupmsg(player, "You pick up a skullchest.");
	  break;
	}

      player->armorpoints++;          // can go over 100%
      if (player->armorpoints > max_armor)
        player->armorpoints = max_armor;
      if (!player->armortype)
        player->armortype = green_armor_class;
      pickupmsg(player, "%s", s_GOTARMBONUS); // Ty 03/22/98 - externalized
      break;

    case SPR_BON3:      // killough 7/11/98: evil sceptre from beta version
      pickupmsg(player, "Picked up an evil sceptre");
      break;

    case SPR_BON4:      // killough 7/11/98: unholy bible from beta version
      pickupmsg(player, "Picked up an unholy bible");
      break;

    case SPR_SOUL:
      player->health += soul_health;
      if (player->health > max_soul)
        player->health = max_soul;
      player->mo->health = player->health;
      pickupmsg(player, "%s", s_GOTSUPER); // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_MEGA:
      if (gamemode != commercial)
        return;
      player->health = mega_health;
      player->mo->health = player->health;
      P_GiveArmor (player,blue_armor_class);
      pickupmsg(player, "%s", s_GOTMSPHERE); // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

        // cards
        // leave cards for everyone
    case SPR_BKEY:
      if (!player->cards[it_bluecard])
        pickupmsg(player, "%s", s_GOTBLUECARD); // Ty 03/22/98 - externalized
      P_GiveCard (player, it_bluecard);
      sound = sfx_keyup; // [Nugget]: [NS] Optional key pickup sound.
      if (!netgame)
        break;
      return;

    case SPR_YKEY:
      if (!player->cards[it_yellowcard])
        pickupmsg(player, "%s", s_GOTYELWCARD); // Ty 03/22/98 - externalized
      P_GiveCard (player, it_yellowcard);
      sound = sfx_keyup; // [Nugget]: [NS] Optional key pickup sound.
      if (!netgame)
        break;
      return;

    case SPR_RKEY:
      if (!player->cards[it_redcard])
        pickupmsg(player, "%s", s_GOTREDCARD); // Ty 03/22/98 - externalized
      P_GiveCard (player, it_redcard);
      sound = sfx_keyup; // [Nugget]: [NS] Optional key pickup sound.
      if (!netgame)
        break;
      return;

    case SPR_BSKU:
      if (!player->cards[it_blueskull])
        pickupmsg(player, "%s", s_GOTBLUESKUL); // Ty 03/22/98 - externalized
      P_GiveCard (player, it_blueskull);
      sound = sfx_keyup; // [Nugget]: [NS] Optional key pickup sound.
      if (!netgame)
        break;
      return;

    case SPR_YSKU:
      if (!player->cards[it_yellowskull])
        pickupmsg(player, "%s", s_GOTYELWSKUL); // Ty 03/22/98 - externalized
      P_GiveCard (player, it_yellowskull);
      sound = sfx_keyup; // [Nugget]: [NS] Optional key pickup sound.
      if (!netgame)
        break;
      return;

    case SPR_RSKU:
      if (!player->cards[it_redskull])
        pickupmsg(player, "%s", s_GOTREDSKULL); // Ty 03/22/98 - externalized
      P_GiveCard (player, it_redskull);
      sound = sfx_keyup; // [Nugget]: [NS] Optional key pickup sound.
      if (!netgame)
        break;
      return;

      // medikits, heals
    case SPR_STIM:
      if (!P_GiveBody (player, 10))
        return;
      pickupmsg(player, "%s", s_GOTSTIM); // Ty 03/22/98 - externalized
      break;

    case SPR_MEDI:
      if (!P_GiveBody (player, 25))
        return;

      // [FG] show "Picked up a Medikit that you really need" message as intended
      if (player->health < 50)
        pickupmsg(player, "%s", s_GOTMEDINEED); // Ty 03/22/98 - externalized
      else
        pickupmsg(player, "%s", s_GOTMEDIKIT); // Ty 03/22/98 - externalized
      break;


      // power ups
    case SPR_PINV:
      if (!P_GivePower (player, pw_invulnerability))
        return;
      pickupmsg(player, "%s", s_GOTINVUL); // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_PSTR:
      if (!P_GivePower (player, pw_strength))
        return;
      pickupmsg(player, "%s", s_GOTBERSERK); // Ty 03/22/98 - externalized
      if (player->readyweapon != wp_fist)
	if ((!beta_emulation // killough 10/98: don't switch as much in -beta
	     || player->readyweapon == wp_pistol)
	    && P_AutoswitchWeapon()) // [Nugget]
	  player->pendingweapon = wp_fist;
      sound = sfx_getpow;
      break;

    case SPR_PINS:
      if (!P_GivePower (player, pw_invisibility))
        return;
      pickupmsg(player, "%s", s_GOTINVIS); // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_SUIT:
      if (!P_GivePower (player, pw_ironfeet))
        return;

      if (beta_emulation)  // killough 7/19/98: beta rad suit did not wear off
	player->powers[pw_ironfeet] = -1;

      pickupmsg(player, "%s", s_GOTSUIT); // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_PMAP:
      if (!P_GivePower (player, pw_allmap))
        return;
      pickupmsg(player, "%s", s_GOTMAP); // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_PVIS:

      if (!P_GivePower (player, pw_infrared))
        return;

      // killough 7/19/98: light-amp visor did not wear off in beta
      if (beta_emulation)
	player->powers[pw_infrared] = -1;

      sound = sfx_getpow;
      pickupmsg(player, "%s", s_GOTVISOR); // Ty 03/22/98 - externalized
      break;

      // ammo
    case SPR_CLIP:
      if (special->flags & MF_DROPPED)
        {
          if (!P_GiveAmmo (player,am_clip,0))
            return;
        }
      else
        {
          if (!P_GiveAmmo (player,am_clip,1))
            return;
        }
      pickupmsg(player, "%s", s_GOTCLIP); // Ty 03/22/98 - externalized
      break;

    case SPR_AMMO:
      if (!P_GiveAmmo (player, am_clip,5))
        return;
      pickupmsg(player, "%s", s_GOTCLIPBOX); // Ty 03/22/98 - externalized
      break;

    case SPR_ROCK:
      if (!P_GiveAmmo (player, am_misl,1))
        return;
      pickupmsg(player, "%s", s_GOTROCKET); // Ty 03/22/98 - externalized
      break;

    case SPR_BROK:
      if (!P_GiveAmmo (player, am_misl,5))
        return;
      pickupmsg(player, "%s", s_GOTROCKBOX); // Ty 03/22/98 - externalized
      break;

    case SPR_CELL:
      if (!P_GiveAmmo (player, am_cell,1))
        return;
      pickupmsg(player, "%s", s_GOTCELL); // Ty 03/22/98 - externalized
      break;

    case SPR_CELP:
      if (!P_GiveAmmo (player, am_cell,5))
        return;
      pickupmsg(player, "%s", s_GOTCELLBOX); // Ty 03/22/98 - externalized
      break;

    case SPR_SHEL:
      if (!P_GiveAmmo (player, am_shell,1))
        return;
      pickupmsg(player, "%s", s_GOTSHELLS); // Ty 03/22/98 - externalized
      break;

    case SPR_SBOX:
      if (!P_GiveAmmo (player, am_shell,5))
        return;
      pickupmsg(player, "%s", s_GOTSHELLBOX); // Ty 03/22/98 - externalized
      break;

    case SPR_BPAK:
      if (!player->backpack)
        {
          for (i=0 ; i<NUMAMMO ; i++)
            player->maxammo[i] *= 2;
          player->backpack = true;
        }
      for (i=0 ; i<NUMAMMO ; i++)
        P_GiveAmmo (player, i, 1);
      pickupmsg(player, "%s", s_GOTBACKPACK); // Ty 03/22/98 - externalized
      break;

        // weapons
    case SPR_BFUG:
      if (!P_GiveWeapon (player, wp_bfg, false) )
        return;
      if (STRICTMODE(classic_bfg) || beta_emulation)
        pickupmsg(player, "You got the BFG2704!  Oh, yes."); // killough 8/9/98: beta BFG
      else
        pickupmsg(player, "%s", s_GOTBFG9000); // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_MGUN:
      if (!P_GiveWeapon (player, wp_chaingun, special->flags & MF_DROPPED))
        return;
      pickupmsg(player, "%s", s_GOTCHAINGUN); // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_CSAW:
      if (!P_GiveWeapon(player, wp_chainsaw, false))
        return;
      pickupmsg(player, "%s", s_GOTCHAINSAW); // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_LAUN:
      if (!P_GiveWeapon (player, wp_missile, false) )
        return;
      pickupmsg(player, "%s", s_GOTLAUNCHER); // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_PLAS:
      if (!P_GiveWeapon(player, wp_plasma, false))
        return;
      pickupmsg(player, "%s", s_GOTPLASMA); // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_SHOT:
      if (!P_GiveWeapon(player, wp_shotgun, special->flags & MF_DROPPED))
        return;
      pickupmsg(player, "%s", s_GOTSHOTGUN); // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_SGN2:
      if (!P_GiveWeapon(player, wp_supershotgun, special->flags & MF_DROPPED))
        return;
      pickupmsg(player, "%s", s_GOTSHOTGUN2); // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    default:
      // I_Error("P_SpecialThing: Unknown gettable thing");
      return;      // killough 12/98: suppress error message
    }

  if (special->flags & MF_COUNTITEM)
  {
    player->itemcount++;

    // [Nugget] Announce milestone completion
    if (!(complete_milestones & MILESTONE_ITEMS))
    {
      int itemcount = 0;

      for (int pl = 0;  pl < MAXPLAYERS;  ++pl) {
        if (playeringame[pl])
        { itemcount += players[pl].itemcount; }
      }

      if (itemcount >= totalitems)
      {
        complete_milestones |= MILESTONE_ITEMS;

        if (announce_milestones) {
          players[displayplayer].secretmessage = "All items acquired!";
          S_StartSound(NULL, sfx_secret);
        }
      }
    }
  }
  P_RemoveMobj (special);
  player->bonuscount += BONUSADD;
  // [Nugget] Bonuscount cap
  if (STRICTMODE(bonuscount_cap >= 0 && player->bonuscount > bonuscount_cap))
  { player->bonuscount = bonuscount_cap; }

  S_StartSoundPitchOptional(player->mo, sound, sfx_itemup, // [Nugget]: [NS] Fallback to itemup.
                            sound == sfx_itemup ? PITCH_NONE : PITCH_FULL);
}

// [Nugget] Check for Extra Gibbing
static boolean P_NuggetExtraGibbing(mobj_t *source, mobj_t *target)
{
  extern void A_Punch(), A_Saw(), A_FireShotgun2();

  if (casual_play && extra_gibbing_on && source && source->player
      && (
          (extra_gibbing[EXGIB_FIST]
           && source->player->psprites->state->action.p2 == (actionf_p2)A_Punch
           && source->player->powers[pw_strength]
           && (P_AproxDistance(target->x - source->x, target->y - source->y)
               < ((64*FRACUNIT) + target->info->radius)))

       || (extra_gibbing[EXGIB_CSAW]
           && source->player->psprites->state->action.p2 == (actionf_p2)A_Saw
           && (P_AproxDistance(target->x - source->x, target->y - source->y)
               < ((65*FRACUNIT) + target->info->radius)))

       || (extra_gibbing[EXGIB_SSG]
           && source->player->psprites->state->action.p2 == (actionf_p2)A_FireShotgun2
           && (P_AproxDistance(target->x - source->x, target->y - source->y)
               < ((128*FRACUNIT) + target->info->radius)))
      )
     )
  {
    return true;
  }

  return false;
}

// [Nugget] Bloodier Gibbing
void P_NuggetGib(mobj_t *mo, const boolean crushed)
{
  int quantity;
  extern boolean idgaf;

  if (!casual_play || !bloodier_gibbing) { return; }
  
  quantity = crushed ? (4 + (Woof_Random() % 5)) : (20 + (Woof_Random() % 21));

  for (int i = 0; i < quantity; i++)
  {
    const fixed_t height = !crushed ? (mo->height * 3/2) : 0;

    mobj_t *splat = P_SpawnMobj(mo->x, mo->y, mo->z + height,
                                (comp_nonbleeders && mo->flags & MF_NOBLOOD)
                                ? MT_PUFF : MT_BLOOD);

    splat->flags |= MF_DROPOFF|MF_TELEPORT;

    if (comp_fuzzyblood && mo->flags & MF_SHADOW)
    { splat->flags |= MF_SHADOW; }

    if (mo->info->bloodcolor || idgaf)
    {
      splat->flags2 |= MF2_COLOREDBLOOD;
      splat->bloodcolor = V_BloodColor(mo->info->bloodcolor);
    }

    splat->momx = (Woof_Random() - Woof_Random()) << (12 - crushed);
    splat->momy = (Woof_Random() - Woof_Random()) << (12 - crushed);

    if (crushed) { splat->height = FRACUNIT; }
    else         { splat->momz = Woof_Random() << 12; }

    // Physics differ between versions (complevels),
    // so this is done to get rather-decent behavior in vanilla
    if (demo_version < DV_BOOM200) { splat->flags |= MF_NOCLIP; }

    splat->tics += (Woof_Random() & 3) - (Woof_Random() & 3);
    splat->tics = MAX(1, splat->tics);
  }
}

//
// KillMobj
//
// killough 11/98: make static

static void WatchKill(player_t* player, mobj_t* target)
{
  player->killcount++;

  if (target->intflags & MIF_SPAWNED_BY_ICON)
  {
    player->maxkilldiscount++;
  }
}

static void P_KillMobj(mobj_t *source, mobj_t *target, method_t mod)
{
  mobjtype_t item;
  mobj_t     *mo;
  extern boolean GIBBERS; // [Nugget] GIBBERS cheat

  target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY);

  if (target->type != MT_SKULL)
    target->flags &= ~MF_NOGRAVITY;

  target->flags |= MF_CORPSE|MF_DROPOFF;
  target->height >>= 2;

  // killough 8/29/98: remove from threaded list
  P_UpdateThinker(&target->thinker);

  if (source && source->player)
    {
      // count for intermission
      // killough 7/20/98: don't count friends
      if (!(target->flags & MF_FRIEND))
	if (target->flags & MF_COUNTKILL)
	  WatchKill(source->player, target);
      if (target->player)
        source->player->frags[target->player-players]++;
    }
    else
      if (!netgame && (target->flags & MF_COUNTKILL) )
        {
          // count all monster deaths,
          // even those caused by other monsters
	  // killough 7/20/98: don't count friends
	  if (!(target->flags & MF_FRIEND))
	    WatchKill(players, target);
        }
#ifndef MBF_STRICT
      // For compatibility with PrBoom+ complevel 11 netgame
      else
        if (netgame && !deathmatch && (target->flags & MF_COUNTKILL))
        {
          if (target->lastenemy && target->lastenemy->health > 0 &&
              target->lastenemy->player)
          {
              WatchKill(target->lastenemy->player, target);
          }
          else
          {
            int i, playerscount;

            for (i = 0, playerscount = 0; i < MAXPLAYERS; ++i)
            {
              if (playeringame[i])
                ++playerscount;
            }

            if (playerscount)
            {
              if (demo_version >= DV_MBF)
                i = P_Random(pr_friends) % playerscount;
              else
                i = Woof_Random() % playerscount;

              WatchKill(&players[i], target);
            }
          }
        }
#endif

  // [Nugget] Announce milestone completion
  if (!(complete_milestones & MILESTONE_KILLS))
  {
    int killcount = 0;

    for (int pl = 0;  pl < MAXPLAYERS;  ++pl) {
      if (playeringame[pl])
      { killcount += players[pl].killcount - players[pl].maxkilldiscount; }
    }

    if (killcount >= max_kill_requirement)
    {
      complete_milestones |= MILESTONE_KILLS;

      if (announce_milestones) {
        players[displayplayer].secretmessage = "All enemies killed!";
        S_StartSound(NULL, sfx_secret);
      }
    }
  }

  if (target->player)
    {
      // count environment kills against you
      if (!source)
        target->player->frags[target->player-players]++;

      target->flags &= ~MF_SOLID;
      target->player->playerstate = PST_DEAD;
      P_DropWeapon (target->player);
      // [crispy] center view when dying
      target->player->centering = true;

      if (target->player == &players[consoleplayer] && automapactive == AM_FULL)
	if (!demoplayback) // killough 11/98: don't switch out in demos, though
	  AM_ChangeMode(AM_OFF); // don't die in auto map; switch view prior to dying

      HU_Obituary(target, source, mod);
    }

  // [Nugget] Extra Gibbing/GIBBERS cheat
  if (target->info->xdeathstate
      && ((target->health < -target->info->spawnhealth)
          || GIBBERS || P_NuggetExtraGibbing(source, target)))
  {
    P_SetMobjState (target, target->info->xdeathstate);
    P_NuggetGib(target, false); // [Nugget] Bloodier Gibbing
  }
  else
    P_SetMobjState (target, target->info->deathstate);

  target->tics -= P_Random(pr_killtics)&3;

  // [crispy] randomly flip corpse, blood and death animation sprites
  if (target->flags2 & MF2_FLIPPABLE)
  {
    if (Woof_Random() & 1)
      target->intflags |= MIF_FLIP;
    else
      target->intflags &= ~MIF_FLIP;
  }

  if (target->tics < 1)
    target->tics = 1;

  // Drop stuff.
  // This determines the kind of object spawned
  // during the death frame of a thing.

  if (target->info->droppeditem != MT_NULL)
  {
    item = target->info->droppeditem;
  }
  else return;

  mo = P_SpawnMobj (target->x,target->y,ONFLOORZ, item);
  mo->flags |= MF_DROPPED;    // special versions of items

  // [Nugget] Toss items upon death
  if (casual_play && tossdrop)
  {
    mo->z += target->height*5/4;
    mo->momx = (Woof_Random() - Woof_Random()) << 7;
    mo->momy = (Woof_Random() - Woof_Random()) << 7;
    mo->momz = (4*FRACUNIT) + ((Woof_Random()%9) * FRACUNIT/8);
  }
}

//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//

// mbf21: dehacked infighting groups
static boolean P_InfightingImmune(mobj_t *target, mobj_t *source)
{
  return // not default behaviour, and same group
    mobjinfo[target->type].infighting_group != IG_DEFAULT &&
    mobjinfo[target->type].infighting_group == mobjinfo[source->type].infighting_group;
}

void P_DamageMobjBy(mobj_t *target,mobj_t *inflictor, mobj_t *source, int damage, method_t mod)
{
  player_t *player;
  boolean justhit;          // killough 11/98

  // killough 8/31/98: allow bouncers to take damage
  if (!(target->flags & (MF_SHOOTABLE | MF_BOUNCES)))
    return; // shouldn't happen...

  if (target->health <= 0)
    return;

  if (target->flags & MF_SKULLFLY)
    target->momx = target->momy = target->momz = 0;

  player = target->player;
  if (player && halfdamage) // [Nugget]
    damage >>= 1;   // take half damage in trainer mode

  // Some close combat weapons should not
  // inflict thrust and push the victim out of reach,
  // thus kick away unless using the chainsaw.

  if (inflictor && !(target->flags & MF_NOCLIP) &&
      (!source || !source->player
       || !(weaponinfo[source->player->readyweapon].flags & WPF_NOTHRUST)))
  {
    unsigned ang = R_PointToAngle2 (inflictor->x, inflictor->y,
                                      target->x,    target->y);

    fixed_t thrust = damage*(FRACUNIT>>3)*100/target->info->mass;

    // make fall forwards sometimes
    if ( damage < 40 && damage > target->health
        && target->z - inflictor->z > 64*FRACUNIT
        && P_Random(pr_damagemobj) & 1)
    {
      ang += ANG180;
      thrust *= 4;
    }

    ang >>= ANGLETOFINESHIFT;
    target->momx += FixedMul (thrust, finecosine[ang]);
    target->momy += FixedMul (thrust, finesine[ang]);

    // killough 11/98: thrust objects hanging off ledges
    if (target->intflags & MIF_FALLING && target->gear >= MAXGEAR)
      target->gear = 0;
  }

  // player specific
  if (player)
    {
      // end of game hell hack
      if (target->subsector->sector->special == 11 && damage >= target->health)
        damage = target->health - 1;

      // Below certain threshold,
      // ignore damage in GOD mode, or with INVUL power.
      // killough 3/26/98: make god mode 100% god mode in non-compat mode

      if ((damage < 1000 || (!comp[comp_god] && player->cheats&CF_GODMODE)) &&
          (player->cheats&CF_GODMODE || player->powers[pw_invulnerability]))
        return;

      if (player->armortype)
        {
          int saved = player->armortype == 1 ? damage/3 : damage/2;
          if (player->armorpoints <= saved)
            {
              // armor is used up
              saved = player->armorpoints;
              player->armortype = 0;
            }
          player->armorpoints -= saved;
          damage -= saved;
        }

      player->health -= damage;       // mirror mobj health here for Dave
      // BUDDHA cheat
      if (player->cheats & CF_BUDDHA &&
          player->health < 1)
        player->health = 1;
      else
      if (player->health < 0)
        player->health = 0;

      player->attacker = source;
      player->damagecount += damage;  // add damage after armor / invuln

      // [Nugget] Custom red tint cap
      if (STRICTMODE(player->damagecount > damagecount_cap))
        player->damagecount = damagecount_cap;
      else
      if (player->damagecount > 100)
        player->damagecount = 100;  // teleport stomp does 10k points...

#if 0
      // killough 11/98:
      // This is unused -- perhaps it was designed for
      // a hand-connected input device or VR helmet,
      // to pinch the player when they're hurt :)

      {
        int temp = damage < 100 ? damage : 100;

        if (player == &players[consoleplayer])
          I_Tactile (40,10,40+temp*2);
      }
#endif

      // [Nugget] Flinching: upon taking damage
      if (STRICTMODE(flinching & FLINCH_DAMAGE))
      {
        const int dmg = MAX(0, damage) / 2;
        int pitch;

        if (inflictor) { // Hitscan, melee, projectile, explosion
          pitch = -(dmg+1) * finecosine[(R_PointToAngle2(inflictor->x, inflictor->y,
                                                            target->x,    target->y)
                                         - player->mo->angle) >> ANGLETOFINESHIFT] / FRACUNIT;
        }
        else { pitch = dmg * ((Woof_Random() % 2) ? -1 : 1); }

        P_SetFlinch(player, pitch);
      }
    }

  // do the damage
  target->health -= damage;

  // BUDDHA cheat
  if (player && player->cheats & CF_BUDDHA &&
      target->health < 1)
  {
    target->health = 1;
  }
  else
  if (target->health <= 0)
    {
      P_KillMobj(source, target, mod);
      return;
    }

  // killough 9/7/98: keep track of targets so that friends can help friends
  if (demo_version >= DV_MBF)
    {
      // If target is a player, set player's target to source,
      // so that a friend can tell who's hurting a player
      if (player)
        P_SetTarget(&target->target, source);

      // killough 9/8/98:
      // If target's health is less than 50%, move it to the front of its list.
      // This will slightly increase the chances that enemies will choose to
      // "finish it off", but its main purpose is to alert friends of danger.
      if (target->health*2 < target->info->spawnhealth)
        {
          thinker_t *cap = &thinkerclasscap[target->flags & MF_FRIEND ?
                                           th_friends : th_enemies];
          (target->thinker.cprev->cnext = target->thinker.cnext)->cprev =
            target->thinker.cprev;
          (target->thinker.cnext = cap->cnext)->cprev = &target->thinker;
          (target->thinker.cprev = cap)->cnext = &target->thinker;
        }
    }

  if ((justhit = (P_Random (pr_painchance) < target->info->painchance &&
                  !(target->flags & MF_SKULLFLY)))) //killough 11/98: see below
    P_SetMobjState(target, target->info->painstate);

  target->reactiontime = 0;           // we're awake now...

  // killough 9/9/98: cleaned up, made more consistent:

  if (source && source != target && !(source->flags2 & MF2_DMGIGNORED) &&
      (!target->threshold || target->flags2 & MF2_NOTHRESHOLD) &&
      ((source->flags ^ target->flags) & MF_FRIEND || 
       monster_infighting || demo_version < DV_MBF) &&
      !P_InfightingImmune(target, source))
    {
      // if not intent on another player, chase after this one
      //
      // killough 2/15/98: remember last enemy, to prevent
      // sleeping early; 2/21/98: Place priority on players
      // killough 9/9/98: cleaned up, made more consistent:

      if (!target->lastenemy || target->lastenemy->health <= 0 ||
	  (demo_version < DV_MBF ? !target->lastenemy->player :
	   !((target->flags ^ target->lastenemy->flags) & MF_FRIEND) &&
	   target->target != source)) // remember last enemy - killough
	P_SetTarget(&target->lastenemy, target->target);

      P_SetTarget(&target->target, source);       // killough 11/98
      target->threshold = BASETHRESHOLD;
      if (target->state == &states[target->info->spawnstate]
          && target->info->seestate != S_NULL)
        P_SetMobjState (target, target->info->seestate);
    }

  // killough 11/98: Don't attack a friend, unless hit by that friend.
  if (justhit && (target->target == source || !target->target ||
                  !(target->flags & target->target->flags & MF_FRIEND)))
    target->flags |= MF_JUSTHIT;    // fight back!
}

//----------------------------------------------------------------------------
//
// $Log: p_inter.c,v $
// Revision 1.10  1998/05/03  23:09:29  killough
// beautification, fix #includes, move some global vars here
//
// Revision 1.9  1998/04/27  01:54:43  killough
// Prevent pickup sounds from silencing player weapons
//
// Revision 1.8  1998/03/28  17:58:27  killough
// Fix spawn telefrag bug
//
// Revision 1.7  1998/03/28  05:32:41  jim
// Text enabling changes for DEH
//
// Revision 1.6  1998/03/23  03:25:44  killough
// Fix weapon pickup sounds in spy mode
//
// Revision 1.5  1998/03/10  07:15:10  jim
// Initial DEH support added, minus text
//
// Revision 1.4  1998/02/23  04:44:33  killough
// Make monsters smarter
//
// Revision 1.3  1998/02/17  06:00:54  killough
// Save last enemy, change RNG calling sequence
//
// Revision 1.2  1998/01/26  19:24:05  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:59  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
