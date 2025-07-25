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
// DESCRIPTION:
//  Refresh of things, i.e. objects represented by sprites.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#include "d_player.h"
#include "doomdef.h"
#include "doomstat.h"
#include "hu_crosshair.h" // [Alaux] Lock crosshair on target
#include "i_printf.h"
#include "i_system.h"
#include "i_video.h"
#include "info.h"
#include "m_swap.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "r_bmaps.h" // [crispy] R_BrightmapForTexName()
#include "r_bsp.h"
#include "r_data.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_segs.h"
#include "r_state.h"
#include "r_things.h"
#include "r_voxel.h"
#include "tables.h"
#include "v_fmt.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

// [Nugget]
#include "m_nughud.h"
#include "p_map.h"
#include "st_stuff.h"
#include "wi_stuff.h"

#define MINZ        (FRACUNIT*4)
#define MAXZ        (FRACUNIT*8192)
#define BASEYCENTER 100

typedef struct {
  int x1;
  int x2;
  int column;
  int topclip;
  int bottomclip;
} maskdraw_t;

//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//

fixed_t pspritescale;
fixed_t pspriteiscale;

lighttable_t **spritelights;        // killough 1/25/98 made static

// [Woof!] optimization for drawing huge amount of drawsegs.
// adapted from prboom-plus/src/r_things.c
typedef struct drawseg_xrange_item_s
{
  short x1, x2;
  drawseg_t *user;
} drawseg_xrange_item_t;

typedef struct drawsegs_xrange_s
{
  drawseg_xrange_item_t *items;
  int count;
} drawsegs_xrange_t;

#define DS_RANGES_COUNT 3
static drawsegs_xrange_t drawsegs_xranges[DS_RANGES_COUNT];

static drawseg_xrange_item_t *drawsegs_xrange;
static unsigned int drawsegs_xrange_size = 0;
static int drawsegs_xrange_count = 0;

// [FG] 32-bit integer math
static int *clipbot = NULL; // killough 2/8/98: // dropoff overflow
static int *cliptop = NULL; // change to MAX_*  // dropoff overflow

// constant arrays
//  used for psprite clipping and initializing clipping

// [FG] 32-bit integer math
int *negonearray = NULL;        // killough 2/8/98:
int *screenheightarray = NULL;  // change to MAX_*

//
// INITIALIZATION FUNCTIONS
//

// variables used to look up and range check thing_t sprites patches

spritedef_t *sprites;

#define MAX_SPRITE_FRAMES 29          /* Macroized -- killough 1/25/98 */

static spriteframe_t sprtemp[MAX_SPRITE_FRAMES];
static int maxframe;

boolean have_crouch_sprites; // [Nugget]

void R_InitSpritesRes(void)
{
  xtoviewangle = Z_Calloc(1, (video.width + 1) * sizeof(*xtoviewangle), PU_RENDERER, NULL);
  linearskyangle = Z_Calloc(1, (video.width + 1) * sizeof(*linearskyangle), PU_RENDERER, NULL);
  negonearray = Z_Calloc(1, video.width * sizeof(*negonearray), PU_RENDERER, NULL);
  screenheightarray = Z_Calloc(1, video.width * sizeof(*screenheightarray), PU_RENDERER, NULL);

  clipbot = Z_Calloc(1, 2 * video.width * sizeof(*clipbot), PU_RENDERER, NULL);
  cliptop = clipbot + video.width;
}

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//

static void R_InstallSpriteLump(int lump, unsigned frame,
                                unsigned rotation, boolean flipped)
{
  if (frame == '^' - 'A')
  {
    frame = '\\' - 'A';
  }

  if (frame >= MAX_SPRITE_FRAMES || rotation > 8)
    I_Error("R_InstallSpriteLump: Bad frame characters in lump %i", lump);

  if ((int) frame > maxframe)
    maxframe = frame;

  if (rotation == 0)
    {    // the lump should be used for all rotations
      int r;
      for (r=0 ; r<8 ; r++)
        if (sprtemp[frame].lump[r]==-1)
          {
            sprtemp[frame].lump[r] = lump - firstspritelump;
            sprtemp[frame].flip[r] = (byte) flipped;
            sprtemp[frame].rotate = false; //jff 4/24/98 if any subbed, rotless
          }
      return;
    }

  // the lump is only used for one rotation

  if (sprtemp[frame].lump[--rotation] == -1)
    {
      sprtemp[frame].lump[rotation] = lump - firstspritelump;
      sprtemp[frame].flip[rotation] = (byte) flipped;
      sprtemp[frame].rotate = true; //jff 4/24/98 only change if rot used
    }
}

//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
// (4 chars exactly) to be used.
//
// Builds the sprite rotation matrixes to account
// for horizontally flipped sprites.
//
// Will report an error if the lumps are inconsistent.
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
//
// A sprite that is flippable will have an additional
//  letter/number appended.
//
// The rotation character can be 0 to signify no rotations.
//
// 1/25/98, 1/31/98 killough : Rewritten for performance
//
// Empirically verified to have excellent hash
// properties across standard Doom sprites:

#define R_SpriteNameHash(s) ((unsigned)((s)[0]-((s)[1]*3-(s)[3]*2-(s)[2])*2))

