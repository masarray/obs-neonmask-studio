# Changelog

All notable public-preview changes are summarized here. The detailed engineering chronology remains in [docs/ROADMAP.md](docs/ROADMAP.md).

## [0.1.0] — 2026-10-14

First public preview.

### Added

- Native OBS facecam mask/neon filter with OBS-native properties.
- 12 editable built-in presets.
- Independent mask sizing/placement and subject pan/zoom.
- Authored Bubble, Angled Card, HUD Cut Panel, Tech HUD, Game UI, and Squircle geometries.
- Solid, dual-color, and animated rainbow pipelines.
- Safe-fit and optional expanded output for outer ornaments/glow.
- Restricted local SVG-path mask preview.
- English and Indonesian localization.
- Windows x64 portable release package with SHA-256 and build metadata.

### Rotating Ring / Dual Ring

- Mask-relative ring geometry.
- Two short 39%-turn external light objects.
- Outer CW / inner CCW counter-motion.
- Primary/secondary object identity and tapered fading endpoints.
- Context-aware UI that hides or disables unrelated controls.
- Regression coverage for small-facecam readability, radial/angular gaps, hotspot suppression, and hidden-border decoupling.

### Verification

- Windows/Ubuntu/macOS headless tests.
- UBSan.
- Native Linux plugin build.
- OpenGL/Mesa direct-effect shader/pixel smoke.
- Windows OBS 31.1.1 / D3D11 direct-effect shader/pixel smoke.

### Preview limitations

See [docs/PROJECT_STATUS.md](docs/PROJECT_STATUS.md) and [docs/QUALITY_GATES.md](docs/QUALITY_GATES.md). The project does not yet claim a broad compatibility matrix, production certification, or completed long-duration OBS soak/performance evidence.
