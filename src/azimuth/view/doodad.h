/*=============================================================================
| Copyright 2012 Matthew D. Steele <mdsteele@alum.mit.edu>                    |
|                                                                             |
| This file is part of Azimuth.                                               |
|                                                                             |
| Azimuth is free software: you can redistribute it and/or modify it under    |
| the terms of the GNU General Public License as published by the Free        |
| Software Foundation, either version 3 of the License, or (at your option)   |
| any later version.                                                          |
|                                                                             |
| Azimuth is distributed in the hope that it will be useful, but WITHOUT      |
| ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       |
| FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for   |
| more details.                                                               |
|                                                                             |
| You should have received a copy of the GNU General Public License along     |
| with Azimuth.  If not, see <http://www.gnu.org/licenses/>.                  |
=============================================================================*/

#pragma once
#ifndef AZIMUTH_VIEW_DOODAD_H_
#define AZIMUTH_VIEW_DOODAD_H_

#include "azimuth/state/node.h"
#include "azimuth/util/clock.h"

/*===========================================================================*/

// Draw a single doodad.  The GL matrix should be at the doodad's position.
void az_draw_doodad(az_doodad_kind_t doodad_kind, az_clock_t clock);

/*===========================================================================*/

#endif // AZIMUTH_VIEW_DOODAD_H_
