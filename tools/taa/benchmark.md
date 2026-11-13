# Upscaler Qualification & GPU Performance Benchmark — User Guide

`taa_harness` evaluates Temporal Anti-Aliasing (TAA) upscalers across
both **visual quality** (`--benchmark` / `-b`) and **isolated GPU execution cost** (`--perf`).
- `--benchmark` (`-b`) evaluates still scenes, sub-pixel geometry, slow camera drift, rigid motion,
  skeletal animation, and screen-space reflections, summarizing visual quality into **three headline
  dB scores** (higher is better).
- `--perf` measures the **isolated GPU cost of the upscaler in milliseconds** (lower is better) at a
  target display resolution (`1920x1080` by default) with zero pixel readback during the timed
  window, paired differential baseline subtraction, and CTS anti-cheat checks.
- Combining `-b --perf` produces a **four-axis scorecard** so you can evaluate quality and GPU cost
  together on the same device.

---

## Quick Start

### 1. Build the tool

```bash
./build.sh release taa_harness
```

### 2. Run the benchmark on an upscaler

Run from the repository root (or any build directory; the tool locates its test assets
automatically):

```bash
H=./out/cmake-release/tools/taa/taa_harness

# Benchmark Filament's default built-in TAA at 2x upscaling (~30s)
$H -b -a vulkan --label "Filament TAA" -o bench_default.json
```

By default, `-b` (`--benchmark`) runs headlessly at **2x upscaling** (`256x256 -> 512x512`) using a
fast reference filter (`-F box`) and prints a scorecard to the terminal while saving the full
results to JSON.

### 3. Benchmark alternative TAA presets

**Test a custom `TemporalAntiAliasingOptions` preset (`-s`):**

```bash
cat > adaptive.json <<'EOF'
{
  "view": {
    "taa": {
      "motionAdaptiveHistory": true,
      "historyReprojection": true
    }
  }
}
EOF

$H -b -a vulkan -s adaptive.json --label "TAA (Adaptive)" -o bench_adaptive.json
```

### 4. Compare configurations side by side

Pass two or more benchmark JSON files to `--compare`. This mode runs instantly on the CPU (no GPU
needed) and highlights the winner (`*`) on each quality axis:

```bash
$H --compare bench_default.json,bench_adaptive.json
```

```text
=============================================================================================
  Upscaler Qualification Comparison (x2, ref: box, protocol: v2.1)
=============================================================================================
  Metric                                   Filament TAA       TAA (Adaptive)
---------------------------------------------------------------------------------------------
  1. Spatial Quality (dB)                      31.84 dB *           29.62 dB
       Static Spatial RMSE                       0.0256               0.0330  
       Interior Texture GMSD                     0.0284               0.0351  
       Correlated Detail (%)                      72.1%                57.9%  
       Spurious HF / Ringing (%)                  56.0%                52.5%  
       Hotspot Top-10 P95 RMSE                   0.0891               0.1120  
       Hotspot Peak Cell P95 RMSE                0.1240               0.1510  
  2. Temporal Stability (dB)                   30.14 dB             33.07 dB *
       Top-1% Flicker RMS                        0.0311               0.0222  
       Sub-pixel Star Ring RMS                   0.0434               0.0114  
       Slow Camera Drift RMS                     0.0396               0.0421  
  3. Motion Robustness (dB)                    24.68 dB *           23.41 dB
       - Tracked Motion (dB)                   24.71 dB             24.89 dB *
       - Unmodeled Motion (dB)                 24.62 dB *           21.48 dB
       Dynamic Edge RMSE                         0.0583               0.0675  
       Wake Ghosting RMS                         0.0342               0.0418  
       Hotspot Top-10 P95 RMSE                   0.1480               0.1720  
       Hotspot Peak Cell P95 RMSE                0.2190               0.2580  
=============================================================================================
  Per-Frame Edge RMSE Localization: TAA (Adaptive) vs Filament TAA
    Regression  fox_skinned_run    window [ 4.. 7] mean +0.0241 | peak t*= 6 (+0.0294)
    Improvement star_static        window [ 0.. 3] mean -0.0082 | peak t*= 2 (-0.0095)
  (* marks best score: highest dB on quality axes 1-3, lowest ms on GPU performance)
```

