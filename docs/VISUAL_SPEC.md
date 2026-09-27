# Visual direction and acceptance

Reference: user-supplied “Gambar ChatGPT 26 Sep 2026, 14.52.27.png”, inspected on 2026-09-26. The image is a concept board, not a screenshot of implemented output. It depicts masked facecams with cyan/magenta, violet, emerald and amber neon, organized into shape, animation, stream-style and customization families. This written mapping remains usable without the external attachment. Do not treat faces, photography or board typography as distributable product assets.

## Binding reference and 2026-09-26 feedback

**Target:** the user's concept board (image #9 in the September 26 OBS test
feedback), not the current plain-outline appearance. Its facecam designs are
visual *recipes*, not literal pixel-identical screenshots: adapt to owned test
imagery, different webcam aspect ratios and real OBS compositing without
redistributing the board's portrait. The design direction must remain
recognizable in actual OBS recordings, not only in documentation/mockups.

**Observed current state:** user OBS screenshots #1–#8 demonstrate the filter
appearing in Effect Filters and rendering rounded rectangle, circle, ellipse,
hexagon and diamond plus Reactor/Emerald/Ember examples. Compared with image
#9, there is no independent face X/Y framing, the halo reads as a thin/soft
outline rather than a graduated luminous bloom, the second track/ornaments
are visually subdued, and motion lacks a pronounced traveling highlight.
This is user feedback on the pictured artifact; exact OBS version, GPU,
scene settings and artifact SHA still need to be recorded for formal G4/G5.

**Acceptance rule:** every claimed *shipped* visual family must have a real
OBS output still plus a short motion clip (when applicable), the named
settings recipe, a dark and light backdrop, the 320 × 180 viewing case and
a recorded human review against the intended reference family. A shape with
only a single ordinary colored outline is not a completed premium design.
Any missing family must be called *planned*, not represented as shipped.

## Artistic direction: authored identity, not generic outlines

Maintainer clarification: **beauty, futuristic feel, artistry and modern
fancy linework** are the product. This is a geometric mask + frame;
there is **no background-removal or portrait-segmentation requirement**.
Keep the original video visible inside the mask and transparent outside
the geometric silhouette. Manual subject X/Y pan/zoom is simply a framing
transform, never a cutout detector.

Each advertised design family needs a coherent visual composition with
a deliberate silhouette and a recognizable **signature motif**. Use
controlled asymmetry, arc-length-placed highlights, selective gaps,
corner geometry, tapered or varying-width accents and rhythm between
bright core and quieter supporting tracks where appropriate. A family
must remain recognizable in static mode and at 320 × 180. Keep a
continuous or intentionally interrupted visual anchor so a luminous
frame does not look like a random collection of disconnected lines.
Decorations support the frame without obscuring the subject.

An ordinary uniform neon stroke with a broad blur, plus changed color,
glow intensity or preset name, is **not** a new artistic family. An
identical frame with randomly added sparks is also insufficient.
For each family, write down its actual contour/ornament grammar and
motion signature; compare actual OBS captures with the corresponding
concept-board tile before calling it complete.

Examples of intended motifs:
- **Cyber Rounded / Neon Flow:** precise corner cuts, two-tone core,
  selectively layered companion track and localized moving flare.
- **Reactor Ring:** circular double-track rhythm, segmented luminous arcs,
  restrained rotating hot spot and a deliberate seam.
- **Tech HUD / Game UI:** anchored corner brackets, calibrated gaps,
  small futuristic detail accents and clear negative space.
- **Streamer / Chat:** stylized silhouette or explicit decorative tail,
  purposeful dots and graceful corner transitions.
- **Electric:** sparse bounded edge arcs, opt-in and visually composed,
  not uncontrolled whole-frame noise.
- **Emerald Hex / Diamond / Polygon:** crisp balanced joints and
  contour-aware color/light placement; no stretched strokes.

Do not introduce background removal, auto-face tracking or segmentation
into the reference-to-delivery matrix. Image, gradient, SVG and OBS-source
mask providers remain separate **mask-geometry capabilities**, not
background removal, and remain later work after the authored procedural
frame collection passes visual acceptance.

