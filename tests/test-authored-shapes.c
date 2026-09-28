/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-shapes.h"
#include "neonmask-presets.h"
#include <math.h>
#include <stdio.h>
static int failures;
static void check(const char *name,int pass)
{
    if(!pass){fprintf(stderr,"FAIL: %s\n",name);++failures;}
}
int main(void)
{
    const int bubble=NM_SHAPE_CHAT_BUBBLE,card=NM_SHAPE_ANGLED_CARD;
    const float bx=24.0f,by=24.0f,r=5.0f,detail=0.23f;
    const float bottom=by-fmaxf(3.0f,by*detail*1.8f);
#define D(ID,X,Y) nm_authored_shape_distance(ID,X,Y,bx,by,r,detail)
    check("bubble face interior",D(bubble,0,0)<-5.0f);
    check("bubble tail contains source image",D(bubble,-16.5f,20.5f)<-0.3f);
    check("bubble outside beside tail",D(bubble,-5.0f,20.5f)>2.0f);
    check("bubble upper exterior",D(bubble,0,-26.0f)>1.0f);
    check("bubble shoulder fully inside the outer contour",
          D(bubble,-15.0f,bottom-1.0f)<-2.8f);
    check("bubble internal seam is not an artificial neon edge",
          D(bubble,-13.0f,bottom)<-2.8f);
    check("bubble internal seam remains clear just below body",
          D(bubble,-13.0f,bottom+0.4f)<-2.5f);
    check("bubble exposed lower rail remains at zero distance",
          fabsf(D(bubble,-5.0f,bottom))<0.001f);
    const float bubble_tip_distance=D(bubble,-0.72f*bx,by-1.0f);
    if(!(bubble_tip_distance>0.6f))
        fprintf(stderr,"DEBUG: rounded Bubble tip distance=%g (expected positive)\\n",
                bubble_tip_distance);
    check("bubble rounded tip removes old sharp pixel",bubble_tip_distance>0.6f);
    check("bubble leaf interior still contains source",
          D(bubble,-17.0f,20.0f)<-0.5f);
    check("roundness zero retains the geometric tail endpoint",
          fabsf(nm_authored_shape_distance(bubble,-0.72f*bx,by-1.0f,
                                          bx,by,0.0f,detail))<0.001f);
    check("bubble external shoulder remains on contour",
          fabsf(D(bubble,-0.34f*bx,bottom))<0.001f);
    /* D4A v2 default point model must preserve the legacy Bubble contour. */
    const float bottom_norm=1.0f-1.8f*detail;
    nm_bubble_controls old={
        -1.0f,-1.0f, 1.0f,-1.0f,
         1.0f,bottom_norm,-1.0f,bottom_norm,
         0.11f,0.33f,0.14f,detail
    };
    for(int yy=-26;yy<=26;yy+=2) for(int xx=-26;xx<=26;xx+=2) {
        const float oldD=D(bubble,(float)xx,(float)yy);
        const float newD=nm_bubble_custom_distance((float)xx,(float)yy,bx,by,r,&old);
        check("D4A freeform defaults match legacy exterior",fabsf(oldD-newD)<0.0005f);
    }

    nm_bubble_controls custom={
        -0.85f,-0.90f, 0.75f,-0.70f,
         0.90f, 0.65f,-0.65f, 0.45f,
         0.18f,0.42f,0.28f,detail
    };
#define BC(X,Y) nm_bubble_custom_distance((X),(Y),bx,by,r,&custom)
    check("D4A freeform center remains filled",BC(0,0)<-2.0f);
    check("D4A old flat top point is now outside",BC(0,-24.0f)>1.0f);
    check("D4A right side independently pulled inward",BC(23.0f,0.0f)>0.8f);
    check("D4A left side independently pulled inward",BC(-23.0f,0.0f)>0.8f);
    check("D4A slanted top rail is contour",
          fabsf(BC((-0.85f+0.75f)*0.5f*bx,
                   (-0.90f-0.70f)*0.5f*by))<0.02f);
    const float x80=(-0.65f+(0.90f+0.65f)*0.80f)*bx;
    const float y80=(0.45f+(0.65f-0.45f)*0.80f)*by;
    check("D4A slanted bottom rail remains contour",fabsf(BC(x80,y80))<0.03f);
    check("D4A tail follows slanted bottom",BC(-4.5f,16.0f)<0.0f);
    check("D4A area beside tail remains transparent",BC(15.0f,20.0f)>1.0f);
    check("D4A hidden tail base stays away from neon",BC(-4.0f,12.5f)<-0.2f);
    check("D4A null control rejected",
          isnan(nm_bubble_custom_distance(0,0,bx,by,r,NULL)));

    custom.tl_x=-1.0f; custom.tl_y=-0.35f;
    custom.tr_x=0.35f; custom.tr_y=-1.0f;
    custom.br_x=1.0f; custom.br_y=0.92f;
    custom.bl_x=-0.35f; custom.bl_y=0.35f;
    custom.tail_start=0.02f; custom.tail_end=0.98f;
    custom.tail_tip_pos=1.0f; custom.tail_depth=0.35f;
    for(int i=-24;i<=24;++i) for(int j=-24;j<=24;++j)
        check("D4A extreme valid freeform stays finite",
              isfinite(nm_bubble_custom_distance(0.4f*i,0.3f*j,
                                                 8.0f,6.0f,30.0f,&custom)));
#undef BC
    check("chamfered card center inside",D(card,0,0)<-5.0f);
    check("card top right removed",D(card,21,-21)>0.2f);
    check("card top right inside diagonal",D(card,13,-21)<-0.2f);
    check("card lower left removed",D(card,-21,21)>0.2f);
    check("card lower right untouched",D(card,21,21)<-0.2f);
    check("card diagonal on contour",fabsf(D(card,24.0f-4.554f,-24.0f+4.554f))<1.0f);
    /* The existing Roundness slider controls only the four endpoints of the
     * two diagonal cuts. Zero must retain D1 straight-chamfer geometry;
     * both remaining 90-degree corners must never be rounded. */
    const float cut=bx*detail*1.65f;
    const float cx=bx-cut, fy=by;
    const float sharp=nm_authored_shape_distance(card,cx,-by,bx,by,0.0f,detail);
    const float leaf=nm_authored_shape_distance(card,cx,-by,bx,by,10.5f,detail);
    check("zero roundness original diagonal endpoint",fabsf(sharp)<0.0001f);
    check("rounding removes cut tip",leaf>0.4f);
    check("rounding removes bottom-left cut tip",
          nm_authored_shape_distance(card,-cx,fy,bx,by,10.5f,detail)>0.4f);
    check("full arc removes former cut shoulder",
          nm_authored_shape_distance(card,14.5f,-23.5f,bx,by,24.0f,detail)>0.4f);
    check("geometric shoulder remains inside at zero",
          nm_authored_shape_distance(card,14.5f,-23.5f,bx,by,0.0f,detail)<-0.2f);
    check("upper-left right-angle remains square",
          nm_authored_shape_distance(card,-23.5f,-23.5f,bx,by,10.5f,detail)<-0.49f);
    check("lower-right right-angle remains square",
          nm_authored_shape_distance(card,23.5f,23.5f,bx,by,10.5f,detail)<-0.49f);
    check("upper-left corner remains at boundary",
          fabsf(nm_authored_shape_distance(card,-bx,-by,bx,by,10.5f,detail))<0.0001f);
    check("lower-right corner remains at boundary",
          fabsf(nm_authored_shape_distance(card,bx,by,bx,by,10.5f,detail))<0.0001f);
    /* Roundness=1 must be a complete quarter-circle, NOT four small arcs
     * separated by an obvious flat diagonal. Its two 45-degree sub-arcs
     * have one center and meet with a continuous tangent. */
    const float root2=1.41421356237f;
    const float rmax=cut*(1.0f+1.0f/root2);
    const float tx=bx-rmax;
    const float mid=bx-0.5f*cut;
    const float max_mid=nm_authored_shape_distance(card,mid,-mid,bx,by,by,detail);
    check("max quarter-circle midpoint is contour",fabsf(max_mid)<0.002f);
    check("max quarter-circle top tangent is contour",
          fabsf(nm_authored_shape_distance(card,tx,-by,bx,by,by,detail))<0.002f);
    check("max quarter-circle right tangent is contour",
          fabsf(nm_authored_shape_distance(card,bx,-tx,bx,by,by,detail))<0.002f);
    check("full arc curves outward on both sides of center seam",
          nm_authored_shape_distance(card,mid+1.0f,-mid,bx,by,by,detail)>0.20f &&
          nm_authored_shape_distance(card,mid-1.0f,-mid,bx,by,by,detail)<-0.20f);
    check("maximum removes diagonal endpoint more than mid slider",
          nm_authored_shape_distance(card,cx,-by,bx,by,by,detail)>
          nm_authored_shape_distance(card,cx,-by,bx,by,0.5f*by,detail)+0.2f);
    check("full arc is centrally symmetric on bottom left",
          fabsf(nm_authored_shape_distance(card,mid,-mid,bx,by,by,detail)-
                 nm_authored_shape_distance(card,-mid,mid,bx,by,by,detail))<0.002f);
    check("max never rounds protected upper-left corner",
          fabsf(nm_authored_shape_distance(card,-bx,-by,bx,by,by,detail))<0.0001f);
    check("max never rounds protected lower-right corner",
          fabsf(nm_authored_shape_distance(card,bx,by,bx,by,by,detail))<0.0001f);
    check("fillet respects shape-detail change",
          nm_authored_shape_distance(card,cx,-by,bx,by,10.5f,0.35f)>0.0f);
    check("large roundness stays finite",
          isfinite(nm_authored_shape_distance(card,cx,-by,bx,by,999.0f,detail)));
    /* Signed-distance sign symmetry across the two authored cut corners. */
    for(int i=0;i<=10;i++){
        float x=(float)i*.7f,y=-(float)i*.7f;
        float tr=nm_authored_shape_distance(card,cx+x,-by-y,bx,by,10.5f,detail);
        float bl=nm_authored_shape_distance(card,-cx-x,by+y,bx,by,10.5f,detail);
        check("cut pair central symmetry",fabsf(tr-bl)<0.0005f);
    }

    /* D2.3: a truly clipped, concave HUD silhouette, not a rectangle
     * with a decorative notch painted over the original source. */
    const int hud=NM_SHAPE_HUD_PANEL,squircle=NM_SHAPE_SQUIRCLE;
    const float hud_cut=fminf(fmaxf(2.0f,bx*detail*1.8f),bx*0.45f);
    check("HUD center source remains visible",D(hud,0,0)<-2.0f);
    check("HUD top-right corner truly clipped",D(hud,23,-23)>1.0f);
    check("HUD stepped lower-left notch actually transparent",
          D(hud,-bx+hud_cut*0.70f,by-hud_cut*0.50f)>0.8f);
    check("HUD inner step remains filled",
          D(hud,-bx+hud_cut*1.25f,by-hud_cut*0.50f)<-0.5f);
    check("HUD exposed bottom rail is contour",
          fabsf(D(hud,0,by))<0.001f);
    /* Both deliberately bevelled notch transitions remain source-aligned. */
    check("HUD double bevel carries a continuous signed contour",
          fabsf(D(hud,-bx+hud_cut*1.65f,by-hud_cut*0.20f))<0.002f &&
          fabsf(D(hud,-bx+hud_cut*0.95f,by-hud_cut*0.38f))<0.002f);
    check("HUD upper rail is contour",
          fabsf(D(hud,0,-by))<0.001f);
    check("HUD detail changes cut size",
          fabsf(D(hud,23,-23)-
                nm_authored_shape_distance(hud,23,-23,bx,by,r,0.35f))>0.20f);

    /* D4B Tech HUD: TL/BR are real mask chamfers, while TR/BL remain square
     * anchors for the separate external bracket ornament. */
    const int tech=NM_SHAPE_TECH_HUD;
    const float tech_cut=fminf(fmaxf(2.0f,bx*detail*1.75f),bx*0.42f);
    check("Tech HUD center contains source",D(tech,0,0)<-5.0f);
    check("Tech HUD top-left cut removes source",D(tech,-23,-23)>1.0f);
    check("Tech HUD top-right stays square",D(tech,23,-23)<-0.5f);
    check("Tech HUD bottom-right cut removes source",D(tech,23,23)>1.0f);
    check("Tech HUD bottom-left stays square",D(tech,-23,23)<-0.5f);
    check("Tech HUD TL diagonal is exact contour",
          fabsf(D(tech,-bx+tech_cut*0.5f,-by+tech_cut*0.5f))<0.002f);
    check("Tech HUD BR diagonal is exact contour",
          fabsf(D(tech,bx-tech_cut*0.5f,by-tech_cut*0.5f))<0.002f);
    check("Tech HUD detail visibly changes chamfer",
          D(tech,-18,-18)<0.0f &&
          nm_authored_shape_distance(tech,-18,-18,bx,by,r,0.35f)>0.0f);

    /* Smooth superellipse has a continuous, aspect-correct implicit
     * contour; its exponent changes with the existing shape_detail. */
    check("squircle center contains source",D(squircle,0,0)<-10.0f);
    check("squircle axis X on contour",fabsf(D(squircle,bx,0))<0.001f);
    check("squircle axis Y on contour",fabsf(D(squircle,0,by))<0.001f);
    check("squircle corner clipped",D(squircle,bx,by)>1.0f);
    check("squircle diagonal inside",D(squircle,0.5f*bx,0.5f*by)<-2.0f);
    check("squircle exponent distinguishes shape detail",
          nm_authored_shape_distance(squircle,0.85f*bx,0.85f*by,
                                     bx,by,r,0.08f)>0.0f &&
          nm_authored_shape_distance(squircle,0.85f*bx,0.85f*by,
                                     bx,by,r,0.35f)<0.0f);
    check("squircle pixel-space X normal",
          fabsf(D(squircle,bx+1.0f,0)-1.0f)<0.25f);
    check("squircle pixel-space Y normal",
          fabsf(D(squircle,0,by+1.0f)-1.0f)<0.25f);
    for(int i=0;i<3;i++){
        const float w=i==0?48.0f:24.0f;
        const float h=i==1?48.0f:24.0f;
        check("HUD remains finite on wide/portrait",
              isfinite(nm_authored_shape_distance(hud,-w*0.8f,h*0.6f,
                                                    w,h,r,0.35f)));
        check("squircle remains finite on wide/portrait",
              isfinite(nm_authored_shape_distance(squircle,w*0.8f,h*0.6f,
                                                    w,h,r,0.35f)));
    }

    check("non-D1 shape rejected",isnan(D(NM_SHAPE_CIRCLE,0,0)));
    for(int i=-80;i<=80;i++)for(int j=-80;j<=80;j++){
        const float x=i*.5f,y=j*.5f;
        check("bubble finite",isfinite(D(bubble,x,y)));
        check("card finite",isfinite(D(card,x,y)));
        check("HUD finite",isfinite(D(hud,x,y)));
        check("squircle finite",isfinite(D(squircle,x,y)));
    }
#undef D
    if(failures)return 1;
    puts("PASS: bubble/quarter-arc card, HUD notch and squircle authored distances");
    return 0;
}
