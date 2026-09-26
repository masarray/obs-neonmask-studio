/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Compile the real OBS .effect against a live OpenGL device under Xvfb.
 * This verifies the effect parser/GL compiler, not visual correctness. */
#include <obs.h>
#include <graphics/graphics.h>
#include <util/bmem.h>
#include <util/dstr.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>

static const char *const uniforms[] = {
    "uv_size", "half_size", "mask_offset", "subject_pan", "subject_zoom",
    "polygon_sides", "polygon_rotation", "corner_radius", "shape_id", "border_width",
    "feather", "glow_radius", "glow_strength", "color_a", "color_b",
    "color_phase", "pulse_phase", "flow_phase", "animation_id", "segment_count",
    "border_enabled", "glow_enabled", "style_id", "image", "ViewProj"
};


/* Partial G4: execute the real shader and read a small GPU render target.
 * This is intentionally NOT a scene/source-chain or final OBS output test.
 * Readback is permitted only in this test; the production render path has none. */
static int verify_pixel_fixture(gs_effect_t *effect, int variant)
{
    enum { W = 64, H = 64 };
    uint8_t pixels[W * H * 4];
    const uint8_t red = variant == 0 ? 255 : variant == 1 ? 128 : 0;
    const uint8_t alpha = variant == 0 ? 255 : variant == 1 ? 128 : 0;
    for (size_t i = 0; i < W * H; ++i) {
        pixels[4 * i + 0] = red;   /* premultiplied red */
        pixels[4 * i + 1] = 0;
        pixels[4 * i + 2] = 0;
        pixels[4 * i + 3] = alpha;
    }

    const uint8_t *layers[] = {pixels};
    gs_texture_t *input = gs_texture_create(W, H, GS_RGBA, 1, layers, 0);
    gs_texrender_t *target = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
    gs_stagesurf_t *stage = gs_stagesurface_create(W, H, GS_RGBA);
    int failed = 0;

    if (!input || !target || !stage) {
        fprintf(stderr, "FAIL: GPU fixture resource creation\n");
        failed = 1;
        goto done;
    }

    struct vec2 dims;
    struct vec2 extents;
    struct vec4 magenta;
    vec2_set(&dims, (float)W, (float)H);
    vec2_set(&extents, 24.0f, 24.0f);
    struct vec2 mask_offset, subject_pan;
    vec2_set(&mask_offset, 0.0f, 0.0f);
    vec2_set(&subject_pan, 0.0f, 0.0f);
    vec4_set(&magenta, 1.0f, 0.0f, 1.0f, 1.0f);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "uv_size"), &dims);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "half_size"), &extents);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "mask_offset"), &mask_offset);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "subject_pan"), &subject_pan);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "subject_zoom"), 1.0f);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "polygon_sides"), 8);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "polygon_rotation"), 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "corner_radius"), 5.0f);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "shape_id"), 0);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "border_width"), 4.0f);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "style_id"), 0);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "feather"), 0.5f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "glow_radius"), 8.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "glow_strength"), 0.0f);
    gs_effect_set_vec4(gs_effect_get_param_by_name(effect, "color_a"), &magenta);
    gs_effect_set_vec4(gs_effect_get_param_by_name(effect, "color_b"), &magenta);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "color_phase"), 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "pulse_phase"), 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "flow_phase"), 0.0f);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "animation_id"), 0);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "segment_count"), 0);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "border_enabled"), variant == 2);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "glow_enabled"), 0);
    gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), input);

    if (!gs_texrender_begin(target, W, H)) {
        fprintf(stderr, "FAIL: GPU fixture render target begin\n");
        failed = 1;
        goto done;
    }

    const bool previous_srgb = gs_framebuffer_srgb_enabled();
    gs_enable_framebuffer_srgb(false);
    gs_blend_state_push();
    gs_enable_blending(false);
    struct vec4 transparent;
    vec4_zero(&transparent);
    gs_clear(GS_CLEAR_COLOR, &transparent, 0.0f, 0);
    gs_ortho(0.0f, (float)W, 0.0f, (float)H, -100.0f, 100.0f);
    int draw_count = 0;
    /* Standalone harness has no OBS scene traversal to establish a model
     * transform/cull state. Make the draw state deterministic. */
    gs_matrix_push();
    gs_matrix_identity();
    const enum gs_cull_mode previous_cull = gs_get_cull_mode();
    gs_set_cull_mode(GS_NEITHER);
    while (gs_effect_loop(effect, "Draw")) {
        ++draw_count;
        gs_draw_sprite(input, 0, W, H);
    }
    gs_set_cull_mode(previous_cull);
    gs_matrix_pop();
    if (draw_count != 1) {
        fprintf(stderr, "FAIL: GPU fixture %d expected one Draw pass, got %d\n", variant, draw_count);
        failed = 1;
    }
    gs_blend_state_pop();
    gs_enable_framebuffer_srgb(previous_srgb);
    gs_texrender_end(target);

    gs_stage_texture(stage, gs_texrender_get_texture(target));
    uint8_t *mapped = NULL;
    uint32_t stride = 0;
    if (!gs_stagesurface_map(stage, &mapped, &stride)) {
        fprintf(stderr, "FAIL: GPU fixture readback\n");
        failed = 1;
        goto done;
    }

    const uint8_t *center = mapped + 32u * stride + 32u * 4u;
    const uint8_t *corner = mapped + 1u * stride + 1u * 4u;
    const uint8_t *rim = mapped + 32u * stride + 55u * 4u;


    /* The black/transparent corner proves the mask is not an opaque box.
     * Semi-transparent center checks the premultiplied-input convention. */
    if (corner[3] > 2 || corner[0] > 2 || corner[1] > 2 || corner[2] > 2) {
        fprintf(stderr, "FAIL: GPU fixture %d outside mask RGBA=(%u,%u,%u,%u)\n",
                variant, corner[0], corner[1], corner[2], corner[3]);
        failed = 1;
    }
    if (variant != 2) {
        const int expected_alpha = variant == 0 ? 255 : 128;
        if (center[0] < 240 || center[1] > 15 || center[2] > 15 ||
            abs((int)center[3] - expected_alpha) > 3) {
            fprintf(stderr, "FAIL: GPU fixture %d center RGBA=(%u,%u,%u,%u)\n",
                    variant, center[0], center[1], center[2], center[3]);
            failed = 1;
        }
    } else if (rim[0] < 180 || rim[1] > 30 || rim[2] < 180 || rim[3] < 180) {
        fprintf(stderr, "FAIL: GPU border-only rim RGBA=(%u,%u,%u,%u)\n",
                rim[0], rim[1], rim[2], rim[3]);
        failed = 1;
    }

    gs_stagesurface_unmap(stage);
