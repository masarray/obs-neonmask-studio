/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "neonmask-art.h"

enum nm_shape {
    NM_SHAPE_ROUNDED = 0,
    NM_SHAPE_CIRCLE = 1,
    NM_SHAPE_ELLIPSE = 2,
    NM_SHAPE_HEXAGON = 3,
    NM_SHAPE_DIAMOND = 4,
    NM_SHAPE_RECTANGLE = 5,
    NM_SHAPE_TRIANGLE = 6,
    NM_SHAPE_POLYGON = 7,
    NM_SHAPE_CHAT_BUBBLE = 8,
    NM_SHAPE_ANGLED_CARD = 9,
    NM_SHAPE_HUD_PANEL = 10,
    NM_SHAPE_SQUIRCLE = 11,
    NM_SHAPE_SVG_PATH = 12,
    NM_SHAPE_TECH_HUD = 13,
    NM_SHAPE_GAME_UI = 14
};

enum nm_animation {
    NM_ANIM_STATIC = 0,
    NM_ANIM_PULSE = 1,
    NM_ANIM_FLOW = 2
};

enum nm_border_style {
    NM_STYLE_CLASSIC = 0,
    NM_STYLE_DOUBLE = 1,
    NM_STYLE_HUD = 2,
    NM_STYLE_MINIMAL = 3
};

typedef struct nm_preset {
    int shape;
    int animation;
    uint32_t primary;
    uint32_t secondary;
    double scale;
    double roundness;
    double border_width;
    double feather;
    double glow_radius;
    double glow_strength;
    double speed;
    int segments;
    int style;
    bool border_enabled;
    bool glow_enabled;
    /* Explicit complete light recipes; existing IDs and preceding fields stay stable. */
    double mid_glow;
    double bloom_strength;
    double hotspot_strength;
    double hotspot_size;
    int ornament_mode;
    double art_intensity;
    double art_gap;
    double shape_detail;
} nm_preset;

/* Preset IDs stay stable. Preset 5 keeps the Tech HUD identity but now
 * deliberately selects the dedicated appended shape 13 when re-applied.
 * Existing scenes keep their saved shape until the user applies a preset. */
bool nm_get_preset(int id, nm_preset *out);