> **Important:** `--compare` enforces a normative protocol signature (`protocol_version: "2.1"` and
> a 64-character SHA-256 `schedule_signature` over all frame-schedule parameters: scenario roster,
> resolution, warmup/capture frame counts, reference filter, reference sample count, jitter pattern,
> edge threshold, and upscaling factors). Attempting to compare JSON files produced under mismatched
> schedules exits with status `1` and lists the exact mismatched parameters.

---

## What the Numbers Represent

### Understanding the dB Scale

All three headline scores (plus the two motion sub-scores) are reported in **decibels (dB)**,
where **higher is always better**:

| Score difference | What it means visually |
| :--- | :--- |
| **`< 0.2 dB`** | Negligible — visually indistinguishable in normal viewing. |
| **`0.5 – 1.0 dB`** | Noticeable improvement on side-by-side or close inspection. |
| **`+3.0 dB`** | **2x less error energy** — a substantial, clearly visible step up in quality. |
| **`+6.0 dB`** | **2x lower pixel RMS error** — dramatically cleaner output. |

Beneath each headline dB score, the table prints raw physical measurements (RMSE in linear light or
percentages) so you can see *why* an upscaler scored higher or lower.

---

### 1. Spatial Quality (dB)

Measures **how sharp, accurate, and artifact-free still scenes look** once the upscaler has
converged. The headline dB score combines silhouette edge MSE (`0.40`), whole-frame MSE (`0.35`),
and foreground-masked Prewitt GMSD (`0.25`) on `fox_static` and `helmet_static`.

- **`Static Spatial RMSE`** *(lower is better; `0.0` = identical to ground truth)*
  Combined linear-light pixel and gradient error on detailed static 3D scenes. Increases when an
  upscaler blurs textures, softens silhouettes, or shifts edges away from their true position.
- **`Interior Texture GMSD`** *(lower is better; `0.0` = identical gradient structure)*
  Gradient Magnitude Similarity Deviation (Xue et al., 2014) computed via $3\times 3$ Prewitt
  filters over foreground geometry and silhouettes. Measures the spatial standard deviation of
  first-order local gradient similarity, catching selective interior texture smoothing on normal
  maps and albedo detail that whole-frame $L_2$ averages and second-order Laplacian step-edge
  metrics can under-weight.
- **`Correlated Detail (%)`** *(higher toward `100%` is better)*
  Percentage of genuine fine detail from the native-resolution reference that survives upscaling. A
  soft upscaler drops into the `50–60%` range; a sharp upscaler reaches `75%+`. Unlike naive
  sharpness metrics, artificial sharpening halos and ghost double-edges **do not** increase this
  number — only detail aligned with the real scene counts.
- **`Spurious HF / Ringing (%)`** *(lower is better)*
  Unwanted high-frequency energy that does not exist in the reference image, such as oversharpening
  halos, ringing around bright highlights, or jagged aliasing. Read this alongside *Correlated
  Detail*: if an upscaler increases *Correlated Detail* without spiking *Spurious HF*, the extra
  sharpness is genuine.
- **`Hotspot Top-10 P95 RMSE` & `Hotspot Peak Cell P95 RMSE`** *(lower is better)*
  Localized spatial error concentration over a $10\times 10$ viewport grid restricted to the inner
  $8\times 8$ core ($64$ cells, discarding the outer viewport border ring) and gated to active
  cells ($\ge 10$ foreground/silhouette pixels). Within each active cell, the $95\text{th}$
  percentile pixel RMSE is computed; *Hotspot Top-10 P95 RMSE* averages the worst 10 cells and
  *Hotspot Peak Cell P95 RMSE* reports the single worst cell. If a candidate upscaler improves the
  global headline dB score ($\ge +0.05\text{ dB}$) while worsening *Hotspot Top-10 P95 RMSE*
  ($> 2\%$ and $> 0.0005$), `--compare` emits a `WARNING [HOTSPOT_RANK_REVERSAL]` alert.

