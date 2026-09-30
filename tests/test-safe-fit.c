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

    nm_config_defaults(&c);
    c.shape_id=NM_SHAPE_TECH_HUD;
    c.ornament_mode=NM_ORNAMENT_TECH_HUD;
    c.art_intensity=1.0f; c.art_gap=24.0f;
    c.ornament_width_px=18.0f;
    c.border_px=5.0f; c.glow_px=14.0f; c.show_glow=false;
    c.expand_canvas=true;
    check("Tech HUD D4F true-gap envelope participates in D3",
          nm_safe_fit_calculate(&c,640,360,&f) && f.fits &&
          f.envelope_px>=44.0f);
    nm_config_defaults(&c);
    c.shape_id=NM_SHAPE_GAME_UI;
    c.ornament_mode=NM_ORNAMENT_GAME_UI;
    c.art_intensity=1.0f; c.art_gap=14.0f;
    c.ornament_width_px=16.0f;
    c.border_px=5.0f; c.glow_px=12.0f; c.show_glow=false;
    c.expand_canvas=true;
    check("Game UI D4F true-gap envelope participates in D3",
          nm_safe_fit_calculate(&c,640,360,&f) && f.fits &&
          f.envelope_px>=32.0f);

    /* D4I regression from real OBS screenshots: dedicated outer-L controls
     * must remain entirely inside an unexpanded source even when an existing
     * custom scene still has safe_fit=false. */
    nm_config_defaults(&c);
    c.safe_fit=false;
    c.mask_width=0.80f; c.mask_height=0.80f;
    c.shape_id=NM_SHAPE_GAME_UI;
    c.ornament_mode=NM_ORNAMENT_GAME_UI;
    c.art_intensity=1.0f;
    c.art_gap=30.0f; c.ornament_width_px=38.5f;
    c.border_px=22.5f; c.show_glow=false;
    check("D4I Game UI auto-fits outer L without preset/safe-fit toggle",
          nm_safe_fit_calculate(&c,640,480,&f) && f.fits &&
          f.scale<1.0f && f.scale>0.0f &&
          f.half_height+f.envelope_px<=240.001f &&
          f.half_width+f.envelope_px<=320.001f);

    nm_config_defaults(&c);
    c.safe_fit=false;
    c.mask_width=0.78f; c.mask_height=0.78f;
    c.shape_id=NM_SHAPE_TECH_HUD;
    c.ornament_mode=NM_ORNAMENT_TECH_HUD;
    c.art_intensity=1.0f;
    c.art_gap=31.0f; c.ornament_width_px=46.5f;
    c.border_px=19.0f; c.show_glow=false;
    check("D4I Tech HUD auto-fits outer L without preset/safe-fit toggle",
          nm_safe_fit_calculate(&c,640,480,&f) && f.fits &&
          f.scale<1.0f && f.scale>0.0f &&
          f.half_height+f.envelope_px<=240.001f &&
          f.half_width+f.envelope_px<=320.001f);

    c.expand_canvas=true;
    check("D4I expansion preserves authored Tech HUD mask size",
          nm_safe_fit_calculate(&c,640,480,&f) && f.fits &&
          near(f.scale,1.0f) && f.pad_top>0 && f.pad_right>0);

    /* D4J: connected L geometry is placed in local shape coordinates before
     * shape rotation. At 45 degrees its local gap+thickness contributes on
     * both output axes; the old scalar post-rotation envelope under-sized
     * this case by roughly (sqrt(2)-1)*(gap+width). */
    nm_config_defaults(&c);
    c.safe_fit=false;
    c.mask_width=0.80f; c.mask_height=0.80f;
    c.shape_id=NM_SHAPE_GAME_UI;
    c.ornament_mode=NM_ORNAMENT_GAME_UI;
    c.art_intensity=1.0f;
    c.art_gap=30.0f; c.ornament_width_px=38.5f;
    c.border_px=22.5f; c.show_glow=false;
    c.shape_rotation_deg=45.0f;
    check("D4J rotated Game UI auto-fit includes local outer-L reach",
          nm_safe_fit_calculate(&c,640,480,&f) && f.fits &&
          f.scale<1.0f && f.scale>0.0f &&
          0.70710679f*f.half_width + 0.70710679f*f.half_height +
          1.41421356f*(c.art_gap+c.ornament_width_px) + 2.0f <=
          240.01f);

    c.expand_canvas=true;
    check("D4J rotated Game UI expansion pads the real connected-L AABB",
          nm_safe_fit_calculate(&c,640,480,&f) && f.fits &&
          near(f.scale,1.0f) &&
          320.0f + 0.70710679f*f.half_width +
          0.70710679f*f.half_height +
          1.41421356f*(c.art_gap+c.ornament_width_px) + 2.0f <=
          (float)f.output_width + 0.01f &&
          240.0f + 0.70710679f*f.half_width +
          0.70710679f*f.half_height +
          1.41421356f*(c.art_gap+c.ornament_width_px) + 2.0f <=
          (float)f.output_height + 0.01f &&
          f.pad_top>=176u && f.pad_right>=96u);

    /* P6B: the outer 2/3 ring is genuine geometry beyond the circular mask.
     * Existing/custom scenes get the same automatic clipping protection as
     * authored outer Ls; expansion must preserve the authored radius. */
    nm_config_defaults(&c);
    c.safe_fit=false;
    c.mask_width=0.96f; c.mask_height=0.96f;
    c.shape_id=NM_SHAPE_CIRCLE;
    c.ornament_mode=NM_ORNAMENT_DUAL_RING;
    c.art_intensity=1.0f; c.art_gap=8.0f;
    c.ornament_width_px=13.0f; c.inner_rail_width_px=10.0f;
    c.border_px=0.9f; c.show_glow=false;
    check("P6D dual ring auto-fits full external orbit stack",
          nm_safe_fit_calculate(&c,320,320,&f) && f.fits &&
          f.scale<1.0f && f.scale>0.0f &&
          f.half_width+f.envelope_px<=160.001f &&
          f.half_height+f.envelope_px<=160.001f);
    c.expand_canvas=true;
    check("P6D dual ring expansion preserves authored external orbits",
          nm_safe_fit_calculate(&c,320,320,&f) && f.fits &&
          near(f.scale,1.0f) && f.pad_left>0 && f.pad_top>0 &&
          f.pad_right>0 && f.pad_bottom>0 &&
          f.envelope_px>=37.0f);
    /* D3 gate: deterministic coverage of input/output dimensions, asymmetric
     * offsets and rotated AABBs over landscape, square and portrait captures.
     * This tests geometry, not OBS scene-item transform semantics. */
    {
        const uint32_t sizes[][2]={{320u,180u},{640u,480u},{640u,640u},
                                  {1080u,1920u},{1920u,1080u},{3840u,2160u}};
        const float shifts[]={-700.0f,-170.0f,0.0f,170.0f,700.0f};
        const float rotations[]={-90.0f,-37.0f,0.0f,37.0f,90.0f};
        unsigned valid=0,limited=0;
        nm_config_defaults(&c);
        c.expand_canvas=true;
        c.safe_fit=true;
        for(unsigned si=0;si<sizeof(sizes)/sizeof(sizes[0]);++si)
          for(unsigned xi=0;xi<sizeof(shifts)/sizeof(shifts[0]);++xi)
            for(unsigned yi=0;yi<sizeof(shifts)/sizeof(shifts[0]);++yi)
              for(unsigned ri=0;ri<sizeof(rotations)/sizeof(rotations[0]);++ri){
                const uint32_t w=sizes[si][0],h=sizes[si][1];
                c.mask_x_px=shifts[xi]; c.mask_y_px=shifts[yi];
                c.shape_rotation_deg=rotations[ri];
                check("D3 matrix calculation",nm_safe_fit_calculate(&c,w,h,&f));
                if(!f.fits){
                    ++limited;
                    check("rejected geometry retains original callback size",
                          f.output_width==w && f.output_height==h);
                    continue;
                }
                ++valid;
                const float t=fabsf(cosf(c.shape_rotation_deg*0.01745329252f));
                const float s=fabsf(sinf(c.shape_rotation_deg*0.01745329252f));
                const float ex=t*f.half_width+s*f.half_height+f.envelope_px;
                const float ey=s*f.half_width+t*f.half_height+f.envelope_px;
                const float x=0.5f*w+c.mask_x_px+f.pad_left;
                const float y=0.5f*h+c.mask_y_px+f.pad_top;
                check("D3 matrix X min/max",x-ex>=-0.02f &&
                      x+ex<=(float)f.output_width+0.02f);
                check("D3 matrix Y min/max",y-ey>=-0.02f &&
                      y+ey<=(float)f.output_height+0.02f);
                check("D3 matrix output and padding identity",
                      f.output_width==w+f.pad_left+f.pad_right &&
                      f.output_height==h+f.pad_top+f.pad_bottom);
                check("D3 matrix bounded output",f.pad_left<=512 && f.pad_top<=512 &&
                      f.pad_right<=512 && f.pad_bottom<=512 &&
                      f.output_width<=8192 && f.output_height<=8192);
                check("D3 expansion never shrinks authored mask",
                      near(f.scale,1.0f) &&
                      near(f.half_width,0.5f*w*c.mask_width) &&
                      near(f.half_height,0.5f*h*c.mask_height));
              }
        check("D3 matrix exercises fit and cap cases",valid>100 && limited>0);
    }
    if (failures) return 1;
    puts("PASS: safe-fit and expanded-output bounds, rotation, limits, bloom and legacy geometry");
    return 0;
}