void R_InitSpriteDefs(char **namelist)
{
  size_t numentries = lastspritelump-firstspritelump+1;
  struct { int index, next; } *hash;
  int i;

  if (!numentries || !*namelist)
    return;

  // [Nugget] Alt. sprites
  sprites = Z_Calloc(num_sprites + NUMALTSPRITES, sizeof(*sprites), PU_STATIC, NULL);

  // Create hash table based on just the first four letters of each sprite
  // killough 1/31/98

  hash = Z_Malloc(sizeof(*hash)*numentries,PU_STATIC,0); // allocate hash table

  for (i=0; i<numentries; i++)             // initialize hash table as empty
    hash[i].index = -1;

  for (i=0; i<numentries; i++)             // Prepend each sprite to hash chain
    {                                      // prepend so that later ones win
      int j = R_SpriteNameHash(lumpinfo[i+firstspritelump].name) % numentries;
      hash[i].next = hash[j].index;
      hash[j].index = i;
    }

  // scan all the lump names for each of the names,
  //  noting the highest frame letter.

  for (i=0 ; i<num_sprites + NUMALTSPRITES ; i++) // [Nugget] Alt. sprites
    {
      const char *spritename = namelist[i];
      int j;

      if (!spritename)
        continue;

      j = hash[R_SpriteNameHash(spritename) % numentries].index;

      if (j >= 0)
        {
          memset(sprtemp, -1, sizeof(sprtemp));
          maxframe = -1;
          do
            {
              register lumpinfo_t *lump = lumpinfo + j + firstspritelump;

              // Fast portable comparison -- killough
              // (using int pointer cast is nonportable):

              if (!((lump->name[0] ^ spritename[0]) |
                    (lump->name[1] ^ spritename[1]) |
                    (lump->name[2] ^ spritename[2]) |
                    (lump->name[3] ^ spritename[3])))
                {
                  R_InstallSpriteLump(j+firstspritelump,
                                      lump->name[4] - 'A',
                                      lump->name[5] - '0',
                                      false);
                  if (lump->name[6])
                    R_InstallSpriteLump(j+firstspritelump,
                                        lump->name[6] - 'A',
                                        lump->name[7] - '0',
                                        true);
                }
            }
          while ((j = hash[j].next) >= 0);

          // check the frames that were found for completeness
          if ((sprites[i].numframes = ++maxframe))  // killough 1/31/98
            {
              int frame;
              for (frame = 0; frame < maxframe; frame++)
                switch ((int) sprtemp[frame].rotate)
                  {
                  case -1:
                    // no rotations were found for that frame at all
                    // [FG] make non-fatal
                    I_Printf (VB_WARNING, "R_InitSprites: No patches found "
                             "for %.8s frame %c", namelist[i], frame+'A');
                    break;

                  case 0:
                    // only the first rotation is needed
                    break;

                  case 1:
                    // must have all 8 frames
                    {
                      int rotation;
                      for (rotation=0 ; rotation<8 ; rotation++)
                        if (sprtemp[frame].lump[rotation] == -1)
                          I_Error ("R_InitSprites: Sprite %.8s frame %c "
                                   "is missing rotations",
                                   namelist[i], frame+'A');
                      break;
                    }
                  }
              // allocate space for the frames present and copy sprtemp to it
              sprites[i].spriteframes =
                Z_Malloc (maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
              memcpy (sprites[i].spriteframes, sprtemp,
                      maxframe*sizeof(spriteframe_t));
            }
        }
    }
  Z_Free(hash);             // free hash table

  // [Nugget]
  have_crouch_sprites = sprites[num_sprites + ASPR_PLYC].numframes > 0;
}

//
// GAME FUNCTIONS
//

static vissprite_t *vissprites, **vissprite_ptrs;  // killough
static size_t num_vissprite, num_vissprite_alloc, num_vissprite_ptrs;

#define M_ARRAY_INIT_CAPACITY 128
#include "m_array.h"

static mobj_t **nearby_sprites = NULL;

//
// R_InitSprites
// Called at program start.
//

void R_InitSprites(char **namelist)
{
  int i;
  for (i = 0; i < video.width; i++)    // killough 2/8/98
    negonearray[i] = -1;
  R_InitSpriteDefs(namelist);
}

//
// R_ClearSprites
// Called at frame start.
//

void R_ClearSprites (void)
{
  rendered_vissprites = num_vissprite;
  num_vissprite = 0;            // killough
}

//
// R_NewVisSprite
//

vissprite_t *R_NewVisSprite(void)
{
  if (num_vissprite >= num_vissprite_alloc)             // killough
    {
      num_vissprite_alloc = num_vissprite_alloc ? num_vissprite_alloc*2 : 128;
      vissprites = Z_Realloc(vissprites,num_vissprite_alloc*sizeof(*vissprites),PU_STATIC,0);
    }
 return vissprites + num_vissprite++;
}

// [Nugget] Actual sprite height (without padding) /--------------------------

static actualspriteheight_t *actual_sprite_heights = NULL;

static const actualspriteheight_t *CalculateActualSpriteHeight(const int lump)
{
  patch_t *const patch = V_CachePatchNum(lump, PU_CACHE);
  const short width = SHORT(patch->width);

  short actualheight = 0, toppadding = SHRT_MAX;

  for (int i = 0;  i < width;  i++)
  {
    const post_t *post = (post_t *) ((byte *) patch + LONG(patch->columnofs[i]));

    if (post->topdelta == 0xFF) { continue; }

    short columnpadding = post->topdelta,
          columnheight,
          topdelta = 0;

    do {
      // [FG] support for tall sprites in DeePsea format
      if (post->topdelta <= topdelta)
      {
        topdelta += post->topdelta;
      }
      else { topdelta = post->topdelta; }

      columnheight = topdelta + SHORT(post->length);

      post = (post_t *) ((byte *) post + 4 + post->length); // 4 = sizeof(topdelta) + sizeof(length)
    } while (post->topdelta != 0xFF);

    toppadding = MIN(toppadding, columnpadding);
    actualheight = MAX(actualheight, columnheight);
  }

  actualheight -= toppadding;

  array_push(
    actual_sprite_heights,
    ((actualspriteheight_t) { lump, actualheight, toppadding })
  );

  return &actual_sprite_heights[array_size(actual_sprite_heights) - 1];
}

const actualspriteheight_t *R_GetActualSpriteHeight(int sprite, int frame)
{
  const int lump = firstspritelump + sprites[sprite].spriteframes[frame].lump[0];

  for (int i = 0;  i < array_size(actual_sprite_heights);  i++)
  {
    if (actual_sprite_heights[i].lump == lump)
    { return &actual_sprite_heights[i]; }
  }

  return CalculateActualSpriteHeight(lump);
}

// [Nugget] -----------------------------------------------------------------/

//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//

int   *mfloorclip; // [FG] 32-bit integer math
int   *mceilingclip; // [FG] 32-bit integer math
fixed_t spryscale;
int64_t sprtopscreen; // [FG] 64-bit integer math

void R_DrawMaskedColumn(column_t *column)
{
  int64_t topscreen, bottomscreen; // [FG] 64-bit integer math
  fixed_t basetexturemid = dc_texturemid;
  int top = -1;

  dc_texheight = 0; // killough 11/98

  while (column->topdelta != 0xff)
    {
      // [FG] support for tall sprites in DeePsea format
      if (column->topdelta <= top)
      {
            top += column->topdelta;
      }
      else
      {
            top = column->topdelta;
      }
      // calculate unclipped screen coordinates for post
      topscreen = sprtopscreen + spryscale*top;
      bottomscreen = topscreen + spryscale*column->length;

      // Here's where "sparkles" come in -- killough:
      dc_yl = (int)((topscreen+FRACMASK)>>FRACBITS); // [FG] 64-bit integer math
      dc_yh = (int)((bottomscreen-1)>>FRACBITS); // [FG] 64-bit integer math

      if (dc_yh >= mfloorclip[dc_x])
        dc_yh = mfloorclip[dc_x]-1;

      if (dc_yl <= mceilingclip[dc_x])
        dc_yl = mceilingclip[dc_x]+1;

      // killough 3/2/98, 3/27/98: Failsafe against overflow/crash:
      if (dc_yl <= dc_yh && dc_yh < viewheight)
        {
          dc_source = (byte *) column + 3;
          dc_texturemid = basetexturemid - (top<<FRACBITS);

          // Drawn by either R_DrawColumn
          //  or (SHADOW) R_DrawFuzzColumn.
          colfunc();
        }
      column = (column_t *)((byte *) column + column->length + 4);
    }
  dc_texturemid = basetexturemid;
}

//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//

void R_DrawVisSprite(vissprite_t *vis, int x1, int x2)
{
  column_t *column;
  int      texturecolumn;
  fixed_t  frac;
  patch_t  *patch = V_CachePatchNum (vis->patch+firstspritelump, PU_CACHE);

  dc_colormap[0] = vis->colormap[0];
  dc_colormap[1] = vis->colormap[1];
  dc_brightmap = vis->brightmap;

  // killough 4/11/98: rearrange and handle translucent sprites
  // mixed with translucent/non-translucent 2s normals

  // [Nugget] Sprite shadows
  if (vis->yscale > 0)
  {
    colfunc = R_DrawColumnShadow;
  }
  else

  if (!dc_colormap[0])   // NULL colormap = shadow draw
    colfunc = R_DrawFuzzColumn;    // killough 3/14/98
  else
    // [FG] colored blood and gibs
    if (vis->mobjflags2 & MF2_COLOREDBLOOD)
      {
        colfunc = R_DrawTranslatedColumn;
        dc_translation = red2col[vis->color];
      }
  else
    if (vis->mobjflags & MF_TRANSLATION)
      {
        colfunc = R_DrawTranslatedColumn;
        dc_translation = translationtables - 256 +
          ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT-8) );
      }
    else
      if (translucency && !(strictmode && demo_compatibility)
          && vis->mobjflags & MF_TRANSLUCENT) // phares
        {
          colfunc = R_DrawTLColumn;
          tranmap = main_tranmap;       // killough 4/11/98

          if (vis->tranmap) { tranmap = vis->tranmap; } // [Nugget]
        }
      else
        colfunc = R_DrawColumn;         // killough 3/14/98, 4/11/98

  dc_iscale = abs(vis->xiscale);
  dc_texturemid = vis->texturemid;
  frac = vis->startfrac;
  spryscale = vis->scale;
  sprtopscreen = centeryfrac - FixedMul(dc_texturemid,spryscale);

  // [Nugget] Sprite shadows
  if (vis->yscale > 0) { spryscale = vis->yscale; }

  // [Nugget] Thing lighting /------------------------------------------------

  boolean percolumn_lighting;

  fixed_t pcl_offset = 0;
  fixed_t pcl_cosine = 0, pcl_sine = 0;
  int pcl_lightindex = 0;

  if (STRICTMODE(thing_lighting_mode) == THINGLIGHTING_PERCOLUMN
      && !vis->fullbright && dc_colormap[0] && !fixedcolormap)
  {
    percolumn_lighting = true;

    pcl_offset = (SHORT(patch->leftoffset) << FRACBITS) - vis->xiscale/2;

    const int angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;

    pcl_cosine = finecosine[angle];
    pcl_sine   =   finesine[angle];

    pcl_lightindex = STRICTMODE(!diminishing_lighting) ? 0 : R_GetLightIndex(spryscale);
  }
  else { percolumn_lighting = false; }

  // [Nugget] ---------------------------------------------------------------/

  for (dc_x=vis->x1 ; dc_x<=vis->x2 ; dc_x++, frac += vis->xiscale)
    {
      texturecolumn = frac>>FRACBITS;

      if (texturecolumn < 0)
        continue;
      else if (texturecolumn >= SHORT(patch->width))
        break;

      // [Nugget] Thing lighting
      if (percolumn_lighting)
      {
        fixed_t offset = frac - pcl_offset;

        if (vis->flipped) { offset = -offset; }

        const fixed_t gx = vis->gx + FixedMul(offset, pcl_cosine),
                      gy = vis->gy + FixedMul(offset, pcl_sine);

        int lightnum = (R_GetLightLevelInPoint(gx, gy) >> LIGHTSEGSHIFT)
                     + extralight;

        dc_colormap[0] = scalelight[BETWEEN(0, LIGHTLEVELS-1, lightnum)][pcl_lightindex];
      }

      column = (column_t *)((byte *) patch +
                            LONG(patch->columnofs[texturecolumn]));
      R_DrawMaskedColumn (column);
    }
  colfunc = R_DrawColumn;         // killough 3/14/98
}

