/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-safe-fit.h"
#include <math.h>
#include <string.h>

/* Conservative support for shader's explicit smoothstep rails, mask
 * feather and exp2 broad bloom. At 2.5 radii past halfCore the outer
 * exp2(-2*g*g/r*r) factor is < 1/5000; even with the shader's
 * flow multiplier its alpha is well below 1/255. Ornament extents
 * include the outermost tip/bracket plus their 4.2px glow cutoff.
 * Re-audit this bound whenever neon-mask.effect changes its formula. */
static float nm_light_envelope(const nm_config *cfg)
{
    float margin = cfg->feather_px + 2.0f;
    if (!cfg->show_border) return margin;

    const float half_core = fmaxf(0.25f, cfg->border_px * 0.5f);
    margin = fmaxf(margin, half_core + 2.0f);
    if (cfg->style_id == NM_STYLE_DOUBLE || cfg->style_id == NM_STYLE_HUD)
        margin = fmaxf(margin, half_core + 5.0f);

    if (cfg->show_glow && cfg->glow_amount > 0.0f &&
        (cfg->mid_glow_strength > 0.0f || cfg->bloom_strength > 0.0f))
        margin = fmaxf(margin, half_core + 2.5f * cfg->glow_px + 2.0f);

    if (cfg->art_intensity > 0.0f) {
        float accent = half_core + cfg->art_gap + 8.0f;
        if (cfg->ornament_mode == NM_ORNAMENT_STREAMER)
            accent = cfg->art_gap + 13.0f;
        if (cfg->ornament_mode == NM_ORNAMENT_TECH_HUD &&
            cfg->shape_id == NM_SHAPE_TECH_HUD) {
            const float stroke = fmaxf(2.4f,cfg->border_px*1.16f);
            const float gap = cfg->art_gap + fmaxf(5.0f,cfg->border_px*1.25f);
            const float glow = cfg->show_glow ?
                fmaxf(4.0f,cfg->glow_px*0.20f) : 0.0f;
            accent = gap + stroke + glow + 2.0f;
        }
        if (cfg->ornament_mode == NM_ORNAMENT_GAME_UI &&
            cfg->shape_id == NM_SHAPE_GAME_UI) {
            const float stroke=fmaxf(3.0f,cfg->border_px*1.34f);
            const float gap=cfg->art_gap+fmaxf(7.0f,cfg->border_px*1.35f);
            const float glow=cfg->show_glow ?
                fmaxf(4.0f,cfg->glow_px*0.18f) : 0.0f;
            accent=gap+stroke+glow+2.0f;
        }
        if (cfg->ornament_mode != NM_ORNAMENT_NONE)
            margin = fmaxf(margin, accent);
    }
    return margin;
}

/* Bounded relative to the CAPTURED target. This prevents an extreme mask X/Y
 * from requesting an unbounded render target through get_width/get_height. */
enum { NM_MAX_PAD_SIDE = 512u, NM_MAX_OUTPUT_SIDE = 8192u };

bool nm_safe_fit_calculate(const nm_config *cfg, uint32_t width,
                           uint32_t height, nm_fit_result *out)
{
    if (!cfg || !out || !width || !height) return false;
    memset(out, 0, sizeof(*out));
    const float hx = 0.5f * (float)width * cfg->mask_width;
    const float hy = 0.5f * (float)height * cfg->mask_height;
    if (!isfinite(hx) || !isfinite(hy) || hx <= 0.0f || hy <= 0.0f)
        return false;

    out->half_width = hx;
    out->half_height = hy;
    out->scale = 1.0f;
    out->fits = true;
    out->output_width = width;
    out->output_height = height;
    if (!cfg->safe_fit && !cfg->expand_canvas)
        return true; /* All saved legacy scenes retain their exact dimensions. */

    const float envelope = nm_light_envelope(cfg);
    out->envelope_px = envelope;
    if (!isfinite(envelope)) return false;
    /* Conservative shape AABB: covers the tail and every rotated silhouette.
     * No change to authored X/Y, image pan/zoom or source dimensions. */
    const float angle = cfg->shape_rotation_deg * 0.01745329251994329577f;
    const float c = fabsf(cosf(angle)), s = fabsf(sinf(angle));
    const float box_x = c * hx + s * hy;
    const float box_y = s * hx + c * hy;

    if (cfg->expand_canvas) {
        /* The OBS filter callback exposes a new top-left-anchored width/height;
         * it has NO API to add a negative scene-space origin. The shader uses
         * pad_left/top to restore input-relative coordinates, but the scene
         * item content moves by that amount unless manually compensated. */
        const float center_x = 0.5f * (float)width + cfg->mask_x_px;
        const float center_y = 0.5f * (float)height + cfg->mask_y_px;
        const float left = ceilf(fmaxf(0.0f, box_x + envelope - center_x));
        const float top = ceilf(fmaxf(0.0f, box_y + envelope - center_y));
        const float right = ceilf(fmaxf(0.0f, center_x + box_x + envelope - (float)width));
        const float bottom = ceilf(fmaxf(0.0f, center_y + box_y + envelope - (float)height));
        if (!isfinite(left) || !isfinite(top) || !isfinite(right) || !isfinite(bottom) ||
            left > NM_MAX_PAD_SIDE || top > NM_MAX_PAD_SIDE ||
            right > NM_MAX_PAD_SIDE || bottom > NM_MAX_PAD_SIDE) {
            out->fits = false;
            return true;
        }
        out->pad_left = (uint32_t)left;
        out->pad_top = (uint32_t)top;
        out->pad_right = (uint32_t)right;
        out->pad_bottom = (uint32_t)bottom;
        const uint64_t ow = (uint64_t)width + out->pad_left + out->pad_right;
        const uint64_t oh = (uint64_t)height + out->pad_top + out->pad_bottom;
        if (ow > NM_MAX_OUTPUT_SIDE || oh > NM_MAX_OUTPUT_SIDE) {
            out->fits = false;
            return true;
        }
        out->output_width = (uint32_t)ow;
        out->output_height = (uint32_t)oh;
        return true;
    }

    /* D3a safe-fit: shrink the mask uniformly within the existing canvas. */
    const float available_x = 0.5f * (float)width -
                              fabsf(cfg->mask_x_px) - envelope;
    const float available_y = 0.5f * (float)height -
                              fabsf(cfg->mask_y_px) - envelope;
    if (!isfinite(available_x) || !isfinite(available_y) ||
        available_x <= 0.0f || available_y <= 0.0f) {
        out->fits = false;
        return true;
    }
    const float factor = fminf(1.0f,
                         fminf(available_x / box_x, available_y / box_y));
    if (!isfinite(factor) || factor <= 0.0f) {
        out->fits = false;
        return true;
    }
    out->scale = factor;
    out->half_width = hx * factor;
    out->half_height = hy * factor;
    return true;
}
