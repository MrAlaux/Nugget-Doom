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
//
// DESCRIPTION:  Platform-independent sound code
//
//-----------------------------------------------------------------------------

// killough 3/7/98: modified to allow arbitrary listeners in spy mode
// killough 5/2/98: reindented, removed useless code, beautified

#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "g_umapinfo.h"
#include "i_printf.h"
#include "i_rumble.h"
#include "i_sound.h"
#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_mobj.h"
#include "s_musinfo.h" // [crispy] struct musinfo
#include "s_sound.h"
#include "s_trakinfo.h"
#include "sounds.h"
#include "w_wad.h"
#include "z_zone.h"

// [Nugget]
#include "g_game.h"
#include "r_main.h"
#include "r_state.h"

// jff end sound enabling variables readable here

typedef struct channel_s
{
    sfxinfo_t *sfxinfo;   // sound information (if null, channel avail.)
    const mobj_t *origin; // origin of sound
    int volume;           // volume scale value for effect -- haleyjd 05/29/06
    int handle;           // handle of the sound being played
    int o_priority;       // haleyjd 09/27/06: stored priority value
    int priority;         // current priority value
    int singularity;      // haleyjd 09/27/06: stored singularity value
    int idnum;            // haleyjd 09/30/06: unique id num for sound event
} channel_t;

// the set of channels available
static channel_t channels[MAX_CHANNELS];
// [FG] removed map objects may finish their sounds
static mobj_t sobjs[MAX_CHANNELS];

// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int snd_SfxVolume = 15;

// Maximum volume of music. Useless so far.
int snd_MusicVolume = 15;

// whether songs are mus_paused
static boolean mus_paused;

// music currently being played
static musicinfo_t *mus_playing;

// following is set
//  by the defaults code in M_misc:
// number of channels available
int snd_channels;

// jff 3/17/98 to keep track of last IDMUS specified music num
int idmusnum;

//
// Internals.
//

//
// S_StopChannel
//
// Stops a sound channel.
//
static void S_StopChannel(int cnum)
{
#ifdef RANGECHECK
    if (cnum < 0 || cnum >= snd_channels)
    {
        I_Error("S_StopChannel: handle %d out of range\n", cnum);
    }
#endif

    if (channels[cnum].sfxinfo)
    {
        I_StopSound(channels[cnum].handle); // stop the sound playing

        // haleyjd 09/27/06: clear the entire channel
        memset(&channels[cnum], 0, sizeof(channel_t));
    }
}

void S_StopChannels(void)
{
    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        I_StopSound(channels[i].handle);
    }

    memset(channels, 0, sizeof(channels));
    memset(sobjs, 0, sizeof(sobjs));
}

//
// S_AdjustSoundParams
//
// Alters a playing sound's volume and stereo separation to account for
// the position and angle of the listener relative to the source.
//
// haleyjd: added channel volume scale value
// haleyjd: added priority scaling
//
static int S_AdjustSoundParams(const mobj_t *listener, const mobj_t *source,
                               int chanvol, int *vol, int *sep, int *pri)
{
    return I_AdjustSoundParams(listener, source, chanvol, vol, sep, pri);
}

