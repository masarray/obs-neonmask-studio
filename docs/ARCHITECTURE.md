# Runtime architecture and decisions

Status: target architecture, anchored to the C11 baseline in [PRODUCT_SPEC.md](PRODUCT_SPEC.md).
These boundaries are introduced incrementally; they are not a claim that the current monolithic filter already implements them.

## Decisions

| ADR | Decision | Reason / revisit condition |
| --- | --- | --- |
| 001 | Native C11/libobs filter; retain current engine | Lowest migration risk; change language only for a demonstrated subsystem need |
| 002 | GPU effects render video; no worker per frame or per filter | Worker count does not reduce pixel cost; add a bounded service when asset loading exists |
| 003 | One validated config and one geometry contract | UI, presets, masks and ornaments cannot diverge |
| 004 | Procedural analytic halo first | Avoid full-frame blur and additional textures for simple shapes |
| 005 | Safe-fit within original canvas first | Expanded output changes coordinates/filter-chain behavior; requires its own proof |
| 006 | Explicit ownership, bounded caches, cancellable jobs | Control lifetime and resource growth before adding external assets |
| 007 | Fixed quality initially; opt-in adaptive quality later | Predictable streaming appearance; profile before adding a controller |
| 008 | Reject arbitrary shaders and remote asset fetching | Keep import behavior deterministic and supportable |
| 009 | No universal crash-proof claim | A native plugin shares the OBS process; a worker thread is not crash isolation |

## Modules and dependency direction

Keep the current file names while extracting cohesive responsibilities. Avoid a framework of empty modules.

| Boundary | Responsibility | Allowed dependencies |
| --- | --- | --- |
| OBS adapter (current neonmask-filter.c) | Callbacks, properties, source integration | Core config, renderer, asset service |
| Config/presets | Defaults, validation, migration, stable enum IDs | Pure C data/math; no graphics |
| Geometry (current neonmask-math.*) | Source-space dimensions, contour, safe bounds | Pure C math |
| Render backend | Effect bindings, pass selection, graphics resources | Validated config/geometry, libobs graphics |
| Mask providers (later) | Analytic coverage or cached field texture | Geometry; bounded asset data |
| Ornament evaluator | Contour-relative rims, ticks, dashes and motion | Geometry; cannot redefine mask coverage |
| Asset service (later) | Decode/rasterize, generation tracking, cache | CPU-only jobs; no borrowed OBS pointers |

Dependency rule: core math/config must remain testable without OBS. Effects have an explicit host/uniform contract. Add modules when implementing their first real feature.

## Canonical state

Canonical means an authoritative representation, not a special optimization technology.

Settings flow: OBS settings → migrate → validate all fields → complete immutable config snapshot → derived geometry/render parameters. Apply presets through this same path. Rendering must never read a partly updated configuration or parse user files.

The config schema owns field name, type, unit, default, valid range and enum identity. Use one schema table where practical for host validation/UI bounds; test GPU enum and uniform agreement. Keep current keys and enum numeric values stable. Add schema_version with migration of missing-version scenes, never reinterpret old enum values. Save explicit user values so changed preset defaults do not rewrite old scenes. Reject unsupported future import versions without overwriting the working configuration.

Reject NaN/infinity, invalid dimensions and integer overflow before allocation or shader upload. Clamp ordinary out-of-range controls consistently; report invalid imported data. Colors, feather and border units must be explicit. Defaults and preset application must cover every intended field, including speed/glow toggles, without accidental dependence on the previous preset.

OBS callback scheduling must be checked against the pinned SDK before assuming serialization. Prefer the host's documented update scheduling for video data. For any genuine cross-thread producer, publish a complete snapshot with a lifetime-safe handoff; an atomic pointer alone does not solve reclamation. No blocking lock or destruction of a large payload in the render callback.

## Geometry, contours and padding

Define geometry in source pixels: center, half extents, rotation, corner parameters and transform. A mask provider yields coverage and a distance/contour representation. Border width is in source pixels; document that scene scaling scales the resulting border. Screen-pixel-invariant borders are a separate feature.

For procedural shapes, clipping, core and halo consume the same signed distance. The existing polygon/ellipse formulas are approximations; measure edge/corner errors before extending them. Offset contours can change corner shape; test sharp and rounded joins.

Perimeter animation uses normalized arc length s in [0,1), contour length, tangent and outward normal, with a defined seam. Angle around the center is acceptable for the existing preview but is not uniform travel around a rectangle or ellipse. Use analytic length where cheap and bounded cached lookup tables otherwise. Rebuild those tables only when geometry changes. Multiple disconnected contours in imported masks require an explicit phase policy; initially disallow flow on providers without a validated contour rather than pretending angle is arc length.

Safe-fit design: compute the entire effect envelope, including outer rim offset, half stroke width, halo cutoff, feather, antialias margin and ornament excursion. For each edge, required margin must not exceed available canvas margin. For a Gaussian-like halo, derive a finite cutoff at a declared alpha threshold; a radius slider is not the support radius. Use exact bounding boxes for rotated/asymmetric shapes.

When an effect cannot fit, offer a predictable fit adjustment or report the limit. Do not silently stretch video. Preserve existing scene behavior via an explicit legacy/safe-fit migration choice.