---

### 2. Temporal Stability (dB)

Measures **how calm and free of shimmer, buzzing, or crawling edges the image is from frame to
frame**. Because flicker usually affects only hard edges or thin lines rather than flat backgrounds,
these metrics focus specifically on the pixels that actually shimmer.

- **`Top-1% Flicker RMS`** *(lower is better)*
  Frame-to-frame brightness fluctuation across the worst 1% of pixels in the frame (silhouettes,
  normal-mapped specular highlights, and fine creases). This is where the human eye immediately
  notices temporal shimmer.
- **`Sub-pixel Star Ring RMS`** *(lower is better)*
  Frame-to-frame shimmer on ultra-thin, high-contrast sub-pixel geometry (representative of wires,
  fences, grates, rigging, and distant foliage). Values above `0.04` indicate visible buzzing on
  thin lines; values near `0.01` mean sub-pixel lines remain steady.
- **`Slow Camera Drift RMS`** *(lower is better)*
  Shimmer and "stair-step pulsing" along edges when the camera creeps very slowly (`0.1` render
  texel per frame — typical of camera damping at the end of a pan or subtle idle camera motion).
  Catches upscalers that look stable when the camera is frozen (`0.0` velocity) but suddenly
  discard history and shimmer as soon as the camera moves slightly.

---

### 3. Motion Robustness (dB)

Measures **how cleanly the upscaler handles moving cameras, moving objects, animations, and
reflections** without leaving ghost trails, smearing detail, or locking onto stale history after
motion stops. The headline dB score combines dynamic silhouette `edge_rmse_mean` (`0.40`),
`edge_ghost_peak` (`0.30`), post-motion `edge_tail_error` (`0.15`), and in-motion analytical
backward-velocity `wake_ghosting_rms` (`0.15`).

Because upscalers behave very differently depending on whether moving pixels carry accurate motion
vectors, this score is split into two sub-scores:

- **`Tracked Motion (dB)`** *(higher is better)*
  Quality when every moving pixel has an accurate motion vector — orbiting cameras, camera dollies
  with foreground/background parallax, rigidly translating or spinning 3D objects, and articulated
  multi-node glTF animations (`BusterDrone.glb`, with `motionVectors` enabled). Tests reprojection
  accuracy and how quickly newly uncovered background regions (disocclusions) clean up.
- **`Unmodeled Motion (dB)`** *(higher is better)*
  Quality when pixels change on screen **without** matching motion vectors — such as skinned
  character animations when vertex-skinning motion vectors are unavailable, animated textures/UVs,
  particles, or sliding specular reflections (SSR) on static floors. Tests whether the upscaler
  rejects outdated history fast enough to prevent ghost trails.
- **`Dynamic Edge RMSE`** *(lower is better)*
  Combined dynamic error across moving silhouettes, transient ghost peaks, post-motion recovery
  tails, and trailing ghost wakes.
- **`Wake Ghosting RMS`** *(lower is better)*
  In-motion trailing ghost-wake persistence sampled at $d \in \{2, 4, 6, 8\}\text{ px}$ along the
  analytical negative screen-space velocity vector $-\hat{\mathbf{v}}(\mathbf{x})$ behind
  disoccluded trailing edges during active motion ($1 \le k \le N_{\text{motion}}$). Isolates
  directional ghost trails during continuous motion from symmetric leading-edge spatial blur and
  static post-motion hold errors.
