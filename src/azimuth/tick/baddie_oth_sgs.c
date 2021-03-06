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

#include "azimuth/tick/baddie_oth_sgs.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include "azimuth/state/baddie.h"
#include "azimuth/state/baddie_oth.h"
#include "azimuth/state/space.h"
#include "azimuth/tick/baddie_oth.h"
#include "azimuth/tick/baddie_util.h"
#include "azimuth/tick/object.h"
#include "azimuth/util/random.h"

/*===========================================================================*/

enum {
  INITIAL_STATE = 0,
  DOGFIGHT_STATE,
  BEAM_SWEEP_STATE,
  CHARGED_BEAM_STATE,
  RETREAT_STATE,
  TRY_TO_CLOAK_STATE,
  SNEAK_UP_BEHIND_STATE,
  AMBUSH_STATE,
  FLEE_WHILE_CLOAKED_STATE,
  BARRAGE_STATE,
  CRAZY_STATE
};

// Engine constants:
#define TURN_RATE (AZ_DEG2RAD(330))
#define MAX_SPEED 350.0
#define FORWARD_ACCEL 300.0

/*===========================================================================*/

static int get_primary_state(az_baddie_t *baddie) {
  return baddie->state & 0xff;
}

static int get_secondary_state(az_baddie_t *baddie) {
  return (baddie->state >> 8) & 0xff;
}

static int get_tertiary_state(az_baddie_t *baddie) {
  return (baddie->state >> 16) & 0xff;
}

static void set_primary_state(az_baddie_t *baddie, int primary) {
  assert(primary >= 0 && primary <= 0xff);
  baddie->state = primary;
}

static void set_secondary_state(az_baddie_t *baddie, int secondary) {
  assert(secondary >= 0 && secondary <= 0xff);
  baddie->state = (baddie->state & 0xff) | (secondary << 8);
}

static void set_tertiary_state(az_baddie_t *baddie, int tertiary) {
  assert(tertiary >= 0 && tertiary <= 0xff);
  baddie->state = (baddie->state & 0xffff) | (tertiary << 16);
}

/*===========================================================================*/

#define TRACTOR_MIN_LENGTH 100.0
#define TRACTOR_MAX_LENGTH 300.0
#define TRACTOR_COMPONENT_INDEX 6
#define TRACTOR_CLOAK_CHARGE_TIME 2.5
#define TRACTOR_CLOAK_DECAY_TIME 8.0

static bool tractor_locked_on(
    const az_baddie_t *baddie, az_vector_t *tractor_pos_out,
    double *tractor_length_out) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  const az_component_t *component =
    &baddie->components[TRACTOR_COMPONENT_INDEX];
  const double tractor_length = component->angle;
  if (tractor_length == 0) return false;
  assert(tractor_length >= TRACTOR_MIN_LENGTH &&
         tractor_length <= TRACTOR_MAX_LENGTH);
  if (tractor_pos_out != NULL) *tractor_pos_out = component->position;
  if (tractor_length_out != NULL) *tractor_length_out = tractor_length;
  return true;
}

static void set_tractor_node(az_baddie_t *baddie, const az_node_t *node) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  if (node == NULL) {
    baddie->components[TRACTOR_COMPONENT_INDEX].angle = 0;
    return;
  }
  assert(node->kind == AZ_NODE_TRACTOR);
  const double tractor_length = az_vdist(baddie->position, node->position);
  assert(tractor_length >= TRACTOR_MIN_LENGTH &&
         tractor_length <= TRACTOR_MAX_LENGTH);
  baddie->components[TRACTOR_COMPONENT_INDEX].position = node->position;
  baddie->components[TRACTOR_COMPONENT_INDEX].angle = tractor_length;
}

static void decloak_immediately(az_space_state_t *state, az_baddie_t *baddie) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  if (baddie->param > 1.0) {
    baddie->param = 1.0;
    az_play_sound(&state->soundboard, AZ_SND_CLOAK_END);
  }
}

/*===========================================================================*/

