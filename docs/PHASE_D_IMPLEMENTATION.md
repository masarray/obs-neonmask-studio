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

## D2: diverse geometry and bounded SVG provider (not yet implemented)

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
   in assets/designs are **authoring samples**, not yet runtime-loadable.

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
