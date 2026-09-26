# Delivery roadmap and evidence ledger

Snapshot: 2026-09-26. Main inspected at `039169cf1b30c9e20a17b9393e0b5de5c708c008`.
This file owns delivery status; [PRODUCT_SPEC.md](PRODUCT_SPEC.md) owns requirements.
Update the ledger in the same change that advances implementation or verification.

## Baseline: what exists versus what is proven

| Area | Observed implementation | Evidence gap |
| --- | --- | --- |
| Native filter | C11/libobs callbacks in src/neonmask-filter.c | Frontend/lifecycle tests pending |
| Geometry | Five shapes, approximate distances, source-size coordinates | Precision and corner/ellipse visual validation pending |
| Neon | Dual color, analytic glow, pulse/flow, four styles, four presets | Concept-board parity not established |
| State | Clamping and preset table; editable properties | Versioned schema, complete migration and atomic handoff design not implemented |
| Safety | Shader load/uniform checks; bypass; effect cleanup | Fault injection, resource accounting and soak pending |
| CI | Headless/platform builds, OpenGL compile smoke, Windows preview workflow | Workflow existence is not runtime certification |
| Assets/workers | No custom image/SVG/live-mask provider | Bounded service is a future requirement |
| Performance | Procedural one-effect design | No hardware budget evidence recorded |
| Padding | Existing source canvas only | Large glow can clip; safe-fit/expanded-output validation pending |

At inspection, [PR #6](https://github.com/masarray/obs-neonmask-studio/pull/6) is open for D3D11 shader compilation smoke; its Windows job is in progress. Re-check current state before working on that slice. It does not establish pixel correctness even after passing. Avoid duplicating this work.

## Milestones, in dependency order

Work in small vertical slices: implementation + relevant automated test + runnable artifact + evidence. These milestones refine the earlier README phase list; old “Phase 2” wording is historical.

| Milestone | Deliverables | Exit / next |
| --- | --- | --- |
| M0 — reproducible baseline | Reconcile PR #6, record exact preview artifact, select reference GPUs; document actual Windows loading and captures | G1–G3 and initial G4/G5 evidence; list failures before adding features |
| M1 — correctness and lifecycle | Canonical config validation/migrations; resource ownership; fault handling; alpha/color fixtures; long-session motion fix if tests expose it | G4/G5 pass for current shapes, baseline performance and memory report |
| M2 — polished procedural core | Safe-fit envelope, precision shapes including triangle/polygon, transforms, perimeter-based dashes/flow, compact grouped controls | Existing and new shapes pass visual matrix; four refined presets within budget |
| M3 — signature visual collection | Capped layers, corner ornaments, ring, streamer/chat frame, optional electric; reduced motion; preset serialization | Reference-family captures, import validation, max-preset benchmarks |
| M4 — broader masks | Image/gradient first; then SVG and live sources through bounded providers/jobs/cache | Cancellation/race/input limits and per-provider cost/failure gates |
| M5 — stable distribution | Eight-hour soak, compatibility matrix, native packaging, artifact provenance, install/upgrade/rollback guide | G1–G7 pass on every advertised platform |

Performance work is continuous through M0–M5. Stable procedural release can ship after M3 and M5 without waiting for all M4 providers, provided scope is explicitly documented. This avoids delaying a useful product for complete upstream parity.

## Immediate work queue

1. Re-check PR #6/main and obtain the exact Windows preview artifact; record what backend smoke actually proves.
2. Render deterministic fixtures through the real OBS composition path. Focus on semi-transparent input, color packing, small sources, maximum glow and filter order.
3. Establish GPU/CPU/memory baseline on named Windows hardware. Do not claim “lighter than Advanced Masks” without a like-for-like mask-only comparison plus separate neon overhead.
4. Extract canonical validation/config from the filter while preserving all current settings and enum IDs. Add regression tests for invalid values and older scenes.
5. Implement safe-fit bounds and perimeter mapping as separate reviewable slices; update screenshots and budgets.
6. Add the signature ornament presets only on top of the verified geometry contract.

## Definition of done for a work item

Record requirement IDs, changed behavior, compatibility implications, tests actually run, results, exact artifact/commit and remaining limits. “Implemented,” “CI passed,” “runtime verified,” and “stable” are distinct states. Screenshots must come from the artifact under review.

A failed gate blocks the affected release claim, not unrelated documentation or diagnosis. Prefer repairing the smallest responsible subsystem over broad rewrites. When an environment prevents hardware verification, continue useful deterministic work and leave the hardware gate visibly pending.

## Maintenance cadence

For every runtime PR: update changed requirements/status and relevant tests. For every dependency/OBS upgrade: pin versions, run supported backend and package checks, verify old scene fixtures. For every release: archive the evidence record and publish known limitations. For every reported crash/regression: retain a minimized reproducer, add a meaningful regression test and document the fix.

Do not start a new roadmap in another file. New architectural choices append or supersede an ADR in ARCHITECTURE with reason, tradeoff and migration plan.
