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
//  Variant of m_misc.h specifically for loading of NUGHUD variables

#ifndef __M_NUGHUD__
#define __M_NUGHUD__

#include "doomtype.h"
#include "doomdef.h"

#include "m_input.h"

#include "m_misc.h"

//
// MISC
//

void M_NughudLoadDefaults(void);
struct default_s *M_NughudLookupDefault(const char *name);
boolean M_NughudParseOption(const char *name, boolean wad);
void M_NughudLoadOptions(void);

#endif
