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

## GitHub contribution and achievement practice

The maintainer wants GitHub Achievements to grow naturally alongside genuine open-source development. This is a **secondary outcome**, never a reason to weaken product quality, create noise or alter the delivery roadmap.

- Prefer a focused branch and pull request for each cohesive feature, bugfix or meaningful documentation change. Tie it to the relevant issue/requirements, include tests and evidence, and use descriptive commits/PRs. Do not split a single cohesive task into artificial micro-PRs, create empty PRs/issues, or manufacture activity for badges.
- Run the applicable CI and review checks before merge; merge only after the change meets its acceptance criteria. Prefer squash merge when appropriate, preserve a traceable commit/artifact and update the roadmap ledger. A green build alone does not satisfy OBS visual/runtime gates.
- Attribute authorship to the actual contributor(s) and the maintainer's authorized GitHub identity. Use Co-authored-by only for real collaborators who contributed and agreed to attribution; do not invent human co-authors or falsely attribute ChatGPT/tool activity. Follow repository review rules, even if a badge could be obtained by skipping them.
- Pull Shark may progress through legitimate merged PRs. Quickdraw and YOLO are already earned and are not a reason to rush issue closure or bypass review. Other achievements (e.g. Pair Extraordinaire, Galaxy Brain, Starstruck) may follow genuine collaboration, helpful discussions and useful releases/documentation; do not guarantee GitHub will award or upgrade a badge.
- Keep new releases, actual OBS screenshots/clips, contributor instructions, changelogs and issue triage useful to real users. Do not solicit fake stars, create sham conversations or make sponsorship/payment decisions on the maintainer's behalf.
- When reporting progress, cite real PRs, commits, tests and GitHub artifact links. Mention an achievement change only when verified on GitHub; otherwise describe the qualifying development activity without claiming the badge was awarded.

- Phase D is [Issue #26](https://github.com/masarray/obs-neonmask-studio/issues/26) plus docs/PHASE_D_IMPLEMENTATION.md. Implement integrated alpha+neon bubble/card silhouettes before more detached ornament modes; all old shape IDs, pan/zoom and legacy scenes must remain intact. No source import from GPL-2.0-only Advanced Masks code without compatible permission; use it as an audited feature baseline and acknowledge provenance.
