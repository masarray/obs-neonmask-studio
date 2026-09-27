/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-config.h"
#include <float.h>
#include <math.h>
#include <stdio.h>

static int failures;

static void check(const char *name, int condition)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

static void near(const char *name, float got, float expected)
{
    if (fabsf(got - expected) > 0.0001f) {
        fprintf(stderr, "FAIL: %s got=%f expected=%f\n", name, got, expected);
        ++failures;
    }
}

int main(void)
{
    nm_config cfg;
    nm_config_defaults(&cfg);
    check("schema current", cfg.schema_version == NM_CONFIG_SCHEMA_VERSION);
    check("default shape", cfg.shape_id == NM_SHAPE_ROUNDED);
    check("default style", cfg.style_id == NM_STYLE_DOUBLE);
    near("default mask width", cfg.mask_width, 0.81f);
    near("default mask height", cfg.mask_height, 0.81f);
    near("default mask x", cfg.mask_x_px, 0.0f);
    near("default mask y", cfg.mask_y_px, 0.0f);
    near("default subject pan x", cfg.subject_pan_x_px, 0.0f);
    near("default subject pan y", cfg.subject_pan_y_px, 0.0f);
    near("default subject zoom", cfg.subject_zoom, 1.0f);
    check("default polygon sides", cfg.polygon_sides == 8);
    near("default shape detail", cfg.shape_detail, 0.22f);
    check("default border enabled", cfg.show_border);
    check("default glow enabled", cfg.show_glow);
    check("legacy custom art defaults off", cfg.ornament_mode == NM_ORNAMENT_NONE && cfg.art_intensity == 0.0f);
    check("premium defaults active", cfg.mid_glow_strength > 0.0f &&
          cfg.bloom_strength > 0.0f && cfg.hotspot_strength > 0.0f);
    check("premium hotspot range", cfg.hotspot_size >= 0.04f && cfg.hotspot_size <= 0.25f);

    cfg.scale = -FLT_MAX;
    cfg.mask_width = -FLT_MAX;
    cfg.mask_height = INFINITY;
    cfg.mask_x_px = -99999.0f;
    cfg.mask_y_px = 99999.0f;
    cfg.subject_pan_x_px = NAN;
    cfg.subject_pan_y_px = INFINITY;
    cfg.subject_zoom = 99.0f;
    cfg.shape_rotation_deg = -999.0f;
    cfg.polygon_sides = 99;
    cfg.roundness = INFINITY;
    cfg.shape_detail = INFINITY;
    cfg.border_px = -10.0f;
    cfg.feather_px = NAN;
    cfg.glow_px = 999.0f;
    cfg.glow_amount = -1.0f;
    cfg.mid_glow_strength = NAN;
    cfg.bloom_strength = INFINITY;
    cfg.hotspot_strength = -999.0f;
    cfg.hotspot_size = 999.0f;
    cfg.ornament_mode = 999;
    cfg.art_gap = INFINITY;
    cfg.art_intensity = NAN;
    cfg.animation_speed = 99.0f;
    cfg.shape_id = 999;
    cfg.animation_id = -99;
    cfg.style_id = 99;
    cfg.segment_count = 999;
    cfg.primary = 0x00010203u;
    cfg.secondary = 0x000A0B0Cu;
    nm_config_validate(&cfg);

    near("scale clamp", cfg.scale, 0.10f);
    near("mask width clamp", cfg.mask_width, 0.10f);
    near("mask height inf fallback", cfg.mask_height, 0.10f);
    near("mask x clamp", cfg.mask_x_px, -4096.0f);
    near("mask y clamp", cfg.mask_y_px, 4096.0f);
    near("subject pan x nan fallback", cfg.subject_pan_x_px, -4096.0f);
    near("subject pan y inf fallback", cfg.subject_pan_y_px, -4096.0f);
    near("subject zoom clamp", cfg.subject_zoom, 4.0f);
    near("shape rotation clamp", cfg.shape_rotation_deg, -180.0f);
    check("polygon sides clamp", cfg.polygon_sides == 12);
    near("roundness inf fallback", cfg.roundness, 0.0f);
    near("shape detail inf fallback", cfg.shape_detail, 0.08f);
    near("border clamp", cfg.border_px, 0.5f);
    near("feather nan fallback", cfg.feather_px, 0.5f);
    near("glow radius clamp", cfg.glow_px, 80.0f);
    near("glow amount clamp", cfg.glow_amount, 0.0f);
    near("mid glow nan clamp", cfg.mid_glow_strength, 0.0f);
    near("bloom inf clamp", cfg.bloom_strength, 0.0f);
    near("hotspot strength clamp", cfg.hotspot_strength, 0.0f);
    near("hotspot size clamp", cfg.hotspot_size, 0.25f);
    check("unknown art rejected", cfg.ornament_mode == NM_ORNAMENT_NONE);
    near("art gap inf fallback", cfg.art_gap, 1.0f);
    near("art intensity nan fallback", cfg.art_intensity, 0.0f);
    near("speed clamp", cfg.animation_speed, 5.0f);
    check("shape fallback", cfg.shape_id == NM_SHAPE_ROUNDED);
    cfg.shape_id = NM_SHAPE_POLYGON;
    cfg.polygon_sides = 5;
    nm_config_validate(&cfg);
    check("new polygon enum accepted", cfg.shape_id == NM_SHAPE_POLYGON);
    check("polygon minimum accepted", cfg.polygon_sides == 5);
    cfg.shape_id = NM_SHAPE_CHAT_BUBBLE;
    cfg.shape_detail = 0.23f;
    nm_config_validate(&cfg);
    check("bubble enum accepted", cfg.shape_id == NM_SHAPE_CHAT_BUBBLE);
    cfg.shape_id = NM_SHAPE_ANGLED_CARD;
    nm_config_validate(&cfg);
    check("angled enum accepted", cfg.shape_id == NM_SHAPE_ANGLED_CARD);
    cfg.shape_id = NM_SHAPE_HUD_PANEL;
    nm_config_validate(&cfg);
    check("HUD enum accepted", cfg.shape_id == NM_SHAPE_HUD_PANEL);
    cfg.shape_id = NM_SHAPE_SQUIRCLE;
    nm_config_validate(&cfg);
    check("squircle enum accepted", cfg.shape_id == NM_SHAPE_SQUIRCLE);
    check("animation fallback", cfg.animation_id == NM_ANIM_STATIC);
    check("style fallback", cfg.style_id == NM_STYLE_CLASSIC);
    check("segments clamp", cfg.segment_count == 48);
    check("primary forced opaque", (cfg.primary & 0xFF000000u) == 0xFF000000u);
    check("secondary forced opaque", (cfg.secondary & 0xFF000000u) == 0xFF000000u);

    cfg.feather_px = 29.0f;
    cfg.glow_px = 70.0f;
    cfg.animation_speed = 4.0f;
    cfg.mid_glow_strength = 0.0f;
    cfg.bloom_strength = 0.0f;
    cfg.hotspot_strength = 0.0f;
    cfg.hotspot_size = 0.25f;
    cfg.art_intensity = 1.0f;
    cfg.art_gap = 16.0f;
    cfg.ornament_mode = NM_ORNAMENT_CYBER;
    cfg.show_border = false;
    cfg.show_glow = false;
    check("apply preset", nm_config_apply_preset(&cfg, 2));
    check("preset shape", cfg.shape_id == NM_SHAPE_CIRCLE);
    check("preset style", cfg.style_id == NM_STYLE_HUD);
    near("preset feather complete", cfg.feather_px, 0.85f);
    near("preset glow radius complete", cfg.glow_px, 18.0f);
    near("preset speed complete", cfg.animation_speed, 0.65f);
    near("preset mid glow complete", cfg.mid_glow_strength, 0.82f);
    near("preset bloom complete", cfg.bloom_strength, 0.90f);
    near("preset highlight complete", cfg.hotspot_strength, 0.92f);
    near("preset hotspot size complete", cfg.hotspot_size, 0.10f);
    check("Reactor has authored ring", cfg.ornament_mode == NM_ORNAMENT_REACTOR);
    near("Reactor art intensity complete", cfg.art_intensity, 0.88f);
    near("Reactor art gap complete", cfg.art_gap, 2.0f);
    near("preset shape detail complete", cfg.shape_detail, 0.22f);
    check("Cyber preset applies authored ornament", nm_config_apply_preset(&cfg, 1) && cfg.ornament_mode == NM_ORNAMENT_CYBER && cfg.art_intensity > 0.8f);
    check("preset restores border", cfg.show_border);
    check("preset restores glow", cfg.show_glow);
    check("Streamer selects actual bubble", nm_config_apply_preset(&cfg, 6) && cfg.shape_id == NM_SHAPE_CHAT_BUBBLE);
    check("Angled preset selects diagonal mask", nm_config_apply_preset(&cfg, 7) && cfg.shape_id == NM_SHAPE_ANGLED_CARD);

    check("HUD preset chooses new shape", nm_config_apply_preset(&cfg, 8) && cfg.shape_id == NM_SHAPE_HUD_PANEL);
    check("Squircle preset chooses new shape", nm_config_apply_preset(&cfg, 9) && cfg.shape_id == NM_SHAPE_SQUIRCLE);
    check("legacy schema supported", nm_config_schema_supported(0));
    check("current schema supported", nm_config_schema_supported(NM_CONFIG_SCHEMA_VERSION));
    check("legacy v1 schema supported", nm_config_schema_supported(1));
    check("future schema rejected", !nm_config_schema_supported(NM_CONFIG_SCHEMA_VERSION + 1));
    check("phase-b keeps schema v2 for additive default-backed fields",
          NM_CONFIG_SCHEMA_VERSION == 2u);

    check("unknown preset rejected", !nm_config_apply_preset(&cfg, 99));
    check("null preset target rejected", !nm_config_apply_preset(NULL, 1));

    if (failures) return 1;
    puts("PASS: canonical config defaults, validation, complete presets and schema policy");
    return 0;
}