//
// R_ProjectSprite
// Generates a vissprite for a thing if it might be visible.
//

boolean flipcorpses = false;

static void R_ProjectSprite (mobj_t* thing)
{
  fixed_t   gzt;               // killough 3/27/98
  fixed_t   tx, txc;
  fixed_t   xscale;
  int       x1;
  int       x2;
  spritedef_t   *sprdef;
  spriteframe_t *sprframe;
  int       lump;
  boolean   flip;
  vissprite_t *vis;
  fixed_t   iscale;
  int heightsec;      // killough 3/27/98

  // [FG] moved declarations here
  fixed_t tr_x, tr_y, gxt, gyt, tz;
  fixed_t interpx, interpy, interpz, interpangle;

  // [Nugget]
  if (thing->intflags & MIF_DONTRENDER) { return; }

  // [Nugget] Freecam
  if (thing == R_GetFreecamMobj() && !R_ChasecamOn()) { return; }

  // andrewj: voxel support
  if (VX_ProjectVoxel (thing))
      return;

  // [AM] Interpolate between current and last position,
  //      if prudent.
  if (uncapped &&
      // Don't interpolate if the mobj did something
      // that would necessitate turning it off for a tic.
      thing->interp == true &&
      // Don't interpolate during a paused state.
      leveltime > oldleveltime)
  {
      interpx = LerpFixed(thing->oldx, thing->x);
      interpy = LerpFixed(thing->oldy, thing->y);
      interpz = LerpFixed(thing->oldz, thing->z);
      interpangle = LerpAngle(thing->oldangle, thing->angle);
  }
  else
  {
      interpx = thing->x;
      interpy = thing->y;
      interpz = thing->z;
      interpangle = thing->angle;
  }

  // transform the origin point
  tr_x = interpx - viewx;
  tr_y = interpy - viewy;

  gxt = FixedMul(tr_x,viewcos);
  gyt = -FixedMul(tr_y,viewsin);

  tz = gxt-gyt;

  // thing is behind view plane?
  if (tz < MINZ || tz > MAXZ)
    return;

  // [Nugget]
  if (thing->intflags & MIF_FLAKE && tz > 1024*FRACUNIT) { return; }

  gxt = -FixedMul(tr_x,viewsin);
  gyt = FixedMul(tr_y,viewcos);
  tx = -(gyt+gxt);

  // too far off the side?
  if (abs(tx) / max_project_slope > tz)
    return;

  xscale = FixedDiv(projection, tz);

    // decide which patch to use for sprite relative to player
  if ((unsigned) thing->sprite >= num_sprites)
    I_Error ("R_ProjectSprite: invalid sprite number %i", thing->sprite);

  sprdef = &sprites[thing->sprite];

  if ((thing->frame&FF_FRAMEMASK) >= sprdef->numframes)
    I_Error ("R_ProjectSprite: invalid frame %i for sprite %s",
             thing->frame & FF_FRAMEMASK, sprnames[thing->sprite]);

  // [Nugget] Alt. sprites /--------------------------------------------------

  int sprite = thing->sprite,
       frame = (thing->altframe > -1) ? thing->altframe : thing->frame;

  if (!strictmode && thing->altsprite > -1
      && sprites[num_sprites + thing->altsprite].numframes > (frame & FF_FRAMEMASK))
  {
    sprite = num_sprites + thing->altsprite;
    sprdef = &sprites[sprite];
  }
  else {
    sprite = thing->sprite;
     frame = thing->frame;
  }

  // [Nugget] ---------------------------------------------------------------/

  sprframe = &sprdef->spriteframes[frame & FF_FRAMEMASK];

  if (sprframe->rotate)
    {
      // choose a different rotation based on player view
      angle_t ang = R_PointToAngle(interpx, interpy);
      unsigned rot = (ang-interpangle+(unsigned)(ANG45/2)*9)>>29;

      if (STRICTMODE(flip_levels)) { rot = (8 - rot) & 7; } // [Nugget] Flip levels

      lump = sprframe->lump[rot];
      flip = (boolean) sprframe->flip[rot];
    }
  else
    {
      // use single rotation for all views
      lump = sprframe->lump[0];
      flip = (boolean) sprframe->flip[0];
    }

  // [crispy] randomly flip corpse, blood and death animation sprites
  if (STRICTMODE(flipcorpses) &&
      (thing->flags2 & MF2_FLIPPABLE) &&
      !(thing->flags & MF_SHOOTABLE) &&
      (thing->intflags & MIF_FLIP))
    {
      flip = !flip;
    }

  if (STRICTMODE(flip_levels)) { flip = !flip; } // [Nugget] Flip levels

  txc = tx; // [FG] sprite center coordinate

  // calculate edges of the shape
  // [crispy] fix sprite offsets for mirrored sprites
  tx -= flip ? spritewidth[lump] - spriteoffset[lump] : spriteoffset[lump];
  x1 = (centerxfrac + FixedMul64(tx,xscale)) >>FRACBITS;

    // off the right side?
  if (x1 > viewwidth)
    return;

  tx +=  spritewidth[lump];
  x2 = ((centerxfrac + FixedMul64(tx,xscale)) >> FRACBITS) - 1;

    // off the left side
  if (x2 < 0)
    return;

  gzt = interpz + spritetopoffset[lump];

  // killough 4/9/98: clip things which are out of view due to height
  if (interpz > (int64_t)viewz + FixedDiv(viewheightfrac, xscale) ||
      gzt < (int64_t)viewz - FixedDiv(viewheightfrac - viewheight, xscale))
    return;

  // killough 3/27/98: exclude things totally separated
  // from the viewer, by either water or fake ceilings
  // killough 4/11/98: improve sprite clipping for underwater/fake ceilings

  heightsec = thing->subsector->sector->heightsec;

  if (heightsec != -1)   // only clip things which are in special sectors
    {
      int phs = viewplayer->mo->subsector->sector->heightsec;
      if (phs != -1 && viewz < sectors[phs].floorheight ?
          interpz >= sectors[heightsec].floorheight :
          gzt < sectors[heightsec].floorheight)
        return;
      if (phs != -1 && viewz > sectors[phs].ceilingheight ?
          gzt < sectors[heightsec].ceilingheight &&
          viewz >= sectors[heightsec].ceilingheight :
          interpz >= sectors[heightsec].ceilingheight)
        return;
    }

  // store information in a vissprite
  vis = R_NewVisSprite ();

  // killough 3/27/98: save sector for special clipping later
  vis->heightsec = heightsec;

  vis->voxel_index = -1;

  vis->mobjflags = thing->flags;
  vis->mobjflags2 = thing->flags2;
  vis->scale = xscale;
  vis->gx = interpx;
  vis->gy = interpy;
  vis->gz = interpz;
  vis->gzt = gzt;                          // killough 3/27/98
  vis->texturemid = gzt - viewz;
  vis->x1 = x1 < 0 ? 0 : x1;
  vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;
  iscale = FixedDiv(FRACUNIT, xscale);
  vis->color = thing->bloodcolor;

  // [Nugget]
  vis->xscale = vis->yscale = 0;
  vis->fullbright = false;
  vis->flipped = flip;

  if (flip)
    {
      vis->startfrac = spritewidth[lump]-1;
      vis->xiscale = -iscale;
    }
  else
    {
      vis->startfrac = 0;
      vis->xiscale = iscale;
    }

  if (vis->x1 > x1)
    vis->startfrac += vis->xiscale*(vis->x1-x1);
  vis->patch = lump;

  // get light level
  if (thing->flags & MF_SHADOW)
    vis->colormap[0] = vis->colormap[1] = NULL;               // shadow draw
  else if (fixedcolormap)
    vis->colormap[0] = vis->colormap[1] = fixedcolormap;      // fixed map
  else if (frame & FF_FULLBRIGHT)
  {
    vis->colormap[0] = vis->colormap[1] = fullcolormap;       // full bright  // killough 3/20/98
    vis->fullbright = true; // [Nugget]
  }
  else
    {      // diminished light
      const int index = STRICTMODE(!diminishing_lighting) // [Nugget]
                        ? 0 : R_GetLightIndex(xscale);

      // [Nugget] Thing lighting
      if (STRICTMODE(thing_lighting_mode) == THINGLIGHTING_HITBOX)
      {
        int lightlevel = 0;

        for (int i = 0;  i < 9;  i++)
        {
          const fixed_t gx = vis->gx + (thing->radius * ((i % 3) - 1)),
                        gy = vis->gy + (thing->radius * ((i / 3) - 1));

          lightlevel += R_GetLightLevelInPoint(gx, gy);
        }

        int lightnum = ((lightlevel / 9) >> LIGHTSEGSHIFT) + extralight;

        spritelights = scalelight[BETWEEN(0, LIGHTLEVELS-1, lightnum)];
      }

      vis->colormap[0] = spritelights[index];
      vis->colormap[1] = fullcolormap;
    }

  vis->brightmap = R_BrightmapForState(thing->state - states);
  if (vis->brightmap == nobrightmap)
    vis->brightmap = R_BrightmapForSprite(sprite);

  vis->tranmap = thing->tranmap; // [Nugget]

  // [Alaux] Lock crosshair on target
  if (STRICTMODE(hud_crosshair_lockon) && thing == crosshair_target
      // [Nugget]
      && (!(crosshair_target->flags & MF_SHADOW) || hud_crosshair_fuzzy))
  {
    if (STRICTMODE(flip_levels)) { txc = -txc; } // [Nugget] Flip levels

    HU_UpdateCrosshairLock
    (
      BETWEEN(0, viewwidth  - 1, (centerxfrac + FixedMul(txc, xscale)) >> FRACBITS),
      // [Nugget] Removed `actualheight`
      BETWEEN(0, viewheight - 1, (centeryfrac + FixedMul(viewz - interpz - crosshair_target->height/2, xscale)) >> FRACBITS)
    );

    crosshair_target = NULL; // Don't update it again until next tic
  }

  // [Nugget] Sprite shadows -------------------------------------------------

  if (!(R_SpriteShadowsOn()
        && (xscale > FRACUNIT/4)
        && !(frame & FF_FULLBRIGHT)
        && !(   (thing->flags & (MF_SHADOW|MF_TRANSLUCENT))
             || (thing->flags & MF_SPAWNCEILING && thing->flags & MF_NOGRAVITY))))
  {
    return;
  }

  fixed_t floorheight, shadow_xscale, shadow_yscale, shadow_gz, shadow_gzt;

  floorheight = R_PointInSubsector(interpx, interpy)->sector->floorheight;

  if (viewz < floorheight + FRACUNIT) { return; }

  int offset_divisor = 0;
  fixed_t yscale_mult;

  fixed_t floordist = MAX(0, interpz - floorheight) * 10/32;
          floordist = FixedMul(floordist, floordist / 8);

  if (sprite_shadows == SPRITESHADOWS_3D)
  {
    #define SHADOW_HEIGHT_DIVISOR 2
    offset_divisor = 8;

    const fixed_t shadow_depth = spriteheight[lump] / SHADOW_HEIGHT_DIVISOR,
                  shadow_dist  = tz + shadow_depth - shadow_depth / offset_divisor;

    const angle_t angle = P_SlopeToPitch(FixedDiv(viewz - floorheight, shadow_dist));

    yscale_mult = finesine[angle >> ANGLETOFINESHIFT] / SHADOW_HEIGHT_DIVISOR;

    floordist /= SHADOW_HEIGHT_DIVISOR * 10;
    yscale_mult -= FixedMul(floordist, yscale_mult);
  }
  else {
    const fixed_t base_yscale_mult = FRACUNIT/10;
    offset_divisor = 4;

    floordist = FixedMul(floordist, base_yscale_mult / 2);
    yscale_mult = base_yscale_mult - FixedMul(floordist, base_yscale_mult);
  }

  shadow_yscale = FixedMul(xscale, yscale_mult);

  if (shadow_yscale <= FRACUNIT * 3/256) { return; }

  shadow_xscale = FixedMul(xscale, FRACUNIT - floordist);

  const fixed_t shadow_height = FixedMul(spriteheight[lump], yscale_mult);

  shadow_gz  = floorheight - shadow_height / offset_divisor;
  shadow_gzt = shadow_gz   + shadow_height;

  // Could clip shadows out of view due to height

  // The following `R_NewVisSprite()` call may leave `vis` dangling,
  // so we'll copy it beforehand
  const vissprite_t vis_copy = *vis;

  vissprite_t *const shadow_vis = R_NewVisSprite();

  *shadow_vis = vis_copy;

  shadow_vis->scale--; // Draw behind main sprite

  shadow_vis->gz = shadow_gz;
  shadow_vis->gzt = shadow_gzt;
  shadow_vis->texturemid = shadow_vis->gzt - viewz;

  shadow_vis->xscale = shadow_xscale;
  shadow_vis->yscale = shadow_yscale;

  const fixed_t midx = centerxfrac + FixedMul64(txc, xscale);
  fixed_t shadow_tx;

  shadow_tx = flip ? spritewidth[lump] - spriteoffset[lump] : spriteoffset[lump];
  const fixed_t shadow_x1 = (midx - FixedMul64(shadow_tx, shadow_xscale)) >> FRACBITS;

  shadow_vis->x1 = MAX(0, shadow_x1);

  shadow_tx = spritewidth[lump];
  const fixed_t shadow_x2 = ((midx + FixedMul64(shadow_tx, shadow_xscale)) >> FRACBITS) - 1;

  shadow_vis->x2 = MIN(viewwidth - 1, shadow_x2);

  const fixed_t shadow_iscale = FixedDiv(FRACUNIT, shadow_xscale);

  if (flip) {
    shadow_vis->startfrac = spritewidth[lump]-1;
    shadow_vis->xiscale = -shadow_iscale;
  }
  else {
    shadow_vis->startfrac = 0;
    shadow_vis->xiscale = shadow_iscale;
  }

  if (shadow_vis->x1 > shadow_x1)
  { shadow_vis->startfrac += shadow_vis->xiscale * (shadow_vis->x1 - shadow_x1); }

  shadow_vis->mobjflags |= MF_TRANSLUCENT;

  // Thing lighting: set true to make per-column lighting not apply to shadows
  shadow_vis->fullbright = true;
}

