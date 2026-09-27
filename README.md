# OBS NeonMask Studio

A native, GPU-rendered OBS facecam filter: **mask + dual-color neon core + analytic glow + animation** driven by the **same signed-distance shape**. This is an early **v0.1.0 unsigned preview** with a native Windows x64 CI artifact, not a production release.

## Project direction

Start with the [product and engineering charter](docs/PRODUCT_SPEC.md). It links the canonical [architecture](docs/ARCHITECTURE.md), [visual specification](docs/VISUAL_SPEC.md), [quality gates](docs/QUALITY_GATES.md) and [delivery roadmap](docs/ROADMAP.md). Requirements are targets; the roadmap distinguishes implemented code from runtime-verified behavior.

## Implemented

- One OBS video filter: `NeonMask Studio — Facecam Mask & Border`.
- Rounded rectangle, circle, oval, rectangle, hexagon, diamond, triangle, polygon, **integrated Chat Bubble** and **Angled Card**; independent mask size/X/Y, subject pan/zoom, rotation and feather.
- Dual-color neon, width, analytic halo radius/intensity, static/pulse/traveling-light modes, angular segmentation.
- Four GPU-composited border styles: Classic Neon, Cyber Double Rim, Tech HUD, Minimal Glow.
- Seven editable presets: Cyber Rounded, Reactor Ring, Emerald Hex, Ember Frame, Tech HUD, Streamer Bubble and Angled Card. Existing IDs 1–6 are stable; Angled Card appends ID 7. Reselecting Streamer uses the integrated bubble; previously saved rounded Streamer scenes retain their stored shape. Explicit signature ornament recipes (Cyber corner traces, Reactor arcs, HUD brackets, legacy Streamer decorative bubble tail) are optional and follow the mask contour.
- OBS-native properties, English and Indonesian strings, direct GPU effect. No web browser or recurring texture uploads.
- Release-safe C geometry, contour, preset and motion tests; static shader/host contract and actual libobs OpenGL/D3D11 direct-effect pixel fixtures.
- Corrected RGBA color handling, captured-alpha compositing and base-size calculations.

## Build

Requires a matching OBS Studio **libobs development SDK**, CMake 3.20+, and C11 compiler.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
cmake --install build --prefix /path/to/obs-portable-root
```

For headless-only tests without OBS installed:

```sh
cmake -S . -B build-test -DNEONMASK_BUILD_PLUGIN=OFF
cmake --build build-test
ctest --test-dir build-test --output-on-failure
```

On Windows, select your OBS SDK toolchain and supply `-Dlibobs_DIR=<directory containing libobs-config.cmake>` as needed. For portable Windows installations, staged output uses `obs-plugins/64bit/obs-neonmask.dll` and `data/obs-plugins/obs-neonmask/`. Restart OBS after installation. An [unsigned Windows x64 preview artifact](https://github.com/masarray/obs-neonmask-studio/actions/workflows/windows-preview.yml) is built from the pinned OBS 31.1.1 SDK; see [Windows preview instructions](docs/WINDOWS_PREVIEW.md). The shader compiles and renders directly sampled GPU pixel fixtures on libobs OpenGL (Mesa) and Windows Direct3D 11 CI backends, but the OBS frontend, filter chaining and final canvas pixels have not yet been verified. Linux staging places the data files under `share/obs/obs-plugins/obs-neonmask/`.

## In OBS

Right-click webcam source → **Filters** → **Effect Filters** → **+** → **NeonMask Studio**. Choose an authored preset, or set **Signature Frame Detailing** to Legacy/Cyber/Reactor/Tech HUD/Streamer and customize its intensity/gap. Mask X/Y moves the geometric frame; Subject Pan/Zoom reframes only the source image. The filter operates on the **source itself**, not a scene-wide frame.

## Deliberate MVP limitations

- Halo stays **within the original source rectangle**; use the built-in scale margin. Expanded output padding / companion frame source is next.
- Custom alpha images, **SVG import** and source masks are **not yet implemented**. Original SVG design references in `assets/designs/` are authoring samples only; the D1 integrated Chat Bubble and Angled Card use production GPU geometry. No graphical gallery, audio reactivity or GPU benchmark evidence yet.
- Ellipse and segmented non-circular paths remain approximate; Cyber Rounded has rounded-perimeter travel, while universal contour-length motion is not yet delivered. Actual visual parity with the concept artwork and final OBS output are **not yet verified**.
- GPU compilation and direct-effect pixel smoke tests run on libobs OpenGL and D3D11. Real OBS frontend/final-output visual tests, filter-chain validation, compatibility matrix and performance measurements remain pending.

## Engineering quality gate

See [docs/PRE_PHASE2_AUDIT.md](docs/PRE_PHASE2_AUDIT.md) for the upstream comparison, resolved baseline defects, CI coverage, and explicit remaining runtime checks.

## Attribution and reuse

This implementation is newly written around the documented libobs filter/effect APIs and signed-distance geometry. The architecture was informed by [FiniteSingularity/obs-advanced-masks](https://github.com/FiniteSingularity/obs-advanced-masks) and [the official OBS plugin template](https://github.com/obsproject/obs-plugintemplate). No upstream source file is pasted or mechanically renamed. If upstream code is subsequently imported, keep its copyright notices and comply with the GPL. The rounded-rectangle SDF method is widely documented by [Inigo Quilez](https://iquilezles.org/articles/distfunctions2d/). The repository is distributed under its existing GPL-3.0 license; new source files carry GPL-2.0-or-later notices, which are GPL-3.0-compatible.

## Phase D: actual authored mask contours

The Chat Bubble tail and Angled Card diagonal cuts are **part of the source alpha mask and its shared neon contour**, not detached overlay ornaments. The new shape-detail control changes tail depth/chamfer size. [Phase D implementation contract](docs/PHASE_D_IMPLEMENTATION.md) separates the shipped code candidate from pending SVG importer, wider authored library, safe-fit and actual OBS concept comparison.

## Roadmap

See the [canonical delivery roadmap and evidence ledger](docs/ROADMAP.md) for milestone order, active work and acceptance gates.
