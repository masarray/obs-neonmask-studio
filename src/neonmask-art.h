/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <stdbool.h>

/* Art direction is separate from the silhouette: it cannot change clipping.
 * ID zero reproduces legacy custom scenes. Existing style/shape IDs are stable. */
enum nm_ornament {
    NM_ORNAMENT_NONE = 0,
    NM_ORNAMENT_CYBER = 1
};

/* Signed-distance contour coordinate in turns [0,1). Starts at the top-left
 * tangent and advances clockwise (positive screen Y is downward). A reference
 * for the shader; not a CPU render path. */
float nm_rounded_contour_turn(float x, float y, float half_width,
                              float half_height, float radius);

/* Art recipe envelope: separate outward track gap and ornament intensity. */
bool nm_art_recipe_valid(int mode, float track_gap, float intensity);
