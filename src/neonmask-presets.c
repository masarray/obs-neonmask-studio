/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-presets.h"

/* Single source of truth for the built-in designs. Color values are OBS RGBA
 * property integers (0x00BBGGRR), not shader float4/BGRA packed colors. */
static const nm_preset builtin[] = {
    {NM_SHAPE_ROUNDED, NM_ANIM_FLOW, 0x00DD31FFu, 0x00FFDB36u,
     0.81, 0.15, 4.0, 0.85, 18.0, 0.65, 0.65, 0, NM_STYLE_DOUBLE,
     true, true, 0.75, 0.92, 0.95, 0.10, NM_ORNAMENT_CYBER, 0.86, 2.0, 0.22,
     10.0, 60.0, 48.0, 1.4, NM_COLOR_DUAL, 0.65, 0.92, 0.0, 1.0},
    {NM_SHAPE_CIRCLE, NM_ANIM_FLOW, 0x00FF8B44u, 0x00F84DFFu,
     0.75, 0.15, 5.0, 0.85, 18.0, 0.65, 0.65, 10, NM_STYLE_HUD,
     true, true, 0.82, 0.90, 0.92, 0.10, NM_ORNAMENT_REACTOR, 0.88, 2.0, 0.22,
     10.0, 60.0, 48.0, 1.4, NM_COLOR_DUAL, 0.65, 0.92, 0.0, 1.0},
    {NM_SHAPE_HEXAGON, NM_ANIM_PULSE, 0x007FFF40u, 0x00B9FF46u,
     0.81, 0.15, 4.0, 0.85, 18.0, 0.65, 0.65, 0, NM_STYLE_HUD,
     true, true, 0.72, 0.76, 0.74, 0.12, NM_ORNAMENT_NONE, 0.0, 2.0, 0.22,
     10.0, 60.0, 48.0, 1.4, NM_COLOR_DUAL, 0.65, 0.92, 0.0, 1.0},
    {NM_SHAPE_ROUNDED, NM_ANIM_STATIC, 0x00236BFFu, 0x0000CCFFu,
     0.81, 0.08, 4.0, 0.85, 18.0, 0.78, 0.65, 0, NM_STYLE_MINIMAL,
     true, true, 0.62, 0.60, 0.35, 0.12, NM_ORNAMENT_NONE, 0.0, 2.0, 0.22,
     10.0, 60.0, 48.0, 1.4, NM_COLOR_DUAL, 0.65, 0.92, 0.0, 1.0},
    /* D4F: Tech HUD uses two dominant outer Ls only (TR + BL). */
    {NM_SHAPE_TECH_HUD, NM_ANIM_FLOW, 0x00F5C62Au, 0x00FF9F00u,
     0.78, 0.0, 5.0, 0.70, 14.0, 0.56, 0.42, 0, NM_STYLE_HUD,
     true, true, 0.66, 0.70, 0.60, 0.09, NM_ORNAMENT_TECH_HUD, 1.00, 24.0, 0.12,
     18.0, 84.0, 68.0, 1.6, NM_COLOR_DUAL, 0.65, 0.92, 0.0, 1.0},
    {NM_SHAPE_CHAT_BUBBLE, NM_ANIM_FLOW, 0x00DA33FFu, 0x00FFAE42u,
     0.81, 0.19, 4.0, 0.85, 18.0, 0.72, 0.58, 0, NM_STYLE_DOUBLE,
     true, true, 0.79, 0.89, 0.86, 0.10, NM_ORNAMENT_NONE, 0.0, 2.0, 0.23,
     10.0, 60.0, 48.0, 1.4, NM_COLOR_DUAL, 0.65, 0.92, 0.0, 1.0},
    {NM_SHAPE_ANGLED_CARD, NM_ANIM_STATIC, 0x00FF317Au, 0x00FFAA19u,
     0.81, 0.0, 4.0, 0.85, 20.0, 0.73, 0.58, 0, NM_STYLE_DOUBLE,
     true, true, 0.77, 0.85, 0.76, 0.11, NM_ORNAMENT_NONE, 0.0, 2.0, 0.23,
     10.0, 60.0, 48.0, 1.4, NM_COLOR_DUAL, 0.65, 0.92, 0.0, 1.0},
    {NM_SHAPE_HUD_PANEL, NM_ANIM_FLOW, 0x00F5C62Au, 0x00FF9F00u,
     0.81, 0.0, 4.0, 0.85, 20.0, 0.73, 0.56, 0, NM_STYLE_DOUBLE,
     true, true, 0.79, 0.89, 0.83, 0.09, NM_ORNAMENT_TECH_HUD, 0.87, 2.0, 0.25,
     10.0, 60.0, 48.0, 1.4, NM_COLOR_DUAL, 0.65, 0.92, 0.0, 1.0},
    {NM_SHAPE_SQUIRCLE, NM_ANIM_PULSE, 0x00FD55CAu, 0x00FFFF35u,
     0.81, 0.15, 4.0, 0.85, 20.0, 0.74, 0.54, 0, NM_STYLE_DOUBLE,
     true, true, 0.77, 0.88, 0.65, 0.12, NM_ORNAMENT_NONE, 0.0, 2.0, 0.22,
     10.0, 60.0, 48.0, 1.4, NM_COLOR_DUAL, 0.65, 0.92, 0.0, 1.0},
    /* D4F: Game UI keeps four symmetric outer Ls plus one thin inner rail. */
    {NM_SHAPE_GAME_UI, NM_ANIM_STATIC, 0x0055FF55u, 0x0040FF66u,
     0.80, 0.0, 5.0, 0.68, 12.0, 0.50, 0.0, 0, NM_STYLE_CLASSIC,
     true, true, 0.62, 0.66, 0.0, 0.10, NM_ORNAMENT_GAME_UI, 1.00, 14.0, 0.14,
     16.0, 76.0, 64.0, 1.4, NM_COLOR_DUAL, 0.65, 0.92, 0.0, 1.0},
    /* P6A: a stable bright frame whose spectrum travels around the real
     * contour. Flow keeps the bounded rainbow clock running; hotspot=0 means
     * hue moves without a separate white comet or whole-frame brightness pulse. */
    {NM_SHAPE_ROUNDED, NM_ANIM_FLOW, 0x000000FFu, 0x00FFFF00u,
     0.82, 0.19, 5.0, 0.75, 20.0, 0.70, 0.70, 0, NM_STYLE_CLASSIC,
     true, true, 0.78, 0.84, 0.0, 0.10, NM_ORNAMENT_NONE, 0.0, 2.0, 0.22,
     10.0, 60.0, 48.0, 1.4, NM_COLOR_RAINBOW, 0.78, 0.96, 0.0, 1.0},
    /* P6G: keep P6F's mask-relative premium rail mass, but make the two
     * counter-orbiting objects unmistakable: each arc is < half a circle and
     * the authored Flow speed is raised for a clearer opposing-motion read. */
    {NM_SHAPE_CIRCLE, NM_ANIM_FLOW, 0x00FFEA28u, 0x00F52CFFu,
     0.72, 0.0, 0.8, 0.70, 22.0, 0.90, 0.72, 0, NM_STYLE_MINIMAL,
     true, true, 0.88, 0.90, 0.0, 0.08, NM_ORNAMENT_DUAL_RING, 1.0, 8.0, 0.22,
     20.0, 60.0, 48.0, 15.0, NM_COLOR_DUAL, 0.65, 0.92, 0.0, 1.0,
     18.0, 30.0, 14.0, 10.0, 11.0, 20.0}
};

bool nm_get_preset(int id, nm_preset *out)
{
    if (!out || id < 1 || id > (int)(sizeof(builtin) / sizeof(builtin[0])))
        return false;
    *out = builtin[id - 1];
    return true;
}
