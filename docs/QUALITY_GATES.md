# Quality, performance and release gates

All numeric budgets below are initial engineering targets, not measurements of v0.1.0.
Every release report names the exact commit, OBS/SDK version, OS, GPU, driver, source resolution, canvas resolution, frame rate and preset.

## Performance budgets and measurement

At 60 fps, the total frame budget is about 16.67 ms; NeonMask receives only a small fraction.
Default procedural mode targets one custom composition pass plus host capture, no recurring CPU allocations, no CPU video readback, no recurring texture upload and no asset worker activity.

| Scenario | Incremental GPU p95 target per active filter | CPU callback p95 target | Incremental steady GPU memory target |
| --- | --- | --- | --- |
| 1920 × 1080 input at 60 fps, default procedural | ≤ 0.50 ms | ≤ 0.10 ms | ≤ 32 MiB |
| 1920 × 1080 input at 60 fps, maximum shipped ornament preset | ≤ 1.00 ms | ≤ 0.15 ms | ≤ 64 MiB |
| 3840 × 2160 input at 60 fps, default procedural | ≤ 1.50 ms | ≤ 0.15 ms | ≤ 128 MiB |

Measure at actual source resolution, not just the small facecam size on the canvas. Four active filters must also be tested as a combined scene; do not assume perfectly linear scaling. Asset-provider budgets are additional and must be declared before shipping them.

M0 selects and records exact reference machines: at least one modest integrated GPU and one discrete gaming GPU on Windows/D3D11. Until those devices and runs exist, budgets remain provisional and “lightweight” remains a design intent. Mesa software rendering validates compilation/pixels but cannot certify consumer GPU performance.

Protocol:
1. Use identical OBS scene, deterministic input and output settings, with filter disabled/enabled. Warm up for 60 seconds.
2. Record three five-minute trials per scenario. Report median, p95, p99 and worst frame, not just average FPS.
3. Use GPU timestamp profiling without synchronous readback in the normal renderer; CPU callback duration is not GPU time. Identify whether measured GPU cost includes host capture.
4. Record OBS render-lag counters and skipped frames during real recording/streaming. Require no reproducible plugin-induced render-lag increase in the controlled scene.
5. Include single/four-filter scenes, 720p/1080p/4K sources, hidden/visible transitions and default/max settings.
6. Save raw timing summaries and memory measurements with methodology. Investigate a reproducible >10% regression even if under the absolute budget; compare run variance.

Fixed Low/Balanced/High modes may be introduced after measurements. Reduce optional halo/ornament cost before affecting mask accuracy. Adaptive quality, if added, needs hysteresis, a cooldown, visible mode indication and a manual lock; it must not oscillate every frame or silently change crop.

## Evidence ladder

| Gate | Required evidence | What it does not prove |
| --- | --- | --- |
| G1 Headless | Release-safe math/config/preset tests; validation/migration cases; sanitizers | Real shader execution |
| G2 Native/package | Pinned SDK compilation, link and archive layout | Loading in OBS or visual correctness |
| G3 GPU compile | Real libobs effect compilation/binding on OpenGL and D3D11 | Correct final pixels |
| G4 Pixel/visual | Render fixtures and final OBS captures per VISUAL_SPEC | Long-term reliability |
| G5 Runtime/lifecycle | OBS frontend, filter chains, scene transitions, device/source changes | Performance on every GPU |
| G6 Performance/soak | Budget report, lifecycle stress and memory checks on declared hardware | Universal absence of crashes |
| G7 Release | Exact artifact provenance, install/restart/rollback checks and known limitations | Support for untested platforms |

Automated suites cover G1–G3 and a **partial G4 direct-effect pixel fixture**: real OpenGL/D3D11 framebuffer readback samples opaque, premultiplied half-alpha and border-only inputs. This does not exercise the OBS source/filter chain or final canvas output. Inspect run results rather than inferring success from script presence. The motion suite advances G1 by simulating eight hours of 60 fps ticks, but it does **not** satisfy the eight-hour OBS runtime/driver/memory soak in G6. Follow [GPU_SMOKE.md](GPU_SMOKE.md) and [WINDOWS_PREVIEW.md](WINDOWS_PREVIEW.md) for setup.

## G4-V visual identity and framing gate (mandatory for polished claim)

The user's September 26 concept board (#9) is the target, while OBS
screenshots #1–#8 document the current functional but visually incomplete
baseline. Distinguish a **feature built**, **a shader-level pixel test**,
**actual OBS rendering**, and **human reference-family acceptance**.
Only the latter two can support "premium" or visual-parity wording.

