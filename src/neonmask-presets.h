/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <stdbool.h>
#include <stdint.h>

enum nm_shape {
    NM_SHAPE_ROUNDED = 0,
    NM_SHAPE_CIRCLE = 1,
    NM_SHAPE_ELLIPSE = 2,
    NM_SHAPE_HEXAGON = 3,
    NM_SHAPE_DIAMOND = 4,
    NM_SHAPE_RECTANGLE = 5,
    NM_SHAPE_TRIANGLE = 6,
    NM_SHAPE_POLYGON = 7
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
} nm_preset;

/* IDs 1..4; 0 is Custom, not an editable preset. */
bool nm_get_preset(int id, nm_preset *out);
