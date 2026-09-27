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
    check("bubble shoulder union hides seam",D(bubble,-15.0f,bottom-1.0f)<-0.8f);
    check("bubble tip is near contour",fabsf(D(bubble,-0.72f*bx,by-1.0f))<0.0001f);
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
    check("leaf fillet removes former cut shoulder",
          nm_authored_shape_distance(card,14.5f,-23.5f,bx,by,10.5f,detail)>0.1f);
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
    check("fillet respects shape-detail change",
          nm_authored_shape_distance(card,cx,-by,bx,by,10.5f,0.35f)<0.0f);
    check("large roundness stays finite",
          isfinite(nm_authored_shape_distance(card,cx,-by,bx,by,999.0f,detail)));
    /* Signed-distance sign symmetry across the two authored cut corners. */
    for(int i=0;i<=10;i++){
        float x=(float)i*.7f,y=-(float)i*.7f;
        float tr=nm_authored_shape_distance(card,cx+x,-by-y,bx,by,10.5f,detail);
        float bl=nm_authored_shape_distance(card,-cx-x,by+y,bx,by,10.5f,detail);
        check("cut pair central symmetry",fabsf(tr-bl)<0.0005f);
    }

    check("non-D1 shape rejected",isnan(D(NM_SHAPE_CIRCLE,0,0)));
    for(int i=-80;i<=80;i++)for(int j=-80;j<=80;j++){
        const float x=i*.5f,y=j*.5f;
        check("bubble finite",isfinite(D(bubble,x,y)));
        check("card finite",isfinite(D(card,x,y)));
    }
#undef D
    if(failures)return 1;
    puts("PASS: integrated bubble, selective leaf-cut roundness and untouched square corners");
    return 0;
}
