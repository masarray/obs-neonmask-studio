# Project status

Last synchronized with the **v0.1.0 Public Preview / P6H** implementation line.

This document is the concise current-state view. The roadmap is a chronological engineering ledger and intentionally contains older phase notes that should not be read as the current product status.

## Current release

- **Release:** v0.1.0 — Public Preview
- **Distribution:** Windows x64 portable OBS plugin ZIP
- **Build SDK:** OBS 31.1.1 pinned in release CI
- **Config schema:** v4
- **Public release:** https://github.com/masarray/obs-neonmask-studio/releases/tag/v0.1.0

## Shipped in the public preview

### Core filter

- Native C11/libobs video filter.
- GPU effect rendering; no browser-source dependency.
- Persisted OBS-native settings with migration.
- English and Indonesian localization.

### Framing and masks

- Independent mask width/height and X/Y placement.
- Independent subject pan X/Y and zoom.
- Rounded rectangle, circle, oval, rectangle, hexagon, diamond, triangle, regular polygon.
- Integrated Chat Bubble, Angled Card, HUD Cut Panel, Tech HUD, Game UI, and Squircle authored shapes.
- Restricted local SVG path-mask preview with bounded parser/rasterization behavior.
- Feathering and relevant shape-detail/rotation controls.

### Neon and authored visuals

- Solid, dual-color, and rainbow pipelines.
- Static/pulse/flow modes where supported.
- Layered sharp-core / glow / bloom treatment.
- Cyber, reactor, HUD, game-UI and other authored ornament families.
- 12 built-in editable presets.
- Gradient Rainbow animated spectrum.
- Rotating Ring / Dual Ring:
  - two 39%-turn external light objects;
  - mask-relative width/offset/spacing;
  - outer CW / inner CCW;
  - primary-dominant outer and secondary-dominant inner object;
  - tapered fading endpoints;
  - context-aware controls that hide/disable unrelated settings.

### Bounds and compositing

- Safe-fit calculation for supported outer ornaments.
- Automatic authored-ornament clipping protection where required.
- Optional expanded filter output/padding.
- Rotation-aware bounds for connected outer-L ornaments.
- Transparent out-of-source sampling.
- Premultiplied-alpha handling covered by GPU fixtures.

## Automated evidence

- Headless geometry/config/preset/motion tests on Windows, Ubuntu, and macOS.
- UBSan job.
- Native Linux plugin build.
- OpenGL/Mesa effect compile and pixel fixtures.
- Windows OBS 31.1.1 / D3D11 effect compile and pixel fixtures.
- Small-facecam Dual Ring mass/gap/direction/fade checks.
- Package layout, checksums, build metadata, and release assets.

See [QUALITY_GATES.md](QUALITY_GATES.md) for what each layer proves and does not prove.

## Public-preview limitations / open gates

The following are **not** claimed complete by v0.1.0:

- production/stable certification;
- broad OBS-version and GPU/driver compatibility matrix;
- named-hardware CPU/GPU/render-lag performance report;
- long-duration full-OBS soak and lifecycle stress evidence;
- exhaustive final-canvas/filter-chain/crop/chroma/transition verification;
- HDR certification;
- official macOS package;
- official Linux binary release;
- unrestricted SVG or arbitrary image/source mask providers;
- universal contour-length motion for every non-circular shape.

Background removal, AI segmentation, automatic face tracking, cloud accounts, arbitrary remote assets, and browser UI are outside the product scope.

## Next engineering priorities

1. Runtime/release hardening on named Windows hardware:
   - actual OBS final-canvas captures;
   - filter-order and scene-transition coverage;
   - resize/restart/device/source lifecycle checks;
   - frame-time, render-lag, CPU/GPU and memory measurements.
2. Compatibility evidence across selected OBS versions/backends.
3. Longer full-OBS animated soak.
4. Only after the procedural core is well-evidenced: broader bounded mask-provider work such as image/gradient providers.

## Documentation rule

- **README / PROJECT_STATUS:** current public product state.
- **RELEASE_NOTES:** what shipped in a tagged release.
- **PRODUCT_SPEC:** product requirements and non-goals.
- **ARCHITECTURE:** runtime contracts and decisions.
- **QUALITY_GATES:** evidence definitions and acceptance boundaries.
- **ROADMAP:** historical implementation ledger plus future sequencing.

When these disagree, update the current-state documents rather than rewriting historical evidence.
