# Windows x64 preview build

The Windows x64 preview CI builds a native x64 DLL against OBS Studio 31.1.1 using pinned official OBS source and prebuilt dependency archives. SHA256 pins come from the official OBS plugin template dependency snapshot.

On GitHub, open Actions > Windows x64 preview > successful workflow run > Artifacts > obs-neonmask-v0.1.0-windows-x64-preview. The artifact contains a portable plugin ZIP and its SHA256 file, an M0 owned visual-fixture ZIP with SHA256, and build-info.json identifying the exact checkout commit, workflow run, pinned SDK and both ZIP hashes.

The ZIP contains these OBS paths:
- obs-plugins/64bit/obs-neonmask.dll
- data/obs-plugins/obs-neonmask/shaders/neon-mask.effect
- data/obs-plugins/obs-neonmask/locale/en-US.ini
- data/obs-plugins/obs-neonmask/locale/id-ID.ini
- data/obs-plugins/obs-neonmask/designs/chat-bubble.svg (original local-only import example)
- data/obs-plugins/obs-neonmask/designs/angled-card.svg (original local-only import example)

Extract contents of the **inner plugin ZIP** into the root of an OBS Studio portable installation and merge folders. Do not extract the outer Actions archive directly into OBS. Restart OBS and add the filter to a video source via Filters > Effect Filters > +.

Follow [the M0 visual test procedure](M0_VISUAL_TEST.md) and extract the **separate fixture ZIP** outside OBS for repeatable image-source, alpha, shape, preset and restart checks. Save the actual OBS output and OBS log with the artifact metadata. The fixture PNGs are test inputs, not plugin screenshots.

The binary is unsigned and open source. No admin access or signing certificate is needed for a portable test.

Successful CI proves compilation/linking, headless tests, archive layout, real libobs D3D11 effect compilation and direct shader framebuffer samples. It does NOT prove OBS Windows frontend interaction, final canvas pixels, capture-path alpha behavior, filter chaining, or frame-time performance. Those runtime gates remain mandatory before stable release. See [GPU smoke details](GPU_SMOKE.md).

Verified preview example: [PR #6 Windows/D3D11 run](https://github.com/masarray/obs-neonmask-studio/actions/runs/36236980372) (head SHA 80cfce3569f0ef7187109b29c9d6f26f21dd67d0). Artifact ID 10904645446, uploaded after all four CTests passed, including D3D11 shader compilation. Artifact retention is 14 days; use the latest successful main run thereafter.
