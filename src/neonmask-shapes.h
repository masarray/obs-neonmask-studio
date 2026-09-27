/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

/* CPU test oracle for the new D1 shader shapes (local, unrotated pixel space).
 * The GPU uses the same contour for clipping, core, glow and secondary rails.
 * Other shape IDs are not exposed by this reference implementation. */
float nm_authored_shape_distance(int shape_id, float x, float y,
                                 float half_width, float half_height,
                                 float radius, float detail);
