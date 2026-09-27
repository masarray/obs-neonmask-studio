# Delivery roadmap and evidence ledger

Snapshot: 2026-09-26. Baseline includes Windows preview/D3D11 evidence through PR #8; this ledger is updated with each verified runtime slice.
This file owns delivery status; [PRODUCT_SPEC.md](PRODUCT_SPEC.md) owns requirements.
Update the ledger in the same change that advances implementation or verification.

## Baseline: what exists versus what is proven

| Area | Observed implementation | Evidence gap |
| --- | --- | --- |
| Native filter | C11/libobs callbacks in src/neonmask-filter.c | Frontend/lifecycle tests pending |
| Geometry | Proposal Phase-A set implemented: rounded rectangle, circle, oval, hexagon, diamond, rectangle, triangle and configurable polygon; mask X/Y and independent width/height | Actual OBS shape/framing captures, safe-fit and precision/corner validation pending |
| Neon | Phase-B analytic sharp/fine core, independent mid glow and broad bloom, additive local Flow hotspot, corner glints; existing four styles/presets and bounded phases | Actual OBS dark/light/320×180 art-direction review, safe-fit and eight-hour soak pending |
| State | Canonical pure-C config snapshot, schema v2; v0/v1 uniform-scale framing migrates additively to centered width/height + pan/zoom defaults | OBS callback handoff/lifecycle concurrency still needs runtime evidence; future schema/import UI not exposed |
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

**North star is unchanged:** achieve the intended image #9 visual families
in actual OBS, not just ship all names from the concept board. The revised
[PRODUCT_SPEC.md](PRODUCT_SPEC.md) and [VISUAL_SPEC.md](VISUAL_SPEC.md) make
visual acceptance explicit; [QUALITY_GATES.md](QUALITY_GATES.md) owns real
capture evidence. Do not claim current screenshots match the board.

Phase-A implementation now separates mask center/size from source sampling: mask X/Y, independent width/height, manual subject pan X/Y and uniform zoom feed distinct GPU transforms; out-of-source UV is explicitly transparent. Rectangle, triangle and configurable polygon append new shape IDs while the five legacy IDs remain unchanged. This is **implemented code awaiting artifact/actual OBS visual verification**, not a completed G4/G5 claim. Premium light hierarchy and contour-aware ornament/motion remain the next visual gaps.

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

## Art-first scope clarification

The maintainer explicitly wants a beautiful, artistic, fancy and futuristic
**mask + neon frame**, not background removal. No AI segmentation,
portrait extraction or auto-face tracking work is planned. Manual X/Y
reframing is the only subject-positioning feature required for the current
goal. Preserve original imagery inside the chosen geometric mask.

After X/Y framing, prioritize the **art-directed light and contour grammar**
over adding more generic shape presets: sharp/mid/bloom hierarchy,
intentional two-tone placement, selective glints, secondary tracks,
designed corner cuts/gaps, localized flow and graceful static forms.
Each family needs its own recognizable visual motif and actual OBS
capture/clip against image #9. A new color, radius, thickness or preset
label without distinct linework is not progress toward the target.

Image/gradient/SVG/source mask providers, if pursued later, are geometric
mask inputs rather than background-removal features; they must not
preempt the premium procedural collection. Keep PERFORMANCE/SAFETY gates
intact while pursuing visual quality, and report any unmet constraints.

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
4. **Art-directed signature recipes:** distinctive authored contour
   and color grammar, selective glints, refined double/triple tracks,
   intentional corner cuts, moving ring, HUD, Streamer/Chat and optional
   bounded Electric. No new advertised premium preset without an actual
   OBS capture and a motif distinguishable beyond hue/thickness changes.
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
5. ✅ Phase-A code implements mask X/Y, independent width/height, subject pan X/Y, uniform zoom, reset framing, rectangle/triangle/polygon and schema-v2 migration. Next: validate the Windows artifact in actual OBS with the off-center fixture, all eight proposal shapes, restart/resize/filter-order cases, then continue premium neon.
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

## Phase B — premium light engine implementation slice

