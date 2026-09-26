/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "neonmask-presets.h"
#include <stdbool.h>
#include <stdint.h>

#define NM_CONFIG_SCHEMA_VERSION 1u

typedef struct nm_config {
    uint32_t schema_version;
    float scale;
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
