# Filament Temporal Anti-Aliasing (`taa.mat`)

This document describes the architecture, mathematical design, edge-case defenses, and empirical tuning of Filament's Temporal Anti-Aliasing (TAA) pass. It serves as the companion design reference for [`taa.mat`](taa.mat), [`TemporalAntiAliasingOptions`](../../../../include/filament/Options.h), and [`PostProcessManager::taa()`](../../../PostProcessManager.cpp).

For measurement methodology, benchmark scenes, and CLI usage of the deterministic evaluation harness and 1-D simulator, see [`tools/taa/taa_harness.md`](../../../../../tools/taa/taa_harness.md).

---

## 1. Pipeline Overview

Filament's TAA resolves sub-pixel jittered renders over time into an anti-aliased (and optionally upscaled) output texture:

1. **Camera Jitter (`PostProcessManager::TaaJitterCamera`)**:
   - Before rendering the scene, the camera projection is translated in clip space by a sub-pixel offset from the configured `JitterPattern` (`RGSS_X4`, `UNIFORM_HELIX_X4`, `HALTON_23_X8`, `HALTON_23_X16` [default], or `HALTON_23_X32`).
   - The unjittered `projection * view` matrix is saved in `FrameHistoryEntry::TemporalAA::projection` so inter-frame camera reprojection is independent of the jitter phase.
   - The shader receives `jitter` (and `previousJitter`) negated relative to the projection offset so they represent the sample offset within the render texel in `[-0.5, 0.5]`.
   - When TAA is enabled, material texture fetches apply an effective mip bias of `lodBias - log2(upscaling)` (`lodBias` defaults to `-1.0`, giving `-1.0` at 1:1 and `-2.0` at 2:1):
     - `lodBias` is the temporal-supersampling term only: because TAA accumulates multiple jittered samples per output pixel, it can afford a sharper mip than a single sample per pixel would justify, relying on temporal accumulation to remove the resulting aliasing.
     - `-log2(upscaling)` is the resolution term, added automatically on top (matching dynamic resolution) because rasterization occurs at render resolution while presentation is at `upscaling` times that resolution.
     - Unlike the resolution term, `lodBias` is a bet on TAA converging, which weakens as `upscaling` rises (at 2:1 each output pixel receives a quarter of the samples per frame). If fine texture detail shimmers at high upscaling factors, `lodBias` should be backed off toward `-0.5` rather than compensated for elsewhere.

2. **History Reprojection & Motion / Disocclusion Analysis**:
   - The current pixel's center depth is fetched from `depth` and multiplied by `reprojection = historyProjection * inverse(current.projection) * normalizedToClip` to obtain `historyUv` and the reprojected clip-space depth `expectedHistoryDepth = q.z / q.w`.
   - `reprojectionDeltaLen = length((historyUv - uv) * colorResolution.xy)` measures camera-induced motion in render texels.
   - If `motionAdaptiveHistory` is enabled, `historyBoxStrength` and `historyAlphaStrength` are relaxed from `1.0` toward `minBoxStrength` and `minAlphaStrength` when camera motion is low.
   - If `depthDisocclusion` is enabled, `expectedHistoryDepth` is compared against `depthHistory` to detect surface changes and restore full rejection strength on disoccluded/occluded pixels.

3. **History Sampling (`filterHistory`)**:
   - History color is sampled at `historyUv` using either a 5-tap bicubic Catmull-Rom filter (`sampleTextureCatmullRom`, with 5-tap min/max deringing) or a single bilinear fetch.
   - If `hdr` is enabled, both history and input taps are mapped into `[0, 1)` via `tonemap(c) = c / (1 + max3(c))`.

4. **Input Unjittering / Reconstruction (`filterInput`)**:
   - A 3x3 neighborhood `s[0..8]` around the nearest input texel center `p` is fetched and tonemapped.
   - Separable 1-D Lanczos-2 weights (`lanczos2`) reconstruct the unjittered color at the output pixel center, optionally extended to 4x4 (`exactLanczosSupport`) at 1:1 resolution.
   - The reconstructed color `filtered` is deringed in RGB against the sampled neighborhood bounds (`deringMin`, `deringMax`).

5. **History Box Clamping (`boxClipping`, `boxType`, `useYCoCg`)**:
   - A neighborhood bounding box (`boxmin`, `boxmax`) is built in either tonemapped RGB or YCoCg from a 50/50 blend of the 5-tap cross (`box5`) and 9-tap 3x3 (`box9`) bounds, optionally intersected with a 9-tap mean $\pm \gamma \sigma$ variance box (`BOX_TYPE_AABB_VARIANCE`) and floored to `kMinBoxHalfWidth`.
   - History is clipped toward `filtered` (`BOX_CLIPPING_ACCURATE` ray-cast or `BOX_CLIPPING_CLAMP`), blended by `historyBoxStrength`, and pulled directly toward `filtered` by `disocclusion` when `depthDisocclusion` is active.

6. **Temporal Blend, Untonemap, and History Export**:
   - Effective blend weight `alpha = (materialParams.alpha * confidence) * historyAlphaStrength` is optionally modulated by luma difference (`preventFlickering`), blending `history` and `filtered`.
   - The result is mapped back to linear HDR via `untonemap()`, exported as the next frame's color history (along with the raw render-resolution `depth` buffer when `depthDisocclusion` is enabled), and optionally followed by an FSR1 RCAS sharpening pass (`sharpness > 0`).

---

