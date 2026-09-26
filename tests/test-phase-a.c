/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-config.h"
#include "neonmask-math.h"
#include <float.h>
#include <math.h>
#include <stdio.h>

static int failures;
static void test(const char *description, bool ok)
{
    if (!ok) {
        fprintf(stderr, "FAIL: %s\n", description);
        ++failures;
    }
}
static void near(const char *description, float value, float expected, float delta)
{
    test(description, value >= -FLT_MAX && value <= FLT_MAX &&
         fabsf(value-expected) <= delta);
}
int main(void)
{
    nm_config c;
    nm_config_defaults(&c);
    nm_geometry base = nm_make_geometry(1280,720,c.scale,c.roundness);
    nm_geometry neutral = nm_make_geometry_framed(1280,720,c.scale,c.roundness,c.mask_width,c.mask_height);
    near("v0/v1 center width unchanged", base.half_width, neutral.half_width, 0);
    near("v0/v1 center height unchanged", base.half_height, neutral.half_height, 0);
    near("v0/v1 roundness unchanged", base.radius, neutral.radius, 0);
    near("v0/v1 mask x unchanged", nm_mask_point(0.5f,0.5f,1280,720,c.mask_x,c.mask_y).x,0,0);
    near("v0/v1 source center unchanged", nm_source_uv(0.5f,0.5f,1280,720,c.subject_x,c.subject_y,c.subject_zoom).x,0.5f,0);
    test("schema v0 supported", nm_config_schema_supported(0));
    test("schema v1 supported", nm_config_schema_supported(1));
    test("schema v2 supported", nm_config_schema_supported(2));
    test("future schema rejected", !nm_config_schema_supported(3));

    c.mask_x=120; c.mask_y=-44;
    nm_point moving = nm_mask_point(0.5f,0.5f,1280,720,c.mask_x,c.mask_y);
    near("X position moves contour",moving.x,-120,0);
    near("Y position moves contour",moving.y,44,0);
    near("mask independent of source",nm_source_uv(0.5f,0.5f,1280,720,0,0,1).x,0.5f,0);

    c.subject_x=128; c.subject_y=-72; c.subject_zoom=2;
    nm_point sample=nm_source_uv(0.5f,0.5f,1280,720,c.subject_x,c.subject_y,c.subject_zoom);
    near("subject X pan independent",sample.x,0.45f,0.00001f);
    near("subject Y pan independent",sample.y,0.55f,0.00001f);
    nm_point image_only=nm_mask_point(0.5f,0.5f,1280,720,0,0);
    near("source pan leaves mask fixed",image_only.x,0,0);
    nm_point xedge=nm_source_uv(1,0.5f,1280,720,0,0,0.5f);
    test("zoom out edge transparent",!nm_uv_inside(xedge));
    test("zero-sized source not valid",!nm_uv_inside(nm_source_uv(0.5f,0.5f,0,720,0,0,1)));

    c.mask_width=0.5f; c.mask_height=1.25f;
    nm_geometry stretched=nm_make_geometry_framed(1280,720,c.scale,c.roundness,c.mask_width,c.mask_height);
    near("independent shape width",stretched.half_width,neutral.half_width*0.5f,0.001f);
    near("independent shape height",stretched.half_height,neutral.half_height*1.25f,0.001f);
    const float circle=fminf(stretched.half_width,stretched.half_height);
    test("circle must remain true circle",nm_sd_circle(circle,0,circle)==0.0f);
    test("ellipse nonuniform shape permitted",nm_sd_ellipse_approx(stretched.half_width,0,stretched.half_width,stretched.half_height)==0.0f);

    for(int n=3;n<=12;n++){
        test("polygon center inside",nm_sd_regular_polygon(0,0,61,43,n,0)<0);
        test("polygon distant corner outside",nm_sd_regular_polygon(100,100,61,43,n,0)>0);
        const float pi=3.14159265358979323846f;
        const float theta=-pi/2+((n%2)==0?pi/n:0);
        near("vertex lies on contour",nm_sd_regular_polygon(61*cosf(theta),43*sinf(theta),61,43,n,0),0,0.001f);
    }

    if(failures)return 1;
    puts("PASS: Phase A eight shapes, framing, zoom, source UV and v0/v1 compatibility");
    return 0;
}
