/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-art.h"
#include <math.h>
#include <stdio.h>
static int failed;
static void test(const char *name, int ok)
{
    if (!ok) { fprintf(stderr, "FAIL: %s\n", name); ++failed; }
}
static void near(const char *name, float a, float b)
{
    test(name, fabsf(a-b) < 0.0005f);
}
int main(void)
{
    /* 40x28 rounded rectangle, radius 4: verified tangent sequence. */
    const float bx=20.0f, by=14.0f, r=4.0f, sx=bx-r, sy=by-r;
    const float pi=3.14159265358979323846f, arc=pi*r/2.0f;
    const float h=2*sx, v=2*sy, total=2*h+2*v+4*arc;
    near("top left tangent",nm_rounded_contour_turn(-sx,-by,bx,by,r),0.0f);
    near("top right tangent",nm_rounded_contour_turn(sx,-by,bx,by,r),h/total);
    near("right top tangent",nm_rounded_contour_turn(bx,-sy,bx,by,r),(h+arc)/total);
    near("right bottom tangent",nm_rounded_contour_turn(bx,sy,bx,by,r),(h+arc+v)/total);
    near("bottom right tangent",nm_rounded_contour_turn(sx,by,bx,by,r),(h+2*arc+v)/total);
    near("bottom left tangent",nm_rounded_contour_turn(-sx,by,bx,by,r),(2*h+2*arc+v)/total);
    near("left bottom tangent",nm_rounded_contour_turn(-bx,sy,bx,by,r),(2*h+3*arc+v)/total);
    near("left top tangent",nm_rounded_contour_turn(-bx,-sy,bx,by,r),(2*h+3*arc+2*v)/total);
    for(int i=-80;i<=80;i++) for(int j=-60;j<=60;j++){
        const float t=nm_rounded_contour_turn(i*.5f,j*.5f,bx,by,r);
        test("bounded turn",isfinite(t)&&t>=0.0f&&t<1.0f);
    }
    test("valid cyber",nm_art_recipe_valid(1,3.0f,0.82f));
    test("legacy none",nm_art_recipe_valid(0,2.0f,0.0f));
    test("Reactor allowed",nm_art_recipe_valid(2,3.0f,.8f));
    test("HUD allowed",nm_art_recipe_valid(3,3.0f,.8f));
    test("Streamer allowed",nm_art_recipe_valid(4,3.0f,.8f));
    test("Game UI allowed",nm_art_recipe_valid(5,3.0f,.8f));
    test("reject mode",!nm_art_recipe_valid(9,3.0f,1.0f));
    test("reject nan",!nm_art_recipe_valid(1,NAN,.7f));
    test("reject inf",!nm_art_recipe_valid(1,3.0f,INFINITY));
    test("reject bad gap",!nm_art_recipe_valid(1,20.0f,.5f));
    if(failed)return 1;
    puts("PASS: rounded contour arc-length reference and bounded art recipe");
    return 0;
}
