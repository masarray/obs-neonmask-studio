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
#include <math.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>

static const char *const uniforms[] = {
    "uv_size", "half_size", "mask_offset", "subject_pan", "subject_zoom",
    "shape_rotation", "polygon_sides", "shape_detail", "bubble_top", "bubble_bottom", "bubble_tail", "corner_radius", "shape_id", "border_width",
    "feather", "glow_radius", "glow_strength", "mid_glow_strength",
    "bloom_strength", "hotspot_strength", "hotspot_size", "color_a", "color_b",
    "color_mode", "rainbow_phase", "rainbow_saturation", "rainbow_hue_offset",
    "rainbow_spread", "color_phase", "pulse_phase", "flow_phase", "animation_id", "segment_count",
    "border_enabled", "glow_enabled", "style_id", "ornament_mode",
    "art_intensity", "art_gap", "ornament_width", "ornament_length_x",
    "ornament_length_y", "inner_rail_width", "svg_sdf", "svg_ready", "image", "ViewProj"
};


/* Partial G4: execute the real shader and read a small GPU render target.
 * This is intentionally NOT a scene/source-chain or final OBS output test.
 * Readback is permitted only in this test; the production render path has none. */
static int verify_pixel_fixture(gs_effect_t *effect, int variant)
{
    enum { W = 64, H = 64 };
    /* 38 validates D3 origin; 40 halo support; 41/42 D4B mask/overlay. */
    const bool expanded = variant == 38;
    const bool halo_guard = variant == 40;
    const bool padded = expanded || halo_guard;
    const uint32_t out_w = padded ? 96u : W;
    const uint32_t out_h = padded ? 80u : H;
    uint8_t pixels[W * H * 4];
    const uint8_t red = variant == 0 ? 255 : variant == 1 ? 128 :
                        (variant == 3 || variant == 4 || variant == 18 ||
                         variant == 19 || variant == 21 || variant == 22 ||
                         variant == 24 || variant == 25 || variant == 27 || variant == 28 ||
                          variant == 30 || variant == 32 || variant == 34 || variant == 35 || variant == 37 || variant == 38 || variant == 39 || variant == 41 || variant == 43 || variant == 45 || variant == 46 ? 255 : 0);
    const uint8_t alpha = variant == 0 ? 255 : variant == 1 ? 128 :
                          (variant == 3 || variant == 4 || variant == 18 ||
                           variant == 19 || variant == 21 || variant == 22 ||
                           variant == 24 || variant == 25 || variant == 27 || variant == 28 ||
                            variant == 30 || variant == 32 || variant == 34 || variant == 35 || variant == 37 || variant == 38 || variant == 39 || variant == 41 || variant == 43 || variant == 45 || variant == 46 ? 255 : 0);
    for (size_t i = 0; i < W * H; ++i) {
        pixels[4 * i + 0] = red;   /* premultiplied red */
        pixels[4 * i + 1] = 0;
        pixels[4 * i + 2] = 0;
        pixels[4 * i + 3] = alpha;
    }

    const uint8_t *layers[] = {pixels};
    gs_texture_t *input = gs_texture_create(W, H, GS_RGBA, 1, layers, 0);
    gs_texture_t *svg_texture = NULL;
    if(variant>=34 && variant<=37){
        enum { SIZE = 256 };
        float *field=bmalloc(SIZE*SIZE*sizeof(float));
        if(!field){fprintf(stderr,"FAIL: SVG GPU fixture allocation\\n");return 1;}
        for(unsigned y=0;y<SIZE;++y)
            for(unsigned x=0;x<SIZE;++x){
                float px=((float)x+0.5f)/(float)SIZE*48.0f-24.0f;
                float py=((float)y+0.5f)/(float)SIZE*48.0f-24.0f;
                /* Case 37 fills the viewBox; clamped edge texels are negative.
                 * Sample just OUTSIDE the local bounds to detect leaked alpha. */
                field[(size_t)y*SIZE+x]=variant==37 ?
                    fmaxf(fabsf(px),fabsf(py))-24.0f :
                    sqrtf(px*px+py*py)-16.0f;
            }
        const uint8_t *svg_layers[]={ (const uint8_t *)field };
        svg_texture=gs_texture_create(SIZE,SIZE,GS_R32F,1,svg_layers,0);
        bfree(field);
        if(!svg_texture){fprintf(stderr,"FAIL: SVG signed distance texture\\n");return 1;}
    }
    gs_texrender_t *target = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
    gs_stagesurf_t *stage = gs_stagesurface_create(out_w, out_h, GS_RGBA);
    int failed = 0;
    const bool glow_case = variant == 5 || variant == 6 || variant == 9 ||
                           variant == 10 || expanded || halo_guard || variant == 42;

    if (!input || !target || !stage) {
        fprintf(stderr, "FAIL: GPU fixture resource creation\n");
        failed = 1;
        goto done;
    }

    struct vec2 dims;
    struct vec2 output_dims;
    struct vec2 source_origin;
    struct vec2 extents;
    struct vec2 zero2;
    struct vec4 magenta;
    struct vec4 bubble_top, bubble_bottom, bubble_tail;
    vec2_set(&dims, (float)W, (float)H);
    vec2_set(&output_dims, (float)out_w, (float)out_h);
    vec2_set(&source_origin, padded ? 16.0f : 0.0f, padded ? 8.0f : 0.0f);
    vec2_set(&extents, 24.0f, 24.0f);
    vec2_set(&zero2, 0.0f, 0.0f);
    struct vec2 mask_shift;
    struct vec2 subject_shift;
    vec2_set(&mask_shift, variant == 4 ? 28.0f :
             variant == 37 ? 0.45f : expanded ? -20.0f : 0.0f, 0.0f);
    vec2_set(&subject_shift, variant == 3 ? 96.0f : 0.0f, 0.0f);
    vec4_set(&magenta, 1.0f, 0.0f, 1.0f, 1.0f);
    if (variant == 39) {
        vec4_set(&bubble_top,-0.85f,-0.90f,0.75f,-0.70f);
        vec4_set(&bubble_bottom,0.90f,0.65f,-0.65f,0.45f);
        vec4_set(&bubble_tail,0.18f,0.42f,0.28f,0.23f);
    } else {
        vec4_set(&bubble_top,-1.0f,-1.0f,1.0f,-1.0f);
        vec4_set(&bubble_bottom,1.0f,0.586f,-1.0f,0.586f);
        vec4_set(&bubble_tail,0.11f,0.33f,0.14f,0.23f);
    }
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "uv_size"), &dims);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "output_size"), &output_dims);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "input_origin"), &source_origin);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "half_size"), &extents);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "mask_offset"), &mask_shift);
    gs_effect_set_vec2(gs_effect_get_param_by_name(effect, "subject_pan"), &subject_shift);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "subject_zoom"), 1.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "shape_rotation"), 0.0f);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "polygon_sides"), 8);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "shape_detail"),
                        variant == 45 ? 0.70f : variant == 46 ? 0.35f : 0.23f);
    gs_effect_set_vec4(gs_effect_get_param_by_name(effect, "bubble_top"), &bubble_top);
    gs_effect_set_vec4(gs_effect_get_param_by_name(effect, "bubble_bottom"), &bubble_bottom);
    gs_effect_set_vec4(gs_effect_get_param_by_name(effect, "bubble_tail"), &bubble_tail);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "corner_radius"),
                        variant == 24 ? 0.0f :
                        (variant == 25 || variant == 26 ? 10.5f :
                          variant == 28 || variant == 29 ? 24.0f : 5.0f));
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "shape_id"),
                      variant == 13 || variant == 14 ? 1 :
                      variant == 15 || variant == 22 ? 5 :
                      variant == 18 || variant == 20 || variant == 27 || variant == 39 ? 8 :
                      variant >= 34 && variant <= 37 ? 12 :
                      variant == 40 ? 0 :
                      variant == 30 || variant == 31 ? 10 :
                       variant == 32 || variant == 33 ? 11 :
                      variant == 41 || variant == 42 || variant == 45 || variant == 46 ? 13 :
                      variant == 43 || variant == 44 ? 14 :
                       variant == 21 || variant == 23 || (variant >= 24 && variant <= 29) ? 9 : 0);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "border_width"), 4.0f);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "style_id"), 0);
    int art_mode = variant == 11 ? 1 : variant == 13 ? 2 :
                   variant == 15 || variant == 42 ? 3 : variant == 16 ? 4 :
                   variant == 44 ? 5 : 0;
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "ornament_mode"), art_mode);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "art_intensity"), 0.90f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "art_gap"), 2.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "ornament_width"), 6.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "ornament_length_x"), 20.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "ornament_length_y"), 20.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "inner_rail_width"), 1.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "feather"), 0.5f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "glow_radius"),
                        halo_guard ? 8.0f : (glow_case ? 12.0f : 8.0f));
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "glow_strength"),
                        glow_case ? 0.85f : 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "mid_glow_strength"), 0.75f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "bloom_strength"), variant == 6 ? 0.0f : 0.92f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "hotspot_strength"), variant == 8 ? 1.0f : 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "hotspot_size"), 0.10f);
    gs_effect_set_vec4(gs_effect_get_param_by_name(effect, "color_a"), &magenta);
    gs_effect_set_vec4(gs_effect_get_param_by_name(effect, "color_b"), &magenta);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "color_mode"), 1);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "rainbow_phase"), 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "rainbow_saturation"), 0.92f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "rainbow_hue_offset"), 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "rainbow_spread"), 1.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "color_phase"), 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "pulse_phase"),
                        variant == 9 ? 0.25f : variant == 10 ? 0.75f : 0.0f);
    gs_effect_set_float(gs_effect_get_param_by_name(effect, "flow_phase"), 0.0f);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "animation_id"),
                      variant == 8 ? 2 : (variant == 9 || variant == 10 ? 1 : 0));
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "segment_count"), 0);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "border_enabled"),
                      variant == 2 || (variant >= 5 && variant <= 17) ||
                      variant == 20 || variant == 23 || variant == 26 ||
                       variant == 27 || variant == 29 || variant == 31 || variant == 33 ||
                       variant == 35 || variant == 36 || expanded || halo_guard ||
                       variant == 42 || variant == 44);
    gs_effect_set_int(gs_effect_get_param_by_name(effect, "glow_enabled"), glow_case);
    gs_effect_set_texture(gs_effect_get_param_by_name(effect, "svg_sdf"),svg_texture);
    gs_effect_set_int(gs_effect_get_param_by_name(effect,"svg_ready"),
                      variant == 34 || variant == 36 || variant == 37 ? 1 : 0);
    gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), input);

    if (!gs_texrender_begin(target, out_w, out_h)) {
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
    gs_ortho(0.0f, (float)out_w, 0.0f, (float)out_h, -100.0f, 100.0f);
    int draw_count = 0;
    /* Standalone harness has no OBS scene traversal to establish a model
     * transform/cull state. Make the draw state deterministic. */
    gs_matrix_push();
    gs_matrix_identity();
    const enum gs_cull_mode previous_cull = gs_get_cull_mode();
    gs_set_cull_mode(GS_NEITHER);
    while (gs_effect_loop(effect, "Draw")) {
        ++draw_count;
        gs_draw_sprite(input, 0, out_w, out_h);
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
    const uint8_t *full_arc_middle = mapped + 13u * stride + 51u * 4u;
    const uint8_t *hud_cut_corner = mapped + 9u * stride + 55u * 4u;
    const uint8_t *hud_notch = mapped + 51u * stride + 15u * 4u;
    const uint8_t *hud_step_fill = mapped + 51u * stride + 20u * 4u;
    const uint8_t *hud_step_neon = mapped + 53u * stride + 16u * 4u;
    const uint8_t *squircle_corner = mapped + 55u * stride + 55u * 4u;
    const uint8_t *squircle_inside = mapped + 50u * stride + 50u * 4u;
    const uint8_t *squircle_axis_neon = mapped + 32u * stride + 56u * 4u;
    const uint8_t *svg_outside = mapped + 32u * stride + 55u * 4u;
    const uint8_t *svg_edge = mapped + 32u * stride + 48u * 4u;
    const uint8_t *svg_viewbox_just_outside = mapped + 32u * stride + 56u * 4u;
    const uint8_t *svg_viewbox_just_inside = mapped + 32u * stride + 54u * 4u;
    const uint8_t *card_diag_neon = mapped + 12u * stride + 52u * 4u;
    const uint8_t *fillet_endpoint = mapped + 8u * stride + 46u * 4u;
    const uint8_t *square_top_left = mapped + 8u * stride + 8u * 4u;
    const uint8_t *square_bottom_right = mapped + 55u * stride + 55u * 4u;
    const uint8_t *tech_tl = mapped + 9u * stride + 9u * 4u;
    const uint8_t *tech_tr = mapped + 9u * stride + 55u * 4u;
    const uint8_t *tech_br = mapped + 55u * stride + 55u * 4u;
    const uint8_t *tech_bl = mapped + 55u * stride + 9u * 4u;
    const uint8_t *tech_tr_bracket = mapped + 3u * stride + 61u * 4u;
    const uint8_t *tech_bl_bracket = mapped + 61u * stride + 3u * 4u;
    const uint8_t *tech_diag_overlay = mapped + 9u * stride + 9u * 4u;
    const uint8_t *tech_detail_probe = mapped + 12u * stride + 12u * 4u;
    /* Sample the horizontal arms rather than the exact render-target corner;
     * the latter is intentionally clipped by the tiny 64x64 fixture edge. */
    const uint8_t *game_corner_tl = mapped + 3u * stride + 13u * 4u;
    const uint8_t *game_corner_tr = mapped + 3u * stride + 51u * 4u;
    const uint8_t *game_corner_br = mapped + 61u * stride + 51u * 4u;
    const uint8_t *game_corner_bl = mapped + 61u * stride + 13u * 4u;
    const uint8_t *game_mask_tl = mapped + 10u * stride + 10u * 4u;
    const uint8_t *game_mask_top = mapped + 10u * stride + 14u * 4u;
    const uint8_t *game_inner_top = mapped + 15u * stride + 32u * 4u;


    /* The black/transparent corner proves the mask is not an opaque box.
     * Semi-transparent center checks the premultiplied-input convention. */
    if (!glow_case && variant != 44 &&
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
    } else if (variant == 28 &&
               (fillet_endpoint[3] > 3 ||
                full_arc_middle[0] < 240 || full_arc_middle[3] < 245 ||
                square_top_left[3] < 245 || square_bottom_right[3] < 245)) {
        fprintf(stderr, "FAIL: full Angled Card arc must clip old shoulder, keep middle and square corners: old=%u mid=%u TL=%u BR=%u\\n",
                fillet_endpoint[3], full_arc_middle[3],
                square_top_left[3], square_bottom_right[3]);
        failed = 1;
    } else if (variant == 29 &&
               (full_arc_middle[0] < 150 || full_arc_middle[2] < 150 ||
                full_arc_middle[3] < 120)) {
        fprintf(stderr, "FAIL: neon must follow full quarter arc at max roundness alpha=%u\\n",
                full_arc_middle[3]);
        failed = 1;
    } else if (variant == 30 &&
               (hud_cut_corner[3] > 3 || hud_notch[3] > 3 ||
                hud_step_fill[0] < 240 || hud_step_fill[3] < 245)) {
        fprintf(stderr, "FAIL: HUD panel must clip corners/notch and fill step: top=%u notch=%u step=%u\\n",
                hud_cut_corner[3],hud_notch[3],hud_step_fill[3]);
        failed = 1;
    } else if (variant == 31 &&
               (hud_step_neon[0] < 140 || hud_step_neon[2] < 140 ||
                hud_step_neon[3] < 105)) {
        fprintf(stderr, "FAIL: HUD neon must track non-convex step alpha=%u\\n",
                hud_step_neon[3]);
        failed = 1;
    } else if (variant == 32 &&
               (squircle_corner[3] > 3 ||
                squircle_inside[0] < 240 || squircle_inside[3] < 245)) {
        fprintf(stderr, "FAIL: squircle needs curved corner and filled inside corner=%u inside=%u\\n",
                squircle_corner[3],squircle_inside[3]);
        failed = 1;
    } else if (variant == 33 &&
               (squircle_axis_neon[0] < 150 || squircle_axis_neon[2] < 150 ||
                squircle_axis_neon[3] < 120)) {
        fprintf(stderr, "FAIL: squircle neon must track superellipse contour alpha=%u\\n",
                squircle_axis_neon[3]);
        failed = 1;
    } else if(variant==34 &&
               (center[0]<240 || center[3]<245 || svg_outside[3]>3)) {
        fprintf(stderr,"FAIL: imported SVG should clip source and fill center: center=%u outside=%u\\n",
                center[3],svg_outside[3]);
        failed=1;
    } else if(variant==35 &&
               (center[0]>2 || center[3]>2 || svg_edge[3]>2)) {
        fprintf(stderr,"FAIL: invalid SVG must fail closed even with border: center=%u edge=%u\\n",
                center[3],svg_edge[3]);
        failed=1;
    } else if(variant==36 &&
               (svg_edge[0]<150 || svg_edge[2]<150 || svg_edge[3]<120)) {
        fprintf(stderr,"FAIL: SVG neon not following SDF contour alpha=%u\\n",
                svg_edge[3]);
        failed=1;
    }

    else if(variant==37 &&
            (svg_viewbox_just_outside[3]>130 ||
             svg_viewbox_just_inside[3]<245)) {
        fprintf(stderr,"FAIL: filled SVG viewBox leaks clamped negative SDF outside: outer=%u inner=%u\n",
                svg_viewbox_just_outside[3],svg_viewbox_just_inside[3]);
        failed=1;
    }

    else if (expanded) {
        /* A real padding pixel before input X=16 must show only the halo;
         * the input itself is not stretched/clamped into the new margin.
         * Source center (input 32,32) must move to output (48,40). */
        const uint8_t *padded_rim = mapped + 40u*stride + 4u*4u;
        const uint8_t *padded_inner = mapped + 40u*stride + 11u*4u;
        const uint8_t *padded_video = mapped + 40u*stride + 28u*4u;
        const uint8_t *padded_far = mapped + 40u*stride + 95u*4u;
        if (padded_rim[2] < 120 || padded_rim[3] < 140 ||
            padded_inner[2] < 20 || padded_inner[3] > 100 ||
            padded_video[0] < 240 || padded_video[3] < 240 ||
            padded_video[2] > 40 || padded_far[3] > 2) {
            fprintf(stderr, "FAIL: expanded input-origin mapping rim=%u/%u pad=%u/%u video=%u/%u/%u far=%u\n",
                    padded_rim[2],padded_rim[3],padded_inner[2],padded_inner[3],
                    padded_video[0],padded_video[2],padded_video[3],padded_far[3]);
            failed=1;
        }
    }

    else if(variant==39) {
        const uint8_t *top_old=mapped+8u*stride+32u*4u;
        const uint8_t *left_old=mapped+32u*stride+8u*4u;
        const uint8_t *right_old=mapped+32u*stride+57u*4u;
        const uint8_t *tail_fill=mapped+48u*stride+27u*4u;
        const uint8_t *tail_side=mapped+50u*stride+47u*4u;
        if(center[0]<240 || center[3]<245 ||
           top_old[3]>3 || left_old[3]>3 || right_old[3]>3 ||
           tail_fill[0]<230 || tail_fill[3]<230 || tail_side[3]>3) {
            fprintf(stderr,"FAIL: D4A freeform bubble center=%u top=%u left=%u right=%u tail=%u side=%u\n",
                    center[3],top_old[3],left_old[3],right_old[3],
                    tail_fill[3],tail_side[3]);
            failed=1;
        }
    }

    else if(halo_guard) {
        const uint8_t *colored=mapped+40u*stride+82u*4u;
        const uint8_t *far=mapped+40u*stride+95u*4u;
        if(colored[3]<5 || colored[0]<=colored[1]+4 ||
           colored[2]<=colored[1]+4 ||
           far[0]>2 || far[1]>2 || far[2]>2 || far[3]>2) {
            fprintf(stderr,"FAIL: finite chromatic halo color=%u/%u/%u/%u far=%u/%u/%u/%u\n",
                    colored[0],colored[1],colored[2],colored[3],
                    far[0],far[1],far[2],far[3]);
            failed=1;
        }
    }

    else if(variant==41) {
        if(center[0]<240 || center[3]<245 ||
           tech_tl[3]>3 || tech_br[3]>3 ||
           tech_tr[0]<240 || tech_tr[3]<245 ||
           tech_bl[0]<240 || tech_bl[3]<245) {
            fprintf(stderr,"FAIL: Tech HUD mask center=%u TL=%u TR=%u BR=%u BL=%u\n",
                    center[3],tech_tl[3],tech_tr[3],tech_br[3],tech_bl[3]);
            failed=1;
        }
    }

    else if(variant==42) {
        if(center[3]>3 ||
           tech_tr_bracket[3]<120 || tech_bl_bracket[3]<120 ||
           tech_diag_overlay[3]<80) {
            fprintf(stderr,"FAIL: Tech HUD overlay center=%u TR=%u BL=%u diag=%u\n",
                    center[3],tech_tr_bracket[3],tech_bl_bracket[3],
                    tech_diag_overlay[3]);
            failed=1;
        }
    }

    else if(variant==43) {
        if(center[0]<240 || center[3]<245 ||
           game_mask_tl[0]<220 || game_mask_tl[3]<220 ||
           game_mask_top[0]<220 || game_mask_top[3]<220) {
            fprintf(stderr,"FAIL: clean Game UI rectangle center=%u TL=%u top=%u\n",
                    center[3],game_mask_tl[3],game_mask_top[3]);
            failed=1;
        }
    }

    else if(variant==44) {
        if(center[3]>3 || game_inner_top[3]<25 ||
           game_corner_tl[3]<80 || game_corner_tr[3]<80 ||
           game_corner_br[3]<80 || game_corner_bl[3]<80) {
            fprintf(stderr,"FAIL: Game UI ornament center=%u inner=%u corners=%u/%u/%u/%u\n",
                    center[3],game_inner_top[3],game_corner_tl[3],
                    game_corner_tr[3],game_corner_br[3],game_corner_bl[3]);
            failed=1;
        }
    }

    else if(variant==45) {
        if(tech_detail_probe[3]>3) {
            fprintf(stderr,"FAIL: D4K Tech HUD 0.70 detail did not deepen cut alpha=%u\n",
                    tech_detail_probe[3]);
            failed=1;
        }
    }

    else if(variant==46) {
        if(tech_detail_probe[0]<240 || tech_detail_probe[3]<245) {
            fprintf(stderr,"FAIL: D4K legacy Tech HUD 0.35 geometry drifted alpha=%u\n",
                    tech_detail_probe[3]);
            failed=1;
        }
    }

    gs_stagesurface_unmap(stage);
done:
    if (stage) gs_stagesurface_destroy(stage);
    if (target) gs_texrender_destroy(target);
    if (input) gs_texture_destroy(input);
    if (svg_texture) gs_texture_destroy(svg_texture);
    return failed;
}

/* D4B.1 regression: the old bracket used an approximately 1.5px source-space
 * stroke. It passed the 64x64 fixture yet disappeared when a normal camera was
 * downscaled in OBS preview. Exercise a 320x180 source and require alpha three
 * pixels away from the bracket centerline, which the old stroke cannot meet. */
static int verify_tech_hud_preview_scale(gs_effect_t *effect)
{
    enum { W=320, H=180 };
    uint8_t *pixels=bzalloc((size_t)W*H*4u);
    if(!pixels) return 1;
    const uint8_t *layers[]={pixels};
    gs_texture_t *input=gs_texture_create(W,H,GS_RGBA,1,layers,0);
    bfree(pixels);
    gs_texrender_t *target=gs_texrender_create(GS_RGBA,GS_ZS_NONE);
    gs_stagesurf_t *stage=gs_stagesurface_create(W,H,GS_RGBA);
    if(!input||!target||!stage){
        if(stage)gs_stagesurface_destroy(stage);
        if(target)gs_texrender_destroy(target);
        if(input)gs_texture_destroy(input);
        fprintf(stderr,"FAIL: Tech HUD 320x180 resources\n");
        return 1;
    }

    struct vec2 dims,zero,half;
    struct vec4 magenta,btop,bbottom,btail;
    vec2_set(&dims,(float)W,(float)H);
    vec2_set(&zero,0.0f,0.0f);
    vec2_set(&half,90.0f,45.0f);
    vec4_set(&magenta,1.0f,0.0f,1.0f,1.0f);
    vec4_set(&btop,-1,-1,1,-1);
    vec4_set(&bbottom,1,0.6f,-1,0.6f);
    vec4_set(&btail,0.11f,0.33f,0.14f,0.22f);
#define P(name) gs_effect_get_param_by_name(effect,name)
    gs_effect_set_vec2(P("uv_size"),&dims);
    gs_effect_set_vec2(P("output_size"),&dims);
    gs_effect_set_vec2(P("input_origin"),&zero);
    gs_effect_set_vec2(P("half_size"),&half);
    gs_effect_set_vec2(P("mask_offset"),&zero);
    gs_effect_set_vec2(P("subject_pan"),&zero);
    gs_effect_set_float(P("subject_zoom"),1.0f);
    gs_effect_set_float(P("shape_rotation"),0.0f);
    gs_effect_set_int(P("polygon_sides"),8);
    gs_effect_set_float(P("shape_detail"),0.12f);
    gs_effect_set_vec4(P("bubble_top"),&btop);
    gs_effect_set_vec4(P("bubble_bottom"),&bbottom);
    gs_effect_set_vec4(P("bubble_tail"),&btail);
    gs_effect_set_float(P("corner_radius"),0.0f);
    gs_effect_set_int(P("shape_id"),13);
    gs_effect_set_float(P("border_width"),5.0f);
    gs_effect_set_int(P("style_id"),3);
    gs_effect_set_int(P("ornament_mode"),3);
    gs_effect_set_float(P("art_intensity"),1.0f);
    gs_effect_set_float(P("art_gap"),24.0f);
    gs_effect_set_float(P("ornament_width"),18.0f);
    gs_effect_set_float(P("ornament_length_x"),84.0f);
    gs_effect_set_float(P("ornament_length_y"),68.0f);
    gs_effect_set_float(P("inner_rail_width"),1.6f);
    gs_effect_set_float(P("feather"),0.5f);
    gs_effect_set_float(P("glow_radius"),14.0f);
    gs_effect_set_float(P("glow_strength"),0.0f);
    gs_effect_set_float(P("mid_glow_strength"),0.72f);
    gs_effect_set_float(P("bloom_strength"),0.76f);
    gs_effect_set_float(P("hotspot_strength"),0.0f);
    gs_effect_set_float(P("hotspot_size"),0.09f);
    gs_effect_set_vec4(P("color_a"),&magenta);
    gs_effect_set_vec4(P("color_b"),&magenta);
    gs_effect_set_int(P("color_mode"),1);
    gs_effect_set_float(P("rainbow_phase"),0.0f);
    gs_effect_set_float(P("rainbow_saturation"),0.92f);
    gs_effect_set_float(P("rainbow_hue_offset"),0.0f);
    gs_effect_set_float(P("rainbow_spread"),1.0f);
    gs_effect_set_float(P("color_phase"),0.0f);
    gs_effect_set_float(P("pulse_phase"),0.0f);
    gs_effect_set_float(P("flow_phase"),0.0f);
    gs_effect_set_int(P("animation_id"),0);
    gs_effect_set_int(P("segment_count"),0);
    gs_effect_set_int(P("border_enabled"),1);
    gs_effect_set_int(P("glow_enabled"),0);
    gs_effect_set_texture(P("svg_sdf"),NULL);
    gs_effect_set_int(P("svg_ready"),0);
    gs_effect_set_texture(P("image"),input);
#undef P

    int failed=0;
    if(!gs_texrender_begin(target,W,H)){failed=1;goto done_hires;}
    const bool srgb=gs_framebuffer_srgb_enabled();
    gs_enable_framebuffer_srgb(false);
    gs_blend_state_push(); gs_enable_blending(false);
    struct vec4 clear; vec4_zero(&clear);
    gs_clear(GS_CLEAR_COLOR,&clear,0.0f,0);
    gs_ortho(0,(float)W,0,(float)H,-100,100);
    gs_matrix_push(); gs_matrix_identity();
    const enum gs_cull_mode cull=gs_get_cull_mode(); gs_set_cull_mode(GS_NEITHER);
    while(gs_effect_loop(effect,"Draw")) gs_draw_sprite(input,0,W,H);
    gs_set_cull_mode(cull); gs_matrix_pop();
    gs_blend_state_pop(); gs_enable_framebuffer_srgb(srgb);
    gs_texrender_end(target);
    gs_stage_texture(stage,gs_texrender_get_texture(target));
    uint8_t *mapped=NULL; uint32_t stride=0;
    if(!gs_stagesurface_map(stage,&mapped,&stride)){failed=1;goto done_hires;}

    /* D4F Tech grammar: only TR + BL are dominant outer Ls. The true
     * 24px empty gap remains dark between base frame and outer bar. */
    const uint8_t *tr_h=mapped+12u*stride+250u*4u;
    /* D4G: inside the outer elbow quadrant that was missing in D4F. */
    const uint8_t *tr_joint=mapped+5u*stride+289u*4u;
    const uint8_t *tr_h_thick=mapped+18u*stride+250u*4u;
    const uint8_t *tr_v=mapped+40u*stride+283u*4u;
    const uint8_t *bl_h=mapped+168u*stride+70u*4u;
    const uint8_t *bl_v=mapped+140u*stride+37u*4u;
    const uint8_t *tl_forbidden=mapped+12u*stride+37u*4u;
    const uint8_t *br_forbidden=mapped+168u*stride+283u*4u;
    const uint8_t *true_gap=mapped+32u*stride+250u*4u;
    const uint8_t *quiet_center=mapped+90u*stride+160u*4u;
    if(tr_h[3]<190 || tr_joint[3]<150 ||
       tr_h_thick[3]<130 || tr_v[3]<190 ||
       bl_h[3]<190 || bl_v[3]<190 ||
       tl_forbidden[3]>15 || br_forbidden[3]>15 ||
       true_gap[3]>15 || quiet_center[3]>3) {
        fprintf(stderr,
                "FAIL: D4G Tech connected joint TR=%u joint=%u thick=%u V=%u BL=%u/%u forbidden=%u/%u gap=%u center=%u\n",
                tr_h[3],tr_joint[3],tr_h_thick[3],tr_v[3],bl_h[3],bl_v[3],
                tl_forbidden[3],br_forbidden[3],true_gap[3],quiet_center[3]);
        failed=1;
    }
    gs_stagesurface_unmap(stage);
done_hires:
    gs_stagesurface_destroy(stage);
    gs_texrender_destroy(target);
    gs_texture_destroy(input);
    return failed;
}

/* D4C acceptance at preview scale: the four L corners and the true inner
 * rail must remain visible at 320x180 without relying on bloom. */
static int verify_game_ui_preview_scale(gs_effect_t *effect)
{
    enum { W=320,H=180 };
    uint8_t *pixels=bzalloc((size_t)W*H*4u);
    if(!pixels) return 1;
    const uint8_t *layers[]={pixels};
    gs_texture_t *input=gs_texture_create(W,H,GS_RGBA,1,layers,0);
    bfree(pixels);
    gs_texrender_t *target=gs_texrender_create(GS_RGBA,GS_ZS_NONE);
    gs_stagesurf_t *stage=gs_stagesurface_create(W,H,GS_RGBA);
    if(!input||!target||!stage){
        if(stage)gs_stagesurface_destroy(stage);
        if(target)gs_texrender_destroy(target);
        if(input)gs_texture_destroy(input);
        fprintf(stderr,"FAIL: Game UI 320x180 resources\n");
        return 1;
    }
    struct vec2 dims,zero,half;
    struct vec4 green,btop,bbottom,btail;
    vec2_set(&dims,(float)W,(float)H); vec2_set(&zero,0,0);
    vec2_set(&half,90.0f,45.0f);
    vec4_set(&green,0.2f,1.0f,0.25f,1.0f);
    vec4_set(&btop,-1,-1,1,-1); vec4_set(&bbottom,1,.6f,-1,.6f);
    vec4_set(&btail,.11f,.33f,.14f,.22f);
#define P(name) gs_effect_get_param_by_name(effect,name)
    gs_effect_set_vec2(P("uv_size"),&dims);
    gs_effect_set_vec2(P("output_size"),&dims);
    gs_effect_set_vec2(P("input_origin"),&zero);
    gs_effect_set_vec2(P("half_size"),&half);
    gs_effect_set_vec2(P("mask_offset"),&zero);
    gs_effect_set_vec2(P("subject_pan"),&zero);
    gs_effect_set_float(P("subject_zoom"),1.0f);
    gs_effect_set_float(P("shape_rotation"),0.0f);
    gs_effect_set_int(P("polygon_sides"),8);
    gs_effect_set_float(P("shape_detail"),0.14f);
    gs_effect_set_vec4(P("bubble_top"),&btop);
    gs_effect_set_vec4(P("bubble_bottom"),&bbottom);
    gs_effect_set_vec4(P("bubble_tail"),&btail);
    gs_effect_set_float(P("corner_radius"),0.0f);
    gs_effect_set_int(P("shape_id"),14);
    gs_effect_set_float(P("border_width"),5.0f);
    gs_effect_set_int(P("style_id"),0);
    gs_effect_set_int(P("ornament_mode"),5);
    gs_effect_set_float(P("art_intensity"),1.0f);
    gs_effect_set_float(P("art_gap"),14.0f);
    gs_effect_set_float(P("ornament_width"),16.0f);
    gs_effect_set_float(P("ornament_length_x"),76.0f);
    gs_effect_set_float(P("ornament_length_y"),64.0f);
    gs_effect_set_float(P("inner_rail_width"),1.4f);
    gs_effect_set_float(P("feather"),0.5f);
    gs_effect_set_float(P("glow_radius"),12.0f);
    gs_effect_set_float(P("glow_strength"),0.0f);
    gs_effect_set_float(P("mid_glow_strength"),0.8f);
    gs_effect_set_float(P("bloom_strength"),0.84f);
    gs_effect_set_float(P("hotspot_strength"),0.0f);
    gs_effect_set_float(P("hotspot_size"),0.1f);
    gs_effect_set_vec4(P("color_a"),&green);
    gs_effect_set_vec4(P("color_b"),&green);
    gs_effect_set_int(P("color_mode"),1);
    gs_effect_set_float(P("rainbow_phase"),0.0f);
    gs_effect_set_float(P("rainbow_saturation"),0.92f);
    gs_effect_set_float(P("rainbow_hue_offset"),0.0f);
    gs_effect_set_float(P("rainbow_spread"),1.0f);
    gs_effect_set_float(P("color_phase"),0);
    gs_effect_set_float(P("pulse_phase"),0);
    gs_effect_set_float(P("flow_phase"),0);
    gs_effect_set_int(P("animation_id"),0);
    gs_effect_set_int(P("segment_count"),0);
    gs_effect_set_int(P("border_enabled"),1);
    gs_effect_set_int(P("glow_enabled"),0);
    gs_effect_set_texture(P("svg_sdf"),NULL);
    gs_effect_set_int(P("svg_ready"),0);
    gs_effect_set_texture(P("image"),input);
#undef P

    int failed=0;
    if(!gs_texrender_begin(target,W,H)){failed=1;goto done_game;}
    const bool srgb=gs_framebuffer_srgb_enabled();
    gs_enable_framebuffer_srgb(false); gs_blend_state_push(); gs_enable_blending(false);
    struct vec4 clear; vec4_zero(&clear); gs_clear(GS_CLEAR_COLOR,&clear,0,0);
    gs_ortho(0,(float)W,0,(float)H,-100,100);
    gs_matrix_push(); gs_matrix_identity();
    const enum gs_cull_mode cull=gs_get_cull_mode(); gs_set_cull_mode(GS_NEITHER);
    while(gs_effect_loop(effect,"Draw")) gs_draw_sprite(input,0,W,H);
    gs_set_cull_mode(cull); gs_matrix_pop(); gs_blend_state_pop();
    gs_enable_framebuffer_srgb(srgb); gs_texrender_end(target);
    gs_stage_texture(stage,gs_texrender_get_texture(target));
    uint8_t *mapped=NULL; uint32_t stride=0;
    if(!gs_stagesurface_map(stage,&mapped,&stride)){failed=1;goto done_game;}

    /* D4F Game UI: all four Ls stay thick while the inset rail stays
     * hairline. The true 14px empty outer gap is also sampled. */
    const uint8_t *tl_h=mapped+23u*stride+70u*4u;
    const uint8_t *tl_h_thick=mapped+29u*stride+70u*4u;
    const uint8_t *tl_v=mapped+50u*stride+48u*4u;
    const uint8_t *tr_h=mapped+23u*stride+250u*4u;
    const uint8_t *tr_v=mapped+50u*stride+272u*4u;
    const uint8_t *bl_h=mapped+157u*stride+70u*4u;
    const uint8_t *bl_v=mapped+130u*stride+48u*4u;
    const uint8_t *br_h=mapped+157u*stride+250u*4u;
    const uint8_t *br_v=mapped+130u*stride+272u*4u;
    /* D4G: BR elbow-square quadrant that D4F left empty. */
    const uint8_t *br_joint=mapped+161u*stride+276u*4u;
    const uint8_t *innerTop=mapped+54u*stride+160u*4u;
    const uint8_t *innerTopOff=mapped+57u*stride+160u*4u;
    const uint8_t *true_gap=mapped+38u*stride+70u*4u;
    const uint8_t *center=mapped+90u*stride+160u*4u;
    if(tl_h[3]<190||tl_h_thick[3]<120||tl_v[3]<190||
       tr_h[3]<190||tr_v[3]<190||bl_h[3]<190||bl_v[3]<190||
       br_h[3]<190||br_v[3]<190||br_joint[3]<150||
       innerTop[3]<40||innerTopOff[3]>35||true_gap[3]>15||center[3]>3||
       tl_h_thick[3] <= innerTop[3]+40){
        fprintf(stderr,
                "FAIL: D4G Game connected outer=%u/%u/%u/%u/%u/%u/%u/%u/%u joint=%u inner=%u off=%u gap=%u center=%u\n",
                tl_h[3],tl_h_thick[3],tl_v[3],tr_h[3],tr_v[3],
                bl_h[3],bl_v[3],br_h[3],br_v[3],br_joint[3],innerTop[3],
                innerTopOff[3],true_gap[3],center[3]);
        failed=1;
    }
    gs_stagesurface_unmap(stage);
done_game:
    gs_stagesurface_destroy(stage); gs_texrender_destroy(target);
    gs_texture_destroy(input);
    return failed;
}


/* D4J regression: reproduce the large Game UI controls that previously
 * clipped at the source top, but render into a genuinely padded output.
 * The outer horizontal L arm must light pixels ABOVE the original input
 * origin; a dark true-gap pixel proves this is ornament geometry rather than
 * stretched/clamped webcam content. */
static int verify_game_ui_expanded_top_padding(gs_effect_t *effect)
{
    enum { W=320, H=180, PAD_X=40, PAD_Y=52,
           OUT_W=W+PAD_X*2, OUT_H=H+PAD_Y*2 };
    uint8_t *pixels=bzalloc((size_t)W*H*4u);
    if(!pixels) return 1;
    const uint8_t *layers[]={pixels};
    gs_texture_t *input=gs_texture_create(W,H,GS_RGBA,1,layers,0);
    bfree(pixels);
    gs_texrender_t *target=gs_texrender_create(GS_RGBA,GS_ZS_NONE);
    gs_stagesurf_t *stage=gs_stagesurface_create(OUT_W,OUT_H,GS_RGBA);
    if(!input||!target||!stage){
        if(stage)gs_stagesurface_destroy(stage);
        if(target)gs_texrender_destroy(target);
        if(input)gs_texture_destroy(input);
        fprintf(stderr,"FAIL: D4J expanded Game UI resources\n");
        return 1;
    }

    struct vec2 dims,out_dims,origin,half,zero;
    struct vec4 green,btop,bbottom,btail;
    vec2_set(&dims,(float)W,(float)H);
    vec2_set(&out_dims,(float)OUT_W,(float)OUT_H);
    vec2_set(&origin,(float)PAD_X,(float)PAD_Y);
    vec2_set(&half,128.0f,72.0f);
    vec2_set(&zero,0.0f,0.0f);
    vec4_set(&green,0.2f,1.0f,0.25f,1.0f);
    vec4_set(&btop,-1,-1,1,-1);
    vec4_set(&bbottom,1,.6f,-1,.6f);
    vec4_set(&btail,.11f,.33f,.14f,.22f);
#define P(name) gs_effect_get_param_by_name(effect,name)
    gs_effect_set_vec2(P("uv_size"),&dims);
    gs_effect_set_vec2(P("output_size"),&out_dims);
    gs_effect_set_vec2(P("input_origin"),&origin);
    gs_effect_set_vec2(P("half_size"),&half);
    gs_effect_set_vec2(P("mask_offset"),&zero);
    gs_effect_set_vec2(P("subject_pan"),&zero);
    gs_effect_set_float(P("subject_zoom"),1.0f);
    gs_effect_set_float(P("shape_rotation"),0.0f);
    gs_effect_set_int(P("polygon_sides"),8);
    gs_effect_set_float(P("shape_detail"),0.14f);
    gs_effect_set_vec4(P("bubble_top"),&btop);
    gs_effect_set_vec4(P("bubble_bottom"),&bbottom);
    gs_effect_set_vec4(P("bubble_tail"),&btail);
    gs_effect_set_float(P("corner_radius"),0.0f);
    gs_effect_set_int(P("shape_id"),14);
    gs_effect_set_float(P("border_width"),22.5f);
    gs_effect_set_int(P("style_id"),0);
    gs_effect_set_int(P("ornament_mode"),5);
    gs_effect_set_float(P("art_intensity"),1.0f);
    gs_effect_set_float(P("art_gap"),30.0f);
    gs_effect_set_float(P("ornament_width"),38.5f);
    gs_effect_set_float(P("ornament_length_x"),76.0f);
    gs_effect_set_float(P("ornament_length_y"),64.0f);
    gs_effect_set_float(P("inner_rail_width"),1.4f);
    gs_effect_set_float(P("feather"),0.5f);
    gs_effect_set_float(P("glow_radius"),12.0f);
    gs_effect_set_float(P("glow_strength"),0.0f);
    gs_effect_set_float(P("mid_glow_strength"),0.8f);
    gs_effect_set_float(P("bloom_strength"),0.84f);
    gs_effect_set_float(P("hotspot_strength"),0.0f);
    gs_effect_set_float(P("hotspot_size"),0.1f);
    gs_effect_set_vec4(P("color_a"),&green);
    gs_effect_set_vec4(P("color_b"),&green);
    gs_effect_set_int(P("color_mode"),1);
    gs_effect_set_float(P("rainbow_phase"),0.0f);
    gs_effect_set_float(P("rainbow_saturation"),0.92f);
    gs_effect_set_float(P("rainbow_hue_offset"),0.0f);
    gs_effect_set_float(P("rainbow_spread"),1.0f);
    gs_effect_set_float(P("color_phase"),0.0f);
    gs_effect_set_float(P("pulse_phase"),0.0f);
    gs_effect_set_float(P("flow_phase"),0.0f);
    gs_effect_set_int(P("animation_id"),0);
    gs_effect_set_int(P("segment_count"),0);
    gs_effect_set_int(P("border_enabled"),1);
    gs_effect_set_int(P("glow_enabled"),0);
    gs_effect_set_texture(P("svg_sdf"),NULL);
    gs_effect_set_int(P("svg_ready"),0);
    gs_effect_set_texture(P("image"),input);
#undef P

    int failed=0;
    if(!gs_texrender_begin(target,OUT_W,OUT_H)){
        failed=1; goto done_expanded_game;
    }
    const bool srgb=gs_framebuffer_srgb_enabled();
    gs_enable_framebuffer_srgb(false);
    gs_blend_state_push(); gs_enable_blending(false);
    struct vec4 clear; vec4_zero(&clear);
    gs_clear(GS_CLEAR_COLOR,&clear,0,0);
    gs_ortho(0,(float)OUT_W,0,(float)OUT_H,-100,100);
    gs_matrix_push(); gs_matrix_identity();
    const enum gs_cull_mode cull=gs_get_cull_mode();
    gs_set_cull_mode(GS_NEITHER);
    while(gs_effect_loop(effect,"Draw"))
        gs_draw_sprite(input,0,OUT_W,OUT_H);
    gs_set_cull_mode(cull);
    gs_matrix_pop();
    gs_blend_state_pop();
    gs_enable_framebuffer_srgb(srgb);
    gs_texrender_end(target);

    gs_stage_texture(stage,gs_texrender_get_texture(target));
    uint8_t *mapped=NULL; uint32_t stride=0;
    if(!gs_stagesurface_map(stage,&mapped,&stride)){
        failed=1; goto done_expanded_game;
    }

    /* Original input starts at y=52. The top L sample at y=12 is therefore
     * unquestionably in added output padding. y=55 lies in the authored
     * 30px negative-space gap between that L and the source frame. */
    const uint8_t *top_l=mapped+12u*stride+350u*4u;
    const uint8_t *true_gap=mapped+55u*stride+350u*4u;
    const uint8_t *center=mapped+142u*stride+200u*4u;
    if(top_l[3]<190 || true_gap[3]>15 || center[3]>3){
        fprintf(stderr,
                "FAIL: D4J expanded top ornament alpha=%u gap=%u center=%u\n",
                top_l[3],true_gap[3],center[3]);
        failed=1;
    }
    gs_stagesurface_unmap(stage);

done_expanded_game:
    gs_stagesurface_destroy(stage);
    gs_texrender_destroy(target);
    gs_texture_destroy(input);
    return failed;
}

/* P6B motion gate: two 240-degree circle rails must derive opposite
 * directions from one bounded flow phase. Positive screen-space atan2 turns
 * clockwise, so +phase moves the outer gap CW while -phase moves the inner
 * gap CCW. Samples are on each rail centerline with glow disabled, ensuring
 * the result is ring geometry rather than base-border spill. */
static int verify_counter_rotating_dual_ring(gs_effect_t *effect)
{
    enum { W=320, H=320 };
    uint8_t *pixels=bzalloc((size_t)W*H*4u);
    if(!pixels) return 1;
    const uint8_t *layers[]={pixels};
    gs_texture_t *input=gs_texture_create(W,H,GS_RGBA,1,layers,0);
    bfree(pixels);
    gs_texrender_t *target=gs_texrender_create(GS_RGBA,GS_ZS_NONE);
    gs_stagesurf_t *stage=gs_stagesurface_create(W,H,GS_RGBA);
    if(!input||!target||!stage){
        if(stage)gs_stagesurface_destroy(stage);
        if(target)gs_texrender_destroy(target);
        if(input)gs_texture_destroy(input);
        fprintf(stderr,"FAIL: P6B dual-ring resources\n");
        return 1;
    }

    struct vec2 dims,zero,half;
    struct vec4 cyan,magenta,btop,bbottom,btail;
    vec2_set(&dims,(float)W,(float)H);
    vec2_set(&zero,0.0f,0.0f);
    vec2_set(&half,100.0f,100.0f);
    vec4_set(&cyan,0.0f,0.86f,1.0f,1.0f);
    vec4_set(&magenta,1.0f,0.16f,0.86f,1.0f);
    vec4_set(&btop,-1,-1,1,-1);
    vec4_set(&bbottom,1,.6f,-1,.6f);
    vec4_set(&btail,.11f,.33f,.14f,.22f);
#define P(name) gs_effect_get_param_by_name(effect,name)
    gs_effect_set_vec2(P("uv_size"),&dims);
    gs_effect_set_vec2(P("output_size"),&dims);
    gs_effect_set_vec2(P("input_origin"),&zero);
    gs_effect_set_vec2(P("half_size"),&half);
    gs_effect_set_vec2(P("mask_offset"),&zero);
    gs_effect_set_vec2(P("subject_pan"),&zero);
    gs_effect_set_float(P("subject_zoom"),1.0f);
    gs_effect_set_float(P("shape_rotation"),0.0f);
    gs_effect_set_int(P("polygon_sides"),8);
    gs_effect_set_float(P("shape_detail"),0.22f);
    gs_effect_set_vec4(P("bubble_top"),&btop);
    gs_effect_set_vec4(P("bubble_bottom"),&bbottom);
    gs_effect_set_vec4(P("bubble_tail"),&btail);
    gs_effect_set_float(P("corner_radius"),0.0f);
    gs_effect_set_int(P("shape_id"),1);
    gs_effect_set_float(P("border_width"),2.5f);
    gs_effect_set_int(P("style_id"),3);
    gs_effect_set_int(P("ornament_mode"),6);
    gs_effect_set_float(P("art_intensity"),1.0f);
    gs_effect_set_float(P("art_gap"),5.0f);
    gs_effect_set_float(P("ornament_width"),6.0f);
    gs_effect_set_float(P("ornament_length_x"),60.0f);
    gs_effect_set_float(P("ornament_length_y"),48.0f);
    gs_effect_set_float(P("inner_rail_width"),4.0f);
    gs_effect_set_float(P("feather"),0.70f);
    gs_effect_set_float(P("glow_radius"),16.0f);
    gs_effect_set_float(P("glow_strength"),0.0f);
    gs_effect_set_float(P("mid_glow_strength"),0.74f);
    gs_effect_set_float(P("bloom_strength"),0.76f);
    gs_effect_set_float(P("hotspot_strength"),0.0f);
    gs_effect_set_float(P("hotspot_size"),0.08f);
    gs_effect_set_vec4(P("color_a"),&cyan);
    gs_effect_set_vec4(P("color_b"),&magenta);
    gs_effect_set_int(P("color_mode"),1);
    gs_effect_set_float(P("rainbow_phase"),0.0f);
    gs_effect_set_float(P("rainbow_saturation"),0.92f);
    gs_effect_set_float(P("rainbow_hue_offset"),0.0f);
    gs_effect_set_float(P("rainbow_spread"),1.0f);
    gs_effect_set_float(P("color_phase"),0.0f);
    gs_effect_set_float(P("pulse_phase"),0.0f);
    gs_effect_set_int(P("animation_id"),2);
    gs_effect_set_int(P("segment_count"),0);
    gs_effect_set_int(P("border_enabled"),1);
    gs_effect_set_int(P("glow_enabled"),0);
    gs_effect_set_texture(P("svg_sdf"),NULL);
    gs_effect_set_int(P("svg_ready"),0);
    gs_effect_set_texture(P("image"),input);
#undef P

    unsigned outer_start[2]={0},outer_arrive[2]={0};
    unsigned inner_hold[2]={0},inner_arrive[2]={0},center_a[2]={0};
    int failed=0;
    for(int frame=0;frame<2;++frame){
        gs_effect_set_float(gs_effect_get_param_by_name(effect,"flow_phase"),
                            frame==0 ? 0.0f : 0.25f);
        /* libobs texrender objects are single-use until reset. Reusing the
         * first frame's target without this makes the second begin fail before
         * any P6B pixels are exercised. */
        gs_texrender_reset(target);
        if(!gs_texrender_begin(target,W,H)){
            fprintf(stderr,"FAIL: P6B texrender begin frame=%d\n",frame);
            failed=1; break;
        }
        const bool srgb=gs_framebuffer_srgb_enabled();
        gs_enable_framebuffer_srgb(false);
        gs_blend_state_push(); gs_enable_blending(false);
        struct vec4 clear; vec4_zero(&clear);
        gs_clear(GS_CLEAR_COLOR,&clear,0,0);
        gs_ortho(0,(float)W,0,(float)H,-100,100);
        gs_matrix_push(); gs_matrix_identity();
        const enum gs_cull_mode cull=gs_get_cull_mode();
        gs_set_cull_mode(GS_NEITHER);
        while(gs_effect_loop(effect,"Draw")) gs_draw_sprite(input,0,W,H);
        gs_set_cull_mode(cull);
        gs_matrix_pop();
        gs_blend_state_pop();
        gs_enable_framebuffer_srgb(srgb);
        gs_texrender_end(target);

        gs_stage_texture(stage,gs_texrender_get_texture(target));
        uint8_t *mapped=NULL; uint32_t stride=0;
        if(!gs_stagesurface_map(stage,&mapped,&stride)){failed=1;break;}
        /* outer_start t=.05, outer_arrive t=.80; inner_hold t=.55 and
         * inner_arrive t=.30. Radius coordinates are derived from the
         * authored 100px mask radius plus/minus the exact rail offsets. */
        outer_start[frame]=(mapped+194u*stride+264u*4u)[3];
        outer_arrive[frame]=(mapped+56u*stride+194u*4u)[3];
        inner_hold[frame]=(mapped+131u*stride+71u*4u)[3];
        inner_arrive[frame]=(mapped+249u*stride+131u*4u)[3];
        center_a[frame]=(mapped+160u*stride+160u*4u)[3];
        gs_stagesurface_unmap(stage);
    }

    if(!failed &&
       (outer_start[0]<150 || outer_arrive[0]>15 ||
        inner_hold[0]<130 || inner_arrive[0]>15 ||
        outer_start[1]>15 || outer_arrive[1]<150 ||
        inner_hold[1]<130 || inner_arrive[1]<130 ||
        center_a[0]>3 || center_a[1]>3)){
        fprintf(stderr,
                "FAIL: P6B counter rotation outer=%u/%u -> %u/%u inner=%u/%u -> %u/%u center=%u/%u\n",
                outer_start[0],outer_arrive[0],outer_start[1],outer_arrive[1],
                inner_hold[0],inner_arrive[0],inner_hold[1],inner_arrive[1],
                center_a[0],center_a[1]);
        failed=1;
    }

    gs_stagesurface_destroy(stage);
    gs_texrender_destroy(target);
    gs_texture_destroy(input);
    return failed;
}

/* D4D preview-scale GPU gate: one full spectrum is distributed continuously
 * around the rounded perimeter. The frame itself does not rotate and the
 * transparent portrait center remains untouched. */
static int verify_rainbow_preview_scale(gs_effect_t *effect)
{
    enum { W=320,H=180 };
    uint8_t *pixels=bzalloc((size_t)W*H*4u);
    if(!pixels) return 1;
    const uint8_t *layers[]={pixels};
    gs_texture_t *input=gs_texture_create(W,H,GS_RGBA,1,layers,0);
    bfree(pixels);
    gs_texrender_t *target=gs_texrender_create(GS_RGBA,GS_ZS_NONE);
    gs_stagesurf_t *stage=gs_stagesurface_create(W,H,GS_RGBA);
    if(!input||!target||!stage){
        if(stage)gs_stagesurface_destroy(stage);
        if(target)gs_texrender_destroy(target);
        if(input)gs_texture_destroy(input);
        fprintf(stderr,"FAIL: Rainbow 320x180 resources\n");
        return 1;
    }

    struct vec2 dims,zero,half;
    struct vec4 white,btop,bbottom,btail;
    vec2_set(&dims,(float)W,(float)H); vec2_set(&zero,0,0);
    vec2_set(&half,110.0f,60.0f);
    vec4_set(&white,1,1,1,1);
    vec4_set(&btop,-1,-1,1,-1); vec4_set(&bbottom,1,.6f,-1,.6f);
    vec4_set(&btail,.11f,.33f,.14f,.22f);
#define P(name) gs_effect_get_param_by_name(effect,name)
    gs_effect_set_vec2(P("uv_size"),&dims);
    gs_effect_set_vec2(P("output_size"),&dims);
    gs_effect_set_vec2(P("input_origin"),&zero);
    gs_effect_set_vec2(P("half_size"),&half);
    gs_effect_set_vec2(P("mask_offset"),&zero);
    gs_effect_set_vec2(P("subject_pan"),&zero);
    gs_effect_set_float(P("subject_zoom"),1.0f);
    gs_effect_set_float(P("shape_rotation"),0.0f);
    gs_effect_set_int(P("polygon_sides"),8);
    gs_effect_set_float(P("shape_detail"),0.22f);
    gs_effect_set_vec4(P("bubble_top"),&btop);
    gs_effect_set_vec4(P("bubble_bottom"),&bbottom);
    gs_effect_set_vec4(P("bubble_tail"),&btail);
    gs_effect_set_float(P("corner_radius"),18.0f);
    gs_effect_set_int(P("shape_id"),0);
    gs_effect_set_float(P("border_width"),4.0f);
    gs_effect_set_int(P("style_id"),0);
    gs_effect_set_int(P("ornament_mode"),0);
    gs_effect_set_float(P("art_intensity"),0.0f);
    gs_effect_set_float(P("art_gap"),2.0f);
    gs_effect_set_float(P("ornament_width"),10.0f);
    gs_effect_set_float(P("ornament_length_x"),60.0f);
    gs_effect_set_float(P("ornament_length_y"),48.0f);
    gs_effect_set_float(P("inner_rail_width"),1.4f);
    gs_effect_set_float(P("feather"),0.5f);
    gs_effect_set_float(P("glow_radius"),18.0f);
    gs_effect_set_float(P("glow_strength"),0.0f);
    gs_effect_set_float(P("mid_glow_strength"),0.75f);
    gs_effect_set_float(P("bloom_strength"),0.92f);
    gs_effect_set_float(P("hotspot_strength"),0.0f);
    gs_effect_set_float(P("hotspot_size"),0.10f);
    gs_effect_set_vec4(P("color_a"),&white);
    gs_effect_set_vec4(P("color_b"),&white);
    gs_effect_set_int(P("color_mode"),2);
    gs_effect_set_float(P("rainbow_phase"),0.0f);
    gs_effect_set_float(P("rainbow_saturation"),1.0f);
    gs_effect_set_float(P("rainbow_hue_offset"),0.0f);
    gs_effect_set_float(P("rainbow_spread"),1.0f);
    gs_effect_set_float(P("color_phase"),0.0f);
    gs_effect_set_float(P("pulse_phase"),0.0f);
    gs_effect_set_float(P("flow_phase"),0.0f);
    gs_effect_set_int(P("animation_id"),0);
    gs_effect_set_int(P("segment_count"),0);
    gs_effect_set_int(P("border_enabled"),1);
    gs_effect_set_int(P("glow_enabled"),0);
    gs_effect_set_texture(P("svg_sdf"),NULL);
    gs_effect_set_int(P("svg_ready"),0);
    gs_effect_set_texture(P("image"),input);
#undef P

    int failed=0;
    if(!gs_texrender_begin(target,W,H)){failed=1;goto done_rainbow;}
    const bool srgb=gs_framebuffer_srgb_enabled();
    gs_enable_framebuffer_srgb(false); gs_blend_state_push(); gs_enable_blending(false);
    struct vec4 clear; vec4_zero(&clear); gs_clear(GS_CLEAR_COLOR,&clear,0,0);
    gs_ortho(0,(float)W,0,(float)H,-100,100);
    gs_matrix_push(); gs_matrix_identity();
    const enum gs_cull_mode cull=gs_get_cull_mode(); gs_set_cull_mode(GS_NEITHER);
    while(gs_effect_loop(effect,"Draw")) gs_draw_sprite(input,0,W,H);
    gs_set_cull_mode(cull); gs_matrix_pop(); gs_blend_state_pop();
    gs_enable_framebuffer_srgb(srgb); gs_texrender_end(target);
    gs_stage_texture(stage,gs_texrender_get_texture(target));
    uint8_t *mapped=NULL; uint32_t stride=0;
    if(!gs_stagesurface_map(stage,&mapped,&stride)){failed=1;goto done_rainbow;}

    const uint8_t *top=mapped+30u*stride+160u*4u;
    const uint8_t *right=mapped+90u*stride+270u*4u;
    const uint8_t *bottom=mapped+150u*stride+160u*4u;
    const uint8_t *left=mapped+90u*stride+50u*4u;
    const uint8_t *center=mapped+90u*stride+160u*4u;
    const int tr=abs((int)top[0]-(int)right[0])+
                 abs((int)top[1]-(int)right[1])+
                 abs((int)top[2]-(int)right[2]);
    const int rb=abs((int)right[0]-(int)bottom[0])+
                 abs((int)right[1]-(int)bottom[1])+
                 abs((int)right[2]-(int)bottom[2]);
    const int bl=abs((int)bottom[0]-(int)left[0])+
                 abs((int)bottom[1]-(int)left[1])+
                 abs((int)bottom[2]-(int)left[2]);
    if(top[3]<180||right[3]<180||bottom[3]<180||left[3]<180||
       tr<100||rb<100||bl<100||center[3]>3){
        fprintf(stderr,
                "FAIL: Rainbow preview A=%u/%u/%u/%u delta=%d/%d/%d center=%u\n",
                top[3],right[3],bottom[3],left[3],tr,rb,bl,center[3]);
        failed=1;
    }
    gs_stagesurface_unmap(stage);
done_rainbow:
    gs_stagesurface_destroy(stage); gs_texrender_destroy(target);
    gs_texture_destroy(input);
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
            for (int variant = 0; variant < 47; ++variant)
                missing += verify_pixel_fixture(effect, variant);
            missing += verify_tech_hud_preview_scale(effect);
            missing += verify_game_ui_preview_scale(effect);
            missing += verify_game_ui_expanded_top_padding(effect);
            missing += verify_counter_rotating_dual_ring(effect);
            missing += verify_rainbow_preview_scale(effect);
        }
        gs_effect_destroy(effect);
    }
    obs_leave_graphics();
    bfree(errors);
    obs_shutdown();
    if (missing) return 1;
    puts("PASS: libobs alpha/framing/light + SVG + D3 + Bubble + Tech HUD + Game UI + counter-rotating ring + Rainbow GPU fixtures (partial G4)");
    return 0;
}
