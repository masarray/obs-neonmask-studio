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