// Fly towards the goal position, getting as close as possible.
static void fly_towards(az_space_state_t *state, az_baddie_t *baddie,
                        double time, az_vector_t goal) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  az_fly_towards_position(state, baddie, time, goal, TURN_RATE, MAX_SPEED,
                          FORWARD_ACCEL, 200, 100);
}

static void fly_towards_ship(az_space_state_t *state, az_baddie_t *baddie,
                             double time) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP ||
         baddie->kind == AZ_BAD_OTH_DECOY);
  az_fly_towards_ship(state, baddie, time, TURN_RATE, MAX_SPEED,
                      FORWARD_ACCEL, 200, 200, 100);
}

static void fly_away_from_ship(az_space_state_t *state, az_baddie_t *baddie,
                               double time, az_vector_t *goal_position_out) {
  // Out of all marker nodes we can see, pick the one farthest from the ship.
  double best_dist = 0.0;
  az_vector_t target = baddie->position;
  AZ_ARRAY_LOOP(node, state->nodes) {
    if (node->kind != AZ_NODE_MARKER) continue;
    if (az_baddie_has_clear_path_to_position(state, baddie, node->position)) {
      const double dist = az_vdist(node->position, state->ship.position);
      if (dist > best_dist) {
        best_dist = dist;
        target = node->position;
      }
    }
  }
  fly_towards(state, baddie, time, target);
  if (goal_position_out != NULL) *goal_position_out = target;
}

static void fire_projectiles(az_space_state_t *state, az_baddie_t *baddie,
                             az_proj_kind_t kind, int num_shots,
                             double dtheta, az_sound_key_t sound) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  assert(num_shots > 0);
  decloak_immediately(state, baddie);
  const double theta0 = -0.5 * dtheta * (num_shots - 1);
  for (int i = 0; i < num_shots; ++i) {
    az_projectile_t *proj = az_fire_baddie_projectile(
        state, baddie, kind, 18, 0, theta0 + dtheta * i);
    if (kind == AZ_PROJ_OTH_PHASE_ROCKET && proj != NULL) {
      proj->param = (i % 2 == 0 ? -1 : 1);
    }
  }
  az_play_sound(&state->soundboard, sound);
}

static void fire_oth_spray(az_space_state_t *state, az_baddie_t *baddie) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  for (int i = 0; i < 360; i += 15) {
    az_fire_baddie_projectile(
        state, baddie, AZ_PROJ_OTH_SPRAY,
        baddie->data->main_body.bounding_radius, AZ_DEG2RAD(i), 0);
  }
  az_play_sound(&state->soundboard, AZ_SND_FIRE_OTH_SPRAY);
  decloak_immediately(state, baddie);
}

static void spawn_razors(az_space_state_t *state, az_vector_t position,
                         double angle, int step_degrees, double speed) {
  for (int i = 0; i < 360; i += step_degrees) {
    const double theta = angle + AZ_DEG2RAD(i);
    az_baddie_t *razor =
      az_add_baddie(state, AZ_BAD_OTH_RAZOR_2,
                    az_vadd(position, az_vpolar(15, theta)), theta);
    if (razor != NULL) {
      razor->velocity = az_vpolar(speed, theta);
      razor->cooldown = 0.25; // incorporeal for this much time
    }
  }
}

/*===========================================================================*/

static void begin_dogfight(az_baddie_t *baddie) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  set_primary_state(baddie, DOGFIGHT_STATE);
  set_secondary_state(baddie, az_randint(10, 20));
  baddie->cooldown = 0.2;
}

static void resume_dogfight(az_baddie_t *baddie) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  set_primary_state(baddie, DOGFIGHT_STATE);
  set_secondary_state(baddie, az_randint(4, 7));
  baddie->cooldown = 0.2;
}

static void begin_beam_sweep(az_baddie_t *baddie) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  set_primary_state(baddie, BEAM_SWEEP_STATE);
  baddie->cooldown = 3.0;
}

static void begin_charged_beam(az_baddie_t *baddie) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  set_primary_state(baddie, CHARGED_BEAM_STATE);
  baddie->cooldown = 3.0;
}

