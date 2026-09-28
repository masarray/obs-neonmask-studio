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
    /* Treat boundary vertices consistently despite float cancellation.
     * Shared by selective card fillets and the leaf-tip excision. */
    const float eps=0.0001f;
    return (x>=-eps&&y>=-eps&&z>=-eps)||
           (x<=eps&&y<=eps&&z<=eps);
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
static float bubble(nm_p2 p,nm_p2 b,float radius,
                    const nm_bubble_controls *c)
{
    const float tail=fmaxf(3.0f,b.y*c->tail_depth*1.8f);
    const float lx=-b.x+2.0f*b.x*c->left_inset;
    const float rx=b.x-2.0f*b.x*c->right_inset;
    const float top=-b.y+2.0f*b.y*c->top_inset;
    const float bottom=b.y-tail-2.0f*b.y*c->bottom_inset;
    const float r=fminf(fmaxf(radius,0.0f),
                      fminf(0.5f*(bottom-top),fminf(0.22f*b.x,0.24f*(rx-lx))));
    const float rail_l=lx+r,rail_r=rx-r;
    const float min_base=fminf(fmaxf(1.0f,0.07f*b.x),0.4f*(rail_r-rail_l));
    const float base_l=nm_clamp(b.x*c->tail_left,rail_l,rail_r-min_base);
    const float base_r=nm_clamp(b.x*c->tail_right,base_l+min_base,rail_r);
    const nm_p2 left={base_l,bottom},right={base_r,bottom};
    const nm_p2 tip={b.x*c->tail_tip,b.y-1.0f};
    const nm_p2 lv=sub(left,tip),rv=sub(right,tip);
    const float ll=hypotf(lv.x,lv.y),rl=hypotf(rv.x,rv.y);
    const nm_p2 ul={lv.x/fmaxf(ll,0.0001f),lv.y/fmaxf(ll,0.0001f)};
    const nm_p2 ur={rv.x/fmaxf(rl,0.0001f),rv.y/fmaxf(rl,0.0001f)};
    const float cosine=nm_clamp(dot(ul,ur),-0.98f,0.98f);
    const float sin_half=sqrtf(0.5f*(1.0f-cosine));
    const float tan_half=sin_half/sqrtf(fmaxf(0.01f,0.5f*(1.0f+cosine)));
    const float desired=fminf(1.8f,fminf(tail*0.12f,r*0.24f));
    const float trim=fminf(desired/fmaxf(tan_half,0.01f),
                           fminf(ll,rl)*0.32f);
    const float tr=trim*tan_half;
    const nm_p2 rt={tip.x+ur.x*trim,tip.y+ur.y*trim};
    const nm_p2 lt={tip.x+ul.x*trim,tip.y+ul.y*trim};
    const nm_p2 bis={ul.x+ur.x,ul.y+ur.y};
    const float offset=tr/fmaxf(sin_half*hypotf(bis.x,bis.y),0.0001f);
    const nm_p2 tc={tip.x+bis.x*offset,tip.y+bis.y*offset};
    const nm_p2 tl={lx+r,top},top_right={rx-r,top};
    const nm_p2 rtop={rx,top+r},rb={rx,bottom-r};
    const nm_p2 br={rx-r,bottom},bl={lx+r,bottom};
    const nm_p2 lb={lx,bottom-r},ltop={lx,top+r};
    float ds=edge2(p,tl,top_right);
    ds=fminf(ds,edge2(p,rtop,rb));
    ds=fminf(ds,edge2(p,br,right));
    ds=fminf(ds,edge2(p,right,rt));
    ds=fminf(ds,edge2(p,lt,left));
    ds=fminf(ds,edge2(p,left,bl));
    ds=fminf(ds,edge2(p,lb,ltop));
    ds=fminf(ds,quarter_arc2(p,(nm_p2){rx-r,top+r},r,1,-1));
    ds=fminf(ds,quarter_arc2(p,(nm_p2){rx-r,bottom-r},r,1,1));
    ds=fminf(ds,quarter_arc2(p,(nm_p2){lx+r,bottom-r},r,-1,1));
    ds=fminf(ds,quarter_arc2(p,(nm_p2){lx+r,top+r},r,-1,-1));
    if(tr>0.0001f) ds=fminf(ds,arc2(p,rt,lt,tc,tr));
    else ds=fminf(ds,edge2(p,rt,lt));
    const nm_p2 body_p={p.x-0.5f*(lx+rx),p.y-0.5f*(top+bottom)};
    const nm_p2 body_b={0.5f*(rx-lx),0.5f*(bottom-top)};
    const int inside=box(body_p,body_b,r)<=0.0f ||
                     triangle(p,right,tip,left)<=0.0f;
    const int removed=tr>0.0001f && excised(p,rt,tip,lt,tc,tr);
    return sqrtf(ds)*(inside&&!removed?-1.0f:1.0f);
}

