/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

/* The GPU uses pixel distances, not UV distances, so borders stay isotropic. */
typedef struct nm_geometry {
    float width;
    float height;
    float half_width;
    float half_height;
    float radius;
} nm_geometry;

typedef struct nm_point {
    float x;
    float y;
} nm_point;

float nm_clamp(float value, float min_value, float max_value);
nm_geometry nm_make_geometry(uint32_t width, uint32_t height, float scale,
                             float roundness);
nm_geometry nm_make_geometry_framed(uint32_t width, uint32_t height,
                                    float scale, float roundness,
                                    float width_factor, float height_factor);
nm_point nm_mask_point(float u, float v, uint32_t width, uint32_t height,
                       float mask_x, float mask_y);
nm_point nm_source_uv(float u, float v, uint32_t width, uint32_t height,
                      float subject_x, float subject_y, float subject_zoom);
bool nm_uv_inside(nm_point uv);
float nm_sd_regular_polygon(float x, float y, float rx, float ry,
                            int sides, float rotation_deg);
float nm_sd_round_rect(float x, float y, float half_width,
                       float half_height, float radius);
float nm_sd_circle(float x, float y, float radius);
float nm_sd_ellipse_approx(float x, float y, float radius_x,
                           float radius_y);
/* OBS property colors use packed 0xAABBGGRR (bytes R,G,B,A on LE). */
uint32_t nm_obs_rgba(unsigned r, unsigned g, unsigned b);