//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//

boolean draw_nearby_sprites;

// killough 9/18/98: add lightlevel as parameter, fixing underwater lighting
void R_AddSprites(sector_t* sec, int lightlevel)
{
  mobj_t *thing;
  int    lightnum;

  // BSP is traversed by subsector.
  // A sector might have been split into several
  //  subsectors during BSP building.
  // Thus we check whether its already added.

  if (sec->validcount == validcount)
    return;

  // Well, now it will be done.
  sec->validcount = validcount;

  if (demo_version <= DV_BOOM)
    lightlevel = sec->lightlevel;

  lightnum = (lightlevel >> LIGHTSEGSHIFT)+extralight;

  if (lightnum < 0)
    spritelights = scalelight[0];
  else if (lightnum >= LIGHTLEVELS)
    spritelights = scalelight[LIGHTLEVELS-1];
  else
    spritelights = scalelight[lightnum];

  // Handle all things in sector.

  for (thing = sec->thinglist; thing; thing = thing->snext)
    R_ProjectSprite(thing);

  if (STRICTMODE(draw_nearby_sprites))
  {
    for (msecnode_t *n = sec->touching_thinglist; n; n = n->m_snext)
    {
      thing = n->m_thing;

      // [FG] sprites in sector have already been projected
      if (thing->subsector->sector->validcount != validcount)
      {
        array_push(nearby_sprites, thing);
      }
    }
  }
}

