/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-filter.h"
#include "neonmask-math.h"
#include "neonmask-presets.h"
#include "neonmask-config.h"
#include "neonmask-motion.h"
#include "neonmask-svg.h"
#include "neonmask-safe-fit.h"
#include <string.h>
#include <math.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>
#include <util/bmem.h>

struct nm_filter {
    obs_source_t *context;
    gs_effect_t *effect;
    gs_eparam_t *uv_size;
    gs_eparam_t *output_size;
    gs_eparam_t *input_origin;
    gs_eparam_t *half_size;
    gs_eparam_t *mask_offset;
    gs_eparam_t *subject_pan;
    gs_eparam_t *subject_zoom;
    gs_eparam_t *shape_rotation;
    gs_eparam_t *polygon_sides;
    gs_eparam_t *shape_detail;
    gs_eparam_t *bubble_body;
    gs_eparam_t *bubble_tail;
    gs_eparam_t *corner_radius;
    gs_eparam_t *shape;
    gs_eparam_t *border_width;
    gs_eparam_t *style;
    gs_eparam_t *ornament_mode;
    gs_eparam_t *art_intensity;
    gs_eparam_t *art_gap;
    gs_eparam_t *feather;
    gs_eparam_t *glow_radius;
    gs_eparam_t *glow_strength;
    gs_eparam_t *mid_glow_strength;
    gs_eparam_t *bloom_strength;
    gs_eparam_t *hotspot_strength;
    gs_eparam_t *hotspot_size;
    gs_eparam_t *color_a;
    gs_eparam_t *color_b;
    gs_eparam_t *color_phase;
    gs_eparam_t *pulse_phase;
    gs_eparam_t *flow_phase;
    gs_eparam_t *animation;
    gs_eparam_t *segments;
    gs_eparam_t *border_enabled;
    gs_eparam_t *glow_enabled;
    gs_eparam_t *svg_sdf;
    gs_eparam_t *svg_ready;
    gs_texture_t *svg_texture; /* owned only inside graphics context */
    nm_svg_shape svg_shape;
    bool svg_ok;
    bool svg_dirty;
    uint32_t svg_width,svg_height;
    float svg_mask_w,svg_mask_h;
    nm_config config;
    nm_motion motion;
    bool fit_warning_sent;
    bool padding_reported;
    uint32_t logged_left, logged_top, logged_right, logged_bottom;
};

static const char *nm_get_name(void *unused)
{
    (void)unused;
    return obs_module_text("Filter.Name");
}

/* Settings/reload-only file I/O: reject any invalid resource and retire its
 * previous texture immediately. The normal video render never reads disk. */
static void nm_refresh_svg(struct nm_filter *f)
{
    nm_svg_shape next={0};
    char why[128]={0};
    bool ok=false;
    if(f->config.svg_path[0])
        ok=nm_svg_read_local(f->config.svg_path,&next,why,sizeof(why));
    if(f->config.svg_path[0] && !ok)
        blog(LOG_WARNING,"[NeonMask Studio] SVG rejected (%s): %s",
             f->config.svg_path,why);
    f->svg_shape=next;
    f->svg_ok=ok;
    f->svg_dirty=true;
    f->svg_width=f->svg_height=0;
    obs_enter_graphics();
    if(f->svg_texture) gs_texture_destroy(f->svg_texture);
    f->svg_texture=NULL;
    obs_leave_graphics();
}

