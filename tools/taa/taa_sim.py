#!/usr/bin/env python3
"""1-D simulation of Filament's TAA accumulation.

Exists because taa_harness measures the whole pipeline on real content, which is the right tool
for "did this change help" and the wrong one for "why does upscaling behave differently". This
strips away the GPU, the polynomial kernel approximation, YCoCg, ACCURATE's ray-clip, and two
dimensions, leaving only the accumulation arithmetic -- where the answer turned out to be.

Models what taa.mat does per frame, in one dimension:

    xr       = output pixel centre expressed in RENDER texels
    k        = floor(xr)                      the nearest render texel
    dcenter  = (k + 0.5 + jitter) - xr        that texel's sample offset from the pixel centre
    w[m]     = lanczos2(dcenter + m, scale)   m = -1, 0, +1
    filtered = sum(w*tap)/sum(w)              or tap[0] when |sum| <= 0.1
    alpha    = feedback * (sum if upscaling else 1) * alpha_strength
    history  = clip(history, box) then mix(history, filtered, alpha)

lanczos2(v, scale) clamps v*scale to [-2, 2] when upscaling, which makes the kernel a fixed size
in OUTPUT pixels at any scale. So an unbiased accumulation must converge to the same result at
1:1 and 2:1, and it does -- 2:1 is in fact marginally sharper, because at 1:1 the 3-tap window
truncates a Lanczos tap that at scale 2 falls inside the support.

What it found (effective MTF at 2:1, cycles per output pixel):

               f=0.1   f=0.2   f=0.3   f=0.4
    no clamp   0.995   0.966   0.874   0.704
    deringing  0.964   0.885   0.800   0.670     <- a few percent
    box clamp  0.934   0.561   0.574   0.097     <- an order of magnitude

The history box, not deringing, is what costs upscaled sharpness, and it is also the dominant
2:1 flicker source (temporal variation over one jitter cycle on a STATIC signal: 0.043 clamped
against 0.019 unclamped, the latter being the 1:1 level). The box is built from render-resolution
taps but clamps history, which lives at output resolution and has accumulated detail no single
render-resolution neighborhood contains -- and because the clamp runs every frame inside the
accumulation loop it is a contraction, destroying that detail as fast as jitter builds it.

Two things that follow, both confirmed against taa_harness afterwards:

  - Widening the box by the upscale factor is NOT the fix. It recovers the band below render
    Nyquist and then plateaus, because above it the box's own CENTRE is aliased and there is
    nothing correct to widen around.
  - Box STRENGTH is the lever, not box width, and strength sets a rate rather than a magnitude:
    at 0.1 strength the clamp still costs 27% of the response at f=0.4, because a 10%-per-frame
    pull repeated to equilibrium still lands most of the way in.

Usage: run directly for the MTF table, or import accumulate()/mtf() and drive them.
"""

import numpy as np

def lanczos2_true(x):
    x = np.asarray(x, float)
    out = np.zeros_like(x)
    m = np.abs(x) < 2.0
    xm = x[m]
    s1 = np.where(xm == 0, 1.0, np.sin(np.pi*xm)/(np.pi*xm + 1e-300))
    h = xm/2.0
    s2 = np.where(h == 0, 1.0, np.sin(np.pi*h)/(np.pi*h + 1e-300))
    out[m] = s1*s2
    return out

def lanczos2_scaled(v, scale, upscaling):
    if upscaling:
        v = np.clip(v*scale, -2.0, 2.0)
    return lanczos2_true(v)

def halton(i, b):
    f, r = 1.0, 0.0
    while i > 0:
        f /= b; r += f*(i % b); i //= b
    return r

JITTER = np.array([halton(i+1, 2) - 0.5 for i in range(16)])

def accumulate(f, N_out, scale, frames=400, feedback=0.12,
               dering=False, boxclamp=False, sum_threshold=0.1,
               confidence_weighting=True, taps=(-1, 0, 1)):
    """Run the accumulation and return the converged output (N_out values)."""
    up = scale > 1
    N_ren = N_out // scale
    xi = np.arange(N_out) + 0.5
    xr = xi/scale
    k = np.floor(xr).astype(int)
    hist = np.zeros(N_out)
    for n in range(frames):
        j = JITTER[n % len(JITTER)]
        pos = (np.arange(N_ren) + 0.5 + j)*scale     # sample positions, output units
        s = f(pos)
        dcenter = (k + 0.5 + j) - xr
        w = np.stack([lanczos2_scaled(dcenter + m, scale, up) for m in taps])
        tap = np.stack([s[(k + m) % N_ren] for m in taps])
        ssum = w.sum(0)
        ok = np.abs(ssum) > sum_threshold
        filt = np.where(ok, (w*tap).sum(0)/np.where(ok, ssum, 1.0), tap[list(taps).index(0)])
        if dering:
            filt = np.clip(filt, tap.min(0), tap.max(0))
        alpha = feedback*(ssum if (up and confidence_weighting) else np.ones_like(ssum))
        alpha = np.clip(alpha, 0.0, 1.0)
        hist = hist*(1.0 - alpha) + filt*alpha
        if boxclamp:
            hist = np.clip(hist, tap.min(0), tap.max(0))
    return hist

def mtf(nu, scale, N_out=256, phases=8, **kw):
    """Response at frequency nu (cycles per output pixel), averaged over signal phases."""
    xi = np.arange(N_out) + 0.5
    amps = []
    for p in range(phases):
        ph = 2*np.pi*p/phases
        f = lambda x, ph=ph: np.cos(2*np.pi*nu*x + ph)
        out = accumulate(f, N_out, scale, **kw)
        c = 2*np.mean(out*np.cos(2*np.pi*nu*xi + ph))
        s = 2*np.mean(out*np.sin(2*np.pi*nu*xi + ph))
        amps.append(np.hypot(c, s))
    return float(np.mean(amps))

if __name__ == "__main__":
    freqs = [0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45]
    print("Effective MTF (response to a cosine of amplitude 1), no clamping, feedback=0.12")
    print("ideal = the Lanczos-2 kernel's own response, radius 2 OUTPUT pixels\n")
    print(f"{'freq':>6s} {'ideal L2':>9s} {'1:1':>9s} {'2:1':>9s} {'2:1 / 1:1':>10s}")
    xs = np.linspace(-2, 2, 4001)
    for nu in freqs:
        k = lanczos2_true(xs); k = k/np.trapezoid(k, xs)
        ideal = abs(np.trapezoid(k*np.cos(2*np.pi*nu*xs), xs))
        a1 = mtf(nu, 1); a2 = mtf(nu, 2)
        print(f"{nu:6.2f} {ideal:9.4f} {a1:9.4f} {a2:9.4f} {a2/max(a1,1e-9):10.3f}")
