# Upscaler Qualification Benchmark — User Guide

`taa_harness --benchmark` evaluates Temporal Anti-Aliasing (TAA) and temporal upscaling
configurations across still scenes, sub-pixel geometry, slow camera drift, rigid motion, skeletal
animation, and screen-space reflections. It summarizes visual quality into **three headline scores**
so you can compare different TAA implementations or configuration presets side by side and choose
the best fit for your application.

---

## Quick Start

### 1. Build the tool

```bash
./build.sh release taa_harness
```

### 2. Run the benchmark

Run from the repository root (or any build directory; the tool locates its test assets
automatically):

```bash
H=./out/cmake-release/tools/taa/taa_harness

# Benchmark Filament's default built-in TAA at 2x upscaling (~30s)
$H -b --label "Filament TAA" -o bench_default.json
```

By default, `-b` (`--benchmark`) runs headlessly at **2x upscaling** (`256x256 -> 512x512`) using a
fast reference filter (`-F box`) and prints a scorecard to the terminal while saving the full
results to JSON.

### 3. Benchmark alternative presets

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

$H -b -s adaptive.json --label "TAA (Adaptive)" -o bench_adaptive.json
```

### 4. Compare configurations side by side

Pass two or more benchmark JSON files to `--compare`. This mode runs instantly on the CPU (no GPU
needed) and highlights the winner (`*`) on each quality axis:

```bash
$H --compare bench_default.json,bench_adaptive.json
```

```text
========================================================================
  Upscaler Qualification Comparison (x2, ref: box)
========================================================================
  Metric                                 Filament TAA     TAA (Adaptive)
------------------------------------------------------------------------
  1. Spatial Quality (dB)                  32.03 dB *         29.58 dB  
       Static Spatial RMSE                   0.0250             0.0332  
       Correlated Detail (%)                  72.1%              57.9%  
       Spurious HF / Ringing (%)              56.0%              52.5%  
  2. Temporal Stability (dB)               30.14 dB           33.07 dB *
       Top-1% Flicker RMS                    0.0311             0.0222  
       Sub-pixel Star Ring RMS               0.0434             0.0114  
       Slow Camera Drift RMS                 0.0396             0.0421  
  3. Motion Robustness (dB)                24.23 dB *         23.00 dB  
       - Tracked Motion (dB)               24.24 dB           24.46 dB *
       - Unmodeled Motion (dB)             24.22 dB *         21.05 dB  
       Dynamic Edge RMSE                     0.0614             0.0708  
========================================================================
  (* marks best dB score on each axis; higher dB is always better)
```

> **Important:** Only compare runs generated with the **same upscaling factor (`-u`)** and **same
> reference filter (`-F`)**. `--compare` automatically checks this and refuses to mix mismatched
> configurations.

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
converged.

- **`Static Spatial RMSE`** *(lower is better; `0.0` = identical to ground truth)*
  Average linear-light pixel error on detailed static 3D scenes. Increases when an upscaler blurs
  textures, softens silhouettes, or shifts edges away from their true position.
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
motion stops.

Because upscalers behave very differently depending on whether moving pixels carry accurate motion
vectors, this score is split into two sub-scores:

- **`Tracked Motion (dB)`** *(higher is better)*
  Quality when every moving pixel has an accurate motion vector — orbiting cameras, camera dollies
  with foreground/background parallax, and rigidly translating or spinning 3D objects (with
  `motionVectors` enabled). Tests reprojection accuracy and how quickly newly uncovered background
  regions (disocclusions) clean up.
- **`Unmodeled Motion (dB)`** *(higher is better)*
  Quality when pixels change on screen **without** matching motion vectors — such as skinned
  character animations when vertex-skinning motion vectors are unavailable, animated textures/UVs,
  particles, or sliding specular reflections (SSR) on static floors. Tests whether the upscaler
  rejects outdated history fast enough to prevent ghost trails.
- **`Dynamic Edge RMSE`** *(lower is better)*
  Average linear-light pixel error around moving silhouettes across all motion tests.

---

## Reading the Per-Scenario Table

When you run `-b` on a single upscaler, the terminal also prints the 10 individual scenarios that
feed into the three headline scores:

| Column | Meaning |
| :--- | :--- |
| **`Edge RMSE`** | Average pixel error around object silhouettes during the scenario (lower is better). |
| **`Detail %`** | Genuine fine detail preserved during the scenario (higher is better; drops naturally under fast motion blur/smear). |
| **`Flicker RMS`** | Worst-pixel temporal shimmer, reported on the four stability scenarios (lower is better). |
| **`Ghost Peak`** | Worst single-frame ghosting/smearing spike during motion, with the static error floor subtracted out (lower = fainter/shorter ghost trails). |
| **`End RMSE`** | Residual silhouette error several frames *after* motion stops (lower = faster recovery; high values mean ghost trails linger on screen). |

The 10 scenarios cover:
- **Static & Stability**: `fox_static` (hard silhouettes), `helmet_static` (normal maps & specular
  highlights), `star_static` (sub-pixel thin lines), `fox_drift` (slow `0.1` tx/frame camera creep).
- **Tracked Motion**: `helmet_cam_rotate` (fast camera orbit), `helmet_cam_dolly` (pure parallax
  dolly), `fox_obj_translate` (moving object with motion vectors), `helmet_obj_spin` (spinning
  specular object with motion vectors).
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
| `-b`, `--benchmark` | off | Run the 10-scenario upscaler qualification benchmark (headless). |
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

