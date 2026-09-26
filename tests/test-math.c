/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-math.h"
#include <math.h>
#include <float.h>
#include <stdio.h>

static int failures;

static void near(const char *name, float got, float expected, float tolerance)
{
    if (!(got >= -FLT_MAX && got <= FLT_MAX) || fabsf(got - expected) > tolerance) {
        fprintf(stderr, "FAIL %s: got %f expected %f (tolerance %f)\n",
                name, got, expected, tolerance);
        failures++;
    }
}

static void check(const char *name, int condition)
{
    if (!condition) {
        fprintf(stderr, "FAIL %s\n", name);
        failures++;
    }
}

int main(void)
{
    /* These checks stay active in Release builds (-DNDEBUG). */
    near("clamp low", nm_clamp(-10.0f, 0.0f, 1.0f), 0.0f, 0.0f);
    near("clamp high", nm_clamp(2.0f, 0.0f, 1.0f), 1.0f, 0.0f);
    near("clamp NaN", nm_clamp(NAN, 0.0f, 1.0f), 0.0f, 0.0f);
    nm_geometry g = nm_make_geometry(1920, 1080, 0.8f, 0.25f);
    near("half width", g.half_width, 768.0f, 0.01f);
    near("half height", g.half_height, 432.0f, 0.01f);
    near("corner radius", g.radius, 108.0f, 0.01f);
    nm_geometry smallest = nm_make_geometry(64, 64, -100.0f, 10.0f);
    near("scale clamp", smallest.half_width, 9.6f, 0.01f);
    near("roundness clamp", smallest.radius, 9.6f, 0.01f);
    near("rect center", nm_sd_round_rect(0, 0, 100, 60, 10), -60.0f, 0.001f);
    near("rect edge", nm_sd_round_rect(100, 0, 100, 60, 10), 0.0f, 0.001f);
    near("rect outer", nm_sd_round_rect(110, 0, 100, 60, 10), 10.0f, 0.001f);
    check("rect invalid dimensions", nm_sd_round_rect(0, 0, -1, 10, 0) > 100000.0f);
    near("circle center", nm_sd_circle(0, 0, 50), -50.0f, 0.001f);
    near("circle edge", nm_sd_circle(30, 40, 50), 0.0f, 0.001f);
    near("circle outer", nm_sd_circle(60, 0, 50), 10.0f, 0.001f);
    near("ellipse center", nm_sd_ellipse_approx(0, 0, 80, 40), -40.0f, 0.001f);
    near("ellipse x edge", nm_sd_ellipse_approx(80, 0, 80, 40), 0.0f, 0.001f);
    near("ellipse y edge", nm_sd_ellipse_approx(0, 40, 80, 40), 0.0f, 0.001f);
    near("ellipse x outer", nm_sd_ellipse_approx(100, 0, 80, 40), 20.0f, 0.001f);
    near("ellipse y outer", nm_sd_ellipse_approx(0, 60, 80, 40), 20.0f, 0.001f);
    check("ellipse invalid radii", nm_sd_ellipse_approx(0, 0, 0, 40) > 100000.0f);
    check("color packed R,G,B", nm_obs_rgba(255, 49, 221) == 0x00DD31FFu);
    check("color channels clamped", nm_obs_rgba(256, 511, 258) == 0x0002FF00u);

    float u = 0.0f, v = 0.0f;
    check("center framing inside", nm_subject_sample_uv(1920, 1080, 0, 0, 0, 0, 1, &u, &v));
    near("center u", u, 0.5f, 0.0001f);
    near("center v", v, 0.5f, 0.0001f);
    check("positive pan remains inside", nm_subject_sample_uv(1920, 1080, 100, 50, 100, 50, 1, &u, &v));
    near("positive pan moves subject right", u, 0.5f, 0.0001f);
    near("positive pan moves subject down", v, 0.5f, 0.0001f);
    check("uniform zoom inside", nm_subject_sample_uv(1920, 1080, 400, 200, 0, 0, 2, &u, &v));
    near("zoom u", u, (200.0f + 960.0f) / 1920.0f, 0.0001f);
    near("zoom v", v, (100.0f + 540.0f) / 1080.0f, 0.0001f);
    check("out of source rejected", !nm_subject_sample_uv(640, 360, 319, 0, -500, 0, 1, &u, &v));
    check("bad dimensions rejected", !nm_subject_sample_uv(0, 360, 0, 0, 0, 0, 1, &u, &v));
    if (failures) {
        fprintf(stderr, "FAIL: %d geometry/color checks\n", failures);
        return 1;
    }
    puts("PASS: geometry/color checks (Release-safe)");
    return 0;
}
