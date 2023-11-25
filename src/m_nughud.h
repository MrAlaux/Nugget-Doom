//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//  Copyright(C) 2020-2021 Fabian Greffrath
//  Copyright(C) 2021-2023 Alaux
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

#define NUGHUDWIDESHIFT(x) ((abs(x) == 2) ? WIDESCREENDELTA * (st_crispyhud != 0) * (2 / (x)) :  \
                            (abs(x) == 1) ? WIDESCREENDELTA * (st_crispyhud == 2) *      (x)  : 0)

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

#define NUMNUGHUDPATCHES 8

typedef struct nughud_s {
  nughud_alignable_t ammo;
  nughud_vlignable_t ammoicon;
  boolean            ammoicon_big;
  nughud_alignable_t health;
  nughud_vlignable_t healthicon;
  nughud_widget_t    arms[9];
  nughud_alignable_t frags;
  nughud_widget_t    face;
  boolean            face_bg;
  nughud_alignable_t armor;
  nughud_vlignable_t armoricon;
  nughud_widget_t    keys[3];
  nughud_alignable_t ammos[4];
  nughud_alignable_t maxammos[4];
  nughud_alignable_t time;
  boolean            time_sts;
  nughud_alignable_t sts;
  boolean            sts_ml;
  nughud_alignable_t title;
  nughud_alignable_t powers;
  nughud_alignable_t coord;
  boolean            coord_ml;
  nughud_alignable_t fps;
  nughud_alignable_t message;
  nughud_alignable_t secret;
  nughud_vlignable_t patches[NUMNUGHUDPATCHES];
  char               *patchnames[NUMNUGHUDPATCHES];
  boolean            patch_offsets;

  boolean            percents;
  fixed_t            weapheight;
} nughud_t;

extern nughud_t nughud;

void M_NughudLoadDefaults(void);
void M_NughudLoadOptions(void);

#endif
