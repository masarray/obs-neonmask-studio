/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-motion.h"
#include "neonmask-presets.h"
#include <math.h>
#include <stdio.h>

#define TAU_D 6.283185307179586476925286766559
static int failures;

static void check(const char *name, int ok)
{
    if (!ok) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

static void near(const char *name, double actual, double expected, double eps)
{
    if (!(actual >= 0.0 && actual < 1.0) || fabs(actual - expected) > eps) {
        fprintf(stderr, "FAIL: %s: actual=%.16f expected=%.16f\n", name, actual, expected);
        ++failures;
    }
}

static double forward_turns(double before, double after)
{
    double delta = after - before;
    if (delta < 0.0) delta += 1.0;
    return delta;
}

int main(void)
{
    nm_motion m = {0};
    nm_motion_tick(NULL, 1.0f, 1.0f, NM_ANIM_FLOW);
    nm_motion_tick(&m, NAN, 1.0f, NM_ANIM_FLOW);
    nm_motion_tick(&m, -1.0f, 1.0f, NM_ANIM_FLOW);
    nm_motion_tick(&m, 1.0f, NAN, NM_ANIM_FLOW);
    check("bad inputs preserve motion", m.color_turns == 0.0 && m.pulse_turns == 0.0 && m.flow_turns == 0.0);

    nm_motion_tick(&m, 0.25f, 1.0f, NM_ANIM_FLOW);
    near("color frequency", m.color_turns, 0.25 * 0.15 / TAU_D, 1e-12);
    near("pulse frequency", m.pulse_turns, 0.25, 1e-12);
    near("flow frequency", m.flow_turns, 0.05, 1e-12);

    const nm_motion frozen = m;
    nm_motion_tick(&m, 0.5f, 2.0f, NM_ANIM_STATIC);
    check("static freezes ALL phases", m.color_turns == frozen.color_turns &&
          m.pulse_turns == frozen.pulse_turns && m.flow_turns == frozen.flow_turns);
    nm_motion_tick(&m, 0.5f, 0.0f, NM_ANIM_FLOW);
    check("zero speed freezes ALL phases", m.color_turns == frozen.color_turns &&
          m.pulse_turns == frozen.pulse_turns && m.flow_turns == frozen.flow_turns);

    nm_motion_tick(&m, 0.25f, 2.0f, NM_ANIM_FLOW);
    near("speed change no phase reset", m.color_turns, frozen.color_turns + 0.5 * 0.15 / TAU_D, 1e-12);
    near("pulse wraps smoothly", m.pulse_turns, 0.75, 1e-12);
    near("flow continuous", m.flow_turns, 0.15, 1e-12);

    nm_motion long_run = {0};
    const float dt = 1.0f / 60.0f;
    const int iterations = 8 * 60 * 60 * 60;
    nm_motion prev = long_run;
    for (int i = 0; i < iterations; ++i) {
        prev = long_run;
        nm_motion_tick(&long_run, dt, 1.0f, NM_ANIM_FLOW);
        check("bounded phases", long_run.color_turns >= 0.0 && long_run.color_turns < 1.0 &&
              long_run.pulse_turns >= 0.0 && long_run.pulse_turns < 1.0 &&
              long_run.flow_turns >= 0.0 && long_run.flow_turns < 1.0);
        if (i == 60 * 60 * 60 - 1 || i == iterations - 1) {
            near("color seam continuous", forward_turns(prev.color_turns, long_run.color_turns),
                 (double)dt * 0.15 / TAU_D, 2e-11);
            near("pulse seam continuous", forward_turns(prev.pulse_turns, long_run.pulse_turns),
                 (double)dt, 2e-11);
            near("flow seam continuous", forward_turns(prev.flow_turns, long_run.flow_turns),
                 (double)dt * 0.2, 2e-11);
        }
    }
    const double total = (double)dt * iterations;
    near("eight-hour color position", long_run.color_turns, fmod(total * 0.15 / TAU_D, 1.0), 1e-6);
    near("eight-hour pulse position", long_run.pulse_turns, fmod(total, 1.0), 1e-6);
    near("eight-hour flow position", long_run.flow_turns, fmod(total * 0.2, 1.0), 1e-6);

    if (failures) return 1;
    puts("PASS: static/zero-speed, speed changes and simulated 8-hour phase continuity");
    return 0;
}
