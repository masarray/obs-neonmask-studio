/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <stdbool.h>
#include <stdint.h>

enum nm_shape {
    NM_SHAPE_ROUNDED = 0,
    NM_SHAPE_CIRCLE = 1,
    NM_SHAPE_ELLIPSE = 2,
    NM_SHAPE_HEXAGON = 3,
    NM_SHAPE_DIAMOND = 4
};

enum nm_animation {
    NM_ANIM_STATIC = 0,
    NM_ANIM_PULSE = 1,
    NM_ANIM_FLOW = 2
};

typedef struct nm_preset {
    int shape;
    int animation;
    uint32_t primary;
    uint32_t secondary;
    double scale;
    double roundness;
    double border_width;
    double glow_strength;
    int segments;
} nm_preset;

/* IDs 1..4; 0 is Custom, not an editable preset. */
bool nm_get_preset(int id, nm_preset *out);
