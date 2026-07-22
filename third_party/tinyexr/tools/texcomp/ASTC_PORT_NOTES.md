# Closing the astcenc speed gap: port vs. continue

Status 2026-07-02 (branch texcomp-astc-quality, commit 12c2ae5), single
thread, 1024x1024 photo, 6x6, pure coding time:

| tier   | ours              | astcenc (-j 1)          | gap  |
|--------|-------------------|-------------------------|------|
| fast   | 0.176 s, 47.30 dB | -fastest 0.045 s, 47.78 | 3.9x |
| medium | 0.650 s, 48.43 dB | -medium  0.095 s, 48.63 | 6.8x |

## Option A: wholesale C11 port of astcenc

License: astcenc is **Apache-2.0** (not BSD). Permissive and compatible
with shipping inside a BSD-3-Clause tree, but ported files must retain
the Arm copyright + Apache-2.0 notice and the top-level NOTICE must say
so. tools/texcomp already mixes licenses knowingly (see NOTICE.md).

Scope: ~20k lines of C++14 built around a mandatory SIMD abstraction
(vfloat4/vint4 with SSE/NEON/SVE backends), float-based pipeline,
context/config machinery, and large generated tables. A faithful C11
port is a multi-week effort and would replace, not reuse, the current
integer encoder — including the reference-decoder test harness whose
bit-exact integer contract does not hold for a float pipeline.

## Option B (recommended): port astcenc's *algorithms*, keep our C11 core

The measured 4-7x comes from identifiable techniques, portable one at a
time into the existing encoder with the PSNR/parity harness as a gate:

1. **Block-mode percentile tables** (astcenc_block_sizes.cpp +
   percentile data): empirically ranked mode usage per footprint;
   -fastest keeps only the top few percent. Replaces our heuristic
   weight_bits+weight_count ranking; biggest search-space lever left.
2. **8-wide kernels** (astcenc_vecmathlib): widen our SSE2 kernels to
   AVX2 and add the LSQ-sum + infill kernels (33% + 15% of the fast
   profile are still scalar).
3. **Iterative ideal-endpoints-and-weights refinement**
   (astcenc_ideal_endpoints_and_weights.cpp): alternate weight/endpoint
   solves instead of our single LSQ pass — quality headroom that buys
   back search-count cuts.
4. **Bounded trial counts** (tune_candidate_limit etc.): astcenc's
   preset knobs map cleanly onto our scan_cap/selected_limit constants.

Measured dead end (do not revisit): winner-only LSQ — 10-20% speed for
0.2-1 dB; the current inline-LSQ config is on the frontier.

Threading remains the biggest single lever (astcenc numbers above are
already single-thread; ours parallelizes trivially per block row now
that the encode context is heap-allocated).

## Measured: AVX2 widening of the recon kernels does not pay (2026-07-02)