Expanded padding is a later experiment: distinguish input dimensions, output dimensions, source UV transform and scene anchor; explicitly zero out samples outside the original input so clamp sampling cannot smear the webcam edge. Test crop/filter order, transform bounds and transitions. A linked companion source is a fallback design with lifecycle/scene-sync costs, not a free optimization.

## Render and color contract

The current path uses libobs filter capture with direct rendering disabled and one custom effect pass. “One pass” means one NeonMask composition pass in addition to capture/OBS work, not one total GPU operation.

Preserve the existing intended captured premultiplied-input → masked premultiplied composition → final output convention until pixel tests confirm the selected OBS blend path. Do not multiply premultiplied input by alpha twice. Convert OBS color property packing correctly; document working color space and texture sampling. Every added render target records format, color space and alpha convention. SDR is the first supported pipeline; unsupported HDR must be documented and handled predictably.

Re-use effects, uniforms, textures and render targets. Compile only on controlled initialization/rebuild; validate technique and all required uniforms before publishing a resource set. No file I/O, decode, synchronous readback, recurring allocation or shader compilation in steady-state rendering. Diagnostic readback belongs in tests or explicit profiling only.

Static mode freezes all motion, including color phase; speed changes must be continuous. Use bounded phase accumulation or stable time mapping with a defined wrap. Long sessions must not produce float jitter or a visible one-hour reset. Inactive sources should not keep asset work running; do not bypass OBS visibility semantics when Studio Mode previews still need rendering.

## Worker service, introduced only with assets

Start with one shared worker and a bounded queue; proposed cap is eight pending jobs globally and one newest pending generation per filter. Profile before increasing parallelism.

1. UI submits an owned CPU request with filter lifetime token, asset identity and generation.
2. Worker checks cancellation, validates file size/format, decodes/rasterizes within budgets and produces owned CPU bytes.
3. Graphics-side consumer accepts only the newest live generation, validates size again, uploads in the proper context and swaps a complete resource set.
4. Stale results are discarded; old resources are released by their owner after the handoff.

A cache key includes content identity, raster size, provider parameters and algorithm version. Recompute only dirty geometry/assets, not on color or time changes. Static images may cache a distance field. Live masks cannot cache indefinitely; version/throttle explicitly and include their distance-field pass cost in benchmarks. Do not put jump-flood generation in the default procedural path.

On destruction: invalidate lifetime token, cancel pending work, prevent publication, release owned CPU results, retire graphics resources in context, and release source references. Join the shared worker on module shutdown only after cancellation; never join while holding the graphics context or a lock a worker needs. Keep decoder inputs bounded because cancellation may not interrupt a third-party decode immediately. Workers never retain raw filter pointers after destruction.

## Ownership and failure behavior

| Resource | Owner and cleanup |
| --- | --- |
| Filter state | OBS create/destroy pair; partial initialization is safe |
| Effect and uniform handles | Renderer owns effect; handles die with it; destroy in graphics context |
| Textures / render targets | Renderer/cache owns bounded resources; resize releases replaced resources |
| Borrowed filter target | Callback-local; never keep as an unowned asynchronous pointer |
| Mask-source reference | Later provider uses documented weak/strong refs with matched release; reject self/cycles |
| Decode buffers / queued jobs | CPU job/result owner; cleanup on cancel, failure, supersession and shutdown |
| Settings snapshots | Explicit reader lifetime; old generation reclaimed only after readers finish |

Initial proposed asset limits: 16 MiB file, 4096 × 4096 raster maximum, one decoded RGBA surface up to 64 MiB, and 128 MiB global retained CPU asset cache; also bound concurrent decode memory. Check backend texture limits and overflow before allocating. SVG imports disable external resources/scripts, cap path complexity and raster size; dynamic sources reject direct and indirect reference cycles. These are implementation targets to validate at the asset milestone.

| Failure | Required response |
| --- | --- |
| Missing shader / invalid bindings | Preserve current preview bypass behavior, log once; never call invalid effect |
| Invalid new preset/asset | Keep last valid state and show concise error |
| No valid custom mask at startup | Default to transparent output with warning, avoiding accidental privacy disclosure |
| Zero-size/unavailable source | No invalid allocation/draw; recover when dimensions become valid |
| Allocation/resource failure | Retain valid state if usable; otherwise documented safe fallback |
| Device/context failure | Stop using invalid resources; recreate only through supported host lifecycle |
| Destroy during loading | Cancel and discard late result; no use-after-free |

For future custom masks, expose explicit failure policy: Hide (default), Last valid, or Bypass. Last valid is usable only while valid resources exist. Never silently reveal an unmasked region. Shader failure in existing procedural scenes remains legacy bypass until an explicit compatible policy is introduced.

## Primary references

- [OBS graphics context and effects](https://docs.obsproject.com/graphics): graphics operations require the host context; render callbacks enter it automatically.
- [OBS source API](https://docs.obsproject.com/reference-sources): callbacks, filter processing and source references.
- [Existing implementation at baseline](https://github.com/masarray/obs-neonmask-studio/tree/039169cf1b30c9e20a17b9393e0b5de5c708c008/src).
- [Advanced Masks reference revision](https://github.com/FiniteSingularity/obs-advanced-masks/tree/7e80d7a5b58e3f039172bf8a647d045bc0d42997).

OBS web documentation may describe a newer version than our pinned 31.1.1 Windows SDK. Confirm each API against that SDK before implementation.
