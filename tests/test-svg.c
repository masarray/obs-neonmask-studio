/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-svg.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int failures=0;
static void check(const char *name,bool pass)
{
    if(!pass){fprintf(stderr,"FAIL: %s\n",name);++failures;}
}
static bool parse(const char *svg,nm_svg_shape *out)
{
    char reason[128]={0};
    return nm_svg_parse(svg,strlen(svg),out,reason,sizeof(reason));
}
int main(int argc,char **argv)
{
    nm_svg_shape shape;
    const char *square="<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\"><path d=\"M10 10 H90 V90 H10 Z\"/></svg>";
    check("simple square",parse(square,&shape));
    check("four edges",shape.edge_count==4);
    float *sdf=malloc(NM_SVG_SDF_SIZE*NM_SVG_SDF_SIZE*sizeof(float));
    if(!sdf) return 2;
    check("raster",nm_svg_raster_sdf(&shape,24,24,sdf,NM_SVG_SDF_SIZE));
    const int n=NM_SVG_SDF_SIZE;
    check("center opaque negative",sdf[(n/2)*n+n/2]<-10);
    check("outside corner positive",sdf[2*n+2]>2);
    check("axis edge nearly zero",fabsf(sdf[(n/2)*n+26])<1.5f);
    const char *curve="<svg viewBox=\"0 0 100 100\"><path d=\"M 10 50 C 10 10 90 10 90 50 Q 90 90 10 50 Z\" fill-rule=\"nonzero\"/></svg>";
    check("bounded cubic quadratic flatten",parse(curve,&shape) && shape.edge_count>=40);
    check("smooth path finite",nm_svg_raster_sdf(&shape,40,20,sdf,n));
    for(int i=0;i<n*n;i++) if(!isfinite(sdf[i])){check("all SDF values finite",false);break;}
    const char *hole="<svg viewBox=\"0 0 100 100\"><path d=\"M5 5 H95 V95 H5 Z M30 30 H70 V70 H30 Z\" fill-rule=\"evenodd\"/></svg>";
    check("evenodd path",parse(hole,&shape) && shape.evenodd);
    check("evenodd raster",nm_svg_raster_sdf(&shape,24,24,sdf,n));
    check("hole transparent",sdf[(n/2)*n+n/2]>2);
    check("rim still filled",sdf[(n/2)*n+28]<-2);
    const char *bad_xml[]={
        "<svg viewBox=\"0 0 100 100\"><script>alert(1)</script></svg>",
        "<!DOCTYPE svg [<!ENTITY x SYSTEM \"file:///etc/passwd\">]><svg viewBox=\"0 0 1 1\"></svg>",
        "<svg viewBox=\"0 0 100 100\"><image href=\"https://example.com/p.png\"/></svg>",
        "<svg viewBox=\"0 0 100 100\"><path d=\"M0 0 L100 0 L100 100 Z\" onload=\"bad()\"/></svg>",
        "<svg viewBox=\"0 0 100 100\"><path d=\"M0 0 A10 10 0 0 1 20 20 Z\"/></svg>",
        "<svg viewBox=\"0 0 100 100\"><path d=\"M0 0 L100 0 L100 100\"/></svg>",
        "<svg viewBox=\"0 0 -100 100\"><path d=\"M0 0L100 0L100 100Z\"/></svg>",
        "<svg viewBox=\"0 0 100 100\"><path d=\"M0 0L100 0L100 100Z\" transform=\"scale(2)\"/></svg>",
        "<svg viewBox=\"0 0 100 100\"><path d=\"M0 0L100 0L100 100Z\" fill=\"url(https://x)\"/></svg>",
        "<svg viewBox=\"0 0 100 100\"><path d=\"M0 0Lnan 0L100 100Z\"/></svg>"
    };
    for(size_t i=0;i<sizeof(bad_xml)/sizeof(bad_xml[0]);i++){
        check("hostile/unsupported SVG rejected",!parse(bad_xml[i],&shape));
        check("failure clears stale geometry",shape.edge_count==0);
    }
    char oversized[NM_SVG_MAX_BYTES+2];
    memset(oversized,'x',sizeof(oversized));
    check("oversized rejected",!nm_svg_parse(oversized,sizeof(oversized),
                                           &shape,NULL,0));
    check("null/invalid raster rejected",
          !nm_svg_raster_sdf(NULL,24,24,sdf,n) &&
          !nm_svg_raster_sdf(&shape,-1,24,sdf,n) &&
          !nm_svg_raster_sdf(&shape,24,24,sdf,128));
    if(argc>=2){
        char why[128];
        check("local file loads",nm_svg_read_local(argv[1],&shape,why,sizeof(why)));
        check("local file raster",nm_svg_raster_sdf(&shape,30,20,sdf,n));
        check("invalid path fails closed",!nm_svg_read_local("/nonexistent/neonmask.svg",
                                                              &shape,why,sizeof(why)));
        check("invalid path clears geometry",shape.edge_count==0);
        for(int i=2;i<argc;i++){
            check("original authored reference SVG imports",
                  nm_svg_read_local(argv[i],&shape,why,sizeof(why)));
            check("original SVG raster",nm_svg_raster_sdf(&shape,40,24,sdf,n));
        }
    }
    free(sdf);
    if(failures)return 1;
    puts("PASS: bounded SVG parse, curves, winding/hole, SDF, hostile/invalid fail closed");
    return 0;
}
