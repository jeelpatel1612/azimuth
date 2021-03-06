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
#ifndef AZIMUTH_STATE_BADDIE_H_
#define AZIMUTH_STATE_BADDIE_H_

#include <stdint.h>

#include "azimuth/state/pickup.h" // for az_pickup_flags_t
#include "azimuth/state/player.h" // for az_damage_kinds_t
#include "azimuth/state/script.h"
#include "azimuth/state/sound.h"
#include "azimuth/state/uid.h"
#include "azimuth/util/color.h"
#include "azimuth/util/polygon.h"
#include "azimuth/util/vector.h"

/*===========================================================================*/

// The maximum number of components that a baddie kind can have:
#define AZ_MAX_BADDIE_COMPONENTS 12
// The maximum number of objects a baddie can carry as cargo:
#define AZ_MAX_BADDIE_CARGO_UUIDS 4

// The number of different baddie kinds there are, not counting AZ_BAD_NOTHING:
#define AZ_NUM_BADDIE_KINDS 114

typedef enum {
  AZ_BAD_NOTHING = 0,
  AZ_BAD_MARKER,
  AZ_BAD_NORMAL_TURRET,
  AZ_BAD_ZIPPER,
  AZ_BAD_BOUNCER,
  AZ_BAD_ATOM,
  AZ_BAD_SPINER,
  AZ_BAD_BOX,
  AZ_BAD_ARMORED_BOX,
  AZ_BAD_CLAM,
  AZ_BAD_NIGHTBUG,
  AZ_BAD_SPINE_MINE,
  AZ_BAD_BROKEN_TURRET,
  AZ_BAD_ZENITH_CORE,
  AZ_BAD_ARMORED_TURRET,
  AZ_BAD_DRAGONFLY,
  AZ_BAD_CAVE_CRAWLER,
  AZ_BAD_CRAWLING_TURRET,
  AZ_BAD_HORNET,
  AZ_BAD_BEAM_SENSOR,
  AZ_BAD_ROCKWYRM,
  AZ_BAD_WYRM_EGG,
  AZ_BAD_WYRMLING,
  AZ_BAD_TRAPDOOR,
  AZ_BAD_CAVE_SWOOPER,
  AZ_BAD_ICE_CRAWLER,
  AZ_BAD_BEAM_TURRET,
  AZ_BAD_OTH_CRAB_1,
  AZ_BAD_OTH_ORB_1,
  AZ_BAD_OTH_SNAPDRAGON,
  AZ_BAD_OTH_RAZOR_1,
  AZ_BAD_GUN_SENSOR,
  AZ_BAD_SECURITY_DRONE,
  AZ_BAD_SMALL_TRUCK,
  AZ_BAD_HEAT_RAY,
  AZ_BAD_NUCLEAR_MINE,
  AZ_BAD_BEAM_WALL,
  AZ_BAD_SPARK,
  AZ_BAD_MOSQUITO,
  AZ_BAD_ARMORED_ZIPPER,
  AZ_BAD_FORCEFIEND,
  AZ_BAD_CHOMPER_PLANT,
  AZ_BAD_COPTER_HORZ,
  AZ_BAD_URCHIN,
  AZ_BAD_BOSS_DOOR,
  AZ_BAD_ROCKET_TURRET,
  AZ_BAD_MINI_ARMORED_ZIPPER,
  AZ_BAD_OTH_CRAB_2,
  AZ_BAD_SPINED_CRAWLER,
  AZ_BAD_DEATH_RAY,
  AZ_BAD_OTH_GUNSHIP,
  AZ_BAD_FIREBALL_MINE,
  AZ_BAD_LEAPER,
  AZ_BAD_BOUNCER_90,
  AZ_BAD_PISTON,
  AZ_BAD_ARMORED_PISTON,
  AZ_BAD_ARMORED_PISTON_EXT,
  AZ_BAD_INCORPOREAL_PISTON,
  AZ_BAD_INCORPOREAL_PISTON_EXT,
  AZ_BAD_COPTER_VERT,
  AZ_BAD_CRAWLING_MORTAR,
  AZ_BAD_OTH_ORB_2,
  AZ_BAD_FIRE_ZIPPER,
  AZ_BAD_SUPER_SPINER,
  AZ_BAD_HEAVY_TURRET,
  AZ_BAD_ECHO_SWOOPER,
  AZ_BAD_SUPER_HORNET,
  AZ_BAD_KILOFUGE,
  AZ_BAD_ICE_CRYSTAL,
  AZ_BAD_SWITCHER,
  AZ_BAD_FAST_BOUNCER,
  AZ_BAD_PROXY_MINE,
  AZ_BAD_NIGHTSHADE,
  AZ_BAD_AQUATIC_CHOMPER,
  AZ_BAD_SMALL_FISH,
  AZ_BAD_NOCTURNE,
  AZ_BAD_MYCOFLAKKER,
  AZ_BAD_MYCOSTALKER,
  AZ_BAD_OTH_CRAWLER,
  AZ_BAD_FIRE_CRAWLER,
  AZ_BAD_JUNGLE_CRAWLER,
  AZ_BAD_FORCE_EGG,
  AZ_BAD_FORCELING,
  AZ_BAD_JUNGLE_CHOMPER,
  AZ_BAD_SMALL_AUV,
  AZ_BAD_SENSOR_LASER,
  AZ_BAD_BEAM_SENSOR_INV,
  AZ_BAD_ERUPTION,
  AZ_BAD_PYROFLAKKER,
  AZ_BAD_PYROSTALKER,
  AZ_BAD_DEMON_SWOOPER,
  AZ_BAD_FIRE_CHOMPER,
  AZ_BAD_GRABBER_PLANT,
  AZ_BAD_POP_OPEN_TURRET,
  AZ_BAD_GNAT,
  AZ_BAD_CREEPY_EYE,
  AZ_BAD_BOMB_SENSOR,
  AZ_BAD_ROCKET_SENSOR,
  AZ_BAD_SPIKED_VINE,
  AZ_BAD_MAGBEEST_HEAD,
  AZ_BAD_MAGBEEST_LEGS_L,
  AZ_BAD_MAGBEEST_LEGS_R,
  AZ_BAD_MAGMA_BOMB,
  AZ_BAD_OTH_BRAWLER,
  AZ_BAD_LARGE_FISH,
  AZ_BAD_CRAB_CRAWLER,
  AZ_BAD_SCRAP_METAL,
  AZ_BAD_RED_ATOM,
  AZ_BAD_REFLECTION,
  AZ_BAD_OTH_MINICRAB,
  AZ_BAD_OTH_RAZOR_2,
  AZ_BAD_OTH_SUPERGUNSHIP,
  AZ_BAD_OTH_DECOY,
  AZ_BAD_CENTRAL_NETWORK_NODE,
  AZ_BAD_OTH_TENTACLE,
} az_baddie_kind_t;