static void nm_update(void *data, obs_data_t *settings)
{
    struct nm_filter *f = data;
    /* OBS returns a default value for an unset field. Inspect the explicit
     * user value or a legacy v0 scene is mistaken for schema v1. */
    const uint32_t schema = obs_data_has_user_value(settings, "schema_version")
                                ? (uint32_t)obs_data_get_int(settings, "schema_version")
                                : 0u;

    /* Future scene/import data must not reinterpret known fields. Keep the
     * last validated snapshot rather than partially applying unknown state. */
    if (!nm_config_schema_supported(schema)) {
        blog(LOG_WARNING,
             "[NeonMask Studio] unsupported settings schema %u (current %u); keeping last valid state",
             schema, NM_CONFIG_SCHEMA_VERSION);
        return;
    }

    const float legacy_scale = (float)obs_data_get_double(settings, "scale");
    const bool legacy_framing = schema < 2u;

    nm_config next = {
        .schema_version = NM_CONFIG_SCHEMA_VERSION,
        .scale = legacy_scale,
        .mask_width = legacy_framing ? legacy_scale : (float)obs_data_get_double(settings, "mask_width"),
        .mask_height = legacy_framing ? legacy_scale : (float)obs_data_get_double(settings, "mask_height"),
        .safe_fit = obs_data_get_bool(settings, "safe_fit"),
        .expand_canvas = obs_data_get_bool(settings, "expand_canvas"),
        .mask_x_px = legacy_framing ? 0.0f : (float)obs_data_get_double(settings, "mask_x"),
        .mask_y_px = legacy_framing ? 0.0f : (float)obs_data_get_double(settings, "mask_y"),
        .subject_pan_x_px = legacy_framing ? 0.0f : (float)obs_data_get_double(settings, "subject_pan_x"),
        .subject_pan_y_px = legacy_framing ? 0.0f : (float)obs_data_get_double(settings, "subject_pan_y"),
        .subject_zoom = legacy_framing ? 1.0f : (float)obs_data_get_double(settings, "subject_zoom"),
        .shape_rotation_deg = legacy_framing ? 0.0f : (float)obs_data_get_double(settings, "shape_rotation"),
        .polygon_sides = legacy_framing ? 8 : (int)obs_data_get_int(settings, "polygon_sides"),
        .roundness = (float)obs_data_get_double(settings, "roundness"),
        .shape_detail = (float)obs_data_get_double(settings, "shape_detail"),
        .bubble_left_inset = (float)obs_data_get_double(settings, "bubble_left_inset"),
        .bubble_right_inset = (float)obs_data_get_double(settings, "bubble_right_inset"),
        .bubble_top_inset = (float)obs_data_get_double(settings, "bubble_top_inset"),
        .bubble_bottom_inset = (float)obs_data_get_double(settings, "bubble_bottom_inset"),
        .bubble_tail_left = (float)obs_data_get_double(settings, "bubble_tail_left"),
        .bubble_tail_right = (float)obs_data_get_double(settings, "bubble_tail_right"),
        .bubble_tail_tip = (float)obs_data_get_double(settings, "bubble_tail_tip"),
        .bubble_tail_depth = (float)obs_data_get_double(settings, "bubble_tail_depth"),
        .border_px = (float)obs_data_get_double(settings, "border_width"),
        .feather_px = (float)obs_data_get_double(settings, "feather"),
        .glow_px = (float)obs_data_get_double(settings, "glow_radius"),
        .glow_amount = (float)obs_data_get_double(settings, "glow_strength"),
        .mid_glow_strength = (float)obs_data_get_double(settings, "mid_glow_strength"),
        .bloom_strength = (float)obs_data_get_double(settings, "bloom_strength"),
        .hotspot_strength = (float)obs_data_get_double(settings, "hotspot_strength"),
        .hotspot_size = (float)obs_data_get_double(settings, "hotspot_size"),
        .animation_speed = (float)obs_data_get_double(settings, "speed"),
        .primary = (uint32_t)obs_data_get_int(settings, "primary"),
        .secondary = (uint32_t)obs_data_get_int(settings, "secondary"),
        .shape_id = (int)obs_data_get_int(settings, "shape"),
        .animation_id = (int)obs_data_get_int(settings, "animation"),
        .segment_count = (int)obs_data_get_int(settings, "segments"),
        .style_id = (int)obs_data_get_int(settings, "style"),
        .ornament_mode = (int)obs_data_get_int(settings, "ornament_mode"),
        .art_intensity = (float)obs_data_get_double(settings, "art_intensity"),
        .art_gap = (float)obs_data_get_double(settings, "art_gap"),
        .show_border = obs_data_get_bool(settings, "border_enabled"),
        .show_glow = obs_data_get_bool(settings, "glow_enabled"),
    };
    const char *file=obs_data_get_string(settings,"svg_path");
    const size_t file_length=file?strlen(file):0u;
    if(file_length>=NM_SVG_PATH_MAX){
        blog(LOG_WARNING,"[NeonMask Studio] local SVG path too long; fail closed");
        next.svg_path[0]=0;
    }else if(file_length){
        memcpy(next.svg_path,file,file_length+1);
    }
    /* Older Bubble scenes used shape_detail for their tail. Preserve the
     * exact authored depth until a new bubble depth was explicitly saved. */
    if (!obs_data_has_user_value(settings, "bubble_tail_depth"))
        next.bubble_tail_depth = next.shape_detail;
    nm_config_validate(&next);
    const bool asset_changed=strcmp(next.svg_path,f->config.svg_path)!=0;
    f->config = next;
    f->fit_warning_sent = false;
    f->padding_reported = false;
    if(asset_changed) nm_refresh_svg(f);

    /* Pin additive Phase-B defaults into saved v2 scenes once. Existing
     * explicit user values are never overwritten by a later preset/default
     * change, and Phase-A framing keys remain untouched. */
    if (!obs_data_has_user_value(settings, "mid_glow_strength"))
        obs_data_set_double(settings, "mid_glow_strength", next.mid_glow_strength);
    if (!obs_data_has_user_value(settings, "bloom_strength"))
        obs_data_set_double(settings, "bloom_strength", next.bloom_strength);
    if (!obs_data_has_user_value(settings, "hotspot_strength"))
        obs_data_set_double(settings, "hotspot_strength", next.hotspot_strength);
    if (!obs_data_has_user_value(settings, "hotspot_size"))
        obs_data_set_double(settings, "hotspot_size", next.hotspot_size);

    /* D1 additive setting. Pin the resolved default without changing the
     * current shape, subject pan/zoom, or any saved preset identity. */
    if (!obs_data_has_user_value(settings, "shape_detail"))
        obs_data_set_double(settings, "shape_detail", next.shape_detail);
    if (!obs_data_has_user_value(settings, "bubble_left_inset"))
        obs_data_set_double(settings, "bubble_left_inset", next.bubble_left_inset);
    if (!obs_data_has_user_value(settings, "bubble_right_inset"))
        obs_data_set_double(settings, "bubble_right_inset", next.bubble_right_inset);
    if (!obs_data_has_user_value(settings, "bubble_top_inset"))
        obs_data_set_double(settings, "bubble_top_inset", next.bubble_top_inset);
    if (!obs_data_has_user_value(settings, "bubble_bottom_inset"))
        obs_data_set_double(settings, "bubble_bottom_inset", next.bubble_bottom_inset);
    if (!obs_data_has_user_value(settings, "bubble_tail_left"))
        obs_data_set_double(settings, "bubble_tail_left", next.bubble_tail_left);
    if (!obs_data_has_user_value(settings, "bubble_tail_right"))
        obs_data_set_double(settings, "bubble_tail_right", next.bubble_tail_right);
    if (!obs_data_has_user_value(settings, "bubble_tail_tip"))
        obs_data_set_double(settings, "bubble_tail_tip", next.bubble_tail_tip);
    if (!obs_data_has_user_value(settings, "bubble_tail_depth"))
        obs_data_set_double(settings, "bubble_tail_depth", next.bubble_tail_depth);

    /* Existing scenes had no authored ornaments. Pin the old appearance
     * explicitly instead of silently adopting a new default in a future build. */
    if (!obs_data_has_user_value(settings, "ornament_mode"))
        obs_data_set_int(settings, "ornament_mode", next.ornament_mode);
    if (!obs_data_has_user_value(settings, "art_intensity"))
        obs_data_set_double(settings, "art_intensity", next.art_intensity);
    if (!obs_data_has_user_value(settings, "art_gap"))
        obs_data_set_double(settings, "art_gap", next.art_gap);

    /* v0/v1 used one uniform scale. Migrate once to independent dimensions
     * while keeping all old keys and enum values intact. */
    if (legacy_framing) {
        obs_data_set_double(settings, "mask_width", next.mask_width);
        obs_data_set_double(settings, "mask_height", next.mask_height);
        obs_data_set_double(settings, "mask_x", 0.0);
        obs_data_set_double(settings, "mask_y", 0.0);
        obs_data_set_double(settings, "subject_pan_x", 0.0);
        obs_data_set_double(settings, "subject_pan_y", 0.0);
        obs_data_set_double(settings, "subject_zoom", 1.0);
        obs_data_set_double(settings, "shape_rotation", 0.0);
        obs_data_set_int(settings, "polygon_sides", 8);
        obs_data_set_int(settings, "schema_version", NM_CONFIG_SCHEMA_VERSION);
    }
}

