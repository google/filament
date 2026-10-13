# taa_harness — for agents

Headless, deterministic measurement tool for Filament's TAA. Source: [taa_harness.cpp](taa_harness.cpp)
— its own top-of-file comment covers *how* the tool works (schedule shape, reference
construction, alignment guards); this file covers *how to drive it and read its output*
without re-deriving that from the code.

Use this tool whenever you are changing `filament/src/materials/antiAliasing/taa/taa.mat`,
`TemporalAntiAliasingOptions` (`filament/include/filament/Options.h`), or the TAA resolve path
in `filament/src/PostProcessManager.cpp` / `filament/src/details/Renderer.cpp`, and you need to
know whether the change helped, hurt, or did nothing measurable. Do not judge a TAA change by
eyeballing `gltf_viewer` — that loop is exactly what this tool exists to replace; see the
rationale at the top of `taa_harness.cpp` if you want the full argument.

## Build it

```bash
ninja -C out/cmake-debug taa_harness
```

This alone is sufficient after editing `taa.mat` — it is a source of `libfilament`, and the
dependency chain (material compile → resource package → `taa.cpp` → `libfilament.a` →
`taa_harness`) is wired correctly through ninja. No need for a full `build.sh`.

## Before/after workflow for a TAA change

1. `tools/taa/taa_harness_baseline.json` in this directory is a checked-in run against the
   `main`/pre-change state, produced by the command below. Do not regenerate it before your
   change — it is already there.
2. Make your change to the TAA material/options/resolve code.
3. `ninja -C out/cmake-debug taa_harness`
4. Run the **same command** as the baseline (below), redirected to a new file, e.g.
   `-o /tmp/after.json` instead of `-o tools/taa/taa_harness_baseline.json`.
5. Diff the two JSONs' `records` arrays by `(regime, upscaling, metric)`. A few percent of
   movement in `edge_rmse_mean`/`sharpness_mean`/`edge_ghost_peak` is real; treat anything smaller
   than the noise floor (see Gotchas) as noise.
6. If the change is a genuine, intentional improvement (or a deliberately accepted regression),
   regenerate `tools/taa/taa_harness_baseline.json` with the same command and include it in your
   commit, so the baseline always reflects `main`'s current behavior. If you can't explain a
   movement, don't update the baseline — find out what happened first.

The canonical baseline command (also recorded, verbatim, in its own file header fields):

```bash
./out/cmake-debug/tools/taa/taa_harness -e -S 512 -M 4 -F lanczos-filament -q \
    -o tools/taa/taa_harness_baseline.json third_party/models/Fox/Fox.glb
```

Takes about 2.5 minutes on Apple silicon (Metal). For quick iteration while narrowing down a
change, drop to `-F box` (~25s, ~2600 frames vs ~17000) — box is a legitimate reference for
*relative* comparisons (did this run get better or worse than that run), it is just not the
reference to trust for an *absolute* claim about how close TAA is to its target; see Gotchas.

## Running faster: split across processes by hand

The harness is single-process; `-r`/`-u` sweeps run sequentially inside it. Splitting the sweep
across several `taa_harness` invocations and running them in parallel does help, but not by as
much as the process count suggests — measured on the box-filter default sweep (23.0s serial, one
GPU, M3 Max/14 cores):

| processes | split | wall time | speedup |
|---|---|---|---|
| 1 | (serial) | 23.0s | 1.0x |
| 2 | by upscaling | 12.2s | 1.9x |
| 3 | by regime | 11.2s | 2.1x |
| 6 | by (regime, upscaling) | 8.2s | 2.8x |

Gains flatten hard past 2-3 processes: a single shared GPU, not CPU, is the bottleneck once a
couple of processes are hitting it concurrently — the same submission/readback latency this tool
spent a while optimizing away inside one process shows up again as contention across processes.
This is why the harness does not fork internally: the complexity of spawning/managing children
and merging their output isn't worth chasing a ceiling under 3x that's mostly captured at 2-way.

Two-way split, backgrounded and merged:

