/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

/* Three independent phase accumulators: color gradient, breathing pulse and
 * traveling highlight. Each is a turn in [0,1), stored in double precision
 * and uploaded as a bounded float. No common one-hour time reset. */
typedef struct nm_motion {
    double color_turns;
    double pulse_turns;
    double flow_turns;
} nm_motion;

/* A static mode freezes all phases. Invalid deltas/speeds cannot poison them.
 * Bounded frame delta preserves the previous filter's pause/recovery policy. */
void nm_motion_tick(nm_motion *motion, float seconds, float speed, int animation_id);