- **`Hotspot Top-10 P95 RMSE` & `Hotspot Peak Cell P95 RMSE`** *(lower is better)*
  Inner $8\times 8$ core localized error concentration across all motion scenarios, paired with
  automatic `HOTSPOT_RANK_REVERSAL` detection in `--compare`.
- **Per-Frame Localization (`window [t_start..t_end]` & `peak t*`)**
  When comparing two or more runs with `--compare`, the harness automatically convolves the
  per-frame `edge_rmse_frame_k` difference series with a 4-frame sliding window and prints the top
  scenario regressions and improvements along with the exact frame interval `[t_start..t_end]` and
  peak frame `t*`, so you know immediately which `capture_NN.png` and `diff_NN.png` files to
  inspect in a `--dump` directory.

---

### 4. GPU Performance (ms, with `--perf`)

Measures **how long the GPU takes to execute the upscaler workload in milliseconds** (lower is
better) at a realistic display viewport (`--perf-resolution 1920x1080` by default) with **zero pixel
readback** inside the timed window.

Because TAA executes inside the frame alongside scene rendering, `--perf`
runs a **paired differential benchmark** across five representative GPU workloads (`perf_fox_skinned_run`,
`perf_helmet_cam_dolly`, `perf_helmet_obj_spin`, `perf_ssr_reflection`, `perf_buster_drone_anim`):
1. **Render-Resolution Baseline** (`W/u x H/u`, `taa.enabled = false`, `disable_subpasses = true`
   so tonemap remains an out-of-subpass render pass matching the upscaler's render-pass layout on
   tile-based mobile GPUs).
2. **Upscaled Run** (`W x H` output, `W/u x H/u` internal render resolution, `taa.enabled = true`,
   `taa.upscaling = u`, `motionVectors = true`).

The scorecard reports:
- **`Upscaler GPU Time p50 / p90 (ms)`** *(lower is better)*
  Differential GPU cost isolated to the upscaler (`max(0, Full Frame GPU Time - Baseline GPU Time)`).
- **`Full Frame GPU Time p50 (ms)`** *(lower is better)*
  Median total GPU frame duration with the upscaler enabled, measured via hardware GPU timestamp
  queries (`Renderer::getFrameInfoHistory()`).
- **`Render-Res Baseline p50 (ms)`**
  Median GPU frame duration of the matching render-resolution baseline pass.
- **`Frame-to-Frame IQR (ms)`** *(lower is better)*
  Interquartile range (`p75 - p25`) of GPU frame times during the timed window — verifies clock
  stability under Android Fixed Performance Mode.
- **`CTS Anti-Cheat Status`** (`PASS` / `FAIL`)
  Combines four tamper-detection checks so OEM/driver results can be audited:
  1. **Hardware vs Wall-Clock Cross-Check (`timing_discrepancy_ms <= 0.25 ms`)**: detects driver
     spoofing of `VK_QUERY_TYPE_TIMESTAMP` results by comparing the sum of hardware GPU durations
     against steady-state wall-clock/fence intervals over a GPU-drained timed window.
  2. **Post-Window Output Audit (`post_check_passed`)**: reads back two consecutive post-window drain
     frames and verifies that the output is neither blank/constant nor a trivial 1-tap blit that
     skipped temporal jitter accumulation.
  3. **Android ADPF Fixed Performance Mode (`fixed_performance_mode`)**: verifies `cmd power
     set-fixed-performance-mode-enabled true` is active on Android.
  4. **Thermal Stability (`thermal_status_end <= 1`)**: verifies the device did not throttle past
     `ADPF_THERMAL_STATUS_LIGHT` during the run.

---

## Running on Android with Fixed Performance Mode & Systrace Verification

On Android, `taa_harness` is packaged as the `:samples:sample-taa-harness` APK
(`android/samples/sample-taa-harness`), which bundles `libtaa_harness.so`, IBL lighting assets, and
the 5 benchmark glTF models (`Fox.glb`, `DamagedHelmet.glb`, `SubpixelStar.glb`,
`SSRReflection.glb`, `BusterDrone.glb`), running on standard `user` and `userdebug` builds via
`am instrument -w` without requiring root or private APIs.

