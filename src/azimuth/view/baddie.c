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

#include "azimuth/view/baddie.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include <GL/gl.h>

#include "azimuth/state/baddie.h"
#include "azimuth/state/space.h"
#include "azimuth/util/clock.h"
#include "azimuth/util/misc.h"
#include "azimuth/util/vector.h"
#include "azimuth/view/baddie_bouncer.h"
#include "azimuth/view/baddie_chomper.h"
#include "azimuth/view/baddie_clam.h"
#include "azimuth/view/baddie_core.h"
#include "azimuth/view/baddie_crawler.h"
#include "azimuth/view/baddie_forcefiend.h"
#include "azimuth/view/baddie_kilofuge.h"
#include "azimuth/view/baddie_machine.h"
#include "azimuth/view/baddie_magbeest.h"
#include "azimuth/view/baddie_myco.h"
#include "azimuth/view/baddie_night.h"
#include "azimuth/view/baddie_oth.h"
#include "azimuth/view/baddie_spiner.h"
#include "azimuth/view/baddie_swooper.h"
#include "azimuth/view/baddie_turret.h"
#include "azimuth/view/baddie_vehicle.h"
#include "azimuth/view/baddie_wyrm.h"
#include "azimuth/view/baddie_zipper.h"
#include "azimuth/view/util.h"

/*===========================================================================*/

#if 0
static void draw_component_outline(const az_component_data_t *component) {
  const az_polygon_t poly = component->polygon;
  if (poly.num_vertices > 0) {
    glBegin(GL_LINE_LOOP); {
      for (int i = 0; i < poly.num_vertices; ++i) {
        az_gl_vertex(poly.vertices[i]);
      }
    } glEnd();
  } else {
    glBegin(GL_LINE_STRIP); {
      const double radius = component->bounding_radius;
      glVertex2f(0, 0);
      for (int i = 0; i <= 360; i += 10) {
        glVertex2d(radius * cos(AZ_DEG2RAD(i)), radius * sin(AZ_DEG2RAD(i)));
      }
    } glEnd();
  }
}

static void draw_baddie_outline(const az_baddie_t *baddie, float frozen,
                                float alpha) {
  const float flare = baddie->armor_flare;
  glColor4f(flare, 0.5f - 0.5f * flare + 0.5f * frozen, frozen, alpha);
  draw_component_outline(&baddie->data->main_body);
  for (int j = 0; j < baddie->data->num_components; ++j) {
    glPushMatrix(); {
      const az_component_t *component = &baddie->components[j];
      az_gl_translated(component->position);
      az_gl_rotated(component->angle);
      draw_component_outline(&baddie->data->components[j]);
    } glPopMatrix();
  }
}
#endif

static void draw_box(const az_baddie_t *baddie, bool armored, float flare) {
  glBegin(GL_QUADS); {
    if (armored) glColor3f(0.45, 0.45 - 0.3 * flare, 0.65 - 0.3 * flare);
    else glColor3f(0.65, 0.65 - 0.3 * flare, 0.65 - 0.3 * flare); // light gray
    glVertex2f(10, 7); glVertex2f(-10, 7);
    glVertex2f(-10, -7); glVertex2f(10, -7);

    glColor3f(0.2, 0.2, 0.2); // dark gray
    glVertex2f(11, 13); glVertex2f(-11, 13);
    if (armored) glColor3f(0.4, 0.4 - 0.3 * flare, 0.6 - 0.3 * flare);
    else glColor3f(0.6, 0.6 - 0.3 * flare, 0.6 - 0.3 * flare); // gray
    glVertex2f(-10, 7); glVertex2f(10, 7);

    glVertex2f(-10, -7); glVertex2f(-10, 7);
    glColor3f(0.2, 0.2, 0.2); // dark gray
    glVertex2f(-16, 8); glVertex2f(-16, -8);

    glVertex2f(16, -8); glVertex2f(16, 8);
    if (armored) glColor3f(0.4, 0.4 - 0.3 * flare, 0.6 - 0.3 * flare);
    else glColor3f(0.6, 0.6 - 0.3 * flare, 0.6 - 0.3 * flare); // gray
    glVertex2f(10, 7); glVertex2f(10, -7);

    glVertex2f(10, -7); glVertex2f(-10, -7);
    glColor3f(0.2, 0.2, 0.2); // dark gray
    glVertex2f(-11, -13); glVertex2f(11, -13);
  } glEnd();
  glBegin(GL_TRIANGLES); {
    glColor3f(0.3, 0.3 - 0.2 * flare, 0.3 - 0.2 * flare); // dark gray
    glVertex2f( 10,  7); glVertex2f( 11,  13); glVertex2f( 16,   8);
    glVertex2f(-10,  7); glVertex2f(-11,  13); glVertex2f(-16,   8);
    glVertex2f(-10, -7); glVertex2f(-16,  -8); glVertex2f(-11, -13);
    glVertex2f( 10, -7); glVertex2f( 11, -13); glVertex2f( 16,  -8);
  } glEnd();
  const double hurt =
    (baddie->data->max_health - baddie->health) / baddie->data->max_health;
  for (int i = 0; i < 2; ++i) {
    const double angle = AZ_DEG2RAD(180) * i;
    az_draw_cracks(az_vpolar(-16, angle), angle, 4.0 * hurt);
  }
}

