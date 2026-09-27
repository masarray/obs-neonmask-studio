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
    /* Expanded output adds real per-side pixels without changing the input. */
    nm_config_defaults(&c);
    check("legacy output dimensions unchanged",
          nm_safe_fit_calculate(&c,1920,1080,&f) && f.output_width==1920 &&
          f.output_height==1080 && f.pad_left==0 && f.pad_top==0 &&
          f.pad_right==0 && f.pad_bottom==0);
    c.expand_canvas=true;
    c.safe_fit=true; /* Expansion takes precedence without shrinking. */
    c.mask_x_px=170.0f; c.mask_y_px=-40.0f;
    check("positive X grows right while retaining authored geometry",
          nm_safe_fit_calculate(&c,1920,1080,&f) && f.fits &&
          near(f.half_width,777.6f) && near(f.half_height,437.4f) &&
          near(f.scale,1.0f) && f.pad_left==0 && f.pad_top==0 &&
          f.pad_right>0 && f.output_width==1920+f.pad_right &&
          f.output_height==1080+f.pad_bottom);
    check("right edge bounds include envelope",
          960.0f+c.mask_x_px+f.half_width+f.envelope_px <=
          1920.0f+(float)f.pad_right+0.001f);
    c.mask_x_px=-170.0f; c.mask_y_px=110.0f;
    check("negative X and positive Y need separate left/bottom padding",
          nm_safe_fit_calculate(&c,1920,1080,&f) && f.fits &&
          f.pad_left>0 && f.pad_bottom>0 && f.pad_right==0 &&
          f.output_width==1920+f.pad_left &&
          f.output_height==1080+f.pad_bottom);
    check("left-side input origin plus input width matches output coordinates",
          (float)f.pad_left+1920.0f <= (float)f.output_width &&
          c.subject_pan_x_px==0.0f && c.subject_zoom==1.0f);
    c.shape_rotation_deg=42.0f;
    check("rotated mask plus envelope lies within expanded bounds",
          nm_safe_fit_calculate(&c,1920,1080,&f) && f.fits &&
          960.0f+c.mask_x_px-
          fabsf(cosf(42.0f*0.01745329252f))*f.half_width-
          fabsf(sinf(42.0f*0.01745329252f))*f.half_height-
          f.envelope_px+(float)f.pad_left>=-0.001f &&
          540.0f+c.mask_y_px+
          fabsf(sinf(42.0f*0.01745329252f))*f.half_width+
          fabsf(cosf(42.0f*0.01745329252f))*f.half_height+
          f.envelope_px <= 1080.0f+(float)f.pad_bottom+0.001f);
    c.shape_rotation_deg=0.0f; c.mask_x_px=4096.0f;
    check("excessive offset fails closed at padding cap",
          nm_safe_fit_calculate(&c,1920,1080,&f) && !f.fits &&
          f.output_width==1920 && f.output_height==1080);
    c.mask_x_px=c.mask_y_px=0.0f;
    c.glow_px=80.0f; c.border_px=32.0f;
    c.art_intensity=1.0f; c.art_gap=16.0f;
    c.ornament_mode=NM_ORNAMENT_STREAMER;
    check("maximum luminous envelope fits all sides without shrink",
          nm_safe_fit_calculate(&c,640,360,&f) && f.fits &&
          f.envelope_px>=218.0f && near(f.scale,1.0f) &&
          f.pad_left>0 && f.pad_right>0 && f.pad_top>0 && f.pad_bottom>0 &&
          f.output_width==640+f.pad_left+f.pad_right &&
          f.output_height==360+f.pad_top+f.pad_bottom);
    c.shape_id=NM_SHAPE_SVG_PATH;
    check("SVG raster geometry stays input relative",
          nm_safe_fit_calculate(&c,640,360,&f) && f.fits &&
          near(f.half_width,320.0f*c.mask_width));
    check("8192 output cap rejects an excessive source",
          nm_safe_fit_calculate(&c,8200,360,&f) && !f.fits);
    c.expand_canvas=false; c.safe_fit=false;
    check("turning off expansion recovers legacy size",
          nm_safe_fit_calculate(&c,640,360,&f) && f.fits &&
          f.output_width==640 && f.output_height==360 && f.pad_left==0);
    if (failures) return 1;
    puts("PASS: safe-fit and expanded-output bounds, rotation, limits, bloom and legacy geometry");
    return 0;
}
