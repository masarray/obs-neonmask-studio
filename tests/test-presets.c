/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-presets.h"
#include <stdio.h>

static int failures;
static void check(const char *name, int condition)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

int main(void)
{
    nm_preset p = {0};
    check("custom preset invalid", !nm_get_preset(0, &p));
    check("negative preset invalid", !nm_get_preset(-1, &p));
    check("unknown preset invalid", !nm_get_preset(5, &p));
    check("null output invalid", !nm_get_preset(1, NULL));
    for (int id = 1; id <= 4; ++id) {
        check("known preset", nm_get_preset(id, &p));
        check("valid shape", p.shape >= NM_SHAPE_ROUNDED && p.shape <= NM_SHAPE_DIAMOND);
        check("valid animation", p.animation >= NM_ANIM_STATIC && p.animation <= NM_ANIM_FLOW);
        check("nonzero colors", p.primary != p.secondary && p.primary != 0 && p.secondary != 0);
        check("scale range", p.scale >= 0.30 && p.scale <= 0.96);
        check("roundness range", p.roundness >= 0.0 && p.roundness <= 1.0);
        check("width range", p.border_width >= 0.5 && p.border_width <= 32.0);
        check("glow range", p.glow_strength >= 0.0 && p.glow_strength <= 1.0);
        check("segments range", p.segments >= 0 && p.segments <= 48);
    }
    nm_get_preset(2, &p);
    check("reactor ring shape", p.shape == NM_SHAPE_CIRCLE);
    check("reactor segmented", p.segments == 10);
    nm_get_preset(3, &p);
    check("emerald hex", p.shape == NM_SHAPE_HEXAGON);
    nm_get_preset(4, &p);
    check("ember static", p.animation == NM_ANIM_STATIC);
    if (failures) return 1;
    puts("PASS: four preset configurations and input validation");
    return 0;
}