static void nm_defaults(obs_data_t *settings)
{
    nm_config cfg;
    nm_config_defaults(&cfg);

    obs_data_set_default_int(settings, "schema_version", NM_CONFIG_SCHEMA_VERSION);
    obs_data_set_default_int(settings, "preset", 0);
    obs_data_set_default_int(settings, "shape", cfg.shape_id);
    obs_data_set_default_string(settings, "svg_path", "");
    obs_data_set_default_double(settings, "scale", cfg.scale);
    obs_data_set_default_double(settings, "mask_width", cfg.mask_width);
    obs_data_set_default_double(settings, "mask_height", cfg.mask_height);
    obs_data_set_default_bool(settings, "safe_fit", cfg.safe_fit);
    obs_data_set_default_bool(settings, "expand_canvas", cfg.expand_canvas);
    obs_data_set_default_double(settings, "mask_x", cfg.mask_x_px);
    obs_data_set_default_double(settings, "mask_y", cfg.mask_y_px);
    obs_data_set_default_double(settings, "subject_pan_x", cfg.subject_pan_x_px);
    obs_data_set_default_double(settings, "subject_pan_y", cfg.subject_pan_y_px);
    obs_data_set_default_double(settings, "subject_zoom", cfg.subject_zoom);
    obs_data_set_default_double(settings, "shape_rotation", cfg.shape_rotation_deg);
    obs_data_set_default_int(settings, "polygon_sides", cfg.polygon_sides);
    obs_data_set_default_double(settings, "roundness", cfg.roundness);
    obs_data_set_default_double(settings, "shape_detail", cfg.shape_detail);
    obs_data_set_default_double(settings, "bubble_left_inset", cfg.bubble_left_inset);
    obs_data_set_default_double(settings, "bubble_right_inset", cfg.bubble_right_inset);
    obs_data_set_default_double(settings, "bubble_top_inset", cfg.bubble_top_inset);
    obs_data_set_default_double(settings, "bubble_bottom_inset", cfg.bubble_bottom_inset);
    obs_data_set_default_double(settings, "bubble_tail_left", cfg.bubble_tail_left);
    obs_data_set_default_double(settings, "bubble_tail_right", cfg.bubble_tail_right);
    obs_data_set_default_double(settings, "bubble_tail_tip", cfg.bubble_tail_tip);
    obs_data_set_default_double(settings, "bubble_tail_depth", cfg.bubble_tail_depth);
    obs_data_set_default_double(settings, "border_width", cfg.border_px);
    obs_data_set_default_double(settings, "feather", cfg.feather_px);
    obs_data_set_default_double(settings, "glow_radius", cfg.glow_px);
    obs_data_set_default_double(settings, "glow_strength", cfg.glow_amount);
    obs_data_set_default_double(settings, "mid_glow_strength", cfg.mid_glow_strength);
    obs_data_set_default_double(settings, "bloom_strength", cfg.bloom_strength);
    obs_data_set_default_double(settings, "hotspot_strength", cfg.hotspot_strength);
    obs_data_set_default_double(settings, "hotspot_size", cfg.hotspot_size);
    obs_data_set_default_int(settings, "primary", cfg.primary & 0x00FFFFFFu);
    obs_data_set_default_int(settings, "secondary", cfg.secondary & 0x00FFFFFFu);
    obs_data_set_default_int(settings, "animation", cfg.animation_id);
    obs_data_set_default_double(settings, "speed", cfg.animation_speed);
    obs_data_set_default_int(settings, "segments", cfg.segment_count);
    obs_data_set_default_int(settings, "style", cfg.style_id);
    obs_data_set_default_int(settings, "ornament_mode", cfg.ornament_mode);
    obs_data_set_default_double(settings, "art_intensity", cfg.art_intensity);
    obs_data_set_default_double(settings, "art_gap", cfg.art_gap);
    obs_data_set_default_bool(settings, "border_enabled", cfg.show_border);
    obs_data_set_default_bool(settings, "glow_enabled", cfg.show_glow);
}

/* Shape-specific controls are intentionally sparse: Squircle/HUD do not
 * respond to the Roundness slider, and generic shapes do not respond to
 * the authored Shape Detail control. Never reset saved user values here. */
static void nm_geometry_visibility(obs_properties_t *props, int shape_id)
{
    obs_property_t *group=obs_properties_get(props, "mask_geometry");
    obs_properties_t *geometry=group ? obs_property_group_content(group) : NULL;
    if(!geometry) return;
    obs_property_t *round=obs_properties_get(geometry,"roundness");
    obs_property_t *detail=obs_properties_get(geometry,"shape_detail");
    obs_property_t *sides=obs_properties_get(geometry,"polygon_sides");
    obs_property_t *svg=obs_properties_get(geometry,"svg_path");
    obs_property_t *reload=obs_properties_get(geometry,"svg_reload");
    obs_property_t *body=obs_properties_get(geometry,"bubble_body_controls");
    obs_property_t *tail=obs_properties_get(geometry,"bubble_tail_controls");
    const bool rounded=shape_id==NM_SHAPE_ROUNDED ||
                       shape_id==NM_SHAPE_CHAT_BUBBLE ||
                       shape_id==NM_SHAPE_ANGLED_CARD;
    const bool authored=shape_id==NM_SHAPE_ANGLED_CARD ||
                        shape_id==NM_SHAPE_HUD_PANEL ||
                        shape_id==NM_SHAPE_SQUIRCLE;
    if(round) obs_property_set_visible(round,rounded);
    if(detail) obs_property_set_visible(detail,authored);
    if(sides) obs_property_set_visible(sides,shape_id==NM_SHAPE_POLYGON);
    if(svg) obs_property_set_visible(svg,shape_id==NM_SHAPE_SVG_PATH);
    if(reload) obs_property_set_visible(reload,shape_id==NM_SHAPE_SVG_PATH);
    if(body) obs_property_set_visible(body,shape_id==NM_SHAPE_CHAT_BUBBLE);
    if(tail) obs_property_set_visible(tail,shape_id==NM_SHAPE_CHAT_BUBBLE);
}

/* One visibility resolver for preset, Shape, Animation, Ornament and
 * on/off controls. Hidden values are retained in OBS settings, never reset. */
static void nm_context_visibility(obs_properties_t *props, int shape_id,
                                  int animation, int ornament, bool border,
                                  bool glow, bool expanded)
{
    nm_geometry_visibility(props,shape_id);
    obs_property_t *p;
#define NM_SHOW(key, yes) do { p=obs_properties_get(props,key); \
                              if(p) obs_property_set_visible(p,yes); } while(0)
    NM_SHOW("border_width",border);
    NM_SHOW("style",border);
    NM_SHOW("primary",border);
    NM_SHOW("secondary",border);
    NM_SHOW("signature_art",border);
    NM_SHOW("glow_enabled",border);
    NM_SHOW("glow_radius",border && glow);
    NM_SHOW("glow_strength",border && glow);
    NM_SHOW("premium_lighting",border && (glow || animation==NM_ANIM_FLOW));
    NM_SHOW("speed",border && animation!=NM_ANIM_STATIC);
    NM_SHOW("segments",border);
