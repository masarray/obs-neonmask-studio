# Delivery roadmap and evidence ledger

Snapshot: 2026-09-26. Baseline includes Windows preview/D3D11 evidence through PR #8; this ledger is updated with each verified runtime slice.
This file owns delivery status; [PRODUCT_SPEC.md](PRODUCT_SPEC.md) owns requirements.
Update the ledger in the same change that advances implementation or verification.

## Baseline: what exists versus what is proven

| Area | Observed implementation | Evidence gap |
| --- | --- | --- |
| Native filter | C11/libobs callbacks in src/neonmask-filter.c | Frontend/lifecycle tests pending |
| Geometry | Five shapes, approximate distances, source-size coordinates | Precision and corner/ellipse visual validation pending |
| Neon | Dual color, analytic glow, pulse/flow, four styles and presets; independent bounded double-precision phase accumulators | Actual eight-hour OBS soak and final rendered pixels pending |
| State | Canonical pure-C config snapshot, schema v1, v0 legacy marker migration, complete preset values | OBS callback handoff/lifecycle concurrency still needs runtime evidence; future schema/import UI not exposed |
| Safety | Shader load/uniform checks; bypass; effect cleanup | Fault injection, resource accounting and soak pending |
| CI | Headless/platform builds, OpenGL and Windows D3D11 compile smokes, Windows preview ZIP; M0 synthetic fixture generator and build provenance in preview pipeline | Direct GPU effect pixel fixtures cover only part of G4; final OBS source/filter-chain captures and benchmarks remain pending |
| Assets/workers | No custom image/SVG/live-mask provider | Bounded service is a future requirement |
| Performance | Procedural one-effect design | No hardware budget evidence recorded |
| Padding | Existing source canvas only | Large glow can clip; safe-fit/expanded-output validation pending |

