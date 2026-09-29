/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "neonmask-presets.h"
#include "neonmask-svg.h"
#include <stdbool.h>
#include <stdint.h>

#define NM_CONFIG_SCHEMA_VERSION 3u

typedef struct nm_config {
    uint32_t schema_version;
    /* Legacy uniform size is retained for v0/v1 migration. */
    float scale;
    float mask_width;
    float mask_height;
    /* D3a opt-in: reduce mask extents, never subject UV, to fit light in canvas. */
    bool safe_fit;
    /* D3b opt-in: report an expanded filter output with explicit origin. */
    bool expand_canvas;
    float mask_x_px;
    float mask_y_px;
    float subject_pan_x_px;
    float subject_pan_y_px;
    float subject_zoom;
    float shape_rotation_deg;
    int polygon_sides;
    float roundness;
    /* D1: proportional tail height / diagonal cut, range 0.08..0.35. */
    float shape_detail;
    /* D4A v1 fields are retained only for schema-2 migration. */
    float bubble_left_inset, bubble_right_inset;
    float bubble_top_inset, bubble_bottom_inset;
    float bubble_tail_left, bubble_tail_right, bubble_tail_tip;
    /* D4A v2: convex freeform body control points normalized by half-size.
     * Each corner stays in its own quadrant so arbitrary slider combinations
     * cannot self-intersect. Tail positions are fractions of BL->BR. */
    float bubble_tl_x, bubble_tl_y, bubble_tr_x, bubble_tr_y;
    float bubble_br_x, bubble_br_y, bubble_bl_x, bubble_bl_y;
    float bubble_tail_start, bubble_tail_end, bubble_tail_tip_pos;
    float bubble_tail_depth;
    float border_px;
    float feather_px;
    float glow_px;
    float glow_amount;
    float mid_glow_strength;
    float bloom_strength;
    float hotspot_strength;
    float hotspot_size;
    float art_intensity;
    float art_gap;
    int ornament_mode;
    float animation_speed;
    uint32_t primary;
    uint32_t secondary;
    /* D4D: color rendering is independent of border motion. Old scenes
     * resolve to Dual, preserving the pre-D4D two-color shader exactly. */
    int color_mode;
    float rainbow_speed;
    float rainbow_saturation;
    float rainbow_hue_offset;
    float rainbow_spread;
    int shape_id;
    int animation_id;
    int segment_count;
    int style_id;
    bool show_border;
    bool show_glow;
    /* Additive v2 field: one local file, never a URL/resource loader. */
    char svg_path[NM_SVG_PATH_MAX];
} nm_config;

/* Pure-C canonical state. No OBS or graphics dependencies. */
void nm_config_defaults(nm_config *cfg);
void nm_config_validate(nm_config *cfg);
bool nm_config_apply_preset(nm_config *cfg, int preset_id);
bool nm_config_schema_supported(uint32_t schema_version);
