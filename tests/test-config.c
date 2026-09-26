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
    check("default border enabled", cfg.show_border);
    check("default glow enabled", cfg.show_glow);
    check("neutral old scene mask position", cfg.mask_x == 0.0f && cfg.mask_y == 0.0f);
    check("neutral old scene subject pan", cfg.subject_x == 0.0f && cfg.subject_y == 0.0f);
    near("neutral old scene mask width", cfg.mask_width, 1.0f);
    near("neutral old scene mask height", cfg.mask_height, 1.0f);
    near("neutral old scene subject zoom", cfg.subject_zoom, 1.0f);
    check("default polygon sides", cfg.polygon_sides == 8);

    cfg.mask_x = INFINITY;
    cfg.mask_y = -5000.0f;
    cfg.mask_width = 0.01f;
    cfg.mask_height = 5.0f;
    cfg.subject_x = NAN;
    cfg.subject_y = 5000.0f;
    cfg.subject_zoom = 0.0f;
    cfg.polygon_sides = 99;
    cfg.polygon_rotation = INFINITY;
    cfg.scale = -FLT_MAX;
    cfg.roundness = INFINITY;
    cfg.border_px = -10.0f;
    cfg.feather_px = NAN;
    cfg.glow_px = 999.0f;
    cfg.glow_amount = -1.0f;
    cfg.animation_speed = 99.0f;
    cfg.shape_id = 999;
    cfg.animation_id = -99;
    cfg.style_id = 99;
    cfg.segment_count = 999;
    cfg.primary = 0x00010203u;
    cfg.secondary = 0x000A0B0Cu;
    nm_config_validate(&cfg);

    near("mask x invalid", cfg.mask_x, -4096.0f);
    near("mask y range", cfg.mask_y, -4096.0f);
    near("mask width range", cfg.mask_width, 0.25f);
    near("mask height range", cfg.mask_height, 1.25f);
    near("subject x NaN", cfg.subject_x, -4096.0f);
    near("subject y range", cfg.subject_y, 4096.0f);
    near("subject zoom range", cfg.subject_zoom, 0.5f);
    check("polygon sides range", cfg.polygon_sides == 12);
    near("rotation invalid", cfg.polygon_rotation, -180.0f);
    near("scale clamp", cfg.scale, 0.30f);
    near("roundness inf fallback", cfg.roundness, 0.0f);
    near("border clamp", cfg.border_px, 0.5f);
    near("feather nan fallback", cfg.feather_px, 0.5f);
    near("glow radius clamp", cfg.glow_px, 80.0f);
    near("glow amount clamp", cfg.glow_amount, 0.0f);
    near("speed clamp", cfg.animation_speed, 5.0f);
    check("shape fallback", cfg.shape_id == NM_SHAPE_ROUNDED);
    check("animation fallback", cfg.animation_id == NM_ANIM_STATIC);
    check("style fallback", cfg.style_id == NM_STYLE_CLASSIC);
    cfg.shape_id = NM_SHAPE_POLYGON;
    nm_config_validate(&cfg);
    check("new shape enum retained", cfg.shape_id == NM_SHAPE_POLYGON);
    check("segments clamp", cfg.segment_count == 48);
    check("primary forced opaque", (cfg.primary & 0xFF000000u) == 0xFF000000u);
    check("secondary forced opaque", (cfg.secondary & 0xFF000000u) == 0xFF000000u);

    cfg.mask_x = 57.0f;
    cfg.mask_y = -30.0f;
    cfg.subject_x = 99.0f;
    cfg.subject_y = -41.0f;
    cfg.subject_zoom = 1.7f;
    cfg.mask_width = 0.7f;
    cfg.mask_height = 1.1f;
    cfg.feather_px = 29.0f;
    cfg.glow_px = 70.0f;
    cfg.animation_speed = 4.0f;
    cfg.show_border = false;
    cfg.show_glow = false;
    check("apply preset", nm_config_apply_preset(&cfg, 2));
    check("preset shape", cfg.shape_id == NM_SHAPE_CIRCLE);
    check("preset style", cfg.style_id == NM_STYLE_HUD);
    near("preset feather complete", cfg.feather_px, 0.85f);
    near("preset glow radius complete", cfg.glow_px, 18.0f);
    near("preset speed complete", cfg.animation_speed, 0.65f);
    check("preset restores border", cfg.show_border);
    check("preset restores glow", cfg.show_glow);
    near("preset retains mask X", cfg.mask_x, 57.0f);
    near("preset retains mask Y", cfg.mask_y, -30.0f);
    near("preset retains subject pan X", cfg.subject_x, 99.0f);
    near("preset retains subject pan Y", cfg.subject_y, -41.0f);
    near("preset retains subject zoom", cfg.subject_zoom, 1.7f);
    near("preset retains custom width", cfg.mask_width, 0.7f);
    near("preset retains custom height", cfg.mask_height, 1.1f);

    check("legacy schema supported", nm_config_schema_supported(0));
    check("v1 legacy schema supported", nm_config_schema_supported(1));
    check("current schema supported", nm_config_schema_supported(NM_CONFIG_SCHEMA_VERSION));
    check("future schema rejected", !nm_config_schema_supported(NM_CONFIG_SCHEMA_VERSION + 1));

    check("unknown preset rejected", !nm_config_apply_preset(&cfg, 99));
    check("null preset target rejected", !nm_config_apply_preset(NULL, 1));

    if (failures) return 1;
    puts("PASS: canonical config defaults, validation, complete presets and schema policy");
    return 0;
}
