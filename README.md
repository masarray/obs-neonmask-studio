# OBS NeonMask Studio

[![Build and test](https://github.com/masarray/obs-neonmask-studio/actions/workflows/ci.yml/badge.svg)](https://github.com/masarray/obs-neonmask-studio/actions/workflows/ci.yml)
[![GPU shader smoke](https://github.com/masarray/obs-neonmask-studio/actions/workflows/gpu-smoke.yml/badge.svg)](https://github.com/masarray/obs-neonmask-studio/actions/workflows/gpu-smoke.yml)
[![Windows x64 preview](https://github.com/masarray/obs-neonmask-studio/actions/workflows/windows-preview.yml/badge.svg)](https://github.com/masarray/obs-neonmask-studio/actions/workflows/windows-preview.yml)
[![Release](https://img.shields.io/github/v/release/masarray/obs-neonmask-studio?include_prereleases&label=public%20preview)](https://github.com/masarray/obs-neonmask-studio/releases/tag/v0.1.0)
[![License: GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](LICENSE)

**A native, GPU-rendered OBS facecam mask and neon-frame filter.** NeonMask Studio combines analytic masks, dual-color neon, glow, authored futuristic ornaments, and deterministic animation in one OBS video filter—without a browser source or recurring CPU texture-upload pipeline.

**Current public release:** [v0.1.0 — Public Preview](https://github.com/masarray/obs-neonmask-studio/releases/tag/v0.1.0)  
**Platform package:** Windows x64 · unsigned preview binary  
**Release notes:** [docs/RELEASE_NOTES_v0.1.0.md](docs/RELEASE_NOTES_v0.1.0.md) · **Current engineering status:** [docs/PROJECT_STATUS.md](docs/PROJECT_STATUS.md)

> **Preview status:** v0.1.0 is suitable for hands-on testing and content creation, but it is not presented as a production-certified/stable compatibility release. See [Known limitations](#known-limitations).

## Why NeonMask Studio?

- **Native OBS filter** — no browser overlay and no web runtime.
- **One contour authority** — mask, border, glow, ornaments, and animation are derived from the same geometry contract.
- **Facecam-first framing** — independent mask placement plus subject pan/zoom.
- **Premium authored designs** — not just a uniform outline with different colors.
- **Scene-scale readable motion** — mask-relative counter-orbits retain visual mass when a facecam is scaled down.
- **Context-aware controls** — irrelevant controls are hidden/disabled for specialized designs such as Dual Ring.
- **GPU verification** — OpenGL/Mesa and Windows D3D11 direct-effect pixel fixtures run in CI.
- **Portable release package** — public Windows ZIP uses the standard OBS plugin folder layout.

## Featured designs

v0.1.0 ships **12 editable presets**:

| # | Preset | Character |
| ---: | --- | --- |
| 1 | Cyber Rounded | Pink/cyan double-rim rounded frame |
| 2 | Reactor Ring | Blue/pink circular reactor treatment |
| 3 | Emerald Hex | Green/cyan hexagonal neon |
| 4 | Ember Frame | Warm orange/gold minimal frame |
| 5 | Tech HUD Advanced | Futuristic cyan/amber cut-frame treatment |
| 6 | Streamer Bubble | Integrated chat-bubble silhouette |
| 7 | Angled Card | Diagonal authored contour |
| 8 | HUD Cut Panel | Stepped/cut HUD panel |
| 9 | Neon Squircle | Soft superellipse-style contour |
| 10 | Game UI | Green authored corner modules |
| 11 | Gradient Rainbow | Animated rotating spectrum |
| 12 | Rotating Ring — Counter Orbit | Two short external light objects rotating in opposite directions |

The **Rotating Ring** design uses two 39%-turn mask-relative light objects, outer CW / inner CCW motion, independent primary/secondary identity, tapered fading endpoints, and dedicated orbit width/spacing controls.

## Install on Windows

1. Open the [v0.1.0 release](https://github.com/masarray/obs-neonmask-studio/releases/tag/v0.1.0).
2. Download **`obs-neonmask-v0.1.0-windows-x64.zip`**.
3. Optional but recommended: verify it against **`obs-neonmask-v0.1.0-windows-x64.sha256`**.
4. Close OBS.
5. Extract the ZIP into the OBS installation/portable root, preserving the included directory structure.
6. Restart OBS.
7. Right-click a camera/image/video source → **Filters** → **Effect Filters** → **+** → **NeonMask Studio**.

The package installs:

```text
obs-plugins/64bit/obs-neonmask.dll
data/obs-plugins/obs-neonmask/
├── shaders/
├── locale/
└── designs/
```

The Windows binary is currently **unsigned**, so browser/Windows reputation systems may warn on download. The source and exact release provenance are public; see [Windows preview and verification](docs/WINDOWS_PREVIEW.md).

## Core capabilities

### Masking and framing

- Rounded rectangle, circle, oval, rectangle, hexagon, diamond, triangle, regular polygon.
- Authored Chat Bubble, Angled Card, HUD Cut Panel, Tech HUD, Game UI, and Squircle shapes.
- Independent mask width/height and mask X/Y.
- Independent subject pan X/Y and subject zoom.
- Rotation/shape-detail controls where geometrically relevant.
- Feathered alpha masking.
- Restricted local SVG-path mask preview for simple closed path artwork.

### Neon and motion

- Solid, dual-color, and animated rainbow color pipelines.
- Sharp core + controlled glow/bloom hierarchy.
- Static, pulse, and flow animation where supported.
- Authored ornaments such as cyber traces, HUD modules, reactor details, and external counter-orbits.
- Mask-relative Dual Ring geometry designed to survive facecam downscaling.

### Bounds and output

- Automatic safe-fit for supported authored outer ornaments.
- Optional **expanded output** that adds filter-local padding so glow/outer ornaments can be preserved without shrinking the portrait.
- Rotation-aware bounds for authored external L-shaped modules.
- Transparent source sampling outside the captured input instead of clamped edge smearing.

### OBS integration

- OBS-native property UI.
- English and Indonesian localization.
- Versioned persisted config with additive migration through current schema v4.
- Existing enum/source identity preserved across the preview development line.

## Compatibility and verification

| Area | v0.1.0 status |
| --- | --- |
| Windows x64 public package | **Published** |
| Build SDK | **OBS 31.1.1 pinned in CI** |
| Windows GPU backend | **D3D11 direct-effect smoke/pixel fixtures** |
| Linux GPU backend | **OpenGL/Mesa direct-effect smoke/pixel fixtures** |
| Headless tests | **Windows, Ubuntu, macOS CI** |
| Undefined behavior checks | **UBSan CI** |
| Linux native plugin build | **CI verified; no official binary release yet** |
| macOS native package | **Not published** |
| Newer OBS frontends | Development testing has exercised newer builds; full compatibility matrix is not yet claimed |
| Stable/production certification | **Not claimed for v0.1.0** |

A green CI run proves the covered build/config/geometry/shader contracts. It does **not** by itself prove every OBS frontend/filter-chain/GPU/driver combination. The evidence boundaries are documented in [QUALITY_GATES.md](docs/QUALITY_GATES.md).

## Known limitations

- The Windows release is unsigned and can trigger browser/SmartScreen reputation warnings.
- General image-alpha/luma masks, live OBS-source masks, unrestricted SVG, and arbitrary user shaders are not implemented.
- Local SVG import is intentionally restricted to a bounded path-only subset; unsupported constructs are rejected.
- Background removal, AI portrait segmentation, face tracking, and camera tracking are explicit non-goals.
- Universal contour-length motion is not yet available for every non-circular shape; some preview motion remains angle/perimeter approximated.
- Full multi-version OBS compatibility, named-hardware performance budgets, long-duration OBS soak, and broad filter-chain certification remain open engineering gates.
- HDR behavior is not certified.
- Linux/macOS public packages are not published in v0.1.0.

## Build from source

Requirements:

- CMake 3.20+
- C11 compiler
- Matching OBS/libobs development SDK for native plugin builds

Headless core tests:

```sh
cmake -S . -B build-test \
  -DNEONMASK_BUILD_PLUGIN=OFF \
  -DNEONMASK_BUILD_TESTS=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-test --config Release --parallel
ctest --test-dir build-test -C Release --output-on-failure
```

Native plugin build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build --output-on-failure
cmake --install build --prefix /path/to/obs-root
```

On Windows, provide the matching `libobsConfig.cmake` location as required by your OBS SDK toolchain. The official release workflow builds against a pinned OBS 31.1.1 source/dependency snapshot.

## Project documentation

| Document | Purpose |
| --- | --- |
| [PROJECT_STATUS.md](docs/PROJECT_STATUS.md) | Current shipped/pending state — start here |
| [RELEASE_NOTES_v0.1.0.md](docs/RELEASE_NOTES_v0.1.0.md) | Public release notes |
| [PRODUCT_SPEC.md](docs/PRODUCT_SPEC.md) | Product charter and non-goals |
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | Runtime/config/render contracts |
| [VISUAL_SPEC.md](docs/VISUAL_SPEC.md) | Visual families and acceptance direction |
| [QUALITY_GATES.md](docs/QUALITY_GATES.md) | Evidence boundaries and release gates |
| [ROADMAP.md](docs/ROADMAP.md) | Chronological engineering ledger and future work |
| [WINDOWS_PREVIEW.md](docs/WINDOWS_PREVIEW.md) | Windows package provenance and test workflow |
| [GPU_SMOKE.md](docs/GPU_SMOKE.md) | Direct libobs GPU smoke methodology |

The roadmap intentionally retains historical phase notes. For the **current** state, use [PROJECT_STATUS.md](docs/PROJECT_STATUS.md), not an old “next phase” paragraph deep in the engineering ledger.

## Contributing

Contributions are welcome. Please read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request. Bugs should include OBS version, OS/GPU backend, reproduction steps, and an OBS log where possible.

For security-sensitive reports, follow [SECURITY.md](SECURITY.md) instead of opening a public issue.

## License and attribution

Distributed under the repository's [GNU GPL v3](LICENSE). New implementation work is written around documented libobs APIs and signed-distance geometry. The architecture has been informed by the official OBS plugin ecosystem and prior-art masking plugins; imported upstream code, if introduced in the future, must retain provenance and compatible notices.

---

**NeonMask Studio v0.1.0 is a public preview.** Back up important OBS scene collections before testing preview updates, and validate the look on the actual camera/source resolution and scene scale you intend to use.