#undef NM_SHOW
    obs_property_t *art=obs_properties_get(props,"signature_art");
    obs_properties_t *arts=art?obs_property_group_content(art):NULL;
    if(arts) {
        p=obs_properties_get(arts,"art_intensity");
        if(p) obs_property_set_visible(p,ornament!=NM_ORNAMENT_NONE);
        p=obs_properties_get(arts,"art_gap");
        if(p) obs_property_set_visible(p,ornament!=NM_ORNAMENT_NONE);
    }
    obs_property_t *light=obs_properties_get(props,"premium_lighting");
    obs_properties_t *lights=light?obs_property_group_content(light):NULL;
    if(lights) {
        const char *const glow_keys[]={"mid_glow_strength","bloom_strength"};
        for(size_t i=0;i<2;++i) {
            p=obs_properties_get(lights,glow_keys[i]);
            if(p) obs_property_set_visible(p,glow);
        }
        const char *const flow_keys[]={"hotspot_strength","hotspot_size"};
        for(size_t i=0;i<2;++i) {
            p=obs_properties_get(lights,flow_keys[i]);
            if(p) obs_property_set_visible(p,animation==NM_ANIM_FLOW);
        }
    }
    obs_property_t *mask=obs_properties_get(props,"mask_geometry");
    obs_properties_t *geometry=mask?obs_property_group_content(mask):NULL;
    if(geometry) {
        p=obs_properties_get(geometry,"safe_fit");
        if(p) obs_property_set_visible(p,!expanded);
    }
}

/* Apply preset as real user-editable property values: no hidden runtime overrides. */
static bool nm_preset_changed(obs_properties_t *props, obs_property_t *property,
                              obs_data_t *settings)
{
    (void)property;

    nm_config cfg;
    nm_config_defaults(&cfg);
    if (!nm_config_apply_preset(&cfg, (int)obs_data_get_int(settings, "preset")))
        return false;

    obs_data_set_int(settings, "schema_version", NM_CONFIG_SCHEMA_VERSION);
    obs_data_set_int(settings, "shape", cfg.shape_id);
    obs_data_set_int(settings, "animation", cfg.animation_id);
    obs_data_set_int(settings, "primary", cfg.primary & 0x00FFFFFFu);
    obs_data_set_int(settings, "secondary", cfg.secondary & 0x00FFFFFFu);
    obs_data_set_double(settings, "scale", cfg.scale);
    obs_data_set_double(settings, "mask_width", cfg.scale);
    obs_data_set_double(settings, "mask_height", cfg.scale);
    obs_data_set_double(settings, "roundness", cfg.roundness);
    obs_data_set_double(settings, "shape_detail", cfg.shape_detail);
    obs_data_set_double(settings, "bubble_left_inset", cfg.bubble_left_inset);
    obs_data_set_double(settings, "bubble_right_inset", cfg.bubble_right_inset);
    obs_data_set_double(settings, "bubble_top_inset", cfg.bubble_top_inset);
    obs_data_set_double(settings, "bubble_bottom_inset", cfg.bubble_bottom_inset);
    obs_data_set_double(settings, "bubble_tail_left", cfg.bubble_tail_left);
    obs_data_set_double(settings, "bubble_tail_right", cfg.bubble_tail_right);
    obs_data_set_double(settings, "bubble_tail_tip", cfg.bubble_tail_tip);
    obs_data_set_double(settings, "bubble_tail_depth", cfg.bubble_tail_depth);
    obs_data_set_double(settings, "border_width", cfg.border_px);
    obs_data_set_double(settings, "feather", cfg.feather_px);
    obs_data_set_double(settings, "glow_radius", cfg.glow_px);
    obs_data_set_double(settings, "glow_strength", cfg.glow_amount);
    obs_data_set_double(settings, "mid_glow_strength", cfg.mid_glow_strength);
    obs_data_set_double(settings, "bloom_strength", cfg.bloom_strength);
    obs_data_set_double(settings, "hotspot_strength", cfg.hotspot_strength);
    obs_data_set_double(settings, "hotspot_size", cfg.hotspot_size);
    obs_data_set_double(settings, "speed", cfg.animation_speed);
    obs_data_set_int(settings, "segments", cfg.segment_count);
    obs_data_set_int(settings, "style", cfg.style_id);
    obs_data_set_int(settings, "ornament_mode", cfg.ornament_mode);
    obs_data_set_double(settings, "art_intensity", cfg.art_intensity);
    obs_data_set_double(settings, "art_gap", cfg.art_gap);
    obs_data_set_bool(settings, "border_enabled", cfg.show_border);
    obs_data_set_bool(settings, "glow_enabled", cfg.show_glow);
    nm_context_visibility(props,cfg.shape_id,cfg.animation_id,cfg.ornament_mode,
                          cfg.show_border,cfg.show_glow,cfg.expand_canvas);
    return true;
}

/* A preset is an operation, not a lock on the editable controls. */
static bool nm_custom_changed(obs_properties_t *props, obs_property_t *property,
                              obs_data_t *settings)
{
    (void)props; (void)property;
    if (obs_data_get_int(settings, "preset") == 0) return false;
    obs_data_set_int(settings, "preset", 0);
    return true;
}

static bool nm_context_changed(obs_properties_t *props, obs_property_t *property,
                               obs_data_t *settings)
{
    nm_custom_changed(props,property,settings);
    nm_context_visibility(props,(int)obs_data_get_int(settings,"shape"),
                          (int)obs_data_get_int(settings,"animation"),
                          (int)obs_data_get_int(settings,"ornament_mode"),
                          obs_data_get_bool(settings,"border_enabled"),
                          obs_data_get_bool(settings,"glow_enabled"),
                          obs_data_get_bool(settings,"expand_canvas"));
    return true;
}

static bool nm_shape_changed(obs_properties_t *props, obs_property_t *property,
                             obs_data_t *settings)
{
    return nm_context_changed(props,property,settings);
}

static bool nm_reload_svg(obs_properties_t *props,obs_property_t *property,
                          void *data)
{
    (void)props; (void)property;
    struct nm_filter *f=data;
    if(!f) return false;
    nm_refresh_svg(f);
    return true;
}

static bool nm_reset_framing(obs_properties_t *props, obs_property_t *property, void *data)
{
    (void)props;
    (void)property;
    struct nm_filter *f = data;
    if (!f || !f->context) return false;

    obs_data_t *settings = obs_source_get_settings(f->context);
    if (!settings) return false;
    obs_data_set_double(settings, "mask_x", 0.0);
    obs_data_set_double(settings, "mask_y", 0.0);
    obs_data_set_double(settings, "subject_pan_x", 0.0);
    obs_data_set_double(settings, "subject_pan_y", 0.0);
    obs_data_set_double(settings, "subject_zoom", 1.0);
    obs_source_update(f->context, settings);
    obs_data_release(settings);
    return true;
}

