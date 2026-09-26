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
    check("segments clamp", cfg.segment_count == 48);
    check("primary forced opaque", (cfg.primary & 0xFF000000u) == 0xFF000000u);
    check("secondary forced opaque", (cfg.secondary & 0xFF000000u) == 0xFF000000u);

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

    check("legacy schema supported", nm_config_schema_supported(0));
    check("current schema supported", nm_config_schema_supported(NM_CONFIG_SCHEMA_VERSION));
    check("future schema rejected", !nm_config_schema_supported(NM_CONFIG_SCHEMA_VERSION + 1));

    check("unknown preset rejected", !nm_config_apply_preset(&cfg, 99));
    check("null preset target rejected", !nm_config_apply_preset(NULL, 1));

    if (failures) return 1;
    puts("PASS: canonical config defaults, validation, complete presets and schema policy");
    return 0;
}
