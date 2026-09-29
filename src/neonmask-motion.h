/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

/* Four independent phase accumulators: dual-color drift, breathing pulse,
 * traveling highlight and D4D rainbow hue. Each is a turn in [0,1), stored in double precision
 * and uploaded as a bounded float. No common one-hour time reset. */
typedef struct nm_motion {
    double color_turns;
    double pulse_turns;
    double flow_turns;
    double rainbow_turns;
} nm_motion;

/* A static mode freezes all phases. Invalid deltas/speeds cannot poison them.
 * Bounded frame delta preserves the previous filter's pause/recovery policy. */
void nm_motion_tick(nm_motion *motion, float seconds, float speed, int animation_id);

/* Rainbow has an independent bounded phase/rate. Static animation remains
 * bit-stable over time; Dual/Solid modes never advance unused rainbow state. */
void nm_rainbow_tick(nm_motion *motion, float seconds, float speed,
                     int animation_id, int color_mode);
