# GPU smoke test
The headless unit suite does **not** compile OBS's HLSL-like effect language.
`neonmask-gpu-smoke` creates a real libobs OpenGL device under Xvfb/Mesa, then calls
`gs_effect_create_from_file` on our packaged shader and checks the Draw technique
and the host's nineteen expected uniforms (17 host-set + 2 OBS-provided).

This is a GPU **compile** smoke, not an image/visual regression test.
Windows D3D11 compilation is also exercised in CI. Full frontend usage, filter chaining,
capture-alpha behavior, glow clipping, and streaming frame-time benchmarks remain separate gates. Do not equate
successful compilation with verified visual output.

For local Linux usage, install libobs development/runtime, OBS Studio and Xvfb:

```sh
cmake -S . -B build-gpu -DNEONMASK_BUILD_SHADER_SMOKE=ON
cmake --build build-gpu
LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a ctest --test-dir build-gpu -R neonmask-gpu-smoke --output-on-failure
```

Windows CI uses NEONMASK_GRAPHICS_MODULE to select the built OBS Direct3D 11 renderer, and NEONMASK_LIBOBS_DATA_DIR to locate OBS's default effects. This is a real shader compiler/device initialization check on the Windows GitHub runner; a full OBS frontend filter interaction and pixel-diff visual test remain separate gates.
