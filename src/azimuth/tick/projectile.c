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

#include "azimuth/tick/projectile.h"

#include "azimuth/state/projectile.h"
#include "azimuth/state/space.h"
#include "azimuth/util/misc.h"

/*===========================================================================*/

static void tick_projectile(az_space_state_t *state, az_projectile_t *proj,
                            double time) {
  // Age the projectile, and remove it if it is expired.
  proj->age += time;
  if (proj->age > proj->lifetime) {
    proj->kind = AZ_PROJ_NOTHING;
    return;
  }

  // Move the projectile by its velocity:
  proj->position = az_vadd(proj->position, az_vmul(proj->velocity, time));

  // Check if the projectile hits anything.  If it was fired by an enemy, it
  // can hit the ship:
  if (proj->fired_by_enemy) {
    // TODO: check if hits ship
  }
  // Or, if the projectile was fired by the ship, it can hit baddies:
  else {
    AZ_ARRAY_LOOP(baddie, state->baddies) {
      if (baddie->kind == AZ_BAD_NOTHING) continue;
      if (az_vwithin(baddie->position, proj->position, 20.0)) {
        az_particle_t *particle;
        if (az_insert_particle(state, &particle)) {
          particle->kind = AZ_PAR_BOOM;
          particle->color = (az_color_t){255, 255, 255, 255};
          particle->position = proj->position;
          particle->velocity = AZ_VZERO;
          particle->lifetime = 0.3;
          particle->param1 = 10;
        }
        baddie->health -= 1.0;
        if (baddie->health <= 0.0) {
          baddie->kind = AZ_BAD_NOTHING;
          az_try_add_pickup(state, AZ_PUP_LARGE_SHIELDS, baddie->position);
        }
        proj->kind = AZ_PROJ_NOTHING;
        return;
      }
    }
  }
  // If it didn't hit the ship or a baddie already, the projectile can hit
  // walls (unless this kind of projectile passes through walls):
  if (!az_proj_kind_passes_through_walls(proj->kind)) {
    if (false) return; // TODO: check for wall collisions
  }

  // The projectile still hasn't hit anything.  Apply kind-specific logic to
  // the projectile (e.g. homing projectiles will home in).
  switch (proj->kind) {
    case AZ_PROJ_BOMB:
      //maybe_detonate_bomb(state, proj);
      break;
    // TODO: implement logic for other special projectile kinds here
    default: break;
  }
}

void az_tick_projectiles(az_space_state_t *state, double time) {
  AZ_ARRAY_LOOP(proj, state->projectiles) {
    if (proj->kind == AZ_PROJ_NOTHING) continue;
    tick_projectile(state, proj, time);
  }
}

/*===========================================================================*/