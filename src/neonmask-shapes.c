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

typedef struct {
    nm_p2 prev_t,next_t,center;
    float r,trim;
} nm_fillet;

static nm_fillet bubble_fillet(nm_p2 prev,nm_p2 v,nm_p2 next,float radius)
{
    const nm_p2 ap=sub(prev,v),an=sub(next,v);
    const float lp=fmaxf(hypotf(ap.x,ap.y),0.0001f);
    const float ln=fmaxf(hypotf(an.x,an.y),0.0001f);
    const nm_p2 up={ap.x/lp,ap.y/lp},un={an.x/ln,an.y/ln};
    const float co=nm_clamp(dot(up,un),-0.96f,0.96f);
    const float sh=sqrtf(fmaxf(0.0001f,0.5f*(1.0f-co)));
    const float ch=sqrtf(fmaxf(0.0001f,0.5f*(1.0f+co)));
    const float th=sh/ch;
    const float trim=fminf(fmaxf(radius,0.0f)/fmaxf(th,0.0001f),
                           0.42f*fminf(lp,ln));
    const float rr=trim*th;
    nm_p2 bis={up.x+un.x,up.y+un.y};
    const float bl=fmaxf(hypotf(bis.x,bis.y),0.0001f);
    bis.x/=bl;bis.y/=bl;
    return (nm_fillet){
        .prev_t={v.x+up.x*trim,v.y+up.y*trim},
        .next_t={v.x+un.x*trim,v.y+un.y*trim},
        .center={v.x+bis.x*(rr/sh),v.y+bis.y*(rr/sh)},
        .r=rr,.trim=trim
    };
}
static float quad_inside(nm_p2 p,nm_p2 tl,nm_p2 tr,nm_p2 br,nm_p2 bl)
{
    float side=cross(sub(tr,tl),sub(p,tl));
    side=fminf(side,cross(sub(br,tr),sub(p,tr)));
    side=fminf(side,cross(sub(bl,br),sub(p,br)));
    side=fminf(side,cross(sub(tl,bl),sub(p,bl)));
    return side;
}
static float tail_max_depth(nm_p2 base,nm_p2 n,nm_p2 b)
{
    float d=100000.0f;
    if(n.x>0.0001f)d=fminf(d,( b.x-base.x)/n.x);
    if(n.x<-0.0001f)d=fminf(d,(-b.x-base.x)/n.x);
    if(n.y>0.0001f)d=fminf(d,( b.y-base.y)/n.y);
    if(n.y<-0.0001f)d=fminf(d,(-b.y-base.y)/n.y);
    return fmaxf(0.0f,d-1.0f);
}

/* A freeform Bubble has ONE exposed perimeter. Four independently authored
 * convex corners define the body; the tail replaces a segment of the actual
 * BL->BR edge so no hidden seam can become neon. */
