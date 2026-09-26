# OBS NeonMask Studio — product and engineering charter

Status: accepted project direction; requirements below are targets, not claims of shipped functionality.
Owner: Mas Ari / repository maintainer. Established: 2026-09-26.
Baseline inspected: `039169cf1b30c9e20a17b9393e0b5de5c708c008`.
Changes to scope, compatibility, geometry, threading or quality gates require a documented reason and corresponding roadmap update.

## Purpose

Give streamers a beautiful facecam frame in under one minute: add one native OBS filter, choose a design, adjust framing and colors, and stream. Combine accurate masking with restrained luminous cores, soft outer glow, coherent motion and futuristic ornaments. The streamer remains the focal point.

“Better than Advanced Masks” means measurable improvements in the selected facecam workflow, visual integration and cost. It is not a claim of complete feature parity or higher performance until measured. Build incrementally on the existing C11/libobs implementation.

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
| MASK-04 | P1 | Position, independent size and subject fit | Geometry and video transforms are explicit; saved values survive restart |
| MASK-05 | P2 | Image alpha/luma, gradient, then SVG and source masks | Same compositing contract; bounded imports and failure behavior |
| NEON-01 | P0 | Dual color core, outer halo, pulse and flow | Face interior stays clear; transparent edges composite correctly |
| NEON-02 | P1 | Double/triple rim, perimeter dashes, ring and HUD ornaments | Spacing is coherent along the actual contour; deterministic motion |
| NEON-03 | P1 | No accidental glow clipping in supported safe-fit mode | Effect alpha at canvas edges satisfies the visual gate |
| NEON-04 | P2 | Bounded electric arcs and chat-bubble frame | No full-face flashing; clean static option and stable silhouette |
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

Deferred: AI subject segmentation, cloud accounts, online preset marketplace, automatic face tracking, arbitrary user shaders, unlimited layers, browser UI, GPU-to-CPU video processing, and automatic in-process binary replacement. Audio reactivity is optional later and never a prerequisite for attractive motion. HDR is unverified until explicitly implemented and tested.

M1 motion contract: static freezes every visual phase; speed zero freezes animation; changing speed preserves phase; no visible time-based reset at the one-hour boundary. A pure tick simulation verifies state integration, while G4 final pixels and G6 full OBS soak remain required before an animation reliability claim.

“Smart” initially means validated settings, safe margins, bounded resource use, capability-aware choices and sensible defaults. Adaptive quality is opt-in later; it must not silently change the silhouette, crop or mask privacy.

## Success and delivery policy

A preview is successful when a new user can install, choose a design and record a correct OBS output with acceptable measured cost. A stable release additionally requires the lifecycle, memory, visual, compatibility and packaging gates.

Prioritize: correctness and reproducibility → beautiful basic output → measured performance → advanced ornaments → wider mask providers. Performance measurement starts with the first render slice, not at the end. Do not expand all subsystems simultaneously. Every completed feature links to implementation and acceptance evidence.
