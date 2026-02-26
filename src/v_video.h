//
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//  Copyright (C) 2013 James Haley et al.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// DESCRIPTION:
//  Color range translation support
//  Functions to draw patches (by post) directly to screen.
//  Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------

#ifndef __V_VIDEO__
#define __V_VIDEO__

// [Nugget]
#include <string.h>

#include "doomtype.h"
#include "m_fixed.h"

// [Nugget]
#include "r_state.h"

struct patch_s;

//
// VIDEO
//

extern int v_lightest_color, v_darkest_color;

// jff 2/16/98 palette color ranges for translation
// jff 2/18/98 conversion to palette lookups for speed
// jff 4/24/98 now pointers to lumps loaded
extern byte *cr_brick;
extern byte *cr_tan;
extern byte *cr_gray;
extern byte *cr_green;
extern byte *cr_brown;
extern byte *cr_gold;
extern byte *cr_red;
extern byte *cr_blue;
extern byte *cr_blue2;
extern byte *cr_orange;
extern byte *cr_yellow;
extern byte *cr_black;
extern byte *cr_purple;
extern byte *cr_white;
// [FG] dark/shaded color translation table
extern byte *cr_dark;
extern byte *cr_shaded;
extern byte *cr_bright;

extern lighttable_t invul_gray[];

// [Nugget]
extern byte cr_bright3[],
            cr_allblack[],
            cr_gray_vc[],  // `V_Colorize()` only
            nightvision[]; // Night-vision visor

// array of pointers to color translation tables
extern byte *colrngs[];
extern byte *red2col[];

// symbolic indices into color translation table pointer array
typedef enum
{
    CR_ORIG = -1,
    CR_BRICK,  // 0
    CR_TAN,    // 1
    CR_GRAY,   // 2
    CR_GREEN,  // 3
    CR_BROWN,  // 4
    CR_GOLD,   // 5
    CR_RED,    // 6
    CR_BLUE1,  // 7
    CR_ORANGE, // 8
    CR_YELLOW, // 9
    CR_BLUE2,  // 10
    CR_BLACK,  // 11
    CR_PURPLE, // 12
    CR_WHITE,  // 13
    CR_NONE,   // 14 // [FG] dummy
    CR_BRIGHT, // 15
    CR_LIMIT   // 16 //jff 2/27/98 added for range check
} crange_idx_e;

#define ORIG_S  "\x1b\x2f"
#define BRICK_S "\x1b\x30"
#define TAN_S   "\x1b\x31"
#define GRAY_S  "\x1b\x32"
#define GREEN_S "\x1b\x33"
#define BROWN_S "\x1b\x34"
#define GOLD_S  "\x1b\x35"
#define RED_S   "\x1b\x36"
#define BLUE1_S "\x1b\x37"
#define BLUE2_S "\x1b\x3a"

// jff 1/16/98 end palette color range additions

crange_idx_e V_CRByName(const char *name);

extern pixel_t *I_VideoBuffer;
extern pixel32_t *I_VideoBuffer32;

// jff 4/24/98 loads color translation lumps
void V_InitColorTranslation(void);

typedef struct
{
    int width;
    int height;
    int pitch;
    int unscaledw; // unscaled width with correction for widecreen
    int deltaw;    // widescreen delta

    fixed_t xscale; // x-axis scaling multiplier
    fixed_t yscale; // y-axis scaling multiplier
    fixed_t xstep;  // x-axis scaling step
    fixed_t ystep;  // y-axis scaling step
} video_t;

extern video_t video;

typedef struct
{
    int x;   // original x coordinate for upper left corner
    int y;   // original y coordinate for upper left corner
    int w;   // original width
    int h;   // original height

    int cx1; // clipped x coordinate for left edge
    int cx2; // clipped x coordinate for right edge
    int cy1; // clipped y coordinate for upper edge
    int cy2; // clipped y coordinate for lower edge
    int cw;  // clipped width
    int ch;  // clipped height

    int sx;  // scaled x
    int sy;  // scaled y
    int sw;  // scaled width
    int sh;  // scaled height
} vrect_t;

void V_ScaleRect(vrect_t *rect);
int V_ScaleX(int x);
int V_ScaleY(int y);

// Allocates buffer screens, call before R_Init.
void V_Init(void);

void V_UseBuffer(pixel_t *buffer);
void V_UseBuffer32(pixel32_t *buffer);

void V_RestoreBuffer(void);

void V_CopyRect(int srcx, int srcy, pixel_t *source, int width, int height,
                int destx, int desty);

void V_CopyRect32(int srcx, int srcy, pixel32_t *source, int width, int height,
                  int destx, int desty);

