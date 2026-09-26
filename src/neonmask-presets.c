/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-presets.h"

/* Single source of truth for the built-in designs. Color values are OBS RGBA
 * property integers (0x00BBGGRR), not shader float4/BGRA packed colors. */
static const nm_preset builtin[] = {
    {NM_SHAPE_ROUNDED, NM_ANIM_FLOW, 0x00DD31FFu, 0x00FFDB36u,
     0.81, 0.15, 4.0, 0.85, 18.0, 0.65, 0.65, 0, NM_STYLE_DOUBLE, true, true, 0.75, 0.92, 0.95, 0.10, NM_ORNAMENT_CYBER, 0.86, 2.0},
    {NM_SHAPE_CIRCLE, NM_ANIM_FLOW, 0x00FF8B44u, 0x00F84DFFu,
     0.75, 0.15, 5.0, 0.85, 18.0, 0.65, 0.65, 10, NM_STYLE_HUD, true, true, 0.82, 0.90, 0.92, 0.10, NM_ORNAMENT_NONE, 0.0, 2.0},
    {NM_SHAPE_HEXAGON, NM_ANIM_PULSE, 0x007FFF40u, 0x00B9FF46u,
     0.81, 0.15, 4.0, 0.85, 18.0, 0.65, 0.65, 0, NM_STYLE_HUD, true, true, 0.72, 0.76, 0.74, 0.12, NM_ORNAMENT_NONE, 0.0, 2.0},
    {NM_SHAPE_ROUNDED, NM_ANIM_STATIC, 0x00236BFFu, 0x0000CCFFu,
     0.81, 0.08, 4.0, 0.85, 18.0, 0.78, 0.65, 0, NM_STYLE_MINIMAL, true, true, 0.62, 0.60, 0.35, 0.12, NM_ORNAMENT_NONE, 0.0, 2.0}
};

bool nm_get_preset(int id, nm_preset *out)
{
    if (!out || id < 1 || id > (int)(sizeof(builtin) / sizeof(builtin[0])))
        return false;
    *out = builtin[id - 1];
    return true;
}