void R_NearbySprites (void)
{
  for (int i = 0; i < array_size(nearby_sprites); i++)
  {
    mobj_t *thing = nearby_sprites[i];
    sector_t* sec = thing->subsector->sector;

    // [FG] sprites in sector have already been projected
    if (sec->validcount != validcount)
    {
      int lightnum = (sec->lightlevel >> LIGHTSEGSHIFT) + extralight;

      if (lightnum < 0)
        spritelights = scalelight[0];
      else if (lightnum >= LIGHTLEVELS)
        spritelights = scalelight[LIGHTLEVELS-1];
      else
        spritelights = scalelight[lightnum];

      R_ProjectSprite(thing);
    }
  }

  array_clear(nearby_sprites);
}

static int queued_weapon_voxels = 0; // [Nugget] Weapon voxels

//
// R_DrawPSprite
//

void R_DrawPSprite (pspdef_t *psp, boolean translucent) // [Nugget] Translucent flashes
{
  fixed_t       tx;
  int           x1, x2;
  spritedef_t   *sprdef;
  spriteframe_t *sprframe;
  int           lump;
  boolean       flip;
  vissprite_t   *vis;
  vissprite_t   avis;

  // [Nugget] Weapon voxels
  if (VX_ProjectWeaponVoxel(psp, translucent))
  {
    queued_weapon_voxels++;
    return;
  }
  // If any are queued, don't draw sprites
  else if (queued_weapon_voxels) { return; }

  // decide which patch to use

#ifdef RANGECHECK
  if ((unsigned) psp->state->sprite >= num_sprites)
    I_Error ("R_DrawPSprite: invalid sprite number %i", psp->state->sprite);
#endif

  sprdef = &sprites[psp->state->sprite];

#ifdef RANGECHECK
  if ((psp->state->frame&FF_FRAMEMASK) >= sprdef->numframes)
    I_Error ("R_DrawPSprite: invalid frame %i for sprite %s",
             (int)(psp->state->frame & FF_FRAMEMASK),
             sprnames[psp->state->sprite]);
#endif

  sprframe = &sprdef->spriteframes[psp->state->frame & FF_FRAMEMASK];

  lump = sprframe->lump[0];
  flip = (boolean) sprframe->flip[0];

  fixed_t sx2, sy2;

  fixed_t wix, wiy; // [Nugget]

  if (uncapped && oldleveltime < leveltime)
  {
    sx2 = LerpFixed(psp->oldsx2, psp->sx2);
    sy2 = LerpFixed(psp->oldsy2, psp->sy2);

    wix = LerpFixed(psp->oldwix, psp->wix);
    wiy = LerpFixed(psp->oldwiy, psp->wiy);
  }
  else
  {
    sx2 = psp->sx2;
    sy2 = psp->sy2;

    wix = psp->wix;
    wiy = psp->wiy;
  }

  // calculate edges of the shape
  tx = sx2 - 160*FRACUNIT; // [FG] centered weapon sprite

  // [Nugget] Weapon inertia | Flip levels
  if (STRICTMODE(weapon_inertia))
  { tx += flip_levels ? -wix : wix; }

  tx -= spriteoffset[lump];
  x1 = (centerxfrac + FixedMul (tx,pspritescale))>>FRACBITS;

  // off the right side
  if (x1 > viewwidth)
    return;

  tx +=  spritewidth[lump];
  x2 = ((centerxfrac + FixedMul (tx, pspritescale) ) >>FRACBITS) - 1;

  // off the left side
  if (x2 < 0)
    return;

  // store information in a vissprite
  vis = &avis;
  vis->mobjflags = translucent ? MF_TRANSLUCENT : 0; // [Nugget] Translucent flashes
  vis->mobjflags2 = 0;

  // killough 12/98: fix psprite positioning problem
  vis->texturemid = (BASEYCENTER<<FRACBITS) /* + FRACUNIT/2 */ -
                    (sy2 - spritetopoffset[lump]); // [FG] centered weapon sprite

  // [Nugget]
  vis->texturemid += (STRICTMODE(weapon_inertia) ? -wiy : 0) // Weapon inertia
                   + MIN(0, R_GetFOVFX(FOVFX_ZOOM) * FRACUNIT/2); // Lower weapon based on zoom

  vis->x1 = x1 < 0 ? 0 : x1;
  vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;
  vis->scale = pspritescale;

  // [Nugget]
  vis->yscale = 0;
  vis->fullbright = true; // Thing lighting: set true to make per-column lighting not apply to psprites
  vis->flipped = flip;

  if (flip)
    {
      vis->xiscale = -pspriteiscale;
      vis->startfrac = spritewidth[lump]-1;
    }
  else
    {
      vis->xiscale = pspriteiscale;
      vis->startfrac = 0;
    }

  if (vis->x1 > x1)
    vis->startfrac += vis->xiscale*(vis->x1-x1);

  vis->patch = lump;

  // killough 7/11/98: beta psprites did not draw shadows
  if (POWER_RUNOUT(viewplayer->powers[pw_invisibility]) && !beta_emulation)

    vis->colormap[0] = vis->colormap[1] = NULL;                    // shadow draw
  else if (fixedcolormap)
    vis->colormap[0] = vis->colormap[1] = fixedcolormap;           // fixed color
  else if (psp->state->frame & FF_FULLBRIGHT)
  {
    vis->colormap[0] = vis->colormap[1] = fullcolormap;            // full bright // killough 3/20/98
    vis->fullbright = true; // [Nugget]
  }
  else
  {
    // [Nugget]
    const int index = (STRICTMODE(!diminishing_lighting)) ? 0 : MAXLIGHTSCALE-1;

    vis->colormap[0] = spritelights[index];  // local light
    vis->colormap[1] = fullcolormap;
  }
  vis->brightmap = R_BrightmapForState(psp->state - states);

  // [Nugget] Translucent flashes
  vis->tranmap = translucent ? R_GetGenericTranMap(pspr_translucency_pct) : NULL;

  // [crispy] free look
  vis->texturemid += (centery - viewheight/2) * pspriteiscale;

  // [Nugget] NUGHUD
  if (STRICTMODE(ST_GetNughudOn()))
  { vis->texturemid -= nughud.weapheight*FRACUNIT; }

  if (STRICTMODE(hide_weapon)
      // [Nugget]
      || R_ChasecamOn() // Chasecam
      || R_FreecamOn() // Freecam
      || (WI_AltInterpicOn() && gamestate == GS_INTERMISSION)) // Alt. intermission background
    return;

  R_DrawVisSprite(vis, vis->x1, vis->x2);
}