```bash
./out/cmake-debug/tools/taa/taa_harness -e -u 1 -F box -q -o /tmp/u1.json third_party/models/Fox/Fox.glb &
./out/cmake-debug/tools/taa/taa_harness -e -u 2 -F box -q -o /tmp/u2.json third_party/models/Fox/Fox.glb &
wait
```

Any command-line option that takes a comma-separated list (`-r`, `-u`) can be split this way —
give each process a disjoint subset and run them concurrently. Merge the resulting JSONs (they
share the same header fields; only `records` needs concatenating) with:

```bash
python3 -c '
import json, sys
merged = None
for path in sys.argv[1:]:
    d = json.load(open(path))
    if merged is None:
        merged = d
    else:
        merged["records"] += d["records"]
json.dump(merged, open("/tmp/merged.json", "w"), indent=2)
' /tmp/u1.json /tmp/u2.json
```

## What a run is

One `(regime, upscaling)` pair. `-e` (headless) with no `-r`/`-u` sweeps all 3 regimes ×
{1:1, 2:1} = 6 runs. Regimes:

| regime | what moves | what to expect |
|---|---|---|
| `static` | nothing | ghost-free by construction; measures flicker and steady-state sharpness |
| `camera` | camera (orbit or dolly, see `--camera-motion`) | expected ghost-free; disocclusions should resolve in a handful of frames |
| `object` | glTF animation | interior ghosting on the moving object is **expected and not a bug** — Filament has no per-object motion vectors. What should still recover is the dis/occluded silhouette. |

Motion runs for the first half of the capture window then **stops** — frames past the midpoint
of `--frames` are a static hold, not moving footage. This is deliberate (it turns the second half
into a clean recovery curve); if you're staring at dumped PNGs wondering why the back half looks
frozen, that's why, not a bug.

## Test scenes

`third_party/models/` holds the real-content scenes; `tools/taa/models/` holds synthetic ones added
to isolate a single TAA failure, because a real model mixes several at once and none of them
cleanly.

| scene | what it is for |
|---|---|
| `third_party/models/Fox/Fox.glb` | the default, and what the checked-in baseline is generated from. Thin limbs, hard silhouettes, and the only scene here with animation — so the only one whose `object` regime measures anything. **`-A` matters**: 0 (`Survey`, the default) barely moves, 1 is `Walk`, 2 is `Run`. |
| `third_party/models/DamagedHelmet/DamagedHelmet.glb` | specular and normal-mapped content Fox has none of; it is what made two upscaling bugs visible that Fox alone missed. No checked-in baseline — generate one before leaning on it. |
| `tools/taa/models/SubpixelStar/SubpixelStar.glb` | sub-pixel flicker on thin high-contrast geometry (below). |
| `tools/taa/models/ParallaxPillar/ParallaxPillar.glb` | a chroma-distinct silhouette edge under camera parallax. |
| `tools/taa/models/SSRReflection/SSRReflection.glb` | chroma motion from a view-dependent reflection on a geometrically static surface. |

Only Fox has animation. On every other scene the `object` regime plays nothing and therefore
measures exactly what `static` does — identical numbers, not a second data point. Don't read them
as one.

### Ring-binned flicker, with `SubpixelStar`

A 256-spoke Siemens star on an unlit backdrop: wedge width falls off linearly toward the centre,
so one static frame holds a continuous sweep from resolved features down to far below a pixel, at
every orientation, and contrast is exactly 1.0 regardless of the harness's IBL and sun. The
generator is checked in beside it — spoke count sets which feature widths land on screen, and
needs retuning for any `-S` other than 512.

The aggregate metrics are the wrong tool here, for the reason under `flicker_variance` above: the
failure lives in a few thousand pixels near the centre. Dump a full jitter cycle and bin the
per-pixel temporal standard deviation by radius instead — that turns the sweep into a curve of
flicker against feature width, which is what makes the failure legible:

```bash
./out/cmake-debug/tools/taa/taa_harness -e -r static -u 1 -n 16 -F box -R 8 -q \
    -d /tmp/star/ -o /tmp/star.json tools/taa/models/SubpixelStar/SubpixelStar.glb
```

