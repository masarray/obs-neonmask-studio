/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-config.h"
#include "neonmask-math.h"

void nm_config_defaults(nm_config *cfg)
{
    if (!cfg) return;

    *cfg = (nm_config){
        .schema_version = NM_CONFIG_SCHEMA_VERSION,
        .scale = 0.81f,
        .mask_x = 0.0f,
        .mask_y = 0.0f,
        .mask_width = 1.0f,
        .mask_height = 1.0f,
        .subject_x = 0.0f,
        .subject_y = 0.0f,
        .subject_zoom = 1.0f,
        .polygon_sides = 8,
        .polygon_rotation = 0.0f,
        .roundness = 0.15f,
        .border_px = 4.0f,
        .feather_px = 0.85f,
        .glow_px = 18.0f,
        .glow_amount = 0.65f,
        .animation_speed = 0.65f,
        .primary = 0xFFDD31FFu,
        .secondary = 0xFFFFDB36u,
        .shape_id = NM_SHAPE_ROUNDED,
        .animation_id = NM_ANIM_FLOW,
        .segment_count = 0,
        .style_id = NM_STYLE_DOUBLE,
        .show_border = true,
        .show_glow = true,
    };

    (void)nm_config_apply_preset(cfg, 1);
}

void nm_config_validate(nm_config *cfg)
{
    if (!cfg) return;

    cfg->schema_version = NM_CONFIG_SCHEMA_VERSION;
    cfg->scale = nm_clamp(cfg->scale, 0.30f, 0.96f);
    cfg->mask_x = nm_clamp(cfg->mask_x, -4096.0f, 4096.0f);
    cfg->mask_y = nm_clamp(cfg->mask_y, -4096.0f, 4096.0f);
    cfg->mask_width = nm_clamp(cfg->mask_width, 0.25f, 1.25f);
    cfg->mask_height = nm_clamp(cfg->mask_height, 0.25f, 1.25f);
    cfg->subject_x = nm_clamp(cfg->subject_x, -4096.0f, 4096.0f);
    cfg->subject_y = nm_clamp(cfg->subject_y, -4096.0f, 4096.0f);
    cfg->subject_zoom = nm_clamp(cfg->subject_zoom, 0.5f, 3.0f);
    if (cfg->polygon_sides < 5) cfg->polygon_sides = 5;
    if (cfg->polygon_sides > 12) cfg->polygon_sides = 12;
    cfg->polygon_rotation = nm_clamp(cfg->polygon_rotation, -180.0f, 180.0f);
    cfg->roundness = nm_clamp(cfg->roundness, 0.0f, 1.0f);
    cfg->border_px = nm_clamp(cfg->border_px, 0.5f, 32.0f);
    cfg->feather_px = nm_clamp(cfg->feather_px, 0.5f, 30.0f);
    cfg->glow_px = nm_clamp(cfg->glow_px, 1.0f, 80.0f);
    cfg->glow_amount = nm_clamp(cfg->glow_amount, 0.0f, 1.0f);
    cfg->animation_speed = nm_clamp(cfg->animation_speed, 0.0f, 5.0f);

    cfg->primary |= 0xFF000000u;
    cfg->secondary |= 0xFF000000u;

    if (cfg->shape_id < NM_SHAPE_ROUNDED || cfg->shape_id > NM_SHAPE_POLYGON)
        cfg->shape_id = NM_SHAPE_ROUNDED;
    if (cfg->animation_id < NM_ANIM_STATIC || cfg->animation_id > NM_ANIM_FLOW)
        cfg->animation_id = NM_ANIM_STATIC;
    if (cfg->style_id < NM_STYLE_CLASSIC || cfg->style_id > NM_STYLE_MINIMAL)
        cfg->style_id = NM_STYLE_CLASSIC;

    if (cfg->segment_count < 0) cfg->segment_count = 0;
    if (cfg->segment_count > 48) cfg->segment_count = 48;
}

bool nm_config_apply_preset(nm_config *cfg, int preset_id)
{
    if (!cfg) return false;

    nm_preset preset;
    if (!nm_get_preset(preset_id, &preset))
        return false;

    cfg->shape_id = preset.shape;
    cfg->animation_id = preset.animation;
    cfg->primary = preset.primary | 0xFF000000u;
    cfg->secondary = preset.secondary | 0xFF000000u;
    cfg->scale = (float)preset.scale;
    cfg->roundness = (float)preset.roundness;
    cfg->border_px = (float)preset.border_width;
    cfg->feather_px = (float)preset.feather;
    cfg->glow_px = (float)preset.glow_radius;
    cfg->glow_amount = (float)preset.glow_strength;
    cfg->animation_speed = (float)preset.speed;
    cfg->segment_count = preset.segments;
    cfg->style_id = preset.style;
    cfg->show_border = preset.border_enabled;
    cfg->show_glow = preset.glow_enabled;
    cfg->schema_version = NM_CONFIG_SCHEMA_VERSION;

    nm_config_validate(cfg);
    return true;
}

bool nm_config_schema_supported(uint32_t schema_version)
{
    /* v0 and v1 load via additive defaults; new framing fields are neutral. */
    return schema_version <= NM_CONFIG_SCHEMA_VERSION;
}
