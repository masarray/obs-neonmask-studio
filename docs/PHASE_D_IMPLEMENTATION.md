# Phase D — artistic mask engine and Advanced Masks capability parity

**Owner:** [Issue #26](https://github.com/masarray/obs-neonmask-studio/issues/26).
**Release gate:** [Issue #24](https://github.com/masarray/obs-neonmask-studio/issues/24).
**Target:** user's reference image #8 (diagonal cut card/family) and #9
(integrated neon chat bubble), not AI/person segmentation.

## Architectural decision

Use a unified mask provider returning coverage and a contour-distance metric.
The production GPU path uses the *same signed distance* for webcam clipping,
neon core, multi-scale bloom and secondary tracks. A disconnected outside
ornament is not an acceptable substitute for a bubble-tail silhouette.
Preserve source-background inside the mask and independently editable mask
position/size versus subject pan/zoom. Existing IDs remain immutable.

Advanced Masks is our audited capability/reference baseline (regular polygon,
rounded corners, star, heart, superformula and SVG/file/text masking). It
rasterizes SVG into a texture using Qt SVG. Our visual differentiator is a
consistent luminous contour, not cloning its UI/source files. Upstream repo
declares GPL-2.0; NeonMask root license is GPL-3.0. Before importing any
specific file, establish GPL-2.0-or-later permission or obtain authorization,
retain exact copyright notices and document provenance; otherwise implement
the feature independently. All current D1 code/art is original.

## D1: real integrated silhouettes (this PR)

- Append shape IDs 8=Chat Bubble and 9=Angled Card; do not reinterpret 0..7.
- Bubble signed distance = union of rounded body with triangular tail; the
  triangle overlaps the body to remove the interior seam. The webcam and
  core/bloom occupy the tail, unlike the old Streamer decorative outline.
- Angled Card uses Euclidean signed distance to a six-edge convex contour,
  clipping top-right and bottom-left by a size-controlled diagonal.
- `shape_detail` controls relative tail depth / diagonal cut; validated
  0.08..0.35. Mask X/Y, independent width/height and rotation still apply.
- Reselecting preset 6 opts into actual Chat Bubble; new preset 7 = Angled
  Card. Existing saved rounded Streamer scenes keep shape 0 and their old
  decorative recipe until the user explicitly selects a new preset.
- Default X/Y centered shapes reserve bubble tail inside the current mask
  extents; large translated frame or outer bloom can *still* clip against
  the original OBS source texture. Do not claim expanded filter output.
- Pure-C geometric oracle tests occupancy, missing corners and edge signs;
  actual libobs OpenGL and D3D11 fixture tests verify source alpha **inside**
  tail, transparency beside it, neon at tip/diagonal and legacy controls.

## D2: diverse geometry and bounded SVG provider (partial implementation; see D2.3/D2.4)

1. Curate original vector silhouettes from the proposal: HUD cut-corner,
   portrait capsule, star, heart, rounded polygon and squircle/superellipse.
   Implement parametric generators first; each gets exact alpha/neon fixtures.
2. Add optional local SVG-file provider: strict parser/rasterizer, reject all
   external resources, scripting, fonts and remote URLs; cap file bytes,
   path count/segments, dimensions and decode/cache memory.
3. On file/size change only, rasterize shape to bounded alpha texture and
   derive a signed distance field at that resolution; no CPU video-frame
   processing. Define contour policy for disconnected holes and motion.
4. Fail invalid asset *closed* (transparent mask with warning), never silently
   bypass and expose an unmasked image. Clear stale textures on failure.
5. Select fill-rule, antialias semantics and aspect-ratio fit policy, with
   deterministic save/reload and resize behavior.
6. Preserve copyright/license metadata for every bundled artwork. SVG files
   in assets/designs are **authoring samples**. From D2.4 the original
   simple path samples are also local-file import examples; general SVG
   rendering remains unsupported.

## D3: art polish, safe-fit and acceptance (not yet implemented)

- Expand libobs filter output bounds correctly (dimension callbacks, origin
  translation, target and downstream scene/filter transforms). Verify with
  real OBS source and scene capture; merely moving UV does not add canvas.
- Mask contour animation must follow actual contour length, including the
  bubble tail and angled diagonal segments. Angle-based flow is provisional.
- Complete artist-designed frames/presets and screenshot/clip comparison with
  reference #8/#9 at full size and 320x180, dark/light scenes, Static/Flow,
  normal and offset X/Y. No generic recolored-rim claim.
- Measure GPU costs/memory on named cards, check source resize/restart,
  filter chaining, old settings and unsigned Windows package. OBS visual
  acceptance remains separate from successful shader/direct-pixel CI.

## Done definition

D1 is a code-level preview only after headless + native Linux + real
OpenGL/D3D11 GPU pixel tests pass. Entire Phase D is complete only when D2/D3
deliverables and the visual/functional/performance gates have real evidence.


## D2.1 — existing Roundness slider, selective Angled Card fillets

The maintainer clarified that Angled Card's *existing* diagonal geometry is
already desirable. Reuse the existing persisted `roundness` slider; do not
replace the shape with a fully rounded rectangle, and do not add another
competing slider. At **0**, the six-edge D1 geometric cut remains exactly as
before. Increasing Roundness introduces circular tangent fillets ONLY at
the four endpoints of the two diagonal cuts (top-right and bottom-left),
transitioning toward a leaf/petal-like cut. The top-left and bottom-right
90-degree vertices MUST remain mathematically sharp for the entire range.

`shape_detail` continues to control cut depth independently of `roundness`.
The effective fillet radius is bounded by cut depth to avoid crossing
tangencies on small frames. The same signed Euclidean distance drives image
alpha, border, glow and secondary rails. The native OBS slider is surfaced
with the mask geometry controls and an explicit Angled Card explanation.
New Angled Card preset selection defaults to Roundness 0 (geometric form),
while explicitly saved user roundness remains intact. No schema/enum migration
is needed. Both shader and pure-C reference implement the same clipped
segments and four arcs.

Tests must prove zero preserves the original shape, a rounded cut removes
the former diagonal vertices, the two square corners stay intact, both cuts
remain symmetric, the input image and border follow the rounded contour,
and existing bubble/other shapes do not regress. Actual OBS aesthetic
acceptance of the leaf-like appearance is still pending.

## D2.2 — Chat Bubble exterior-only leaf-tail contour (implementation)

The user's reported horizontal line in the Bubble is not adequately addressed
by a wider glow or a detached overlay. In the D1 formula, `min(body, triangle)`
is a valid **alpha occupancy union** but underestimates *interior distance*:
the covered bottom of the body and the covered triangle base can remain close
to zero, creating a phantom neon stripe inside the speech bubble.

D2.2 separates occupancy/sign from the **single visible exterior perimeter**.
Explicit body straight segments, four rounded corner arcs, the two exposed
tail sides and a tangent rounded leaf tip form one signed contour distance.
The covered body-bottom interval and triangle base are not distance candidates.
The same signed distance still drives source alpha, core, secondary rim, bloom
and ornaments. Reuse the existing Roundness and shape-detail controls; no
new setting key, enum ID, PNG overlay, dependency or per-frame CPU work.
A zero Roundness value retains a pointed geometric tail; positive Roundness
makes its tip softly rounded. Angled Card's independent selective leaf-chamfer
behavior remains unchanged.

This is an intentional appearance correction for shape ID 8, including
previously saved Bubble scenes. Other shape IDs and existing source X/Y,
pan/zoom and schema mappings remain untouched. The original SVG sample
remains a design reference, not an implemented runtime importer.

Required proof: pure-C distance checks establish no near-zero interior seam,
a still-covered tail, a real exterior bottom rail and tip shape; OpenGL/D3D11
test-only GPU readback additionally checks that the tail-base strip is
red source content without a magenta line, while the actual outside edge
still lights. Actual OBS dark/light captures and leaf-tail artistic acceptance
remain pending; so do SVG import, wider shape pack, safe-fit and G6.

## D2.2 GUI feedback — shared Roundness label

The 2026-09-27 OBS screenshots show the GUI still labels Roundness as
"Angled Card: diagonal cuts only" even when Chat Bubble is selected.
The same saved control also rounds the Bubble's leaf-tail tip after D2.2.
Use the compact shape-neutral label **Contour roundness** in English and
**Kelengkungan kontur** in Indonesian. Angled Card still rounds only its
diagonal endpoints, leaving two square corners sharp; rounded rectangle
uses corner roundness, and Bubble uses tail-tip curvature. No new key,
slider, schema version, or geometry change is involved. Visual acceptance
of the Bubble seam must use the exact post-PR30 build, not an earlier
screenshot/artifact.

## D2.3a — full-arc Angled Card refinement (user OBS feedback)

User OBS screenshots confirm the integrated Chat Bubble seam is visually clean on the new build. On Angled Card, however, Roundness 1.00 still shows a conspicuous straight diagonal between two small corner fillets. Increase the **geometry**, not blur: normalize the existing source-pixel radius by the smaller half-extent to recover the full 0..1 slider travel, and map it to a maximum radius `cut × (1 + 1/√2)`. At maximum, the two 45-degree arcs have **the same circle center**, join tangentially, and form one continuous 90-degree arc between the top and right rails (and the opposite bottom-left pair). Zero preserves the original chamfer; half slider creates intermediate curvature; upper-left and lower-right remain mathematically sharp. The same distance still drives actual source alpha and all neon layers. No extra property, preset ID, schema or scene migration is needed; existing saved Angled Card settings intentionally gain fuller curvature. CPU oracle and real OpenGL/D3D11 pixel fixtures are the code-level gates; visual acceptance remains pending actual OBS captures on user source.

## D2.3b — Authored Shape Pack v1: HUD Cut Panel + Squircle

The first incremental D2 library milestone appends stable shape IDs 10
(HUD Cut Panel) and 11 (Squircle), and additive built-in preset IDs 8/9.
Existing IDs, schema v2, scene assignments and old presets are unchanged.

HUD Cut Panel uses a ten-segment *concave exterior contour*: asymmetrical
top/right chamfers plus an intentionally stepped lower-left notch that
actually clips source video. A new SDF-aligned selective supporting rail
follows that contour rather than using old rectangle-only bracket geometry.
Squircle is a continuously curved superellipse, with exponent 4..5
controlled by the existing `shape_detail`; the implicit-function gradient
normalizes near-contour stroke width in source pixels. Both shapes feed the
same mask alpha, core, secondary rim and glow from one signed distance.
Shape detail now has shape-neutral EN/ID labels. No runtime image asset,
SVG parser, worker, new uniform or source readback.

Acceptance at code level: CPU oracle across aspect ratios, notch alpha
and finite distance; separate real OpenGL/D3D11 GPU tests for filled interiors,
transparent clipped corners and neon on concave/curved exterior. OBS
real-output aesthetic review, custom SVG import, Star/Heart/Rounded
Polygon and D3 safe-fit/motion/performance remain explicitly open.

## D2.3c — maintainer OBS audit and refinement

The 2026-09-27 user OBS captures are direct visual evidence: the angled maximum arc is accepted; the user manually changed **Mask shape** while **Apply design preset** remained *Custom/current settings*. That does not apply the curated HUD/Squircle light recipe and may retain an earlier segmented/ornament configuration. Explicitly select Preset 8/9 for signature lighting, or customize intentionally; do not silently overwrite the user's saved colors/segments when manually changing shape. HUD's two square staircase joins have been replaced with deliberately bevelled concave steps, and its opt-in Tech HUD preset adds two asymmetrical calibration traces. Squircle's near-horizontal/vertical signed distance uses exact axis intersection blended to the gradient distance at corners, to stabilize the luminous rail; the source shape and colored border remain one contour. Hide ineffective Roundness/Shape Detail/Polygon Sides native controls based on active shape while preserving persisted values. CPU+GPU regression and frontend screenshot on the revised build remain separate evidence gates; no blanket claim of image #9 parity.

## D2.4 — bounded local SVG path-to-SDF provider (preview candidate)

A file picker on the appended SVG shape ID 12 accepts **local files only**.
This initial secure subset deliberately supports a standalone `<svg viewBox>`
with one to eight self-closing, solid-fill `<path d=.../>` elements.
Commands: M/m, L/l, H/h, V/v, Q/q, C/c and Z/z. Evenodd or nonzero
fill rules, closed contours and multiple contours allow intentional holes.
No group, transform, stroke, style, clipPath, gradients, arc command,
filters, scripts, URL, DTD, XML entity, image, font or external resources.
Unsupported/invalid features are refused; this is NOT a general SVG viewer.
File size is limited to 64 KiB, flattened edge count to 512, viewBox width/
height to 10000 and number of paths to eight. Cubics and quadratics use a
bounded 24/16-step tessellation. All artwork must be original/appropriately
licensed; never load or redistribute the proposal's portrait.

A source-geometry-size dirty event rasterizes a 256² signed physical-pixel
distance field using aspect-preserving **contain**, then uploads one R32F
texture inside the libobs graphics context. File I/O occurs only on path
selection/reload, rasterization and upload only on asset/size changes.
No CPU video-frame processing and no synchronous GPU readback in production.
One canonical sampled signed field controls webcam transparency and the same
luminous line/core/glow. Rotation and manual mask/subject X/Y remain separate.
No interpolated path data is constructed by untrusted shader source.

**Failure policy:** invalid/missing assets immediately invalidate their
previous cached texture; the SVG shape renders transparent, never silently
revealing an unmasked facecam. Changing to a built-in shape still works;
the saved file path is not silently discarded. A dedicated Reload button
retries an edited file at the same path. Graphics resources are released
on path change/destroy and replaced on resize. Do not claim asset decode
fail-closed if the entire effect cannot be loaded: that pre-existing shader
failure path needs an independent lifecycle audit.

Automated exit: malformed/extreme/hostile paths, evenodd hole and cubic
cases in pure C, direct OpenGL/D3D11 SVG alpha/border and empty-asset pixels,
and reload/resize resource smoke. Final OBS source/filter-chain save/restart,
light/dark 320×180 capture, complex-path fidelity and hardware timings are
still pending G4-V/G5/G6; original advanced SVG features remain future work.

## D2.4 follow-up — SVG viewBox occupancy boundary

Audit of the first SVG preview found that clamped SDF sampling could repeat a **negative** edge texel outside a path that touches the viewBox. Adding the outside distance to that negative texel caused a narrow band of unintended source alpha outside the authored bounds. The corrected shader keeps the sampled signed distance inside the viewBox but clamps negative texture distance to zero before extrapolating **outside** it. This preserves legitimate exterior neon/bloom while forbidding mask occupancy beyond the declared local canvas. Real OpenGL and D3D11 pixel fixtures use an all-the-way-to-viewBox filled shape, a subpixel translated frame and test-only GPU readback to catch the leak. This is a preview correctness fix, not completed complex-SVG support, real-OBS visual parity or D3 safe-fit.

## D2.4 safety follow-up — missing-effect fail-closed behavior

An unavailable main shader (missing file, compile failure, or missing uniform)
previously invoked `obs_source_skip_video_filter`, exposing the unmasked
upstream video despite the already transparent invalid-SVG path. The render
callback now emits no draw when its effect or target is absent or source
dimensions are zero. This is a fail-closed render policy, not a replacement
shader or recovery from driver/device loss. Normal rendering, IDs, settings,
subject framing and SVG cache are unchanged. A structural regression forbids
unmasked bypass from the render callback. Verify real OBS missing-effect,
save/restart and filter-chain behavior separately under G4-V/G5; code and GPU
CI cannot substitute for frontend proof.

## D3a — opt-in source-canvas safe-fit (code preview)

The first D3 vertical slice deliberately does **not** claim expanded OBS
filter output. The libobs capture helper renders the target at its existing
base dimensions; increasing the final sprite width/height alone would not
translate the target origin or fix filter-chain/scene alignment. D3a instead
offers an explicitly opt-in `safe_fit` checkbox, default **off** for all old
scenes. It computes a conservative luminous envelope from shader core, feather,
double rails, art/brackets/tail and exp2 bloom cutoff, plus a rotated silhouette
bounding rectangle. If the authored geometry would clip, only the mask
half-extents shrink uniformly; mask center and source subject pan/zoom/UV,
saved width/height and source dimensions are unchanged. SVG raster/tick and
render use the same effective half-size. If an offset leaves no room even for
the envelope, emit transparent output and a one-shot warning instead of
silently repositioning the portrait. This is an opt-in **inside-existing-canvas**
mode, not a general solution to arbitrary off-source translations.

Gates: pure-C fit/rotation/impossible-position and schema/locale host contracts,
then Linux/Windows native, OpenGL/D3D11 direct-pixel CI. Actual OBS screenshots
at off-center framing, source resize, SVG reload, filter-chain order and the
320×180 downscale still require G4-V/G5. Expanded output (dimension callbacks,
source-origin translation, exact per-shape envelope and downstream transforms)
remains D3b, and hardware timing/soak remains G6. No premature #24/#26 closure.

## D3b — opt-in expanded OBS filter output (code preview)

The pinned OBS 31.1.1 libobs `get_base_width/height` uses enabled filter
`get_width/get_height` callbacks, while `obs_source_process_filter_begin`
captures the target at its own base dimensions and `tech_end` draws the
final sprite using the requested dimensions. D3b uses a **shared pure-C
bounds calculation** across dimension callbacks, SVG tick and video render:
compute a conservative rotated silhouette AABB, include the D3a light
support envelope, and ceil separate left/top/right/bottom padding. Output
= input + these pads; input is mapped to expanded pixel (L,T) in shader,
while subject sampling stays input-relative and outside-source UV is
transparent, not Clamp-smear. Source mask width/height, X/Y, subject pan/zoom,
SVG raster resolution, enum IDs and schema v2 are unchanged. Existing scenes
have `expand_canvas=false` by default. When both checkboxes are selected,
expanded output takes precedence over shrinking safe-fit.

Explicit limits: <=512 extra pixels per side and <=8192 total pixels per
axis. Impossible offsets/dimensions return original callback dimensions but
render no draw (transparent) with one warning; never make a half-cropped
approximation or reveal an unmasked feed. The renderer logs actual pad L/T/R/B
on changes. A **normal OBS filter has no negative scene origin callback**:
expanded content appears shifted by (+L,+T) within the reported output;
for unchanged scene-world placement compensate the scene item's position
by (-L,-T) in source-local units, accounting for scene scaling/rotation.
Do not silently mutate scenes or claim their transforms are preserved.
A source used by multiple scenes or filter chains needs manual review.

Tests: canonical bounds/rotation/offset/limits/legacy behavior and shader
input-origin pixel probes (OpenGL and D3D11), including a luminous rim in the
new left pad, no source pixel smearing there, unchanged red source interior,
and transparent far-output pixel. This is partial G4 GPU proof;
real OBS scene item anchor, filter order, source resize/restart, SVG reload,
320x180 dark/light captures, render lag and soak are pending G4-V/G5/G6.
Issue #24/#26 stay open. No claim of final automatic scene-origin parity.

## D3 verification ledger and frontend acceptance boundary

D3a/D3b geometry and shader code are present on main. The expanded output
is bounded and opt-in, uses separate L/T/R/B padding and a source-relative
input origin, and preserves old scenes when disabled. The D3 test matrix
covers 320x180, 640x480, square, 1080x1920, 1920x1080 and 3840x2160,
combined X/Y offsets and rotations; unsupported positions reject without
changing reported original dimensions. Direct OpenGL/D3D11 pixel fixtures
exercise the translated origin and transparent out-of-source samples.

**D3 is not yet frontend-accepted solely by CI.** The remaining evidence is
an actual OBS run using the artifact from the exact tested commit. Capture:

1. Two matched source scenes with `expand_canvas` off and on, at 320x180
   and source resolution. Verify input landmarks remain undistorted, halo and
   tail do not clip, and output alpha outside the envelope is transparent.
2. Off-center X-only, Y-only, combined shifts and rotated shape; record the
   logged L/T/R/B. For exact old scene-world position, compensate scene-item
   transform for the positive L/T content offset. Log the compensated
   position; ordinary OBS filters cannot report a negative scene origin.
3. Filter order before/after Crop, Color Correction and Chroma Key; source
   resize, source disconnect/reconnect, scene duplicate, OBS save/restart,
   SVG reload and missing/invalid SVG. Inspect alpha and source references.
4. Static/Flow/Pulse, dark/light/moving backgrounds, plus GPU timing and
   render lag on the declared Windows hardware. Eight-hour OBS soak is G6.

Keep #24/#26 open until screenshots/clips and hardware records exist. D4A
can proceed independently, but later new tail/body parameters must preserve
the D3 bounded silhouette and re-run the matrix.

## D4A v1 — parametric Bubble and context-aware OBS controls

Bubble ID 8 gains four nonnegative body-side insets, two bottom-tail
shoulders, independent tail tip X and tail depth, alongside the existing
Roundness. All parameters persist as additive schema-v2 keys. The initial
zero-inset/legacy-anchor values reproduce the old exposed contour exactly;
legacy scenes with no new tail-depth key migrate from their authored
`shape_detail` and pin that value once. Existing preset selection explicitly
resets designer values, while switching Shape or hiding a UI control never
silently deletes the custom geometry. A shared CPU oracle and shader retain
ONE exterior contour: no neon along the hidden triangle base/body bottom.
The new points stay within the original half-extents, preserving D3 padding
conservatism. No arbitrary Bézier/drag point editor is claimed yet.

The OBS Properties panel now shows Bubble Body/Tail groups only for Bubble,
hides generic Shape Detail there (dedicated Tail Depth replaces it), and
resolves border, ornament, glow, animation and expand/safe-fit visibility
from the current selection. Hidden values remain persisted; no shader
feature is implicitly disabled by hiding a slider. Shape ID and schema stay
unchanged. Pure CPU legacy/custom/extreme geometry, shader binding and GPU
custom-body/tail pixels require CI; actual OBS visual review at 320x180,
small sizes, save/restart and live dropdown transitions remains G4-V/G5.
D4A v2 may add a separate point-drag editor after this parametric model is
accepted.

## D4A v2 — corrected freeform Bubble + halo stabilization

User OBS testing rejected the v1 "side inset" model: it only cropped four
parallel rails and could not produce an intentionally skewed/peyang Bubble.
Schema 3 therefore migrates that preview representation once to a true
four-corner convex body. TL/TR/BR/BL each have independent normalized X/Y
controls. Their slider ranges keep each corner in a disjoint quadrant, so
every possible UI combination remains convex and non-self-intersecting.
The tail start/end/tip are fractions of the actual BL->BR line, and the tip
travels along its outward pixel-space normal. Tail depth is bounded against
the original half-size rectangle; D3's existing conservative rectangle
therefore remains valid even for a slanted bottom. One exposed perimeter is
still authoritative for mask alpha, core, tracks and glow. Body roundness is
a tangent circular fillet calculated from the two incident freeform sides.

Schema-2 D4A inset values migrate to equivalent axis-aligned corner points
where possible; pre-D4A scenes are the zero-inset special case. Named preset
application resets to the canonical rectangular Bubble, while manual shape
switching preserves authored points. The obsolete inset controls are no
longer exposed. Point-drag editing is still a future frontend enhancement;
these eight normalized point values are the intended backing model.

The gray/pixelated fringe reported in OBS Filters preview is treated as a
plugin visual defect, not an OBS decoration. Broad bloom now preserves neon
hue instead of whitening low-alpha pixels, receives a smooth finite 2.30R-
2.50R support gate aligned with D3's 2.5R envelope, and emits exact transparent
black once total alpha is negligible. OpenGL/D3D11 fixtures add a far-envelope
RGBA==0 guard and chromatic near-bloom check. Real OBS preview/full-canvas
screenshots are still required to close visual acceptance.

## D4A visual checkpoint and D4B Tech HUD Advanced

Maintainer OBS review on 2026-09-28 accepted the revised four-corner Bubble
interaction as the intended freeform behavior ("user bisa freeform"). This is
useful G4-V product evidence for the geometry/UX direction, but it does not by
itself close the full D3/D4 lifecycle, performance, 320x180 or halo matrix.
The reported preview image showed a deliberately skewed Bubble with its tail
remaining integrated; no claim is made here about unrecorded hardware data.

D4B appends **shape ID 13, Tech HUD Advanced**, while preserving every prior
shape ID and schema-3 setting. Its source mask is a six-edge rectangle-family
silhouette: top-left and bottom-right are true diagonal cut corners, while
top-right and bottom-left stay square. Therefore the webcam coverage itself
matches the HUD silhouette rather than remaining a plain rectangle.

With the existing Tech HUD ornament selected, D4B adds two exterior luminous
L-brackets anchored to the square corners and two shorter companion traces
parallel to the real cut edges. These accents never alter source alpha. The
ordinary neon core/glow still follows the mask SDF, so mask border and overlay
are separate but geometrically coordinated. Preset ID 5 is retained and now
selects shape 13 when deliberately re-applied; already-saved scenes continue
to load their stored shape. Existing HUD Cut Panel ID 10 remains unchanged.
Shape Detail controls the two chamfers and is context-visible for this shape.
D3's ornament envelope remains conservative for the new bracket reach.

Acceptance requires CPU contour probes (cut/square corners/detail), OpenGL
and D3D11 pixels for source clipping plus both outer brackets/diagonal trace,
and an actual OBS comparison to the approved Tech HUD reference at normal and
320x180 size before visual completion is claimed.

## D4B.1 — visible ornament scale correction

Real OBS testing with the Tech HUD preset active proved the state path was
already correct (shape 13, ornament recipe Tech HUD, intensity 0.91, gap 2,
HUD border style and glow all visible in Properties), yet the ornament was
visually absent. Root cause: D4B's line-distance thresholds were fixed around
1.2–1.55 **source pixels**. That is generous in the 64x64 GPU fixture but
collapses to sub-pixel weight when a 720p/1080p camera is scaled into the OBS
Filters preview.

D4B.1 derives external L-bracket thickness from border_width (half-stroke
max(2.2px, 1.15x border)), the diagonal companion trace from 0.72x border,
and its separation from max(4px, 1.30x border)+art_gap. Arm length also scales
with border/mask instead of a tiny fixed cap. Glow support is measured from
the edge of those strokes. The D3 Tech HUD envelope mirrors the same gap,
stroke and glow formulas.

CI now includes a separate 320x180 GPU fixture. It requires substantial alpha
more than three source pixels off the bracket centerline; the old 1.5px
implementation cannot satisfy that test. The existing 64x64 fixtures remain
for exact geometry. Actual OBS screenshot comparison remains the final visual
gate.

## D4B.2 — recognizable Tech HUD assembly

A second real OBS screenshot showed D4B.1 technically visible but still
visually far below the reference: only tiny top-right/bottom-left corner ticks
read at preview scale. D4B.2 therefore changes the *ornament grammar*, not just
stroke thickness. Tech HUD now has two long square-corner L assemblies, two
three-piece shells around the actual TL/BR chamfers, four short inner rails
near the image edge, and a sparse broken contour-support rail. Arm lengths are
proportional to mask half-extents with bounded floors/caps; major/minor stroke
weights remain derived from border width and glow.

The 320x180 GPU acceptance fixture now samples distant points on both long
brackets, both cut-corner shells and an inner rail while requiring the portrait
center to stay quiet. This specifically rejects the prior "two tiny ticks"
appearance even though those ticks were technically rendered. Actual OBS
comparison to the supplied Tech HUD reference remains the visual gate.

## D4C — Game UI

D4C appends shape ID 14 and preset ID 10 without renumbering any existing
shape/preset. The mask is a restrained symmetric eight-edge gaming panel with
small chamfers on all four source corners. Its ornament recipe is distinct
from Tech HUD: four mirrored, large exterior L brackets plus a complete
secondary inner rail following the exact signed mask contour. Small inner
corner nodes provide a gaming accent while the portrait center stays clear.

All outer dimensions are proportional to mask size and border width; no
1-pixel fixed ornament is accepted after the D4B lesson. The initial built-in
Game UI preset is static neon green so its identity is visible without motion.
The D3 envelope accounts for its bracket stroke/gap/glow. CPU geometry,
config/preset/art contracts and 320x180 OpenGL/D3D11 visual-scale fixtures are
required before merge; actual OBS screenshot remains the human visual gate.

## D4D — Animated Rainbow Gradient

D4D adds color mode as a rendering concern independent from shape, ornament
and border motion. Persisted values are append-only: Solid=0, Dual=1 and
Rainbow=2. Existing scenes resolve to **Dual**, so their pre-D4D two-color
formula remains the compatibility path; applying any existing named preset
also deliberately restores Dual rather than carrying a previous Rainbow
customization into another design. No shape or preset ID is renumbered and
the additive settings do not require a scene-schema bump.

Rainbow is generated analytically on the GPU from a bounded hue phase; there
is no texture animation, CPU video processing or GPU readback in production.
The controls are Rainbow Speed (0..5), Saturation (0..1), Hue Offset
(0..1 turn) and Gradient Spread (0.25..3 spectrum cycles). The existing
Overall Glow Intensity remains the glow control rather than introducing a
duplicate slider. Core, fine line, mid glow and outer bloom derive from the
same local rainbow hue. Rainbow speed has its own bounded double-precision
host accumulator so changing Flow/Pulse speed does not unexpectedly change
the spectrum drift.

Static remains deterministic: Rainbow + Static freezes the hue phase, and
speed zero also freezes it. Rounded rectangle/rectangle use actual perimeter
length through `roundedContourTurn`; circle uses its exact angular perimeter
parameter and ellipse uses normalized parametric angle. Other authored and
polygon masks receive a continuous normalized contour proxy in this D4D slice
rather than duplicating every shape's geometry in a second color-only path.
D4E actual-OBS visual review decides which of those shapes need dedicated
perimeter-length routing.

The OBS property surface is contextual: Primary/Secondary stay visible for
Solid/Dual as applicable, while the Rainbow group appears only in Rainbow
mode and hides Rainbow Speed in Static. Hidden values are preserved. Pure-C
config/motion tests cover default compatibility, validation, preset reset,
static/zero-speed behavior and simulated eight-hour phase continuity. The
real libobs OpenGL/D3D11 shader fixture additionally renders a 320x180
four-region rainbow frame so a shader that collapses back to one hue cannot
pass CI. This is still partial G4; D4E requires actual OBS dark/light captures,
motion clips and preset-by-preset aesthetic polish before visual completion.


## D4B.3 / D4C.1 — visual-parity cleanup after real OBS audit

Actual OBS screenshots on 2026-09-29 rejected D4B.2 and D4C as visually
complete. Tech HUD was noisy: an oversized chamfer, the generic HUD rail,
floating inner fragments and a support trace competed with each other. Game UI
still read as a full neon box with tiny corner accents rather than the supplied
four-bracket gaming frame.

D4B.3 keeps IDs stable, reduces the Tech HUD TL/BR cut to 1.10x detail with a
20% cap, suppresses the generic Double/HUD rail for authored overlays, and
renders only four coherent modules: strong TR/BL L anchors plus compact
three-piece shells on the real TL/BR cuts. Floating inner rails and the broken
support contour are removed. Preset 5 uses smaller cuts, wider separation and
restrained bloom.

D4C.1 makes shape 14 source coverage a clean rectangle and hides Shape Detail
for Game UI. Its identity now comes from four large outer L brackets, separated
from the image, plus one thin continuous inner rail. Corner nodes are removed.
Preset 10 narrows the base border, increases bracket separation and reduces
bloom so the outer modules dominate.

The 320x180 GPU gates add negative composition checks: the former Tech inner-
noise region must remain transparent with glow disabled; Game UI must illuminate
both arms of every outer bracket and all four inner-rail sides while the center
remains clear. These are automated composition invariants; a fresh OBS capture
is still the human visual acceptance gate.

## D4E — Artistic Thickness Rework

Real OBS review after D4B.3/D4C.1 found the composition cleaner but still too
line-like: the artistic identity should come from **very thick L modules**,
while the inner/base frame stays thin. D4E changes the underlying geometry
rather than merely increasing alpha.

Game UI's four outer corners are now unions of filled horizontal/vertical
rounded-box bars, not distance-to-line strokes. The default full bar is about
9–12 source pixels at preview-scale presets, while the continuous inset rail
is roughly 1.3–1.8 pixels (>5:1 intended mass ratio). The ordinary mask border
is narrowed and attenuated so it reads as a secondary frame.

Tech HUD gets an asymmetric authored mass hierarchy matching the reference
language: a dominant filled top-left L, medium filled top-right/bottom-left L
modules, and three short bottom-right diagonal blades. The mask border is
again the thin subordinate frame. Preset 5 and preset 10 use narrower base
borders, separated ornaments and restrained bloom. D3's envelope mirrors the
new half-thickness/gap formulas.

The 320x180 GPU gates now test *thickness*, not merely presence: outer bars
must retain substantial alpha four pixels away from their centerline; Game
UI's inner rail must already be quiet two pixels away; outer mass must exceed
the inner/base frame by a fixed alpha margin. This prevents a regression back
to decorative hairlines. Actual OBS screenshots remain the visual sign-off.

## D4F — Ornament & Border Presence Controls

Real OBS canvas testing after D4E showed a remaining product-level issue: scene-item downscaling makes even correct source-space neon look weak. D4F therefore stops deriving authored ornament mass from the base border and makes the three visual layers independently controllable.

The OBS UI now exposes a base-border range of 0.5..64 px. For Tech HUD and Game UI only, contextual controls expose outer ornament width (1..64 px), horizontal/vertical L-arm lengths (8..240 px), and the existing ornament gap now spans 0..96 px. Game UI additionally exposes inner-rail width (0.5..12 px). Hidden controls retain their values.

Art gap semantics are corrected: it is the actual empty pixel distance from the base frame edge to the OUTER bar's inner edge. Bar center is placed at half-size + gap + width/2, so changing gap from 16 to 32 really moves the ornament outward by 16 source pixels instead of being partly cancelled by stroke math.

Tech HUD's default grammar is strict: only the top-right and bottom-left corners receive dominant outer L bars. Top-left/bottom-right may carry only thin diagonal technical accents. Game UI retains four symmetric outer Ls plus one thin inner rail. Presets 5/10 use 5px base borders for better canvas presence, with Tech HUD 18px outer bars / 24px true gap and Game UI 16px outer bars / 14px gap. D3 safe-fit/expanded-output uses the exact gap + full ornament width + glow support reach.

These controls compensate for OBS scene-transform downscaling without trying to infer scene scale inside a source filter. Visual acceptance still requires real OBS canvas screenshots at the user's intended facecam size.

## D4G — Corner Join Integrity

Real OBS screenshots with Game UI outer width 37.5 px and Tech HUD width
56.5 px exposed a geometry defect hidden by normal-width tests. D4F built each
L from two centered bars. At the elbow those bars overlapped by only one
quarter of the requested thickness, leaving the outer elbow quadrant empty.
The missing quadrant became an obvious stepped/broken corner as width grew.

D4G replaces that construction with a dedicated `connectedLCornerDistance`
primitive. Its anchor is the **actual outermost square corner**; horizontal and
vertical bars each run inward by one full thickness and an explicit full
thickness elbow square is unioned with both bars. This keeps the joint
watertight for large widths on both OpenGL and D3D11. Tech HUD continues to
render dominant Ls only at top-right and bottom-left; Game UI mirrors one
canonical connected L into all four corners.

A safety clamp preserves at least a 12 px arm stub when a pathological width
exceeds the available arm length. Normal values are untouched.

There is a separate physical limit: an outer L cannot remain visible after its
elbow leaves the source render target. The contextual UI now states this
explicitly, and the D3 checkbox is renamed to "Expand output for glow / outer
ornaments". Users choosing very large gap+width values should enable expansion
to avoid source-bound clipping. D3's existing envelope already includes
`gap + full ornament width + glow support`.

## D4H — canvas-aware outer ornament fit

Real OBS evidence after D4G shows the connected-L geometry is correct, but a
large top ornament can still be clipped by the source render-target boundary
when the frame sits closer to the top than the bottom. This is not an elbow
defect: for example Game UI gap 30 + width 38.5 needs roughly 68.5 px of
outward support before glow, while the screenshot's available top margin is
smaller.

The existing D3 safe-fit path already solves this without scene-space
translation: it uniformly reduces only mask half-extents until the complete
light/ornament envelope lies inside the source. D4H makes the curated Tech HUD
and Game UI preset operation opt into safe-fit automatically when expanded
output is not already enabled. This happens only when the user applies the
preset; saved/custom scenes are not silently changed. Once enabled, later
gap/width edits continuously recompute fit, so an aggressive L cannot be
silently cropped.

Users who need the authored mask size unchanged can instead enable expanded
output. The UI now describes both choices explicitly: safe-fit trades mask
size for fixed source dimensions, while expansion preserves mask size but
adds output padding (with the documented scene-item +left/+top shift).

Pure-C regression cases use the real screenshot-scale values (Game UI
30/38.5/22.5 and Tech HUD 31/46.5/19) and require the resulting
half-size + D4F/D4G ornament envelope to remain within a 640x480 source.

## D4I — automatic authored-ornament fit invariant

A second real OBS screenshot showed D4H did not solve an already-custom Game
UI scene: the top L was still clipped. Root cause: D4H only enabled `safe_fit`
inside the **preset callback**. Existing/custom scenes that did not re-apply a
preset kept `safe_fit=false`, so large gap/width edits continued to render
against the original source bounds and the top horizontal arm disappeared.

D4I moves the protection into `nm_safe_fit_calculate` itself. Whenever a
visible dedicated Tech HUD or Game UI outer-ornament recipe is active and
expanded output is off, the calculator automatically performs in-source fit.
It only shrinks when the real envelope would otherwise overflow, so ordinary
settings are unchanged. This applies to old scenes, custom scenes and live
slider edits without requiring a preset re-apply.

The legacy Safe Fit checkbox keeps its original semantics for all other shapes.
For dedicated outer-L recipes it is hidden because clipping protection is now a
visual invariant, not an optional preset side effect. Expanded output still
takes precedence and preserves the authored mask size by adding padding.

Regression tests now use the user's large Game UI / Tech HUD screenshot values
with `safe_fit=false` and require automatic shrink, while a separate expansion
case requires scale 1.0 plus real top/right padding.