Issue #17 implementation: one shared mask SDF feeds the sharp color core,
a fine bright centerline, a separate compact mid-glow envelope and a wider
exterior bloom. New native OBS controls expose mid strength, bloom strength,
highlight intensity and highlight width; existing overall strength/radius,
border width, colors and animation continue to work. In Flow, the base rim
remains bright and a white-biased local flare travels; Pulse modulates glow
while preserving the core. Supporting tracks gain bounded corner glints
in rounded/rectangle styles. All light layers composite in premultiplied
space and output OBS-compatible straight alpha. No segmentation, no CPU
video readback, no new rendering pass or worker.

The config remains schema v2: the four added fields have explicit OBS defaults
and complete preset snapshots. Missing light values are pinned as explicit
user values on first update so subsequent default revisions cannot silently
rewrite saved scenes; existing explicit values and saved framing/pan/zoom are
not touched. Old preset/shape IDs remain stable. Tests now target mid-only versus broad-bloom falloff, the disabled
glow path and an east/west local hotspot using the actual OpenGL/D3D11 effect.

**Evidence boundary:** passing shader/device/pixel checks is not an OBS
frontend image/animation comparison. Actual screenshots/clips against the
proposal, filter-chain tests, safe-fit for displaced masks and performance
on named hardware remain open G4-V/G5/G6 gates. Existing angle-based Flow
is still provisional; arc-length routing is a follow-up.


## Phase C1/C2 — art recipe foundation + Cyber Rounded (implementation candidate)

The first art-directed renderer slice adds an opt-in ornament recipe separate
from mask coverage. Existing scenes and Custom settings default to no new
ornament; explicitly selecting Cyber Rounded applies the new complete recipe.
The source image, Phase-A mask X/Y and subject pan/zoom remain independent.
Cyber Rounded gains paired geometric corner traces, a deliberately broken
outer support track and arc-length-based Flow around its rounded perimeter.
This is analytical contour/path grammar authored in code, **not** a PNG pasted
over the source, and does not require SVG parsing at OBS runtime.

New controls: ornament mode (None/Cyber), accent intensity and outer-track
gap; parameter limits are validated centrally and saved explicitly. Existing
shape IDs, border-style IDs and config schema v2 remain stable. Art details
cannot change image-mask coverage. Pure-C perimeter tests and direct-effect
OpenGL/D3D11 pixel fixtures must pass before merge.

**Scope boundary:** This is the first hero *implementation*, not acceptance
of the whole Phase C proposal. Reactor orbit, Tech HUD, Streamer/Chat,
Electric and the full vector authoring/import pipeline remain future slices.
Real OBS dark/light, 320×180 and displaced-mask captures, safe-fit, and
measured GPU frame times remain mandatory before visual parity claims.


## Phase C3/C4 — three additional authored families (implementation candidate)

This slice extends the opt-in recipe enum without changing existing IDs:
Reactor Ring uses deliberately unequal segmented concentric arcs, radial ticks
and a localized orbit head; Tech HUD uses long orthogonal corner brackets and
a cut support rail; Streamer Bubble uses a two-segment decorative tail and a
small status-dot trio. Preset IDs 1–4 remain stable and IDs 5/6 add Tech HUD
and Streamer. Selecting Reactor preset ID 2 now explicitly opts into its
authored ornament; saved scenes without an ornament field remain legacy mode
0. Source image/background remains inside its geometric mask: the bubble
tail is an **ornament, not a person cutout or expanded image silhouette**.

No runtime SVG parser or asset texture dependency is introduced: geometry is
expressed as contour-relative analytical paths, with one existing effect
pass and no video readback. Motion remains bounded in the existing phase
model. Test on real OpenGL and D3D11 against transparent input to ensure
ornaments are present ONLY when selected, while preserving existing alpha,
mask X/Y, subject pan/zoom and Phase-B light tests.

Real visual quality, corner-detail legibility, safe-fit, GPU frame timing and
full user OBS comparison remain open G4-V/G5/G6 gates. Do not claim marketing
board parity or a stable release from this preview build.


## Phase D scope merged with Issue #26 — first executable vertical slice