[PR #6](https://github.com/masarray/obs-neonmask-studio/pull/6) merged after Windows D3D11 shader compilation smoke passed: [workflow run 36236980372](https://github.com/masarray/obs-neonmask-studio/actions/runs/36236980372), head SHA `80cfce3569f0ef7187109b29c9d6f26f21dd67d0`, preview artifact ID `10904645446`. This certifies a backend compiler/device check, not rendered pixels, frontend use, performance or other OBS versions.

## Actual user OBS feedback — September 26 (design-gap record)

User screenshots #1–#8 demonstrate that the filter loads and displays the
five shipped shapes and four presets in OBS. This is useful preliminary
frontend evidence, **not** complete G4/G5 (the exact artifact SHA, host
version, test matrix and video/performance data have not been recorded).
Relative to user concept board image #9, the pictured outputs still
lack independent facepan X/Y, the multi-scale premium luminous appearance,
clearly distinguishable secondary tracks and pronounced localized animation.

**Scope clarification:** pursue **art-directed geometric masks and futuristic
luminous frame compositions**, not background removal or an AI portrait cutout.
The webcam background remains inside the selected mask. Distinctive stroke
hierarchy, intentional gaps, contour-anchored ornaments and premium color/light
balance matter more than simply adding generic shapes or preset names.

**North star is unchanged:** achieve the intended image #9 visual families
in actual OBS, not just ship all names from the concept board. The revised
[PRODUCT_SPEC.md](PRODUCT_SPEC.md) and [VISUAL_SPEC.md](VISUAL_SPEC.md) make
visual acceptance explicit; [QUALITY_GATES.md](QUALITY_GATES.md) owns real
capture evidence. Do not claim current screenshots match the board.

Current code-level diagnosis: the config exposes only a uniform mask scale
and no independent pan/center/zoom; the shader evaluates a centered SDF,
samples unchanged UV and mostly combines core + one analytic halo with
angle-derived color/flow. The image #9 target calls for independent
framing, luminous hierarchy and contour-aware details. This is a
prioritized implementation gap, not a reason for a rewrite.

## Milestones, in dependency order

Work in small vertical slices: implementation + relevant automated test + runnable artifact + evidence. These milestones refine the earlier README phase list; old “Phase 2” wording is historical.

| Milestone | Deliverables | Exit / next |
| --- | --- | --- |
| M0 — reproducible baseline | Reconcile PR #6, record exact preview artifact, select reference GPUs; document actual Windows loading and captures | G1–G3 and initial G4/G5 evidence; list failures before adding features |
| M1 — correctness and lifecycle | Canonical config validation/migrations; resource ownership; fault handling; alpha/color fixtures; long-session motion fix if tests expose it | G4/G5 pass for current shapes, baseline performance and memory report |
| M2 — polished procedural core | **First** mask X/Y, subject pan X/Y and uniform zoom; then independent width/height and safe-fit; precision shapes/triangle/polygon and contour-length flow follow; compact grouped controls | Off-center source reframes without contour shift or stretch; existing and new shapes pass visual matrix; four premium refined presets within budget |
| M3 — signature visual collection | Premium sharp core + mid glow + broad bloom + traveling hotspot ahead of cosmetic preset proliferation; then capped layers/corner ornaments/ring/streamer/chat/optional electric, reduced motion and preset serialization | Actual OBS image #9 family comparison, static/motion captures, import validation and max-preset benchmarks |
| M4 — broader masks | Image/gradient first; then SVG and live sources through bounded providers/jobs/cache | Cancellation/race/input limits and per-provider cost/failure gates |
| M5 — stable distribution | Eight-hour soak, compatibility matrix, native packaging, artifact provenance, install/upgrade/rollback guide | G1–G7 pass on every advertised platform |

Performance work is continuous through M0–M5. Stable procedural release can ship after M3 and M5 without waiting for all M4 providers, provided scope is explicitly documented. This avoids delaying a useful product for complete upstream parity.

## Next-preview implementation order (vertical PRs)

1. **Framing / X-Y first (MASK-04):** separate mask center X/Y from subject
   pan X/Y and uniform zoom; keep default centered and preserve existing
   scene settings and preset IDs. Add OBS controls, pure-C geometry tests,
   shader uniform tests and a Windows preview artifact. Prove with an
   off-center subject; do not hide a pan under global scene transform.
2. **Premium neon fundamentals (NEON-01/05):** refine crisp core, mid
   glow and outer bloom, independent contrast/falloff and localized
   visible flow/pulse while leaving inner image clear. Test direct GPU
   pixels and real OBS captures, including 320 × 180. Keep a polished
   static appearance. Profile before adding a render pass.
3. **Safe-fit and geometry:** account for all translated light/ornament
   bounds; no source-edge smear or unwanted outer clipping. Add
   independent shape dimensions, precision contour and arc-length
   motion in reviewable slices.
4. **Signature design recipes:** implement the art-direction grammar in
   VISUAL_SPEC with coherent static compositions (line hierarchy, negative
   space, contour-aware accents and dual-color balance) before adding
   double/triple rim, moving ring, Streamer/Chat and optional Electric.
   No new advertised premium preset without an actual OBS capture; no
   background-removal or person-segmentation work is needed.
5. **Reliability and release:** source/restart/legacy-scene tests,
   real performance measurements and eight-hour soak; honor G1–G7.
   Image/gradient/SVG/live-mask providers remain later M4 work.

Each runtime slice must update this ledger with its commit, CI jobs,
downloadable GitHub artifact, actual OBS screenshots/clips where available,
measured performance if applicable and explicit pending gates. Existing
default/preset save compatibility is an acceptance condition, not optional.

## Immediate work queue

1. Preserve the user screenshots #1–#8 as initial functional feedback; obtain OBS log, version, artifact SHA and full-output captures to complete formal G4/G5. Use [M0 visual test procedure](M0_VISUAL_TEST.md).
2. Render deterministic fixtures through the real OBS composition path. Focus on semi-transparent input, color packing, small sources, maximum glow, filter order, and static/flow frames at deterministic times.
3. Establish GPU/CPU/memory baseline on named Windows hardware. Do not claim “lighter than Advanced Masks” without a like-for-like mask-only comparison plus separate neon overhead.
4. ✅ Canonical validation/config extracted without changing existing setting keys or enum IDs; schema v1 treats missing-version scenes as v0, rejects future schemas, and has invalid-value/complete-preset regression tests. Next: verify saved v0/v1 scene fixtures in OBS and the callback handoff assumptions.
5. Implement the next-preview X/Y mask placement and independent subject pan/zoom before safe-fit/perimeter mapping; compare with image #9 family anatomy and update screenshots/budgets.
6. M1 motion slice: replace the former 3600-second clock reset with independent bounded phases. Cross-platform tests simulate 8 hours of ticks, but do not establish the G6 eight-hour OBS runtime soak. Add signature ornaments only after geometry/pixel validation.

## M0 reproducible visual test inputs (automation, not runtime evidence)

The visual-fixture generator produces six owned RGBA PNG inputs (opaque bars,
alpha ramp, soft edge, small checkerboard, portrait bars and fully transparent)
plus an SHA256/alpha manifest. The Windows preview workflow packages them
separately from the installable plugin ZIP and records the exact checked-out
commit, workflow run, OBS 31.1.1 SDK and both ZIP checksums. The headless
matrix verifies PNG structure, CRC, dimensions, RGBA and alpha categories.

This removes test-input ambiguity. The shader smoke now renders three small
synthetic textures directly to an actual GPU target and reads alpha/color pixels
(opaque center, premultiplied half-alpha center, outside-mask transparency, and
border-only rim). It is **partial G4**: not the final OBS composition, not an
image-source/crop/filter-chain test, and not G5 OBS frontend/lifecycle evidence. Those still require real
captured OBS output and logs on Windows as specified in
[M0_VISUAL_TEST.md](M0_VISUAL_TEST.md). G6 hardware timing and eight-hour soak
are not claimed.

## M0 partial GPU pixel gate

`tests/test-gpu-smoke.c` uses libobs to compile and **draw** the shipped effect
with a tiny synthetic texture, stage its output, and compare sampled pixels
under both Linux/OpenGL and Windows/D3D11 workflows. Runtime readback occurs
only in the CI executable; the shipping filter has no GPU-to-CPU readback.
The selected samples assert a transparent outside corner, a preserved opaque
red center, a premultiplied half-alpha center, and a neon rim with fully
transparent source. This is a targeted shader-level acceptance gate, not a
substitute for the final OBS canvas screenshots and filter-chain evidence
required by G4/G5 or the GPU performance report required by G6.

## Definition of done for a work item

Record requirement IDs, changed behavior, compatibility implications, tests actually run, results, exact artifact/commit and remaining limits. “Implemented,” “CI passed,” “runtime verified,” and “stable” are distinct states. Screenshots must come from the artifact under review.

A failed gate blocks the affected release claim, not unrelated documentation or diagnosis. Prefer repairing the smallest responsible subsystem over broad rewrites. When an environment prevents hardware verification, continue useful deterministic work and leave the hardware gate visibly pending.

## Maintenance cadence

For every runtime PR: update changed requirements/status and relevant tests. For every dependency/OBS upgrade: pin versions, run supported backend and package checks, verify old scene fixtures. For every release: archive the evidence record and publish known limitations. For every reported crash/regression: retain a minimized reproducer, add a meaningful regression test and document the fix.

Do not start a new roadmap in another file. New architectural choices append or supersede an ADR in ARCHITECTURE with reason, tradeoff and migration plan.

## M1 animation continuity (implementation and evidence limits)

The same M1 correction distinguishes an explicit saved `schema_version` from an OBS default via `obs_data_has_user_value`. Without this, the default `1` would hide legacy v0 scenes; the libobs smoke now tests that API contract. Actual scene import/restart evidence remains pending.

The procedural renderer now uses pure-C `neonmask-motion.*`: bounded double-precision
phase for color gradient, pulse, and traveling accent, uploaded as float turns.
This fixes the exact-hour `fmodf(time, 3600)` jump and preserves phase when a
user changes speed. Static mode and zero speed freeze all phase channels;
relative visual frequencies remain unchanged. `neonmask-motion-tests` exercises
invalid inputs, mode/speed changes, one-hour boundary and simulated 8-hour
60 fps phase integration. Backend effect compilation is covered by the existing
OpenGL and D3D11 smoke checks. This is **not** measured continuous rendering,
final-pixel verification or a hardware performance result. G4–G6 stay pending.