static void draw_mine_arms(GLfloat length, float flare, float frozen) {
  glPushMatrix(); {
    for (int i = 0; i < 3; ++i) {
      glBegin(GL_QUADS); {
        glColor3f(0.55 + 0.4 * flare, 0.55, 0.5 + 0.5 * frozen);
        glVertex2f(0, 1.5); glVertex2f(length, 1.5);
        glColor3f(0.35 + 0.3 * flare, 0.35, 0.3 + 0.3 * frozen);
        glVertex2f(length, -1.5); glVertex2f(0, -1.5);
      } glEnd();
      glRotatef(120, 0, 0, 1);
    }
  } glPopMatrix();
}

static void draw_eruption_bubble(double max_radius, int frames,
                                 az_clock_t clock) {
  glBegin(GL_TRIANGLE_FAN); {
    const double mod = (double)az_clock_mod(frames, 1, clock) / (double)frames;
    glColor4f(1, 0.47, 0.3, 0.65 - 0.3 * mod);
    glVertex2f(0, 0);
    glColor4f(0.5, 0.235, 0.15, 0.85 - 0.3 * mod);
    const double radius = max_radius * mod;
    for (int i = -90; i < 90; i += 10) {
      glVertex2d(radius * cos(AZ_DEG2RAD(i)), radius * sin(AZ_DEG2RAD(i)));
    }
    for (int i = 90; i <= 270; i += 10) {
      glVertex2d(0.25 * radius * cos(AZ_DEG2RAD(i)),
                 radius * sin(AZ_DEG2RAD(i)));
    }
  } glEnd();
}

/*===========================================================================*/

