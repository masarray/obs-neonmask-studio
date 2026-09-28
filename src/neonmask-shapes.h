/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

/* D4A freeform Bubble geometry. Coordinates are normalized by half-size;
 * tail start/end/tip_pos are fractions along BL->BR. */
typedef struct nm_bubble_controls {
    float tl_x,tl_y,tr_x,tr_y;
    float br_x,br_y,bl_x,bl_y;
    float tail_start,tail_end,tail_tip_pos,tail_depth;
} nm_bubble_controls;

float nm_bubble_custom_distance(float x,float y,float half_width,float half_height,
                                float radius,const nm_bubble_controls *controls);

/* CPU test oracle for the authored shader shapes (local, unrotated pixels).
 * GPU uses the same contour for alpha clipping, core, glow and secondary rails.
 * Bubble, Angled Card, HUD Cut Panel, Tech HUD and Squircle are covered. */
float nm_authored_shape_distance(int shape_id, float x, float y,
                                 float half_width, float half_height,
                                 float radius, float detail);
