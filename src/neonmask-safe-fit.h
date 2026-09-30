/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include "neonmask-config.h"

/* D3a operates within the EXISTING libobs target; it does not pretend to
 * expand the source/filter dimensions. This is an opt-in conservative fit. */
typedef struct nm_fit_result {
    float half_width, half_height;
    float envelope_px;
    float scale;
    bool fits;
    /* Source pixels begin at (pad_left,pad_top) in the expanded output. */
    uint32_t pad_left, pad_right, pad_top, pad_bottom;
    uint32_t output_width, output_height;
} nm_fit_result;

/* Returns false for invalid inputs; fits=false for impossible placement.
 * Expand mode takes precedence. Dedicated Tech HUD / Game UI outer ornaments
 * auto-fit inside the source when expansion is off; other/legacy shapes keep
 * the explicit safe_fit opt-in behavior. It never changes cfg, mask center,
 * subject pan/zoom or input UV. */
bool nm_safe_fit_calculate(const nm_config *cfg, uint32_t width,
                           uint32_t height, nm_fit_result *out);
