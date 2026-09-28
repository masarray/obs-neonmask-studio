/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-config.h"
#include "neonmask-math.h"

void nm_config_defaults(nm_config *cfg)
{
    if (!cfg) return;

    *cfg = (nm_config){
        .schema_version = NM_CONFIG_SCHEMA_VERSION,
        .scale = 0.81f,
        .mask_width = 0.81f,
        .mask_height = 0.81f,
        .safe_fit = false,
        .expand_canvas = false,
        .mask_x_px = 0.0f,
        .mask_y_px = 0.0f,
        .subject_pan_x_px = 0.0f,
        .subject_pan_y_px = 0.0f,
        .subject_zoom = 1.0f,
        .shape_rotation_deg = 0.0f,
        .polygon_sides = 8,
        .roundness = 0.15f,
        .shape_detail = 0.22f,
        .bubble_left_inset = 0.0f,
        .bubble_right_inset = 0.0f,
        .bubble_top_inset = 0.0f,
        .bubble_bottom_inset = 0.0f,
        .bubble_tail_left = -0.78f,
        .bubble_tail_right = -0.34f,
        .bubble_tail_tip = -0.72f,
        .bubble_tl_x = -1.0f, .bubble_tl_y = -1.0f,
        .bubble_tr_x = 1.0f,  .bubble_tr_y = -1.0f,
        .bubble_br_x = 1.0f,  .bubble_br_y = 0.604f,
        .bubble_bl_x = -1.0f, .bubble_bl_y = 0.604f,
        .bubble_tail_start = 0.11f,
        .bubble_tail_end = 0.33f,
        .bubble_tail_tip_pos = 0.14f,
        .bubble_tail_depth = 0.22f,
        .border_px = 4.0f,
        .feather_px = 0.85f,
        .glow_px = 18.0f,
        .glow_amount = 0.65f,
        .mid_glow_strength = 0.75f,
        .bloom_strength = 0.92f,
        .hotspot_strength = 0.95f,
        .hotspot_size = 0.10f,
        .art_intensity = 0.0f,
        .art_gap = 2.0f,
        .ornament_mode = NM_ORNAMENT_NONE,
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
    /* Existing custom scenes retain their unornamented appearance. Cyber
     * artwork is opt-in through the explicit Cyber preset or design control. */
    cfg->ornament_mode = NM_ORNAMENT_NONE;
    cfg->art_intensity = 0.0f;
}

void nm_config_validate(nm_config *cfg)
{
    if (!cfg) return;

    cfg->schema_version = NM_CONFIG_SCHEMA_VERSION;
    cfg->svg_path[NM_SVG_PATH_MAX-1] = 0;
    cfg->scale = nm_clamp(cfg->scale, 0.10f, 0.98f);
    cfg->mask_width = nm_clamp(cfg->mask_width, 0.10f, 0.98f);
    cfg->mask_height = nm_clamp(cfg->mask_height, 0.10f, 0.98f);
    cfg->mask_x_px = nm_clamp(cfg->mask_x_px, -4096.0f, 4096.0f);
    cfg->mask_y_px = nm_clamp(cfg->mask_y_px, -4096.0f, 4096.0f);
    cfg->subject_pan_x_px = nm_clamp(cfg->subject_pan_x_px, -4096.0f, 4096.0f);
    cfg->subject_pan_y_px = nm_clamp(cfg->subject_pan_y_px, -4096.0f, 4096.0f);
    cfg->subject_zoom = nm_clamp(cfg->subject_zoom, 0.25f, 4.0f);
    cfg->shape_rotation_deg = nm_clamp(cfg->shape_rotation_deg, -180.0f, 180.0f);
    if (cfg->polygon_sides < 5) cfg->polygon_sides = 5;
    if (cfg->polygon_sides > 12) cfg->polygon_sides = 12;
    cfg->roundness = nm_clamp(cfg->roundness, 0.0f, 1.0f);
    cfg->shape_detail = nm_clamp(cfg->shape_detail, 0.08f, 0.35f);
    cfg->bubble_left_inset = nm_clamp(cfg->bubble_left_inset, 0.0f, 0.25f);
    cfg->bubble_right_inset = nm_clamp(cfg->bubble_right_inset, 0.0f, 0.25f);
    cfg->bubble_top_inset = nm_clamp(cfg->bubble_top_inset, 0.0f, 0.25f);
    cfg->bubble_bottom_inset = nm_clamp(cfg->bubble_bottom_inset, 0.0f, 0.25f);
    cfg->bubble_tail_left = nm_clamp(cfg->bubble_tail_left, -0.90f, 0.80f);
    cfg->bubble_tail_right = nm_clamp(cfg->bubble_tail_right,
                                      cfg->bubble_tail_left + 0.08f, 0.90f);
    cfg->bubble_tail_tip = nm_clamp(cfg->bubble_tail_tip, -0.95f, 0.95f);
    /* Freeform corners deliberately stay in four disjoint quadrants. This
     * makes every slider combination a convex, non-self-intersecting body. */
    cfg->bubble_tl_x = nm_clamp(cfg->bubble_tl_x, -1.0f, -0.35f);
    cfg->bubble_tl_y = nm_clamp(cfg->bubble_tl_y, -1.0f, -0.35f);
    cfg->bubble_tr_x = nm_clamp(cfg->bubble_tr_x, 0.35f, 1.0f);
    cfg->bubble_tr_y = nm_clamp(cfg->bubble_tr_y, -1.0f, -0.35f);
    cfg->bubble_br_x = nm_clamp(cfg->bubble_br_x, 0.35f, 1.0f);
    cfg->bubble_br_y = nm_clamp(cfg->bubble_br_y, 0.35f, 0.92f);
    cfg->bubble_bl_x = nm_clamp(cfg->bubble_bl_x, -1.0f, -0.35f);
    cfg->bubble_bl_y = nm_clamp(cfg->bubble_bl_y, 0.35f, 0.92f);
    cfg->bubble_tail_start = nm_clamp(cfg->bubble_tail_start, 0.02f, 0.88f);
    cfg->bubble_tail_end = nm_clamp(cfg->bubble_tail_end,
                                    cfg->bubble_tail_start + 0.06f, 0.98f);
    cfg->bubble_tail_tip_pos = nm_clamp(cfg->bubble_tail_tip_pos, 0.0f, 1.0f);
    cfg->bubble_tail_depth = nm_clamp(cfg->bubble_tail_depth, 0.08f, 0.35f);
    cfg->border_px = nm_clamp(cfg->border_px, 0.5f, 32.0f);
    cfg->feather_px = nm_clamp(cfg->feather_px, 0.5f, 30.0f);
    cfg->glow_px = nm_clamp(cfg->glow_px, 1.0f, 80.0f);
    cfg->glow_amount = nm_clamp(cfg->glow_amount, 0.0f, 1.0f);
    cfg->mid_glow_strength = nm_clamp(cfg->mid_glow_strength, 0.0f, 1.0f);
    cfg->bloom_strength = nm_clamp(cfg->bloom_strength, 0.0f, 1.0f);
    cfg->hotspot_strength = nm_clamp(cfg->hotspot_strength, 0.0f, 1.0f);
    cfg->hotspot_size = nm_clamp(cfg->hotspot_size, 0.04f, 0.25f);
    cfg->art_gap = nm_clamp(cfg->art_gap, 1.0f, 16.0f);
    cfg->art_intensity = nm_clamp(cfg->art_intensity, 0.0f, 1.0f);
    if (cfg->ornament_mode < NM_ORNAMENT_NONE || cfg->ornament_mode > NM_ORNAMENT_GAME_UI)
        cfg->ornament_mode = NM_ORNAMENT_NONE;
    cfg->animation_speed = nm_clamp(cfg->animation_speed, 0.0f, 5.0f);

    cfg->primary |= 0xFF000000u;
    cfg->secondary |= 0xFF000000u;

    if (cfg->shape_id < NM_SHAPE_ROUNDED || cfg->shape_id > NM_SHAPE_GAME_UI)
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
    cfg->shape_detail = (float)preset.shape_detail;
    /* Applying a named preset resets the bubble designer. Shape switching
     * alone never resets user-authored values. */
    cfg->bubble_left_inset = cfg->bubble_right_inset = 0.0f;
    cfg->bubble_top_inset = cfg->bubble_bottom_inset = 0.0f;
    cfg->bubble_tail_left = -0.78f;
    cfg->bubble_tail_right = -0.34f;
    cfg->bubble_tail_tip = -0.72f;
    cfg->bubble_tl_x = -1.0f; cfg->bubble_tl_y = -1.0f;
    cfg->bubble_tr_x = 1.0f;  cfg->bubble_tr_y = -1.0f;
    cfg->bubble_br_x = 1.0f;  cfg->bubble_br_y = 1.0f-1.8f*cfg->shape_detail;
    cfg->bubble_bl_x = -1.0f; cfg->bubble_bl_y = cfg->bubble_br_y;
    cfg->bubble_tail_start = 0.11f;
    cfg->bubble_tail_end = 0.33f;
    cfg->bubble_tail_tip_pos = 0.14f;
    cfg->bubble_tail_depth = cfg->shape_detail;
    cfg->border_px = (float)preset.border_width;
    cfg->feather_px = (float)preset.feather;
    cfg->glow_px = (float)preset.glow_radius;
    cfg->glow_amount = (float)preset.glow_strength;
    cfg->mid_glow_strength = (float)preset.mid_glow;
    cfg->bloom_strength = (float)preset.bloom_strength;
    cfg->hotspot_strength = (float)preset.hotspot_strength;
    cfg->hotspot_size = (float)preset.hotspot_size;
    cfg->ornament_mode = preset.ornament_mode;
    cfg->art_intensity = (float)preset.art_intensity;
    cfg->art_gap = (float)preset.art_gap;
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
    /* Version 0 is the pre-schema legacy scene format. */
    return schema_version <= NM_CONFIG_SCHEMA_VERSION;
}
