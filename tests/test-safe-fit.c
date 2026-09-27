/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-safe-fit.h"
#include <math.h>
#include <stdio.h>

static int failures;
static void check(const char *name, bool valid)
{
    if (!valid) { fprintf(stderr, "FAIL: %s\n", name); ++failures; }
}
static bool near(float a, float b)
{
    return fabsf(a-b) < 0.001f;
}

int main(void)
{
    nm_config c;
    nm_fit_result f;
    nm_config_defaults(&c);
    check("null arguments", !nm_safe_fit_calculate(NULL, 1920, 1080, &f));
    check("zero dimensions", !nm_safe_fit_calculate(&c, 0, 1080, &f));
    check("legacy geometry preserved", nm_safe_fit_calculate(&c, 1920, 1080, &f) &&
          f.fits && near(f.scale,1.0f) && near(f.half_width,777.6f) &&
          near(f.half_height,437.4f));
    c.safe_fit = true;
    check("center frame fits", nm_safe_fit_calculate(&c, 1920, 1080, &f) &&
          f.fits && near(f.scale,1.0f) && f.envelope_px > 0.0f);
    c.mask_x_px = 170.0f;
    c.mask_y_px = -40.0f;
    check("translated frame shrinks rather than moves",
          nm_safe_fit_calculate(&c,1920,1080,&f) && f.fits &&
          f.scale < 1.0f && f.scale > 0.0f &&
          c.mask_x_px == 170.0f && c.mask_y_px == -40.0f);
    const float max_x = f.half_width + fabsf(c.mask_x_px) + f.envelope_px;
    const float max_y = f.half_height + fabsf(c.mask_y_px) + f.envelope_px;
    check("edge coverage within source", max_x < 960.001f && max_y < 540.001f);
    c.shape_rotation_deg=40.0f;
    check("rotated bounds fit both axes",
          nm_safe_fit_calculate(&c,1920,1080,&f) && f.fits &&
          fabsf(cosf(40.0f*0.01745329252f))*f.half_width +
          fabsf(sinf(40.0f*0.01745329252f))*f.half_height +
          fabsf(c.mask_x_px)+f.envelope_px <=960.001f &&
          fabsf(sinf(40.0f*0.01745329252f))*f.half_width +
          fabsf(cosf(40.0f*0.01745329252f))*f.half_height +
          fabsf(c.mask_y_px)+f.envelope_px <=540.001f);
    c.mask_y_px = 600.0f;
    check("impossible offset fails closed",
          nm_safe_fit_calculate(&c,1920,1080,&f) && !f.fits);
    c.safe_fit = false;
    check("opt-out restores exact geometry at same offset",
          nm_safe_fit_calculate(&c,1920,1080,&f) && f.fits && near(f.scale,1.0f));

    c.safe_fit = true;
    c.mask_x_px = c.mask_y_px = c.shape_rotation_deg = 0.0f;
    c.glow_px = 80.0f; c.border_px = 32.0f;
    c.art_intensity = 1.0f; c.art_gap=16.0f;
    c.ornament_mode=NM_ORNAMENT_STREAMER;
    check("maximum luminous envelope covers bloom/tail",
          nm_safe_fit_calculate(&c,1920,1080,&f) && f.fits &&
          f.envelope_px >= 218.0f && f.scale < 1.0f);
    c.shape_id = NM_SHAPE_SVG_PATH;
    check("SVG uses identical fit dimensions",
          nm_safe_fit_calculate(&c,1920,1080,&f) && f.fits &&
          near(f.half_width,0.5f*1920.0f*c.mask_width*f.scale));
    if (failures) return 1;
    puts("PASS: opt-in safe-fit bounds, rotation, offset, bloom and legacy geometry");
    return 0;
}