//
// R_DrawPlayerSprites
//

void R_DrawPlayerSprites(void)
{
  int i, lightnum;
  pspdef_t *psp;
  sector_t tmpsec;
  int floorlightlevel, ceilinglightlevel;

  // get light level
  // killough 9/18/98: compute lightlevel from floor and ceiling lightlevels
  // (see r_bsp.c for similar calculations for non-player sprites)

  // [Nugget] Thing lighting
  if (STRICTMODE(thing_lighting_mode) >= THINGLIGHTING_HITBOX)
  {
    int lightlevel = 0;

    for (int i = 0;  i < 9;  i++)
    {
      const fixed_t gx = viewx + (viewplayer->mo->radius * ((i % 3) - 1)),
                    gy = viewy + (viewplayer->mo->radius * ((i / 3) - 1));

      sector_t *const sector = R_PointInSubsector(gx, gy)->sector;

      R_FakeFlat(sector, &tmpsec, &floorlightlevel, &ceilinglightlevel, 0);

      lightlevel += floorlightlevel + ceilinglightlevel;
    }

    lightnum = ((lightlevel / 9) >> (LIGHTSEGSHIFT+1)) + extralight;
  }
  else
  {
    R_FakeFlat(viewplayer->mo->subsector->sector, &tmpsec,
               &floorlightlevel, &ceilinglightlevel, 0);
    lightnum = ((floorlightlevel+ceilinglightlevel) >> (LIGHTSEGSHIFT+1))
      + extralight;
  }

  if (lightnum < 0)
    spritelights = scalelight[0];
  else if (lightnum >= LIGHTLEVELS)
    spritelights = scalelight[LIGHTLEVELS-1];
  else
    spritelights = scalelight[lightnum];

  // clip to screen bounds
  mfloorclip = screenheightarray;
  mceilingclip = negonearray;

  // display crosshair
  if (hud_crosshair_on) // [Nugget] Use crosshair toggle
    HU_DrawCrosshair();

  queued_weapon_voxels = 0; // [Nugget] Weapon voxels

  // add all active psprites
  for (i=0, psp=viewplayer->psprites;
       // [Nugget]: [crispy] A11Y number of player (first person) sprites to draw
       i < ((strictmode || a11y_weapon_pspr) ? NUMPSPRITES : ps_flash);
       i++,psp++)
    if (psp->state)
      R_DrawPSprite (psp, i == ps_flash && STRICTMODE(pspr_translucency_pct != 100)); // [Nugget] Translucent flashes

  // [Nugget] Weapon voxels
  if (queued_weapon_voxels)
  {
    const fixed_t old_centeryfrac = centeryfrac;

    // Hack to make weapon voxels be unaffected by view pitch
    if (default_vertical_aiming != VERTAIM_AUTO)
    { centeryfrac = (viewheight / 2 + (R_GetNughudViewPitch() >> FRACBITS)) << FRACBITS; }

    // Drawn in reverse order
    for (i = 0;  i < queued_weapon_voxels;  i++)
    { VX_DrawVoxel(&vissprites[num_vissprite - (1 + i)]); }

    centeryfrac = old_centeryfrac;
  }
}

