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
static bool nm_dual_ring_active(const nm_config *cfg)
{
    return cfg && cfg->show_border && cfg->art_intensity > 0.0f &&
           cfg->shape_id == NM_SHAPE_CIRCLE &&
           cfg->ornament_mode == NM_ORNAMENT_DUAL_RING;
}

static float nm_light_envelope(const nm_config *cfg, float mask_radius)
{
    float margin = cfg->feather_px + 2.0f;
    if (!cfg->show_border) return margin;

    const float half_core = fmaxf(0.25f, cfg->border_px * 0.5f);
    margin = fmaxf(margin, half_core + 2.0f);
    if (cfg->style_id == NM_STYLE_DOUBLE || cfg->style_id == NM_STYLE_HUD)
        margin = fmaxf(margin, half_core + 5.0f);

    const bool dual_ring = nm_dual_ring_active(cfg);
    if (!dual_ring && cfg->show_glow && cfg->glow_amount > 0.0f &&
        (cfg->mid_glow_strength > 0.0f || cfg->bloom_strength > 0.0f))
        margin = fmaxf(margin, half_core + 2.5f * cfg->glow_px + 2.0f);

    if (cfg->art_intensity > 0.0f) {
        float accent = half_core + cfg->art_gap + 8.0f;
        if (cfg->ornament_mode == NM_ORNAMENT_STREAMER)
            accent = cfg->art_gap + 13.0f;
        if (cfg->ornament_mode == NM_ORNAMENT_TECH_HUD &&
            cfg->shape_id == NM_SHAPE_TECH_HUD) {
            const float glow = cfg->show_glow ?
                fmaxf(4.0f,cfg->glow_px*0.18f) : 0.0f;
            accent = cfg->art_gap + cfg->ornament_width_px + glow + 2.0f;
        }
        if (cfg->ornament_mode == NM_ORNAMENT_GAME_UI &&
            cfg->shape_id == NM_SHAPE_GAME_UI) {
            const float glow=cfg->show_glow ?
                fmaxf(3.5f,cfg->glow_px*0.16f) : 0.0f;
            accent=cfg->art_gap+cfg->ornament_width_px+glow+2.0f;
        }
        if (dual_ring) {
            const float r=fmaxf(1.0f,mask_radius);
            const float design=0.01f*r;
            const float inner_offset=cfg->ring_inner_offset_pct*design;
            const float spacing=cfg->ring_spacing_pct*design;
            const float outer_half=0.5f*cfg->ring_outer_width_pct*design;
            const float glow_scale=nm_clamp(cfg->glow_px/22.0f,0.5f,2.0f);
            const float glow=cfg->show_glow && cfg->glow_amount>0.0f ?
                r*0.045f*glow_scale : 0.0f;
            /* P6F support scales with mask radius. This is the same authored
             * geometry the host sends to the shader after safe-fit. */
            accent=half_core+inner_offset+spacing+outer_half+glow+2.0f;
        }
        if (cfg->ornament_mode != NM_ORNAMENT_NONE)
            margin = fmaxf(margin, accent);
    }
    return margin;
}

/* D4J: the connected outer-L is authored in the shape's LOCAL axes, then
 * the entire shape is rotated by shape_rotation. A scalar envelope applied
 * after rotating only the mask underestimates the L reach by up to sqrt(2)
 * near 45 degrees. Keep the legacy scalar envelope for every other recipe,
 * but rotate the authored local gap+thickness vector before sizing/fitting.
 *
 * The requested ornament width is intentionally conservative here: the shader
 * may clamp pathological width/arm combinations down, never up. */
