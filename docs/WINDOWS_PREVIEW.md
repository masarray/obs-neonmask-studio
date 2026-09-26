# Windows x64 preview build

The Windows x64 preview CI builds a native x64 DLL against OBS Studio 31.1.1 using pinned official OBS source and prebuilt dependency archives. SHA256 pins come from the official OBS plugin template dependency snapshot.

On GitHub, open Actions > Windows x64 preview > successful workflow run > Artifacts > obs-neonmask-v0.1.0-windows-x64-preview. The artifact contains a portable plugin ZIP and its SHA256 file.

The ZIP contains these OBS paths:
- obs-plugins/64bit/obs-neonmask.dll
- data/obs-plugins/obs-neonmask/shaders/neon-mask.effect
- data/obs-plugins/obs-neonmask/locale/en-US.ini
- data/obs-plugins/obs-neonmask/locale/id-ID.ini

Extract contents of the inner ZIP into the root of an OBS Studio portable installation and merge folders. Restart OBS and add the filter to a video source via Filters > Effect Filters > +.

The binary is unsigned and open source. No admin access or signing certificate is needed for a portable test.

Successful CI proves compilation/linking, headless tests, and archive layout. It does NOT prove actual OBS Windows frontend loading, D3D11 shader compilation, visual fidelity, alpha behavior, or frame-time performance. Run-time checks remain mandatory before stable release.