//
// R_SortVisSprites
//
// Rewritten by Lee Killough to avoid using unnecessary
// linked lists, and to use faster sorting algorithm.
//

#define bcopyp(d, s, n) memcpy(d, s, (n) * sizeof(void *))

// killough 9/2/98: merge sort

static void msort(vissprite_t **s, vissprite_t **t, int n)
{
  if (n >= 16)
    {
      int n1 = n/2, n2 = n - n1;
      vissprite_t **s1 = s, **s2 = s + n1, **d = t;

      msort(s1, t, n1);
      msort(s2, t, n2);

      while ((*s1)->scale >= (*s2)->scale ?
             (*d++ = *s1++, --n1) : (*d++ = *s2++, --n2));

      if (n2)
        bcopyp(d, s2, n2);
      else
        bcopyp(d, s1, n1);

      bcopyp(s, t, n);
    }
  else
    {
      int i;
      for (i = 1; i < n; i++)
        {
          vissprite_t *temp = s[i];
          if (s[i-1]->scale < temp->scale)
            {
              int j = i;
              while ((s[j] = s[j-1])->scale < temp->scale && --j);
              s[j] = temp;
            }
        }
    }
}

void R_SortVisSprites (void)
{
  if (num_vissprite)
    {
      int i = num_vissprite;

      // If we need to allocate more pointers for the vissprites,
      // allocate as many as were allocated for sprites -- killough
      // killough 9/22/98: allocate twice as many

      if (num_vissprite_ptrs < num_vissprite*2)
        {
          Z_Free(vissprite_ptrs);  // better than realloc -- no preserving needed
          vissprite_ptrs = Z_Malloc((num_vissprite_ptrs = num_vissprite_alloc*2)
                                  * sizeof *vissprite_ptrs, PU_STATIC, 0);
        }

      // Sprites of equal distance need to be sorted in inverse order.
      // This is most easily achieved by filling the sort array
      // backwards before the sort.

      while (--i>=0)
        vissprite_ptrs[num_vissprite-i-1] = vissprites+i;

      // killough 9/22/98: replace qsort with merge sort, since the keys
      // are roughly in order to begin with, due to BSP rendering.

      msort(vissprite_ptrs, vissprite_ptrs + num_vissprite, num_vissprite);
    }
}

//
// R_DrawSprite
//

void R_DrawSprite (vissprite_t* spr)
{
  drawseg_t *ds;
  int     x;
  int     r1;
  int     r2;
  fixed_t scale;
  fixed_t lowscale;

  for (x = spr->x1 ; x<=spr->x2 ; x++)
    clipbot[x] = cliptop[x] = -2;

  // Scan drawsegs from end to start for obscuring segs.
  // The first drawseg that has a greater scale is the clip seg.

  // Modified by Lee Killough:
  // (pointer check was originally nonportable
  // and buggy, by going past LEFT end of array):

  //    for (ds=ds_p-1 ; ds >= drawsegs ; ds--)    old buggy code

  // [Woof!] Andrey Budko: optimization
  if (drawsegs_xrange_size)
  {
    const drawseg_xrange_item_t *last = &drawsegs_xrange[drawsegs_xrange_count - 1];
    drawseg_xrange_item_t *curr = &drawsegs_xrange[-1];
    while (++curr <= last)
    {
      // determine if the drawseg obscures the sprite
      if (curr->x1 > spr->x2 || curr->x2 < spr->x1)
        continue;      // does not cover sprite

      ds = curr->user;

      if (ds->scale1 > ds->scale2)
        {
          lowscale = ds->scale2;
          scale = ds->scale1;
        }
      else
        {
          lowscale = ds->scale1;
          scale = ds->scale2;
        }

      if (scale < spr->scale || (lowscale < spr->scale &&
                    !R_PointOnSegSide (spr->gx, spr->gy, ds->curline)))
      {
        if (ds->maskedtexturecol)       // masked mid texture?
        {
          r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
          r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;
          R_RenderMaskedSegRange(ds, r1, r2);
        }
        continue;               // seg is behind sprite
      }

      r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
      r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

      // clip this piece of the sprite
      // killough 3/27/98: optimized and made much shorter

      if (ds->silhouette&SIL_BOTTOM && spr->gz < ds->bsilheight) //bottom sil
        for (x=r1 ; x<=r2 ; x++)
          if (clipbot[x] == -2)
            clipbot[x] = ds->sprbottomclip[x];

      if (ds->silhouette&SIL_TOP && spr->gzt > ds->tsilheight)   // top sil
        for (x=r1 ; x<=r2 ; x++)
          if (cliptop[x] == -2)
            cliptop[x] = ds->sprtopclip[x];
    }
  }

  // killough 3/27/98:
  // Clip the sprite against deep water and/or fake ceilings.
  // killough 4/9/98: optimize by adding mh
  // killough 4/11/98: improve sprite clipping for underwater/fake ceilings
  // killough 11/98: fix disappearing sprites

  if (spr->heightsec != -1)  // only things in specially marked sectors
    {
      fixed_t h,mh;
      int phs = viewplayer->mo->subsector->sector->heightsec;
      if ((mh = sectors[spr->heightsec].floorheight) > spr->gz &&
          (h = centeryfrac - FixedMul(mh-=viewz, spr->scale)) >= 0 &&
          (h >>= FRACBITS) < viewheight)
      {
        if (mh <= 0 || (phs != -1 && viewz > sectors[phs].floorheight))
          {                          // clip bottom
            for (x=spr->x1 ; x<=spr->x2 ; x++)
              if (clipbot[x] == -2 || h < clipbot[x])
                clipbot[x] = h;
          }
        else                        // clip top
          if (phs != -1 && viewz <= sectors[phs].floorheight) // killough 11/98
            for (x=spr->x1 ; x<=spr->x2 ; x++)
              if (cliptop[x] == -2 || h > cliptop[x])
                cliptop[x] = h;
      }

      if ((mh = sectors[spr->heightsec].ceilingheight) < spr->gzt &&
          (h = centeryfrac - FixedMul(mh-viewz, spr->scale)) >= 0 &&
          (h >>= FRACBITS) < viewheight)
      {
        if (phs != -1 && viewz >= sectors[phs].ceilingheight)
          {                         // clip bottom
            for (x=spr->x1 ; x<=spr->x2 ; x++)
              if (clipbot[x] == -2 || h < clipbot[x])
                clipbot[x] = h;
          }
        else                       // clip top
          for (x=spr->x1 ; x<=spr->x2 ; x++)
            if (cliptop[x] == -2 || h > cliptop[x])
              cliptop[x] = h;
      }
    }
  // killough 3/27/98: end special clipping for deep water / fake ceilings

  // all clipping has been performed, so draw the sprite
  // check for unclipped columns

  for (x = spr->x1 ; x<=spr->x2 ; x++)
    {
      if (clipbot[x] == -2)
        clipbot[x] = viewheight;

      if (cliptop[x] == -2)
        cliptop[x] = -1;
    }

  mfloorclip = clipbot;
  mceilingclip = cliptop;

  // andrewj: voxel support
  if (spr->voxel_index >= 0)
    VX_DrawVoxel (spr);
  else
    R_DrawVisSprite (spr, spr->x1, spr->x2);
}

