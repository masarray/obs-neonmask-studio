/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-art.h"
#include "neonmask-math.h"
#include <math.h>

#define NM_PI 3.14159265358979323846f

float nm_rounded_contour_turn(float x, float y, float half_width,
                              float half_height, float radius)
{
    const float bx = fmaxf(1.0f, half_width);
    const float by = fmaxf(1.0f, half_height);
    const float r = nm_clamp(radius, 0.0f, fminf(bx, by));
    const float sx = bx - r;
    const float sy = by - r;
    const float h = 2.0f * sx;
    const float v = 2.0f * sy;
    const float a = NM_PI * 0.5f * r;
    const float total = 2.0f * h + 2.0f * v + 4.0f * a;
    const float ax = fabsf(x), ay = fabsf(y);
    float distance;
    if (ax > sx && ay > sy && r > 0.0f) {
        const float angle = atan2f(y - copysignf(sy, y), x - copysignf(sx, x));
        if (x >= 0.0f && y < 0.0f) distance = h + r * (angle + NM_PI * 0.5f);
        else if (x >= 0.0f) distance = h + a + v + r * angle;
        else if (y >= 0.0f) distance = h + a + v + a + h + r * (angle - NM_PI * 0.5f);
        else distance = h + a + v + a + h + a + v + r * (angle + NM_PI);
    } else if (ay > sy) {
        if (y < 0.0f) distance = x + sx;
        else distance = h + a + v + a + sx - x;
    } else if (x >= 0.0f) {
        distance = h + a + y + sy;
    } else {
        distance = h + a + v + a + h + a + sy - y;
    }
    distance = fmodf(distance, total);
    if (distance < 0.0f) distance += total;
    return distance / total;
}

bool nm_art_recipe_valid(int mode, float track_gap, float intensity)
{
    return (mode >= NM_ORNAMENT_NONE && mode <= NM_ORNAMENT_STREAMER) &&
           isfinite(track_gap) && track_gap >= 1.0f && track_gap <= 16.0f &&
           isfinite(intensity) && intensity >= 0.0f && intensity <= 1.0f;
}