```python
import numpy as np; from PIL import Image
f = np.stack([np.asarray(Image.open(f"/tmp/star/static_x1/capture_{i:02d}.png").convert("L"))
              for i in range(16)]).astype(np.float32) / 255.
std = f.std(0)
Y, X = np.mgrid[0:512, 0:512]
R = np.hypot(X - 256.0, Y - 275.0)          # star centre at -S 512
for lo, hi in [(14,22), (22,34), (34,52), (52,78)]:
    width = (lo + hi) * 0.5 * 2 * np.pi / (2 * 256)   # wedge width in px at that radius
    print(f"{width:5.2f} px  {std[(R >= lo) & (R < hi)].mean():.4f}")
```

Reference numbers at the time of writing, to tell a regression from the expected shape:

| config | 0.22 px | 0.34 px | 0.53 px | 0.80 px |
|---|---|---|---|---|
| `boxClipping: NONE` | 0.0364 | 0.0357 | 0.0305 | 0.0232 |
| defaults | 0.0595 | 0.0602 | 0.0746 | 0.0235 |
| `motionAdaptiveHistory: true` | 0.0131 | 0.0135 | 0.0121 | 0.0078 |

Two things that shape are meant to show. The history box makes flicker **worse** than no clipping
at all, but only below about 0.7 px — at 0.80 px and above `defaults` and `NONE` agree to four
decimals, so anything that moves those columns is not this effect. And `motionAdaptiveHistory` is
what fixes it, landing below `NONE`, which is the case it exists for: a change that looks fine on
Fox and quietly puts the third column back up near `defaults` has broken it.

## Metric glossary (with the gotcha that goes with each)

- **`edge_rmse_mean`** / **`edge_rmse_frame_N`** — error against the reference, masked to
  silhouette texels. **Gotcha:** frame 0 (pre-motion, fully converged) is *not* zero. That's the
  floor contributed by the reference definition itself (reconstruction-kernel mismatch, 8-bit
  quantization) — read `edge_ghost_peak` for the part actually caused by motion.
- **`edge_ghost_peak`** — worst captured frame's edge error, **with the frame-0 floor already
  subtracted out**. This is "how bad did it get," isolated from reference-definition noise.
- **`edge_recovery_frames`** — frames after motion stops until edge error returns to within 25%
  of the peak-minus-floor transient. **`-1` means it never recovered within the capture window** —
  that's a real finding, not a missing value; don't filter it out as `NaN`.
- **`sharpness_mean`** / **`sharpness_frame_N`** — high-frequency energy vs the reference, as a
  ratio. **Gotcha: a large relative change here can be an invisible absolute change.** It is a
  ratio of high-frequency *energy*, and that is a small fraction of a typical image, so a big
  move in it is a sub-LSB move nearly everywhere. Measured: a change that took `sharpness_mean`
  from 1.10 to 1.40 -- 27% -- altered the image by a mean of 0.35/255, with 0.4% of pixels
  differing by more than 8/255. Never conclude a sharpness change is worth shipping from this
  number alone; dump frames with `-d` and take a per-pixel delta. **Gotcha: 1.0 is not "perfect."** It means "matches whatever kernel `--reference-filter`
  integrated against." With the default `lanczos-filament` (the actual polynomial `taa.mat`
  evaluates, not an idealized Lanczos-2), 1.0 would mean TAA hits what it's targeting exactly;
  the current baseline sits close to that target in every regime (roughly 0.97–1.14 -- see the
  next gotcha for why it can land above 1.0), which says TAA closely matches its own polynomial's
  intent, not that it matches an idealized Lanczos-2 -- run `-F lanczos` to check that instead,
  where the baseline currently sits a bit lower (roughly 0.86–1.01). Never compare a `sharpness_mean`
  computed under one `--reference-filter` to one computed under another — they're different
  kernels, not just different precision.