//
// S_getChannel :
//
//   If none available, return -1.  Otherwise channel #.
//   haleyjd 09/27/06: fixed priority/singularity bugs
//   Note that a higher priority number means lower priority!
//
static int S_getChannel(const mobj_t *origin, sfxinfo_t *sfxinfo, int priority,
                        int singularity)
{
    // channel number to use
    int cnum;
    int lowestpriority = -1; // haleyjd
    int lpcnum = -1;

    // haleyjd 09/28/06: moved this here. If we kill a sound already
    // being played, we can use that channel. There is no need to
    // search for a free one again because we already know of one.

    // kill old sound
    // killough 12/98: replace is_pickup hack with singularity flag
    // haleyjd 06/12/08: only if subchannel matches
    for (cnum = 0; cnum < snd_channels; cnum++)
    {
        if (channels[cnum].sfxinfo && channels[cnum].singularity == singularity
            && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
    }

    // Find an open channel
    if (cnum == snd_channels)
    {
        // haleyjd 09/28/06: it isn't necessary to look for playing sounds in
        // the same singularity class again, as we just did that above. Here
        // we are looking for an open channel. We will also keep track of the
        // channel found with the lowest sound priority while doing this.
        for (cnum = 0; cnum < snd_channels && channels[cnum].sfxinfo; cnum++)
        {
            if (channels[cnum].priority > lowestpriority)
            {
                lowestpriority = channels[cnum].priority;
                lpcnum = cnum;
            }
        }
    }

    // None available?
    if (cnum == snd_channels)
    {
        // Look for lower priority
        // haleyjd: we have stored the channel found with the lowest priority
        // in the loop above
        if (priority > lowestpriority)
        {
            return -1; // No lower priority.  Sorry, Charlie.
        }
        else
        {
            S_StopChannel(lpcnum); // Otherwise, kick out lowest priority.
            cnum = lpcnum;
        }
    }

#ifdef RANGECHECK
    if (cnum >= snd_channels)
    {
        I_Error("S_getChannel: handle %d out of range\n", cnum);
    }
#endif

    return cnum;
}

static int optionals[NUG_SFX_END - NUG_SFX_START]; // [Nugget]

static void StartSound(const mobj_t *origin, int sfx_id,
                       pitchrange_t pitch_range, rumble_type_t rumble_type)
{
    int sep, pitch, o_priority, priority, singularity, cnum, handle;
    int volumeScale = 127;
    int volume = snd_SfxVolume;
    sfxinfo_t *sfx;

    // jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
    {
        return;
    }

    // [FG] ignore request to play no sound
    if (sfx_id == sfx_None)
    {
        return;
    }

#ifdef RANGECHECK
    // check for bogus sound #
    if (sfx_id < 1 || sfx_id > num_sfx)
    {
        I_Error("Bad sfx #: %d", sfx_id);
    }
#endif

    sfx = &S_sfx[sfx_id];

    // Initialize sound parameters
    pitch = NORM_PITCH;

    // haleyjd: modified so that priority value is always used
    // haleyjd: also modified to get and store proper singularity value
    o_priority = priority = sfx->priority;
    singularity = sfx->singularity;

    // Check to see if it is audible, modify the params
    // killough 3/7/98, 4/25/98: code rearranged slightly

    // [Nugget] Freecam
    const mobj_t *playermo = (R_FreecamOn() && !nodrawers)
                             ? viewplayer->mo
                             : players[displayplayer].mo;

    if (!S_AdjustSoundParams(playermo, origin, volumeScale,
                             &volume, &sep, &priority))
    {
        return;
    }

    if (pitched_sounds)
    {
        // hacks to vary the sfx pitches
        if (pitch_range == PITCH_HALF)
        {
            pitch += 8 - (M_Random() & 15);
        }
        else if (pitch_range == PITCH_FULL)
        {
            pitch += 16 - (M_Random() & 31);
        }

        if (pitch < 0)
        {
            pitch = 0;
        }

        if (pitch > 255)
        {
            pitch = 255;
        }
    }

    // try to find a channel
    if ((cnum = S_getChannel(origin, sfx, priority, singularity)) < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if (cnum < 0 || cnum >= snd_channels)
    {
        I_Error("S_StartSfxInfo: handle %d out of range\n", cnum);
    }
#endif

    channels[cnum].sfxinfo = sfx;
    channels[cnum].origin = origin;

    // [Nugget] Check if there's a lump for the sound,
    // and skip the loop if there is, therefore using the lump instead
    while (sfx->link && !(I_GetSfxLumpNum(sfx) >= 0))
    {
        sfx = sfx->link; // sf: skip thru link(s)
    }

    // Assigns the handle to one of the channels in the mix/output buffer.
    handle = I_StartSound(sfx, volume, sep, pitch);

    // haleyjd: check to see if the sound was started
    if (handle >= 0)
    {
        channels[cnum].handle = handle;

        // haleyjd 05/29/06: record volume scale value
        // haleyjd 09/27/06: store priority and singularity values (!!!)
        channels[cnum].volume = volumeScale;
        channels[cnum].o_priority = o_priority; // original priority
        channels[cnum].priority = priority;     // scaled priority
        channels[cnum].singularity = singularity;
        channels[cnum].idnum = I_SoundID(handle); // unique instance id

        if (rumble_type != RUMBLE_NONE)
        {
            I_StartRumble(players[displayplayer].mo, origin, sfx, handle,
                          rumble_type);
        }
    }
    else // haleyjd: the sound didn't start, so clear the channel info
    {
        memset(&channels[cnum], 0, sizeof(channel_t));
    }
}

void S_StartSoundPitch(const mobj_t *origin, int sfx_id,
                       pitchrange_t pitch_range)
{
    StartSound(origin, sfx_id, pitch_range, RUMBLE_NONE);
}

static boolean IsRumblePlayer(const mobj_t *mo)
{
    return (I_RumbleEnabled() && mo && mo == players[displayplayer].mo);
}

static rumble_type_t RumbleType(const mobj_t *mo, rumble_type_t rumble_type)
{
    return (IsRumblePlayer(mo) ? rumble_type : RUMBLE_NONE);
}

void S_StartSoundPitchEx(const mobj_t *origin, int sfx_id,
                         pitchrange_t pitch_range)
{
    StartSound(origin, sfx_id, pitch_range, RumbleType(origin, RUMBLE_PLAYER));
}

void S_StartSoundPistol(const mobj_t *origin, int sfx_id)
{
    StartSound(origin, sfx_id, PITCH_FULL, RumbleType(origin, RUMBLE_PISTOL));
}

void S_StartSoundShotgun(const mobj_t *origin, int sfx_id)
{
    StartSound(origin, sfx_id, PITCH_FULL, RumbleType(origin, RUMBLE_SHOTGUN));
}

void S_StartSoundSSG(const mobj_t *origin, int sfx_id)
{
    StartSound(origin, sfx_id, PITCH_FULL, RumbleType(origin, RUMBLE_SSG));
}

void S_StartSoundCGun(const mobj_t *origin, int sfx_id)
{
    StartSound(origin, sfx_id, PITCH_FULL, RumbleType(origin, RUMBLE_CGUN));
}

void S_StartSoundBFG(const mobj_t *origin, int sfx_id)
{
    StartSound(origin, sfx_id, PITCH_FULL, RumbleType(origin, RUMBLE_BFG));
}

static rumble_type_t RumbleTypePreset(const mobj_t *origin, int sfx_id)
{
    if (IsRumblePlayer(origin))
    {
        switch (sfx_id)
        {
            case sfx_keyup: // [Nugget]: [NS] Optional key pickup sound.
            case sfx_itemup:
                return RUMBLE_ITEMUP;
            case sfx_wpnup:
                return RUMBLE_WPNUP;
            case sfx_getpow:
                return RUMBLE_GETPOW;
            case sfx_oof:
                return RUMBLE_OOF;
        }
    }
    return RUMBLE_NONE;
}

void S_StartSoundPreset(const mobj_t *origin, int sfx_id,
                        pitchrange_t pitch_range)
{
    StartSound(origin, sfx_id, pitch_range, RumbleTypePreset(origin, sfx_id));
}

void S_StartSoundPain(const mobj_t *origin, int sfx_id)
{
    // [Nugget] As of writing this, this function is only used by the player,
    // so we can place this code right here
    if (STRICTMODE(sfx_id == sfx_plpain))
    {
        int i = BETWEEN(0, 3, (origin->health - 1) / 25);

        while (optionals[sfx_ppai25 + i - NUG_SFX_START] == -1)
        {
          if (3 < ++i) { break; }
        }

        if (i <= 3) { sfx_id = sfx_ppai25 + i; }
    }

    StartSound(origin, sfx_id, PITCH_FULL, RumbleType(origin, RUMBLE_PAIN));
}

void S_StartSoundHitFloor(const mobj_t *origin, int sfx_id)
{
    StartSound(origin, sfx_id, PITCH_FULL, RumbleType(origin, RUMBLE_HITFLOOR));
}

void S_StartSoundSource(const mobj_t *source, const mobj_t *origin, int sfx_id)
{
    StartSound(origin, sfx_id, PITCH_FULL, RumbleType(source, RUMBLE_PLAYER));
}

static rumble_type_t RumbleTypeMissile(const mobj_t *source,
                                       const mobj_t *origin)
{
    if (IsRumblePlayer(source))
    {
        if (origin)
        {
            switch (origin->type)
            {
                case MT_ROCKET:
                    return RUMBLE_ROCKET;
                case MT_PLASMA:
                case MT_PLASMA1:
                case MT_PLASMA2:
                    return RUMBLE_PLASMA;

                default:
                    break;
            }
        }
        return RUMBLE_PLAYER;
    }
    return RUMBLE_NONE;
}

void S_StartSoundMissile(const mobj_t *source, const mobj_t *origin, int sfx_id)
{
    StartSound(origin, sfx_id, PITCH_FULL, RumbleTypeMissile(source, origin));
}

void S_StartSoundOrigin(const mobj_t *source, const mobj_t *origin, int sfx_id)
{
    StartSound(origin, sfx_id, PITCH_FULL, RumbleType(source, RUMBLE_ORIGIN));
}

// [Nugget] /-----------------------------------------------------------------

// [NS] Try to play an optional sound.
static int OptionalOrFallback(const int opt_sound_id, const int sound_id)
{
    // If `opt_sound_id` corresponds to a non-optional sound, use it without checking,
    // otherwise use the optional sound if present
    if (   !(NUG_SFX_START <= opt_sound_id && opt_sound_id < NUG_SFX_END)
        ||  (optionals[opt_sound_id - NUG_SFX_START] >= 0))
    {
        return opt_sound_id;
    }
    else if (sound_id >= 0) // Use the fallback
    {
        return sound_id;
    }

    return -1;
}

void S_StartSoundPitchOptional(const mobj_t *const origin,
                               const int opt_sound_id, const int sound_id,
                               const pitchrange_t pitch_range)
{
    const int final_id = OptionalOrFallback(opt_sound_id, sound_id);

    if (final_id >= 0)
    { StartSound(origin, final_id, pitch_range, RUMBLE_NONE); }
}

void S_StartSoundPresetOptional(const mobj_t *const origin,
                                const int opt_sound_id, const int sound_id,
                                const pitchrange_t pitch_range)
{
    const int final_id = OptionalOrFallback(opt_sound_id, sound_id);

    if (final_id >= 0)
    { StartSound(origin, final_id, pitch_range, RumbleTypePreset(origin, final_id)); }
}

void S_StartSoundHitFloorOptional(const mobj_t *const origin,
                                  const int opt_sound_id, const int sound_id)
{
    const int final_id = OptionalOrFallback(opt_sound_id, sound_id);

    if (final_id >= 0)
    { StartSound(origin, final_id, PITCH_FULL, RumbleType(origin, RUMBLE_HITFLOOR)); }
}

// [Nugget] -----------------------------------------------------------------/

//
// S_StopSound
//
void S_StopSound(const mobj_t *origin)
{
    int cnum;

    // jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
    {
        return;
    }

    for (cnum = 0; cnum < snd_channels; ++cnum)
    {
        if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
    }
}

// [FG] play sounds in full length
boolean full_sounds;

// [FG] removed map objects may finish their sounds
void S_UnlinkSound(mobj_t *origin)
{
    int cnum;

    if (nosfxparm)
    {
        return;
    }

    if (origin)
    {
        for (cnum = 0; cnum < snd_channels; cnum++)
        {
            if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
            {
                mobj_t *const sobj = &sobjs[cnum];
                sobj->x = origin->x;
                sobj->y = origin->y;
                sobj->z = origin->z;
                sobj->info = origin->info;
                channels[cnum].origin = sobj;
                break;
            }
        }
    }
}

//
// Stop and resume music, during game PAUSE.
//

void S_PauseSound(void)
{
    if (mus_playing && !mus_paused)
    {
        I_PauseSong(mus_playing->handle);
        mus_paused = true;
    }
}

void S_ResumeSound(void)
{
    if (mus_playing && mus_paused)
    {
        I_ResumeSong(mus_playing->handle);
        mus_paused = false;
    }
}

//
// Updates music & sounds
//

void S_InitListener(const mobj_t *listener)
{
    if (nosfxparm)
    {
        return;
    }

    I_DeferSoundUpdates();
    I_UpdateListenerParams(listener);
    I_ProcessSoundUpdates();
}

void S_UpdateSounds(const mobj_t *listener)
{
    int cnum;

    // jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
    {
        return;
    }

    // [Nugget] Freecam
    if (R_FreecamOn() && !nodrawers) { listener = viewplayer->mo; }

    I_DeferSoundUpdates();

    for (cnum = 0; cnum < snd_channels; ++cnum)
    {
        channel_t *c = &channels[cnum];
        sfxinfo_t *sfx = c->sfxinfo;

        // haleyjd: has this software channel lost its hardware channel?
        if (c->idnum != I_SoundID(c->handle))
        {
            // clear the channel and keep going
            memset(c, 0, sizeof(channel_t));
            continue;
        }

        if (sfx)
        {
            if (I_SoundIsPlaying(c->handle))
            {
                // initialize parameters
                int volume = snd_SfxVolume;
                int sep = NORM_SEP;
                int pri = c->o_priority; // haleyjd 09/27/06: priority

                // check non-local sounds for distance clipping
                // or modify their params

                if (c->origin && listener != c->origin) // killough 3/20/98
                {
                    if (!S_AdjustSoundParams(listener, c->origin, c->volume,
                                             &volume, &sep, &pri))
                    {
                        S_StopChannel(cnum);
                    }
                    else
                    {
                        I_UpdateSoundParams(c->handle, volume, sep);
                        c->priority = pri; // haleyjd
                    }
                }

                I_UpdateRumbleParams(listener, c->origin, c->handle);
            }
            else // if channel is allocated but sound has stopped, free it
            {
                S_StopChannel(cnum);
            }
        }
    }

    I_UpdateListenerParams(listener);
    I_ProcessSoundUpdates();
    I_UpdateRumble();
}

void S_SetMusicVolume(int volume)
{
    // jff 1/22/98 return if music is not enabled
    if (nomusicparm)
    {
        return;
    }

#ifdef RANGECHECK
    if (volume < 0 || volume > 16)
    {
        I_Error("Attempt to set music volume at %d\n", volume);
    }
#endif

    I_SetMusicVolume(volume);
    snd_MusicVolume = volume;
}

void S_SetSfxVolume(int volume)
{
    // jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
    {
        return;
    }

#ifdef RANGECHECK
    if (volume < 0 || volume > 127)
    {
        I_Error("Attempt to set sfx volume at %d", volume);
    }
#endif

    snd_SfxVolume = volume;
}

static extra_music_t extra_music;

static int current_musicnum = -1;

void S_ChangeMusic(int musicnum, int looping)
{
    musicinfo_t *music;

    musinfo.current_item = -1;
    S_music[mus_musinfo].lumpnum = -1;

    if (musicnum <= mus_None || musicnum >= NUMMUSIC)
    {
        I_Error("Bad music number %d", musicnum);
    }

    current_musicnum = musicnum;

    // jff 1/22/98 return if music is not enabled
    if (nomusicparm)
    {
        return;
    }

    music = &S_music[musicnum];

    if (mus_playing == music)
    {
        return;
    }

    // shutdown old music
    S_StopMusic();

    // get lumpnum if neccessary
    if (!music->lumpnum)
    {
        char namebuf[9];
        M_snprintf(namebuf, sizeof(namebuf), "d_%s", music->name);
        music->lumpnum = W_GetNumForName(namebuf);
    }

    int old_lumpnum = music->lumpnum;

    // load & register it
    music->data = W_CacheLumpNum(music->lumpnum, PU_STATIC);
    if (extra_music && trakinfo_found)
    {
        const char *extra =
            S_GetExtra(music->data, W_LumpLength(music->lumpnum), extra_music);
        if (extra)
        {
            music->lumpnum = W_GetNumForName(extra);
            Z_Free(music->data);
            music->data = W_CacheLumpNum(music->lumpnum, PU_STATIC);
        }
    }
    // julian: added lump length
    music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

    // play it
    I_PlaySong((void *)music->handle, looping);

    // [crispy] log played music
    I_Printf(VB_DEBUG, "S_ChangeMusic: %.8s (%s), %s",
             lumpinfo[music->lumpnum].name,
             W_WadNameForLump(music->lumpnum),
             I_MusicFormat());

    music->lumpnum = old_lumpnum;

    mus_playing = music;

    // [crispy] musinfo.items[0] is reserved for the map's default music
    if (!musinfo.items[0])
    {
        musinfo.items[0] = music->lumpnum;
        S_music[mus_musinfo].lumpnum = -1;
    }
}

// [crispy] adapted from prboom-plus/src/s_sound.c:552-590

void S_ChangeMusInfoMusic(int lumpnum, int looping)
{
    musicinfo_t *music;

    if (PLAYBACK_SKIP)
    {
        musinfo.current_item = lumpnum;
        return;
    }

    if (nomusicparm)
    {
        return;
    }

    if (mus_playing && mus_playing->lumpnum == lumpnum)
    {
        return;
    }

    music = &S_music[mus_musinfo];

    S_StopMusic();

    music->lumpnum = lumpnum;

    music->data = W_CacheLumpNum(music->lumpnum, PU_STATIC);
    if (extra_music && trakinfo_found)
    {
        const char *extra =
            S_GetExtra(music->data, W_LumpLength(music->lumpnum), extra_music);
        if (extra)
        {
            music->lumpnum = W_GetNumForName(extra);
            Z_Free(music->data);
            music->data = W_CacheLumpNum(music->lumpnum, PU_STATIC);
        }
    }
    music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

    I_PlaySong((void *)music->handle, looping);

    // [crispy] log played music
    I_Printf(VB_DEBUG, "S_ChangeMusInfoMusic: %.8s (%s), %s",
             lumpinfo[music->lumpnum].name,
             W_WadNameForLump(music->lumpnum),
             I_MusicFormat());

    music->lumpnum = lumpnum;

    mus_playing = music;

    musinfo.current_item = lumpnum;
}

void S_RestartMusic(void)
{
    if (musinfo.current_item != -1)
    {
        S_ChangeMusInfoMusic(musinfo.current_item, true);
    }
    else if (current_musicnum != -1)
    {
        S_ChangeMusic(current_musicnum, true);
    }
}

//
// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
    S_ChangeMusic(m_id, false);
}

void S_StopMusic(void)
{
    if (!mus_playing)
    {
        return;
    }

    if (mus_paused)
    {
        I_ResumeSong(mus_playing->handle);
    }

    I_StopSong((void *)mus_playing->handle);
    I_UnRegisterSong((void *)mus_playing->handle);

    // for wads with "empty" music lumps (Nihility.wad)
    if (mus_playing->data!= NULL)
    {
        Z_ChangeTag(mus_playing->data, PU_CACHE);
    }

    mus_playing->data = NULL;
    mus_playing = NULL;
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

static inline int WRAP(int i, int w)
{
    while (i < 0)
    {
        i += w;
    }

    return i % w;
}

void S_Start(void)
{
    int cnum, mnum;

    // kill all playing sounds at start of level
    //  (trust me - a good idea)

    // jff 1/22/98 skip sound init if sound not enabled
    if (!nosfxparm)
    {
        for (cnum = 0; cnum < snd_channels; ++cnum)
        {
            if (channels[cnum].sfxinfo)
            {
                S_StopChannel(cnum);
            }
        }
    }

    // [crispy] don't load map's default music if loaded from a savegame with
    // MUSINFO data
    if (musinfo.from_savegame)
    {
        musinfo.from_savegame = false;
        return;
    }

    // start new music for the level
    mus_paused = 0;

    if (gamemapinfo && gamemapinfo->music[0])
    {
        int muslump = W_CheckNumForName(gamemapinfo->music);
        if (muslump >= 0)
        {
            S_ChangeMusInfoMusic(muslump, true);
            return;
        }
        // If the mapinfo defined music cannot be found, try the default for the
        // given map.
    }

    if (idmusnum != -1)
    {
        mnum = idmusnum; // jff 3/17/98 reload IDMUS music if not -1
    }
    else
    {
        if (gamemode == commercial)
        {
            mnum = mus_runnin + WRAP(gamemap - 1, NUMMUSIC - mus_runnin);
        }
        else
        {
            mnum = mus_e1m1
                   + WRAP((gameepisode - 1) * 9 + gamemap - 1,
                          mus_runnin - mus_e1m1);
        }
    }

    // [crispy] reset musinfo data at the start of a new map
    memset(&musinfo, 0, sizeof(musinfo));

    S_ChangeMusic(mnum, true);
}

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

static void InitE4Music(void)
{
    int i, j;
    static const int spmus[] =  // Song - Who? - Where?
    {
        mus_e3m4,  // American    e4m1
        mus_e3m2,  // Romero      e4m2
        mus_e3m3,  // Shawn       e4m3
        mus_e1m5,  // American    e4m4
        mus_e2m7,  // Tim         e4m5
        mus_e2m4,  // Romero      e4m6
        mus_e2m6,  // J.Anderson  e4m7 CHIRON.WAD
        mus_e2m5,  // Shawn       e4m8
        mus_e1m9   // Tim         e4m9
    };

    for (i = mus_e4m1, j = 0; i <= mus_e4m9; i++, j++)
    {
        musicinfo_t *music = &S_music[i];
        char namebuf[9];

        M_snprintf(namebuf, sizeof(namebuf), "d_%s", music->name);

        if (W_CheckNumForName(namebuf) == -1)
        {
            music->name = S_music[spmus[j]].name;
        }
    }
}

// [crispy] add support for alternative music tracks for Final Doom's
// TNT and Plutonia as introduced in DoomMetalVol5.wad

typedef struct {
    char *const from;
    char *const to;
} altmusic_t;

static const altmusic_t altmusic_tnt[] =
{
    {"runnin", "sadist"}, //  MAP01
    {"stalks", "burn"},   //  MAP02
    {"countd", "messag"}, //  MAP03
    {"betwee", "bells"},  //  MAP04
    {"doom",   "more"},   //  MAP05
    {"the_da", "agony"},  //  MAP06
    {"shawn",  "chaos"},  //  MAP07
    {"ddtblu", "beast"},  //  MAP08
    {"in_cit", "sadist"}, //  MAP09
    {"dead",   "infini"}, //  MAP10
    {"stlks2", "kill"},   //  MAP11
    {"theda2", "ddtbl3"}, //  MAP12
    {"doom2",  "bells"},  //  MAP13
    {"ddtbl2", "cold"},   //  MAP14
    {"runni2", "burn2"},  //  MAP15
    {"dead2",  "blood"},  //  MAP16
    {"stlks3", "more"},   //  MAP17
    {"romero", "infini"}, //  MAP18
    {"shawn2", "countd"}, //  MAP19
    {"messag", "horizo"}, //  MAP20
    {"count2", "in_cit"}, //  MAP21
    {"ddtbl3", "aim"},    //  MAP22
    {"ampie",  "ampie"},  // (MAP23)
    {"theda3", "betwee"}, //  MAP24
    {"adrian", "doom"},   //  MAP25
    {"messg2", "blood"},  //  MAP26
    {"romer2", "beast"},  //  MAP27
    {"tense",  "aim"},    //  MAP28
    {"shawn3", "bells"},  //  MAP29
    {"openin", "beast"},  //  MAP30
    {"evil",   "evil"},   // (MAP31)
    {"ultima", "in_cit"}, //  MAP32
    {NULL,     NULL},
};

// Plutonia music is completely taken from Doom 1 and 2, but re-arranged.
// That is, Plutonia's D_RUNNIN (for MAP01) is the renamed D_E1M2. So,
// it makes sense to play the D_E1M2 replacement from DoomMetal in Plutonia.

static const altmusic_t altmusic_plut[] =
{
    {"runnin", "e1m2"},   //  MAP01
    {"stalks", "e1m3"},   //  MAP02
    {"countd", "e1m6"},   //  MAP03
    {"betwee", "e1m4"},   //  MAP04
    {"doom",   "e1m9"},   //  MAP05
    {"the_da", "e1m8"},   //  MAP06
    {"shawn",  "e2m1"},   //  MAP07
    {"ddtblu", "e2m2"},   //  MAP08
    {"in_cit", "e3m3"},   //  MAP09
    {"dead",   "e1m7"},   //  MAP10
    {"stlks2", "bunny"},  //  MAP11
    {"theda2", "e3m8"},   //  MAP12
    {"doom2",  "e3m2"},   //  MAP13
    {"ddtbl2", "e2m8"},   //  MAP14
    {"runni2", "e2m7"},   //  MAP15
    {"dead2",  "e3m1"},   //  MAP16
    {"stlks3", "e1m1"},   //  MAP17
    {"romero", "e2m5"},   //  MAP18
    {"shawn2", "e1m5"},   //  MAP19
    {"messag", "messag"}, // (MAP20)
    {"count2", "count2"}, // (MAP21, d_read_m has no instumental cover in Doom Metal)
    {"ddtbl3", "ddtbl3"}, // (MAP22)
    {"ampie",  "ampie"},  // (MAP23)
    {"theda3", "theda3"}, // (MAP24)
    {"adrian", "adrian"}, // (MAP25)
    {"messg2", "messg2"}, // (MAP26)
    {"romer2", "e2m1"},   //  MAP27
    {"tense",  "e2m2"},   //  MAP28
    {"shawn3", "e1m1"},   //  MAP29
    {"openin", "openin"}, // (MAP30, d_victor has no instumental cover in Doom Metal)
    {"evil",   "e3m4"},   //  MAP31
    {"ultima", "e2m8"},   //  MAP32
    {NULL,     NULL},
};

static void InitFinalDoomMusic()
{
    const altmusic_t *altmusic;

    if (gamemission == pack_tnt)
    {
        altmusic = altmusic_tnt;
    }
    else if (gamemission == pack_plut)
    {
        altmusic = altmusic_plut;
    }
    else
    {
        return;
    }

    // [crispy] chicken-out if only one lump is missing, something must be wrong
    for (int j = 0; altmusic[j].from; j++)
    {
        char name[9] = "d_";

        strncpy(&name[2], altmusic[j].to, 6);

        if (W_CheckNumForName(name) == -1)
        {
            return;
        }
    }

    for (int i = mus_runnin, j = 0; altmusic[j].from; i++, j++)
    {
        S_music[i].name = altmusic[j].to;
    }
}

void S_Init(int sfxVolume, int musicVolume)
{
    // jff 1/22/98 skip sound init if sound not enabled
    if (!nosfxparm)
    {
        // haleyjd
        I_SetChannels();

        S_SetSfxVolume(sfxVolume);

        // Reset channel memory
        memset(channels, 0, sizeof(channels));
        memset(sobjs, 0, sizeof(sobjs));
    }

    S_SetMusicVolume(musicVolume);

    // no sounds are playing, and they are not mus_paused
    mus_paused = 0;

    if (gamemode != commercial)
    {
        InitE4Music();
    }
    else
    {
        // [crispy] add support for alternative music tracks for Final Doom's
        // TNT and Plutonia as introduced in DoomMetalVol5.wad
        InitFinalDoomMusic();
    }

    // [Nugget] Get lump nums for optional sounds
    for (int i = NUG_SFX_START;  i < NUG_SFX_END;  i++)
    { optionals[i - NUG_SFX_START] = I_GetSfxLumpNum(&S_sfx[i]); }
}

void S_BindSoundVariables(void)
{
    BIND_NUM(extra_music, EXMUS_OFF, EXMUS_OFF, EXMUS_ORIGINAL,
             "Extra soundtrack (0 = Off; 1 = Remix; 2 = Original");
}

//----------------------------------------------------------------------------
//
// $Log: s_sound.c,v $
// Revision 1.11  1998/05/03  22:57:06  killough
// beautification, #include fix
//
// Revision 1.10  1998/04/27  01:47:28  killough
// Fix pickups silencing player weapons
//
// Revision 1.9  1998/03/23  03:39:12  killough
// Fix spy-mode sound effects
//
// Revision 1.8  1998/03/17  20:44:25  jim
// fixed idmus non-restore, space bug
//
// Revision 1.7  1998/03/09  07:32:57  killough
// ATTEMPT to support hearing with displayplayer's hears
//
// Revision 1.6  1998/03/04  07:46:10  killough
// Remove full-volume sound hack from MAP08
//
// Revision 1.5  1998/03/02  11:45:02  killough
// Make missing sounds non-fatal
//
// Revision 1.4  1998/02/02  13:18:48  killough
// stop incorrect looping of music (e.g. bunny scroll)
//
// Revision 1.3  1998/01/26  19:24:52  phares
// First rev with no ^Ms
//
// Revision 1.2  1998/01/23  01:50:49  jim
// Added music/sound options, and enables
//
// Revision 1.1.1.1  1998/01/19  14:03:04  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