## 2. HDR Tonemapping, `untonemap()` Pole Protection, and YCoCg Gamut Clamping

### The `untonemap()` Pole at 1.0
`tonemap(c) = c / (1.0 + max3(c))` maps $[0, \infty)$ onto $[0, 1)$. Its inverse:
```glsl
vec3 untonemap(const vec3 c) {
    const float epsilon = 1.0 / 65504.0;
    vec3 t = clamp(c, 0.0, 1.0 - epsilon);
    return t * rcp(max(epsilon, 1.0 - max3(t)));
}
```
has a pole at `max3(c) == 1.0` and is undefined for negative inputs.

Without the `clamp(c, 0.0, 1.0 - epsilon)` guard, any input with `max3(c)` slightly above `1.0` makes `1.0 - max3(c)` negative; `max(epsilon, ...)` floors the denominator at `epsilon = 1 / 65504`, and the reciprocal jumps straight to `65504`. A pixel of any brightness turns into a ~65000 firefly, which overflows the fp16 history buffer to `+Inf`. On the following frame:
1. `tonemap(+Inf)` evaluates `Inf * rcp(1.0 + Inf) = Inf * 0.0 = NaN`.
2. In `clipToBox()`, `lessThan(h.rgb, boxmin)` and `greaterThan(h.rgb, boxmax)` both evaluate to `false` against `NaN`.
3. `clipToBox()` takes the "already in the box" fast path and returns the `NaN` history untouched forever.

### Why YCoCg Clipping Can Exceed the RGB Gamut
Callers in pure RGB mode always pass values convexly combined from `tonemap()` outputs, which stay strictly inside $[0, 1)$. The `useYCoCg` path does not:
- The clipping box is axis-aligned in YCoCg space, so its corners are not valid RGB colors.
- Clipping can combine $Y$ from one neighborhood tap, $Co$ from a second, and $Cg$ from a third.
- `YCoCg_RGB()` (`r = Y + Co - Cg`, `g = Y + Cg`, `b = Y - Co - Cg`) can then reconstruct a channel in $[-1, 2]$ from taps that were all in $[0, 1)$.
- Only strong lights push tonemapped values close enough to `1.0` for a few percent of YCoCg corner overshoot to cross `1.0`, making the resulting fp16 firefly rare and erratic unless guarded.

To keep the YCoCg reconstruction physically meaningful rather than merely finite, `taa.mat` clamps `YCoCg_RGB(history.rgb)` back to the 3x3 RGB neighborhood box (`rgbBoxMin`, `rgbBoxMax`) before blending:
```glsl
vec3 hRgb = YCoCg_RGB(history.rgb);
history = vec4(mix(hRgb, clamp(hRgb, rgbBoxMin, rgbBoxMax), historyBoxStrength), history.a);
```
Because `[rgbBoxMin, rgbBoxMax]` is the RGB hull of the same 9 taps used to build the YCoCg box, no color actually present in the neighborhood is rejected, and a pixel that would have overshot lands on the brightest valid color in its local neighborhood.

> **Open Issue (`FIXME` in `taa.mat`)**: When `historyBoxStrength` is in $(0, 1)$ (under `motionAdaptiveHistory`), the earlier `mix(history, clippedHistory, historyBoxStrength)` introduces `historyBoxStrength * delta` of the out-of-gamut excursion from `clippedHistory`. Scaling the subsequent RGB gamut clamp by `historyBoxStrength` a second time only removes `historyBoxStrength` of that excursion, leaving `historyBoxStrength * (1 - historyBoxStrength) * delta` (up to $0.25 \times \text{delta}$ at strength $0.5$) past the RGB box. Clamping `clippedHistory` to the RGB gamut at full strength *before* the partial-strength `historyBoxStrength` mix should be investigated.

---

## 3. Input Reconstruction Filter (`filterInput`)

### Anchoring `p` to `uv` Alone (Not `jitter`)
```glsl
highp vec2 reconstructedFragmentPos = uv * inputSize;
highp vec2 closestInputSamplePos = floor(reconstructedFragmentPos) + 0.5;
highp vec2 p = closestInputSamplePos * inputPixelSize;
highp vec2 samplePos = closestInputSamplePos + materialParams.jitter;
vec2 dcenter = samplePos - reconstructedFragmentPos;
```
`p` anchors both the reconstruction taps and the 3x3 neighborhood box `s[0..8]` (reused for deringing and for history box-clipping). `p` **must** be a function of `uv` alone, never `jitter`:
- The neighborhood box represents "plausible current-frame content at this screen position," whereas `jitter` determines where the actual captured sample landed relative to `p` (which is what `dcenter` measures).
- At 1:1 resolution, `reconstructedFragmentPos` is always texel-center-aligned, more than a jitter step away from any texel boundary, so adding `jitter` before `floor()` would never cross a boundary.
- Under upscaling (e.g. 2:1), most output pixels sit within a jitter step of an input texel boundary. If `p` depended on `jitter`, `p` would jump to a different input texel almost every frame on ordinary jitter changes alone, even with a completely static camera and scene. Because temporally smooth history is clipped against the box built around `p`, a jumping `p` makes the *box itself* pop between two unrelated neighborhoods every jitter cycle (observed on `DamagedHelmet.glb` static 2:1 with `ACCURATE` clipping as isolated pixels snapping between two colors at fixed screen locations).