static float bubble(nm_p2 p,nm_p2 b,float radius,
                    const nm_bubble_controls *c)
{
    const nm_p2 tl={c->tl_x*b.x,c->tl_y*b.y};
    const nm_p2 tr={c->tr_x*b.x,c->tr_y*b.y};
    const nm_p2 br={c->br_x*b.x,c->br_y*b.y};
    const nm_p2 bl={c->bl_x*b.x,c->bl_y*b.y};
    const float body_r=fminf(fmaxf(radius,0.0f),0.22f*b.x);
    const nm_fillet ftl=bubble_fillet(bl,tl,tr,body_r);
    const nm_fillet ftr=bubble_fillet(tl,tr,br,body_r);
    const nm_fillet fbr=bubble_fillet(tr,br,bl,body_r);
    const nm_fillet fbl=bubble_fillet(br,bl,tl,body_r);

    const nm_p2 bottom=sub(br,bl);
    const float bottom_len=fmaxf(hypotf(bottom.x,bottom.y),0.0001f);
    const nm_p2 dir={bottom.x/bottom_len,bottom.y/bottom_len};
    const float rail_min=fbl.trim/bottom_len;
    const float rail_max=1.0f-fbr.trim/bottom_len;
    const float min_base=fminf(fmaxf(1.0f/bottom_len,0.04f),
                               0.30f*fmaxf(rail_max-rail_min,0.001f));
    const float st=nm_clamp(c->tail_start,rail_min,rail_max-min_base);
    const float et=nm_clamp(c->tail_end,st+min_base,rail_max);
    const nm_p2 left={bl.x+bottom.x*st,bl.y+bottom.y*st};
    const nm_p2 right={bl.x+bottom.x*et,bl.y+bottom.y*et};
    const float tt=nm_clamp(c->tail_tip_pos,0.0f,1.0f);
    const nm_p2 tip_base={bl.x+bottom.x*tt,bl.y+bottom.y*tt};
    const nm_p2 outward={-dir.y,dir.x};
    const float wanted=fmaxf(2.0f,b.y*c->tail_depth*1.8f-1.0f);
    const float depth=fminf(wanted,tail_max_depth(tip_base,outward,b));
    const nm_p2 tip={tip_base.x+outward.x*depth,
                     tip_base.y+outward.y*depth};

    const nm_p2 lv=sub(left,tip),rv=sub(right,tip);
    const float ll=fmaxf(hypotf(lv.x,lv.y),0.0001f);
    const float rl=fmaxf(hypotf(rv.x,rv.y),0.0001f);
    const nm_p2 ul={lv.x/ll,lv.y/ll},ur={rv.x/rl,rv.y/rl};
    const float co=nm_clamp(dot(ul,ur),-0.98f,0.98f);
    const float sh=sqrtf(fmaxf(0.0001f,0.5f*(1.0f-co)));
    const float th=sh/sqrtf(fmaxf(0.01f,0.5f*(1.0f+co)));
    const float desired=fminf(1.8f,fminf(depth*0.12f,body_r*0.24f));
    const float trim=fminf(desired/fmaxf(th,0.01f),fminf(ll,rl)*0.32f);
    const float tip_r=trim*th;
    const nm_p2 rt={tip.x+ur.x*trim,tip.y+ur.y*trim};
    const nm_p2 lt={tip.x+ul.x*trim,tip.y+ul.y*trim};
    nm_p2 bis={ul.x+ur.x,ul.y+ur.y};
    const float bis_len=fmaxf(hypotf(bis.x,bis.y),0.0001f);
    const float off=tip_r/fmaxf(sh*bis_len,0.0001f);
    const nm_p2 tc={tip.x+bis.x*off,tip.y+bis.y*off};

    float ds=edge2(p,ftl.next_t,ftr.prev_t);
    ds=fminf(ds,edge2(p,ftr.next_t,fbr.prev_t));
    ds=fminf(ds,edge2(p,fbr.next_t,right));
    ds=fminf(ds,edge2(p,right,rt));
    ds=fminf(ds,edge2(p,lt,left));
    ds=fminf(ds,edge2(p,left,fbl.prev_t));
    ds=fminf(ds,edge2(p,fbl.next_t,ftl.prev_t));
    if(ftl.r>0.0001f)ds=fminf(ds,arc2(p,ftl.prev_t,ftl.next_t,ftl.center,ftl.r));
    if(ftr.r>0.0001f)ds=fminf(ds,arc2(p,ftr.prev_t,ftr.next_t,ftr.center,ftr.r));
    if(fbr.r>0.0001f)ds=fminf(ds,arc2(p,fbr.prev_t,fbr.next_t,fbr.center,fbr.r));
    if(fbl.r>0.0001f)ds=fminf(ds,arc2(p,fbl.prev_t,fbl.next_t,fbl.center,fbl.r));
    if(tip_r>0.0001f)ds=fminf(ds,arc2(p,rt,lt,tc,tip_r));
    else ds=fminf(ds,edge2(p,rt,lt));

    const int body=quad_inside(p,tl,tr,br,bl)>=0.0f;
    const int tail=triangle(p,right,tip,left)<=0.0f;
    const int removed=(ftl.r>0.0001f&&excised(p,ftl.prev_t,tl,ftl.next_t,ftl.center,ftl.r))||
                      (ftr.r>0.0001f&&excised(p,ftr.prev_t,tr,ftr.next_t,ftr.center,ftr.r))||
                      (fbr.r>0.0001f&&excised(p,fbr.prev_t,br,fbr.next_t,fbr.center,fbr.r))||
                      (fbl.r>0.0001f&&excised(p,fbl.prev_t,bl,fbl.next_t,fbl.center,fbl.r))||
                      (tip_r>0.0001f&&excised(p,rt,tip,lt,tc,tip_r));
    return sqrtf(ds)*((body||tail)&&!removed?-1.0f:1.0f);
}

float nm_bubble_custom_distance(float x,float y,float half_width,float half_height,
                                float radius,const nm_bubble_controls *controls)
{
    if(!controls || !isfinite(x)||!isfinite(y)||!isfinite(half_width)||
       !isfinite(half_height)||!isfinite(radius)||
       half_width<=0.0f||half_height<=0.0f) return NAN;
    const float *v=&controls->tl_x;
    for(int i=0;i<12;++i) if(!isfinite(v[i])) return NAN;
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
/* D4B Tech HUD: opposite chamfers are part of source coverage.
 * The remaining top-right / bottom-left square corners are reserved for
 * external luminous L-brackets in the shader ornament layer. */
static float tech_hud(nm_p2 p,nm_p2 b,float detail)
{
    const float cut=fminf(fmaxf(2.0f,fminf(b.x,b.y)*detail*1.10f),
                          fminf(b.x,b.y)*0.20f);
    const nm_p2 v[6]={
        {-b.x+cut,-b.y}, {b.x,-b.y}, {b.x,b.y-cut},
        {b.x-cut,b.y}, {-b.x,b.y}, {-b.x,-b.y+cut}
    };
    float side=cross(sub(v[1],v[0]),sub(p,v[0]));
    float ds=edge2(p,v[0],v[1]);
    for(int i=1;i<6;++i){
        const nm_p2 a=v[i],n=v[(i+1)%6];
        side=fminf(side,cross(sub(n,a),sub(p,a)));
        ds=fminf(ds,edge2(p,a,n));
    }
    return sqrtf(ds)*(side>=0.0f?-1.0f:1.0f);
}

/* D4C Game UI: the source itself uses a restrained eight-edge gaming
 * panel. Outer L brackets and the inner rail are decoration only. */
static float game_ui(nm_p2 p,nm_p2 b,float detail)
{
    (void)detail;
    return box(p,b,0.0f);
}

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
        const float bottom=1.0f-1.8f*detail;
        const nm_bubble_controls legacy={
            -1.0f,-1.0f, 1.0f,-1.0f,
             1.0f,bottom,-1.0f,bottom,
             0.11f,0.33f,0.14f,detail
        };
        return bubble(p,b,radius,&legacy);
    }
    if(shape_id==NM_SHAPE_ANGLED_CARD) return card(p,b,detail,radius);
    if(shape_id==NM_SHAPE_HUD_PANEL) return hud_panel(p,b,detail);
    if(shape_id==NM_SHAPE_TECH_HUD) return tech_hud(p,b,detail);
    if(shape_id==NM_SHAPE_GAME_UI) return game_ui(p,b,detail);
    if(shape_id==NM_SHAPE_SQUIRCLE) return squircle(p,b,detail);
    return NAN;
}
