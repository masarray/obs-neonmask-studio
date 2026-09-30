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
    check("safe-fit opt-in by default", !cfg.safe_fit);
    check("expanded output opt-in by default", !cfg.expand_canvas);
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
    check("D4D default preserves legacy dual color",cfg.color_mode == NM_COLOR_DUAL);
    near("D4D rainbow speed default",cfg.rainbow_speed,0.65f);
    near("D4D rainbow saturation default",cfg.rainbow_saturation,0.92f);
    near("D4D rainbow hue default",cfg.rainbow_hue_offset,0.0f);
    near("D4D rainbow spread default",cfg.rainbow_spread,1.0f);
    near("D4F legacy-compatible ornament width default",cfg.ornament_width_px,10.0f);
    near("D4F ornament X length default",cfg.ornament_length_x_px,60.0f);
    near("D4F ornament Y length default",cfg.ornament_length_y_px,48.0f);
    near("D4F inner rail width default",cfg.inner_rail_width_px,1.4f);

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
    cfg.ornament_width_px = INFINITY;
    cfg.ornament_length_x_px = NAN;
    cfg.ornament_length_y_px = 9999.0f;
    cfg.inner_rail_width_px = -20.0f;
    cfg.animation_speed = 99.0f;
    cfg.color_mode = 99;
    cfg.rainbow_speed = 99.0f;
    cfg.rainbow_saturation = INFINITY;
    cfg.rainbow_hue_offset = -5.0f;
    cfg.rainbow_spread = 99.0f;
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
    near("art gap inf fallback", cfg.art_gap, 0.0f);
    near("art intensity nan fallback", cfg.art_intensity, 0.0f);
    near("D4F ornament width inf fallback",cfg.ornament_width_px,1.0f);
    near("D4F ornament X nan fallback",cfg.ornament_length_x_px,8.0f);
    near("D4F ornament Y max clamp",cfg.ornament_length_y_px,240.0f);
    near("D4F inner rail min clamp",cfg.inner_rail_width_px,0.5f);
    near("speed clamp", cfg.animation_speed, 5.0f);
    check("unknown color mode falls back to Dual",cfg.color_mode == NM_COLOR_DUAL);
    near("rainbow speed clamp",cfg.rainbow_speed,5.0f);
    near("rainbow saturation invalid fallback",cfg.rainbow_saturation,0.0f);
    near("rainbow hue clamp",cfg.rainbow_hue_offset,0.0f);
    near("rainbow spread clamp",cfg.rainbow_spread,3.0f);
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
    cfg.shape_id = NM_SHAPE_SVG_PATH;
    nm_config_validate(&cfg);
    check("SVG provider shape ID accepted",cfg.shape_id == NM_SHAPE_SVG_PATH);
    cfg.shape_id = NM_SHAPE_TECH_HUD;
    nm_config_validate(&cfg);
    check("Tech HUD advanced shape ID accepted",cfg.shape_id == NM_SHAPE_TECH_HUD);
    cfg.shape_detail = 0.70f;
    nm_config_validate(&cfg);
    near("D4K Tech HUD persisted detail accepts 0.70",cfg.shape_detail,0.70f);
    cfg.shape_detail = 9.0f;
    nm_config_validate(&cfg);
    near("D4K detail hard cap remains bounded",cfg.shape_detail,0.70f);
    cfg.shape_id = NM_SHAPE_GAME_UI;
    cfg.ornament_mode = NM_ORNAMENT_GAME_UI;
    nm_config_validate(&cfg);
    check("Game UI shape ID accepted",cfg.shape_id == NM_SHAPE_GAME_UI);
    check("Game UI ornament accepted",cfg.ornament_mode == NM_ORNAMENT_GAME_UI);
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
    cfg.art_gap = 96.0f;
    cfg.ornament_width_px = 64.0f;
    cfg.ornament_length_x_px = 240.0f;
    cfg.ornament_length_y_px = 240.0f;
    cfg.inner_rail_width_px = 12.0f;
    cfg.ornament_mode = NM_ORNAMENT_CYBER;
    cfg.color_mode = NM_COLOR_RAINBOW;
    cfg.rainbow_speed = 4.0f;
    cfg.rainbow_saturation = 0.25f;
    cfg.rainbow_hue_offset = 0.75f;
    cfg.rainbow_spread = 2.5f;
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
    near("preset D4F ornament width complete",cfg.ornament_width_px,10.0f);
    near("preset D4F ornament X complete",cfg.ornament_length_x_px,60.0f);
    near("preset D4F ornament Y complete",cfg.ornament_length_y_px,48.0f);
    near("preset D4F inner rail complete",cfg.inner_rail_width_px,1.4f);
    near("preset shape detail complete", cfg.shape_detail, 0.22f);
    check("named preset restores Dual color",cfg.color_mode == NM_COLOR_DUAL);
    near("named preset restores rainbow speed default",cfg.rainbow_speed,0.65f);
    near("named preset restores rainbow saturation default",cfg.rainbow_saturation,0.92f);
    near("named preset restores rainbow hue default",cfg.rainbow_hue_offset,0.0f);
    near("named preset restores rainbow spread default",cfg.rainbow_spread,1.0f);
    check("Cyber preset applies authored ornament", nm_config_apply_preset(&cfg, 1) && cfg.ornament_mode == NM_ORNAMENT_CYBER && cfg.art_intensity > 0.8f);
    check("preset restores border", cfg.show_border);
    check("preset restores glow", cfg.show_glow);
    cfg.safe_fit = true;
    check("presets preserve opt-in safe-fit", nm_config_apply_preset(&cfg, 2) && cfg.safe_fit);
    cfg.expand_canvas = true;
    check("presets preserve expanded-output selection", nm_config_apply_preset(&cfg, 2) && cfg.expand_canvas);
    check("Streamer selects actual bubble", nm_config_apply_preset(&cfg, 6) && cfg.shape_id == NM_SHAPE_CHAT_BUBBLE);
    check("Angled preset selects diagonal mask", nm_config_apply_preset(&cfg, 7) && cfg.shape_id == NM_SHAPE_ANGLED_CARD);

    check("HUD preset chooses new shape", nm_config_apply_preset(&cfg, 8) && cfg.shape_id == NM_SHAPE_HUD_PANEL);
    check("Squircle preset chooses new shape", nm_config_apply_preset(&cfg, 9) && cfg.shape_id == NM_SHAPE_SQUIRCLE);
    check("D4F Tech preset exposes bold independent presence controls",
          nm_config_apply_preset(&cfg,5) &&
          fabsf(cfg.border_px-5.0f)<0.0001f &&
          fabsf(cfg.art_gap-24.0f)<0.0001f &&
          fabsf(cfg.ornament_width_px-18.0f)<0.0001f &&
          fabsf(cfg.ornament_length_x_px-84.0f)<0.0001f &&
          fabsf(cfg.ornament_length_y_px-68.0f)<0.0001f);
    check("D4F Game preset exposes bold independent presence controls",
          nm_config_apply_preset(&cfg,10) &&
          fabsf(cfg.border_px-5.0f)<0.0001f &&
          fabsf(cfg.art_gap-14.0f)<0.0001f &&
          fabsf(cfg.ornament_width_px-16.0f)<0.0001f &&
          fabsf(cfg.inner_rail_width_px-1.4f)<0.0001f);
    check("P6A Gradient Rainbow preset selects first-class rainbow recipe",
          nm_config_apply_preset(&cfg,11) &&
          cfg.shape_id==NM_SHAPE_ROUNDED &&
          cfg.color_mode==NM_COLOR_RAINBOW &&
          cfg.animation_id==NM_ANIM_FLOW &&
          fabsf(cfg.rainbow_speed-0.78f)<0.0001f &&
          fabsf(cfg.rainbow_saturation-0.96f)<0.0001f &&
          fabsf(cfg.rainbow_spread-1.0f)<0.0001f &&
          fabsf(cfg.hotspot_strength)<0.0001f);
    check("P6A applying legacy preset after Rainbow restores Dual recipe",
          nm_config_apply_preset(&cfg,2) &&
          cfg.color_mode==NM_COLOR_DUAL &&
          fabsf(cfg.rainbow_speed-0.65f)<0.0001f &&
          fabsf(cfg.rainbow_saturation-0.92f)<0.0001f);
    check("P6E Rotating Ring preset selects decoupled premium orbit state",
          nm_config_apply_preset(&cfg,12) &&
          cfg.shape_id==NM_SHAPE_CIRCLE &&
          cfg.ornament_mode==NM_ORNAMENT_DUAL_RING &&
          cfg.animation_id==NM_ANIM_FLOW &&
          fabsf(cfg.border_px-0.8f)<0.0001f &&
          fabsf(cfg.ornament_width_px-20.0f)<0.0001f &&
          fabsf(cfg.inner_rail_width_px-15.0f)<0.0001f &&
          fabsf(cfg.ring_inner_offset_px-18.0f)<0.0001f &&
          fabsf(cfg.ring_spacing_px-30.0f)<0.0001f &&
          fabsf(cfg.glow_px-22.0f)<0.0001f &&
          fabsf(cfg.glow_amount-0.90f)<0.0001f &&
          fabsf(cfg.animation_speed-0.58f)<0.0001f);
    cfg.shape_id=NM_SHAPE_CIRCLE;
    cfg.ornament_mode=NM_ORNAMENT_DUAL_RING;
    cfg.ornament_width_px=64.0f;
    cfg.inner_rail_width_px=64.0f;
    cfg.ring_inner_offset_px=999.0f;
    cfg.ring_spacing_px=999.0f;
    nm_config_validate(&cfg);
    check("P6E Dual Ring pathological controls clamp to artistic envelope",
          fabsf(cfg.ornament_width_px-26.0f)<0.0001f &&
          fabsf(cfg.inner_rail_width_px-20.0f)<0.0001f &&
          fabsf(cfg.ring_inner_offset_px-30.0f)<0.0001f &&
          fabsf(cfg.ring_spacing_px-44.0f)<0.0001f);
    check("legacy schema supported", nm_config_schema_supported(0));
    check("current schema supported", nm_config_schema_supported(NM_CONFIG_SCHEMA_VERSION));
    check("legacy v1 schema supported", nm_config_schema_supported(1));
    check("future schema rejected", !nm_config_schema_supported(NM_CONFIG_SCHEMA_VERSION + 1));
    check("D4A freeform migration advances schema to v3",
          NM_CONFIG_SCHEMA_VERSION == 3u);

    check("unknown preset rejected", !nm_config_apply_preset(&cfg, 99));
    check("null preset target rejected", !nm_config_apply_preset(NULL, 1));

    nm_config_defaults(&cfg);
    near("D4A TL X default",cfg.bubble_tl_x,-1.0f);
    near("D4A TL Y default",cfg.bubble_tl_y,-1.0f);
    near("D4A TR X default",cfg.bubble_tr_x,1.0f);
    near("D4A TR Y default",cfg.bubble_tr_y,-1.0f);
    near("D4A BR X default",cfg.bubble_br_x,1.0f);
    near("D4A BL X default",cfg.bubble_bl_x,-1.0f);
    near("D4A bottom default",cfg.bubble_br_y,1.0f-1.8f*cfg.shape_detail);
    near("D4A bottom pair default",cfg.bubble_bl_y,cfg.bubble_br_y);
    near("D4A tail start default",cfg.bubble_tail_start,0.11f);
    near("D4A tail end default",cfg.bubble_tail_end,0.33f);
    near("D4A tail tip-pos default",cfg.bubble_tail_tip_pos,0.14f);
    near("D4A default depth tracks preset shape detail",cfg.bubble_tail_depth,cfg.shape_detail);

    cfg.bubble_tl_x=1.0f; cfg.bubble_tl_y=INFINITY;
    cfg.bubble_tr_x=-1.0f; cfg.bubble_tr_y=NAN;
    cfg.bubble_br_x=-1.0f; cfg.bubble_br_y=99.0f;
    cfg.bubble_bl_x=1.0f; cfg.bubble_bl_y=-99.0f;
    cfg.bubble_tail_start=0.90f; cfg.bubble_tail_end=0.01f;
    cfg.bubble_tail_tip_pos=INFINITY; cfg.bubble_tail_depth=NAN;
    nm_config_validate(&cfg);
    near("D4A TL X quadrant clamp",cfg.bubble_tl_x,-0.35f);
    near("D4A TL Y finite clamp",cfg.bubble_tl_y,-1.0f);
    near("D4A TR X quadrant clamp",cfg.bubble_tr_x,0.35f);
    near("D4A TR Y finite clamp",cfg.bubble_tr_y,-1.0f);
    near("D4A BR X quadrant clamp",cfg.bubble_br_x,0.35f);
    near("D4A BR Y clamp",cfg.bubble_br_y,0.92f);
    near("D4A BL X quadrant clamp",cfg.bubble_bl_x,-0.35f);
    near("D4A BL Y clamp",cfg.bubble_bl_y,0.35f);
    check("D4A tail ordering repaired",
          cfg.bubble_tail_end>=cfg.bubble_tail_start+0.0599f &&
          cfg.bubble_tail_end<=0.98f);
    near("D4A invalid tip position",cfg.bubble_tail_tip_pos,0.0f);
    near("D4A invalid tail depth",cfg.bubble_tail_depth,0.08f);

    cfg.bubble_tl_x=-0.55f; cfg.bubble_tr_y=-0.55f;
    cfg.bubble_br_x=0.52f; cfg.bubble_bl_y=0.44f;
    cfg.bubble_tail_start=0.30f; cfg.bubble_tail_tip_pos=0.65f;
    check("D4A preset operation resets freeform geometry",
          nm_config_apply_preset(&cfg,6) &&
          fabsf(cfg.bubble_tl_x+1.0f)<0.0001f &&
          fabsf(cfg.bubble_tr_y+1.0f)<0.0001f &&
          fabsf(cfg.bubble_br_x-1.0f)<0.0001f &&
          fabsf(cfg.bubble_tail_start-0.11f)<0.0001f &&
          fabsf(cfg.bubble_tail_tip_pos-0.14f)<0.0001f &&
          fabsf(cfg.bubble_tail_depth-cfg.shape_detail)<0.0001f);
    if (failures) return 1;
    puts("PASS: canonical config defaults, validation, complete presets and schema policy");
    return 0;
}
