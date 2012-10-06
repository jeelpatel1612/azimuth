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

#include "test/test.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // for EXIT_FAILURE and EXIT_SUCCESS

/*===========================================================================*/

#define EPSILON 0.00000001

bool _current_test_failed = false;
unsigned int _num_tests_failed = 0u;

int final_test_summary(void) {
  if (_num_tests_failed == 0u) {
    printf("\x1b[32;1mAll tests passed.\x1b[m\n");
    return EXIT_SUCCESS;
  } else {
    printf("\x1b[31;1mSorry, %u test%s failed.\x1b[m\n",
           _num_tests_failed, (_num_tests_failed == 1u ? "" : "s"));
    return EXIT_FAILURE;
  }
}

void _run_test(const char *name, void (*function)(void)) {
  _current_test_failed = false;
  printf("Running %s...\n", name);
  function();
  if (_current_test_failed) {
    ++_num_tests_failed;
  } else {
    printf("    \x1b[32mOK\x1b[m\n");
  }
}

static bool dapprox(double a, double b) {
  double d = a - b;
  return (d < EPSILON && d > -EPSILON);
}

bool _expect_true(bool condition, const char *message) {
  if (condition) return true;
  _current_test_failed = true;
  printf("    \x1b[1;31mFAILED:\x1b[m %s\n", message);
  return false;
}

bool _expect_approx(double expected, double actual, const char *message) {
  if (dapprox(expected, actual)) return true;
  _current_test_failed = true;
  printf("    \x1b[1;31mFAILED:\x1b[m %s\n            %g vs. %g\n",
         message, expected, actual);
  return false;
}

/*===========================================================================*/