For each advertised preset family, record an actual OBS output screenshot
against a dark and light background and a short clip for motion modes.
Archive settings/geometry, test image identity, source and canvas sizes,
OBS version/backend/GPU/driver, commit and ZIP hash. Inspect full
resolution and downscaled 320 × 180, before/after filter chaining and
a resized source. Compare with the named recipe in VISUAL_SPEC, not
pixel-match a different person's photo.

Specific next-preview exit cases:

1. **Framing:** move an off-center subject horizontally/vertically with
   subject pan while holding the border position fixed; then move the
   mask while holding source sampling fixed. Verify independent width/height,
   uniform zoom, reset, saved-scene restart and unchanged legacy default.
   Out-of-range UV cannot smear edge pixels or reveal unintended areas.
2. **Premium light:** show a stable narrow core, distinguishable mid glow
   and wider soft outer bloom; document core, halo and accent settings.
   Check over dark, light and moving footage. A single uniformly brighter
   rim is not an acceptable substitution.
3. **Motion:** provide fixed-time frames and short videos showing a clear
   localized flow accent, preserved base rim and visible controlled pulse;
   speed zero/static must freeze all channels. Test shape seams and
   reduced-motion variant.
4. **Geometry:** common mask/border/light alignment including translated
   shape, effect envelope and independent source transform; preserve
   existing mask-edge/stroke/safe-fit pixel tolerances.
5. **Resource tradeoff:** capture GPU/CPU and render-lag measurements for
   new light layers on declared hardware and compare to the baseline.
   If added bloom passes exceed budgets, optimize or publish the failure;
   do not quietly substitute the older plain look while claiming parity.

Human visual review is required in addition to automated tolerances.
Unverified images, synthetic marketing mocks and shader compilation
cannot be recorded as G4 final OBS evidence. The quality thresholds in
the existing G4/G5/G6/G7 gates remain in force.

## Reliability and failure matrix

| Test | Minimum procedure | Pass criterion |
| --- | --- | --- |
| Repeated lifecycle | 1,000 create/update/destroy cycles in a harness; 100 add/remove or scene-switch cycles in OBS | No crash, UAF, double-free, sanitizer error or retained plugin-owned resources |
| Long session | Eight-hour default and animated soak on declared Windows runtime | No crash, hang, phase reset/jitter or sustained unexplained memory growth |
| Memory recovery | Warm baseline, repeated peak use, destroy all filters, wait for documented cleanup | Owned allocations/refs return to zero; bounded caches plateau; driver retention separated from leaks |
| Malformed values | NaN/Inf, enum overflow, extreme sizes, missing keys, old settings | Finite validated state, no invalid GPU parameters |
| Shader failure | Missing file, compile error, missing uniform/technique | Documented fallback and bounded logs |
| Source disruption | Resize, disconnect/reconnect, zero-size, duplicate/remove, nested scenes | Correct recovery and no stale pointer use |
| Filter order | Crop, color correction, chroma key before/after NeonMask | Correct geometry/alpha or explicit supported-order restriction |
| Future asset races | Rapid replace, destroy during decode, stale result, cancellation, invalid image/SVG | Latest live generation only; bounded memory; no deadlock |
| Future source masks | Self-reference, indirect cycles, deleted source | Reject recursion, matched reference release and safe output |

Use ASan/UBSan for applicable native/harness paths; test concurrency with a race detector where supported when workers are introduced. Do not rely on process RSS alone to assert leaks: track plugin allocations, graphics resources, reference lifetimes and cache occupancy. Retained driver memory must be distinguished from unbounded growth.

## Automated and manual responsibilities

Automate pure geometry/config tests, invalid-input cases, scene migrations, uniform contracts, backend compilation, deterministic pixel fixtures and archive manifests. Use a real OBS frontend for camera/capture/filter chaining and visual interaction checks. If the execution environment cannot run OBS, mark these checks Pending and provide the exact artifact/test instructions; never mark complete.

CI build success alone cannot promote a stable release. Release evidence should use a short record:

- Commit and artifact SHA256:
- OS / OBS / SDK / GPU / driver:
- Source / canvas / fps / preset:
- Gates G1–G7: pass, fail or pending with evidence links:
- Frame timing and memory report:
- Screenshots/clips and deterministic fixture settings:
- Known limitations, reproduction steps and rollback instructions:

## Packaging and support

Windows x64 portable unsigned ZIP is the first delivery. Include DLL, matching effects/locales, version, license and checksum. A signing certificate is not a prerequisite for this open-source preview. Never replace a loaded DLL; close OBS for manual install/update. Validate fresh install, upgrade preserving settings, missing shader behavior and rollback.

The baseline Windows build pins OBS 31.1.1; this does not establish compatibility with every later OBS version. Define a supported runtime matrix from actual tests, then add newer stable OBS versions deliberately. Linux and macOS headless builds do not establish native packaging/runtime support. Publish only platforms that passed the relevant gates, with preview/stable labels matching the evidence.

Do not add paid certification, DCO/signoff or CLA machinery as a substitute for engineering validation. Keep provenance/license notices and ordinary review.

## Phase B targeted shader pixel checks (partial G4 only)

The actual libobs GPU smoke now distinguishes three cases over a transparent
source: broad+mid glow, mid-only with bloom disabled, and both glow envelopes
disabled. It also compares the local Flow highlight near and opposite its
configured phase while checking transparency outside the frame. This is
a *direct-effect* framebuffer test; actual OBS frontend compositing,
dark/light backgrounds, small 320×180 visual review, filter chaining and
frame timing remain required before G4-V/G5/G6 acceptance. A green CI
result alone does not mean proposal-level visual parity.


## Phase C art-direction gate (first authored family)

Unit-test rounded-rectangle contour-turn mapping across straight/arc
tangencies and enforce bounded recipe settings. Direct libobs OpenGL/D3D11
pixel fixtures must distinguish a bright Cyber corner trace from the same
fixture with ornament mode 0; baseline alpha/framing/glow tests must continue
to pass. This establishes **partial shader-level G4**, not final OBS output.

For Cyber acceptance, collect actual OBS stills (Static, Flow) at full
resolution and 320×180 with dark/light background. Review intentional gaps,
visible paired corner traces, proportions, downscale readability and motion
continuity on a non-square frame. Test extreme mask X/Y for clipping. Do not
claim Phase C completion without verified Reactor/HUD/Streamer signatures,
full G4-V/G5/G6 evidence and release acceptance.


## Phase C additional direct-effect proof (partial G4)

Add independently sampled shader fixtures for Reactor outer arc, Tech HUD
bracket and Streamer speech tail. Each has a corresponding opt-out/legacy
control where relevant, to detect accidental decoration of old scenes.
The fixtures run against real libobs OpenGL/D3D11 render targets and do not
claim final OBS frontend compositing, finished proposal aesthetics, safe-fit
or measured performance. All original alpha/framing/light fixtures must
remain green.


## Phase D1 integrated shape proof

Pure-C CPU oracle checks bubble tip occupancy, transparent region beside
the tip, absence of seam, card missing asymmetrical corners and signed
Euclidean edge samples. Direct libobs OpenGL/D3D11 GPU fixtures must check
opaque source RGB/alpha inside bubble tip, alpha outside beside tail,
bubble-tip border over transparent input, missing angled-card corner and
neon following a diagonal; include legacy rounded/rectangle controls. This
only covers effect-level G4; real OBS post-filter output, safe-fit and
concept-art parity still need human/image evidence.


## Selective Angled Card Roundness (D2.1)

Do not mark this feature complete from the unchanged rectangle test. Exercise
the EXISTING `roundness` key through zero and nonzero settings for shape ID
9. Pure-C and actual OpenGL/D3D11 GPU fixtures must prove the straight
D1 cut at zero, removal of the previous diagonal endpoint at higher values,
and preserved 90-degree top-left/bottom-right corners. Ensure the neon
uses the same rounded contour (no detached line) and shape-detail still
changes cut depth independently. Actual OBS screenshots at zero, midpoint
and maximum are needed to judge leaf-like visual quality.


## D3-specific verification

D3 source/output coordinate, separated padding and impossible-offset checks
are automated in neonmask-safe-fit-tests, with expanded shader origin pixels
in neonmask-gpu-smoke. Source-recursive filter composition, scene-item origin
alignment, crop/chroma order, resize/restart and real GPU soak require actual
OBS frontend G4-V/G5/G6 evidence. An enabled `expand_canvas` filter shifts
content inside the reported source by left/top pixels; never record this as
automatic scene transform preservation. See PHASE_D_IMPLEMENTATION.md.