- **`edge_sharpness_mean`** / **`edge_sharpness_frame_N`** — the same high-frequency-energy ratio
  as `sharpness_mean`, restricted to the silhouette mask. Whole-frame sharpness is a poor stand-in
  for this: silhouettes are typically only a few percent of the frame (see `edge_fraction`), so
  `sharpness_mean` is almost entirely driven by non-edge content and can't tell you whether edges
  specifically are soft under motion. **Gotcha: normalized differently from `sharpness_mean`.**
  Whole-frame sharpness divides by a single, fixed frame-0 reference energy for every frame in the
  series; this metric divides frame k's edge-masked energy by frame k's *own* reference's
  edge-masked energy, because the mask is a union across the whole motion sweep (see
  `edge_recovery_frames`'s note on the mask) -- most of that union is background or object-interior
  at any single frame, so frame 0's edge content there is a poor stand-in for what the silhouette
  actually looks like once the sweep reaches it. `edge_rmse_frame_N` already makes this same choice
  for the same reason. Don't compare `edge_sharpness_mean` numerically against `sharpness_mean` as
  if they were on the same scale -- they usually land close (both are ~1.0-centered ratios against
  a converged reference) but for different reasons, and treating a gap between them as meaningful
  on its own, without checking whether it holds up per-frame, invites reading noise as signal.
- **`flicker_variance`** / **`edge_flicker_variance`** — temporal variance across the capture
  window on `static`. Only meaningful over a full jitter cycle (default 16 frames = one
  `HALTON_23_X16` cycle); a shorter `--frames` measures jitter phase, not stability.
  **Gotcha: both are whole-frame means, and flicker is almost never whole-frame.** It
  concentrates wherever the content is hardest -- sub-pixel geometry, a silhouette, a specular
  highlight -- which is usually a small percentage of the pixels, so a change that triples the
  flicker where it lives can move these numbers by a third and read as ordinary noise. A change
  aimed at flicker needs a metric masked to the region it targets (see "Test scenes" for the
  ring-binned version) before either an improvement or a regression can be believed.
- **`reference_alignment_x/y/gain`** — self-check, not a TAA metric. `gain` is the fraction of
  error a sub-texel realignment would remove; near-zero means the reference is correctly centred.
  **If the harness prints a `WARNING` about this, every other number in that run is unreliable**
  — stop and fix the reference construction (or increase `--reference-samples`/`--warmup`)
  before trusting anything else in the file.
- **`msaa_*`** (only present with `--msaa N`) — cross-check against an independently rendered
  hardware-MSAA reference, sharing none of the harness's own jitter math. `msaa_vs_reference_edge_rmse`
  is the **noise floor**: two legitimate references disagreeing with each other. A TAA error
  below this number is not distinguishable from measurement noise; only trust conclusions drawn
  from errors clearly above it. In the checked-in baseline this floor is ~0.02-0.026 depending on
  regime; 1:1 TAA-vs-reference error sits below it in every regime, 2:1 sits clearly above it in
  every regime.
- **`aliased_*`** (only meaningful for `--camera-motion rotate --jitter-pattern helix4` at 2:1) —
  exactness check against a plain non-antialiased render at output resolution. Only interpretable
  in that specific configuration; ignore it otherwise.

## CLI cheat sheet (defaults as of this file; `--help` is authoritative)

| flag | default | for |
|---|---|---|
| `-r static,camera,object` | all three | which regimes to run |
| `-u 1,2` | `1,2` | upscaling factors |
| `-S` | 512 | output resolution (square) |
| `-F box\|lanczos\|lanczos-filament` | `lanczos` | reference reconstruction kernel — see Gotchas above |
| `-M` | 0 (off) | MSAA cross-check sample count; capped at 4 by the CLI (8x silently renders identically to 4x on hardware without 8x support, e.g. Apple silicon, so the flag doesn't offer it) |
| `-W` | 64 | warmup frames before motion; measured-converged, don't lower casually |
| `-n` | 16 | capture frames (should stay a full jitter cycle for flicker metrics) |
| `-R` | 0 (auto: 32 for `box`, 256 for either Lanczos variant) | reference accumulation samples |
| `--camera-motion rotate\|translate` | `rotate` | orbit vs pure dolly (parallax only, no rotation) |
| `--camera-speed` | 90 (deg/s) | rotate-mode angular speed |
| `--camera-texels-per-frame` | 3.5 | translate-mode parallax speed, in render-resolution texels/frame at the target's depth |
| `--camera-translation-align as-requested\|pixel\|subpixel` | `as-requested` | isolates history-buffer resampling specifically — `pixel` = every reprojection an exact texel fetch, `subpixel` = every reprojection a half-texel fetch |
| `-A` | 0 | which glTF animation the `object` regime plays — **check this**, the default is often not the one that moves; see Test scenes |
| `--object-speed` | 1.0 | animation playback multiplier, object regime |
| `-d <dir>` | off | dump captures/references/amplified diffs/mask as PNGs — use this to actually *look at* a discrepancy the numbers flag |
| `-s <settings.json>` | off | full `TemporalAntiAliasingOptions` via the same JSON `gltf_viewer`'s viewer reads/writes (`enabled`/`upscaling` are schedule-owned and ignored) |
| `-q` | auto | suppress the progress bar (auto-on already when stderr isn't a terminal) |
| `--visualize-disocclusion` | off | replace every frame with `taa.mat`'s disocclusion rejection mask (white = history fully rejected) -- pair with `-d`, and ignore that run's metrics |

## JSON output shape

```json
{
  "model": "...", "backend": "3",   // 3 = Metal; see backend::Backend in DriverEnums.h
  "size": 512, "warmup_frames": 64, "capture_frames": 16,
  "reference_filter": "lanczos-filament", "reference_samples": 256, "msaa": 4,
  "camera_motion": "rotate", "camera_speed_deg_per_sec": 90,
  "records": [
    { "regime": "static", "upscaling": 1, "metric": "edge_rmse_mean", "value": 0.0212 },
    ...
  ]
}
```

Flat `(regime, upscaling, metric) -> value` rows — trivial to load and filter with any JSON
library; no nested schema to navigate.

## Things that look like bugs and aren't

- Sharpness above 1.0 (seen in most regimes of the current baseline under the default
  `lanczos-filament` reference, e.g. static 1:1 sits at 1.07) — TAA's reconstruction filter can
  legitimately be sharper than the reference kernel; it is a ratio, not a bounded score, and
  above 1.0 is not an error.
- `edge_recovery_frames: -1` — did not recover within the window, a real (bad) result.
- Back half of a `--dump` capture sequence looking frozen — motion stops at the midpoint by
  design (see "What a run is" above).
- `box` and `lanczos` giving noticeably different `sharpness_mean` than `lanczos-filament` for
  the same run (static 1:1 in the current baseline: 0.87 / 0.95 / 1.07 respectively) — expected;
  see the `-F` gotcha above. The gap between the two Lanczos variants (idealized vs the polynomial
  `taa.mat` actually evaluates) narrowed considerably when that polynomial was refit for accuracy
  (see `lanczos2_SquaredInputApproximate`'s comment) but is still there and still the dominant
  term over the box/idealized-Lanczos gap. Re-derive the exact split before citing a number; it
  moves as the harness's own defaults (warmup, motion speed, etc.) are tuned.

## Debugging `depthDisocclusion` specifically

`--visualize-disocclusion` replaces every rendered frame with the mask `taa.mat` computes
(white = history fully rejected, black = trusted), so a `-d` dump shows exactly which texels the
test fires on. Use it with a short capture and a throwaway reference -- the metrics from such a
run describe the mask, not the image, and are meaningless:

```bash
echo '{ "view": { "taa": { "depthDisocclusion": true } } }' > /tmp/dd_on.json
./out/cmake-debug/tools/taa/taa_harness -e -r static -u 1 -F box -R 1 -n 8 -q \
    --visualize-disocclusion -d /tmp/mask/ -s /tmp/dd_on.json \
    -o /tmp/ignore.json third_party/models/Fox/Fox.glb
```

Then count firing texels per frame out of `capture_NN.png`. This is the difference between "fires
on a handful of silhouette texels" and "fires over the whole image" -- two failures that look
identical in the aggregate numbers, and the reason a static-scene regression that cost
`edge_flicker_variance` +292% was chased for a while as a whole-image effect when it was ~130
texels out of 262144.

**Nothing here covers a slowly-moving camera, and that is a real gap.** `static` holds the camera
matrix bit-identical, so its reprojection delta is exactly zero; the `camera` regimes run at 3.5
(`translate`) and ~17 (`rotate`) render texels/frame. There is nothing in between, so anything
whose behaviour depends on the delta being *small* rather than *zero* measures clean in every
regime the sweep runs. That is not a hypothetical: it hid a bug where a tenth of a texel per frame
rejected history on every edge in the frame. A slow pan, the ease at either end of a camera move,
and the coast after a `FreeFlightManipulator` key release all live in that gap. Add a barely-moving
run by hand:

```bash
./out/cmake-debug/tools/taa/taa_harness -e -r camera -u 1 -F box -R 1 -n 8 -q \
    --camera-motion translate --camera-texels-per-frame 0.1 \
    --visualize-disocclusion -d /tmp/drift/ -s /tmp/dd_on.json \
    -o /tmp/ignore.json third_party/models/Fox/Fox.glb
```

A tenth of a render texel per frame is visually static but numerically moving. The mask should
stay near-empty there too; it once went to ~150 texels a frame.

The expectation the current code is written to: on a static scene the mask must be **empty**
(0-2 texels a frame is the measured floor), at 1:1 and under upscaling alike. Any more than that
is a bug in the jitter tolerance, not a tuning question -- see the depth-spread derivation in
`taa.mat`'s `depthDisocclusion` block.

## When to reach for the simulator instead

[taa_sim.py](taa_sim.py) is a 1-D model of the accumulation arithmetic alone -- no GPU, no
polynomial kernel, no YCoCg, no ray-clip, one dimension. It answers a different question from
this harness: not "did this change help on real content" but "what is this term actually doing".

Reach for it when a difference is *systematic* rather than content-dependent -- anything about
the kernel, the clamps, the jitter sequence, or how upscaling changes the accumulation. It
computes an effective MTF by driving a cosine to convergence and projecting the result back,
which separates bias from variance in a way no measurement on real content can: this harness's
metrics mix the two, and on real content the variance term usually dominates.

That distinction resolved the upscaled-sharpness question. Every measurement here pointed at the
deringing clamp; the simulation showed deringing costs a few percent and the *history box* costs
an order of magnitude, which then reproduced on `SubpixelStar` at 2:1 once the right scene was
used. The harness could not have shown it, because the effect lives above 0.2 cycles/pixel and
`DamagedHelmet` has almost no energy there -- box clipping off changes it by 0.20/255 against
`SubpixelStar`'s 2.04/255.

The converse also holds: the simulator cannot tell you whether anything is *visible*. Pair it
with a per-pixel delta from a `-d` dump before believing a change matters, and see the
`sharpness_mean` gotcha below.

## What this tool does not measure yet

No disocclusion-mask-specific *metrics* -- `--visualize-disocclusion` renders the mask so it can
be dumped and looked at (see "Debugging `depthDisocclusion` specifically" above), but nothing
aggregates it into the JSON, so "how many texels fired" still has to be counted out of the dumped
PNGs by hand. Same for the ring-binned flicker curve under "Test scenes": both live in throwaway
Python around the tool rather than in it.

The silhouette mask is a luminance-gradient heuristic on the reference image, not a true
depth-edge mask. One backend per run — nothing here is cross-backend validated. Every scene under
"Test scenes" has been exercised through the tool, but only the checked-in baseline (`Fox.glb`,
default settings) is regression-tested; the others are run ad hoc, so a number quoted from one of
them is a snapshot rather than something being held. Don't extrapolate beyond what was actually
run.
