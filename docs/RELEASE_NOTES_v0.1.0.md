# OBS NeonMask Studio v0.1.0 — Public Preview

First public preview release of OBS NeonMask Studio: a native GPU-rendered OBS video filter for facecam masking, neon borders, authored frame details and animated counter-orbits.

## Highlights

- Native OBS filter; no browser source or recurring texture upload pipeline.
- Circle, oval, rounded rectangle, rectangle, polygonal and authored analytic mask shapes.
- Integrated Chat Bubble, Angled Card, HUD Cut Panel and Squircle contours.
- Dual-color neon core, analytic glow, segmentation and animation.
- Context-aware OBS property UI with English and Indonesian localization.
- Mask-relative Dual Ring geometry that remains readable when the facecam is scaled down in a scene.
- **Rotating Ring — short counter-orbits**:
  - two 39%-turn light objects rather than near-complete rings;
  - outer CW / inner CCW motion;
  - cyan/primary-dominant outer object and magenta/secondary-dominant inner object;
  - tapered fading endpoints;
  - dedicated width/spacing controls relative to mask radius;
  - unrelated border/lighting controls hidden or disabled.
- Expanded output support for preserving external ornaments and glow without shrinking the portrait.
- Restricted local SVG-path mask preview for simple, closed path artwork.

## Windows package

The release ZIP contains the portable OBS plugin layout:

- `obs-plugins/64bit/obs-neonmask.dll`
- `data/obs-plugins/obs-neonmask/`

Extract the ZIP into the OBS installation or portable root, preserving the directory structure, then restart OBS.

In OBS:

**Source → Filters → Effect Filters → + → NeonMask Studio**

The Windows package is built against the pinned **OBS 31.1.1 libobs SDK**. Development testing has also exercised the plugin in a newer OBS frontend, but this preview does not claim a complete OBS compatibility matrix.

## Verification

Release CI includes:

- Windows, Linux and macOS headless geometry/config/preset tests;
- undefined-behavior sanitizer;
- native Linux plugin build;
- OpenGL/Mesa shader compilation and GPU pixel fixtures;
- Windows D3D11 effect compilation and direct GPU pixel fixtures;
- small-facecam Dual Ring readability, radial-gap, endpoint-fade and counter-motion gates;
- SHA-256 checksum for the Windows ZIP.

The release assets also include `build-info.json` and owned visual test fixtures.

## Important preview notes

- The Windows binary is **unsigned**. Windows/Chrome/SmartScreen may warn because the project does not yet use a commercial code-signing certificate.
- This is a **public preview**, not a claim of production certification.
- Back up important OBS scene collections before upgrading preview builds.
- Real-world visual acceptance still benefits from testing on the intended camera, source resolution, scene scale and GPU backend.

## License

GPL-3.0-compatible project licensing as documented in the repository.
