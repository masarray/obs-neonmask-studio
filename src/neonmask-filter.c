/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-filter.h"
#include "neonmask-math.h"
#include "neonmask-presets.h"
#include <math.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>
#include <util/bmem.h>

struct nm_filter {
    obs_source_t *context;
    gs_effect_t *effect;
    gs_eparam_t *uv_size;
    gs_eparam_t *half_size;
    gs_eparam_t *corner_radius;
    gs_eparam_t *shape;
    gs_eparam_t *border_width;
    gs_eparam_t *feather;
    gs_eparam_t *glow_radius;
    gs_eparam_t *glow_strength;
    gs_eparam_t *color_a;
    gs_eparam_t *color_b;
    gs_eparam_t *elapsed_time;
    gs_eparam_t *speed;
    gs_eparam_t *animation;
    gs_eparam_t *segments;
    gs_eparam_t *border_enabled;
    gs_eparam_t *glow_enabled;
    float scale;
    float roundness;
    float border_px;
    float feather_px;
    float glow_px;
    float glow_amount;
    float animation_speed;
    float time;
    uint32_t primary;
    uint32_t secondary;
    int shape_id;
    int animation_id;
    int segment_count;
    bool show_border;
    bool show_glow;
};

static const char *nm_get_name(void *unused)
{
    (void)unused;
    return obs_module_text("Filter.Name");
}

static void nm_update(void *data, obs_data_t *settings)
{
    struct nm_filter *f = data;
    f->scale = nm_clamp((float)obs_data_get_double(settings, "scale"), 0.30f, 0.96f);
    f->roundness = nm_clamp((float)obs_data_get_double(settings, "roundness"), 0.0f, 1.0f);
    f->border_px = nm_clamp((float)obs_data_get_double(settings, "border_width"), 0.5f, 32.0f);
    f->feather_px = nm_clamp((float)obs_data_get_double(settings, "feather"), 0.5f, 30.0f);
    f->glow_px = nm_clamp((float)obs_data_get_double(settings, "glow_radius"), 1.0f, 80.0f);
    f->glow_amount = nm_clamp((float)obs_data_get_double(settings, "glow_strength"), 0.0f, 1.0f);
    f->animation_speed = nm_clamp((float)obs_data_get_double(settings, "speed"), 0.0f, 5.0f);
    f->primary = (uint32_t)obs_data_get_int(settings, "primary") | 0xFF000000u;
    f->secondary = (uint32_t)obs_data_get_int(settings, "secondary") | 0xFF000000u;
    f->shape_id = (int)obs_data_get_int(settings, "shape");
    if (f->shape_id < 0 || f->shape_id > 4) f->shape_id = 0;
    f->animation_id = (int)obs_data_get_int(settings, "animation");
    if (f->animation_id < 0 || f->animation_id > 2) f->animation_id = 0;
    f->segment_count = (int)obs_data_get_int(settings, "segments");
    if (f->segment_count < 0) f->segment_count = 0;
    if (f->segment_count > 48) f->segment_count = 48;
    f->show_border = obs_data_get_bool(settings, "border_enabled");
    f->show_glow = obs_data_get_bool(settings, "glow_enabled");
}

static void nm_defaults(obs_data_t *settings)
{
    obs_data_set_default_int(settings, "preset", 0);
    nm_preset initial;
    if (!nm_get_preset(1, &initial)) return;
    obs_data_set_default_int(settings, "shape", initial.shape);
    obs_data_set_default_double(settings, "scale", initial.scale);
    obs_data_set_default_double(settings, "roundness", initial.roundness);
    obs_data_set_default_double(settings, "border_width", initial.border_width);
    obs_data_set_default_double(settings, "feather", 0.85);
    obs_data_set_default_double(settings, "glow_radius", 18.0);
    obs_data_set_default_double(settings, "glow_strength", initial.glow_strength);
    obs_data_set_default_int(settings, "primary", initial.primary);
    obs_data_set_default_int(settings, "secondary", initial.secondary);
    obs_data_set_default_int(settings, "animation", initial.animation);
    obs_data_set_default_double(settings, "speed", 0.65);
    obs_data_set_default_int(settings, "segments", initial.segments);
    obs_data_set_default_bool(settings, "border_enabled", true);
    obs_data_set_default_bool(settings, "glow_enabled", true);
}

