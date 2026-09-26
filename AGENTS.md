# Maintaining OBS NeonMask Studio

Read [docs/PRODUCT_SPEC.md](docs/PRODUCT_SPEC.md), [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md), [docs/QUALITY_GATES.md](docs/QUALITY_GATES.md), [docs/VISUAL_SPEC.md](docs/VISUAL_SPEC.md) and the current [docs/ROADMAP.md](docs/ROADMAP.md) before changing runtime behavior. These are the project reference documents; keep them consistent with implementation and evidence.

- Inspect current main and open work before editing. Preserve the latest verified baseline and avoid duplicating active PRs.
- Treat the user's concept board (image #9 in September 26 feedback) as the binding visual north star; user OBS screenshots #1–#8 document current gaps, not parity. Read VISUAL_SPEC's premium-frame anatomy and framing contract before altering shaders, geometry, presets or UX. Never relabel a plain rim as a completed premium family.
- Product identity is *art-directed geometric masks and beautiful futuristic luminous frames*: no background removal, AI portrait segmentation or auto-face tracking. X/Y subject pan/zoom is manual video reframing inside the mask, not subject extraction. Each advertised visual family needs its own intentional contour/ornament motif; color/glow/width variations of a generic line do not count as new designs.
- Prioritize independent mask X/Y and subject pan/zoom, then distinct sharp core/mid-glow/outer bloom and visible localized motion, then safe-fit/contour ornaments. Require actual OBS stills/clips, not concept artwork or CI compilation, for visual completion claims.
- Keep changes cohesive. Preserve the native C11/libobs architecture unless a documented decision justifies a change.
- Keep source IDs, setting keys and enum numeric values compatible; provide migrations for intentional schema changes.
- Use one validated config/geometry path for presets, UI and rendering. Keep pure code testable without OBS.
- Do not add worker threads to render frames. Add bounded/cancellable asset work only when a concrete feature needs it.
- Respect graphics-context ownership and source-reference lifetimes. No steady-state I/O, decode, recurring allocation, blocking wait or synchronous GPU readback in render callbacks.
- Do not claim crash-proof, leak-free, upstream parity, hardware performance or visual correctness without corresponding evidence. CI compilation is not OBS runtime validation.
- Maintain compact OBS-native settings and protect the facecam image from distracting ornaments.
- Update the roadmap/evidence with each completed slice. Report tests run and pending runtime gates honestly.
- Keep provenance and notices for reused code. Do not introduce DCO/signoff, CLA gates or paid signing requirements unless the maintainer explicitly requests them.
- For docs-only changes, verify links and consistency; runtime builds are unnecessary unless repository CI requires them.
