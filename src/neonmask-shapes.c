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
/* Distance to a true exposed quarter-arc (not its hidden full circle). */
static float quarter_arc2(nm_p2 p,nm_p2 center,float r,float sx,float sy)
{
    const nm_p2 q={(p.x-center.x)*sx,(p.y-center.y)*sy};
    if(q.x>=0.0f&&q.y>=0.0f){
        const float d=hypotf(q.x,q.y)-r;
        return d*d;
    }
    const nm_p2 ex={center.x+sx*r,center.y};
    const nm_p2 ey={center.x,center.y+sy*r};
    return fminf(dot(sub(p,ex),sub(p,ex)),
                 dot(sub(p,ey),sub(p,ey)));
}

/* A bubble has ONE exposed perimeter. A min(box,triangle) is a valid
 * occupancy union but is NOT a true interior distance: the box's hidden
 * bottom and triangle's hidden base create a bright phantom horizontal rim.
 * Evaluate only exterior paths; the same distance drives mask and neon. */
static float bubble(nm_p2 p,nm_p2 b,float radius,float detail)
{
    const float tail=fmaxf(3.0f,b.y*detail*1.8f);
    const float bottom=b.y-tail,top=-b.y;
    const nm_p2 left={-0.78f*b.x,bottom};
    const nm_p2 right={-0.34f*b.x,bottom};
    const nm_p2 tip={-0.72f*b.x,b.y-1.0f};
    /* Keep the bottom-left body tangent before the start of the tail. */
    const float r=fminf(fmaxf(radius,0.0f),
                        fminf(b.y-0.5f*tail,0.22f*b.x));
    const nm_p2 lv=sub(left,tip),rv=sub(right,tip);
    const float ll=hypotf(lv.x,lv.y),rl=hypotf(rv.x,rv.y);
    const nm_p2 ul={lv.x/ll,lv.y/ll},ur={rv.x/rl,rv.y/rl};
    const float cosine=nm_clamp(dot(ul,ur),-0.98f,0.98f);
    const float sin_half=sqrtf(0.5f*(1.0f-cosine));
    const float tan_half=sin_half/sqrtf(fmaxf(0.01f,0.5f*(1.0f+cosine)));
    const float desired=fminf(1.8f,fminf(tail*0.12f,r*0.24f));
    const float trim=fminf(desired/fmaxf(tan_half,0.01f),
                           fminf(ll,rl)*0.32f);
    const float tr=trim*tan_half;
    const nm_p2 right_tangent={tip.x+ur.x*trim,tip.y+ur.y*trim};
    const nm_p2 left_tangent={tip.x+ul.x*trim,tip.y+ul.y*trim};
    const nm_p2 bis={ul.x+ur.x,ul.y+ur.y};
    const float bis_len=hypotf(bis.x,bis.y);
    const float offset=tr/fmaxf(sin_half*bis_len,0.0001f);
    const nm_p2 tip_center={tip.x+bis.x*offset,tip.y+bis.y*offset};

    /* Straight exposed rails; the internal body-bottom span under the
     * leaf-tail is deliberately omitted from the distance calculation. */
    const nm_p2 tl={-b.x+r,top},top_right={b.x-r,top};
    const nm_p2 rt={b.x,top+r},rb={b.x,bottom-r};
    const nm_p2 br={b.x-r,bottom},bl={-b.x+r,bottom};
    const nm_p2 lb={-b.x,bottom-r},lt={-b.x,top+r};
    float ds=edge2(p,tl,top_right);
    ds=fminf(ds,edge2(p,rt,rb));
    ds=fminf(ds,edge2(p,br,right));
    ds=fminf(ds,edge2(p,right,right_tangent));
    ds=fminf(ds,edge2(p,left_tangent,left));
    ds=fminf(ds,edge2(p,left,bl));
    ds=fminf(ds,edge2(p,lb,lt));
    ds=fminf(ds,quarter_arc2(p,(nm_p2){b.x-r,top+r},r,1,-1));
    ds=fminf(ds,quarter_arc2(p,(nm_p2){b.x-r,bottom-r},r,1,1));
    ds=fminf(ds,quarter_arc2(p,(nm_p2){-b.x+r,bottom-r},r,-1,1));
    ds=fminf(ds,quarter_arc2(p,(nm_p2){-b.x+r,top+r},r,-1,-1));
    if(tr>0.0001f)
        ds=fminf(ds,arc2(p,right_tangent,left_tangent,tip_center,tr));
    else
        ds=fminf(ds,edge2(p,right_tangent,left_tangent));

    const nm_p2 body_p={p.x,p.y+0.5f*tail};
    const nm_p2 body_b={b.x,b.y-0.5f*tail};
    const int inside=box(body_p,body_b,r)<=0.0f ||
                     triangle(p,right,tip,left)<=0.0f;
    const int removed=tr>0.0001f &&
        excised(p,right_tangent,tip,left_tangent,tip_center,tr);
    return sqrtf(ds)*(inside&&!removed?-1.0f:1.0f);
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
    if(shape_id==NM_SHAPE_CHAT_BUBBLE) return bubble(p,b,radius,detail);
    if(shape_id==NM_SHAPE_ANGLED_CARD) return card(p,b,detail,radius);
    return NAN;
}
