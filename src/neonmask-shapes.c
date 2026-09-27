/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-shapes.h"
#include "neonmask-presets.h"
#include "neonmask-math.h"
#include <math.h>

typedef struct { float x, y; } nm_p2;
static nm_p2 sub(nm_p2 a, nm_p2 b) { return (nm_p2){a.x-b.x,a.y-b.y}; }
static float dot(nm_p2 a,nm_p2 b) { return a.x*b.x+a.y*b.y; }
static float cross(nm_p2 a,nm_p2 b) { return a.x*b.y-a.y*b.x; }
static float edge2(nm_p2 p,nm_p2 a,nm_p2 b)
{
    nm_p2 ab=sub(b,a), ap=sub(p,a);
    float t=nm_clamp(dot(ap,ab)/fmaxf(dot(ab,ab),0.0001f),0.0f,1.0f);
    nm_p2 e={ap.x-t*ab.x,ap.y-t*ab.y};
    return dot(e,e);
}
static float box(nm_p2 p,nm_p2 b,float r)
{
    float x=fabsf(p.x)-(b.x-r), y=fabsf(p.y)-(b.y-r);
    return hypotf(fmaxf(x,0),fmaxf(y,0))+fminf(fmaxf(x,y),0)-r;
}
static float triangle(nm_p2 p,nm_p2 a,nm_p2 b,nm_p2 c)
{
    float d=fminf(edge2(p,a,b),fminf(edge2(p,b,c),edge2(p,c,a)));
    float side=fminf(cross(sub(b,a),sub(p,a)),
               fminf(cross(sub(c,b),sub(p,b)),cross(sub(a,c),sub(p,c))));
    return sqrtf(d)*(side>=0?-1:1);
}
static float card(nm_p2 p,nm_p2 b,float detail)
{
    const float cut=fmaxf(1.0f,fminf(b.x,b.y)*detail*1.65f);
    nm_p2 a={-b.x,-b.y},c={b.x-cut,-b.y},d={b.x,-b.y+cut};
    nm_p2 e={b.x,b.y},f={-b.x+cut,b.y},g={-b.x,b.y-cut};
    float ds=edge2(p,a,c);
    ds=fminf(ds,edge2(p,c,d));ds=fminf(ds,edge2(p,d,e));
    ds=fminf(ds,edge2(p,e,f));ds=fminf(ds,edge2(p,f,g));
    ds=fminf(ds,edge2(p,g,a));
    float side=cross(sub(c,a),sub(p,a));
    side=fminf(side,cross(sub(d,c),sub(p,c)));
    side=fminf(side,cross(sub(e,d),sub(p,d)));
    side=fminf(side,cross(sub(f,e),sub(p,e)));
    side=fminf(side,cross(sub(g,f),sub(p,f)));
    side=fminf(side,cross(sub(a,g),sub(p,g)));
    return sqrtf(ds)*(side>=0?-1:1);
}
float nm_authored_shape_distance(int shape_id,float x,float y,
                                 float half_width,float half_height,
                                 float radius,float detail)
{
    if (!isfinite(x)||!isfinite(y)||!isfinite(half_width)||
        !isfinite(half_height)||!isfinite(radius)||!isfinite(detail)||
        half_width<=0||half_height<=0) return NAN;
    detail=nm_clamp(detail,0.08f,0.35f);
    const nm_p2 p={x,y},b={half_width,half_height};
    if(shape_id==NM_SHAPE_CHAT_BUBBLE){
        const float tail=fmaxf(3.0f,b.y*detail*1.8f),bottom=b.y-tail;
        const nm_p2 body_p={x,y+tail*0.5f},body_b={b.x,b.y-tail*0.5f};
        const float body=box(body_p,body_b,fminf(fmaxf(radius,0),fminf(body_b.x,body_b.y)));
        const nm_p2 a={-0.78f*b.x,bottom-2.0f};
        const nm_p2 c={-0.34f*b.x,bottom-2.0f};
        const nm_p2 tip={-0.72f*b.x,b.y-1.0f};
        return fminf(body,triangle(p,a,c,tip));
    }
    if(shape_id==NM_SHAPE_ANGLED_CARD) return card(p,b,detail);
    return NAN;
}