typedef enum {
  AZ_DEATH_SHARDS = 0,
  AZ_DEATH_EMBERS,
  AZ_DEATH_OTH
} az_death_style_t;

typedef struct {
  az_vector_t init_position;
  double init_angle;
  double bounding_radius;
  az_polygon_t polygon;
  az_damage_flags_t immunities;
  double impact_damage;
} az_component_data_t;

// Bitset of flags dictating special baddie behavior:
typedef uint_fast16_t az_baddie_flags_t;
// BOUNCE_PERP: when bouncing, pretend normal is perpendicular to velocity
#define AZ_BADF_BOUNCE_PERP    ((az_baddie_flags_t)(1u << 0))
// CARRIES_CARGO: objects in cargo_uuids are moved along with the baddie
#define AZ_BADF_CARRIES_CARGO  ((az_baddie_flags_t)(1u << 1))
// DRAW_BG: the baddie is drawn behind walls rather than in front of them
#define AZ_BADF_DRAW_BG        ((az_baddie_flags_t)(1u << 2))
// INCORPOREAL: baddie cannot be hit by ship or by weapons
#define AZ_BADF_INCORPOREAL    ((az_baddie_flags_t)(1u << 3))
// INVINCIBLE: baddie cannot take damage
#define AZ_BADF_INVINCIBLE     ((az_baddie_flags_t)(1u << 4))
// KAMIKAZE: baddie dies when it hits the ship
#define AZ_BADF_KAMIKAZE       ((az_baddie_flags_t)(1u << 5))
// NO_HOMING_BEAM: homing beam ignores this baddie
#define AZ_BADF_NO_HOMING_BEAM ((az_baddie_flags_t)(1u << 6))
// NO_HOMING_PHASE: homing phase ignores this baddie
#define AZ_BADF_NO_HOMING_PHASE ((az_baddie_flags_t)(1u << 7))
// NO_HOMING_PROJ: homing projectiles ignore this baddie
#define AZ_BADF_NO_HOMING_PROJ ((az_baddie_flags_t)(1u << 8))
// NO_HOMING: all homing weapons ignore this baddie
#define AZ_BADF_NO_HOMING \
  (AZ_BADF_NO_HOMING_BEAM | AZ_BADF_NO_HOMING_PHASE | AZ_BADF_NO_HOMING_PROJ)
// QUAD_IMPACT: impact damage is quadrupled
#define AZ_BADF_QUAD_IMPACT    ((az_baddie_flags_t)(1u << 9))
// VULNERABLE: main body takes double damage (compared to components)
#define AZ_BADF_VULNERABLE     ((az_baddie_flags_t)(1u << 10))
// WALL_LIKE: baddie counts as a wall when calculating impacts
#define AZ_BADF_WALL_LIKE      ((az_baddie_flags_t)(1u << 11))
// WATER_BOUNCE: baddie bounces off of liquid surfaces
#define AZ_BADF_WATER_BOUNCE   ((az_baddie_flags_t)(1u << 12))