// killough 11/98: Consolidated V_DrawPatch and V_DrawPatchFlipped

void V_DrawPatchGeneral(int x, int y, struct patch_s *patch, boolean flipped);

#define V_DrawPatch(x, y, p) V_DrawPatchGeneral(x, y, p, false)

#define V_DrawPatchFlipped(x, y, p) V_DrawPatchGeneral(x, y, p, true)

void V_DrawPatchTranslated(int x, int y, struct patch_s *patch, byte *outr);

void V_DrawPatchTRTR(int x, int y, struct patch_s *patch, byte *outr1,
                     byte *outr2);

void V_DrawPatchTL(int x, int y, struct patch_s *patch, byte *tl);

void V_DrawPatchTRTL(int x, int y, struct patch_s *patch, byte *outr, byte *tl);

// [Nugget] /=================================================================

extern int automap_overlay_darkening;
extern int menu_backdrop_darkening;

void V_SetPatchCrop(int left, int right, int top, int bottom, boolean shadow_only);
void V_ClearPatchCrop(void);

// True color ----------------------------------------------------------------

extern int *palcolors, palscolors[14][256];

void V_InitPalsColors(void);
void V_SetPalColors(const int palette_index);
void V_SetCurrentColormap(const int colormap_index);

inline static lighttable_t *V_ColormapRowByIndex(const cmapoffset_t row_index)
{
  return fullcolormap + row_index;
}

inline static lighttable32_t *V_ColormapRowByIndex32(const cmapoffset_t row_index)
{
  return fullcolormap32 + row_index;
}

// Avoids shifting right then left
inline static uint_fast16_t V_TranMapRowFromRGB(const pixel32_t rgb)
{
  return (rgb & PIXEL_INDEX_MASK) >> (PIXEL_INDEX_SHIFT - 8);
}

inline static pixel32_t V_IndexToRGB(const pixel_t index)
{
  // Should use palcolors
  return fullcolormap32[index];
}

inline static pixel32_t V_IndexToColormapRGB(const pixel_t index)
{
  return fullcolormap32[index];
}

#define V_IndexSet(dest, color, count) V_RGBSet(dest, V_IndexToRGB(color), count)
inline static void V_RGBSet(pixel32_t *const dest, const pixel32_t color, const int count)
{
  for (int i = 0;  i < count;  i++) { dest[i] = color; }
}

inline static void V_IndexCopy(pixel32_t *dest, const pixel_t *src, int count)
{
  while (count--) { *dest++ = V_IndexToRGB(*src++); }
}

inline static void V_RGBCopy(pixel32_t *const dest, const pixel32_t *const src, const int count)
{
  memcpy(dest, src, sizeof(pixel32_t) * count);
}

inline static pixel32_t V_LerpRGB(const pixel32_t a, const pixel32_t b, const double factor)
{
  const byte
    ar = V_RedFromRGB(a),
    ag = V_GreenFromRGB(a),
    ab = V_BlueFromRGB(a),
    br = V_RedFromRGB(b),
    bg = V_GreenFromRGB(b),
    bb = V_BlueFromRGB(b);

  #define CALC(a, b, shift) ( \
    (pixel32_t) ((a) + ((b) - (a)) * factor) << shift \
  )

  return ((factor >= 0.5 ? b : a) & PIXEL_INDEX_MASK)
       | CALC(ar, br, PIXEL_RED_SHIFT)
       | CALC(ag, bg, PIXEL_GREEN_SHIFT)
       | CALC(ab, bb, PIXEL_BLUE_SHIFT);

  #undef CALC
}

inline static pixel_t V_ShadeRGB(const pixel32_t rgb, const double factor)
{
  return (rgb & PIXEL_INDEX_MASK)
       | ((pixel32_t) (V_RedFromRGB(rgb)   * factor) << PIXEL_RED_SHIFT)
       | ((pixel32_t) (V_GreenFromRGB(rgb) * factor) << PIXEL_GREEN_SHIFT)
       | ((pixel32_t) (V_BlueFromRGB(rgb)  * factor) << PIXEL_BLUE_SHIFT);
}

void V_InitColorFunctions(void);

// HUD/menu shadows ----------------------------------------------------------

extern boolean hud_menu_shadows;
extern int hud_menu_shadows_filter_pct;

void V_InitShadowTranMap(void);
void V_ToggleShadows(const boolean on);

// ---------------------------------------------------------------------------

void V_DrawPatchTRTRTL(int x, int y, struct patch_s *patch,
                       byte *outr1, byte *outr2, byte *tl);