//
// R_DrawMasked
//

void R_DrawMasked(void)
{
  int i;
  drawseg_t *ds;

  R_SortVisSprites();

  // [Woof!] Andrey Budko
  // Reducing of cache misses in the following R_DrawSprite()
  // Makes sense for scenes with huge amount of drawsegs.
  // ~12% of speed improvement on epic.wad map05
  for(i = 0; i < DS_RANGES_COUNT; i++)
    drawsegs_xranges[i].count = 0;

  if (num_vissprite > 0)
  {
    if (drawsegs_xrange_size < maxdrawsegs)
    {
      drawsegs_xrange_size = 2 * maxdrawsegs;
      for(i = 0; i < DS_RANGES_COUNT; i++)
      {
        drawsegs_xranges[i].items = Z_Realloc(
          drawsegs_xranges[i].items,
          drawsegs_xrange_size * sizeof(drawsegs_xranges[i].items[0]),
          PU_STATIC, 0);
      }
    }
    for (ds = ds_p; ds-- > drawsegs;)
    {
      if (ds->silhouette || ds->maskedtexturecol)
      {
        drawsegs_xranges[0].items[drawsegs_xranges[0].count].x1 = ds->x1;
        drawsegs_xranges[0].items[drawsegs_xranges[0].count].x2 = ds->x2;
        drawsegs_xranges[0].items[drawsegs_xranges[0].count].user = ds;

        // Andrey Budko: ~13% of speed improvement on sunder.wad map10
        if (ds->x1 < centerx)
        {
          drawsegs_xranges[1].items[drawsegs_xranges[1].count] =
            drawsegs_xranges[0].items[drawsegs_xranges[0].count];
          drawsegs_xranges[1].count++;
        }
        if (ds->x2 >= centerx)
        {
          drawsegs_xranges[2].items[drawsegs_xranges[2].count] =
            drawsegs_xranges[0].items[drawsegs_xranges[0].count];
          drawsegs_xranges[2].count++;
        }

        drawsegs_xranges[0].count++;
      }
    }
  }

  // draw all vissprites back to front

  for (i = num_vissprite ;--i>=0; )
  {
    vissprite_t* spr = vissprite_ptrs[i];

    if (spr->x2 < centerx)
    {
      drawsegs_xrange = drawsegs_xranges[1].items;
      drawsegs_xrange_count = drawsegs_xranges[1].count;
    }
    else if (spr->x1 >= centerx)
    {
      drawsegs_xrange = drawsegs_xranges[2].items;
      drawsegs_xrange_count = drawsegs_xranges[2].count;
    }
    else
    {
      drawsegs_xrange = drawsegs_xranges[0].items;
      drawsegs_xrange_count = drawsegs_xranges[0].count;
    }

    R_DrawSprite(vissprite_ptrs[i]);         // killough
  }

  // render any remaining masked mid textures

  // Modified by Lee Killough:
  // (pointer check was originally nonportable
  // and buggy, by going past LEFT end of array):

  //    for (ds=ds_p-1 ; ds >= drawsegs ; ds--)    old buggy code

  for (ds=ds_p ; ds-- > drawsegs ; )  // new -- killough
    if (ds->maskedtexturecol)
      R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

  // [Nugget] Flip levels
  if (STRICTMODE(flip_levels))
  {
    for (int y = 0;  y < viewheight;  y++)
    {
      for (int x = 0;  x < viewwidth/2;  x++)
      {
        pixel_t *const restrict left = &I_VideoBuffer[(viewwindowy + y) * video.pitch + viewwindowx + x],
                *const restrict right = left + viewwidth - 1 - x*2,
                temp = *left;

        *left = *right;
        *right = temp;
      }
    }
  }

  // draw the psprites on top of everything
  //  but does not draw on side views
  if (!viewangleoffset)
    R_DrawPlayerSprites ();
}

//----------------------------------------------------------------------------
//
// $Log: r_things.c,v $
// Revision 1.22  1998/05/03  22:46:41  killough
// beautification
//
// Revision 1.21  1998/05/01  15:26:50  killough
// beautification
//
// Revision 1.20  1998/04/27  02:04:43  killough
// Fix incorrect I_Error format string
//
// Revision 1.19  1998/04/24  11:03:26  jim
// Fixed bug in sprites in PWAD
//
// Revision 1.18  1998/04/13  09:45:30  killough
// Fix sprite clipping under fake ceilings
//
// Revision 1.17  1998/04/12  02:02:19  killough
// Fix underwater sprite clipping, add wall translucency
//
// Revision 1.16  1998/04/09  13:18:48  killough
// minor optimization, plus fix ghost sprites due to huge z-height diffs
//
// Revision 1.15  1998/03/31  19:15:27  killough
// Fix underwater sprite clipping bug
//
// Revision 1.14  1998/03/28  18:15:29  killough
// Add true deep water / fake ceiling sprite clipping
//
// Revision 1.13  1998/03/23  03:41:43  killough
// Use 'fullcolormap' for fully-bright colormap
//
// Revision 1.12  1998/03/16  12:42:37  killough
// Optimize away some function pointers
//
// Revision 1.11  1998/03/09  07:28:16  killough
// Add primitive underwater support
//
// Revision 1.10  1998/03/02  11:48:59  killough
// Add failsafe against texture mapping overflow crashes
//
// Revision 1.9  1998/02/23  04:55:52  killough
// Remove some comments
//
// Revision 1.8  1998/02/20  22:53:22  phares
// Moved TRANMAP initialization to w_wad.c
//
// Revision 1.7  1998/02/20  21:56:37  phares
// Preliminarey sprite translucency
//
// Revision 1.6  1998/02/09  03:23:01  killough
// Change array decl to use MAX screen width/height
//
// Revision 1.5  1998/02/02  13:32:49  killough
// Performance tuning, program beautification
//
// Revision 1.4  1998/01/26  19:24:50  phares
// First rev with no ^Ms
//
// Revision 1.3  1998/01/26  06:13:58  killough
// Performance tuning
//
// Revision 1.2  1998/01/23  20:28:14  jim
// Basic sprite/flat functionality in PWAD added
//
// Revision 1.1.1.1  1998/01/19  14:03:06  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