```bash
# 1. Build and install libtaa_harness.so for Android arm64-v8a
ninja -C out/cmake-android-release-aarch64 taa_harness install

# 2. Assemble the sample-taa-harness APK
cd android && ./gradlew \
    -Pcom.google.android.filament.dist-dir="$PWD/../out/android-release/filament" \
    -Pcom.google.android.filament.tools-dir="$PWD/../out/release/filament" \
    -Pcom.google.android.filament.abis=arm64-v8a \
    :samples:sample-taa-harness:assembleDebug && cd ..

# 3. Run GPU performance benchmark on the connected Android device
python3 tools/taa/run_android_benchmark.py

# 4. Run both quality (--quality) and GPU performance, plus Systrace ground-truth verification
python3 tools/taa/run_android_benchmark.py --quality --verify-systrace
```

`run_android_benchmark.py` automatically:
1. Installs `sample-taa-harness-debug.apk` onto the target device (`adb install -r -t`).
2. Checks `dumpsys thermalservice` thermal headroom and enables **ADPF Fixed Performance Mode**
   (`adb shell cmd power set-fixed-performance-mode-enabled true`), restoring it in a `finally`
   block on exit.
3. Runs `default` (plus any `-s` settings presets) sequentially via `am instrument -w`, pulls the
   JSON files from `/sdcard/Android/data/com.google.android.filament.taaharness/files/`, and prints
   a side-by-side `--compare` table on the host when comparing multiple presets.
4. When `--verify-systrace` is passed, captures a simultaneous Perfetto trace (`atrace` `gfx`, `view`,
   `power`, `gpu_renderstages`, `gpu_frequency`) and queries it via Perfetto's `trace_processor` SQL
   engine to verify that `taa_harness`'s self-reported JSON metrics match the Systrace ground truth
   within `max(0.15 ms, 5%)`.

---

## Reading the Per-Scenario Table

When you run `-b` on a single upscaler, the terminal also prints the 11 individual scenarios that
feed into the three headline scores:

| Column | Meaning |
| :--- | :--- |
| **`Edge RMSE`** | Average pixel error around object silhouettes during the scenario (lower is better). |
| **`GMSD`** | First-order Prewitt Gradient Magnitude Similarity Deviation over foreground geometry and silhouettes (lower is better; `0.0` = identical gradient structure). |
| **`Hotspot10`** | Mean $95\text{th}$-percentile pixel RMSE across the worst 10 active cells of the inner $8\times 8$ core grid (lower is better). |
| **`Detail %`** | Genuine fine detail preserved during the scenario (higher is better; drops naturally under fast motion blur/smear). |
| **`Flicker RMS`** | Worst-pixel temporal shimmer, reported on the four stability scenarios (lower is better). |
| **`Ghost Peak`** | Worst single-frame ghosting/smearing spike during motion, with the static error floor subtracted out (lower = fainter/shorter ghost trails). |
| **`Wake RMS`** | In-motion trailing ghost-wake persistence sampled along $-\hat{\mathbf{v}}$ behind disoccluded trailing edges (lower = cleaner trailing edge wakes). |

The 11 scenarios cover:
- **Static & Stability**: `fox_static` (hard silhouettes), `helmet_static` (normal maps & specular
  highlights), `star_static` (sub-pixel thin lines), `fox_drift` (slow `0.1` tx/frame camera creep).
- **Tracked Motion**: `helmet_cam_rotate` (fast camera orbit), `helmet_cam_dolly` (pure parallax
  dolly), `fox_obj_translate` (moving object with motion vectors), `helmet_obj_spin` (spinning
  specular object with motion vectors), `buster_drone_anim` (articulated multi-node rigid glTF
  animation with per-object motion vectors).
