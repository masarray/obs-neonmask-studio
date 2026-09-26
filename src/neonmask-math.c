/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-math.h"
#include <math.h>

float nm_clamp(float value, float min_value, float max_value)
{
    if (!isfinite(value)) return min_value;
    return fminf(max_value, fmaxf(min_value, value));
}

nm_geometry nm_make_geometry(uint32_t width, uint32_t height, float scale,
                             float roundness)
{
    nm_geometry g = {0};
    g.width = (float)width;
    g.height = (float)height;
    scale = nm_clamp(scale, 0.30f, 0.96f);
    /* Reserve source-space margin for a halo without resizing OBS's source. */
    g.half_width = 0.5f * g.width * scale;
    g.half_height = 0.5f * g.height * scale;
    g.radius = nm_clamp(roundness, 0.0f, 1.0f) *
               fminf(g.half_width, g.half_height);
    return g;
}

float nm_sd_round_rect(float x, float y, float half_width,
                       float half_height, float radius)
{
    if (half_width <= 0.0f || half_height <= 0.0f) return 1.0e6f;
    radius = nm_clamp(radius, 0.0f, fminf(half_width, half_height));
    const float qx = fabsf(x) - half_width + radius;
    const float qy = fabsf(y) - half_height + radius;
    const float outer = hypotf(fmaxf(qx, 0.0f), fmaxf(qy, 0.0f));
    return outer + fminf(fmaxf(qx, qy), 0.0f) - radius;
}

float nm_sd_circle(float x, float y, float radius)
{
    return hypotf(x, y) - radius;
}

/* Fast ellipse distance approximation, exact on its principal axes. */
float nm_sd_ellipse_approx(float x, float y, float rx, float ry)
{
    if (rx <= 0.0f || ry <= 0.0f) return 1.0e6f;
    /* Match the GPU distance approximation, including its center guard. */
    const float k0 = hypotf(x / rx, y / ry);
    if (k0 < 0.0001f) return -fminf(rx, ry);
    const float k1 = hypotf(x / (rx * rx), y / (ry * ry));
    return k0 * (k0 - 1.0f) / fmaxf(k1, 0.00001f);
}

uint32_t nm_obs_rgba(unsigned r, unsigned g, unsigned b)
{
    return (r & 255u) | ((g & 255u) << 8) | ((b & 255u) << 16);
}