static obs_properties_t *nm_properties(void *data)
{
    (void)data;
    obs_properties_t *props = obs_properties_create();
    obs_property_t *preset = obs_properties_add_list(props, "preset", obs_module_text("Preset"),
                                                      OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(preset, obs_module_text("Preset.Custom"), 0);
    obs_property_list_add_int(preset, obs_module_text("Preset.Cyber"), 1);
    obs_property_list_add_int(preset, obs_module_text("Preset.Reactor"), 2);
    obs_property_list_add_int(preset, obs_module_text("Preset.Emerald"), 3);
    obs_property_list_add_int(preset, obs_module_text("Preset.Ember"), 4);
    obs_property_list_add_int(preset, obs_module_text("Preset.TechHUD"), 5);
    obs_property_list_add_int(preset, obs_module_text("Preset.Streamer"), 6);
    obs_property_list_add_int(preset, obs_module_text("Preset.AngledCard"), 7);
    obs_property_list_add_int(preset, obs_module_text("Preset.HUDCut"), 8);
    obs_property_list_add_int(preset, obs_module_text("Preset.Squircle"), 9);
    obs_property_set_modified_callback(preset, nm_preset_changed);

    obs_property_t *shape = obs_properties_add_list(props, "shape", obs_module_text("Shape"),
                                                     OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(shape, obs_module_text("Shape.Rounded"), NM_SHAPE_ROUNDED);
    obs_property_list_add_int(shape, obs_module_text("Shape.Circle"), NM_SHAPE_CIRCLE);
    obs_property_list_add_int(shape, obs_module_text("Shape.Ellipse"), NM_SHAPE_ELLIPSE);
    obs_property_list_add_int(shape, obs_module_text("Shape.Hexagon"), NM_SHAPE_HEXAGON);
    obs_property_list_add_int(shape, obs_module_text("Shape.Diamond"), NM_SHAPE_DIAMOND);
    obs_property_list_add_int(shape, obs_module_text("Shape.Rectangle"), NM_SHAPE_RECTANGLE);
    obs_property_list_add_int(shape, obs_module_text("Shape.Triangle"), NM_SHAPE_TRIANGLE);
    obs_property_list_add_int(shape, obs_module_text("Shape.Polygon"), NM_SHAPE_POLYGON);
    obs_property_list_add_int(shape, obs_module_text("Shape.ChatBubble"), NM_SHAPE_CHAT_BUBBLE);
    obs_property_list_add_int(shape, obs_module_text("Shape.AngledCard"), NM_SHAPE_ANGLED_CARD);
    obs_property_list_add_int(shape, obs_module_text("Shape.HUDPanel"), NM_SHAPE_HUD_PANEL);
    obs_property_list_add_int(shape, obs_module_text("Shape.Squircle"), NM_SHAPE_SQUIRCLE);
    obs_property_list_add_int(shape, obs_module_text("Shape.SVG"), NM_SHAPE_SVG_PATH);
    obs_property_set_modified_callback(shape, nm_shape_changed);

#define NM_CUSTOM(expr) obs_property_set_modified_callback((expr), nm_custom_changed)
    obs_properties_t *mask_group = obs_properties_create();
    NM_CUSTOM(obs_properties_add_float_slider(mask_group, "mask_width", obs_module_text("Mask.Width"), 0.10, 0.98, 0.01));
    NM_CUSTOM(obs_properties_add_float_slider(mask_group, "mask_height", obs_module_text("Mask.Height"), 0.10, 0.98, 0.01));
    NM_CUSTOM(obs_properties_add_bool(mask_group, "safe_fit", obs_module_text("Mask.SafeFit")));
    obs_property_t *expand=obs_properties_add_bool(mask_group,"expand_canvas",obs_module_text("Mask.ExpandCanvas"));
    obs_property_set_modified_callback(expand,nm_context_changed);
    NM_CUSTOM(obs_properties_add_float_slider(mask_group, "shape_detail", obs_module_text("Mask.Detail"), 0.08, 0.35, 0.01));
    obs_properties_t *bubble_body_props=obs_properties_create();
    NM_CUSTOM(obs_properties_add_float_slider(bubble_body_props,"bubble_left_inset",obs_module_text("Bubble.InsetLeft"),0.0,0.25,0.01));
    NM_CUSTOM(obs_properties_add_float_slider(bubble_body_props,"bubble_right_inset",obs_module_text("Bubble.InsetRight"),0.0,0.25,0.01));
    NM_CUSTOM(obs_properties_add_float_slider(bubble_body_props,"bubble_top_inset",obs_module_text("Bubble.InsetTop"),0.0,0.25,0.01));
    NM_CUSTOM(obs_properties_add_float_slider(bubble_body_props,"bubble_bottom_inset",obs_module_text("Bubble.InsetBottom"),0.0,0.25,0.01));
    obs_properties_add_group(mask_group,"bubble_body_controls",obs_module_text("Bubble.Body"),OBS_GROUP_NORMAL,bubble_body_props);
    obs_properties_t *bubble_tail_props=obs_properties_create();
    NM_CUSTOM(obs_properties_add_float_slider(bubble_tail_props,"bubble_tail_left",obs_module_text("Bubble.TailLeft"),-0.90,0.80,0.01));
    NM_CUSTOM(obs_properties_add_float_slider(bubble_tail_props,"bubble_tail_right",obs_module_text("Bubble.TailRight"),-0.80,0.90,0.01));
    NM_CUSTOM(obs_properties_add_float_slider(bubble_tail_props,"bubble_tail_tip",obs_module_text("Bubble.TailTip"),-0.95,0.95,0.01));
    NM_CUSTOM(obs_properties_add_float_slider(bubble_tail_props,"bubble_tail_depth",obs_module_text("Bubble.TailDepth"),0.08,0.35,0.01));
    obs_properties_add_group(mask_group,"bubble_tail_controls",obs_module_text("Bubble.Tail"),OBS_GROUP_NORMAL,bubble_tail_props);
    /* One persisted slider: rounded-box corners, Bubble tail-tip curvature,
     * and ONLY the Angled Card diagonal endpoints (square corners stay sharp).
     * Shape-neutral label avoids misleading Bubble users. */
    NM_CUSTOM(obs_properties_add_float_slider(mask_group, "roundness", obs_module_text("Roundness"), 0.0, 1.0, 0.01));
    NM_CUSTOM(obs_properties_add_float_slider(mask_group, "mask_x", obs_module_text("Mask.PositionX"), -4096.0, 4096.0, 1.0));
    NM_CUSTOM(obs_properties_add_float_slider(mask_group, "mask_y", obs_module_text("Mask.PositionY"), -4096.0, 4096.0, 1.0));
    NM_CUSTOM(obs_properties_add_float_slider(mask_group, "shape_rotation", obs_module_text("Mask.Rotation"), -180.0, 180.0, 1.0));
    NM_CUSTOM(obs_properties_add_int_slider(mask_group, "polygon_sides", obs_module_text("Mask.PolygonSides"), 5, 12, 1));
    NM_CUSTOM(obs_properties_add_path(mask_group, "svg_path", obs_module_text("SVG.Path"),
                                      OBS_PATH_FILE, "SVG files (*.svg)", NULL));
    obs_properties_add_button2(mask_group, "svg_reload", obs_module_text("SVG.Reload"),
                               nm_reload_svg,data);
    obs_properties_add_group(props, "mask_geometry", obs_module_text("Group.MaskGeometry"), OBS_GROUP_NORMAL, mask_group);

    obs_properties_t *subject_group = obs_properties_create();
    NM_CUSTOM(obs_properties_add_float_slider(subject_group, "subject_pan_x", obs_module_text("Subject.PanX"), -4096.0, 4096.0, 1.0));
    NM_CUSTOM(obs_properties_add_float_slider(subject_group, "subject_pan_y", obs_module_text("Subject.PanY"), -4096.0, 4096.0, 1.0));
    NM_CUSTOM(obs_properties_add_float_slider(subject_group, "subject_zoom", obs_module_text("Subject.Zoom"), 0.25, 4.0, 0.01));
    obs_properties_add_button2(subject_group, "reset_framing", obs_module_text("Subject.ResetFraming"), nm_reset_framing, data);
    obs_properties_add_group(props, "subject_framing", obs_module_text("Group.SubjectFraming"), OBS_GROUP_NORMAL, subject_group);


    NM_CUSTOM(obs_properties_add_float_slider(props, "feather", obs_module_text("Feather"), 0.5, 30.0, 0.5));
    obs_property_t *border_switch=obs_properties_add_bool(props,"border_enabled",obs_module_text("Border.Enabled"));
    obs_property_set_modified_callback(border_switch,nm_context_changed);
    NM_CUSTOM(obs_properties_add_float_slider(props, "border_width", obs_module_text("Border.Width"), 0.5, 32.0, 0.5));
    obs_property_t *style = obs_properties_add_list(props, "style", obs_module_text("Border.Style"),
                                                    OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(style, obs_module_text("Border.Style.Classic"), NM_STYLE_CLASSIC);
    obs_property_list_add_int(style, obs_module_text("Border.Style.Double"), NM_STYLE_DOUBLE);
    obs_property_list_add_int(style, obs_module_text("Border.Style.Hud"), NM_STYLE_HUD);
    obs_property_list_add_int(style, obs_module_text("Border.Style.Minimal"), NM_STYLE_MINIMAL);
    obs_property_set_modified_callback(style, nm_custom_changed);
    obs_properties_t *art_group = obs_properties_create();
    obs_property_t *ornament = obs_properties_add_list(art_group, "ornament_mode",
                         obs_module_text("Art.Mode"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(ornament, obs_module_text("Art.None"), NM_ORNAMENT_NONE);
    obs_property_list_add_int(ornament, obs_module_text("Art.Cyber"), NM_ORNAMENT_CYBER);
    obs_property_list_add_int(ornament, obs_module_text("Art.Reactor"), NM_ORNAMENT_REACTOR);
    obs_property_list_add_int(ornament, obs_module_text("Art.TechHUD"), NM_ORNAMENT_TECH_HUD);
    obs_property_list_add_int(ornament, obs_module_text("Art.Streamer"), NM_ORNAMENT_STREAMER);
    obs_property_set_modified_callback(ornament, nm_context_changed);
    NM_CUSTOM(obs_properties_add_float_slider(art_group, "art_intensity",
                         obs_module_text("Art.Intensity"), 0.0, 1.0, 0.01));
    NM_CUSTOM(obs_properties_add_float_slider(art_group, "art_gap",
                         obs_module_text("Art.Gap"), 1.0, 16.0, 0.5));
    obs_properties_add_group(props, "signature_art", obs_module_text("Art.Group"),
                             OBS_GROUP_NORMAL, art_group);
    NM_CUSTOM(obs_properties_add_color(props, "primary", obs_module_text("Color.Primary")));
    NM_CUSTOM(obs_properties_add_color(props, "secondary", obs_module_text("Color.Secondary")));
    obs_property_t *glow_switch=obs_properties_add_bool(props,"glow_enabled",obs_module_text("Glow.Enabled"));
    obs_property_set_modified_callback(glow_switch,nm_context_changed);
    NM_CUSTOM(obs_properties_add_float_slider(props, "glow_radius", obs_module_text("Glow.Radius"), 1.0, 80.0, 1.0));
    NM_CUSTOM(obs_properties_add_float_slider(props, "glow_strength", obs_module_text("Glow.Strength"), 0.0, 1.0, 0.01));
    obs_properties_t *light_group = obs_properties_create();
    NM_CUSTOM(obs_properties_add_float_slider(light_group, "mid_glow_strength", obs_module_text("Glow.MidStrength"), 0.0, 1.0, 0.01));
    NM_CUSTOM(obs_properties_add_float_slider(light_group, "bloom_strength", obs_module_text("Glow.BloomStrength"), 0.0, 1.0, 0.01));
    NM_CUSTOM(obs_properties_add_float_slider(light_group, "hotspot_strength", obs_module_text("Glow.HotspotStrength"), 0.0, 1.0, 0.01));
    NM_CUSTOM(obs_properties_add_float_slider(light_group, "hotspot_size", obs_module_text("Glow.HotspotSize"), 0.04, 0.25, 0.01));
    obs_properties_add_group(props, "premium_lighting", obs_module_text("Group.PremiumLighting"), OBS_GROUP_NORMAL, light_group);

    obs_property_t *animation = obs_properties_add_list(props, "animation", obs_module_text("Animation"),
                                                         OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(animation, obs_module_text("Animation.Static"), NM_ANIM_STATIC);
    obs_property_list_add_int(animation, obs_module_text("Animation.Pulse"), NM_ANIM_PULSE);
    obs_property_list_add_int(animation, obs_module_text("Animation.Flow"), NM_ANIM_FLOW);
    obs_property_set_modified_callback(animation, nm_context_changed);
    NM_CUSTOM(obs_properties_add_float_slider(props, "speed", obs_module_text("Speed"), 0.0, 5.0, 0.05));
    NM_CUSTOM(obs_properties_add_int_slider(props, "segments", obs_module_text("Segments"), 0, 48, 1));
#undef NM_CUSTOM
    struct nm_filter *current = data;
    nm_config initial;
    if(!current) nm_config_defaults(&initial);
    const nm_config *cfg=current?&current->config:&initial;
    nm_context_visibility(props,cfg->shape_id,cfg->animation_id,cfg->ornament_mode,
                          cfg->show_border,cfg->show_glow,cfg->expand_canvas);
    return props;
}

static void *nm_create(obs_data_t *settings, obs_source_t *context)
{
    struct nm_filter *f = bzalloc(sizeof(*f));
    f->context = context;
    nm_config_defaults(&f->config);
    char *path = obs_module_file("shaders/neon-mask.effect");
    char *error = NULL;
    obs_enter_graphics();
    if (path) f->effect = gs_effect_create_from_file(path, &error);
    if (f->effect) {
        bool valid = true;
#define NM_PARAM(field, name) do { \
        f->field = gs_effect_get_param_by_name(f->effect, name); \
        if (!f->field) { \
            blog(LOG_ERROR, "[NeonMask Studio] missing uniform: %s", name); \
            valid = false; \
        } \
    } while (0)
        NM_PARAM(uv_size, "uv_size");
        NM_PARAM(output_size, "output_size");
        NM_PARAM(input_origin, "input_origin");
        NM_PARAM(half_size, "half_size");
        NM_PARAM(mask_offset, "mask_offset");
        NM_PARAM(subject_pan, "subject_pan");
        NM_PARAM(subject_zoom, "subject_zoom");
        NM_PARAM(shape_rotation, "shape_rotation");
        NM_PARAM(polygon_sides, "polygon_sides");
        NM_PARAM(shape_detail, "shape_detail");
        NM_PARAM(bubble_body, "bubble_body");
        NM_PARAM(bubble_tail, "bubble_tail");
        NM_PARAM(corner_radius, "corner_radius");
        NM_PARAM(shape, "shape_id");
        NM_PARAM(border_width, "border_width");
        NM_PARAM(style, "style_id");
        NM_PARAM(ornament_mode, "ornament_mode");
        NM_PARAM(art_intensity, "art_intensity");
        NM_PARAM(art_gap, "art_gap");
        NM_PARAM(feather, "feather");
        NM_PARAM(glow_radius, "glow_radius");
        NM_PARAM(glow_strength, "glow_strength");
        NM_PARAM(mid_glow_strength, "mid_glow_strength");
        NM_PARAM(bloom_strength, "bloom_strength");
        NM_PARAM(hotspot_strength, "hotspot_strength");
        NM_PARAM(hotspot_size, "hotspot_size");
        NM_PARAM(color_a, "color_a");
        NM_PARAM(color_b, "color_b");
        NM_PARAM(color_phase, "color_phase");
        NM_PARAM(pulse_phase, "pulse_phase");
        NM_PARAM(flow_phase, "flow_phase");
        NM_PARAM(animation, "animation_id");
        NM_PARAM(segments, "segment_count");
        NM_PARAM(border_enabled, "border_enabled");
        NM_PARAM(glow_enabled, "glow_enabled");
        NM_PARAM(svg_sdf, "svg_sdf");
        NM_PARAM(svg_ready, "svg_ready");
#undef NM_PARAM
        if (!valid) {
            gs_effect_destroy(f->effect);
            f->effect = NULL;
        }
    }
    obs_leave_graphics();
    if (!f->effect) blog(LOG_ERROR, "[NeonMask Studio] effect unavailable; filter will render transparent. %s",
                         error ? error : "Check shader path and uniforms");
    bfree(error);
    bfree(path);
    nm_update(f, settings);
    return f;
}

static void nm_destroy(void *data)
{
    struct nm_filter *f = data;
    if (!f) return;
    obs_enter_graphics();
    if (f->svg_texture) gs_texture_destroy(f->svg_texture);
    if (f->effect) gs_effect_destroy(f->effect);
    obs_leave_graphics();
    bfree(f);
}

static void nm_tick(void *data, float seconds)
{
    struct nm_filter *f = data;
    if (!f) return;
    nm_motion_tick(&f->motion, seconds, f->config.animation_speed, f->config.animation_id);
    if(f->config.shape_id!=NM_SHAPE_SVG_PATH) return;
    obs_source_t *target=obs_filter_get_target(f->context);
    if(!target) return;
    const uint32_t width=obs_source_get_base_width(target);
    const uint32_t height=obs_source_get_base_height(target);
    if(!width||!height) return;
    nm_fit_result fit;
    if (!nm_safe_fit_calculate(&f->config, width, height, &fit) || !fit.fits)
        return;
    const float bx=fit.half_width;
    const float by=fit.half_height;
    if(!f->svg_dirty && f->svg_width==width && f->svg_height==height &&
       f->svg_mask_w==bx && f->svg_mask_h==by) return;
    f->svg_dirty=false;
    f->svg_width=width;f->svg_height=height;
    f->svg_mask_w=bx;f->svg_mask_h=by;
    /* One bounded CPU raster + GPU upload per asset/size change, not per frame.
     * Failed raster/texture creation remains transparent until another edit. */
    if(!f->svg_ok) return;
    const unsigned n=NM_SVG_SDF_SIZE;
    float *pixels=bmalloc((size_t)n*n*sizeof(float));
    if(!pixels) return;
    const bool valid=nm_svg_raster_sdf(&f->svg_shape,bx,by,pixels,n);
    if(valid){
        const uint8_t *layers[]={ (const uint8_t *)pixels };
        obs_enter_graphics();
        gs_texture_t *texture=gs_texture_create(n,n,GS_R32F,1,layers,0);
        if(f->svg_texture) gs_texture_destroy(f->svg_texture);
        f->svg_texture=texture;
        obs_leave_graphics();
        if(!texture) blog(LOG_WARNING,"[NeonMask Studio] SVG SDF GPU allocation failed");
    }else{
        obs_enter_graphics();
        if(f->svg_texture) gs_texture_destroy(f->svg_texture);
        f->svg_texture=NULL;
        obs_leave_graphics();
        blog(LOG_WARNING,"[NeonMask Studio] SVG SDF raster failed closed");
    }
    bfree(pixels);
}

/* libobs queries filters' dimensions through these callbacks. Target capture
 * stays at the target's base dimensions; only OUR output gains padding.
 * The shared pure-C calculator is used by dimension callbacks and render. */
static uint32_t nm_output_dimension(void *data, bool horizontal)
{
    struct nm_filter *f = data;
    if (!f || !f->context) return 0;
    obs_source_t *target = obs_filter_get_target(f->context);
    if (!target) return 0;
    const uint32_t width = obs_source_get_base_width(target);
    const uint32_t height = obs_source_get_base_height(target);
    if (!width || !height) return horizontal ? width : height;
    nm_fit_result fit;
    if (!nm_safe_fit_calculate(&f->config, width, height, &fit) || !fit.fits)
        return horizontal ? width : height; /* render fails closed */
    return horizontal ? fit.output_width : fit.output_height;
}

static uint32_t nm_get_width(void *data) { return nm_output_dimension(data, true); }
static uint32_t nm_get_height(void *data) { return nm_output_dimension(data, false); }

static void nm_render(void *data, gs_effect_t *unused)
{
    (void)unused;
    struct nm_filter *f = data;
    obs_source_t *target = obs_filter_get_target(f->context);
    /* A missing effect must never expose the unmasked upstream source.
     * No draw is transparent in OBS's filter composition. */
    if (!target || !f->effect)
        return;
    /* Match the texrender dimensions used by libobs's filter capture. */
    const uint32_t width = obs_source_get_base_width(target);
    const uint32_t height = obs_source_get_base_height(target);
    /* Transient zero-size targets must not bypass the mask either. */
    if (!width || !height)
        return;
    nm_fit_result fit;
    if (!nm_safe_fit_calculate(&f->config, width, height, &fit))
        return;
    if (!fit.fits) {
        /* A mask center outside the available halo envelope cannot be fitted
         * without moving the user's frame. Never silently reposition it. */
        if (!f->fit_warning_sent) {
            blog(LOG_WARNING, "[NeonMask Studio] safe-fit/expanded output cannot accommodate "
                              "this mask/envelope within 512px-per-side / 8192px output limits; "
                              "adjust placement or glow. Output is transparent.");
            f->fit_warning_sent = true;
        }
        return;
    }
    f->fit_warning_sent = false;
    if (f->config.expand_canvas && (!f->padding_reported ||
        fit.pad_left != f->logged_left || fit.pad_top != f->logged_top ||
        fit.pad_right != f->logged_right || fit.pad_bottom != f->logged_bottom)) {
        blog(LOG_INFO, "[NeonMask Studio] expanded output padding L=%u T=%u R=%u B=%u px; "
                       "scene item content shifts +L/+T until scene transform is compensated",
             fit.pad_left, fit.pad_top, fit.pad_right, fit.pad_bottom);
        f->padding_reported = true;
        f->logged_left = fit.pad_left;
        f->logged_top = fit.pad_top;
        f->logged_right = fit.pad_right;
        f->logged_bottom = fit.pad_bottom;
    }
    const float half_width = fit.half_width;
    const float half_height = fit.half_height;
    const float rx = fminf(half_width, half_height);
    const float corner_radius = f->config.roundness * fminf(half_width, half_height);
    struct vec2 dimensions;
    struct vec2 output_dimensions;
    struct vec2 origin;
    struct vec2 halfsize;
    struct vec2 mask_offset;
    struct vec2 subject_pan;
    struct vec4 primary;
    struct vec4 secondary;
    vec2_set(&dimensions, (float)width, (float)height);
    vec2_set(&output_dimensions, (float)fit.output_width, (float)fit.output_height);
    vec2_set(&origin, (float)fit.pad_left, (float)fit.pad_top);
    vec2_set(&halfsize, f->config.shape_id == NM_SHAPE_CIRCLE ? rx : half_width,
             f->config.shape_id == NM_SHAPE_CIRCLE ? rx : half_height);
    vec2_set(&mask_offset, f->config.mask_x_px, f->config.mask_y_px);
    vec2_set(&subject_pan, f->config.subject_pan_x_px, f->config.subject_pan_y_px);
    /* Native OBS color properties are RGBA-packed. The filter's SRGB path
     * expects linear RGB uniform values; the BGRA helper is not applicable. */
    vec4_from_rgba_srgb(&primary, f->config.primary);
    vec4_from_rgba_srgb(&secondary, f->config.secondary);
    /* Disable direct bypass: the shader relies on captured premultiplied RGB. */
    if (!obs_source_process_filter_begin(f->context, GS_RGBA, OBS_NO_DIRECT_RENDERING)) return;
    gs_effect_set_vec2(f->uv_size, &dimensions);
    gs_effect_set_vec2(f->output_size, &output_dimensions);
    gs_effect_set_vec2(f->input_origin, &origin);
    gs_effect_set_vec2(f->half_size, &halfsize);
    gs_effect_set_vec2(f->mask_offset, &mask_offset);
    gs_effect_set_vec2(f->subject_pan, &subject_pan);
    gs_effect_set_float(f->subject_zoom, f->config.subject_zoom);
    gs_effect_set_float(f->shape_rotation, f->config.shape_rotation_deg * 0.01745329251994329577f);
    gs_effect_set_int(f->polygon_sides, f->config.polygon_sides);
    gs_effect_set_float(f->shape_detail, f->config.shape_detail);
    struct vec4 bubble_body, bubble_tail;
    vec4_set(&bubble_body, f->config.bubble_left_inset,
             f->config.bubble_right_inset, f->config.bubble_top_inset,
             f->config.bubble_bottom_inset);
    vec4_set(&bubble_tail, f->config.bubble_tail_left,
             f->config.bubble_tail_right, f->config.bubble_tail_tip,
             f->config.bubble_tail_depth);
    gs_effect_set_vec4(f->bubble_body, &bubble_body);
    gs_effect_set_vec4(f->bubble_tail, &bubble_tail);
    gs_effect_set_float(f->corner_radius, corner_radius);
    gs_effect_set_int(f->shape, f->config.shape_id);
    gs_effect_set_float(f->border_width, f->config.border_px);
    gs_effect_set_int(f->style, f->config.style_id);
    gs_effect_set_int(f->ornament_mode, f->config.ornament_mode);
    gs_effect_set_float(f->art_intensity, f->config.art_intensity);
    gs_effect_set_float(f->art_gap, f->config.art_gap);
    gs_effect_set_float(f->feather, f->config.feather_px);
    gs_effect_set_float(f->glow_radius, f->config.glow_px);
    gs_effect_set_float(f->glow_strength, f->config.glow_amount);
    gs_effect_set_float(f->mid_glow_strength, f->config.mid_glow_strength);
    gs_effect_set_float(f->bloom_strength, f->config.bloom_strength);
    gs_effect_set_float(f->hotspot_strength, f->config.hotspot_strength);
    gs_effect_set_float(f->hotspot_size, f->config.hotspot_size);
    gs_effect_set_vec4(f->color_a, &primary);
    gs_effect_set_vec4(f->color_b, &secondary);
    gs_effect_set_float(f->color_phase, (float)f->motion.color_turns);
    gs_effect_set_float(f->pulse_phase, (float)f->motion.pulse_turns);
    gs_effect_set_float(f->flow_phase, (float)f->motion.flow_turns);
    gs_effect_set_int(f->animation, f->config.animation_id);
    gs_effect_set_int(f->segments, f->config.segment_count);
    gs_effect_set_int(f->border_enabled, f->config.show_border ? 1 : 0);
    gs_effect_set_int(f->glow_enabled, f->config.show_glow ? 1 : 0);
    const bool svg_ready=f->svg_ok && f->svg_texture &&
        f->svg_width==width && f->svg_height==height &&
        f->svg_mask_w==half_width && f->svg_mask_h==half_height;
    gs_effect_set_int(f->svg_ready,svg_ready ? 1 : 0);
    gs_effect_set_texture(f->svg_sdf,svg_ready ? f->svg_texture : NULL);
    obs_source_process_filter_tech_end(f->context, f->effect,
                                       fit.output_width, fit.output_height, "Draw");
}

struct obs_source_info neonmask_filter_info = {
    .id = "neonmask_studio_filter",
    .type = OBS_SOURCE_TYPE_FILTER,
    .output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_SRGB,
    .get_name = nm_get_name,
    .create = nm_create,
    .destroy = nm_destroy,
    .update = nm_update,
    .get_defaults = nm_defaults,
    .get_properties = nm_properties,
    .video_tick = nm_tick,
    .video_render = nm_render,
    .get_width = nm_get_width,
    .get_height = nm_get_height,
};
