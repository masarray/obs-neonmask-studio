# Original authored shape references (Phase D)

These SVGs are original NeonMask Studio geometric design references and carry
GPL-3.0-or-later headers. They are **not currently loaded by the runtime**.
The D1 native GPU shader independently evaluates equivalent parameterized
contours, so user X/Y, width/height, rotation and neon operate without asset
parsing or video readback. SVGs express art direction/proportions; they are not
a promise of pixel-identical correspondence at arbitrary source aspect ratios.

The D2 SVG mask provider must import only trusted local shape data, validate
viewBox/content, reject external resources/scripts and enforce complexity and
texture limits. Its alpha and derived distance field must share the same
contour for clipping and luminous border. See docs/PHASE_D_IMPLEMENTATION.md.

Do not import Advanced Masks' GPL-2.0-only source into this GPL-3.0 project
without file-level compatible permission. Feature parity and independently
authored silhouettes do not require source-code copying.
