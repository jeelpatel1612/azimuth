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
#ifndef AZIMUTH_GUI_SCREEN_H_
#define AZIMUTH_GUI_SCREEN_H_

#include <stdbool.h>

/*===========================================================================*/

typedef void (*az_init_func_t)(void);

// Register a function to be called each time OpenGL is (re)initialized, which
// happens when calling az_init_gui and/or az_set_fullscreen.  Any calls made
// to this function must be made _before_ calling az_init_gui.
void az_register_gl_init_func(az_init_func_t func);

// Initialize the GUI/window.  This should be called exactly once, at program
// startup, before making any OpenGL calls.
void az_init_gui(bool fullscreen, bool enable_audio);

// Query whether we are currently in full-screen mode.
bool az_is_fullscreen(void);

// Set whether the GUI is full-screen or not.  This must not be called before
// az_init_gui.
void az_set_fullscreen(bool fullscreen);

// Call this functions just before and just after drawing a frame to the screen
// with OpenGL.
void az_start_screen_redraw(void);
void az_finish_screen_redraw(void);

/*===========================================================================*/

#endif // AZIMUTH_GUI_SCREEN_H_
