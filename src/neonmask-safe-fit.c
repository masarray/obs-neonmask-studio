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
        if (cfg->ornament_mode != NM_ORNAMENT_NONE)
            margin = fmaxf(margin, accent);
    }
    return margin;
}

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
    if (!cfg->safe_fit) return true; /* Saved legacy scenes are untouched. */

    const float envelope = nm_light_envelope(cfg);
    const float available_x = 0.5f * (float)width -
                              fabsf(cfg->mask_x_px) - envelope;
    const float available_y = 0.5f * (float)height -
                              fabsf(cfg->mask_y_px) - envelope;
    out->envelope_px = envelope;
    if (!isfinite(available_x) || !isfinite(available_y) ||
        available_x <= 0.0f || available_y <= 0.0f) {
        out->fits = false;
        return true;
    }

    /* Every authored/parametric silhouette is contained in its unrotated
     * half-size rectangle (Chat Bubble tail included). A rotated AABB is
     * deliberately conservative for circles and empty cut corners. */
    const float angle = cfg->shape_rotation_deg * 0.01745329251994329577f;
    const float c = fabsf(cosf(angle)), s = fabsf(sinf(angle));
    const float box_x = c * hx + s * hy;
    const float box_y = s * hx + c * hy;
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