done:
    if (stage) gs_stagesurface_destroy(stage);
    if (target) gs_texrender_destroy(target);
    if (input) gs_texture_destroy(input);
    return failed;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s path/to/neon-mask.effect\n", argv[0]);
        return 2;
    }
    if (!obs_startup("en-US", NULL, NULL)) {
        fprintf(stderr, "FAIL: obs_startup\n");
        return 2;
    }

    /* G1 migration contract: a default schema is not a saved user value.
     * This matters because legacy v0 scene settings acquire OBS defaults. */
    obs_data_t *schema_fixture = obs_data_create();
    obs_data_set_default_int(schema_fixture, "schema_version", 1);
    if (obs_data_has_user_value(schema_fixture, "schema_version") ||
        obs_data_get_int(schema_fixture, "schema_version") != 1) {
        fprintf(stderr, "FAIL: OBS schema default/user-value contract\n");
        obs_data_release(schema_fixture);
        obs_shutdown();
        return 1;
    }
    obs_data_set_int(schema_fixture, "schema_version", 1);
    if (!obs_data_has_user_value(schema_fixture, "schema_version")) {
        fprintf(stderr, "FAIL: OBS explicit schema value not visible\n");
        obs_data_release(schema_fixture);
        obs_shutdown();
        return 1;
    }
    obs_data_release(schema_fixture);

    /* The CI SDK is staged outside an installed OBS directory. Add the real
     * libobs base effects before initializing the graphics subsystem. */
    const char *libobs_data = getenv("NEONMASK_LIBOBS_DATA_DIR");
    if (libobs_data && *libobs_data) {
        /* libobs check_path concatenates the directory and basename directly;
         * a trailing separator is therefore part of this API contract. */
        struct dstr data_dir = {0};
        dstr_init_copy(&data_dir, libobs_data);
        size_t n = strlen(libobs_data);
        if (libobs_data[n - 1] != '/' && libobs_data[n - 1] != '\\')
            dstr_cat(&data_dir, "/");
        obs_add_data_path(data_dir.array);
        dstr_free(&data_dir);
        char *effect_file = obs_find_data_file("default.effect");
        if (!effect_file) {
            fprintf(stderr, "FAIL: cannot find libobs default.effect in %s\n", libobs_data);
            obs_shutdown();
            return 2;
        }
        bfree(effect_file);
    }

    struct obs_video_info video = {0};
    /* Windows CI selects its built Direct3D 11 module; Linux defaults to OpenGL. */
    const char *module = getenv("NEONMASK_GRAPHICS_MODULE");
    video.graphics_module = module && *module ? module : "libobs-opengl";
    video.fps_num = 30;
    video.fps_den = 1;
    video.base_width = 640;
    video.base_height = 360;
    video.output_width = 640;
    video.output_height = 360;
    video.output_format = VIDEO_FORMAT_RGBA;
    video.adapter = 0;

    if (obs_reset_video(&video) != OBS_VIDEO_SUCCESS) {
        fprintf(stderr, "FAIL: obs_reset_video: graphics device unavailable\n");
        obs_shutdown();
        return 2;
    }

    char *errors = NULL;
    int missing = 0;
    obs_enter_graphics();
    gs_effect_t *effect = gs_effect_create_from_file(argv[1], &errors);
    if (!effect) {
        fprintf(stderr, "FAIL: shader compile: %s\n", errors ? errors : "(no details)");
        missing = 1;
    } else {
        if (!gs_effect_get_technique(effect, "Draw")) {
            fprintf(stderr, "FAIL: Draw technique not found\n");
            missing++;
        }
        for (size_t i = 0; i < sizeof(uniforms)/sizeof(uniforms[0]); ++i) {
            if (!gs_effect_get_param_by_name(effect, uniforms[i])) {
                fprintf(stderr, "FAIL: missing parameter %s\n", uniforms[i]);
                missing++;
            }
        }
        if (!missing) {
            for (int variant = 0; variant < 3; ++variant)
                missing += verify_pixel_fixture(effect, variant);
        }
        gs_effect_destroy(effect);
    }
    obs_leave_graphics();
    bfree(errors);
    obs_shutdown();
    if (missing) return 1;
    puts("PASS: real libobs shader compilation + direct GPU pixel fixtures (partial G4)");
    return 0;
}