- **Unmodeled Motion**: `fox_skinned_run` (running skeletal character without skinning motion
  vectors), `ssr_reflection` (camera dolly where a reflected pillar slides across a static floor).

---

## Choosing an Upscaler for Your Application

Every temporal upscaler balances a fundamental three-way trade-off:
- **Trusting history more** improves **Temporal Stability** (less shimmer) and **Spatial Quality**,
  but hurts **Unmodeled Motion** (more ghosting on effects, reflections, and un-vectored
  animations).
- **Clamping history aggressively** improves **Unmodeled Motion** (crisp, ghost-free motion), but
  lowers **Temporal Stability** (more sub-pixel buzzing) and **Spatial Quality** (softer edges).

Use your application's workload to decide which axis matters most:

1. **Product viewers, CAD, architectural walk-throughs, and mostly static scenes**
   Prioritize **Spatial Quality** and **Temporal Stability** (especially *Sub-pixel Star Ring RMS*
   and *Slow Camera Drift RMS* for smooth camera eases). A lower *Unmodeled Motion* score is rarely
   an issue if the scene has no skeletal characters or animated materials.
2. **3D navigation and games with full rigid/skinned motion vectors**
   Prioritize **Tracked Motion** and **Spatial Quality**, with good *Slow Camera Drift RMS* so
   camera stops feel seamless.
3. **Scenes with un-vectored animations, heavy SSR reflections, scrolling shaders, or particles**
   Pay close attention to **Unmodeled Motion** and `Ghost Peak` on `fox_skinned_run` and
   `ssr_reflection`. Avoid presets that trade away 2–3 dB of *Unmodeled Motion* for stability, or
   moving characters and reflections will leave visible trails.

---

## Useful Benchmark Options

| Flag | Default | Description |
| :--- | :--- | :--- |
| `-b`, `--benchmark` | off | Run the 11-scenario upscaler qualification benchmark (headless). |
| `--perf` | off | Run the GPU performance benchmark (paired differential timing, zero readback in timed window); combine with `-b` for a 4-axis scorecard. |
| `--perf-resolution <WxH>` | `1920x1080` | Output viewport resolution for `--perf` runs (`render = W/u x H/u`). |
| `--perf-warmup <N>` | `64` | Untimed warmup frames per `--perf` run. |
| `--perf-frames <N>` | `128` | Timed frames per `--perf` run. |
| `--label <name>` | auto | Display name for this configuration in scorecards and `--compare`. |
| `--compare <a.json,b.json,...>` | off | Print a side-by-side comparison table from saved benchmark JSON files. |
| `-a`, `--api <backend>` | default | Rendering backend: `opengl`, `vulkan`, or `metal`. |
| `-s`, `--settings <file.json>` | none | Load `TemporalAntiAliasingOptions` from a viewer-compatible JSON file. |
| `-u`, `--upscaling <list>` | `2` | Upscaling factor(s) to benchmark, e.g. `-u 1.5`, `-u 2`, or `-u 1.5,2`. |
| `-F`, `--reference-filter <k>` | `box` | `box` (~30s fast iteration) or `lanczos` (~3 min high-precision sign-off). |
| `-S`, `--size <px>` | `512` | Output viewport width and height in pixels (render resolution is `size / upscaling`). |
| `-o`, `--output <file.json>` | `taa_harness.json` | Path where the benchmark JSON file is written. |
| `-d`, `--dump <dir>` | none | Save rendered PNG frames, references, and amplified error heatmaps for visual inspection. |
| `-q`, `--quiet` | auto | Hide the terminal progress bar. |

### Visual Inspection Tip (`--dump`)

Whenever a benchmark score surprises you and you want to see the artifact with your own eyes, pass
`-d /tmp/bench_frames/`. Inside that directory, each scenario gets its own subfolder containing the
upscaled `capture_NN.png` frames, the `reference_NN.png` ground truth, and `diff_NN.png` (an 8x
amplified error image showing exactly which pixels caused the error).

