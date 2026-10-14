# Windows x64 public preview and CI package

OBS NeonMask Studio v0.1.0 publishes an unsigned Windows x64 portable OBS plugin package.

## Public release

For normal testing, use the permanent GitHub Release:

**https://github.com/masarray/obs-neonmask-studio/releases/tag/v0.1.0**

Download:

- `obs-neonmask-v0.1.0-windows-x64.zip`
- `obs-neonmask-v0.1.0-windows-x64.sha256`

The release also contains:

- `build-info.json` — exact checkout/workflow/SDK/package hash metadata;
- `obs-neonmask-M0-visual-fixtures.zip` and checksum — owned repeatable test inputs.

The Windows binary is currently **unsigned**. Browser/Windows reputation systems may warn even when the checksum matches.

## Package layout

The plugin ZIP contains the portable OBS layout:

```text
obs-plugins/64bit/obs-neonmask.dll
data/obs-plugins/obs-neonmask/
├── shaders/neon-mask.effect
├── locale/en-US.ini
├── locale/id-ID.ini
└── designs/
    ├── chat-bubble.svg
    └── angled-card.svg
```

Close OBS, extract the ZIP into the OBS installation or portable root, preserve the directory structure, then restart OBS.

Add the filter through:

**Source → Filters → Effect Filters → + → NeonMask Studio**

## CI preview artifact

Every successful `main` build also produces a short-lived Actions artifact from:

**Actions → Windows x64 preview**

Artifact name:

`obs-neonmask-v0.1.0-windows-x64-preview`

The **outer Actions artifact** contains the inner portable plugin ZIP, checksum, fixtures, and `build-info.json`. Extract the inner plugin ZIP into OBS; do not merge the outer Actions archive directly into the OBS installation.

CI artifacts are for development/testing and have limited retention. Prefer the tagged GitHub Release for a persistent public download.

## Build provenance

The Windows workflow builds against a pinned **OBS Studio 31.1.1 libobs SDK** assembled from:

- official OBS Studio 31.1.1 source;
- pinned official OBS dependency archive;
- SHA-256-verified downloads.

The workflow then:

1. builds the libobs development SDK and D3D11 graphics module;
2. compiles NeonMask x64 Release;
3. runs the full CTest set;
4. compiles/renders the NeonMask effect on libobs D3D11;
5. performs direct GPU framebuffer pixel fixtures;
6. stages the portable OBS plugin layout;
7. creates the plugin ZIP and SHA-256;
8. records exact checkout/workflow/package metadata in `build-info.json`.

Tagged/release-publish workflows can publish the verified package as a GitHub prerelease.

## What CI proves

A successful Windows workflow is evidence for:

- compilation and linking against the pinned SDK;
- headless config/geometry/preset/motion tests;
- package layout;
- D3D11 effect compilation;
- direct shader GPU pixel fixtures;
- package checksum/provenance.

It does **not**, by itself, prove:

- every OBS frontend version;
- every GPU/driver;
- every filter-chain/crop/chroma ordering;
- final scene-canvas appearance;
- frame-time/performance budgets;
- long-duration OBS runtime stability.

Those remain separate evidence gates in [QUALITY_GATES.md](QUALITY_GATES.md).

## Visual test fixtures

The separate M0 fixture ZIP is a set of test inputs, **not marketing screenshots**. Follow [M0_VISUAL_TEST.md](M0_VISUAL_TEST.md) for repeatable image-source/alpha/shape/restart checks.

For the current shipped/pending matrix, see [PROJECT_STATUS.md](PROJECT_STATUS.md).