static void nm_rotated_authored_support(const nm_config *cfg, float c, float s,
                                        float envelope, float *support_x,
                                        float *support_y)
{
    *support_x = envelope;
    *support_y = envelope;
    if (!cfg->show_border || cfg->art_intensity <= 0.0f)
        return;

    float local_outset = 0.0f;
    float isotropic = 0.0f;
    if (cfg->shape_id == NM_SHAPE_TECH_HUD &&
        cfg->ornament_mode == NM_ORNAMENT_TECH_HUD) {
        const float glow = cfg->show_glow ?
            fmaxf(4.0f, cfg->glow_px * 0.18f) : 0.0f;
        local_outset = fmaxf(0.0f, cfg->art_gap) +
                       fmaxf(1.0f, cfg->ornament_width_px);
        isotropic = glow + 2.0f;
    } else if (cfg->shape_id == NM_SHAPE_GAME_UI &&
               cfg->ornament_mode == NM_ORNAMENT_GAME_UI) {
        const float glow = cfg->show_glow ?
            fmaxf(3.5f, cfg->glow_px * 0.16f) : 0.0f;
        local_outset = fmaxf(0.0f, cfg->art_gap) +
                       fmaxf(1.0f, cfg->ornament_width_px);
        isotropic = glow + 2.0f;
    } else {
        return;
    }

    const float rotated = (c + s) * local_outset + isotropic;
    *support_x = fmaxf(*support_x, rotated);
    *support_y = fmaxf(*support_y, rotated);
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

    /* D4I/P6F: dedicated authored geometry outside the mask silhouette
     * (Tech/Game outer Ls and both external rotating rings) gets automatic
     * clipping protection when expansion is off. Other shapes keep legacy
     * safe_fit. */
    const bool outer_authored =
        cfg->art_intensity > 0.0f &&
        ((cfg->shape_id == NM_SHAPE_TECH_HUD &&
          cfg->ornament_mode == NM_ORNAMENT_TECH_HUD) ||
         (cfg->shape_id == NM_SHAPE_GAME_UI &&
          cfg->ornament_mode == NM_ORNAMENT_GAME_UI) ||
         (cfg->shape_id == NM_SHAPE_CIRCLE &&
          cfg->ornament_mode == NM_ORNAMENT_DUAL_RING));
    const bool fit_inside_source = cfg->safe_fit || outer_authored;
    if (!fit_inside_source && !cfg->expand_canvas)
        return true; /* Legacy/non-authored scenes retain exact dimensions. */

    const float envelope = nm_light_envelope(cfg, fminf(hx,hy));
    out->envelope_px = envelope;
    if (!isfinite(envelope)) return false;
    /* Conservative shape AABB: covers the tail and every rotated silhouette.
     * Authored outer-L support is rotated in local space as well; treating its
     * reach as a post-rotation scalar is insufficient at diagonal angles. */
    const float angle = cfg->shape_rotation_deg * 0.01745329251994329577f;
    const float c = fabsf(cosf(angle)), s = fabsf(sinf(angle));
    const float box_x = c * hx + s * hy;
    const float box_y = s * hx + c * hy;
    float support_x = envelope, support_y = envelope;
    nm_rotated_authored_support(cfg, c, s, envelope, &support_x, &support_y);

    if (cfg->expand_canvas) {
        /* The OBS filter callback exposes a new top-left-anchored width/height;
         * it has NO API to add a negative scene-space origin. The shader uses
         * pad_left/top to restore input-relative coordinates, but the scene
         * item content moves by that amount unless manually compensated. */
        const float center_x = 0.5f * (float)width + cfg->mask_x_px;
        const float center_y = 0.5f * (float)height + cfg->mask_y_px;
        const float left = ceilf(fmaxf(0.0f, box_x + support_x - center_x));
        const float top = ceilf(fmaxf(0.0f, box_y + support_y - center_y));
        const float right = ceilf(fmaxf(0.0f, center_x + box_x + support_x - (float)width));
        const float bottom = ceilf(fmaxf(0.0f, center_y + box_y + support_y - (float)height));
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

    /* D3a/P6F safe-fit: legacy ornaments have a mostly fixed-pixel
     * support and keep the closed-form path. Dual Ring support itself scales
     * with mask radius, so solve the authored composition as one uniformly
     * shrinking unit. This prevents auto-fit from destroying its proportions. */
    const float available_x = 0.5f * (float)width - fabsf(cfg->mask_x_px);
    const float available_y = 0.5f * (float)height - fabsf(cfg->mask_y_px);
    if (!isfinite(available_x) || !isfinite(available_y) ||
        available_x <= 0.0f || available_y <= 0.0f) {
        out->fits = false;
        return true;
    }

    float factor=1.0f;
    if (nm_dual_ring_active(cfg)) {
        float lo=0.0f, hi=1.0f;
        for (unsigned i=0;i<24u;++i) {
            const float mid=0.5f*(lo+hi);
            const float env=nm_light_envelope(cfg,fminf(hx,hy)*mid);
            const bool ok=(box_x*mid+env<=available_x) &&
                          (box_y*mid+env<=available_y);
            if(ok) lo=mid; else hi=mid;
        }
        factor=lo;
        if (!isfinite(factor) || factor <= 0.0001f) {
            out->fits=false;
            return true;
        }
        out->envelope_px=nm_light_envelope(cfg,fminf(hx,hy)*factor);
    } else {
        const float legacy_available_x=available_x-support_x;
        const float legacy_available_y=available_y-support_y;
        if (legacy_available_x<=0.0f || legacy_available_y<=0.0f) {
            out->fits=false;
            return true;
        }
        factor=fminf(1.0f,fminf(legacy_available_x/box_x,
                                legacy_available_y/box_y));
        if (!isfinite(factor) || factor <= 0.0f) {
            out->fits=false;
            return true;
        }
    }
    out->scale=factor;
    out->half_width=hx*factor;
    out->half_height=hy*factor;
    return true;
}