static void begin_retreat(az_baddie_t *baddie) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  set_primary_state(baddie, RETREAT_STATE);
  baddie->cooldown = 5.0;
}

static void begin_charging_phase_missiles(az_space_state_t *state,
                                          az_baddie_t *baddie) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  set_tractor_node(baddie, NULL);
  decloak_immediately(state, baddie);
  set_primary_state(baddie, BARRAGE_STATE);
  baddie->cooldown = 2.0;
  az_play_sound(&state->soundboard, AZ_SND_MAGBEEST_MAGNET_CHARGE);
}

/*===========================================================================*/

void az_tick_bad_oth_supergunship(
    az_space_state_t *state, az_baddie_t *baddie, double time) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  const double old_angle = baddie->angle;
  const double hurt = 1.0 - baddie->health / baddie->data->max_health;

  if (hurt >= 0.95 && get_primary_state(baddie) != CRAZY_STATE) {
    set_primary_state(baddie, CRAZY_STATE);
  }

  // Handle tractor beam:
  az_vector_t tractor_position;
  double tractor_length;
  if (tractor_locked_on(baddie, &tractor_position, &tractor_length)) {
    baddie->position = az_vadd(az_vwithlen(az_vsub(baddie->position,
                                                   tractor_position),
                                           tractor_length),
                               tractor_position);
    baddie->velocity = az_vflatten(baddie->velocity,
        az_vsub(baddie->position, tractor_position));
    az_loop_sound(&state->soundboard, AZ_SND_TRACTOR_BEAM);
    baddie->param += time / TRACTOR_CLOAK_CHARGE_TIME;
    if (baddie->param >= 1.0) {
      az_play_sound(&state->soundboard, AZ_SND_CLOAK_BEGIN);
      baddie->param = 1.0 + TRACTOR_CLOAK_DECAY_TIME;
      set_tractor_node(baddie, NULL);
    }
  } else if (baddie->param >= 0.0) {
    const double old_param = baddie->param;
    baddie->param = fmax(0.0, baddie->param - time);
    if (old_param > 1.0 && baddie->param <= 1.0) {
      az_play_sound(&state->soundboard, AZ_SND_CLOAK_END);
    }
  }
  // If the Oth Supergunship is cloaked, homing weapons can't track it.
  if (baddie->param >= 1.0) {
    baddie->temp_properties |= AZ_BADF_NO_HOMING;
    // Try to prevent collisions with the player's ship while cloaked, since
    // those feel kind of unfair.
    if (az_vwithin(baddie->position, state->ship.position, 50.0) &&
        baddie->armor_flare <= 0.0) {
      baddie->temp_properties |= AZ_BADF_INCORPOREAL;
    }
  }

  switch (get_primary_state(baddie)) {
    case INITIAL_STATE: {
      begin_dogfight(baddie);
    } break;
    case DOGFIGHT_STATE: {
      // For dogfighting, secondary state is number of shots left; tertiary
      // state is positive if we're getting ready to fire a charged phase shot.
      fly_towards_ship(state, baddie, time);
      int secondary = get_secondary_state(baddie);
      if (baddie->cooldown <= 0.0) {
        if (az_can_see_ship(state, baddie)) {
          if (hurt >= 0.2 && secondary >= 8 && secondary <= 9 &&
              az_ship_in_range(state, baddie, 250)) {
            begin_beam_sweep(baddie);
          } else if (az_ship_within_angle(state, baddie, 0, AZ_DEG2RAD(3))) {
            const double cross_speed =
              az_vnorm(az_vflatten(state->ship.velocity,
                                   az_vsub(state->ship.position,
                                           baddie->position)));
            if (cross_speed < 40.0 && !az_ship_in_range(state, baddie, 100)) {
              fire_projectiles(state, baddie, AZ_PROJ_OTH_ROCKET, 1, 0,
                               AZ_SND_FIRE_OTH_ROCKET);
              baddie->cooldown = 0.6;
              secondary -= 3;
            } else {
              fire_projectiles(state, baddie, AZ_PROJ_OTH_BULLET, 3,
                               AZ_DEG2RAD(10), AZ_SND_FIRE_GUN_NORMAL);
              baddie->cooldown = 0.2;
              --secondary;
            }
            if (secondary <= 0) {
              begin_retreat(baddie);
            } else {
              set_secondary_state(baddie, secondary);
            }
          }
        } else if (get_tertiary_state(baddie) >= 10) {
          if (az_ship_within_angle(state, baddie, 0, AZ_DEG2RAD(3))) {
            fire_projectiles(state, baddie, AZ_PROJ_OTH_CHARGED_PHASE, 1,
                             0, AZ_SND_FIRE_GUN_CHARGED_PHASE);
            baddie->cooldown = 0.6;
            secondary -= 4;
            if (secondary <= 0) {
              begin_retreat(baddie);
            } else {
              set_secondary_state(baddie, secondary);
              set_tertiary_state(baddie, 10);
            }
          }
        } else {
          set_tertiary_state(baddie, get_tertiary_state(baddie) + 1);
          baddie->cooldown = 0.2;
        }
      }
    } break;
    case BEAM_SWEEP_STATE: {
      if (az_ship_is_decloaked(&state->ship)) {
        const bool beam_is_on = (get_secondary_state(baddie) != 0);
        // Turn towards the ship.
        baddie->angle = az_angle_towards(
            baddie->angle, (beam_is_on ? AZ_DEG2RAD(90) : TURN_RATE) * time,
            az_vtheta(az_vsub(state->ship.position, baddie->position)));
        // Turn on the beam once we're almost facing the ship.
        if (!beam_is_on &&
            az_ship_within_angle(state, baddie, 0, AZ_DEG2RAD(30))) {
          set_secondary_state(baddie, 1);
          baddie->cooldown = 3.0;
        }
      }
      // If the beam has been turned on, fire the beam.
      if (get_secondary_state(baddie) != 0) {
        decloak_immediately(state, baddie);
        const az_vector_t beam_start =
          az_vadd(az_vpolar(18, baddie->angle), baddie->position);
        az_impact_t impact;
        az_ray_impact(state, beam_start, az_vpolar(5000, baddie->angle),
                      AZ_IMPF_BADDIE, baddie->uid, &impact);
        if (impact.type == AZ_IMP_SHIP) {
          az_damage_ship(state, 20.0 * time, false);
        }
        // Add particles/sound for the beam.
        const uint8_t alt = 128 + 16 * az_clock_zigzag(6, 1, state->clock);
        const az_color_t beam_color = {224, 255, alt, 192};
        az_add_beam(state, beam_color, beam_start, impact.position, 0.0,
                    (3.0 + 0.5 * az_clock_zigzag(8, 1, state->clock)));
        az_add_speck(state, AZ_WHITE, 1.0, impact.position,
                     az_vpolar(az_random(20.0, 70.0),
                               az_vtheta(impact.normal) +
                               az_random(-AZ_HALF_PI, AZ_HALF_PI)));
        az_loop_sound(&state->soundboard, AZ_SND_BEAM_NORMAL);
      }
      // When we run out of time, switch modes.
      if (baddie->cooldown <= 0.0) {
        if (get_secondary_state(baddie) != 0) {
          begin_charged_beam(baddie);
        } else resume_dogfight(baddie);
      }
    } break;
    case CHARGED_BEAM_STATE: {
      az_loop_sound(&state->soundboard, AZ_SND_CHARGED_GUN);
      bool ready_to_fire = false;
      if (get_secondary_state(baddie) == 0) {
        fly_towards_ship(state, baddie, time);
        AZ_ARRAY_LOOP(proj, state->projectiles) {
          if (proj->kind == AZ_PROJ_NOTHING) continue;
          if (proj->fired_by != AZ_SHIP_UID) continue;
          if (proj->data->properties & AZ_PROJF_NO_HIT) continue;
          if (az_vwithin(proj->position, baddie->position, 200)) {
            ready_to_fire = true;
            break;
          }
        }
      } else if (az_ship_is_decloaked(&state->ship)) {
        fly_towards(state, baddie, time, state->ship.position);
        if (az_ship_in_range(state, baddie, 120)) ready_to_fire = true;
      } else ready_to_fire = true;
      if (ready_to_fire) {
        fire_projectiles(state, baddie, AZ_PROJ_OTH_CHARGED_BEAM, 1, 0,
                         AZ_SND_FIRE_GUN_CHARGED_BEAM);
        resume_dogfight(baddie);
      } else if (baddie->cooldown <= 0.0) {
        resume_dogfight(baddie);
      }
    } break;
    case RETREAT_STATE: {
      az_vector_t goal_position;
      fly_away_from_ship(state, baddie, time, &goal_position);
      // Use an Orion boost to accelerate away.
      if (get_secondary_state(baddie) == 0 &&
          az_ship_in_range(state, baddie, 200) &&
          az_ship_within_angle(state, baddie, AZ_PI, AZ_DEG2RAD(60)) &&
          fabs(az_mod2pi(az_vtheta(az_vsub(goal_position, baddie->position)) -
                         baddie->angle)) < AZ_DEG2RAD(5) &&
          az_baddie_has_clear_path_to_position(state, baddie,
              az_vadd(baddie->position, az_vpolar(500, baddie->angle)))) {
        az_projectile_t *proj = az_fire_baddie_projectile(
            state, baddie, AZ_PROJ_OTH_ORION_BOMB,
            baddie->data->main_body.bounding_radius, AZ_PI, 0);
        if (proj != NULL) {
          az_vpluseq(&proj->velocity, baddie->velocity);
          az_play_sound(&state->soundboard, AZ_SND_ORION_BOOSTER);
        }
        decloak_immediately(state, baddie);
        set_secondary_state(baddie, 1);
      }
      // If we get far away enough, or if the cooldown expires, change states.
      if (baddie->cooldown <= 0.0 || !az_ship_in_range(state, baddie, 800)) {
        baddie->cooldown = 0.0;
        if (az_ship_in_range(state, baddie, 300)) {
          fire_oth_spray(state, baddie);
          begin_dogfight(baddie);
        } else {
          set_primary_state(baddie, TRY_TO_CLOAK_STATE);
        }
      }
    } break;
    case TRY_TO_CLOAK_STATE: {
      fly_away_from_ship(state, baddie, time, NULL);
      if (!tractor_locked_on(baddie, NULL, NULL)) {
        az_node_t *nearest = NULL;
        double best_dist = INFINITY;
        AZ_ARRAY_LOOP(node, state->nodes) {
          if (node->kind != AZ_NODE_TRACTOR) continue;
          const double dist = az_vdist(baddie->position, node->position);
          if (dist >= TRACTOR_MIN_LENGTH && dist < best_dist) {
            nearest = node;
            best_dist = dist;
          }
        }
        if (nearest != NULL && best_dist <= TRACTOR_MAX_LENGTH) {
          set_tractor_node(baddie, nearest);
        }
      }
      if (baddie->param >= 1.0) {
        spawn_razors(state, baddie->position, baddie->angle + AZ_DEG2RAD(90),
                     180, 100);
        bool flee = az_ship_in_range(state, baddie, 250);
        if (az_random(0, 1) < 0.5 * hurt) flee = !flee;
        if (flee) {
          set_primary_state(baddie, FLEE_WHILE_CLOAKED_STATE);
        } else {
          set_primary_state(baddie, SNEAK_UP_BEHIND_STATE);
        }
      }
    } break;
    case SNEAK_UP_BEHIND_STATE: {
      if (baddie->param < 1.0) {
        if (az_ship_in_range(state, baddie, 200)) {
          fire_oth_spray(state, baddie);
        }
        begin_dogfight(baddie);
      } else if (az_ship_is_decloaked(&state->ship)) {
        const az_vector_t target =
          az_vadd(az_vpolar(-75, state->ship.angle), state->ship.position);
        if (az_vwithin(baddie->position, target, 50)) {
          set_primary_state(baddie, AMBUSH_STATE);
          baddie->cooldown = 1.0;
        } else if (baddie->armor_flare <= 0.0) {
          az_fly_towards_position(state, baddie, time, target, 2 * TURN_RATE,
                                  1.5 * MAX_SPEED, 2.5 * FORWARD_ACCEL, 200,
                                  100);
        } else fly_towards(state, baddie, time, target);
      } else {
        set_primary_state(baddie, FLEE_WHILE_CLOAKED_STATE);
      }
    } break;
    case AMBUSH_STATE: {
      az_vpluseq(&baddie->velocity, az_vcaplen(az_vneg(baddie->velocity),
                                               300 * time));
      const az_vector_t delta_to_ship =
        az_vsub(state->ship.position, baddie->position);
      az_vector_t rel_impact;
      if (!az_lead_target(delta_to_ship, state->ship.velocity, 1200,
                          &rel_impact)) {
        rel_impact = delta_to_ship;
      }
      baddie->angle = az_angle_towards(baddie->angle, 2 * TURN_RATE * time,
                                       az_vtheta(rel_impact));
      if (baddie->cooldown <= 0.0 ||
          az_ship_within_angle(state, baddie, 0, AZ_DEG2RAD(2))) {
        fire_projectiles(state, baddie, AZ_PROJ_OTH_ROCKET, 1, 0,
                         AZ_SND_FIRE_OTH_ROCKET);
        begin_dogfight(baddie);
      }
    } break;
    case FLEE_WHILE_CLOAKED_STATE: {
      fly_away_from_ship(state, baddie, time, NULL);
      if (az_ship_is_decloaked(&state->ship) &&
          !az_ship_in_range(state, baddie, 500)) {
        begin_charging_phase_missiles(state, baddie);
      } else if (baddie->param < 1.0) {
        if (az_ship_in_range(state, baddie, 200)) {
          fire_oth_spray(state, baddie);
        }
        begin_dogfight(baddie);
      }
    } break;
    case BARRAGE_STATE: {
      az_vpluseq(&baddie->velocity, az_vcaplen(az_vneg(baddie->velocity),
                                               300 * time));
      const az_vector_t delta_to_ship =
        az_vsub(state->ship.position, baddie->position);
      az_vector_t rel_impact;
      if (!az_lead_target(delta_to_ship, state->ship.velocity, 1200,
                          &rel_impact)) {
        rel_impact = delta_to_ship;
      }
      baddie->angle = az_angle_towards(baddie->angle, TURN_RATE * time,
                                       az_vtheta(rel_impact));
      if (baddie->cooldown <= 0.0) {
        const int secondary = get_secondary_state(baddie);
        if (secondary == 0 && az_can_see_ship(state, baddie)) {
          fire_projectiles(state, baddie, AZ_PROJ_OTH_BARRAGE, 1, 0,
                           AZ_SND_NOTHING);
          begin_dogfight(baddie);
        } else {
          fire_projectiles(state, baddie, AZ_PROJ_OTH_PHASE_ROCKET, 2, 0,
                           AZ_SND_FIRE_OTH_ROCKET);
          if (secondary >= 2) {
            begin_dogfight(baddie);
          } else {
            set_secondary_state(baddie, secondary + 1);
            baddie->cooldown = 0.4;
          }
        }
      }
    } break;
    case CRAZY_STATE: {
      decloak_immediately(state, baddie);
      set_tractor_node(baddie, NULL);
      baddie->cooldown = fmin(0.3, baddie->cooldown);
      if (az_ship_is_decloaked(&state->ship)) {
        fly_towards(state, baddie, time, state->ship.position);
      }
      if (baddie->cooldown <= 0.0) {
        for (int i = 0; i < 2; ++i) {
          az_fire_baddie_projectile(state, baddie, AZ_PROJ_OTH_MINIROCKET, 12,
                                    az_random(-AZ_PI, AZ_PI), 0);
        }
        az_play_sound(&state->soundboard, AZ_SND_FIRE_OTH_ROCKET);
        baddie->cooldown = 0.3;
        int secondary = get_secondary_state(baddie);
        if (secondary >= 3) {
          set_secondary_state(baddie, 0);
          spawn_razors(state, baddie->position, az_random(-AZ_PI, AZ_PI), 360,
                       150);
          az_play_sound(&state->soundboard, AZ_SND_LAUNCH_OTH_RAZORS);
        } else {
          set_secondary_state(baddie, secondary + 1);
        }
      }
    } break;
    default:
      begin_dogfight(baddie);
      break;
  }

  az_tick_oth_tendrils(baddie, &AZ_OTH_SUPERGUNSHIP_TENDRILS, old_angle, time,
                       state->ship.player.total_time);
}

