# GPU smoke test
The headless unit suite does **not** compile OBS's HLSL-like effect language.
`neonmask-gpu-smoke` creates a real libobs OpenGL device under Xvfb/Mesa, then calls
`gs_effect_create_from_file` on our packaged shader and checks the Draw technique
and the host's eighteen expected uniforms (16 host-set + 2 OBS-provided).

This is a GPU **compile** smoke, not an image/visual regression test.
Windows D3D11, filter chaining, capture-alpha behavior, glow clipping, and
streaming frame-time benchmarks remain separate release gates. Do not equate
successful compilation with verified visual output.

For local Linux usage, install libobs development/runtime, OBS Studio and Xvfb:

```sh
cmake -S . -B build-gpu -DNEONMASK_BUILD_SHADER_SMOKE=ON
cmake --build build-gpu
LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a ctest --test-dir build-gpu -R neonmask-gpu-smoke --output-on-failure
```
