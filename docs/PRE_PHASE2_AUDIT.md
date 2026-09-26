# Pre-Phase-2 quality gate

## Source study and attribution

The reference [OBS Advanced Masks](https://github.com/FiniteSingularity/obs-advanced-masks) uses separate masking implementations and OBS GPU effects:
- `src/advanced-masks-filter.c`: lifecycle, dispatch, filter capture.
- `src/mask-shape.c` + `data/shaders/*-mask.effect`: shape parameters and signed-distance clipping.
- `src/mask-source.c`: mask image and OBS source references.
- `src/mask-svg.c`: SVG rasterization and texture management.
- `src/obs-utils.c`: filter texture and alpha/capture utilities.

We used these as architectural references, along with official libobs implementation.
**No upstream source was copied into this repository.** If importing GPL code later, retain notices and recheck license obligations. No hiding provenance or relabeling third-party work.

## Bugs addressed before expanding the feature set

1. OBS Color Picker provides RGBA-packed property values; `gs_effect_set_color` consumes BGRA-packed input. Use `vec4_from_rgba_srgb` and `gs_effect_set_vec4` to preserve intended hues.
2. OBS filter capture blends into premultiplied input. Avoid multiplying captured RGB by alpha twice; disable the direct-rendering shortcut so the shader's input contract is stable.
3. Use target **base** dimensions, matching libobs's filter texture allocation.
4. Reject partially bound GPU effects and bypass safely on shader load failure.
5. Presets are single-source-of-truth tested data, editable after application; a manual edit resets the selection to Custom.
6. Replace `assert()`-only C tests (which disappear in Release builds) with explicit failing checks.
7. Match CPU reference ellipse approximation and GPU formula, including center/invalid-radius guards.
8. Run release headless tests across Windows/Linux/macOS, a Linux UBSan pass, and a real libobs *compile* job. Static shader checks protect host/uniform drift.

## Remaining hard requirements for next phase

- Shader **runtime compilation** on OpenGL and D3D11, and live source/scene/filter-chain smoke tests are not yet certified by CI. Native compilation and static parsing alone cannot prove GPU correctness.
- Exercise semi-transparent input (hair, webcam virtual background), sRGB/non-sRGB and straight/premultiplied compositing visually in OBS.
- Glow still uses the source canvas and can clip at large radius/high scale; expanded render area or linked companion source is a separate feature.
- Polygon/ellipse path lengths and segmented animation are still approximations.
- Image/SVG/source masks, per-platform installer, Windows OBS artifact, and benchmark matrix belong to later phases.

Do not tag a production release solely on green headless/native compile CI.
