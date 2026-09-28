/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

/* Parametric Bubble geometry; default zero-insets + legacy anchors preserve
 * saved shape ID 8. User values are sanitized by nm_config_validate. */
typedef struct nm_bubble_controls {
    float left_inset, right_inset, top_inset, bottom_inset;
    float tail_left, tail_right, tail_tip, tail_depth;
} nm_bubble_controls;

float nm_bubble_custom_distance(float x,float y,float half_width,float half_height,
                                float radius,const nm_bubble_controls *controls);

/* CPU test oracle for the authored shader shapes (local, unrotated pixels).
 * GPU uses the same contour for alpha clipping, core, glow and secondary rails.
 * Bubble, Angled Card, HUD Cut Panel and Squircle are covered. */
float nm_authored_shape_distance(int shape_id, float x, float y,
                                 float half_width, float half_height,
                                 float radius, float detail);
