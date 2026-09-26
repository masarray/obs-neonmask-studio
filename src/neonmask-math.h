/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>

/* The GPU uses pixel distances, not UV distances, so borders stay isotropic. */
typedef struct nm_geometry {
    float width;
    float height;
    float half_width;
    float half_height;
    float radius;
} nm_geometry;

float nm_clamp(float value, float min_value, float max_value);
nm_geometry nm_make_geometry(uint32_t width, uint32_t height, float scale,
                             float roundness);
float nm_sd_round_rect(float x, float y, float half_width,
                       float half_height, float radius);
float nm_sd_circle(float x, float y, float radius);
float nm_sd_ellipse_approx(float x, float y, float radius_x,
                           float radius_y);
uint32_t nm_obs_bgr(unsigned r, unsigned g, unsigned b);
