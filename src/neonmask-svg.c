/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-svg.h"
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool bad(char *why,size_t n,const char *message)
{
    if(why && n) snprintf(why,n,"%s",message);
    return false;
}
static void spaces(const char **p)
{
    while(isspace((unsigned char)**p) || **p==',') ++*p;
}
static bool number(const char **p,float *out)
{
    spaces(p);
    char *end=NULL;
    if(!**p) return false;
    float v=strtof(*p,&end);
    if(end==*p || !isfinite(v) || fabsf(v)>100000.0f) return false;
    *p=end; *out=v;
    return true;
}
static bool pair(const char **p,float *x,float *y)
{
    return number(p,x)&&number(p,y);
}
static bool edge(nm_svg_shape *s,float ax,float ay,float bx,float by)
{
    if(fabsf(ax-bx)+fabsf(ay-by)<0.000001f) return true;
    if(s->edge_count>=NM_SVG_MAX_EDGES) return false;
    s->edges[s->edge_count++]=(nm_svg_edge){ax,ay,bx,by};
    return true;
}
static bool point(nm_svg_shape *s,float *x,float *y,float nx,float ny)
{
    if(!edge(s,*x,*y,nx,ny)) return false;
    *x=nx; *y=ny;
    return true;
}
static bool path_data(nm_svg_shape *s,const char *p)
{
    char cmd=0;
    float x=0,y=0,sx=0,sy=0;
    bool open=false;
    unsigned contour_edges=0;
    while(1){
        spaces(&p);
        if(!*p) break;
        if(isalpha((unsigned char)*p)){
            cmd=*p++;
            if(!strchr("MmLlHhVvQqCcZz",cmd)) return false;
            if(cmd=='Z'||cmd=='z'){
                if(!open || contour_edges<2 ||
                   !point(s,&x,&y,sx,sy)) return false;
                open=false; cmd=0;
                continue;
            }
        }
        if(!cmd) return false;
        if(cmd=='M'||cmd=='m'){
            float nx,ny;
            if(!pair(&p,&nx,&ny) || open) return false;
            if(cmd=='m'){nx+=x;ny+=y;}
            x=sx=nx; y=sy=ny; open=true;
            contour_edges=0;
            cmd=cmd=='m'?'l':'L';
            continue;
        }
        if(!open) return false;
        const bool relative=islower((unsigned char)cmd)!=0;
        if(cmd=='L'||cmd=='l'){
            float nx,ny;
            if(!pair(&p,&nx,&ny)) return false;
            if(relative){nx+=x;ny+=y;}
            if(!point(s,&x,&y,nx,ny)) return false;
        }else if(cmd=='H'||cmd=='h'){
            float nx;
            if(!number(&p,&nx)) return false;
            if(relative) nx+=x;
            if(!point(s,&x,&y,nx,y)) return false;
        }else if(cmd=='V'||cmd=='v'){
            float ny;
            if(!number(&p,&ny)) return false;
            if(relative) ny+=y;
            if(!point(s,&x,&y,x,ny)) return false;
        }else if(cmd=='Q'||cmd=='q'){
            float a,bb,nx,ny;
            if(!pair(&p,&a,&bb)||!pair(&p,&nx,&ny)) return false;
            if(relative){a+=x;bb+=y;nx+=x;ny+=y;}
            const float px=x,py=y;
            /* Fixed bounded flattening: 16 edges / quadratic command. */
            for(int i=1;i<=16;++i){
                const float t=(float)i/16.0f,u=1.0f-t;
                const float xx=u*u*px+2*u*t*a+t*t*nx;
                const float yy=u*u*py+2*u*t*bb+t*t*ny;
                if(!point(s,&x,&y,xx,yy)) return false;
            }
        }else if(cmd=='C'||cmd=='c'){
            float ax,ay,bx,by,nx,ny;
            if(!pair(&p,&ax,&ay)||!pair(&p,&bx,&by)||
               !pair(&p,&nx,&ny)) return false;
            if(relative){ax+=x;ay+=y;bx+=x;by+=y;nx+=x;ny+=y;}
            const float px=x,py=y;
            /* 24 straight edges / cubic. No unbounded recursive tessellation. */
            for(int i=1;i<=24;++i){
                const float t=(float)i/24.0f,u=1.0f-t;
                const float xx=u*u*u*px+3*u*u*t*ax+3*u*t*t*bx+t*t*t*nx;
                const float yy=u*u*u*py+3*u*u*t*ay+3*u*t*t*by+t*t*t*ny;
                if(!point(s,&x,&y,xx,yy)) return false;
            }
        }else return false;
        ++contour_edges;
    }
    return !open && s->edge_count>=3;
}
static bool copy_value(char *dest,size_t cap,const char *a,size_t n)
{
    if(n>=cap || memchr(a,'&',n) || memchr(a,'<',n) ||
       memchr(a,'>',n)) return false;
    memcpy(dest,a,n); dest[n]=0; return true;
}
static bool attr_tag(const char **cursor,bool root,nm_svg_shape *shape,
                     char *view,size_t vcap,char *data,size_t dcap,
                     bool *selfclose)
{
    const char *p=*cursor;
    *selfclose=false;
    while(1){
        while(isspace((unsigned char)*p)) ++p;
        if(*p=='/' && p[1]=='>'){*selfclose=true;*cursor=p+2;return true;}
        if(*p=='>'){*cursor=p+1;return true;}
        if(!isalpha((unsigned char)*p)) return false;
        char key[32];size_t k=0;
        while(isalnum((unsigned char)*p)||*p=='-'||*p==':'){
            if(k+1>=sizeof(key)) return false;
            key[k++]=*p++;
        }
        key[k]=0;
        while(isspace((unsigned char)*p)) ++p;
        if(*p++!='=') return false;
        while(isspace((unsigned char)*p)) ++p;
        const char quote=*p++;
        if(quote!='"' && quote!='\'') return false;
        const char *a=p;
        while(*p && *p!=quote){
            if(*p=='<'||*p=='>'||*p=='&') return false;
            ++p;
        }
        if(!*p) return false;
        const size_t n=(size_t)(p-a);
        ++p;
        if(root && !strcmp(key,"viewBox")){
            if(view[0] || !copy_value(view,vcap,a,n)) return false;
        }else if(root && !strcmp(key,"xmlns")){
            static const char uri[]="http://www.w3.org/2000/svg";
            if(n!=sizeof(uri)-1 || strncmp(a,uri,n)) return false;
        }else if(root && (!strcmp(key,"width") || !strcmp(key,"height") ||
                           !strcmp(key,"version"))){
            /* Ignored presentation metadata; viewBox is authoritative. */
            if(n>32) return false;
        }else if(!root && !strcmp(key,"d")){
            if(data[0] || !copy_value(data,dcap,a,n)) return false;
        }else if(!root && !strcmp(key,"fill-rule")){
            if(n==7 && !strncmp(a,"evenodd",7)) shape->evenodd=true;
            else if(n==7 && !strncmp(a,"nonzero",7)) shape->evenodd=false;
            else return false;
        }else if(!root && !strcmp(key,"fill")){
            /* Only solid filled geometry, no url(...)/external paint. */
            if(n<2 || n>9 || *a!='#') return false;
            for(size_t i=1;i<n;i++)
                if(!isxdigit((unsigned char)a[i])) return false;
        }else if(!strcmp(key,"id")){
            if(n>64) return false;
            for(size_t i=0;i<n;i++)
                if(!isalnum((unsigned char)a[i]) && a[i]!='-' && a[i]!='_')
                    return false;
        }else return false; /* No style, script, href, transform, event attrs. */
    }
}
bool nm_svg_parse(const char *bytes,size_t length,nm_svg_shape *out,
                  char *why,size_t why_size)
{
    if(!out) return bad(why,why_size,"null destination");
    memset(out,0,sizeof(*out));
    if(!bytes || !length || length>NM_SVG_MAX_BYTES ||
       memchr(bytes,0,length)) return bad(why,why_size,"invalid SVG length");
    char *xml=malloc(length+1);
    if(!xml) return bad(why,why_size,"out of memory");
    memcpy(xml,bytes,length);xml[length]=0;
    const char *p=xml;
    nm_svg_shape parsed={0};
    char view[128]={0},path[8192]={0};
    bool selfclose=false,ok=false;
    while(isspace((unsigned char)*p)) ++p;
    if(!strncmp(p,"<?xml",5)){
        const char *end=strstr(p,"?>");
        if(!end){bad(why,why_size,"invalid XML declaration");goto done;}
        p=end+2;
        while(isspace((unsigned char)*p)) ++p;
    }
    if(strncmp(p,"<svg",4) || (!isspace((unsigned char)p[4]) && p[4]!='>')){
        bad(why,why_size,"only root svg is allowed");goto done;
    }
    p+=4;
    if(!attr_tag(&p,true,&parsed,view,sizeof(view),path,sizeof(path),&selfclose)||
       selfclose || !view[0]){
        bad(why,why_size,"invalid root/viewBox attribute");goto done;
    }
    const char *vp=view;
    if(!number(&vp,&parsed.min_x)||!number(&vp,&parsed.min_y)||
       !number(&vp,&parsed.width)||!number(&vp,&parsed.height)){
        bad(why,why_size,"viewBox must contain four finite numbers");goto done;
    }
    spaces(&vp);
    if(*vp || parsed.width<=0.0f || parsed.height<=0.0f ||
       parsed.width>10000.0f || parsed.height>10000.0f){
        bad(why,why_size,"viewBox out of bounds");goto done;
    }
    unsigned paths=0;
    while(1){
        while(isspace((unsigned char)*p)) ++p;
        if(!strncmp(p,"</svg>",6)){
            p+=6;
            while(isspace((unsigned char)*p)) ++p;
            if(*p || !paths){bad(why,why_size,"missing path/trailing data");goto done;}
            *out=parsed;ok=true;goto done;
        }
        if(strncmp(p,"<path",5) || (!isspace((unsigned char)p[5]) &&
                                   p[5]!='/' && p[5]!='>') || paths>=8){
            bad(why,why_size,"only eight self-closing path tags are supported");
            goto done;
        }
        p+=5;path[0]=0;
        if(!attr_tag(&p,false,&parsed,view,sizeof(view),path,sizeof(path),&selfclose)||
           !selfclose || !path[0] || !path_data(&parsed,path)){
            bad(why,why_size,"invalid/unsupported SVG path");goto done;
        }
        ++paths;
    }
done:
    free(xml);
    if(!ok) memset(out,0,sizeof(*out));
    return ok;
}
bool nm_svg_read_local(const char *path,nm_svg_shape *out,
                       char *why,size_t why_size)
{
    if(!out) return bad(why,why_size,"null destination");
    memset(out,0,sizeof(*out));
    if(!path || !*path || strlen(path)>=1024)
        return bad(why,why_size,"select a local SVG path");
    FILE *f=fopen(path,"rb");
    if(!f) return bad(why,why_size,"cannot open local SVG");
    char buffer[NM_SVG_MAX_BYTES+1];
    size_t size=fread(buffer,1,sizeof(buffer),f);
    const bool io_error=ferror(f)!=0;
    fclose(f);
    if(io_error || !size || size>NM_SVG_MAX_BYTES)
        return bad(why,why_size,"SVG must be 1..65536 bytes");
    return nm_svg_parse(buffer,size,out,why,why_size);
}
static float segment2(float x,float y,nm_svg_edge e)
{
    const float ax=e.bx-e.ax,ay=e.by-e.ay;
    const float px=x-e.ax,py=y-e.ay;
    float t=(px*ax+py*ay)/fmaxf(ax*ax+ay*ay,0.000001f);
    t=fmaxf(0.0f,fminf(t,1.0f));
    const float dx=px-t*ax,dy=py-t*ay;
    return dx*dx+dy*dy;
}
bool nm_svg_raster_sdf(const nm_svg_shape *s,float bx,float by,
                       float *out,unsigned res)
{
    if(!s || !out || !s->edge_count || s->edge_count>NM_SVG_MAX_EDGES ||
       !isfinite(bx)||!isfinite(by)||bx<=0||by<=0 ||
       !isfinite(s->width)||!isfinite(s->height)||
       s->width<=0||s->height<=0 || res!=NM_SVG_SDF_SIZE)
        return false;
    const float fit=fminf(2.0f*bx/s->width,2.0f*by/s->height);
    if(!isfinite(fit)||fit<=0) return false;
    const float centerx=s->min_x+0.5f*s->width;
    const float centery=s->min_y+0.5f*s->height;
    for(unsigned y=0;y<res;++y){
        const float py=((float)y+0.5f)/(float)res*2.0f*by-by;
        const float vy=py/fit+centery;
        for(unsigned x=0;x<res;++x){
            const float px=((float)x+0.5f)/(float)res*2.0f*bx-bx;
            const float vx=px/fit+centerx;
            float ds=FLT_MAX;
            int winding=0;
            bool parity=false;
            for(size_t i=0;i<s->edge_count;++i){
                const nm_svg_edge e=s->edges[i];
                ds=fminf(ds,segment2(vx,vy,e));
                const float cross=(e.bx-e.ax)*(vy-e.ay)-
                                  (e.by-e.ay)*(vx-e.ax);
                if(e.ay<=vy && e.by>vy && cross>0) ++winding;
                else if(e.ay>vy && e.by<=vy && cross<0) --winding;
                if((e.ay>vy)!=(e.by>vy)){
                    const float ix=e.ax+(vy-e.ay)*(e.bx-e.ax)/(e.by-e.ay);
                    if(vx<ix) parity=!parity;
                }
            }
            const bool inside=s->evenodd?parity:(winding!=0);
            const float distance=sqrtf(ds)*fit;
            out[(size_t)y*res+x]=(inside?-1.0f:1.0f)*
                                  fminf(distance,4096.0f);
        }
    }
    return true;
}