[Phase D execution contract](PHASE_D_IMPLEMENTATION.md) owns the sequence and
[Issue #26](https://github.com/masarray/obs-neonmask-studio/issues/26)
tracks implementation. D1 adds actual bubble and asymmetric cut-corner shape
IDs 8/9, default-backed shape-detail parameter, reselectable Streamer bubble
and new Angled Card preset 7. The same signed distance drives alpha, border,
glow and secondary tracks. Old saved IDs/shape assignments remain unchanged.
The original SVG authoring samples are not a runtime SVG importer. D2 is
bounded SVG/distance field and expanded original shape library; D3 is safe-fit,
actual OBS comparison and performance acceptance in Issue #24. Do not mark
D2/D3 or image #8/#9 visual parity complete from a shader CI result.

## D2.2 — exterior-only Bubble contour and rounded leaf tip

The latest user's continuation request retains Phase D Issue #26 and the
image #8/#9 visual north star. D2.1 selective Angled Card Roundness (PR #29)
is the verified baseline. This next focused slice replaces the Bubble's
distance-to-two-overlapping-primitives with a distance to one **exposed
exterior contour**. Hidden body-bottom/tail-base edges are excluded so they
cannot be illuminated as a horizontal internal seam. Roundness controls
the tail-tip fillet; shape-detail controls tail depth. Both share a single
coverage/neon/bloom contour. Existing IDs, source X/Y and zoom stay intact.
See [PHASE_D_IMPLEMENTATION.md](PHASE_D_IMPLEMENTATION.md).

Add pure-C and OpenGL/D3D11 shader pixel regression fixtures for the seam
and tail; do not claim an OBS visual pass until actual captures at multiple
Roundness and image sizes are reviewed. SVG/custom mask importer and shape
library, safe-fit, 320×180 aesthetic review, performance and eight-hour soak
remain outstanding Phase D gates. This intentionally corrects Bubble ID 8
appearance without altering unrelated saved shapes.

## Phase D2.3a — fuller authored Angled Card arc

The user confirmed Bubble's former interior seam visually clean on the post-PR30 preview, while Angled Card Roundness 1.00 remained too angular. This focused refinement replaces the old 1.20×cut fillet-radius cap with a normalized 0..1 mapping reaching two tangent arcs with **one shared center and no flat diagonal** at maximum. Square top-left and bottom-right corners remain sharp, cut size remains separately adjustable, and a single SDF still controls mask, neon core and bloom. Existing shape ID 9/scene keys/presets are preserved, with an intentional visual improvement for saved nonzero Angled Card roundness. The rest of D2 (authored shape pack, bounded SVG importer) and D3 safe-fit/performance are not claimed complete from automated tests alone. See [PHASE_D_IMPLEMENTATION.md](PHASE_D_IMPLEMENTATION.md).

## Phase D2.3b — first authored shape pack milestone

Added HUD Cut Panel (shape 10; asymmetrical chamfers and a true stepped
lower-left cutout) and Squircle (shape 11; continuous exponent-controlled
superellipse). Preset IDs 8/9 are additive and do not rewrite older saved
scenes. HUD signature supporting rail follows the actual non-convex mask
distance. Both masks are source-alpha silhouettes, not decorative PNGs;
the common SDF drives border, mid glow and bloom. Shape Detail is now
labelled for tail, cut and squircle in English/Indonesian. The GPU path
continues to use one composition effect without CPU frame processing.
See [PHASE_D_IMPLEMENTATION.md](PHASE_D_IMPLEMENTATION.md).

The deliverable is **code-level D2 built-in shapes v1**, not all of D2.
The local bounded SVG import/cache provider, Star/Heart/Rounded Polygon,
actual full-size/320×180 OBS captures, D3 safe-fit and hardware timing/soak
remain pending and keep Issue #26 and #24 open.

## D2.3c OBS refinement and clarity

User-approved the Angled Card arc; HUD/Squircle screenshots showed a plain custom-state outline and remaining visual roughness. Audited actual **Custom/current settings** state versus opt-in presets 8/9. The HUD notch now has two chamfered transitions instead of unstyled square stairs, plus deliberate preset-only top/side accent traces. Squircle near-flat SDF is axis-stabilized. Irrelevant native geometry controls are hidden for the selected shape without deleting or resetting the user's saved data. The gallery and other design families are NOT considered complete. Verify exact main build in OBS, compare with reference #9 over dark/light and 320×180, and keep Issue #24 release gate open.
