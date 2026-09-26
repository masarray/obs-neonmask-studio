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

float nm_clamp(float value, float min_value, float max_value);
nm_geometry nm_make_geometry(uint32_t width, uint32_t height, float scale,
                             float roundness);
float nm_sd_round_rect(float x, float y, float half_width,
                       float half_height, float radius);
float nm_sd_circle(float x, float y, float radius);
float nm_sd_ellipse_approx(float x, float y, float radius_x,
                           float radius_y);
/* OBS property colors use packed 0xAABBGGRR (bytes R,G,B,A on LE). */
uint32_t nm_obs_rgba(unsigned r, unsigned g, unsigned b);

/* Reference implementation of Phase-A source framing. Coordinates are source
 * pixels with origin at source center, +X right/+Y down. Positive pan moves
 * the visible subject right/down. Returns false when transformed sampling
 * lies outside the source; callers must use transparent RGBA in that case. */
bool nm_subject_sample_uv(float source_width, float source_height,
                          float output_x, float output_y,
                          float pan_x, float pan_y, float zoom,
                          float *u, float *v);