### Polynomial Approximation (`lanczos2`)
```glsl
vec3 lanczos2(vec3 v) {
    highp vec3 x2 = v * v;
    highp vec3 wA = x2 - 4.0;
    return (wA * wA) * ((x2 - 1.0) * (-0.354377 * x2 + 1.0)) * (-1.0 / 16.0);
}
```
This evaluates a degree-4 polynomial in $x^2 = v^2$ on $v \in [-2, 2]$:
$$P(x^2) = -\frac{1}{16} (x^2 - 4)^2 (x^2 - 1) (t x^2 + 1), \quad t = -0.354377$$

- **Load-bearing roots**: The factors $(x^2 - 4)^2$ and $(x^2 - 1)$ place an exact double root at $|x| = 2$ and an exact simple root at $|x| = 1$, matching the true Lanczos-2 kernel's roots and multiplicities, while fixing $P(0) = 1$. If either root is approximate, a neighbor tap at an aligned jitter phase ($|x| = 1$) or at the support boundary ($|x| = 2$) picks up a nonzero spurious weight.
- **Minimax shape parameter $t = -0.354377$**: Once the roots at $|x| \in \{1, 2\}$ and $P(0) = 1$ are fixed, $t$ is the single remaining degree of freedom. Fitting $t$ by minimax error over $[-2, 2]$ against true Lanczos-2 reduces the maximum absolute error from $0.049$ (at the previous $t = -0.25$) to $0.029$, with the largest improvement where taps land most frequently (e.g. at $|x| = 0.5$, error drops from $0.045$ to $0.028$).
- **Deep negative lobe behavior**: For $|x| \in [1.8, 2.0]$, the true kernel is a small negative number while $P(x^2)$ is a slightly smaller positive number (wrong sign, but both within $0.03$ of zero). Those arguments are only reached by `exactLanczosSupport` taps near the extreme quarter of the jitter phase range.
- **Interaction with deringing**: The previous $t = -0.25$ polynomial had an overly deep negative lobe that accidentally compensated for the RGB deringing clamp eating legitimate sharpening overshoot. With the deringing clamp disabled (`taa_harness`, `Fox.glb`), $t = -0.354377$ lands closer to `sharpness_mean = 1.0` against the true Lanczos-2 reference than $t = -0.25$ across every regime tested.

### 3-Tap Truncation vs. `exactLanczosSupport`
At 1:1 resolution, `reconstructedFragmentPos` is at the center of the texel, so `dcenter` lies in $(-0.5, +0.5]$ and the 3x3 taps sit at offsets `dcenter + k` for $k \in \{-1, 0, +1\}$. Because Lanczos-2 has radius 2:
- $k \in \{-1, 0, +1\}$ are always inside $(-2, 2)$.
- Whenever `dcenter != 0`, exactly one of $k = \pm 2$ also lies inside $(-2, 2)$ ($k = -2$ when $\text{dcenter} > 0$, $k = +2$ when $\text{dcenter} < 0$).
- Only at `dcenter == 0` is the 3-tap window complete on its own.

When the 4th tap per axis is dropped (`exactLanczosSupport = false`, the default):
- Sum normalization (`filtered / sum`) preserves DC brightness, but dropping the negative-lobe tap alters the kernel shape in a phase-dependent way.
- At half-texel phase (`|dcenter| = 0.5`), the dropped tap carries weight $-0.105$ against kept weights $(0.618, 0.618, -0.105)$ — roughly 10% of the kernel's weight.
- Because `dcenter` follows the jitter sequence, effective reconstruction sharpness pulses over the jitter cycle (visible if stepping a static scene with `filterHistory: false`).

When `exactLanczosSupport = true` (active only at 1:1, i.e., `!materialConstants_upscaling`):
- The shader fetches the missing column (`sc[0..2]`), row (`sc[3..5]`), and corner (`sc[6]`) at $k_x, k_y \in \{-2, +2\}$ (7 extra fetches, 16 total vs. 9).
- Because `dcenter` depends only on `materialParams.jitter` at 1:1, `dcenter >= 0.0` is uniform across the entire draw call and incurs no warp/wavefront branch divergence.
- `textureLod` with explicit UV offsets is used instead of `textureLodOffset` because `kx`/`ky` signs are only known at runtime; since `materialParams_color` uses nearest sampling, the UV offsets land on exact texel centers.
- `deringMin` and `deringMax` are widened over `sc[0..6]` so deringing does not clamp the 4th-tap negative-lobe sharpening back down to the 3x3 range.
- **Measured (`taa_harness`, `Fox.glb`, 1:1)**: Raises `sharpness_mean` across all regimes (`static` $0.834 \to 0.940$, `camera` $0.808 \to 0.855$, `object` $0.756 \to 0.835$) with `edge_rmse_mean` and `edge_ghost_peak` flat to slightly improved. The one tradeoff is that `camera/1:1`'s per-frame sharpness on the first two frames after motion starts (already $> 1.0$ at $1.02, 1.16$ in the 3-tap baseline) increases to $1.06, 1.22$ (inherent to evaluating the full kernel; disabling the `deringMin`/`deringMax` widening only reduces frame 2's increase by ~15%).
- **Why `exactLanczosSupport` is skipped under upscaling**: Under upscaling, `vx` and `vy` are multiplied by `materialParams.scale` before clamping to $[-2, 2]$. At `scale = 2`, $k = \pm 2$ maps to $|v| \cdot 2 \ge 2.5 > 2.0$ for every reachable `dcenter` ($\in [-0.75, +0.75]$), yielding exact zero weight.

