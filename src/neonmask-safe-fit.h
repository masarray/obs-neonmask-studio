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
} nm_fit_result;

/* Returns false for invalid inputs; fits=false for impossible placement.
 * It never changes cfg, the mask center, subject pan/zoom or source UV. */
bool nm_safe_fit_calculate(const nm_config *cfg, uint32_t width,
                           uint32_t height, nm_fit_result *out);
