/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Compile the real OBS .effect against a live OpenGL device under Xvfb.
 * This verifies the effect parser/GL compiler, not visual correctness. */
#include <obs.h>
#include <graphics/graphics.h>
#include <util/bmem.h>
#include <stdio.h>

static const char *const uniforms[] = {
    "uv_size", "half_size", "corner_radius", "shape_id", "border_width",
    "feather", "glow_radius", "glow_strength", "color_a", "color_b",
    "elapsed_time", "animation_speed", "animation_id", "segment_count",
    "border_enabled", "glow_enabled", "image", "ViewProj"
};

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

    struct obs_video_info video = {0};
    video.graphics_module = "libobs-opengl";
    video.fps_num = 30;
    video.fps_den = 1;
    video.base_width = 640;
    video.base_height = 360;
    video.output_width = 640;
    video.output_height = 360;
    video.output_format = VIDEO_FORMAT_RGBA;
    video.adapter = 0;

    if (obs_reset_video(&video) != OBS_VIDEO_SUCCESS) {
        fprintf(stderr, "FAIL: obs_reset_video: OpenGL context unavailable\n");
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
        gs_effect_destroy(effect);
    }
    obs_leave_graphics();
    bfree(errors);
    obs_shutdown();
    if (missing) return 1;
    puts("PASS: compiled NeonMask shader with real libobs OpenGL graphics device");
    return 0;
}