## Visual rules

The visible face is the subject; the luminous frame supports it. Keep the center clear, avoid tinting skin, use a sharp core with a softer exterior halo, and keep rim spacing intentional. Glow should remain attractive over black, white and moving game footage. No opaque black rectangle around transparent content.

Small facecam use matters: judge at 320 × 180 as well as full source resolution. Thin forms and restrained movement should survive downscaling. A design that only looks good in a large marketing image is not accepted.

Separate the mask silhouette from decorative accents. Chat tails, HUD corners and electric sparks must have explicit semantics: silhouette-changing shapes affect coverage; decorative accents do not punch holes in the facecam. Use the same contour anchor data for all decorative placement.

## Premium frame anatomy (shared renderer contract)

1. **Base image / silhouette:** preserved facecam color inside one canonical
   mask; no unintended RGB fringe outside it. The user's face can be positioned
   independently of the shape with no nonuniform stretch.
2. **Fine luminous core:** crisp near-white/colored highlight at the intended
   contour, readable at small facecam sizes, not a uniform blurred ribbon.
3. **Mid glow and outer bloom:** at least two visually distinguishable light
   scales around the core; restrained inward spill, stronger falloff outside,
   controllable cutoff and no unintentional canvas clipping in safe-fit mode.
   Analytic or measured GPU implementation is acceptable; the look, not an
   artificial single-pass rule, is the acceptance criterion.
4. **Secondary graphic track:** configurable narrow inner/outer rim, dashes,
   ring or corner accent anchored to the same distance/contour geometry.
   Features do not become disconnected when size, position or aspect changes.
5. **Motion identity:** static keeps a recognizable premium form. Flow has a
   clear traveling hot spot on a stable base rim; pulse modulates bloom without
   switching off the outline; ring rotation follows the contour. Speed zero
   freezes all phases; reduced-motion variant remains polished.
6. **Color hierarchy:** two editable base colors; optional ordered contour
   gradients later. Keep skin/background inside the silhouette unchanged
   except at intentionally composited contour pixels.

The image #9 examples are design targets across the four reference families,
not evidence of implementation. A high halo slider alone, or an increased
global brightness that washes out the image, does not meet this anatomy.

## Reframing and fit visual behavior

Phase-A code exposes **mask center X/Y** and **subject pan X/Y** separately, plus independent mask width/height and uniform zoom. These remain pending actual OBS acceptance until the corresponding artifact is tested. The mask
coordinates control the shape, rim, glow and ornaments together; subject
pan changes where captured webcam pixels appear *within* the mask without
moving the frame itself. Subject zoom uses a uniform scale and does not
stretch faces. Independent shape width/height handles silhouette dimensions,
with circle remaining truly circular unless the user selects ellipse.
Default values reproduce the existing centered image. Reset to center must
be immediate and predictable. Outside-source sampling must be transparent
rather than clamped edge smear. Safe-fit must account for translations,
glow support, stroke and ornaments, and must never silently undo user
framing or move the face to satisfy effect padding.

Check face alignment using an **owned off-center portrait/synthetic subject**
and a movable landmark, not only a centrally posed reference image.
Verify X-only, Y-only, combined shift, zoom, aspect change, source resize,
preset edit and restart. OBS scene/source transform stays independent:
filter pixel controls are source-relative, not screen/canvas coordinates.

## Reference-to-delivery mapping