8-texel AVX2 variants of the recon/recon_pt kernels were bit-exact but
*slower* (fast 0.131 -> 0.170 s, medium 0.512 -> 0.751 s): at 6x6 the
kernels see 36-texel calls, so the per-call ymm constant/shuffle-mask
setup and the SSE tail dominate. AVX2 only makes sense with a batched
pipeline (many blocks' texels processed per call, astcenc-style), not
by widening the current per-candidate call shape. Reverted.

## Status after trial-count tuning (2026-07-02, ISA-matched comparison)

astcenc built with -DASTCENC_ISA_SSE2 (same 128-bit width as our
kernels; also representative of the NEON-vs-NEON situation on Arm):

| tier   | ours              | astcenc-sse2 (-j 1)      | gap  |
|--------|-------------------|--------------------------|------|
| fast   | 0.085 s, 47.16 dB | -fastest 0.057 s, 47.78  | 1.5x |
| medium | 0.538 s, 48.39 dB | -medium  0.119 s, 48.63  | 4.5x |

Anomaly (unexplained, reverted): cutting medium trial counts
(pass1 20->12, shortlist 16->8, partition candidates 24->12) made q1
*slower* (0.54 -> 0.69 s) as well as worse (48.39 -> 47.95) - fewer
fits should not cost time; investigate the interaction with the
partition/dual dispatch before retrying medium cuts. Closing medium
further likely needs astcenc's batched pipeline shape.

## Port complete (2026-07-02, through eb09ab2)

All four planned astcenc algorithm ports have landed: (1) block-mode
percentile tables, (2) weight-space candidate scoring (per-texel form of
compute_error_of_weight_set; grid-point-only scoring mis-ranks coarse
grids on high-frequency content), (3) iterative endpoint/weight
refinement, (4) trial-count presets. Final single-thread standings
(1024x1024 photo, 6x6, pure coding, ISA-matched astcenc-sse2 -j 1):

| tier   | ours              | astcenc-sse2             | gap  |
|--------|-------------------|--------------------------|------|
| fast   | 0.080 s, 47.14 dB | -fastest 0.057 s, 47.78  | 1.4x |
| medium | 0.400 s, 48.17 dB | -medium  0.119 s, 48.63  | 3.4x |

Notes: the dual-plane path keeps exact candidate fits (weight-space
scores tie across direct grids and mis-pick); the normal tier keeps
exact scoring everywhere for quality. The remaining medium delta needs
either the wholesale batched-pipeline rewrite (rejected above) or
threading (deferred by project decision; multiplies both columns'
relationship unchanged since astcenc scales the same way).

## Tried and REJECTED: blue-contract endpoint trial (2026-07-03)

Prototyped the astcenc-style blue-contract encoding trial for CEM 8/12 at
quality 2 only: at emit, also encode the inverse-blue-contract preimages of
the chosen endpoints (V1=invcontract(lo), V0=invcontract(hi), taking the
decoder's swap path when stored sum(V0)>sum(V1)), score both direct and
contract encodings against the block with the fixed winning weights via
tc_astc_recon_sse, and keep the lower-SSE one. Correct and strictly
non-regressing (picks better of two), but measured gain is below the 0.05 dB
keep threshold on a real corpus:

| image (q2)  | 4x4    | 6x6    | 8x8    |
|-------------|--------|--------|--------|
| StillLife   | +0.001 | +0.005 | +0.006 |
| Desk        | +0.001 | +0.003 | +0.003 |
| Carrots     | +0.009 | +0.032 | +0.036 |
| MtTamWest   | +0.003 | +0.034 | +0.027 |
| Tree        | +0.002 | +0.014 | +0.014 |
| asakusa.png | +0.030 | +0.080 | +0.043 |

Corpus average ~0.02 dB (best real EXR ~0.034 dB at 6x6); smooth synthetic
gradients gain ~0.000. Not worth ~90 lines of subtle endpoint bit-packing
plus a per-CEM8/12-block rescore in the emit path. Decoder already handles
the swap+contract path correctly (aref_blue_contract), so this is purely an
encoder-side quantization choice; revisit only if it can be folded into the
per-candidate LSQ quantization for free rather than as a separate emit trial.

## Medium-tier gap: real-photo characterization (2026-07-03)

The value-noise photo1k benchmark substantially OVERSTATED competitiveness.
Re-measured on real photos (OpenEXR ScanLines, linear->sRGB tonemapped to
8-bit, 6x6, single thread, pure coding, decoded with astcenc unorm8; our
output verified to decode identically under astcenc fp16/unorm8 and the
in-tree reference decoder, so the gap is real, not a decode-mode artifact):

| photo     | ours q1 (medium) | ours q2 (normal) | astcenc -fast | astcenc -medium |
|-----------|------------------|------------------|---------------|-----------------|
| StillLife | 0.16 s / 40.82   | 3.37 s / 42.12   | 0.13 s / 42.83| 0.43 s / 43.53  |
| Desk      | 0.08 s / 33.25   | 2.04 s / 35.35   | 0.27 s / 36.58| 0.62 s / 37.40  |
| Carrots   | 0.04 s / 42.19   | 0.73 s / 42.95   | 0.03 s / 43.17| 0.12 s / 44.64  |

On real content astcenc's -fast preset beats OUR -normal (e.g. Desk 36.58 vs
35.35), and our medium trails astcenc-medium by 1.7-4.2 dB. Contrast the
value-noise photo1k, where our normal is only -0.29 dB from astcenc-medium
(44.48 vs 44.77) -- that number, and the memory's "-0.32 dB", reflect
high-frequency noise where weight error dominates and endpoint/partition
precision barely matters. It is not representative of photographic content.

The medium quality KNOBS ARE SATURATED. Cranking every medium lever at once
(opaque scan 24, selected 8, full-axis 4, partition scan 32, part2 shortlist
48) moves StillLife medium only +0.10 dB (40.82 -> 40.92) for 2.2x the time.
partition-scan and part2-shortlist knobs move it 0.000 dB. So there is no
knob configuration that meaningfully closes the gap, and none was shipped.

The scan-cap non-monotonicity (opaque scan 10 -> 16 gets WORSE, 16 -> 20
recovers) is now understood: pass 1 proxy-scores `scan_cap` candidates with
score_from_ideal and shortlists the top `selected_limit`; pass 2 exact-fits
only the shortlist. The proxy is imperfectly correlated with the exact fit,
so a larger scan pool can admit proxy-good/exact-bad candidates that evict
proxy-mediocre/exact-good ones. It is a selection imperfection (~0.1 dB), not
a correctness bug, and is dwarfed by the architectural gap.

Conclusion: the real-photo medium (and normal) gap is a QUALITY-CEILING
problem in the partition / dual-plane / endpoint search, not effort tuning.
Closing it needs algorithmic work (astcenc's batched trial-partitioning +
decimation pipeline), which remains the deliberately-deferred big rewrite.
Knob tuning is exhausted; do not re-sweep it expecting real-content gains.
Harness to reproduce lives in the session scratchpad: genphoto.c (photo1k),
exr2png.c (tonemap), ourtime.c (our timer+PSNR), astctime.c (astcenc-sse2
ref), sweepimg.sh (knob A/B).
