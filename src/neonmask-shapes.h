/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

/* CPU test oracle for the authored shader shapes (local, unrotated pixels).
 * GPU uses the same contour for alpha clipping, core, glow and secondary rails.
 * Bubble, Angled Card, HUD Cut Panel and Squircle are covered. */
float nm_authored_shape_distance(int shape_id, float x, float y,
                                 float half_width, float half_height,
                                 float radius, float detail);
