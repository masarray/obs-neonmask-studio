/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-presets.h"

/* Single source of truth for the built-in designs. Color values are OBS RGBA
 * property integers (0x00BBGGRR), not shader float4/BGRA packed colors. */
static const nm_preset builtin[] = {
    {NM_SHAPE_ROUNDED, NM_ANIM_FLOW, 0x00DD31FFu, 0x00FFDB36u,
     0.81, 0.15, 4.0, 0.85, 18.0, 0.65, 0.65, 0, NM_STYLE_DOUBLE, true, true, 0.75, 0.92, 0.95, 0.10, NM_ORNAMENT_CYBER, 0.86, 2.0, 0.22},
    {NM_SHAPE_CIRCLE, NM_ANIM_FLOW, 0x00FF8B44u, 0x00F84DFFu,
     0.75, 0.15, 5.0, 0.85, 18.0, 0.65, 0.65, 10, NM_STYLE_HUD, true, true, 0.82, 0.90, 0.92, 0.10, NM_ORNAMENT_REACTOR, 0.88, 2.0, 0.22},
    {NM_SHAPE_HEXAGON, NM_ANIM_PULSE, 0x007FFF40u, 0x00B9FF46u,
     0.81, 0.15, 4.0, 0.85, 18.0, 0.65, 0.65, 0, NM_STYLE_HUD, true, true, 0.72, 0.76, 0.74, 0.12, NM_ORNAMENT_NONE, 0.0, 2.0, 0.22},
    {NM_SHAPE_ROUNDED, NM_ANIM_STATIC, 0x00236BFFu, 0x0000CCFFu,
     0.81, 0.08, 4.0, 0.85, 18.0, 0.78, 0.65, 0, NM_STYLE_MINIMAL, true, true, 0.62, 0.60, 0.35, 0.12, NM_ORNAMENT_NONE, 0.0, 2.0, 0.22},
    /* Geometrically distinct Tech HUD brackets; old IDs are untouched. */
    {NM_SHAPE_RECTANGLE, NM_ANIM_FLOW, 0x00F5C62Au, 0x00FF9F00u,
     0.78, 0.0, 3.5, 0.75, 20.0, 0.70, 0.55, 0, NM_STYLE_HUD,
     true, true, 0.78, 0.82, 0.83, 0.09, NM_ORNAMENT_TECH_HUD, 0.91, 2.0, 0.22},
    /* Reselecting Streamer now gives a real integrated bubble silhouette;
     * old saved scenes keep their stored shape/ornament until changed. */
    {NM_SHAPE_CHAT_BUBBLE, NM_ANIM_FLOW, 0x00DA33FFu, 0x00FFAE42u,
     0.81, 0.19, 4.0, 0.85, 18.0, 0.72, 0.58, 0, NM_STYLE_DOUBLE,
     true, true, 0.79, 0.89, 0.86, 0.10, NM_ORNAMENT_NONE, 0.0, 2.0, 0.23},
    /* First built-in geometric angled/cut-corner mask, no generic overlay. */
    {NM_SHAPE_ANGLED_CARD, NM_ANIM_STATIC, 0x00FF317Au, 0x00FFAA19u,
     0.81, 0.08, 4.0, 0.85, 20.0, 0.73, 0.58, 0, NM_STYLE_DOUBLE,
     true, true, 0.77, 0.85, 0.76, 0.11, NM_ORNAMENT_NONE, 0.0, 2.0, 0.23}
};

bool nm_get_preset(int id, nm_preset *out)
{
    if (!out || id < 1 || id > (int)(sizeof(builtin) / sizeof(builtin[0])))
        return false;
    *out = builtin[id - 1];
    return true;
}