void az_on_oth_supergunship_damaged(
    az_space_state_t *state, az_baddie_t *baddie, double amount,
    az_damage_flags_t damage_kind) {
  assert(baddie->kind == AZ_BAD_OTH_SUPERGUNSHIP);
  if (damage_kind & (AZ_DMGF_ROCKET | AZ_DMGF_BOMB | AZ_DMGF_CPLUS)) {
    decloak_immediately(state, baddie);
    set_tractor_node(baddie, NULL);
    switch (get_primary_state(baddie)) {
      case TRY_TO_CLOAK_STATE:
      case SNEAK_UP_BEHIND_STATE:
      case AMBUSH_STATE:
        begin_dogfight(baddie);
        break;
      case BEAM_SWEEP_STATE:
      case CHARGED_BEAM_STATE:
        resume_dogfight(baddie);
        break;
      case FLEE_WHILE_CLOAKED_STATE:
        begin_retreat(baddie);
        break;
      default: break;
    }
    AZ_ARRAY_LOOP(decoy, state->baddies) {
      if (decoy->kind != AZ_BAD_OTH_DECOY) continue;
      az_kill_baddie(state, decoy);
    }
    double theta =
      az_vtheta(az_vrot90ccw(az_vsub(baddie->position, state->ship.position)));
    if (az_randint(0, 1)) theta = -theta;
    baddie->velocity = az_vpolar(250, theta);
    az_baddie_t *decoy = az_add_baddie(state, AZ_BAD_OTH_DECOY,
                                       baddie->position, baddie->angle);
    if (decoy != NULL) {
      decoy->velocity = az_vneg(baddie->velocity);
      decoy->cooldown = 0.5;
    }
  } else if (damage_kind & AZ_DMGF_BEAM) {
    if (get_primary_state(baddie) == CHARGED_BEAM_STATE) {
      set_secondary_state(baddie, 1);
    }
  }
}

/*===========================================================================*/

void az_tick_bad_oth_decoy(
    az_space_state_t *state, az_baddie_t *baddie, double time) {
  assert(baddie->kind == AZ_BAD_OTH_DECOY);
  const double old_angle = baddie->angle;
  fly_towards_ship(state, baddie, time);
  if (baddie->cooldown <= 0.0 &&
      az_ship_within_angle(state, baddie, 0, AZ_DEG2RAD(3)) &&
      az_can_see_ship(state, baddie)) {
    for (int i = -1; i <= 1; ++i) {
      az_fire_baddie_projectile(state, baddie, AZ_PROJ_OTH_BULLET,
                                20.0, 0.0, AZ_DEG2RAD(10) * i);
    }
    az_play_sound(&state->soundboard, AZ_SND_FIRE_GUN_NORMAL);
    baddie->cooldown = 0.2;
  }
  az_tick_oth_tendrils(baddie, &AZ_OTH_SUPERGUNSHIP_TENDRILS, old_angle, time,
                       state->ship.player.total_time);
}

void az_on_oth_decoy_killed(
    az_space_state_t *state, az_vector_t position, double angle) {
  spawn_razors(state, position, angle + AZ_DEG2RAD(180), 120, 175);
}

/*===========================================================================*/
