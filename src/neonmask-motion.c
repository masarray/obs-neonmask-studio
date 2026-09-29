/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-motion.h"
#include "neonmask-math.h"
#include "neonmask-presets.h"
#include <math.h>

#define NM_TAU_D 6.283185307179586476925286766559

void nm_motion_tick(nm_motion *motion, float seconds, float speed, int animation_id)
{
    if (!motion || animation_id == NM_ANIM_STATIC)
        return;

    const double dt = (double)nm_clamp(seconds, 0.0f, 1.0f);
    const double rate = (double)nm_clamp(speed, 0.0f, 5.0f);
    if (dt == 0.0 || rate == 0.0)
        return;

    const double step = dt * rate;
    /* Reproduces the existing visual frequencies without making GPU shader
     * arithmetic depend on ever-increasing floating-point wall-clock time. */
    motion->color_turns = fmod(motion->color_turns + step * (0.15 / NM_TAU_D), 1.0);
    motion->pulse_turns = fmod(motion->pulse_turns + step, 1.0);
    motion->flow_turns = fmod(motion->flow_turns + step * 0.20, 1.0);
}

void nm_rainbow_tick(nm_motion *motion, float seconds, float speed,
                     int animation_id, int color_mode)
{
    if (!motion || color_mode != NM_COLOR_RAINBOW ||
        animation_id == NM_ANIM_STATIC)
        return;

    const double dt = (double)nm_clamp(seconds, 0.0f, 1.0f);
    const double rate = (double)nm_clamp(speed, 0.0f, 5.0f);
    if (dt == 0.0 || rate == 0.0)
        return;

    /* 0.08 turns/s at speed=1 is intentionally slower than the local Flow
     * hotspot: the spectrum drifts smoothly instead of strobing the frame. */
    motion->rainbow_turns =
        fmod(motion->rainbow_turns + dt * rate * 0.08, 1.0);
}
