# OBS NeonMask Studio — product and engineering charter

Status: accepted project direction; requirements below are targets, not claims of shipped functionality.
Owner: Mas Ari / repository maintainer. Established: 2026-09-26.
Baseline inspected: `039169cf1b30c9e20a17b9393e0b5de5c708c008`.
Changes to scope, compatibility, geometry, threading or quality gates require a documented reason and corresponding roadmap update.

## Purpose

Give streamers a beautiful facecam frame in under one minute: add one native OBS filter, choose a design, adjust framing and colors, and stream. Combine accurate masking with restrained luminous cores, soft outer glow, coherent motion and futuristic ornaments. The streamer remains the focal point.

“Better than Advanced Masks” means measurable improvements in the selected facecam workflow, visual integration and cost. It is not a claim of complete feature parity or higher performance until measured. Build incrementally on the existing C11/libobs implementation.

## Product identity: artistic neon masking, not subject removal

Maintainer clarification (2026-09-26): NeonMask Studio is an **art-directed
facecam mask and futuristic luminous frame designer**. Its differentiation
is beauty, recognizable visual identity, modern/fancy details and expressive
yet tasteful animation — **not** background removal, AI segmentation,
subject cutouts, camera tracking, or a generic uniform neon outline.
The user's source photo/video remains intact *inside* the chosen geometric
mask; masking outside that silhouette is the intentional effect.
Manual X/Y framing and zoom position the image *within* the shape. These
controls must not introduce any segmentation dependency.

Art direction is a first-class product requirement: each shipped family
needs a recognizable authored contour, intentional color placement,
well-composed negative space, fine luminous core, graduated glow, purposeful
secondary linework (rather than random decoration), and a signature motion
behavior. Variations of only hue, thickness or halo radius do **not** count
as distinct premium design families. The target is the creative language
of the user-approved board #9 adapted to real OBS rendering; it is not a
pixel-for-pixel reproduction of a photo/marketing board.

**Explicit non-goal:** do not add background-removal/portrait-segmentation
work to this project's plan. Optional use with another OBS filter is an OBS
integration detail, not a NeonMask feature or prerequisite.

## Visual north star and user acceptance

