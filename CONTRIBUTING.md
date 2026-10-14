# Contributing to OBS NeonMask Studio

Thank you for helping improve NeonMask Studio.

The project values **small, evidence-backed changes** over broad rewrites. Visual work must preserve scene/config compatibility and should be tested at both filter-preview scale and realistic small-facecam scene scale.

## Before you start

1. Read [docs/PROJECT_STATUS.md](docs/PROJECT_STATUS.md).
2. Read the relevant contracts:
   - [PRODUCT_SPEC.md](docs/PRODUCT_SPEC.md)
   - [ARCHITECTURE.md](docs/ARCHITECTURE.md)
   - [QUALITY_GATES.md](docs/QUALITY_GATES.md)
   - [VISUAL_SPEC.md](docs/VISUAL_SPEC.md)
3. For non-trivial changes, open an issue or clearly describe the problem/acceptance criteria in the pull request.

## Development principles

- Keep the native C11/libobs architecture unless a subsystem has a demonstrated need to change.
- Preserve the OBS source ID, persisted keys, enum numeric identities, and additive migration semantics.
- Keep config/math logic testable without OBS where practical.
- Use one canonical geometry/config authority; do not duplicate derived formulas across UI, safe-fit, and shader setup.
- Avoid blocking work, recurring CPU readback/upload, unbounded allocation, arbitrary remote fetching, and user-provided shaders.
- Do not silently weaken a quality/visual target because the current implementation is easier.
- Do not present concept art or synthetic fixtures as proof of final OBS output.

## Build and test

Headless:

```sh
cmake -S . -B build-test \
  -DNEONMASK_BUILD_PLUGIN=OFF \
  -DNEONMASK_BUILD_TESTS=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-test --config Release --parallel
ctest --test-dir build-test -C Release --output-on-failure
```

Native plugin builds additionally require a matching libobs development SDK.

Before requesting review, run the tests relevant to your change. Pull requests are also checked by:

- cross-platform headless CI;
- UBSan;
- native Linux plugin build;
- OpenGL/Mesa shader smoke;
- Windows/D3D11 plugin + shader smoke.

## Pull requests

A good pull request includes:

- problem statement;
- scope and non-goals;
- compatibility/migration impact;
- test evidence;
- visual before/after evidence when appearance changes;
- known remaining limitations.

Keep unrelated cleanup out of functional changes.

### Visual changes

For shader/preset/geometry work:

- test dark and light content;
- test normal and small facecam sizes;
- inspect transparency/alpha edges;
- verify controls actually affect the selected mode;
- avoid introducing hidden coupling between unrelated controls;
- update GPU/contract tests when the visual rule is deterministic enough to encode.

Human aesthetic acceptance still matters; pixel tests are regression evidence, not taste.

## Bugs

Please include:

- OBS version;
- OS version;
- GPU and graphics backend if known;
- source type/resolution;
- NeonMask preset/settings involved;
- exact reproduction steps;
- expected vs actual behavior;
- OBS log and screenshots/video where useful.

Never post private credentials, stream keys, or sensitive recordings.

## Security

Do not file a public issue for a security vulnerability. Follow [SECURITY.md](SECURITY.md).

## License

By contributing, you agree that your contribution may be distributed under the repository's GPL-compatible licensing terms. Do not submit code, assets, fonts, presets, or artwork that you do not have the right to redistribute.