static void draw_baddie_internal(const az_baddie_t *baddie, az_clock_t clock) {
  const float flare = baddie->armor_flare;
  const float frozen = (baddie->frozen <= 0.0 ? 0.0 :
                        baddie->frozen >= 0.2 ? 0.5 + 0.5 * baddie->frozen :
                        az_clock_mod(3, 2, clock) < 2 ? 0.6 : 0.0);
  if (baddie->frozen > 0.0) clock = 0;
  switch (baddie->kind) {
    case AZ_BAD_NOTHING: AZ_ASSERT_UNREACHABLE();
    case AZ_BAD_MARKER:
      glColor3f(1, 0, 1); // magenta
      glBegin(GL_LINE_STRIP); {
        glVertex2f(-20, 0); glVertex2f(20, 0); glVertex2f(0, 20);
        glVertex2f(0, -20); glVertex2f(20, 0);
      } glEnd();
      break;
    case AZ_BAD_NORMAL_TURRET:
      az_draw_bad_normal_turret(baddie, frozen, clock);
      break;
    case AZ_BAD_ZIPPER:
      az_draw_bad_zipper(baddie, frozen, clock);
      break;
    case AZ_BAD_BOUNCER:
      az_draw_bad_bouncer(baddie, frozen, clock);
      break;
    case AZ_BAD_ATOM:
      az_draw_bad_atom(baddie, frozen, clock);
      break;
    case AZ_BAD_SPINER:
      az_draw_bad_spiner(baddie, frozen, clock);
      break;
    case AZ_BAD_BOX:
      assert(frozen == 0.0);
      draw_box(baddie, false, flare);
      break;
    case AZ_BAD_ARMORED_BOX:
      assert(frozen == 0.0);
      draw_box(baddie, true, flare);
      break;
    case AZ_BAD_CLAM:
      az_draw_bad_clam(baddie, frozen, clock);
      break;
    case AZ_BAD_NIGHTBUG:
      az_draw_bad_nightbug(baddie, frozen, clock);
      break;
    case AZ_BAD_SPINE_MINE:
      az_draw_bad_spine_mine(baddie, frozen, clock);
      break;
    case AZ_BAD_BROKEN_TURRET:
      az_draw_bad_broken_turret(baddie, frozen, clock);
      break;
    case AZ_BAD_ZENITH_CORE:
      az_draw_bad_zenith_core(baddie, clock);
      break;
    case AZ_BAD_ARMORED_TURRET:
      az_draw_bad_armored_turret(baddie, frozen, clock);
      break;
    case AZ_BAD_DRAGONFLY:
      az_draw_bad_dragonfly(baddie, frozen, clock);
      break;
    case AZ_BAD_CAVE_CRAWLER:
      az_draw_bad_cave_crawler(baddie, frozen, clock);
      break;
    case AZ_BAD_CRAWLING_TURRET:
      az_draw_bad_crawling_turret(baddie, frozen, clock);
      break;
    case AZ_BAD_HORNET:
      az_draw_bad_hornet(baddie, frozen, clock);
      break;
    case AZ_BAD_BEAM_SENSOR:
      az_draw_bad_beam_sensor(baddie, frozen, clock);
      break;
    case AZ_BAD_BEAM_SENSOR_INV:
      az_draw_bad_beam_sensor_inv(baddie, frozen, clock);
      break;
    case AZ_BAD_ROCKWYRM:
      az_draw_bad_rockwyrm(baddie);
      break;
    case AZ_BAD_WYRM_EGG:
      az_draw_bad_wyrm_egg(baddie, frozen, clock);
      break;
    case AZ_BAD_WYRMLING:
      az_draw_bad_wyrmling(baddie, frozen);
      break;
    case AZ_BAD_TRAPDOOR:
      glBegin(GL_TRIANGLE_FAN); {
        glColor3f(0.8f - 0.8f * frozen, 0.5, 0.5f + 0.5f * frozen);
        glVertex2f(0, 0);
        glColor3f(0.4f - 0.4f * frozen, 0.2, 0.2f + 0.3f * frozen);
        const az_polygon_t polygon = baddie->data->main_body.polygon;
        for (int i = polygon.num_vertices - 1, j = 0;
             i < polygon.num_vertices; i = j++) {
          glVertex2d(polygon.vertices[i].x, polygon.vertices[i].y);
        }
      } glEnd();
      glPushMatrix(); {
        assert(!az_vnonzero(baddie->components[0].position));
        glRotated(AZ_RAD2DEG(baddie->components[0].angle), 0, 0, 1);
        glBegin(GL_POLYGON); {
          const az_component_data_t *data = &baddie->data->components[0];
          glColor3f(0.5f - 0.1f * frozen, 0.5, 0.5f + 0.1f * frozen);
          for (int i = 0; i < data->polygon.num_vertices; ++i) {
            if (i == 2) glColor3f(0.4f - 0.1f * frozen, 0.3,
                                  0.3f + 0.1f * frozen);
            glVertex2d(data->polygon.vertices[i].x,
                       data->polygon.vertices[i].y);
          }
        } glEnd();
      } glPopMatrix();
      break;
    case AZ_BAD_CAVE_SWOOPER:
      az_draw_bad_cave_swooper(baddie, frozen, clock);
      break;
    case AZ_BAD_ICE_CRAWLER:
      az_draw_bad_ice_crawler(baddie, frozen, clock);
      break;
    case AZ_BAD_BEAM_TURRET:
      az_draw_bad_beam_turret(baddie, frozen, clock);
      break;
    case AZ_BAD_OTH_CRAB_1:
    case AZ_BAD_OTH_CRAB_2:
      az_draw_bad_oth_crab(baddie, frozen, clock);
      break;
    case AZ_BAD_OTH_ORB_1:
    case AZ_BAD_OTH_ORB_2:
      az_draw_bad_oth_orb(baddie, frozen, clock);
      break;
    case AZ_BAD_OTH_SNAPDRAGON:
      az_draw_bad_oth_snapdragon(baddie, frozen, clock);
      break;
    case AZ_BAD_OTH_RAZOR_1:
      az_draw_bad_oth_razor_1(baddie, frozen, clock);
      break;
    case AZ_BAD_GUN_SENSOR:
      az_draw_bad_gun_sensor(baddie, frozen, clock);
      break;
    case AZ_BAD_SECURITY_DRONE:
      az_draw_bad_security_drone(baddie, frozen, clock);
      break;
    case AZ_BAD_SMALL_TRUCK:
      az_draw_bad_small_truck(baddie, frozen, clock);
      break;
    case AZ_BAD_HEAT_RAY:
      az_draw_bad_heat_ray(baddie, frozen, clock);
      break;
    case AZ_BAD_NUCLEAR_MINE:
      draw_mine_arms(18, flare, frozen);
      // Body:
      glBegin(GL_POLYGON); {
        glColor3f(0.65 + 0.3 * flare - 0.3 * frozen, 0.65 - 0.3 * flare,
                  0.5 - 0.3 * flare + 0.5 * frozen);
        for (int i = 0; i <= 360; i += 60) {
          glVertex2d(8 * cos(AZ_DEG2RAD(i)), 8 * sin(AZ_DEG2RAD(i)));
        }
      } glEnd();
      glBegin(GL_QUAD_STRIP); {
        for (int i = 0; i <= 360; i += 60) {
          glColor3f(0.65 + 0.3 * flare - 0.3 * frozen, 0.65 - 0.3 * flare,
                    0.5 - 0.3 * flare + 0.5 * frozen);
          glVertex2d(8 * cos(AZ_DEG2RAD(i)), 8 * sin(AZ_DEG2RAD(i)));
          glColor3f(0.35f + 0.3f * flare, 0.35f, 0.15f + 0.3f * frozen);
          glVertex2d(12 * cos(AZ_DEG2RAD(i)), 12 * sin(AZ_DEG2RAD(i)));
        }
      } glEnd();
      // Radiation symbol:
      if (baddie->state == 1 && az_clock_mod(2, 3, clock)) glColor3f(1, 0, 0);
      else glColor3f(0, 0, 0.5f * frozen);
      glBegin(GL_TRIANGLE_FAN); {
        glVertex2d(0, 0);
        for (int i = 0; i <= 360; i += 30) {
          glVertex2d(1.5 * cos(AZ_DEG2RAD(i)), 1.5 * sin(AZ_DEG2RAD(i)));
        }
      } glEnd();
      for (int j = 60; j < 420; j += 120) {
        glBegin(GL_QUAD_STRIP); {
          for (int i = j - 30; i <= j + 30; i += 10) {
            glVertex2d(3 * cos(AZ_DEG2RAD(i)), 3 * sin(AZ_DEG2RAD(i)));
            glVertex2d(8 * cos(AZ_DEG2RAD(i)), 8 * sin(AZ_DEG2RAD(i)));
          }
        } glEnd();
      }
      break;
    case AZ_BAD_BEAM_WALL:
      glBegin(GL_QUADS); {
        // Interior:
        glColor3f(0.3, 0.3, 0.3);
        glVertex2f(50, 15); glVertex2f(-50, 15);
        glVertex2f(-50, -15); glVertex2f(50, -15);
        // Diagonal struts:
        for (int i = 0; i < 3; ++i) {
          const float x = -50 + 32 * i;
          for (int j = 0; j < 2; ++j) {
            const float y = (j ? -12 : 12);
            glColor3f(0.75 + 0.25 * flare, 0.75, 0.75);
            glVertex2f(x, y); glVertex2f(x + 32, -y);
            glColor3f(0.35 + 0.35 * flare, 0.4, 0.35);
            glVertex2f(x + 32 + 4, -y); glVertex2f(x + 4, y);
          }

        }
        // Top and bottom struts:
        for (int y = -10; y <= 15; y += 25) {
          glColor3f(0.75 + 0.25 * flare, 0.75, 0.75);
          glVertex2f(52, y); glVertex2f(-52, y);
          glColor3f(0.35 + 0.35 * flare, 0.4, 0.35);
          glVertex2f(-52, y - 5); glVertex2f(52, y - 5);
        }
      } glEnd();
      break;
    case AZ_BAD_SPARK:
      glBegin(GL_TRIANGLE_FAN); {
        glColor4f(1, 1, 1, 0.8);
        glVertex2f(0, 0);
        glColor4f(0, 1, 0, 0);
        for (int i = 0; i <= 360; i += 45) {
          const double radius =
            (i % 2 ? 1.0 : 0.5) * (8.0 + 0.25 * az_clock_zigzag(8, 3, clock));
          const double theta = AZ_DEG2RAD(i + 7 * az_clock_mod(360, 1, clock));
          glVertex2d(radius * cos(theta), radius * sin(theta));
        }
      } glEnd();
      break;
    case AZ_BAD_MOSQUITO:
      az_draw_bad_mosquito(baddie, frozen, clock);
      break;
    case AZ_BAD_ARMORED_ZIPPER:
      az_draw_bad_armored_zipper(baddie, frozen, clock);
      break;
    case AZ_BAD_FORCEFIEND:
      az_draw_bad_forcefiend(baddie);
      break;
    case AZ_BAD_CHOMPER_PLANT:
      az_draw_bad_chomper_plant(baddie, frozen, clock);
      break;
    case AZ_BAD_COPTER_HORZ:
    case AZ_BAD_COPTER_VERT:
      az_draw_bad_copter(baddie, frozen, clock);
      break;
    case AZ_BAD_URCHIN:
      az_draw_bad_urchin(baddie, frozen, clock);
      break;
    case AZ_BAD_BOSS_DOOR:
      az_draw_bad_boss_door(baddie, frozen, clock);
      break;
    case AZ_BAD_ROCKET_TURRET:
      az_draw_bad_rocket_turret(baddie, frozen, clock);
      break;
    case AZ_BAD_MINI_ARMORED_ZIPPER:
      az_draw_bad_mini_armored_zipper(baddie, frozen, clock);
      break;
    case AZ_BAD_SPINED_CRAWLER:
      az_draw_bad_spined_crawler(baddie, frozen, clock);
      break;
    case AZ_BAD_DEATH_RAY:
      az_draw_bad_death_ray(baddie, frozen, clock);
      break;
    case AZ_BAD_OTH_GUNSHIP:
      az_draw_bad_oth_gunship(baddie, frozen, clock);
      break;
    case AZ_BAD_FIREBALL_MINE:
      glPushMatrix(); {
        glScalef(1, 1.07, 1);
        const GLfloat blink =
          fmax(flare, (baddie->state == 1 &&
                       az_clock_mod(2, 4, clock) == 0 ? 0.5 : 0.0));
        const double radius = baddie->data->main_body.bounding_radius;
        glBegin(GL_TRIANGLE_FAN); {
          glColor3f(0.6f + 0.4f * blink, 0.6f, 0.6f);
          glVertex2d(-0.15 * radius, 0.2 * radius);
          glColor3f(0.2f + 0.3f * blink, 0.2f, 0.2f);
          for (int i = 0; i <= 360; i += 15) {
            glVertex2d(radius * cos(AZ_DEG2RAD(i)),
                       radius * sin(AZ_DEG2RAD(i)));
          }
        } glEnd();
        const double hurt = (baddie->data->max_health - baddie->health) /
          baddie->data->max_health;
        for (int i = 0; i < 5; ++i) {
          const double angle = AZ_DEG2RAD(20) + i * AZ_DEG2RAD(72);
          az_draw_cracks(az_vpolar(-radius, angle), angle,
                         5 * hurt * (1.5 - 0.33 * ((3 * i) % 5)));
        }
        for (int i = 0; i < 10; ++i) {
          glBegin(GL_TRIANGLE_FAN); {
            glColor3f(0.4f + 0.3f * blink, 0.4f, 0.4f);
            glVertex2d(radius - 4, 0);
            glColor3f(0.2f + 0.3f * blink, 0.2f, 0.2f);
            glVertex2d(radius - 1, 2); glVertex2d(radius + 6, 0);
            glVertex2d(radius - 1, -2);
          } glEnd();
          glRotatef(36, 0, 0, 1);
        }
        glRotatef(18, 0, 0, 1);
        for (int i = 0; i < 5; ++i) {
          glBegin(GL_TRIANGLE_FAN); {
            glColor3f(0.6f + 0.4f * blink, 0.6f, 0.6f);
            glVertex2d(radius - 9, 0);
            glColor3f(0.4f + 0.3f * blink, 0.4f, 0.4f);
            glVertex2d(radius - 8, 2); glVertex2d(radius - 3, 0);
            glVertex2d(radius - 8, -2);
          } glEnd();
          glRotatef(72, 0, 0, 1);
        }
      } glPopMatrix();
      break;
    case AZ_BAD_LEAPER: {
      const double tilt_degrees =
        (baddie->state != 0 ? 0.0 :
         (1.0 - fmin(baddie->cooldown / 0.5, 1.0)) * 10.0 +
         az_clock_zigzag(3, 8, clock));
      // Legs:
      for (int flip = 0; flip < 2; ++flip) {
        glPushMatrix(); {
          if (flip) glScalef(1, -1, 1);
          // Upper leg:
          glPushMatrix(); {
            glTranslated(-0.5 * tilt_degrees, 0, 0);
            if (baddie->state == 0) glRotatef(48 - tilt_degrees, 0, 0, 1);
            else glRotatef(70, 0, 0, 1);
            glBegin(GL_QUAD_STRIP); {
              glColor3f(0, 0.2, 0.1); glVertex2d(0, 5); glVertex2d(21, 2);
              glColor3f(0, 0.3, 0.2); glVertex2d(0, 0); glVertex2d(23, 0);
              glColor3f(0, 0.2, 0.1); glVertex2d(0, -4); glVertex2d(21, -3);
            } glEnd();
          } glPopMatrix();
          // Lower leg:
          if (baddie->state == 0) {
            glTranslated(-20, 20 + az_clock_zigzag(3, 8, clock), 0);
            glRotated(-tilt_degrees, 0, 0, 1);
          } else {
            glTranslated(-30, 18, 0);
            glRotated(5, 0, 0, 1);
          }
          glBegin(GL_QUAD_STRIP); {
            glColor3f(0, 0.25, 0.1); glVertex2d(2, 5);
            glColor3f(0.25, 0.15, 0); glVertex2d(35, 4);
            glColor3f(0.5f * flare, 0.6, 0.4f + 0.6f * frozen);
            glVertex2d(0, 0);
            glColor3f(0.3f + 0.3f * flare, 0.2, frozen);
            glVertex2d(35, 0);
            glColor3f(0, 0.25, 0.1); glVertex2d(2, -6);
            glColor3f(0.25, 0.15, 0); glVertex2d(35, -4);
          } glEnd();
          glBegin(GL_QUAD_STRIP); {
            glColor3f(0, 0.25, 0.1); glVertex2d(18, 6); glVertex2d(35, 4);
            glColor3f(0.5f * flare, 0.6f + 0.4f * flare, 0.4f + 0.6f * frozen);
            glVertex2d(16, 0); glVertex2d(35, 0);
            glColor3f(0, 0.25, 0.1); glVertex2d(18, -6); glVertex2d(35, -4);
          } glEnd();
          // Foot:
          glBegin(GL_TRIANGLE_FAN); {
            glColor3f(0.5, 0.5, 0.5); glVertex2d(0, -1);
            glColor3f(0.2, 0.3, 0.3);
            for (int i = -105; i <= 105; i += 30) {
              glVertex2d(5 * cos(AZ_DEG2RAD(i)), 7 * sin(AZ_DEG2RAD(i)) - 1);
            }
          } glEnd();
          // Knee knob:
          glTranslatef(35, 0, 0);
          glBegin(GL_TRIANGLE_FAN); {
            glColor3f(0.5f * flare, 0.6, 0.4 + 0.6f * frozen);
            glVertex2d(0, 0);
            glColor3f(0, 0.25, 0.1);
            for (int i = -135; i <= 135; i += 30) {
              glVertex2d(6 * cos(AZ_DEG2RAD(i)), 5 * sin(AZ_DEG2RAD(i)));
            }
          } glEnd();
          // Knee spike:
          glBegin(GL_TRIANGLE_FAN); {
            glColor3f(0.5, 0.5, 0.5); glVertex2f(4, 0);
            glColor3f(0.25, 0.25, 0.25);
            glVertex2f(5, 2); glVertex2f(10, 0); glVertex2f(5, -2);
          } glEnd();
        } glPopMatrix();
      }
      glPushMatrix(); {
        glTranslated(-0.5 * tilt_degrees, 0, 0);
        // Teeth:
        const int x = (baddie->state == 0 ? 0 : 3);
        for (int y = -2; y <= 2; y += 4) {
          glBegin(GL_TRIANGLE_FAN); {
            glColor3f(0.5, 0.5, 0.5); glVertex2i(8 + x, y);
            glColor3f(0.25, 0.25, 0.25);
              glVertex2i(9 + x, 2 + y); glVertex2i(15 + x, y);
              glVertex2i(9 + x, -2 + y);
          } glEnd();
        }
        // Body:
        glBegin(GL_TRIANGLE_FAN); {
          glColor3f(flare, 0.9, 0.5f + 0.5f * frozen); glVertex2d(-3, 0);
          glColor3f(0.5f * flare, 0.25, 0.1f + 0.9f * frozen);
          glVertex2d(-10, 0);
          for (int i = -135; i <= 135; i += 30) {
            glVertex2d(12 * cos(AZ_DEG2RAD(i)), 9 * sin(AZ_DEG2RAD(i)));
          }
          glVertex2d(-10, 0);
        } glEnd();
        // Eye:
        glBegin(GL_POLYGON); {
          glColor4f(1, 0, 0, 0.4);
          glVertex2f(10, 1); glVertex2f(9, 2); glVertex2f(7, 0);
          glVertex2f(9, -2), glVertex2f(10, -1);
        } glEnd();
      } glPopMatrix();
    } break;
    case AZ_BAD_BOUNCER_90:
      az_draw_bad_bouncer_90(baddie, frozen, clock);
      break;
    case AZ_BAD_PISTON:
      az_draw_bad_piston(baddie, frozen, clock);
      break;
    case AZ_BAD_ARMORED_PISTON:
    case AZ_BAD_ARMORED_PISTON_EXT:
      az_draw_bad_armored_piston(baddie, frozen, clock);
      break;
    case AZ_BAD_INCORPOREAL_PISTON:
    case AZ_BAD_INCORPOREAL_PISTON_EXT:
      az_draw_bad_incorporeal_piston(baddie, frozen, clock);
      break;
    case AZ_BAD_CRAWLING_MORTAR:
      az_draw_bad_crawling_mortar(baddie, frozen, clock);
      break;
    case AZ_BAD_FIRE_ZIPPER:
      az_draw_bad_fire_zipper(baddie, frozen, clock);
      break;
    case AZ_BAD_SUPER_SPINER:
      az_draw_bad_super_spiner(baddie, frozen, clock);
      break;
    case AZ_BAD_HEAVY_TURRET:
      az_draw_bad_heavy_turret(baddie, frozen, clock);
      break;
    case AZ_BAD_ECHO_SWOOPER:
      az_draw_bad_echo_swooper(baddie, frozen, clock);
      break;
    case AZ_BAD_SUPER_HORNET:
      az_draw_bad_super_hornet(baddie, frozen, clock);
      break;
    case AZ_BAD_KILOFUGE:
      az_draw_bad_kilofuge(baddie, clock);
      break;
    case AZ_BAD_ICE_CRYSTAL:
      az_draw_bad_ice_crystal(baddie);
      break;
    case AZ_BAD_SWITCHER:
      az_draw_bad_switcher(baddie, frozen, clock);
      break;
    case AZ_BAD_FAST_BOUNCER:
      az_draw_bad_fast_bouncer(baddie, frozen, clock);
      break;
    case AZ_BAD_PROXY_MINE:
      draw_mine_arms(15, flare, frozen);
      // Body:
      glBegin(GL_TRIANGLE_FAN); {
        glColor3f(0.65 + 0.35 * flare - 0.3 * frozen, 0.65 - 0.3 * flare,
                  0.65 - 0.3 * flare + 0.35 * frozen);
        glVertex2f(0, 0);
        glColor3f(0.35 + 0.3 * flare - 0.15 * frozen, 0.35 - 0.15 * flare,
                  0.35 - 0.15 * flare + 0.3 * frozen);
        for (int i = 0; i <= 360; i += 15) {
          glVertex2d(7 * cos(AZ_DEG2RAD(i)), 7 * sin(AZ_DEG2RAD(i)));
        }
      } glEnd();
      // Light bulb:
      glBegin(GL_TRIANGLE_FAN); {
        if (baddie->state == 1 && az_clock_mod(2, 3, clock)) {
          glColor3f(1, 0.6, 0.5);
        } else glColor3f(0.2, 0.2, 0.2);
        glVertex2f(0, 0);
        glColor3f(0, 0, 0);
        for (int i = 0; i <= 360; i += 20) {
          glVertex2d(3 * cos(AZ_DEG2RAD(i)), 3 * sin(AZ_DEG2RAD(i)));
        }
      } glEnd();
      break;
    case AZ_BAD_NIGHTSHADE:
      az_draw_bad_nightshade(baddie, frozen, clock);
      break;
    case AZ_BAD_AQUATIC_CHOMPER:
      az_draw_bad_aquatic_chomper(baddie, frozen, clock);
      break;
    case AZ_BAD_SMALL_FISH:
      az_draw_bad_small_fish(baddie, frozen, clock);
      break;
    case AZ_BAD_NOCTURNE:
      az_draw_bad_nocturne(baddie, clock);
      break;
    case AZ_BAD_MYCOFLAKKER:
      az_draw_bad_mycoflakker(baddie, frozen, clock);
      break;
    case AZ_BAD_MYCOSTALKER:
      az_draw_bad_mycostalker(baddie, frozen, clock);
      break;
    case AZ_BAD_OTH_CRAWLER:
      az_draw_bad_oth_crawler(baddie, frozen, clock);
      break;
    case AZ_BAD_FIRE_CRAWLER:
      az_draw_bad_fire_crawler(baddie, frozen, clock);
      break;
    case AZ_BAD_JUNGLE_CRAWLER:
      az_draw_bad_jungle_crawler(baddie, frozen, clock);
      break;
    case AZ_BAD_FORCE_EGG:
      az_draw_bad_force_egg(baddie);
      break;
    case AZ_BAD_FORCELING:
      az_draw_bad_forceling(baddie, frozen, clock);
      break;
    case AZ_BAD_JUNGLE_CHOMPER:
      az_draw_bad_jungle_chomper(baddie, frozen, clock);
      break;
    case AZ_BAD_SMALL_AUV:
      az_draw_bad_small_auv(baddie, frozen, clock);
      break;
    case AZ_BAD_SENSOR_LASER:
      az_draw_bad_sensor_laser(baddie, frozen, clock);
      break;
    case AZ_BAD_ERUPTION:
      draw_eruption_bubble(10.0, 47, clock);
      glPushMatrix(); {
        glTranslatef(0, -9, 0);
        draw_eruption_bubble(5.0, 23, clock);
      } glPopMatrix();
      glPushMatrix(); {
        glTranslatef(0, 8, 0);
        draw_eruption_bubble(6.0, 27, clock);
      } glPopMatrix();
      break;
    case AZ_BAD_PYROFLAKKER:
      az_draw_bad_pyroflakker(baddie, frozen, clock);
      break;
    case AZ_BAD_PYROSTALKER:
      az_draw_bad_pyrostalker(baddie, frozen, clock);
      break;
    case AZ_BAD_DEMON_SWOOPER:
      az_draw_bad_demon_swooper(baddie, frozen, clock);
      break;
    case AZ_BAD_FIRE_CHOMPER:
      az_draw_bad_fire_chomper(baddie, frozen, clock);
      break;
    case AZ_BAD_GRABBER_PLANT:
      az_draw_bad_grabber_plant(baddie, frozen, clock);
      break;
    case AZ_BAD_POP_OPEN_TURRET:
      az_draw_bad_pop_open_turret(baddie, frozen, clock);
      break;
    case AZ_BAD_GNAT:
      az_draw_bad_gnat(baddie, frozen, clock);
      break;
    case AZ_BAD_CREEPY_EYE:
      az_draw_bad_creepy_eye(baddie, frozen, clock);
      break;
    case AZ_BAD_BOMB_SENSOR:
      az_draw_bad_bomb_sensor(baddie, frozen, clock);
      break;
    case AZ_BAD_ROCKET_SENSOR:
      az_draw_bad_rocket_sensor(baddie, frozen, clock);
      break;
    case AZ_BAD_SPIKED_VINE:
      az_draw_bad_spiked_vine(baddie, frozen, clock);
      break;
    case AZ_BAD_MAGBEEST_HEAD:
      az_draw_bad_magbeest_head(baddie, clock);
      break;
    case AZ_BAD_MAGBEEST_LEGS_L:
      az_draw_bad_magbeest_legs_l(baddie, clock);
      break;
    case AZ_BAD_MAGBEEST_LEGS_R:
      az_draw_bad_magbeest_legs_r(baddie, clock);
      break;
    case AZ_BAD_MAGMA_BOMB:
      az_draw_bad_magma_bomb(baddie, clock);
      break;
    case AZ_BAD_OTH_BRAWLER:
      az_draw_bad_oth_brawler(baddie, frozen, clock);
      break;
    case AZ_BAD_LARGE_FISH:
      az_draw_bad_large_fish(baddie, frozen, clock);
      break;
    case AZ_BAD_CRAB_CRAWLER:
      az_draw_bad_crab_crawler(baddie, frozen, clock);
      break;
    case AZ_BAD_SCRAP_METAL:
      az_draw_bad_scrap_metal(baddie);
      break;
    case AZ_BAD_RED_ATOM:
      az_draw_bad_red_atom(baddie, frozen, clock);
      break;
    case AZ_BAD_REFLECTION:
      az_draw_bad_reflection(baddie, clock);
      break;
    case AZ_BAD_OTH_MINICRAB:
      az_draw_bad_oth_minicrab(baddie, frozen, clock);
      break;
    case AZ_BAD_OTH_RAZOR_2:
      az_draw_bad_oth_razor_2(baddie, frozen, clock);
      break;
    case AZ_BAD_OTH_SUPERGUNSHIP:
    case AZ_BAD_OTH_DECOY:
      az_draw_bad_oth_supergunship(baddie, frozen, clock);
      break;
    case AZ_BAD_CENTRAL_NETWORK_NODE:
      az_draw_bad_central_network_node(baddie, clock);
      break;
    case AZ_BAD_OTH_TENTACLE:
      az_draw_bad_oth_tentacle(baddie, frozen, clock);
      break;
  }
}

void az_draw_baddie(const az_baddie_t *baddie, az_clock_t clock) {
  assert(baddie->kind != AZ_BAD_NOTHING);
  glPushMatrix(); {
    az_gl_translated(baddie->position);
    az_gl_rotated(baddie->angle);
    draw_baddie_internal(baddie, clock);
  } glPopMatrix();
}

void az_draw_background_baddies(const az_space_state_t *state) {
  AZ_ARRAY_LOOP(baddie, state->baddies) {
    if (baddie->kind == AZ_BAD_NOTHING ||
        baddie->kind == AZ_BAD_MARKER) continue;
    if (!az_baddie_has_flag(baddie, AZ_BADF_DRAW_BG)) continue;
    az_draw_baddie(baddie, state->clock);
  }
}

void az_draw_foreground_baddies(const az_space_state_t *state) {
  AZ_ARRAY_LOOP(baddie, state->baddies) {
    if (baddie->kind == AZ_BAD_NOTHING ||
        baddie->kind == AZ_BAD_MARKER) continue;
    if (az_baddie_has_flag(baddie, AZ_BADF_DRAW_BG)) continue;
    az_draw_baddie(baddie, state->clock);
  }
}

/*===========================================================================*/