| Reference example | Design recipe | Baseline and planned gap |
| --- | --- | --- |
| Rounded rectangle / rectangle | Continuous thin core, softly rounded or sharp corners | Both shape IDs implemented in Phase A; actual OBS precision/reference capture pending |
| Circle / oval | Consistent stroke around circular/elliptic facecam | Both implemented; label aligned to proposal; actual OBS ellipse precision pending |
| Hexagon / diamond | Crisp sci-fi silhouette with stable corner joins | Exists approximately; validate corners in M2 |
| Triangle / polygon | Parameterized sides with optional rounded joins | Triangle + regular polygon (5–12 sides) implemented in Phase A; rounded joins/visual parity still pending |
| Neon Flow | Bright traveling accent over dim continuous rim | Angular preview exists; perimeter travel in M2 |
| Rotating Ring | Circular segmented track with coherent rotation | Formal preset and seam validation in M3 |
| Pulse Glow | Slow breathing halo with visible stable core | Pulse exists; restrained amplitude and long-session checks in M2 |
| Electric | Sparse bounded arcs just outside the frame | New, opt-in M3; no full-screen flash |
| Streamer | Rounded rim with small corner/status dots | New ornament recipe in M3 |
| Chat Bubble | Rounded body with tail and small dots | New M3; declare tail as silhouette or decoration |
| Tech HUD / Game UI | Corner brackets, gaps, secondary tracks | Basic angular HUD exists; contour-aware layout in M3 |
| Dual Color / Gradient Rainbow | Two colors or bounded color stops along contour | Two-color exists; rainbow optional M3 |
| Multiple Layers | Main core plus one or two secondary offset rims | Double rim exists; explicit capped layer model in M3 |
| Dashed Segments | Even visible segments and consistent gaps | Angular segmentation exists; arc-length placement in M2 |
| Glow / Speed / Thickness | Independent controls with safe envelopes | Controls exist; validated fit and continuous motion in M2 |

## Mandatory next-preview comparison

| Actual screenshots #1–#8 | Reference-board requirement | Next evidence |
| --- | --- | --- |
| Person cannot be centered within mask independently | Flexible X/Y subject positioning with independent mask placement | Off-center fixture moved into frame without changing contour |
| Single rim dominates; exterior bloom is comparatively faint | Sharp luminous core, visible middle glow, wider soft bloom | Dark/light OBS captures at default and boosted glow, both full size and 320 × 180 |
| Existing Cyber Double Rim/HUD visuals are understated | Distinct supporting tracks/corners; not just a preset label | Actual capture demonstrates secondary element and alignment on multiple shapes |
| Flow/pulse hard to distinguish in stills | Visible but controlled dynamic travel/pulse | Short OBS clips at fixed speeds plus static/reduced-motion baseline |
| Diamond/circle naturally cut subject where shape overlaps | Subject pan/zoom separate from mask; transparency outside silhouette | Reframe subject without stretching or revealing out-of-source pixels |

Aesthetic acceptance is a documented visual comparison, not a fabricated
numeric "premium score". The [QUALITY_GATES.md](QUALITY_GATES.md) pixel,
performance and reliability requirements still apply.

## Launch preset collection

Keep four existing preset IDs stable: Cyber Rounded, Reactor Ring, Emerald Hex and Ember Frame. Refine their output with migration awareness; do not overwrite saved custom values. Add presets only after the underlying renderer passes its gate.

Target collection: Cyber Flow (cyan/magenta), Reactor Ring (blue/violet), Emerald Hex (green/cyan), Ember Minimal (amber), Tech HUD (cyan), Streamer Bubble (magenta), and Electric Violet (opt-in high detail). Each must have a static or reduced-motion form. Cap the initial renderer at three luminous layers and 32 discrete ornaments; reject or clamp imports consistently. These are proposed implementation caps, not existing controls.

## Visual acceptance procedure

Use owned/synthetic fixtures, not redistributed reference portraits. Include opaque color bars, a transparent gradient, soft hair-like edges, a checkerboard and a moving clip. Capture the actual final OBS composition and an alpha-capable intermediate test output.

Evaluate every supported shape and style at minimum/default/maximum valid settings; test small source dimensions, 16:9, 4:3, 1:1 and portrait aspect ratios. Composite against dark, light and saturated backgrounds. Check crop before/after filter, source transforms, source resize and nested scenes.

Quantitative targets at 1:1 source pixels:
- Mask edge and intended core centerline differ by no more than 1 pixel on reference contours.
- Nominal stroke width variation away from declared joins is at most 10% or 1 pixel, whichever is larger.
- Safe-fit outer canvas alpha is at most 1/255 for the supported effect envelope.
- No NaN/Inf pixel output; fully transparent fixtures produce no unwanted RGB fringe after composition.
- Static mode has identical output across sampled times for identical inputs.
- Flow crosses its seam without a visible jump; fixed-time snapshots are reproducible.

