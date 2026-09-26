/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "neonmask-presets.h"
#include <stdbool.h>
#include <stdint.h>

#define NM_CONFIG_SCHEMA_VERSION 2u

typedef struct nm_config {
    uint32_t schema_version;
    /* Legacy uniform size is retained for v0/v1 migration. */
    float scale;
    float mask_width;
    float mask_height;
    float mask_x_px;
    float mask_y_px;
    float subject_pan_x_px;
    float subject_pan_y_px;
    float subject_zoom;
    float shape_rotation_deg;
    int polygon_sides;
    float roundness;
    float border_px;
    float feather_px;
    float glow_px;
    float glow_amount;
    float animation_speed;
    uint32_t primary;
    uint32_t secondary;
    int shape_id;
    int animation_id;
    int segment_count;
    int style_id;
    bool show_border;
    bool show_glow;
} nm_config;

/* Pure-C canonical state. No OBS or graphics dependencies. */
void nm_config_defaults(nm_config *cfg);
void nm_config_validate(nm_config *cfg);
bool nm_config_apply_preset(nm_config *cfg, int preset_id);
bool nm_config_schema_supported(uint32_t schema_version);
