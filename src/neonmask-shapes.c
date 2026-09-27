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
/* CPU reference mirrors the GPU's trimmed segments and selective arc
 * fillets. Only diagonal endpoint vertices c/d/f/g are rounded. */
static float arc2(nm_p2 p,nm_p2 start,nm_p2 finish,
                  nm_p2 center,float radius)
{
    nm_p2 rs=sub(start,center),re=sub(finish,center),v=sub(p,center);
    if(cross(rs,v)>=0.0f && cross(v,re)>=0.0f){
        float d=sqrtf(dot(v,v))-radius;
        return d*d;
    }
    return fminf(dot(sub(p,start),sub(p,start)),
                 dot(sub(p,finish),sub(p,finish)));
}
static int in_tri(nm_p2 p,nm_p2 a,nm_p2 b,nm_p2 c)
{
    float x=cross(sub(b,a),sub(p,a)),y=cross(sub(c,b),sub(p,b)),
          z=cross(sub(a,c),sub(p,c));
    return (x>=0&&y>=0&&z>=0)||(x<=0&&y<=0&&z<=0);
}
static int excised(nm_p2 p,nm_p2 start,nm_p2 vertex,nm_p2 finish,
                   nm_p2 center,float r)
{
    nm_p2 q=sub(p,center);
    return in_tri(p,start,vertex,finish) && dot(q,q)>r*r;
}
static float card(nm_p2 p,nm_p2 b,float detail,float radius)
{
    const float inv_root2=0.70710678118f,tan_pi8=0.41421356237f;
    const float cut=fmaxf(1.0f,fminf(b.x,b.y)*detail*1.65f);
    nm_p2 a={-b.x,-b.y},c={b.x-cut,-b.y},d={b.x,-b.y+cut};
    nm_p2 e={b.x,b.y},f={-b.x+cut,b.y},g={-b.x,b.y-cut};
    float side=cross(sub(c,a),sub(p,a));
    side=fminf(side,cross(sub(d,c),sub(p,c)));
    side=fminf(side,cross(sub(e,d),sub(p,d)));
    side=fminf(side,cross(sub(f,e),sub(p,e)));
    side=fminf(side,cross(sub(g,f),sub(p,f)));
    side=fminf(side,cross(sub(a,g),sub(p,g)));

    const float r=fminf(fmaxf(radius,0.0f),cut*1.20f);
    if(r<=0.0001f){
        float ds=edge2(p,a,c);
        ds=fminf(ds,edge2(p,c,d));ds=fminf(ds,edge2(p,d,e));
        ds=fminf(ds,edge2(p,e,f));ds=fminf(ds,edge2(p,f,g));
        ds=fminf(ds,edge2(p,g,a));
        return sqrtf(ds)*(side>=0?-1:1);
    }
    float t=r*tan_pi8,k=t*inv_root2;
    nm_p2 cs={c.x-t,c.y},ce={c.x+k,c.y+k};
    nm_p2 ds={d.x-k,d.y-k},de={d.x,d.y+t};
    nm_p2 fs={f.x+t,f.y},fe={f.x-k,f.y-k};
    nm_p2 gs={g.x+k,g.y+k},ge={g.x,g.y-t};
    nm_p2 cc={cs.x,cs.y+r},dc={de.x-r,de.y};
    nm_p2 fc={fs.x,fs.y-r},gc={ge.x+r,ge.y};
    float dist2=edge2(p,a,cs);
    dist2=fminf(dist2,edge2(p,ce,ds));
    dist2=fminf(dist2,edge2(p,de,e));
    dist2=fminf(dist2,edge2(p,e,fs));
    dist2=fminf(dist2,edge2(p,fe,gs));
    dist2=fminf(dist2,edge2(p,ge,a));
    dist2=fminf(dist2,arc2(p,cs,ce,cc,r));
    dist2=fminf(dist2,arc2(p,ds,de,dc,r));
    dist2=fminf(dist2,arc2(p,fs,fe,fc,r));
    dist2=fminf(dist2,arc2(p,gs,ge,gc,r));
    const int removed=excised(p,cs,c,ce,cc,r)||
                      excised(p,ds,d,de,dc,r)||
                      excised(p,fs,f,fe,fc,r)||
                      excised(p,gs,g,ge,gc,r);
    return sqrtf(dist2)*(side>=0&&!removed?-1:1);
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
    if(shape_id==NM_SHAPE_ANGLED_CARD) return card(p,b,detail,radius);
    return NAN;
}