float nm_bubble_custom_distance(float x,float y,float half_width,float half_height,
                                float radius,const nm_bubble_controls *controls)
{
    if(!controls || !isfinite(x)||!isfinite(y)||!isfinite(half_width)||
       !isfinite(half_height)||!isfinite(radius)||
       half_width<=0.0f||half_height<=0.0f) return NAN;
    const float *v=&controls->left_inset;
    for(int i=0;i<8;++i) if(!isfinite(v[i])) return NAN;
    return bubble((nm_p2){x,y},(nm_p2){half_width,half_height},radius,controls);
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

    /* At slider 1 the two 45-degree tangent fillets meet with one common
     * center: a single full 90-degree circular arc from the top to right
     * rail (and its 180-degree counterpart). Old cap 1.20*cut left a
     * straight diagonal and looked under-rounded even at maximum.
     * Recover normalized Roundness from the host's source-pixel radius so
     * the entire 0..1 slider travel remains useful on every aspect ratio. */
    const float amount=nm_clamp(fmaxf(radius,0.0f)/
                                fminf(b.x,b.y),0.0f,1.0f);
    const float r=amount*cut*(1.0f+inv_root2);
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

/* D2.3: a deliberately asymmetric, continuous HUD panel silhouette.
 * Top/right bevels and a stepped lower-left notch clip the video itself,
 * not an ornament hovering outside a conventional rectangle. */
static int ray_crosses(nm_p2 p,nm_p2 a,nm_p2 b)
{
    return ((a.y>p.y)!=(b.y>p.y)) &&
           p.x<a.x+(p.y-a.y)*(b.x-a.x)/(b.y-a.y);
}
static float hud_panel(nm_p2 p,nm_p2 b,float detail)
{
    const float cut=fminf(fmaxf(2.0f,fminf(b.x,b.y)*detail*1.8f),
                          fminf(b.x,b.y)*0.45f);
    /* Bevel both transitions of the inset: the former two square stairs
     * looked like a generic UI corner, not a purposely authored HUD path. */
    const nm_p2 v[11]={
        {-b.x,-b.y},{b.x-cut,-b.y},{b.x,-b.y+cut},
        {b.x,b.y-cut*0.40f},{b.x-cut*0.35f,b.y},
        {-b.x+cut*1.80f,b.y},
        {-b.x+cut*1.65f,b.y-cut*0.20f},
        {-b.x+cut*1.13f,b.y-cut*0.20f},
        {-b.x+cut*0.95f,b.y-cut*0.38f},
        {-b.x+cut*0.95f,b.y-cut*0.70f},
        {-b.x,b.y-cut*0.70f}
    };
    float ds=edge2(p,v[10],v[0]);
    int inside=0;
    for(int i=0;i<11;++i){
        const nm_p2 a=v[i],next=v[(i+1)%11];
        ds=fminf(ds,edge2(p,a,next));
        inside^=ray_crosses(p,a,next);
    }
    return sqrtf(ds)*(inside?-1.0f:1.0f);
}

/* A superellipse is NOT a corner-radius variant of a rounded rectangle.
 * The implicit surface and its gradient yield approximately pixel-uniform
 * near-edge distance for arbitrary source aspect ratios. Center is bounded. */
static float squircle(nm_p2 p,nm_p2 b,float detail)
{
    const float exponent=4.0f+(detail-0.08f)/0.27f;
    const float ax=fminf(fabsf(p.x)/b.x,32.0f);
    const float ay=fminf(fabsf(p.y)/b.y,32.0f);
    const float fx=powf(ax,exponent),fy=powf(ay,exponent);
    const float gx=exponent*powf(ax,exponent-1.0f)/b.x;
    const float gy=exponent*powf(ay,exponent-1.0f)/b.y;
    float d=(fx+fy-1.0f)/fmaxf(hypotf(gx,gy),1.0f/fminf(b.x,b.y));
    /* The long near-horizontal and near-vertical sections benefit from
     * solving the implicit superellipse for the exact axis boundary.
     * This prevents flat-rail speckling when gradient linearization sees
     * subpixel changes; blend smoothly into the original corner distance. */
    if(ax<0.72f && ay>0.72f){
        const float rem=fmaxf(0.00001f,1.0f-fx);
        const float edge=b.y*powf(rem,1.0f/exponent);
        const float slope=(b.y/b.x)*powf(ax,exponent-1.0f)*
                          powf(rem,1.0f/exponent-1.0f);
        const float axis=(fabsf(p.y)-edge)/hypotf(1.0f,slope);
        const float t=nm_clamp((ax-0.58f)/0.14f,0.0f,1.0f);
        const float blend=t*t*(3.0f-2.0f*t);
        d=axis*(1.0f-blend)+d*blend;
    }else if(ay<0.72f && ax>0.72f){
        const float rem=fmaxf(0.00001f,1.0f-fy);
        const float edge=b.x*powf(rem,1.0f/exponent);
        const float slope=(b.x/b.y)*powf(ay,exponent-1.0f)*
                          powf(rem,1.0f/exponent-1.0f);
        const float axis=(fabsf(p.x)-edge)/hypotf(1.0f,slope);
        const float t=nm_clamp((ay-0.58f)/0.14f,0.0f,1.0f);
        const float blend=t*t*(3.0f-2.0f*t);
        d=axis*(1.0f-blend)+d*blend;
    }
    return fmaxf(d,-fminf(b.x,b.y));
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
    if(shape_id==NM_SHAPE_CHAT_BUBBLE) {
        const nm_bubble_controls legacy={0,0,0,0,-0.78f,-0.34f,-0.72f,detail};
        return bubble(p,b,radius,&legacy);
    }
    if(shape_id==NM_SHAPE_ANGLED_CARD) return card(p,b,detail,radius);
    if(shape_id==NM_SHAPE_HUD_PANEL) return hud_panel(p,b,detail);
    if(shape_id==NM_SHAPE_SQUIRCLE) return squircle(p,b,detail);
    return NAN;
}
