# Visual direction and acceptance

Reference: user-supplied “Gambar ChatGPT 26 Sep 2026, 14.52.27.png”, inspected on 2026-09-26. The image is a concept board, not a screenshot of implemented output. It depicts masked facecams with cyan/magenta, violet, emerald and amber neon, organized into shape, animation, stream-style and customization families. This written mapping remains usable without the external attachment. Do not treat faces, photography or board typography as distributable product assets.

## Visual rules

The visible face is the subject; the luminous frame supports it. Keep the center clear, avoid tinting skin, use a sharp core with a softer exterior halo, and keep rim spacing intentional. Glow should remain attractive over black, white and moving game footage. No opaque black rectangle around transparent content.

Small facecam use matters: judge at 320 × 180 as well as full source resolution. Thin forms and restrained movement should survive downscaling. A design that only looks good in a large marketing image is not accepted.

Separate the mask silhouette from decorative accents. Chat tails, HUD corners and electric sparks must have explicit semantics: silhouette-changing shapes affect coverage; decorative accents do not punch holes in the facecam. Use the same contour anchor data for all decorative placement.

## Reference-to-delivery mapping

| Reference example | Design recipe | Baseline and planned gap |
| --- | --- | --- |
| Rounded rectangle / rectangle | Continuous thin core, softly rounded or sharp corners | Rounded exists; named rectangle and precision controls in M2 |
| Circle / oval | Consistent stroke around circular/elliptic facecam | Exists approximately; ellipse precision in M2 |
| Hexagon / diamond | Crisp sci-fi silhouette with stable corner joins | Exists approximately; validate corners in M2 |
| Triangle / polygon | Parameterized sides with optional rounded joins | New in M2 |
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

Human review also checks visual balance, skin-color preservation, compression/downscale appearance, electric flicker and whether the face remains the focus. Store preset, input fixture identity, OBS/backend/version, resolution and deterministic animation times with each capture. Never label the concept board as an actual plugin screenshot.