The **user-supplied concept board (image #9 in the 2026-09-26 OBS feedback) is the
binding visual direction** for the procedural/ornament product, not optional
marketing inspiration. It depicts luminous contour-aware frames with bright
fine cores, visible graduated bloom, expressive moving accents, purposeful
secondary tracks/corners and controlled two-color distribution. The
actual OBS screenshots #1–#8 confirm filter loading and the five existing
shapes/presets, but also document material gaps: no independent X/Y face
reframing, comparatively flat/weak bloom, and styles that do not yet reproduce
the intended layered futuristic appearance. They are a verified user
observation, not evidence that all G4/G5 tests or hardware budgets passed.

**Definition of visual completion:** ship actual OBS captures for each
implemented reference family, alongside named recipes and a documented
side-by-side review against [VISUAL_SPEC.md](VISUAL_SPEC.md). A successful
compile, functioning plain outline, new preset name, or illustration that
resembles the board does **not** establish visual parity. Do not silently
reduce the target to fit the current shader. If a reference family is deferred,
mark it explicitly *not yet implemented* rather than calling the collection
complete. Source imagery/people and the layout of the concept board are not
included in the distributable plugin.

The next tester-facing vertical slices prioritize (1) independently adjustable
mask position and facecam subject pan/zoom, (2) premium multi-component neon
and clearly visible optional motion with a stable sharp core, (3) safe-fit and
contour geometry, and (4) actual OBS captures and performance evidence. Only
then broaden the ornaments/shapes. Defaults and existing scenes remain
compatible, and the facecam image remains the focus.

## Document authority

| Document | Owns |
| --- | --- |
| This charter | Product requirements, scope, priorities and user experience |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Runtime contracts, ownership, canonical state and design decisions |
| [QUALITY_GATES.md](QUALITY_GATES.md) | Measurement protocol and release acceptance |
| [ROADMAP.md](ROADMAP.md) | Delivery order, evidence and current gaps |
| [VISUAL_SPEC.md](VISUAL_SPEC.md) | Visual families and image-reference translation |
| [../AGENTS.md](../AGENTS.md) | Maintenance workflow for future contributors and coding agents |

Existing PRE_PHASE2_AUDIT, GPU_SMOKE and WINDOWS_PREVIEW documents remain evidence/setup notes, not competing roadmaps. If an implementation disagrees with a target, record the gap; do not silently weaken the target or describe it as complete.

## Users and main journey

Primary user: a streamer who knows OBS source filters but does not know shaders. Secondary user: a power user creating reusable scene collections and presets.

1. Add NeonMask Studio to webcam/video source; a usable default appears immediately.
2. Select a small curated preset list; show shape and style separately.
3. Adjust mask size/position and subject fit without stretching the image.
4. Change two colors, border width, glow and motion speed.
5. Save the scene; reopening OBS reproduces the same appearance.
6. Advanced controls expose ornaments, precision geometry and quality without overwhelming the first screen.

Use compact OBS-native controls, normal typography, logical collapsible groups and English/Indonesian labels. Avoid oversized cards, decorative settings dashboards or a browser dependency. Preset selection applies editable values; later changes become Custom. Reset should be explicit and predictable. Controls irrelevant to the selected mode should be hidden or disabled with a short reason.

## Requirements and acceptance

P0 = required for a reliable preview; P1 = first polished product; P2 = expansion after the core gates.

| ID | Priority | Requirement | Acceptance |
| --- | --- | --- | --- |
| MASK-01 | P0 | Unified shape for clipping, border and halo | No visible mismatch between mask edge and border across the fixture matrix |
| MASK-02 | P0 | Rounded rectangle, circle, ellipse, hexagon, diamond | Stable aspect ratio, feather and thickness at small and large sizes |
| MASK-03 | P1 | Rectangle, triangle and regular polygon controls | Correct corners, predictable side count/rotation; no angular stretch artifacts |
| MASK-04 | P0 next preview | Independent X/Y mask position and subject pan/zoom plus independent mask width/height | A user can center the face without distorting/stretching video; reset, source resize, preset edit and saved-scene restart are deterministic |
| MASK-05 | P2 | Image alpha/luma, gradient, then SVG and source masks | Same compositing contract; bounded imports and failure behavior |
| NEON-01 | P0 | Dual color sharp core, graduated outer halo, pulse and flow | Clearly distinguishable core and exterior bloom over dark/light footage; clear face interior and correct transparent composition |
| NEON-02 | P1 | Double/triple rim, perimeter dashes, ring and HUD ornaments | Spacing is coherent along the actual contour; deterministic motion |
| NEON-03 | P1 | No accidental glow clipping in supported safe-fit mode | Effect alpha at canvas edges satisfies the visual gate |
| NEON-04 | P2 | Bounded electric arcs and chat-bubble frame | No full-face flashing; clean static option and stable silhouette |
| NEON-05 | P0 next preview | Premium light hierarchy and adjustable motion contrast | Bright narrow core + controlled mid glow + soft outer bloom; moving accent is visible at 320 × 180 without making the entire frame flash |
| VISUAL-01 | P0 next preview | Reference-board fidelity gate for shipped family/preset | Actual OBS capture and reference recipe reviewed side by side; known gaps labelled; no concept artwork passed off as a runtime screenshot |
| UX-01 | P0 | One filter, immediate preset and editable controls | New-user trial reaches usable frame within 60 seconds after installation |
| UX-02 | P1 | Versioned preset export/import | Round-trip values, helpful errors, no arbitrary code execution |
| SAFE-01 | P0 | Predictable failure and complete cleanup | Fault/lifecycle tests pass; no known plugin-caused crash or leak |
| PERF-01 | P0 | Measured bounded cost | Meet the declared hardware/quality budgets in QUALITY_GATES |
| COMPAT-01 | P0 | Preserve source ID, enum IDs and scene settings | Existing scene fixtures load without visual/configuration loss |
| RELEASE-01 | P0 | Reproducible artifact from tested commit | Artifact SHA, compiler/SDK, tests and runtime evidence are traceable |

## Mask reference strategy

[Advanced Masks at inspected revision](https://github.com/FiniteSingularity/obs-advanced-masks/tree/7e80d7a5b58e3f039172bf8a647d045bc0d42997) is the functional reference. Its documented shape, image/source, gradient and adjustment masking workflows establish the comparison surface. Its source tree also contains SVG-related modules. Presence in upstream source does not establish our implementation or a compatibility promise.

| Capability | NeonMask plan |
| --- | --- |
| Parameterized geometric alpha masks | First-class core; close precision and transformation gaps first |
| Static image and gradient masks | Later bounded providers behind the same mask interface |
| SVG and live OBS source masks | Later, after cancellation, source ownership and cost controls exist |
| Color adjustment masks | Deferred; outside the facecam-frame launch scope |
| Neon core, luminous layers, coherent perimeter motion | Main differentiation, independently validated |
| Full Advanced Masks settings import/parity | Not a launch promise; requires an explicit mapping project |

Study proven algorithms and integration patterns, then implement the smallest compatible solution. Preserve provenance and notices when reusing code; track imported files/revisions. Keep the current repository license policy consistent. A wholesale rewrite or a copied upstream plugin with renamed symbols is not the plan.

## Scope boundaries

Launch scope: local native filter, procedural shapes, curated neon designs, editable settings, reliable saving, safe degradation, Windows preview and measured SDR rendering. Linux/OpenGL provides a second validation backend. Native macOS and additional OBS versions only become supported after runtime/package evidence.

Explicitly out of scope: background removal, AI subject/portrait segmentation, and automatic face tracking. Deferred: cloud accounts, online preset marketplace, arbitrary user shaders, unlimited layers, browser UI, GPU-to-CPU video processing, and automatic in-process binary replacement. Audio reactivity is optional later and never a prerequisite for attractive motion. HDR is unverified until explicitly implemented and tested.

M1 motion contract: static freezes every visual phase; speed zero freezes animation; changing speed preserves phase; no visible time-based reset at the one-hour boundary. A pure tick simulation verifies state integration, while G4 final pixels and G6 full OBS soak remain required before an animation reliability claim.

“Smart” initially means validated settings, safe margins, bounded resource use, capability-aware choices and sensible defaults. Adaptive quality is opt-in later; it must not silently change the silhouette, crop or mask privacy.

## Success and delivery policy

A preview is successful when a new user can install, choose a design and record a correct OBS output with acceptable measured cost. The next *visual* preview additionally must let the user reframe the face with X/Y and show a visibly improved neon hierarchy. A polished/stable procedural release cannot be declared solely from G1–G3 or partial G4; VISUAL-01 and actual OBS composition evidence are required. A stable release additionally requires the lifecycle, memory, visual, compatibility and packaging gates.

Prioritize: correctness and reproducibility → beautiful basic output → measured performance → advanced ornaments → wider mask providers. Performance measurement starts with the first render slice, not at the end. Do not expand all subsystems simultaneously. Every completed feature links to implementation and acceptance evidence.

## Phase B delivery interpretation (NEON-01/05)

User-proven Phase-A manual X/Y framing is preserved. Next implementation
delivers an adjustable *light hierarchy* rather than one uniformly stronger
outline: stable fine core, compact mid glow, wider bloom and independently
controllable local traveling highlight. The initial shape-aware corner
accents are a first grammar element, not a claim that the entire signature
preset collection matches the concept artwork. Actual OBS captures,
light/dark/320×180 art review, displacement-safe halo bounds and measured
performance are mandatory follow-up evidence. No background removal.
