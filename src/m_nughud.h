//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//  Copyright(C) 2020-2021 Fabian Greffrath
//  Copyright(C) 2021-2024 Alaux
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
//  Variant of m_misc.h specifically for declaration and loading of NUGHUD
//  variables

#ifndef __M_NUGHUD__
#define __M_NUGHUD__

#include "m_fixed.h"

#define NUGHUDWIDESHIFT(x) (                                       \
  st_crispyhud ? (abs(x) == 2) ? video.deltaw      * (2 / (x)) :   \
                 (abs(x) == 1) ? distributed_delta *      (x)  : 0 \
               : 0                                                 \
)

typedef struct nughud_widget_s {
  int x, y;
  int wide;
} nughud_widget_t;

typedef struct nughud_alignable_s {
  int x, y;
  int wide;
  int align;
} nughud_alignable_t;

typedef struct nughud_vlignable_s {
  int x, y;
  int wide;
  int align;
  int vlign;
} nughud_vlignable_t;

typedef struct nughud_bar_s {
  int x, y;
  int wide;
  int align;
  int ups; // Units per slice
  int gap;
} nughud_bar_t;

typedef struct nughud_textline_s {
  int x, y;
  int wide;
  int align;
  int vlign;
  int stack;
  int order;
} nughud_textline_t;

typedef struct nughud_sbchunk_s {
  int x, y;
  int wide;
  int sx, sy, sw, sh;
} nughud_sbchunk_t;

#define NUMNUGHUDPATCHES 8
#define NUMNUGHUDSTACKS 8
#define NUMSBCHUNKS 8

typedef struct nughud_s {
  nughud_alignable_t ammo;
  nughud_vlignable_t ammoicon;
  boolean            ammoicon_big;
  nughud_bar_t       ammobar;
  nughud_alignable_t health;
  nughud_vlignable_t healthicon;
  nughud_bar_t       healthbar;
  nughud_widget_t    arms[9];
  nughud_alignable_t frags;
  nughud_widget_t    face;
  boolean            face_bg;
  nughud_alignable_t armor;
  nughud_vlignable_t armoricon;
  nughud_bar_t       armorbar;
  nughud_widget_t    keys[3];
  nughud_alignable_t ammos[4];
  nughud_alignable_t maxammos[4];

  nughud_textline_t  time;
  nughud_textline_t  sts;
  int                sts_ml;
  nughud_textline_t  title;
  nughud_textline_t  powers;
  nughud_textline_t  coord;
  int                coord_ml;
  nughud_textline_t  fps;
  nughud_textline_t  rate;
  nughud_textline_t  message;
  boolean            message_defx;
  nughud_textline_t  secret;

  nughud_vlignable_t stacks[NUMNUGHUDSTACKS];

  nughud_vlignable_t patches[NUMNUGHUDPATCHES];
  char               *patchnames[NUMNUGHUDPATCHES];
  boolean            patch_offsets;

  nughud_sbchunk_t   sbchunks[NUMSBCHUNKS];

  boolean percents;
  fixed_t weapheight;
  int     viewoffset;
} nughud_t;

extern nughud_t nughud;

void M_NughudLoadDefaults(void);
void M_NughudLoadOptions(void);

#endif