### Deringing Clamp (`deringMin` / `deringMax`)
Reconstruction is clamped in RGB (`filtered = clamp(filtered, deringMin, deringMax)`):
- It must occur in RGB because clamping in YCoCg can generate out-of-gamut RGB colors.
- Because Lanczos-2 achieves sharpness above a box filter by overshooting local neighborhood bounds via its negative lobe, clamping to the neighborhood min/max suppresses legitimate sharpening along with ringing.
- **Measured (`taa_harness`, `Fox.glb`, 1:1, `exactLanczosSupport: true`, against true Lanczos-2 reference)**: Disabling the deringing clamp raises `sharpness_mean` (`static` $0.96 \to 1.06$, `camera` $0.93 \to 0.97$, `object` $0.87 \to 0.94$ with $t = -0.354377$; and $1.08 \to 1.24$, $0.99 \to 1.06$, $0.96 \to 1.08$ with the old $t = -0.25$) while `edge_rmse_mean` and `rmse_mean` remain flat or slightly better.
- The clamp is retained because unclamped overshoot at high-contrast edges can introduce localized ringing not penalized by whole-frame aggregate `sharpness_mean`. Deliberate user-facing sharpening belongs in the post-TAA FSR1 RCAS pass (`TemporalAntiAliasingOptions::sharpness`).

### Upscaling Kernel Support & Zero-Weight `sum` Fallback (`abs(sum) > 0.1`)
```glsl
float sum = wx * wy;
bool hasReconstruction = abs(sum) > 0.1;
filtered = hasReconstruction ? (filtered / sum) : s[0];
```
Under upscaling (`materialConstants_upscaling`), multiplying `vx` and `vy` by `materialParams.scale` keeps the reconstruction filter's support fixed in **output** pixels rather than input texels. Because the input buffer only has one sample per `scale` output pixels on any single frame:
- At `scale = 2` and `dcenter = +-0.5`, the three taps $k \in \{-1, 0, +1\}$ land at scaled offsets $\{-3, -1, +1\}$ or $\{-1, +1, +3\}$, all of which are roots of `lanczos2()` ($\pm 1$ and clamped $\pm 2$).
- Thus `sum = wx * wy` legitimately reaches **exact zero** whenever the current frame's jittered grid places no sample inside this output pixel's support.
- `confidence = sum` already handles this for temporal accumulation: `alpha = materialParams.alpha * confidence * historyAlphaStrength` becomes `0`, so `filtered` contributes nothing to the history blend on that frame, and subsequent jitter phases fill in the missing samples.
- However, `filtered` still feeds the deringing clamp and `clipToBox()`'s target `c`. Unconditional division `filtered / sum` at `sum = 0` produces `Inf`/`NaN`, which `clamp()` cannot reliably contain (`clamp(NaN, ...)` is implementation-defined in GLSL), causing isolated pixels to flash bright/saturated under 2:1 upscaling (`DamagedHelmet.glb`).
- **Why an epsilon-bounded denominator fails**: Because `sum` varies continuously through zero across output pixels and jitter phases, adding or clamping by `epsilon` (`max(abs(sum), epsilon)`) faces an impossible tradeoff. An `epsilon` small enough to leave valid low-`sum` reconstructions unperturbed fails to stop the `DamagedHelmet.glb` flashing; an `epsilon` large enough to stop it perturbs legitimate reconstructions across the image (measured on `Fox.glb` 2:1: `edge_rmse_mean` $+15\text{--}28\%$, `sharpness_mean` pushed past $1.0$, and a spurious `edge_recovery_frames` event on static zero-motion).
- **Fallback to `s[0]` at `abs(sum) <= 0.1`**: Falling back to the already-fetched, in-gamut center sample `s[0]` when `|sum| <= 0.1` leaves every pixel above `0.1` completely unperturbed (statistically indistinguishable from baseline on `Fox.glb` 2:1 across `static`, `camera`, and `object`) while eliminating the `DamagedHelmet.glb` 2:1 flicker (`0.1` yields 642 pixels changing $> 40\text{ LSB}$/frame-pair vs. 616 at `0.3`, capturing the improvement before the `Fox.glb` quality degradation that starts appearing around `0.3`).

---

## 4. History Clamping Box (`clipToBox`, `AABB_VARIANCE`, `kMinBoxHalfWidth`)

