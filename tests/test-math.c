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
    nm_geometry framed = nm_make_geometry_framed(1920, 1080, 0.8f, 0.25f, 0.5f, 1.25f);
    near("independent width factor", framed.half_width, 384.0f, 0.01f);
    near("independent height factor", framed.half_height, 540.0f, 0.01f);
    near("framed corner", framed.radius, 96.0f, 0.01f);
    nm_point center = nm_mask_point(0.5f, 0.5f, 1920, 1080, 90, -45);
    near("mask center relative X", center.x, -90.0f, 0.01f);
    near("mask center relative Y", center.y, 45.0f, 0.01f);
    nm_point uv = nm_source_uv(0.5f, 0.5f, 1920, 1080, 96, -54, 1.0f);
    near("pan X shifts content right", uv.x, 0.45f, 0.0001f);
    near("pan Y shifts content down", uv.y, 0.55f, 0.0001f);
    nm_point zoom = nm_source_uv(1.0f, 0.5f, 1920, 1080, 0, 0, 2.0f);
    near("uniform zoom X", zoom.x, 0.75f, 0.0001f);
    near("uniform zoom Y", zoom.y, 0.5f, 0.0001f);
    check("off-source transparent contract", !nm_uv_inside(nm_source_uv(1, 0.5f, 1920, 1080, 0, 0, 0.5f)));
    check("inside-source contract", nm_uv_inside(nm_source_uv(0.5f, 0.5f, 1920, 1080, 0, 0, 1.0f)));
    check("invalid UV is outside", !nm_uv_inside((nm_point){NAN, 0.5f}));
    near("triangle top tip", nm_sd_regular_polygon(0,-50,50,50,3,0), 0, 0.001f);
    check("triangle interior", nm_sd_regular_polygon(0,0,50,50,3,0) < -10.0f);
    check("triangle exterior", nm_sd_regular_polygon(0,65,50,50,3,0) > 10.0f);
    check("hexagon interior", nm_sd_regular_polygon(0,0,50,50,6,0) < -10.0f);
    check("hexagon exterior", nm_sd_regular_polygon(55,0,50,50,6,0) > 4.0f);
    near("octagon vertex", nm_sd_regular_polygon(50*cosf(-3.14159265f/2+3.14159265f/8),
                 30*sinf(-3.14159265f/2+3.14159265f/8),50,30,8,0),0,0.001f);
    check("polygon invalid sides", nm_sd_regular_polygon(0,0,50,50,13,0)>1e5f);
    check("color packed R,G,B", nm_obs_rgba(255, 49, 221) == 0x00DD31FFu);
    check("color channels clamped", nm_obs_rgba(256, 511, 258) == 0x0002FF00u);
    if (failures) {
        fprintf(stderr, "FAIL: %d geometry/color checks\n", failures);
        return 1;
    }
    puts("PASS: geometry/color checks (Release-safe)");
    return 0;
}
