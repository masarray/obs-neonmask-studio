/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-math.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static void near(float got, float expected)
{
    if (fabsf(got - expected) > 0.001f) {
        fprintf(stderr, "distance mismatch: got %f expected %f\n", got, expected);
        assert(0);
    }
}

int main(void)
{
    near(nm_clamp(-10.0f, 0.0f, 1.0f), 0.0f);
    near(nm_clamp(2.0f, 0.0f, 1.0f), 1.0f);
    near(nm_clamp(NAN, 0.0f, 1.0f), 0.0f);
    nm_geometry g = nm_make_geometry(1920, 1080, 0.8f, 0.25f);
    near(g.half_width, 768.0f);
    near(g.half_height, 432.0f);
    near(g.radius, 108.0f);
    near(nm_sd_round_rect(0, 0, 100, 60, 10), -60.0f);
    near(nm_sd_round_rect(100, 0, 100, 60, 10), 0.0f);
    near(nm_sd_round_rect(110, 0, 100, 60, 10), 10.0f);
    near(nm_sd_circle(0, 0, 50), -50.0f);
    near(nm_sd_circle(30, 40, 50), 0.0f);
    near(nm_sd_circle(60, 0, 50), 10.0f);
    near(nm_sd_ellipse_approx(80, 0, 80, 40), 0.0f);
    near(nm_sd_ellipse_approx(0, 40, 80, 40), 0.0f);
    assert(nm_obs_bgr(255, 49, 221) == 0x00DD31FFu);
    puts("PASS: geometry, color encoding and boundary tests");
    return 0;
}