### 9-Tap Variance Box (`BOX_TYPE_AABB_VARIANCE`) and `varianceGamma`
When `boxType == BOX_TYPE_AABB_VARIANCE` (Salvi, *"An Excursion in Temporal Supersampling"*), the 5-tap/9-tap AABB (`mix(box5, box9, 0.5)`) is intersected with $\mu \pm \gamma \sigma$:
- **9 taps, not 5**: The moment accumulation loops over all `count = 9` samples (`s[0..8]`), including the 4 diagonal neighbors `s[5..8]`. Because diagonal taps sit $\sqrt{2}$ texels from center, they capture real spatial variance that the 5-tap orthogonal cross misses.
- **Paired `varianceGamma = 0.7`**: Including diagonals widens $\sigma$ across the board (helping static stability at the expense of motion ghosting rejection). Lowering the default `varianceGamma` from `1.0` to `0.7` compensates: measured with `taa_harness` on both `Fox.glb` and `DamagedHelmet.glb`, 9-tap + `0.7` improves `edge_rmse_mean` on `static/1:1`, `camera/1:1`, and `static/2:1` over the old 5-tap + `1.0` pairing, trading only ~1.2–1.3% on `camera/2:1`.
- **CPU Upscale Scaling (`PostProcessManager::taa`)**: Under upscaling, `s[0..8]` are sampled at render-texel granularity around `p`, which is shared by multiple output pixels. Without scaling, clamping every output pixel in that render texel to the same render-texel $\sigma$ destroys the finer output-resolution detail accumulated across jitter frames. `PostProcessManager.cpp` multiplies `varianceGamma` by `taaOptions.upscaling` when `upscaling > 1.0`:
  - **Measured (`DamagedHelmet.glb`, 2:1 `static`)**: Unscaled `AABB_VARIANCE` drops `sharpness_mean` from `AABB`'s $0.96$ down to $0.81$ and increases `edge_rmse_mean` by $+44\%$. Scaling `varianceGamma` by `upscaling` restores `sharpness_mean` to $0.96$ while preserving most of `AABB_VARIANCE`'s camera-motion advantage over `AABB` (`camera/2:1` `edge_rmse_mean` $0.069$ vs. `AABB`'s $0.072$).

### Minimum Box Half-Width (`kMinBoxHalfWidth = 0.002`)
```glsl
vec3 boxCenter = (boxmin + boxmax) * 0.5;
boxmin = min(boxmin, boxCenter - kMinBoxHalfWidth);
boxmax = max(boxmax, boxCenter + kMinBoxHalfWidth);
```
On low-variance axes — most notably YCoCg's $Co$ and $Cg$ channels on low-chroma-variance (mostly-gray or metallic) surfaces — the local neighborhood spread can collapse to nearly a single point:
- In `BOX_CLIPPING_ACCURATE`, the fast path (`if (any(lessThan(h.rgb, boxmin)) || any(greaterThan(h.rgb, boxmax)))`) only skips the ray-cast if history is already inside the box.
- When the box collapses to a near-point along $Co$ or $Cg$, ordinary temporal history noise falls outside that axis on almost every frame, triggering the ray-cast to pull history all the way toward the current frame's (noisy, especially under upscaling) reconstruction `c` every frame.
- **Measured (`DamagedHelmet.glb`, `static`, `ACCURATE + useYCoCg`)**: Without `kMinBoxHalfWidth`, `edge_flicker_variance` was $+47\%$ worse than RGB at 1:1 and grew to $+54\%$ worse at 2:1. Under `BOX_CLIPPING_CLAMP` (which clamps per-channel without ray-casting all channels toward `c`), the YCoCg penalty was only $+7\% / +18\%$, confirming `ACCURATE`'s ray-cast on degenerate single-channel bounds as the amplifier.
- **Tuning `0.002` vs. `0.005`**: Because the floor is applied unconditionally around `boxCenter`, it only widens boxes narrower than `2 * kMinBoxHalfWidth` and leaves high-contrast motion edges untouched. A wider `0.005` cut the YCoCg flicker penalty to $+16\% / +16\%$ but cost $3\text{--}8\%$ on `edge_ghost_peak` and `rmse_mean` in RGB camera/object motion (`Fox.glb`). `kMinBoxHalfWidth = 0.002` keeps the RGB motion cost under $1.5\%$ across 1:1 and 2:1 while reducing the `ACCURATE + useYCoCg` penalty from $+47\% / +54\%$ to a flat $+29\% / +30\%$ (matching `CLAMP`'s inherent YCoCg floor).

### `clipToBox()` Numerical Defenses
```glsl
vec3 cClamped = clamp(c.rgb, boxmin, boxmax);
vec4 r = vec4(cClamped, c.a) - h;
vec3 rSign = step(0.0, r.rgb) * 2.0 - 1.0;
vec3 ir = 1.0 / (r.rgb + rSign * epsilon);
```
1. **Defensive clamping of `c` into `[boxmin, boxmax]`**:
   - The `h -> c` ray-box intersection assumes the target `c` lies inside the box so the segment crosses the box boundary at some $t \in [0, 1]$.
   - `filtered` (`c`) is deringed against the looser 9-tap RGB box (`deringMin`/`deringMax`), whereas `clipToBox()` is called with the tighter `mix(box5, box9, 0.5)` (and `AABB_VARIANCE`) box. At sharp edges, `c` can sit outside `[boxmin, boxmax]`, which previously caused the ray-cast to yield $t = 0$ and return a stale `h` completely unmodified.
2. **Signed `epsilon` in the ray denominator**:
   - When a channel of `cClamped` and `h` is nearly stationary ($r \approx 0$), $r$ can approach zero from *either* the positive or negative side.
   - An unsigned `1.0 / (r.rgb + epsilon)` only guards $r \ge 0$ and shifts the singularity to $r = -\epsilon$, which stationary channels cross frequently — observed as isolated pixels snapping between two plausible colors frame-to-frame ("popping") at fixed screen locations exclusively under `BOX_CLIPPING_ACCURATE`. Pushing `epsilon` in the sign of `r.rgb` (`rSign`) keeps the denominator bounded away from zero for both signs.

---

## 5. Motion-Adaptive History (`motionAdaptiveHistory`)

On static scenes with sub-pixel geometry or normal-mapped specular detail, full-strength box clipping and fixed `alpha` feedback fight the jittered accumulation every frame, causing steady-state flicker (see the `SubpixelStar` ring-binned table in [`tools/taa/taa_harness.md`](../../../../../tools/taa/taa_harness.md): below $0.7\text{ px}$, default box clipping flickers *more* than `boxClipping: NONE`).

When `motionAdaptiveHistory` is enabled, `taa.mat` relaxes `historyBoxStrength` and `historyAlphaStrength` from `1.0` down to `minBoxStrength` (`0.1`) and `minAlphaStrength` (`0.3`) when `reprojectionDeltaLen` is small:
```glsl
float rawMotionAmount = motionAmount(reprojectionDeltaLen);
float farness = 1.0 - smoothstep(0.0, materialParams.skyDepthThreshold, depth);
rawMotionAmount = max(rawMotionAmount, farness);
rawMotionAmount = max(rawMotionAmount, materialParams.motionHoldAmount);

historyBoxStrength = mix(materialParams.minBoxStrength, 1.0, rawMotionAmount);
historyAlphaStrength = mix(materialParams.minAlphaStrength, 1.0, rawMotionAmount);
```

### Floor Selection (`minBoxStrength = 0.1`, `minAlphaStrength = 0.3`)
- Both floors must remain $> 0$ so static pixels still respond to non-camera changes (lighting changes, animated materials) and avoid numerical drift.
- **`minBoxStrength = 0.1`**: Because the box is a rejection clamp rather than the temporal integrator, lowering it does not bound how long a lighting change takes to appear. On `SubpixelStar` ($0.53\text{ px}$ feature ring, static 1:1), lowering `minBoxStrength` from `0.35` to `0.1` reduces temporal standard deviation from $0.0250$ to $0.0173$ (vs. $0.0746$ without relaxation), at the cost of slower ghosting cleanup on moving objects under a static camera (`Fox.glb` `object` regime `edge_rmse_mean` $0.0556 \to 0.0622$).
- **`minAlphaStrength = 0.3`**: Directly scales the EMA integration rate $\alpha_{\text{eff}} = \text{feedback} \times \text{minAlphaStrength}$, which sets how many frames $n = \ln(0.25) / \ln(1 - \alpha_{\text{eff}})$ a step change in lighting takes to reach 75% convergence on a static camera. At `feedback = 0.12`, `0.5` is ~22 frames ($0.37\text{ s}$ at 60 Hz), `0.3` is ~38 frames ($0.63\text{ s}$), and `0.1` is ~115 frames ($1.9\text{ s}$). `0.3` captures most of the static flicker reduction (`SubpixelStar` $0.0121$ vs. $0.0079$ at `0.1`) without a two-second lag on static lighting changes.

### Sky / Far-Plane Override (`skyDepthThreshold = 0.001`)
Pure camera translation produces zero reprojection UV delta at the far plane (reversed-Z `depth ~ 0`). When a foreground object moves or camera translation uncovers sky, `reprojectionDeltaLen` on the newly exposed sky texel reads ~0 even though history holds the foreground object. `farness` forces `rawMotionAmount = 1.0` below `skyDepthThreshold`.

### Global Camera-Motion Hold-Off (`motionHoldFrames = 12.0`, `motionHoldAmount`)
`reprojectionDeltaLen` is memoryless: the instant the camera stops moving, every pixel's `reprojectionDeltaLen` drops to `0` on the very next frame. Without a hold-off, pixels that were disoccluded just before the camera stopped immediately drop to relaxed box/alpha strength ($3.6\%$ effective alpha), freezing residual ghosting in place for dozens of frames.

- `PostProcessManager::TaaJitterCamera` compares the unjittered `projection * view` matrix against the previous frame's (`kCameraStaticThreshold = 1e-6`). Whenever the camera moves (or history is newly initialized), `motionHoldRemaining` resets to `motionHoldFrames` (`12.0`), then counts down by `1.0` per static frame, passing `motionHoldAmount = motionHoldRemaining / motionHoldFrames` in $[0, 1]$ to the shader.
- **Sizing `motionHoldFrames = 12.0`**: For an exponential moving average $h_n = (1 - \alpha) h_{n-1} + \alpha c$, a step change decays to fraction $T = 0.25$ in $n = \ln(0.25) / \ln(1 - \alpha)$ frames: ~10.8 frames at full `feedback = 0.12` (and ~37.8 frames at the fully relaxed rate $\text{feedback} \times \text{minAlphaStrength} = 0.036$; in practice recovery is even faster than 10.8 frames because box clamping assists during recovery: 5–6 frames on `Fox.glb` and `DamagedHelmet.glb` at 1:1). Holding for `12` frames outlasts natural disocclusion recovery before relaxation resumes, whereas raising it to `24` or `38` gave no further `camera/1:1` improvement on `Fox.glb` (`edge_ghost_peak` $0.0370$, recovery 4 frames) and slightly hurt `camera/2:1` ($0.0612 \to 0.0622 \to 0.0633$).
- **Alignment of `feedback = 0.12` with `HALTON_23_X16`**: Under the same EMA model, the effective accumulated sample count in steady state is $(2 - \alpha) / \alpha \approx 15.7$ at the default `feedback = 0.12` — closely matching one full 16-frame `HALTON_23_X16` jitter cycle so the EMA memory window spans approximately one complete cycle of sub-pixel offsets before older samples are forgotten. Changing `feedback` or `jitterPattern` shifts both that alignment and the recovery-time numbers above.

---

## 6. Depth-Based Disocclusion (`depthDisocclusion`)

Because Filament does not generate per-object motion vectors, `reprojectionDeltaLen` only reflects camera motion. A moving object under a static camera has `reprojectionDeltaLen == 0`, and even under camera motion, a stale background history sample can accidentally lie inside the current frame's color box at a silhouette edge. `depthDisocclusion` detects surface changes directly by comparing reprojected depth against the previous frame's depth buffer (`depthHistory`).

### Raw Reversed-Z Comparison & Bidirectional Delta
```glsl
highp float expectedHistoryDepth = q.z * invQW;
highp vec2 jitterCorrection =
        (materialParams.jitter - materialParams.previousJitter) *
                materialParams.colorResolution.zw;
highp vec2 historyDepthUv = historyUv + jitterCorrection;
highp float historyDepth = textureLod(materialParams_depthHistory, historyDepthUv, 0.0).r;
highp float delta = historyDepth - expectedHistoryDepth;
```
- **Raw reversed-Z**: Depth is compared directly in reversed-Z clip space (`q.z / q.w` vs. `depthHistory`), avoiding `linearizeDepth()`'s division blow-up near the far plane and matching the space used by `skyDepthThreshold`.
- **Both signs of `delta` (`abs(delta)`)**: In reversed-Z (larger = nearer), `delta > 0` means history held a nearer surface (a hidden background surface was uncovered — a true disocclusion), while `delta < 0` means a nearer surface arrived in front of what used to be there (an occlusion / leading edge). Both mean history belongs to a different surface. Rejecting only `delta > 0` leaves leading edges uncorrected and also fires on only one of the two states a jittered silhouette alternates between, rejecting history every other frame and amplifying flicker.

### Inter-Frame Jitter Grid Realignment (`jitterCorrection`)
Unlike `history` (which stores the resolved TAA output centered on unjittered pixel centers), `depth` and `depthHistory` are raw rasterized depth buffers captured at each frame's sub-pixel jitter offset. Their sample grids are displaced from one another by `jitter - previousJitter` (up to a full render texel).
- Without `jitterCorrection`, a static silhouette straddles the two misaligned depth grids and reports a full surface-to-background depth jump every frame with nothing moving.
- **Render resolution (`colorResolution.zw`), NOT `historyResolution.zw`**: `jitter` is expressed in render-resolution texels and both `depth` and `depthHistory` are at render resolution, whereas `historyResolution` is the upscaled output resolution. Using `historyResolution.zw` compiles and works at 1:1, but is off by the upscaling factor whenever `upscaling > 1.0`.

### Exact 3x3 Depth Neighborhood Spread (`depthNeighborhoodRange`)
At a silhouette edge, a tiny sub-texel shift of the sample point produces the same full surface-to-background depth jump as a real parallax disocclusion, so no threshold on `|delta|` alone can separate jitter from disocclusion.

On a static scene, after `jitterCorrection` aligns the two sample grids to within the half-texel rounding of point sampling, `historyDepth` is guaranteed to be a sample from inside the 3x3 render-texel neighborhood around `uv`. Therefore, the 3x3 depth spread `depthRange.y - depthRange.x` is an **exact geometric bound** on how much `|delta|` can be explained by sub-texel jitter on a static scene:
- **All 9 taps (including diagonals)**: A thin feature crossing a texel corner (fox legs, foliage, wires) can leave all 4 orthogonal neighbors on the same surface while a diagonal neighbor sits across the full background depth jump. Any 4-tap or `fwidth()` estimate reports ~0 spread on those texels, which are the exact texels a jitter shift can flip.
- **Why this replaced `kJitterSpanTexels * depthGradient`**:
  - Using `fwidth(depth)` at 1:1 computed derivatives per 2x2 GPU quad; whenever a silhouette texel's quad partner sat on the same side of the edge, `fwidth()` reported ~0 gradient next to a full depth jump. On `Fox.glb` `static/1:1` with nothing moving, that fired on ~130 texels/frame and regressed `edge_flicker_variance` by $+292\%$, `edge_ghost_peak` by $+162\%$, and `edge_rmse_mean` by $+31\%$.
  - Using a 4-tap cross gradient under upscaling was blind to diagonal corners and $2\times$ looser than necessary at hard silhouettes (`2 * jump` tolerated twice the maximum possible resample jump).
- **Measured (`Fox.glb` & `DamagedHelmet.glb`, `depthDisocclusion` alone vs. off)**: `static/1:1` and `static/2:1` match the feature-off baseline to five significant figures (0–2 firing texels per $512 \times 512$ frame, down from ~130), while `camera/1:1` retains its ghosting reduction (`edge_rmse_mean` $0.036 \to 0.027$, `edge_ghost_peak` $0.040 \to 0.024$).

### Inherent Geometric Limits (Sub-Texel Object Drift & Leading Edges)
1. **Sub-texel object motion under a static camera**: Without per-object motion vectors, an object moving $< 1$ render texel/frame under a static camera is geometrically indistinguishable from a sub-texel jitter resample using depth point samples alone. `Fox.glb`'s default `Survey` animation (`-A 0`) is entirely in this sub-texel regime and is a no-op under the exact bound (whereas `fwidth()`'s quad blind spot previously fired indiscriminately on its silhouettes). On the `Run` animation (`-A 2`), where limbs move several texels per frame, `depthDisocclusion` fires on ~2000 texels/frame during motion and drops to 0 once motion stops.
2. **Leading edges**: When a foreground surface arrives over background, the current frame's 3x3 depth neighborhood straddles both surfaces, so the depth jump is contained within `depthRange.y - depthRange.x` and cannot be separated from a jitter coverage flip by depth point samples alone.

### Camera-Motion Tolerance Fade & the 1-Texel Dead Zone (`kMotionFadeStartTexels = 1.0`, `kMotionFadeEndTexels = 2.0`)
```glsl
highp float jitterExplainable = (depthRange.y - depthRange.x) *
        (1.0 - smoothstep(kMotionFadeStartTexels, kMotionFadeEndTexels,
                reprojectionDeltaLen));
```
Under camera motion, history at a silhouette is genuinely stale even when `|delta|` sits inside the local 3x3 depth spread (dropping the fade entirely regresses `Fox.glb` `camera/1:1` `edge_ghost_peak` from $0.024$ back to $0.034$, vs. $0.040$ with `depthDisocclusion` off).

However, the fade **must not start at 0 displacement**:
- At any depth discontinuity, both `|delta|` and `depthRange.y - depthRange.x` equal the full depth jump across the edge (typically $0.1\text{--}0.5$ in reversed-Z), so `evidence = abs(delta) - jitterExplainable` reduces to $\text{jump} \times \text{fade}$.
- Because `disocclusion` ramps over an absolute `disocclusionDepthThreshold = 0.001`, shaving even 1% off `jitterExplainable` produces $\text{evidence} > 0.001$ and saturates `disocclusion` to `1.0` (full history rejection).
- When the fade started at `0.0` (via `1.0 - motionAmount()`, which is quadratic from zero and reaches 1 by `motionDeltaThreshold = 1.0` texel), a slow camera drift of `0.1` render texels/frame rejected history on ~150 silhouette texels every frame on `Fox.glb` (`--camera-motion translate --camera-texels-per-frame 0.1`). This affects slow pans, camera ease-in/ease-out, and `FreeFlightManipulator` damping after key release.
- Geometrically, for any reprojection displacement up to 1 render texel, `historyDepth` is still resampled from inside the 3x3 neighborhood covered by `depthRange`, so the static bound remains exact up to `kMotionFadeStartTexels = 1.0` and fades out over `[1.0, 2.0]` render texels (`kMotionFadeEndTexels = 2.0`).

### Mirrored History-Side Neighborhood Test (`disocclusionHistoryNeighborhood`)
```glsl
highp float evidence = abs(delta) - jitterExplainable;
if (materialConstants_disocclusionHistoryNeighborhood) {
    highp vec2 historyRange = depthNeighborhoodRange(
            materialParams_depthHistory, historyDepthUv, historyDepth);
    evidence = min(evidence, max(expectedHistoryDepth - historyRange.y,
                                 historyRange.x - expectedHistoryDepth));
}
disocclusion = smoothstep(0.0, materialParams.disocclusionDepthThreshold,
        max(0.0, evidence));
```
- **Why the current-side bound alone is one-sided**: `depthRange` checks whether `historyDepth` exists in the *current* frame's 3x3 depth neighborhood. When a static feature is thinner than a render texel (wires, fences, railings, foliage), the previous frame's jittered sample can hit the feature while all 9 of the current frame's samples miss it. `depthRange` then contains only background depth while `|delta|` is the full foreground-to-background jump, causing static sub-texel features to reject history on their own edges every frame.
- **Two-sided test**: `disocclusionHistoryNeighborhood` (default `true`) asks the mirrored question: does `expectedHistoryDepth` fall outside the 3x3 neighborhood of `depthHistory` around `historyDepthUv`? For a static sub-texel feature that was hit last frame, the feature is present in `depthHistory` (which is where `historyDepth` came from), so `historyRange` spans the jump and suppresses the false positive. For a genuine disocclusion, the reprojected history neighborhood was entirely occluded last frame and contains only the occluder's depth, so both sides agree and `evidence` remains high.
- **Measured (`tools/taa/models/SubpixelStar/SubpixelStarDepth.glb`, `static/1:1`)**: Reduces false-positive firing texels from ~2000/frame (current-side only) to ~230/frame (two-sided; the remaining ~230 occur where ~1-texel-period aliasing leaves one frame's 3x3 all gap and the other all feature). Costs 9 extra `depthHistory` taps and a small reduction in camera-motion sensitivity (`Fox.glb` `camera/1:1` `edge_ghost_peak` $0.0251 \to 0.0277$, vs. $0.0399$ with `depthDisocclusion` off), so it is gated by a specialization constant.

### Applying `disocclusion` Without Relaxing Baseline Rejection
1. **Strength restoration (`max`, never `mix`)**:
   ```glsl
   historyBoxStrength = max(historyBoxStrength, disocclusion);
   historyAlphaStrength = max(historyAlphaStrength, disocclusion);
   ```
   `depthDisocclusion` must only ever *increase* rejection strength toward `1.0`. Folding `disocclusion` into a shared `mix(min*Strength, 1.0, ...)` when `motionAdaptiveHistory` is off would relax rejection to `min*Strength` everywhere `disocclusion == 0` (i.e. across almost the entire image), globally worsening ghosting whenever `depthDisocclusion` is enabled on its own.
2. **Direct history replacement after `clipToBox()`**:
   ```glsl
   if (materialConstants_depthDisocclusion) {
       history = mix(history, target4, disocclusion);
   }
   ```
   At a moving object's silhouette, the current frame's 3x3 color neighborhood straddles both background and object colors, so a stale background `history` sample often already lies inside `[boxmin, boxmax]`. `BOX_CLIPPING_ACCURATE` skips any sample already inside the color box, which would leave the disoccluded pixel to decay slowly via `alpha` (~10+ frames) unless `disocclusion` pulls `history` directly toward `target4`.

