# OBS NeonMask Studio

A native, GPU-rendered OBS facecam filter: **mask + dual-color neon core + analytic glow + animation** driven by the **same signed-distance shape**. This is an early **v0.1.0 source-code milestone**, not a production release.

## Implemented

- One OBS video filter: `NeonMask Studio — Facecam Mask & Border`.
- Rounded rectangle, circle, ellipse, hexagon and diamond; size, roundness and feather controls.
- Dual-color neon, width, analytic halo radius/intensity, static/pulse/traveling-light modes, angular segmentation.
- Four editable presets: Cyber Rounded, Reactor Ring, Emerald Hex, Ember Frame.
- OBS-native properties, English and Indonesian strings, direct GPU effect. No web browser or recurring texture uploads.
- Headless C geometry/color tests.

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

On Windows, select your OBS SDK toolchain and supply `-Dlibobs_DIR=<directory containing libobs-config.cmake>` as needed. For portable Windows installations, staged output uses `obs-plugins/64bit/obs-neonmask.dll` and `data/obs-plugins/obs-neonmask/`. Restart OBS after installation. The first deliverable is source only; no Windows binary is supplied or claimed tested here. Linux staging places the data files under `share/obs/obs-plugins/obs-neonmask/`.

## In OBS

Right-click webcam source → **Filters** → **Effect Filters** → **+** → **NeonMask Studio**. Select a preset or adjust shape, colors and effect strength. The filter operates on the **source itself**, not a scene-wide frame.

## Deliberate MVP limitations

- Halo stays **within the original source rectangle**; use the built-in scale margin. Expanded output padding / companion frame source is next.
- Custom alpha images, SVG and source masks are **not yet implemented**, nor are a graphical preset gallery, audio reactivity and GPU benchmarks.
- The outline on ellipses and segmented non-circular shapes is approximate; later versions will use better distance-field/path-length algorithms.
- No physical OBS runtime smoke test or cross-platform release verification has been completed yet.

## Attribution and reuse

This implementation is newly written around the documented libobs filter/effect APIs and signed-distance geometry. The architecture was informed by [FiniteSingularity/obs-advanced-masks](https://github.com/FiniteSingularity/obs-advanced-masks) and [the official OBS plugin template](https://github.com/obsproject/obs-plugintemplate). No upstream source file is pasted or mechanically renamed. If upstream code is subsequently imported, keep its copyright notices and comply with the GPL. The rounded-rectangle SDF method is widely documented by [Inigo Quilez](https://iquilezles.org/articles/distfunctions2d/). The repository is distributed under its existing GPL-3.0 license; new source files carry GPL-2.0-or-later notices, which are GPL-3.0-compatible.

## Roadmap

1. Test shader and dynamic properties in Windows OBS; fix packaging and GPU differences.
2. Expanded render area / no-clipping glow, correct chained-filter dimensions.
3. Image alpha, source mask, SVG, and cached generated distance fields.
4. Curvilinear segment parametrization, multilayer borders, preset gallery, audio response.
5. Frame-time benchmark matrix and platform release automation.