typedef struct {
  double overall_bounding_radius;
  double max_health;
  az_color_t color;
  az_sound_key_t hurt_sound, armor_sound, death_sound;
  az_death_style_t death_style;
  az_pickup_flags_t potential_pickups;
  az_baddie_flags_t static_properties;
  az_component_data_t main_body;
  int num_components;
  const az_component_data_t *components; // array of length num_components
} az_baddie_data_t;

// A "component" describes the positions of a baddie subpart.  The meanings of
// these components are specific to the baddie kind.
typedef struct {
  az_vector_t position;
  double angle;
} az_component_t;

typedef struct {
  az_baddie_kind_t kind; // if AZ_BAD_NOTHING, this baddie is not present
  const az_baddie_data_t *data;
  az_uid_t uid;
  const az_script_t *on_kill; // not owned; NULL if no script
  az_vector_t position;
  az_vector_t velocity;
  double angle;
  double health;
  double armor_flare; // from 0.0 (nothing) to 1.0 (was just now hit)
  double frozen; // from 0.0 (unfrozen) to 1.0 (was just now frozen)
  double cooldown; // time until baddie can attack again, in seconds
  double param; // the meaning of this is baddie-kind-specific
  double param2; // the meaning of this is baddie-kind-specific
  int state; // the meaning of this is baddie-kind-specific
  az_baddie_flags_t temp_properties;
  az_component_t components[AZ_MAX_BADDIE_COMPONENTS];
  az_uuid_t cargo_uuids[AZ_MAX_BADDIE_CARGO_UUIDS];
} az_baddie_t;

/*===========================================================================*/

// Call this at program startup to initialize all baddie data.  In particular,
// this must be called before any calls to az_get_baddie_data or
// az_init_baddie.
void az_init_baddie_datas(void);

// Get the static baddie data struct for a particular baddie kind.  The kind
// must not be AZ_BAD_NOTHING.
const az_baddie_data_t *az_get_baddie_data(az_baddie_kind_t kind);

// Set reasonable initial field values for a baddie of the given kind, at the
// given position.  All fields except baddie->uid will be reset.
void az_init_baddie(az_baddie_t *baddie, az_baddie_kind_t kind,
                    az_vector_t position, double angle);

/*===========================================================================*/

// True if the baddie has the given flag set (either temporarily for this
// particular baddie, or permanently for this baddie kind).
bool az_baddie_has_flag(const az_baddie_t *baddie, az_baddie_flags_t flag);

// Determine if the specified point overlaps the baddie.  If so, stores one of
// the overlapped components in *component_out (if component_out is non-NULL),
// and the absolute position of that component in *component_pos_out (if
// component_pos_out is non-NULL).
bool az_point_touches_baddie(const az_baddie_t *baddie, az_vector_t point,
                             const az_component_data_t **component_out,
                             az_vector_t *component_pos_out);

// Determine if the specified circle overlaps any part of the baddie.  If so,
// stores one of the overlapped components in *component_out (if component_out
// is non-NULL).
bool az_circle_touches_baddie(
    const az_baddie_t *baddie, double radius, az_vector_t center,
    const az_component_data_t **component_out);

// Determine if a ray, travelling delta from start, will hit the baddie.  If it
// does, stores the intersection point in *point_out (if point_out is non-NULL)
// and the normal vector in *normal_out (if normal_out is non-NULL).
bool az_ray_hits_baddie(
    const az_baddie_t *baddie, az_vector_t start, az_vector_t delta,
    az_vector_t *point_out, az_vector_t *normal_out,
    const az_component_data_t **component_out);

// Determine if a circle with the given radius, travelling delta from start,
// will hit the baddie.  If it does, the function stores in *pos_out the
// earliest position of the circle at which it touches the baddie (if pos_out
// is non-NULL) and the normal vector in *normal_out (if normal_out is
// non-NULL).
bool az_circle_hits_baddie(
    const az_baddie_t *baddie, double radius, az_vector_t start,
    az_vector_t delta, az_vector_t *pos_out, az_vector_t *normal_out,
    const az_component_data_t **component_out);

// Determine if a circle with the given radius, travelling in a circular path
// from start around spin_center by spin_angle radians, will hit the baddie.
// If it does, the function stores in *angle_out the angle travelled by the
// circle until impact (if angle_out is non-NULL), in *pos_out the first
// position of the circle at which it touches the baddie (if pos_out is
// non-NULL), and a vector normal to the baddie at the impact point in
// *normal_out (if normal_out is non-NULL).
bool az_arc_circle_hits_baddie(
    const az_baddie_t *baddie, double circle_radius,
    az_vector_t start, az_vector_t spin_center, double spin_angle,
    double *angle_out, az_vector_t *pos_out, az_vector_t *normal_out,
    const az_component_data_t **component_out);

/*===========================================================================*/

#endif // AZIMUTH_STATE_BADDIE_H_
