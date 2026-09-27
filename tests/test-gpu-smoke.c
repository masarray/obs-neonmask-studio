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
    "shape_rotation", "polygon_sides", "shape_detail", "corner_radius", "shape_id", "border_width",
    "feather", "glow_radius", "glow_strength", "mid_glow_strength",
    "bloom_strength", "hotspot_strength", "hotspot_size", "color_a", "color_b",
    "color_phase", "pulse_phase", "flow_phase", "animation_id", "segment_count",
    "border_enabled", "glow_enabled", "style_id", "ornament_mode",
    "art_intensity", "art_gap", "image", "ViewProj"
};


/* Partial G4: execute the real shader and read a small GPU render target.
 * This is intentionally NOT a scene/source-chain or final OBS output test.
 * Readback is permitted only in this test; the production render path has none. */
static int verify_pixel_fixture(gs_effect_t *effect, int variant)
{
    enum { W = 64, H = 64 };
    uint8_t pixels[W * H * 4];
    const uint8_t red = variant == 0 ? 255 : variant == 1 ? 128 :
                        (variant == 3 || variant == 4 || variant == 18 ||
                         variant == 19 || variant == 21 || variant == 22 ||
                         variant == 24 || variant == 25 || variant == 27 ? 255 : 0);
    const uint8_t alpha = variant == 0 ? 255 : variant == 1 ? 128 :
                          (variant == 3 || variant == 4 || variant == 18 ||
                           variant == 19 || variant == 21 || variant == 22 ||
                           variant == 24 || variant == 25 || variant == 27 ? 255 : 0);
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
    const bool glow_case = variant == 5 || variant == 6 || variant == 9 || variant == 10;

    if (!input || !target || !stage) {
        fprintf(stderr, "FAIL: GPU fixture resource creation\n");
        failed = 1;
        goto done;
    }

    struct vec2 dims;
    struct vec2 extents;
    struct vec2 zero2;
    struct vec4 magenta;
    vec2_set(&dims, (float)W, (float)H);
    vec2_set(&extents, 24.0f, 24.0f);
    vec2_set(&zero2, 0.0f, 0.0f);
    struct vec2 mask_shift;
    struct vec2 subject_shift;
    vec2_set(&mask_shift, variant == 4 ? 28.0f : 0.0f, 0.0f);
    vec2_set(&subject_shift, variant == 3 ? 96.0f : 0.0f, 0.0f);
    vec4_set(&magenta, 1.0f, 0.0f, 1.0f, 1.0f);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "uv_size"), &dims);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "half_size"), &extents);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "mask_offset"), &mask_shift);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "subject_pan"), &subject_shift);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "subject_zoom"), 1.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "shape_rotation"), 0.0f);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "polygon_sides"), 8);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "shape_detail"), 0.23f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "corner_radius"),
                        variant == 24 ? 0.0f :
                        (variant == 25 || variant == 26 ? 10.5f : 5.0f));
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "shape_id"),
                      variant == 13 || variant == 14 ? 1 :
                      variant == 15 || variant == 22 ? 5 :
                      variant == 18 || variant == 20 || variant == 27 ? 8 :
                      variant == 21 || variant == 23 || variant >= 24 ? 9 : 0);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "border_width"), 4.0f);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "style_id"), 0);
    int art_mode = variant == 11 ? 1 : variant == 13 ? 2 :
                   variant == 15 ? 3 : variant == 16 ? 4 : 0;
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "ornament_mode"), art_mode);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "art_intensity"), 0.90f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "art_gap"), 2.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "feather"), 0.5f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "glow_radius"),
                        glow_case ? 12.0f : 8.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "glow_strength"),
                        glow_case ? 0.85f : 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "mid_glow_strength"), 0.75f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "bloom_strength"), variant == 6 ? 0.0f : 0.92f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "hotspot_strength"), variant == 8 ? 1.0f : 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "hotspot_size"), 0.10f);
    gs_effect_set_vec4(gs_effect_get_param_by_name(effect, "color_a"), &magenta);
    gs_effect_set_vec4(gs_effect_get_param_by_name(effect, "color_b"), &magenta);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "color_phase"), 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "pulse_phase"),
                        variant == 9 ? 0.25f : variant == 10 ? 0.75f : 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "flow_phase"), 0.0f);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "animation_id"),
                      variant == 8 ? 2 : (variant == 9 || variant == 10 ? 1 : 0));
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "segment_count"), 0);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "border_enabled"),
                      variant == 2 || (variant >= 5 && variant <= 17) ||
                      variant == 20 || variant == 23 || variant == 26 || variant == 27);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "glow_enabled"), glow_case);
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
    const uint8_t *shifted_center = mapped + 32u * stride + 60u * 4u;
    const uint8_t *near_glow = mapped + 32u * stride + 59u * 4u;
    const uint8_t *far_glow = mapped + 32u * stride + 63u * 4u;
    const uint8_t *opposite_rim = mapped + 32u * stride + 8u * 4u;
    const uint8_t *cyber_bar = mapped + 3u * stride + 52u * 4u;
    const uint8_t *reactor_arc = mapped + 1u * stride + 32u * 4u;
    const uint8_t *streamer_tail = mapped + 63u * stride + 17u * 4u;
    const uint8_t *bubble_tip_fill = mapped + 52u * stride + 15u * 4u;
    const uint8_t *bubble_beside = mapped + 52u * stride + 27u * 4u;
    const uint8_t *bubble_tip_neon = mapped + 55u * stride + 15u * 4u;
    /* The body-tail overlap lies wholly INSIDE the bubble. A min-of-two
     * SDFs incorrectly draws a blue neon strip along the horizontal seam. */
    const uint8_t *bubble_internal_seam = mapped + 46u * stride + 19u * 4u;
    const uint8_t *bubble_exposed_bottom = mapped + 46u * stride + 27u * 4u;
    const uint8_t *card_cut_corner = mapped + 11u * stride + 53u * 4u;
    const uint8_t *card_diag_neon = mapped + 12u * stride + 52u * 4u;
    const uint8_t *fillet_endpoint = mapped + 8u * stride + 46u * 4u;
    const uint8_t *square_top_left = mapped + 8u * stride + 8u * 4u;
    const uint8_t *square_bottom_right = mapped + 55u * stride + 55u * 4u;


    /* The black/transparent corner proves the mask is not an opaque box.
     * Semi-transparent center checks the premultiplied-input convention. */
    if (!glow_case &&
        (corner[3] > 2 || corner[0] > 2 || corner[1] > 2 || corner[2] > 2)) {
        fprintf(stderr, "FAIL: GPU fixture %d outside mask RGBA=(%u,%u,%u,%u)\n",
                variant, corner[0], corner[1], corner[2], corner[3]);
        failed = 1;
    }
    if (variant == 0 || variant == 1) {
        const int expected_alpha = variant == 0 ? 255 : 128;
        if (center[0] < 240 || center[1] > 15 || center[2] > 15 ||
            abs((int)center[3] - expected_alpha) > 3) {
            fprintf(stderr, "FAIL: GPU fixture %d center RGBA=(%u,%u,%u,%u)\n",
                    variant, center[0], center[1], center[2], center[3]);
            failed = 1;
        }
    } else if (variant == 2 && (rim[0] < 180 || rim[1] > 150 || rim[2] < 180 || rim[3] < 180)) {
        fprintf(stderr, "FAIL: GPU border-only rim RGBA=(%u,%u,%u,%u)\n",
                rim[0], rim[1], rim[2], rim[3]);
        failed = 1;
    } else if (variant == 3 && (center[0] > 2 || center[1] > 2 || center[2] > 2 || center[3] > 2)) {
        fprintf(stderr, "FAIL: subject pan must sample transparent outside source, got (%u,%u,%u,%u)\n",
                center[0], center[1], center[2], center[3]);
        failed = 1;
    } else if (variant == 4) {
        if (center[3] > 2 || shifted_center[0] < 240 || shifted_center[3] < 250) {
            fprintf(stderr, "FAIL: mask translation center=(%u,%u,%u,%u), shifted=(%u,%u,%u,%u)\n",
                    center[0], center[1], center[2], center[3],
                    shifted_center[0], shifted_center[1], shifted_center[2], shifted_center[3]);
            failed = 1;
        }
    } else if (variant == 5 &&
               (near_glow[3] < 55 || far_glow[3] < 25 || near_glow[3] <= far_glow[3])) {
        fprintf(stderr, "FAIL: layered glow near=%u far=%u (expected mid + broad bloom)\n",
                near_glow[3], far_glow[3]);
        failed = 1;
    } else if (variant == 6 &&
               (near_glow[3] < 30 || far_glow[3] >= 22)) {
        fprintf(stderr, "FAIL: mid-only glow near=%u far=%u (bloom must be independent)\n",
                near_glow[3], far_glow[3]);
        failed = 1;
    } else if (variant == 7 && far_glow[3] > 2) {
        fprintf(stderr, "FAIL: disabled glow leaks outside core: %u\n", far_glow[3]);
        failed = 1;
    } else if (variant == 8 &&
               ((int)rim[1] - (int)opposite_rim[1] < 65 || far_glow[3] > 2)) {
        fprintf(stderr, "FAIL: localized hot spot / glow bypass eastG=%u westG=%u farA=%u\n",
                rim[1], opposite_rim[1], far_glow[3]);
        failed = 1;
    } else if (variant == 9 && near_glow[3] < 113) {
        fprintf(stderr, "FAIL: glow pulse high phase is too dim: %u\n", near_glow[3]);
        failed = 1;
    } else if (variant == 10 && (near_glow[3] > 103 || near_glow[3] < 35)) {
        fprintf(stderr, "FAIL: glow pulse low phase not bounded/visible: %u\n", near_glow[3]);
        failed = 1;
    } else if (variant == 11 && cyber_bar[3] < 100) {
        fprintf(stderr, "FAIL: authored Cyber corner trace missing alpha=%u\n", cyber_bar[3]);
        failed = 1;
    } else if (variant == 12 && cyber_bar[3] > 3) {
        fprintf(stderr, "FAIL: legacy scene acquired Cyber ornament alpha=%u\n", cyber_bar[3]);
        failed = 1;
    } else if (variant == 13 && reactor_arc[3] < 75) {
        fprintf(stderr, "FAIL: authored Reactor outer arc missing alpha=%u\n", reactor_arc[3]);
        failed = 1;
    } else if (variant == 14 && reactor_arc[3] > 3) {
        fprintf(stderr, "FAIL: legacy circle acquired Reactor rail alpha=%u\n", reactor_arc[3]);
        failed = 1;
    } else if (variant == 15 && cyber_bar[3] < 75) {
        fprintf(stderr, "FAIL: authored Tech HUD bracket missing alpha=%u\n", cyber_bar[3]);
        failed = 1;
    } else if (variant == 16 && streamer_tail[3] < 60) {
        fprintf(stderr, "FAIL: authored Streamer tail missing alpha=%u\n", streamer_tail[3]);
        failed = 1;
    } else if (variant == 17 && streamer_tail[3] > 3) {
        fprintf(stderr, "FAIL: legacy rounded frame acquired Streamer tail alpha=%u\n", streamer_tail[3]);
        failed = 1;
    } else if (variant == 18 &&
               (bubble_tip_fill[0] < 240 || bubble_tip_fill[3] < 240 ||
                bubble_beside[3] > 3)) {
        fprintf(stderr, "FAIL: integrated bubble source tip=%u side=%u\n",
                bubble_tip_fill[3], bubble_beside[3]);
        failed = 1;
    } else if (variant == 19 && bubble_beside[3] < 240) {
        fprintf(stderr, "FAIL: legacy rounded silhouette was changed alpha=%u\n",
                bubble_beside[3]);
        failed = 1;
    } else if (variant == 20 &&
               (bubble_tip_neon[0] < 150 || bubble_tip_neon[2] < 150 ||
                bubble_tip_neon[3] < 120 || bubble_beside[3] > 3)) {
        fprintf(stderr, "FAIL: neon does not follow bubble tail tip=%u side=%u\n",
                bubble_tip_neon[3], bubble_beside[3]);
        failed = 1;
    } else if (variant == 21 && card_cut_corner[3] > 3) {
        fprintf(stderr, "FAIL: angled card must clip diagonal alpha=%u\n",
                card_cut_corner[3]);
        failed = 1;
    } else if (variant == 22 && card_cut_corner[3] < 240) {
        fprintf(stderr, "FAIL: legacy rectangle corner regressed alpha=%u\n",
                card_cut_corner[3]);
        failed = 1;
    } else if (variant == 23 &&
               (card_diag_neon[0] < 150 || card_diag_neon[2] < 150 ||
                card_diag_neon[3] < 120)) {
        fprintf(stderr, "FAIL: neon missing on angled cut alpha=%u\n",
                card_diag_neon[3]);
        failed = 1;
    } else if (variant == 24 && fillet_endpoint[3] < 245) {
        fprintf(stderr, "FAIL: zero roundness must preserve geometric chamfer endpoint alpha=%u\n",
                fillet_endpoint[3]);
        failed = 1;
    } else if (variant == 25 &&
               (fillet_endpoint[3] > 180 || square_top_left[3] < 245 ||
                square_bottom_right[3] < 245)) {
        fprintf(stderr, "FAIL: selective rounded cut endpoint=%u squareTL=%u squareBR=%u\n",
                fillet_endpoint[3], square_top_left[3], square_bottom_right[3]);
        failed = 1;
    } else if (variant == 26 &&
               (fillet_endpoint[0] < 170 || fillet_endpoint[2] < 170 ||
                fillet_endpoint[3] < 170)) {
        fprintf(stderr, "FAIL: neon must follow the rounded diagonal fillet alpha=%u\n",
                fillet_endpoint[3]);
        failed = 1;
    } else if (variant == 27 &&
               (bubble_internal_seam[0] < 220 ||
                bubble_internal_seam[3] < 245 ||
                bubble_internal_seam[2] > 40 ||
                bubble_exposed_bottom[2] < 120)) {
        fprintf(stderr, "FAIL: bubble phantom inner line seam RGBA=(%u,%u,%u,%u), exteriorB=%u\\n",
                bubble_internal_seam[0],bubble_internal_seam[1],
                bubble_internal_seam[2],bubble_internal_seam[3],
                bubble_exposed_bottom[2]);
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
            for (int variant = 0; variant < 28; ++variant)
                missing += verify_pixel_fixture(effect, variant);
        }
        gs_effect_destroy(effect);
    }
    obs_leave_graphics();
    bfree(errors);
    obs_shutdown();
    if (missing) return 1;
    puts("PASS: libobs alpha/framing/light + bubble seam/leaf + selective card cut GPU fixtures (partial G4)");
    return 0;
}