These are proposed gates; establish reference captures per supported backend before claiming a pass. Antialiasing differences need a documented tolerance, not unconditional pixel identity across drivers.

Human review also checks visual balance, skin-color preservation, whether the core/mid glow/outer bloom are distinguishable at 320 × 180, presence and trajectory of the flow hot spot, purposeful separation of graphic tracks, compression/downscale appearance, electric flicker and whether the face remains the focus. Review the signature motif, intentional color placement, selective highlights, rhythm of solid/gapped lines, restrained asymmetry and recognizability without motion. Reject generic recolors or a single uniformly bright outline presented as a new premium family. Store preset, input fixture identity, OBS/backend/version, resolution and deterministic animation times with each capture. Never label the concept board as an actual plugin screenshot.

## Phase B — implemented light grammar versus proposal

Current Phase-B implementation contains distinct sharp colored core,
a fine white-biased centerline, compact mid glow, broader bloom, local
white-biased Flow flare, and corner-weighted supporting track for the
existing rounded/rectangle double-rim style. Static and Pulse retain
the underlying luminous core. The new controls permit tuning envelope
strength and hotspot width separately. Mask/framing remains geometric;
no person/background removal is introduced.

**Not yet accepted as proposal parity:** authored Streamer/Chat/Electric
motifs, full contour-length motion, safe-fit for extreme mask translations,
OBS dark/light and 320×180 actual capture review, and GPU-frame timing.
Do not advertise a named new visual family from a recolored common rim.
Evaluate the Phase-B build side by side with the supplied proposal using
the exact same owned source and scene layout. Capture Static/Flow/Pulse,
dark/light scene, size/downscale, and a mask shifted near canvas edges.


## Phase C first hero: Cyber Rounded

Design grammar: preserve the sharp luminous base, separate broken outer
support track and subtler inner guide, paired short horizontal/vertical
corner traces, corner glints, intentional negative space, and an optional
bright local highlight running around the *rounded contour's actual lengths*.
In Static mode the corner traces and deliberate gaps must still communicate
an authored, futuristic frame. The mask and subject transform remain those
from Phase A; cosmetic tracks never redefine the portrait silhouette.

This implementation does **not** claim pixel-identical reproduction of the
marketing board. The Cyber recipe is opt-in (selected Cyber preset or manual
Signature Frame Detailing control). It requires actual Windows OBS comparison
against the supplied proposal on light/dark and 320×180 scenes, including
cropping near edges and skin-color preservation. Other proposed style
families are not rebranded copies of Cyber; they need distinct geometry and
their own visual proof.


## Phase C additional signature targets

- **Reactor Ring:** bright circular primary with two unequal supporting arc
  spans, separated negative space, sparse ticks and an independently bright
  moving orbit point. Static mode retains an authored circular composition.
- **Tech HUD:** orthogonal bracket pairs, deliberate support-rail cuts,
  small edge highlights and cyan/amber hierarchy; not a recolored rounded box.
- **Streamer Bubble:** clear rounded body, a two-stroke decorative lower-left
  speech tail and bounded small status dots, leaving the webcam intact.

These are *first implementation targets*. Actual screenshots and clips on
Windows OBS, light and dark backgrounds, 320×180 and the proposal comparison
are the required visual acceptance. Tail ornament does not expand the
underlying video mask; it can be clipped if safe-fit is not provided.


## Phase D: mask silhouette is the artwork

A Chat Bubble is NOT a rounded rectangle with an outside speech-pointer
ornament: webcam pixels must occupy the tail, the body/tail join has no
internal border, and neon follows the outer joined silhouette. Angled Card
removes top-right and bottom-left image corners along a true diagonal. Its
neon and glow follow those diagonal segments, not an uncut rectangle. D1
implements these contours; actual proposal image #8/#9 similarity remains
subject to OBS screenshots with safe-fit, light/dark and small-facecam checks.