void V_DrawPatchTranslucent2(int x, int y, struct patch_s *patch, boolean flipped,
                             byte *outr1, byte *outr2, byte *tl);

#define V_DrawPatchTL2(x, y, p, tp) \
  V_DrawPatchTranslucent2(x, y, p, false, NULL, NULL, tp)

#define V_DrawPatchFlippedTL2(x, y, p, tp) \
  V_DrawPatchTranslucent2(x, y, p, true, NULL, NULL, tp)

#define V_DrawPatchTRTL2(x, y, p, cr, tp) \
  V_DrawPatchTranslucent2(x, y, p, false, cr, NULL, tp)

#define V_DrawPatchTRTRTL2(x, y, p, cr1, cr2, tp) \
  V_DrawPatchTranslucent2(x, y, p, false, cr1, cr2, tp)

void V_DrawPatchShadowed(int x, int y, struct patch_s *patch, boolean flipped,
                         byte *outr1, byte *outr2, byte *tl);

#define V_DrawPatchSH(x, y, p) \
  V_DrawPatchShadowed(x, y, p, false, NULL, NULL, NULL)

#define V_DrawPatchFlippedSH(x, y, p) \
  V_DrawPatchShadowed(x, y, p, true, NULL, NULL, NULL)

#define V_DrawPatchTranslatedSH(x, y, p, cr) \
  V_DrawPatchShadowed(x, y, p, false, cr, NULL, NULL)

#define V_DrawPatchTRTRSH(x, y, p, cr1, cr2) \
  V_DrawPatchShadowed(x, y, p, false, cr1, cr2, NULL)

#define V_DrawPatchTLSH(x, y, p, tp) \
  V_DrawPatchShadowed(x, y, p, false, NULL, NULL, tp)

#define V_DrawPatchTRTLSH(x, y, p, cr, tp) \
  V_DrawPatchShadowed(x, y, p, false, cr, NULL, tp)

#define V_DrawPatchTRTRTLSH(x, y, p, cr1, cr2, tp) \
  V_DrawPatchShadowed(x, y, p, false, cr1, cr2, tp)

extern void (*V_ShadowRect)(int x, int y, int width, int height);

#define SHADOW_REDRAW(_code_)                     \
  if (hud_menu_shadows)                           \
  {                                               \
    const boolean old_shadows = hud_menu_shadows; \
    hud_menu_shadows = false;                     \
                                                  \
    _code_;                                       \
                                                  \
    hud_menu_shadows = old_shadows;               \
  }

// [Nugget] =================================================================/

void V_DrawPatchFullScreen(struct patch_s *patch);

// Draw a linear block of pixels into the view buffer.

void V_DrawBlock(int x, int y, int width, int height, pixel_t *src);
void V_DrawBlock32(int x, int y, int width, int height, pixel32_t *src);

// Reads a linear block of pixels into the view buffer.

void V_GetBlock(int x, int y, int width, int height, pixel_t *dest);
void V_GetBlock32(int x, int y, int width, int height, pixel32_t *dest);

// [FG] non hires-scaling variant of V_DrawBlock, used in disk icon drawing

void V_PutBlock(int x, int y, int width, int height, pixel_t *src);
void V_PutBlock32(int x, int y, int width, int height, pixel32_t *src);

extern void (*V_FillRect)(int x, int y, int width, int height, pixel_t color);
void V_FillRectRGB(int x, int y, int width, int height, pixel32_t color);

extern void (*V_TileBlock64)(int line, int width, int height, const byte *src);

void V_DrawBackground(const char *patchname);

extern void (*V_ShadeScreen)(int level); // [Nugget]

// [FG] colored blood and gibs

int V_BloodColor(int blood);

void V_ScreenShot(void);

#endif

//----------------------------------------------------------------------------
//
// $Log: v_video.h,v $
// Revision 1.9  1998/05/06  11:12:54  jim
// Formattted v_video.*
//
// Revision 1.8  1998/05/03  22:53:58  killough
// beautification
//
// Revision 1.7  1998/04/24  08:09:44  jim
// Make text translate tables lumps
//
// Revision 1.6  1998/03/02  11:43:06  killough
// Add cr_blue_status for blue statusbar numbers
//
// Revision 1.5  1998/02/27  19:22:11  jim
// Range checked hud/sound card variables
//
// Revision 1.4  1998/02/19  16:55:06  jim
// Optimized HUD and made more configurable
//
// Revision 1.3  1998/02/17  23:00:41  jim
// Added color translation machinery and data
//
// Revision 1.2  1998/01/26  19:27:59  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:05  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
