/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-presets.h"
#include <stdio.h>

static int failures;
static void check(const char *name, int condition)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

int main(void)
{
    nm_preset p = {0};
    check("custom preset invalid", !nm_get_preset(0, &p));
    check("negative preset invalid", !nm_get_preset(-1, &p));
    check("unknown preset invalid", !nm_get_preset(12, &p));
    check("null output invalid", !nm_get_preset(1, NULL));
    for (int id = 1; id <= 11; ++id) {
        check("known preset", nm_get_preset(id, &p));
        check("valid shape", p.shape >= NM_SHAPE_ROUNDED && p.shape <= NM_SHAPE_GAME_UI);
        check("valid animation", p.animation >= NM_ANIM_STATIC && p.animation <= NM_ANIM_FLOW);
        check("nonzero colors", p.primary != p.secondary && p.primary != 0 && p.secondary != 0);
        check("scale range", p.scale >= 0.10 && p.scale <= 0.98);
        check("roundness range", p.roundness >= 0.0 && p.roundness <= 1.0);
        check("shape detail range", p.shape_detail >= 0.08 && p.shape_detail <= 0.35);
        check("width range", p.border_width >= 0.5 && p.border_width <= 64.0);
        check("D4F ornament width range",p.ornament_width >= 1.0 && p.ornament_width <= 64.0);
        check("D4F ornament X range",p.ornament_length_x >= 8.0 && p.ornament_length_x <= 240.0);
        check("D4F ornament Y range",p.ornament_length_y >= 8.0 && p.ornament_length_y <= 240.0);
        check("D4F inner rail range",p.inner_rail_width >= 0.5 && p.inner_rail_width <= 12.0);
        check("feather range", p.feather >= 0.5 && p.feather <= 30.0);
        check("glow radius range", p.glow_radius >= 1.0 && p.glow_radius <= 80.0);
        check("glow range", p.glow_strength >= 0.0 && p.glow_strength <= 1.0);
        check("mid glow range", p.mid_glow >= 0.0 && p.mid_glow <= 1.0);
        check("bloom range", p.bloom_strength >= 0.0 && p.bloom_strength <= 1.0);
        check("hotspot range", p.hotspot_strength >= 0.0 && p.hotspot_strength <= 1.0);
        check("hotspot size range", p.hotspot_size >= 0.04 && p.hotspot_size <= 0.25);
        check("speed range", p.speed >= 0.0 && p.speed <= 5.0);
        check("art recipe valid", nm_art_recipe_valid(p.ornament_mode, (float)p.art_gap, (float)p.art_intensity));
        check("border enabled", p.border_enabled);
        check("glow enabled", p.glow_enabled);
        check("segments range", p.segments >= 0 && p.segments <= 48);
        check("border style range", p.style >= NM_STYLE_CLASSIC && p.style <= NM_STYLE_MINIMAL);
        check("color mode range", p.color_mode >= NM_COLOR_SOLID && p.color_mode <= NM_COLOR_RAINBOW);
        check("rainbow speed range", p.rainbow_speed >= 0.0 && p.rainbow_speed <= 5.0);
        check("rainbow saturation range", p.rainbow_saturation >= 0.0 && p.rainbow_saturation <= 1.0);
        check("rainbow hue range", p.rainbow_hue_offset >= 0.0 && p.rainbow_hue_offset <= 1.0);
        check("rainbow spread range", p.rainbow_spread >= 0.25 && p.rainbow_spread <= 3.0);
    }
    nm_get_preset(1, &p);
    check("Cyber has authored ornament", p.ornament_mode == NM_ORNAMENT_CYBER);
    check("Cyber art visible", p.art_intensity > 0.80);
    nm_get_preset(2, &p);
    check("reactor ring shape", p.shape == NM_SHAPE_CIRCLE);
    check("reactor authored orbit", p.ornament_mode == NM_ORNAMENT_REACTOR);
    check("reactor segmented", p.segments == 10);
    check("reactor HUD style", p.style == NM_STYLE_HUD);
    nm_get_preset(3, &p);
    check("emerald hex", p.shape == NM_SHAPE_HEXAGON);
    nm_get_preset(4, &p);
    check("ember static", p.animation == NM_ANIM_STATIC);
    check("ember minimal style", p.style == NM_STYLE_MINIMAL);
    nm_get_preset(5, &p);
    check("Tech HUD preset uses dedicated mask + ornament",
          p.shape == NM_SHAPE_TECH_HUD && p.ornament_mode == NM_ORNAMENT_TECH_HUD);
    check("Tech HUD D4F is canvas-visible and strongly separated",
          p.shape_detail <= 0.13 && p.art_gap >= 20.0 &&
          p.border_width >= 4.0 && p.ornament_width >= 16.0 &&
          p.ornament_length_x >= 80.0 && p.ornament_length_y >= 64.0 &&
          p.art_intensity >= 0.99);
    nm_get_preset(6, &p);
    check("Streamer preset becomes true bubble mask", p.shape == NM_SHAPE_CHAT_BUBBLE && p.ornament_mode == NM_ORNAMENT_NONE);
    nm_get_preset(7, &p);
    check("Angled Card preset", p.shape == NM_SHAPE_ANGLED_CARD && p.ornament_mode == NM_ORNAMENT_NONE);
    nm_get_preset(8, &p);
    check("HUD cut preset clips source with authored silhouette",
          p.shape == NM_SHAPE_HUD_PANEL && p.ornament_mode == NM_ORNAMENT_TECH_HUD);
    nm_get_preset(9, &p);
    check("Squircle preset uses genuine superellipse", p.shape == NM_SHAPE_SQUIRCLE);
    nm_get_preset(10, &p);
    check("Game UI preset uses dedicated mask and recipe",
          p.shape == NM_SHAPE_GAME_UI && p.ornament_mode == NM_ORNAMENT_GAME_UI &&
          p.animation == NM_ANIM_STATIC);
    check("Game UI D4F outer brackets dominate a visible base frame",
          p.border_width >= 4.0 && p.art_gap >= 12.0 &&
          p.ornament_width >= 14.0 && p.inner_rail_width <= 2.0 &&
          p.art_intensity >= 0.99);
    nm_get_preset(11, &p);
    check("P6A Gradient Rainbow uses rounded hero frame",
          p.shape == NM_SHAPE_ROUNDED && p.ornament_mode == NM_ORNAMENT_NONE);
    check("P6A Gradient Rainbow owns Rainbow color recipe",
          p.color_mode == NM_COLOR_RAINBOW &&
          p.rainbow_speed > 0.0 && p.rainbow_saturation >= 0.90 &&
          p.rainbow_spread > 0.80 && p.rainbow_spread < 1.20);
    check("P6A Gradient Rainbow moves hue without white Flow comet",
          p.animation == NM_ANIM_FLOW && p.hotspot_strength == 0.0);
    if (failures) return 1;
    puts("PASS: stable preset IDs through P6A Gradient Rainbow preset 11");
    return 0;
}