/* Apply preset as real user-editable property values: no hidden runtime overrides. */
static bool nm_preset_changed(obs_properties_t *props, obs_property_t *property,
                              obs_data_t *settings)
{
    (void)props; (void)property;
    nm_preset preset;
    if (!nm_get_preset((int)obs_data_get_int(settings, "preset"), &preset))
        return false;
    obs_data_set_int(settings, "shape", preset.shape);
    obs_data_set_int(settings, "animation", preset.animation);
    obs_data_set_int(settings, "primary", preset.primary);
    obs_data_set_int(settings, "secondary", preset.secondary);
    obs_data_set_double(settings, "scale", preset.scale);
    obs_data_set_double(settings, "roundness", preset.roundness);
    obs_data_set_double(settings, "border_width", preset.border_width);
    obs_data_set_double(settings, "glow_strength", preset.glow_strength);
    obs_data_set_int(settings, "segments", preset.segments);
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
    obs_property_set_modified_callback(preset, nm_preset_changed);

    obs_property_t *shape = obs_properties_add_list(props, "shape", obs_module_text("Shape"),
                                                     OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(shape, obs_module_text("Shape.Rounded"), NM_SHAPE_ROUNDED);
    obs_property_list_add_int(shape, obs_module_text("Shape.Circle"), NM_SHAPE_CIRCLE);
    obs_property_list_add_int(shape, obs_module_text("Shape.Ellipse"), NM_SHAPE_ELLIPSE);
    obs_property_list_add_int(shape, obs_module_text("Shape.Hexagon"), NM_SHAPE_HEXAGON);
    obs_property_list_add_int(shape, obs_module_text("Shape.Diamond"), NM_SHAPE_DIAMOND);
    obs_property_set_modified_callback(shape, nm_custom_changed);

#define NM_CUSTOM(expr) obs_property_set_modified_callback((expr), nm_custom_changed)
    NM_CUSTOM(obs_properties_add_float_slider(props, "scale", obs_module_text("Scale"), 0.30, 0.96, 0.01));
    NM_CUSTOM(obs_properties_add_float_slider(props, "roundness", obs_module_text("Roundness"), 0.0, 1.0, 0.01));
    NM_CUSTOM(obs_properties_add_float_slider(props, "feather", obs_module_text("Feather"), 0.5, 30.0, 0.5));
    NM_CUSTOM(obs_properties_add_bool(props, "border_enabled", obs_module_text("Border.Enabled")));
    NM_CUSTOM(obs_properties_add_float_slider(props, "border_width", obs_module_text("Border.Width"), 0.5, 32.0, 0.5));
    NM_CUSTOM(obs_properties_add_color(props, "primary", obs_module_text("Color.Primary")));
    NM_CUSTOM(obs_properties_add_color(props, "secondary", obs_module_text("Color.Secondary")));
    NM_CUSTOM(obs_properties_add_bool(props, "glow_enabled", obs_module_text("Glow.Enabled")));
    NM_CUSTOM(obs_properties_add_float_slider(props, "glow_radius", obs_module_text("Glow.Radius"), 1.0, 80.0, 1.0));
    NM_CUSTOM(obs_properties_add_float_slider(props, "glow_strength", obs_module_text("Glow.Strength"), 0.0, 1.0, 0.01));

    obs_property_t *animation = obs_properties_add_list(props, "animation", obs_module_text("Animation"),
                                                         OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(animation, obs_module_text("Animation.Static"), NM_ANIM_STATIC);
    obs_property_list_add_int(animation, obs_module_text("Animation.Pulse"), NM_ANIM_PULSE);
    obs_property_list_add_int(animation, obs_module_text("Animation.Flow"), NM_ANIM_FLOW);
    obs_property_set_modified_callback(animation, nm_custom_changed);
    NM_CUSTOM(obs_properties_add_float_slider(props, "speed", obs_module_text("Speed"), 0.0, 5.0, 0.05));
    NM_CUSTOM(obs_properties_add_int_slider(props, "segments", obs_module_text("Segments"), 0, 48, 1));
#undef NM_CUSTOM
    return props;
}

static void *nm_create(obs_data_t *settings, obs_source_t *context)
{
    struct nm_filter *f = bzalloc(sizeof(*f));
    f->context = context;
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
        NM_PARAM(half_size, "half_size");
        NM_PARAM(corner_radius, "corner_radius");
        NM_PARAM(shape, "shape_id");
        NM_PARAM(border_width, "border_width");
        NM_PARAM(feather, "feather");
        NM_PARAM(glow_radius, "glow_radius");
        NM_PARAM(glow_strength, "glow_strength");
        NM_PARAM(color_a, "color_a");
        NM_PARAM(color_b, "color_b");
        NM_PARAM(elapsed_time, "elapsed_time");
        NM_PARAM(speed, "animation_speed");
        NM_PARAM(animation, "animation_id");
        NM_PARAM(segments, "segment_count");
        NM_PARAM(border_enabled, "border_enabled");
        NM_PARAM(glow_enabled, "glow_enabled");
#undef NM_PARAM
        if (!valid) {
            gs_effect_destroy(f->effect);
            f->effect = NULL;
        }
    }
    obs_leave_graphics();
    if (!f->effect) blog(LOG_ERROR, "[NeonMask Studio] effect unavailable; filter will bypass. %s",
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
    if (f->effect) gs_effect_destroy(f->effect);
    obs_leave_graphics();
    bfree(f);
}

static void nm_tick(void *data, float seconds)
{
    struct nm_filter *f = data;
    if (!f) return;
    if (f->animation_id != NM_ANIM_STATIC) {
        f->time = fmodf(f->time + nm_clamp(seconds, 0.0f, 1.0f), 3600.0f);
    }
}

static void nm_render(void *data, gs_effect_t *unused)
{
    (void)unused;
    struct nm_filter *f = data;
    obs_source_t *target = obs_filter_get_target(f->context);
    if (!target || !f->effect) {
        obs_source_skip_video_filter(f->context);
        return;
    }
    /* Match the texrender dimensions used by libobs's filter capture. */
    const uint32_t width = obs_source_get_base_width(target);
    const uint32_t height = obs_source_get_base_height(target);
    if (!width || !height) {
        obs_source_skip_video_filter(f->context);
        return;
    }
    const nm_geometry g = nm_make_geometry(width, height, f->scale, f->roundness);
    const float rx = fminf(g.half_width, g.half_height);
    struct vec2 dimensions;
    struct vec2 halfsize;
    struct vec4 primary;
    struct vec4 secondary;
    vec2_set(&dimensions, g.width, g.height);
    vec2_set(&halfsize, f->shape_id == NM_SHAPE_CIRCLE ? rx : g.half_width,
             f->shape_id == NM_SHAPE_CIRCLE ? rx : g.half_height);
    /* Native OBS color properties are RGBA-packed. The filter's SRGB path
     * expects linear RGB uniform values; the BGRA helper is not applicable. */
    vec4_from_rgba_srgb(&primary, f->primary);
    vec4_from_rgba_srgb(&secondary, f->secondary);
    /* Disable direct bypass: the shader relies on captured premultiplied RGB. */
    if (!obs_source_process_filter_begin(f->context, GS_RGBA, OBS_NO_DIRECT_RENDERING)) return;
    gs_effect_set_vec2(f->uv_size, &dimensions);
    gs_effect_set_vec2(f->half_size, &halfsize);
    gs_effect_set_float(f->corner_radius, g.radius);
    gs_effect_set_int(f->shape, f->shape_id);
    gs_effect_set_float(f->border_width, f->border_px);
    gs_effect_set_float(f->feather, f->feather_px);
    gs_effect_set_float(f->glow_radius, f->glow_px);
    gs_effect_set_float(f->glow_strength, f->glow_amount);
    gs_effect_set_vec4(f->color_a, &primary);
    gs_effect_set_vec4(f->color_b, &secondary);
    gs_effect_set_float(f->elapsed_time, f->time);
    gs_effect_set_float(f->speed, f->animation_speed);
    gs_effect_set_int(f->animation, f->animation_id);
    gs_effect_set_int(f->segments, f->segment_count);
    gs_effect_set_int(f->border_enabled, f->show_border ? 1 : 0);
    gs_effect_set_int(f->glow_enabled, f->show_glow ? 1 : 0);
    obs_source_process_filter_tech_end(f->context, f->effect, 0, 0, "Draw");
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
};
