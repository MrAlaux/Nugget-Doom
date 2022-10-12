//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//  Copyright(C) 2020-2021 Fabian Greffrath
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
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
//
// DESCRIPTION:
//  Variant of m_misc.h specifically for declaration and loading of NUGHUD
//  variables

#ifndef __M_NUGHUD__
#define __M_NUGHUD__

typedef struct nughud_widget_s {
  int x;
  int y;
  int wide;
  int bg;
} nughud_widget_t;

typedef struct nughud_s {
  struct nughud_widget_s ammo;
  struct nughud_widget_s health;
  struct nughud_widget_s arms[8];
  struct nughud_widget_s frags;
  struct nughud_widget_s face;
  struct nughud_widget_s armor;
  struct nughud_widget_s keys[3];
  struct nughud_widget_s ammos[4];
  struct nughud_widget_s maxammos[4];
} nughud_t;

extern nughud_t nughud;

void M_NughudLoadDefaults(void);
struct default_s *M_NughudLookupDefault(const char *name);
boolean M_NughudParseOption(const char *name, boolean wad);
void M_NughudLoadOptions(void);

#endif
