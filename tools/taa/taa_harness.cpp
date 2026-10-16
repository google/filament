/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * A headless, deterministic measurement harness for temporal anti-aliasing.
 *
 * TAA artifacts are small, spatially sparse, phase-dependent and regime-dependent, which makes
 * them exactly the class of defect that eyeballing a running viewer is worst at. This tool turns
 * them into numbers instead: it renders a fixed, reproducible frame schedule and reports masked
 * per-regime metrics as JSON, so a change can be judged by comparing two runs.
 *
 * How a run works
 * ---------------
 * A run is one (regime, upscaling) pair. Every run has the same shape:
 *
 *   1. References. Animation and camera are placed at a given state, TAA is DISABLED, and N
 *      frames are rendered while the camera projection is jittered by hand with the same
 *      sequence PostProcessManager uses. Averaging them is the converged supersampled image
 *      for that instant -- i.e. exactly what TAA is trying to converge to.
 *   2. Warm-up. TAA is enabled and W frames are rendered with the scene held at its pre-motion
 *      state, so history is converged before anything moves.
 *   3. Capture. Motion begins and K frames are rendered and read back, one per reference. Motion
 *      runs for the first half of the window and then STOPS, so the second half is history
 *      recovering from it rather than more of the same.
 *
 * Because motion starts precisely at capture frame 0, the error curve over the captured frames
 * IS the ghosting curve: how wrong history is the instant the scene changes, and how fast that
 * decays. With no motion (the "static" regime) the same captures measure flicker instead, which
 * is why K defaults to 16, a full HALTON_23_X16 jitter cycle -- a shorter window measures jitter
 * phase, not stability.
 *
 * How the camera regime moves
 * ----------------------------
 * --camera-motion picks between two very different tests, and the untested default this used to
 * ship with (a fixed 20deg/s orbit) barely moved anything -- a few texels of apparent motion
 * anywhere on screen over the whole capture window.
 *
 *   - rotate: the original orbit, now --camera-speed degrees/second and defaulting higher.
 *     Mixes rotation with a little parallax, and screen-space speed is not uniform -- near zero
 *     at the target, fastest at the silhouette.
 *   - translate: a pure dolly. Eye and look-at point translate together, so orientation never
 *     changes and every pixel's motion is parallax with a known, uniform magnitude at the
 *     target's depth: --camera-texels-per-frame render-resolution texels of it, every frame.
 *     --camera-translation-align then isolates history-buffer RESAMPLING specifically: `pixel`
 *     makes every frame's reprojection an exact texel fetch (the best case), `subpixel` makes
 *     every one a half-texel fetch (the worst case), and comparing the two at the same speed
 *     attributes whatever changes to that resampling and nothing else translate mode does.
 *
 * --object-speed is the same idea for the object regime: a multiplier on animation playback time,
 * for when the authored animation itself moves too little to measure.
 *
 * Determinism
 * -----------
 * Nothing here reads the wall clock. Animation time, camera pose and jitter are all functions of
 * the frame index, and exactly one scheduled frame is rendered per iteration of the app loop, so
 * frame N of a run is reproducible. Metrics still travel better than golden images across
 * backends, which is why the output is numbers with tolerances rather than reference PNGs;
 * the backend that produced a baseline is recorded in the JSON.
 *
 * How good is the reference itself
 * --------------------------------
 * The accumulated reference is the harness's most load-bearing part and the easiest thing to get
 * quietly wrong, so two numbers police it. reference_alignment_x/y/gain reports whether it is
 * converged on the pixel centre, where the gain -- how much error aligning actually removes --
 * is what separates a real offset from a flat minimum. And --msaa renders a second reference in
 * hardware, sharing none of the jitter arithmetic; msaa_vs_reference_* is the two of them
 * checked against each other.
 *
 * The reference's own convergence is not left to guesswork either: measured against a
 * 256-sample accumulation, the default 32 box samples land within 0.0007 RMSE, against TAA
 * errors in the 0.02-0.08 range. Sampling noise is not the dominant term here, though the margin
 * is narrower than it looks at first: View::AntiAliasing defaults to FXAA and, left on, quietly
 * smooths every rendered frame this harness produces -- captures, references and the aliased
 * comparison alike -- which is why it is force-disabled in setup() alongside dithering. Turning
 * it off is also what made the numbers above bigger than an earlier pass reported; a softened
 * reference converges faster and disagrees with a softened capture less, so FXAA was making the
 * measurement look better than the thing it was supposed to be measuring.
 *
 * What the reference integrates against is a choice, and --reference-filter makes it one. A box
 * is what an unweighted accumulation and an MSAA resolve both give; Filament aims at Lanczos-2
 * instead, so `lanczos` integrates against a true one and `lanczos-filament` against the
 * polynomial taa.mat actually evaluates. Running the last two and differencing them separates
 * "did TAA converge to its own polynomial" from "how close is that polynomial (plus the 3x3
 * deringing clamp) to an idealized Lanczos-2".
 *
 * That second reference also sets the noise floor. MSAA antialiases coverage but not shading, so
 * it and the accumulated reference are two legitimate but not identical definitions of ground
 * truth, and msaa_vs_reference_edge_rmse is how far apart they are. A TAA error smaller than
 * that is below the resolution of the whole measurement and should not be read as a finding.
 *
 * Looking at it
 * -------------
 * --dump writes one directory per run containing, for every captured frame, what TAA produced,
 * the converged reference it was scored against, and their amplified difference, plus the mask
 * the run measured over. The difference images are the point: an artifact worth chasing is a few
 * hundredths of a unit over a few hundred texels, which is invisible in the captures themselves
 * and invisible in an unamplified diff.
 *
 * Example:
 *     taa_harness --headless -o metrics.json third_party/models/Fox/Fox.glb
 *     taa_harness --headless -r camera -u 2 -d frames/ third_party/models/Fox/Fox.glb
 */

#include "common/arguments.h"

#include "materials/uberarchive.h"

#include <viewer/Settings.h>

#include <imageio/ImageEncoder.h>

#include <image/LinearImage.h>

#include <gltfio/Animator.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/FilamentInstance.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>
#include <filamentapp/IBL.h>

#include <filament/Box.h>
#include <filament/Camera.h>
#include <filament/DebugRegistry.h>
#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/Options.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <backend/PixelBufferDescriptor.h>

#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>
#include <utils/Path.h>

#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <samples/SampleConfig.h>

#include <algorithm>
#include <atomic>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// isatty()/fileno() portability: kept out of the sorted include block above so the header
// reorganizer does not skip the whole file over one platform conditional.
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

using namespace filament;
using namespace filament::math;
using namespace filament::gltfio;
using namespace image;
using namespace utils;

namespace {

// ------------------------------------------------------------------------------------------------
// Jitter
// ------------------------------------------------------------------------------------------------

// Deliberately a copy of PostProcessManager's halton(), including the +409 skip: the reference is
// only the image TAA converges to if it is accumulated over the same sample distribution.
float haltonSample(unsigned int i, unsigned int const b) {
    i += 409;
    float f = 1.0f;
    float r = 0.0f;
    while (i > 0u) {
        f /= float(b);
        r += f * float(i % b);
        i /= b;
    }
    return r;
}

constexpr double kBaseAzimuthRadians = 60.0 * F_PI / 180.0;

// Which kernel the reference integrates the scene against -- that is, what "correctly filtered"
// is taken to MEAN here. It is a real choice, not a detail.
//
// BOX weights every sample in the pixel equally, which is what MSAA's resolve does and what an
// unweighted accumulation gives you for free. But it is not what Filament aims at: taa.mat
// reconstructs with a Lanczos-2 kernel, so a box reference scores a correct TAA as permanently
// over-sharp and would score a regression towards box blur as an improvement.
//
// LANCZOS integrates against a true Lanczos-2 -- sinc(x)sinc(x/2), separable, radius 2 -- so the
// cost of taa.mat's polynomial approximation stays inside what gets measured. LANCZOS_FILAMENT
// uses that polynomial instead, which cancels it out. The pair is the point: run both and the
// difference between them IS the approximation's contribution, separated from everything else.
// Closer than the previous polynomial, but still not exact -- at x = 1.5 it gives -0.048 against
// a true -0.064, versus -0.105 before, and past x = 1.8 or so it is the wrong sign by a few
// hundredths (see the fit rationale on taa.mat's lanczos2_SquaredInputApproximate).
enum class ReferenceFilter : uint8_t { BOX, LANCZOS, LANCZOS_FILAMENT };

// Radius in texels. The box is a pixel-area average; Lanczos-2 reaches two texels, so its samples
// have to be spread over that whole support -- the jitter range TAA itself uses covers only the
// central half-texel and cannot represent the kernel's shoulders or its negative lobes at all.
float filterRadius(ReferenceFilter const filter) {
    return filter == ReferenceFilter::BOX ? 0.5f : 2.0f;
}

float sinc(float const x) {
    if (x == 0.0f) {
        return 1.0f;
    }
    float const px = float(F_PI) * x;
    return std::sin(px) / px;
}

float lanczos2(float const x) {
    return std::abs(x) >= 2.0f ? 0.0f : sinc(x) * sinc(x * 0.5f);
}

// The approximation taa.mat actually evaluates, transcribed. Normalised to 1 at the origin like
// the true kernel, and agreeing with it at 0, 1 and 2, but not in between.
float lanczos2Filament(float const x) {
    if (std::abs(x) >= 2.0f) {
        return 0.0f;
    }
    float const x2 = x * x;
    float const wA = x2 - 4.0f;
    float const wA2 = wA * wA;
    float const wC = x2 - 1.0f;
    float const wD = -0.354377f * x2 + 1.0f;
    float const wE = wC * wD;
    return wA2 * wE * (-1.0f / 16.0f);
}

// Separable, matching how taa.mat applies its kernel.
float filterWeight(float2 const offset, ReferenceFilter const filter) {
    switch (filter) {
        case ReferenceFilter::BOX:
            return 1.0f;
        case ReferenceFilter::LANCZOS:
            return lanczos2(offset.x) * lanczos2(offset.y);
        case ReferenceFilter::LANCZOS_FILAMENT:
            return lanczos2Filament(offset.x) * lanczos2Filament(offset.y);
    }
    return 1.0f;
}

// Sample offset within a texel, CENTRED on the texel. The raw sequence runs over [0,1) -- the
// +409 skip puts its mean at 0.5, not at 0 -- so an unweighted average of frames jittered by the
// raw values has its centroid half a texel off the pixel centre, while TAA resolves to the pixel
// centre. Subtracting 0.5 is what makes the two comparable; without it the reference is a
// correctly supersampled image of the wrong point, and every metric inherits the mismatch as a
// floor. This is measured, not assumed: reference_alignment_x/y report the residual offset, and
// it is 0.00 with the 0.5 here and 0.50 without it.
float2 haltonJitter(size_t const index, ReferenceFilter const filter) {
    float const scale = 2.0f * filterRadius(filter);
    return { (haltonSample(unsigned(index), 2) - 0.5f) * scale,
             (haltonSample(unsigned(index), 3) - 0.5f) * scale };
}

// ------------------------------------------------------------------------------------------------
// Frame schedule
// ------------------------------------------------------------------------------------------------

enum class Regime : uint8_t {
    // Camera and scene both still. Measures flicker and steady-state sharpness: with nothing
    // moving, every frame should be the same frame.
    STATIC,
    // Camera moves (orbit or a pure dolly translation, see CameraMotion), scene still. Expected
    // to be ghost-free, and disocclusions (which here are pure parallax) are expected to resolve
    // quickly.
    CAMERA,
    // Scene animates, camera still. Filament has no per-object motion vectors, so interior
    // ghosting is EXPECTED here and is reported, not failed; what matters is that the
    // dis/occluded edges recover.
    OBJECT,
};

const char* toString(Regime const regime) {
    switch (regime) {
        case Regime::STATIC: return "static";
        case Regime::CAMERA: return "camera";
        case Regime::OBJECT: return "object";
    }
    return "?";
}

bool parseJitterPattern(std::string const& name,
        TemporalAntiAliasingOptions::JitterPattern* out) {
    using JitterPattern = TemporalAntiAliasingOptions::JitterPattern;
    if (name == "rgss4") { *out = JitterPattern::RGSS_X4; return true; }
    if (name == "helix4") { *out = JitterPattern::UNIFORM_HELIX_X4; return true; }
    if (name == "halton8") { *out = JitterPattern::HALTON_23_X8; return true; }
    if (name == "halton16") { *out = JitterPattern::HALTON_23_X16; return true; }
    if (name == "halton32") { *out = JitterPattern::HALTON_23_X32; return true; }
    return false;
}

bool parseRegime(std::string const& name, Regime* out) {
    if (name == "static") { *out = Regime::STATIC; return true; }
    if (name == "camera") { *out = Regime::CAMERA; return true; }
    if (name == "object") { *out = Regime::OBJECT; return true; }
    return false;
}

// Motion time for capture frame `k`: it advances for the first half of the capture window and
// then holds. Continuous motion only ever answers "how wrong is history while things move"; the
// interesting question -- how fast a disocclusion resolves -- needs the motion to STOP, so that
// what follows is a transient decaying towards a floor that can be measured rather than a
// sustained offset that cannot.
double motionTimeForFrame(size_t const k, size_t const captureFrames, double const step) {
    size_t const motionFrames = std::max<size_t>(1, captureFrames / 2);
    return double(std::min(k, motionFrames)) * step;
}

// Same clamp as motionTimeForFrame, as an integer frame count rather than a re-derived seconds
// value. TRANSLATE motion needs this directly: multiplying a per-frame texel step by an index
// that came from dividing motionTime back by stepSeconds would work, but would make an exact,
// reproducible texel position depend on floating-point round-tripping for no reason.
size_t motionFrameIndex(size_t const k, size_t const captureFrames) {
    size_t const motionFrames = std::max<size_t>(1, captureFrames / 2);
    return std::min(k, motionFrames);
}

enum class Destination : uint8_t {
    NONE,
    // A jitter sample accumulating into one of the converged references.
    REFERENCE,
    // A TAA frame being measured.
    CAPTURE,
    // The hardware-MSAA cross-check reference.
    MSAA,
    // A plain render with no antialiasing of any kind at output resolution: TAA off, no jitter,
    // no MSAA. Not ground truth -- it is the fully aliased image -- but it is the exact thing a
    // 2:1 upscale should reproduce under UNIFORM_HELIX_X4, see aliased_edge_rmse.
    ALIASED,
};

// One scheduled frame. The whole run is a flat list of these, and exactly one is consumed per
// iteration of the app loop, which is what makes the schedule -- and therefore TAA's history --
// reproducible.
struct Shot {
    // Seconds since motion onset. Zero throughout the reference and warm-up phases, so the
    // captured frames start from a converged, pre-motion history.
    double motionTime = 0.0;
    // Same clock, as an integer frame count -- see motionFrameIndex(). Only TRANSLATE camera
    // motion uses this; motionTime is enough for everything else.
    size_t motionFrameIndex = 0;
    bool taaEnabled = true;
    // Manual sub-texel offset, in render-target texels, applied to the camera projection. Only
    // used while TAA is off, i.e. when accumulating a reference; TAA applies its own otherwise.
    float2 jitter = {};
    int runIndex = -1;
    // Where this frame's read-back goes, if anywhere. Warm-up frames go nowhere.
    Destination destination = Destination::NONE;
    int slot = -1;
    // This sample's filter weight. Can be negative -- Lanczos-2 has negative lobes, and dropping
    // them would quietly turn the kernel back into something blurrier than the one being aimed at.
    float weight = 1.0f;
    // Hardware sample count for this frame. Non-zero only for the MSAA reference: MSAA on any
    // other frame would change what TAA is being fed and therefore what is being measured.
    uint8_t msaaSamples = 0;
};

struct Run {
    Regime regime = Regime::STATIC;
    float upscaling = 1.0f;
    // Accumulators for the reference phase, in linear light, summed over the jitter samples and
    // divided at the end. STATIC has a single reference shared by every captured frame, since
    // the scene state does not change.
    std::vector<LinearImage> references;
    // Summed filter weight per reference, the divisor for the accumulation. A weight sum rather
    // than a sample count because the samples are not equally weighted once the filter is not a
    // box, and because a self-normalising ratio estimator does not care that the kernel's own
    // integral is not exactly one.
    std::vector<double> referenceWeight;
    std::vector<LinearImage> captures;
    // An independent reference rendered by hardware MSAA, when --msaa asks for one. Not a
    // replacement for the accumulated reference -- MSAA supersamples coverage but not shading, so
    // it says nothing about texture or specular aliasing, caps out at a handful of samples, and
    // its sample positions are the driver's rather than ours. What it is good for is checking the
    // accumulated reference, which is the harness's most load-bearing and most error-prone part:
    // it is built from our own jitter arithmetic, and a reference that is wrong in a smooth way
    // looks entirely plausible while putting a floor under every metric. This one shares none of
    // that arithmetic, so where the two disagree at geometric edges, one of them is wrong.
    LinearImage msaaReference;
    LinearImage aliasedReference;
};

// ------------------------------------------------------------------------------------------------
// Image helpers
// ------------------------------------------------------------------------------------------------

float srgbToLinear(float const c) {
    return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
}

// readPixels hands back bottom-up sRGB-encoded RGBA bytes -- RGBA and not RGB because Metal
// rejects the 3-channel combination. Flip to top-down, drop alpha and decode the display
// encoding, so metrics are computed on linear values. Note this undoes the sRGB transfer function
// but NOT the tone mapper: the comparison is between two tone-mapped images, which is the space
// the artifacts are actually seen in.
LinearImage decode(uint8_t const* rgba, uint32_t const w, uint32_t const h) {
    LinearImage out(w, h, 3);
    float* dst = out.getPixelRef();
    for (uint32_t y = 0; y < h; y++) {
        uint8_t const* src = rgba + size_t(h - 1 - y) * w * 4;
        for (uint32_t x = 0; x < w; x++) {
            for (uint32_t c = 0; c < 3; c++) {
                dst[(size_t(y) * w + x) * 3 + c] =
                        srgbToLinear(float(src[x * 4 + c]) * (1.0f / 255.0f));
            }
        }
    }
    return out;
}

float luma(float const* p) {
    return 0.2126f * p[0] + 0.7152f * p[1] + 0.0722f * p[2];
}

// A silhouette mask, derived from the converged reference rather than from any TAA output so
// that it does not move when the thing being measured changes. A depth gradient would be a
// truer silhouette signal, but it is not reachable from a read-back of the final swap chain;
// the luminance gradient of a converged image is a serviceable stand-in, and it is at least
// stable across runs.
std::vector<uint8_t> edgeMask(LinearImage const& reference, float const threshold) {
    uint32_t const w = reference.getWidth();
    uint32_t const h = reference.getHeight();
    std::vector<uint8_t> raw(size_t(w) * h, 0);
    for (uint32_t y = 1; y + 1 < h; y++) {
        for (uint32_t x = 1; x + 1 < w; x++) {
            float const l  = luma(reference.getPixelRef(x, y));
            float const lx = luma(reference.getPixelRef(x + 1, y));
            float const ly = luma(reference.getPixelRef(x, y + 1));
            float const gradient = std::abs(lx - l) + std::abs(ly - l);
            raw[size_t(y) * w + x] = gradient > threshold ? 1 : 0;
        }
    }
    // Dilate by one texel: TAA's failures sit ON the edge and in the texel either side of it,
    // and a one-texel-wide mask would miss the stale-history band that hugs a moving edge.
    std::vector<uint8_t> mask(size_t(w) * h, 0);
    for (uint32_t y = 1; y + 1 < h; y++) {
        for (uint32_t x = 1; x + 1 < w; x++) {
            uint8_t v = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    v |= raw[size_t(y + dy) * w + (x + dx)];
                }
            }
            mask[size_t(y) * w + x] = v;
        }
    }
    return mask;
}

// Root-mean-square error over the selected texels, in linear light. `mask` may be null, in which
// case every texel counts.
double rmse(LinearImage const& a, LinearImage const& b, std::vector<uint8_t> const* mask) {
    uint32_t const w = a.getWidth();
    uint32_t const h = a.getHeight();
    double sum = 0.0;
    size_t count = 0;
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            if (mask && !(*mask)[size_t(y) * w + x]) {
                continue;
            }
            float const* pa = a.getPixelRef(x, y);
            float const* pb = b.getPixelRef(x, y);
            for (int c = 0; c < 3; c++) {
                double const d = double(pa[c]) - double(pb[c]);
                sum += d * d;
            }
            count += 3;
        }
    }
    return count ? std::sqrt(sum / double(count)) : 0.0;
}

// RMSE between `a` and `b` with `b` bilinearly resampled at a sub-texel offset. Used only to find
// the offset that best aligns the two, below.
double shiftedRmse(LinearImage const& a, LinearImage const& b, std::vector<uint8_t> const& mask,
        double const dx, double const dy) {
    uint32_t const w = a.getWidth();
    uint32_t const h = a.getHeight();
    double sum = 0.0;
    size_t count = 0;
    for (uint32_t y = 1; y + 2 < h; y++) {
        for (uint32_t x = 1; x + 2 < w; x++) {
            if (!mask[size_t(y) * w + x]) {
                continue;
            }
            double const sx = double(x) + dx;
            double const sy = double(y) + dy;
            auto const x0 = uint32_t(std::floor(sx));
            auto const y0 = uint32_t(std::floor(sy));
            if (x0 < 1 || y0 < 1 || x0 + 2 >= w || y0 + 2 >= h) {
                continue;
            }
            double const fx = sx - double(x0);
            double const fy = sy - double(y0);
            float const* pa = a.getPixelRef(x, y);
            float const* p00 = b.getPixelRef(x0, y0);
            float const* p10 = b.getPixelRef(x0 + 1, y0);
            float const* p01 = b.getPixelRef(x0, y0 + 1);
            float const* p11 = b.getPixelRef(x0 + 1, y0 + 1);
            for (int c = 0; c < 3; c++) {
                double const top = double(p00[c]) * (1.0 - fx) + double(p10[c]) * fx;
                double const bottom = double(p01[c]) * (1.0 - fx) + double(p11[c]) * fx;
                double const d = double(pa[c]) - (top * (1.0 - fy) + bottom * fy);
                sum += d * d;
            }
            count += 3;
        }
    }
    return count ? std::sqrt(sum / double(count)) : 0.0;
}

// The sub-texel offset at which the reference best matches the capture -- a self-check on the
// harness rather than a measurement of TAA.
//
// TAA resolves to pixel centres, so a correctly built reference lines up with its output at
// exactly (0, 0) and any other answer means the reference is a well-converged image of the wrong
// point. That is a failure mode with no visible symptom: the reference still looks like a
// plausible render, and the mismatch surfaces only as a large, mysterious error floor spread
// along every edge -- which is precisely how it presented before it was found. Reported on every
// run so it cannot come back quietly.
// Returned as (x, y, gain), where gain is the fraction of the error that aligning removes.
//
// The gain is not decoration. This minimum is very flat -- a correctly built reference differs
// from the TAA output by filter shape, which no shift improves -- so the position of the minimum
// alone is mostly the search grid talking. A coarse grid duly reported a confident quarter-texel
// offset under upscaling that turned out to be its own quantisation. Read the offset only when
// the gain says there is something to find; the real misalignment that prompted all this scored
// 76%.
math::double3 referenceAlignment(LinearImage const& capture, LinearImage const& reference,
        std::vector<uint8_t> const& mask) {
    // Coarse pass wide enough to catch a whole-texel error, then a fine pass around it, so the
    // reported offset is precise without scanning a large grid at fine resolution.
    math::double2 offset = {};
    double best = std::numeric_limits<double>::max();
    for (double step: { 0.25, 0.05 }) {
        math::double2 const centre = offset;
        for (int iy = -6; iy <= 6; iy++) {
            for (int ix = -6; ix <= 6; ix++) {
                math::double2 const candidate = { centre.x + step * ix, centre.y + step * iy };
                double const e = shiftedRmse(capture, reference, mask, candidate.x, candidate.y);
                if (e < best) {
                    best = e;
                    offset = candidate;
                }
            }
        }
    }
    double const unaligned = shiftedRmse(capture, reference, mask, 0.0, 0.0);
    double const gain = unaligned > 0.0 ? 1.0 - best / unaligned : 0.0;
    return { offset.x, offset.y, gain };
}

// Mean per-texel temporal variance across a sequence. With a static camera and a static scene
// this is flicker, and it is only meaningful over a whole jitter cycle -- over a handful of
// frames it measures which phase of the jitter the window happened to catch.
double temporalVariance(std::vector<LinearImage> const& frames, std::vector<uint8_t> const* mask) {
    if (frames.size() < 2) {
        return 0.0;
    }
    uint32_t const w = frames[0].getWidth();
    uint32_t const h = frames[0].getHeight();
    double total = 0.0;
    size_t count = 0;
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            if (mask && !(*mask)[size_t(y) * w + x]) {
                continue;
            }
            for (int c = 0; c < 3; c++) {
                double mean = 0.0;
                for (auto const& f: frames) {
                    mean += double(f.getPixelRef(x, y)[c]);
                }
                mean /= double(frames.size());
                double variance = 0.0;
                for (auto const& f: frames) {
                    double const d = double(f.getPixelRef(x, y)[c]) - mean;
                    variance += d * d;
                }
                total += variance / double(frames.size());
                count++;
            }
        }
    }
    return count ? total / double(count) : 0.0;
}

// Mean squared Laplacian of luminance: a scalar stand-in for high-frequency energy, reported as a
// ratio against the reference.
//
// 1.0 means "carries the same high-frequency energy as a scene filtered by --reference-filter",
// so what it is measuring depends on which kernel that is, and the pair of Lanczos references
// splits the question in two. Against Filament's own polynomial (`lanczos-filament`), a static 1:1
// run sits at ~1.07; against a true Lanczos-2 (`lanczos`) it reads ~0.95, and against a box
// reference (`box`) ~0.87 (see taa_harness.md and lanczos2_SquaredInputApproximate in taa.mat).
//
// The per-frame series is the other half of the value: whether sharpness PULSES across the jitter
// cycle on a static scene is a yes/no question it answers directly.
//
// `mask` restricts the sum to the silhouette (see edge_sharpness_mean below); nullptr sums the
// whole frame like sharpness_mean does.
double highFrequencyEnergy(LinearImage const& img, std::vector<uint8_t> const* mask = nullptr) {
    uint32_t const w = img.getWidth();
    uint32_t const h = img.getHeight();
    double sum = 0.0;
    size_t count = 0;
    for (uint32_t y = 1; y + 1 < h; y++) {
        for (uint32_t x = 1; x + 1 < w; x++) {
            if (mask && !(*mask)[size_t(y) * w + x]) {
                continue;
            }
            double const l = 4.0 * luma(img.getPixelRef(x, y))
                    - luma(img.getPixelRef(x - 1, y)) - luma(img.getPixelRef(x + 1, y))
                    - luma(img.getPixelRef(x, y - 1)) - luma(img.getPixelRef(x, y + 1));
            sum += l * l;
            count++;
        }
    }
    return count ? sum / double(count) : 0.0;
}

// World-space bounds of everything the asset will draw.
Aabb worldBounds(Engine* engine, FilamentAsset const* asset) {
    auto const& rcm = engine->getRenderableManager();
    auto const& tcm = engine->getTransformManager();
    Aabb bounds = { float3{ FLT_MAX }, float3{ -FLT_MAX } };
    Entity const* entities = asset->getRenderableEntities();
    for (size_t i = 0, c = asset->getRenderableEntityCount(); i < c; i++) {
        auto const ri = rcm.getInstance(entities[i]);
        auto const ti = tcm.getInstance(entities[i]);
        if (!ri || !ti) {
            continue;
        }
        Box const box = rcm.getAxisAlignedBoundingBox(ri);
        mat4f const transform = tcm.getWorldTransform(ti);
        float3 const min = box.center - box.halfExtent;
        float3 const max = box.center + box.halfExtent;
        for (int corner = 0; corner < 8; corner++) {
            float3 const p = (transform * float4{
                    (corner & 1) ? max.x : min.x,
                    (corner & 2) ? max.y : min.y,
                    (corner & 4) ? max.z : min.z, 1.0f }).xyz;
            for (int c = 0; c < 3; c++) {
                bounds.min[c] = std::min(bounds.min[c], p[c]);
                bounds.max[c] = std::max(bounds.max[c], p[c]);
            }
        }
    }
    if (bounds.min.x > bounds.max.x) {
        bounds = { float3{ -1.0f }, float3{ 1.0f } };
    }
    return bounds;
}

// The mask as a viewable image, so "which texels did this run actually measure" is a thing that
// can be looked at rather than inferred from the edge_fraction number.
LinearImage maskImage(std::vector<uint8_t> const& mask, uint32_t const w, uint32_t const h) {
    LinearImage out(w, h, 3);
    float* dst = out.getPixelRef();
    for (size_t i = 0; i < size_t(w) * h; i++) {
        float const v = mask[i] ? 1.0f : 0.0f;
        dst[i * 3 + 0] = dst[i * 3 + 1] = dst[i * 3 + 2] = v;
    }
    return out;
}

// Per-channel absolute difference, amplified. Amplification is the whole point: the artifacts
// worth finding are a few hundredths of a unit over a few hundred texels, and an unscaled diff of
// one is indistinguishable from black. Kept per-channel rather than reduced to luminance so that
// a chroma-only error still shows up, and clamped so the gain cannot wrap a large error around
// into looking small.
LinearImage differenceImage(LinearImage const& a, LinearImage const& b, float const gain) {
    uint32_t const w = a.getWidth();
    uint32_t const h = a.getHeight();
    LinearImage out(w, h, 3);
    float* dst = out.getPixelRef();
    float const* pa = a.getPixelRef();
    float const* pb = b.getPixelRef();
    for (size_t i = 0; i < size_t(w) * h * 3; i++) {
        dst[i] = std::min(1.0f, std::abs(pa[i] - pb[i]) * gain);
    }
    return out;
}

void writePng(LinearImage const& img, Path const& path) {
    std::ofstream out(path.c_str(), std::ios::binary | std::ios::trunc);
    ImageEncoder::encode(out, ImageEncoder::Format::PNG, img, "", path.c_str());
}

// ------------------------------------------------------------------------------------------------
// Application state
// ------------------------------------------------------------------------------------------------

// How the camera moves during the CAMERA regime.
enum class CameraMotion : uint8_t {
    // Orbit around the target at a fixed distance, always re-aimed at it. Mixes rotation with a
    // small amount of parallax (the eye itself moves along an arc), and the apparent screen-space
    // speed varies across the frame -- near zero at the target, fastest at the silhouette.
    ROTATE,
    // A pure dolly: eye translates, orientation held fixed (the look-at point translates by the
    // same vector, so the view direction never changes). No rotation at all, so every pixel's
    // motion is pure parallax, and -- unlike ROTATE -- its magnitude is exactly known and
    // uniform for scene content at the target's depth, which is what makes camera-texels-per-
    // frame and camera-translation-align meaningful only in this mode.
    TRANSLATE,
};

// Whether a TRANSLATE step lands exactly on a render-resolution texel or deliberately does not.
// This is the one lever that isolates history-buffer RESAMPLING from everything else TAA does:
// reprojecting last frame's history samples a non-integer number of texels away forces a
// bilinear fetch of the history texture on every single frame, where an exact integer step
// fetches the same texel it already held. Two runs at the same speed, differing only in this,
// attribute whatever changes to that resampling specifically.
enum class TranslationAlign : uint8_t {
    // Whatever fractional part --camera-texels-per-frame happens to have.
    AS_REQUESTED,
    // Rounded to the nearest integer texel count: every frame's reprojection is an exact texel
    // fetch, the best case for history sampling.
    PIXEL,
    // Forced to exactly a half-texel fractional part: every frame's reprojection falls precisely
    // between four texels, the worst case for history sampling.
    SUBPIXEL,
};

struct Options {
    Path model;
    Path output = "taa_metrics.json";
    Path dumpDirectory;
    uint32_t size = 512;
    // Measured on a static scene: TAA sharpness is 0.96x converged at 8 frames, 0.87x at 16,
    // 0.985x at 32, 0.996x at 40, and indistinguishable from a 192-frame warmup by 48 (feedback's
    // exponential decay, unsurprisingly). 24 -- roughly 1.5 jitter cycles -- looked reasonable
    // without being checked and was quietly measuring TAA 1.4% short of its own steady state.
    uint32_t warmupFrames = 64;
    uint32_t captureFrames = 16;
    uint32_t referenceSamples = 32;
    double stepSeconds = 1.0 / 60.0;
    CameraMotion cameraMotion = CameraMotion::ROTATE;
    // Untested guess when this was 20: over the default 8-frame motion burst that is a 2.7 degree
    // sweep, barely a few texels of apparent motion anywhere on screen. Raised well past that; see
    // --camera-speed. TRANSLATE mode does not use this at all -- see cameraTexelsPerFrame.
    double cameraRotateSpeedDegPerSec = 90.0;
    // TRANSLATE mode's speed, in render-resolution texels/frame of parallax at the TARGET's
    // depth -- not output-resolution texels, and not uniform across depths (that non-uniformity,
    // parallax, is the point of a translation test). Deliberately not integral by default, so
    // AS_REQUESTED alignment does not silently coincide with PIXEL.
    double cameraTexelsPerFrame = 3.5;
    TranslationAlign cameraTranslationAlign = TranslationAlign::AS_REQUESTED;
    // Multiplies elapsed animation time in the OBJECT regime. The animation itself sets the
    // natural time scale, so unlike the camera there is no principled default speed -- 1.0 is
    // "play it as authored" and this exists for when that turns out to move too little to
    // measure, the same complaint camera rotation had at its old default.
    double objectSpeed = 1.0;
    float edgeThreshold = 0.08f;
    float diffGain = 8.0f;
    uint8_t msaaSamples = 0;
    TemporalAntiAliasingOptions::JitterPattern jitterPattern =
            TemporalAntiAliasingOptions::JitterPattern::HALTON_23_X16;
    TemporalAntiAliasingOptions taaOptions = {};
    ReferenceFilter referenceFilter = ReferenceFilter::LANCZOS;
    // Which glTF animation the object regime plays. It matters: a survey/idle clip barely moves,
    // and a regime that does not move measures nothing.
    uint32_t animation = 0;
    // A live-updating "[====    ] 1234/5000  camera x2" line on stderr. Defaults to whatever
    // stderr looks like it can take -- a real terminal gets one, a redirected/piped stderr does
    // not, so a log file does not fill up with carriage-return noise. --quiet forces it off
    // either way.
    bool showProgress = true;
    // Replaces every rendered frame with taa.mat's disocclusion rejection mask (white = history
    // fully rejected). Pairs with --dump: the metrics computed from a masked frame are
    // meaningless, this exists so the mask can be LOOKED AT -- which is the only way to tell
    // "fires on a few silhouette texels" apart from "fires over the whole image", two failures
    // that read identically in the aggregate numbers.
    bool visualizeDisocclusion = false;
    std::vector<Regime> regimes;
    std::vector<float> upscalings;
};

struct App {
    Options options;

    Engine* engine = nullptr;
    View* view = nullptr;
    Scene* scene = nullptr;
    FilamentApp2* filamentApp = nullptr;

    NameComponentManager* names = nullptr;
    MaterialProvider* materials = nullptr;
    gltfio::AssetLoader* assetLoader = nullptr;
    gltfio::ResourceLoader* resourceLoader = nullptr;
    gltfio::TextureProvider* stbDecoder = nullptr;
    gltfio::TextureProvider* ktxDecoder = nullptr;
    FilamentAsset* asset = nullptr;
    FilamentInstance* instance = nullptr;
    Entity sunlight;

    // Framing, derived once from the asset bounds so the camera path is scale-independent.
    float3 target = {};
    float radius = 1.0f;

    std::vector<Run> runs;
    std::vector<Shot> shots;
    size_t nextShot = 0;

    // FilamentApp2 re-applies its own lens projection and viewport after preRender whenever it
    // decides the window changed, and in headless mode that lands over an unpredictable number of
    // early iterations. Measuring across it silently mixes two framings into one average, so the
    // schedule does not start until the configuration has held still, and aborts if it ever moves
    // again -- a measurement that quietly changed its own setup is worse than no measurement.
    float cameraSignature = 0.0f;
    uint32_t stableFrames = 0;
    static constexpr uint32_t kRequiredStableFrames = 4;
    // Set by preRender when it actually placed a shot, and the only thing postRender is allowed
    // to test. The two callbacks re-deriving "is this frame part of the schedule" independently
    // is how the first reference frame ended up being captured off a frame that was never set up.
    bool shotApplied = false;

    // Readbacks dispatched but not yet delivered, purely diagnostic (see printProgress). Nothing
    // in the schedule gates on this: readPixels() is fire-and-forget, camera/animation/jitter
    // state for frame N+1 does not depend on frame N's readback having landed, and the only place
    // that genuinely needs every callback delivered is the end of the run, which already
    // flushAndWait()s before reading the accumulated results. An earlier version synced every
    // frame to bound this and paid a full CPU/GPU round trip ten thousand times over; a version in
    // between synced only once a cap was reached. Removing the cap entirely made no further
    // difference to wall-clock time (the GPU pipeline is the actual limiter) and peak RSS stayed
    // near 300MB across the heaviest schedule this harness builds, so there is nothing left here
    // to bound on purpose. PixelBufferDescriptor callbacks with no custom CallbackHandler are
    // dispatched on the main thread during Engine::execute() / flushAndWait(), so the callback's
    // writes into run.references / run.captures are single-threaded; pendingReadbacks is kept
    // atomic defensively in case a custom handler is ever used.
    std::atomic<int> pendingReadbacks{ 0 };

    bool finished = false;
};

// ------------------------------------------------------------------------------------------------
// Schedule construction
// ------------------------------------------------------------------------------------------------

void buildSchedule(App* app) {
    Options const& o = app->options;
    for (float const upscaling: o.upscalings) {
        for (Regime const regime: o.regimes) {
            int const runIndex = int(app->runs.size());
            Run run;
            run.regime = regime;
            run.upscaling = upscaling;

            // With nothing moving there is only one scene state, so one reference serves every
            // captured frame. The other regimes need one per frame: the reference has to track
            // the scene, or the "error" being reported is just the motion.
            size_t const referenceCount = (regime == Regime::STATIC) ? 1 : o.captureFrames;
            run.references.resize(referenceCount);
            run.referenceWeight.assign(referenceCount, 0.0);
            run.captures.resize(o.captureFrames);
            app->runs.push_back(std::move(run));

            if (o.msaaSamples > 1) {
                Shot shot;
                shot.taaEnabled = false;
                shot.runIndex = runIndex;
                shot.destination = Destination::MSAA;
                shot.msaaSamples = o.msaaSamples;
                app->shots.push_back(shot);
            }
            {
                Shot shot;
                shot.taaEnabled = false;
                shot.runIndex = runIndex;
                shot.destination = Destination::ALIASED;
                app->shots.push_back(shot);
            }

            for (size_t r = 0; r < referenceCount; r++) {
                double const motionTime =
                        motionTimeForFrame(r, o.captureFrames, o.stepSeconds);
                size_t const frameIndex = motionFrameIndex(r, o.captureFrames);
                for (uint32_t s = 0; s < o.referenceSamples; s++) {
                    Shot shot;
                    shot.motionTime = motionTime;
                    shot.motionFrameIndex = frameIndex;
                    shot.taaEnabled = false;
                    shot.jitter = haltonJitter(s, o.referenceFilter);
                    shot.weight = filterWeight(shot.jitter, o.referenceFilter);
                    shot.runIndex = runIndex;
                    shot.destination = Destination::REFERENCE;
                    shot.slot = int(r);
                    app->shots.push_back(shot);
                }
            }

            for (uint32_t w = 0; w < o.warmupFrames; w++) {
                Shot shot;
                shot.runIndex = runIndex;
                app->shots.push_back(shot);
            }

            for (uint32_t k = 0; k < o.captureFrames; k++) {
                Shot shot;
                shot.motionTime = motionTimeForFrame(k, o.captureFrames, o.stepSeconds);
                shot.motionFrameIndex = motionFrameIndex(k, o.captureFrames);
                shot.runIndex = runIndex;
                shot.destination = Destination::CAPTURE;
                shot.slot = int(k);
                app->shots.push_back(shot);
            }
        }
    }
}

// Places camera, animation and TAA settings for one scheduled frame. Runs from preRender, which
// is after FilamentApp2 has pushed the camera manipulator's pose, so these writes win.
void applyShot(App* app, Shot const& shot) {
    Options const& o = app->options;
    Run const& run = app->runs[shot.runIndex];
    Camera& camera = app->view->getCamera();

    // Frame from the camera's ACTUAL field of view rather than one this function installs.
    // FilamentApp2 re-applies its own lens projection and viewport right after preRender whenever
    // it thinks the window changed, so a projection written here survives some frames and not
    // others -- which silently rendered the reference phase and the capture phase of a run at two
    // different zoom levels. Reading the fov instead is immune to when that reconfiguration lands.
    double const fov = double(camera.getFieldOfViewInDegrees(Camera::Fov::VERTICAL)) * F_PI / 180.0;
    double const distance = double(app->radius) / std::sin(0.5 * fov) * 1.15;

    // The motion-free pose: fixed off-axis azimuth (seen end-on, a model presents almost no
    // silhouette, and silhouettes are where everything being measured happens), and the basis
    // TRANSLATE mode dollies along.
    float3 const baseEye = app->target + float3{
            float(std::sin(kBaseAzimuthRadians) * distance), float(app->radius) * 0.35f,
            float(std::cos(kBaseAzimuthRadians) * distance) };
    float3 const up = { 0.0f, 1.0f, 0.0f };
    float3 const right = normalize(cross(normalize(app->target - baseEye), up));

    float3 eye = baseEye;
    float3 lookAtPoint = app->target;
    double animationTime = 0.0;
    switch (run.regime) {
        case Regime::STATIC:
            break;
        case Regime::CAMERA:
            if (o.cameraMotion == CameraMotion::ROTATE) {
                double const angle = kBaseAzimuthRadians
                        + shot.motionTime * o.cameraRotateSpeedDegPerSec * (F_PI / 180.0);
                eye = app->target + float3{ float(std::sin(angle) * distance),
                        float(app->radius) * 0.35f, float(std::cos(angle) * distance) };
            } else {
                // texels/frame -> world units/frame at the target's depth, then a straight
                // multiple of the (integer) motion frame count -- see TranslationAlign and
                // motionFrameIndex() for why an index rather than motionTime / stepSeconds.
                double texelsPerFrame = o.cameraTexelsPerFrame;
                switch (o.cameraTranslationAlign) {
                    case TranslationAlign::AS_REQUESTED:
                        break;
                    case TranslationAlign::PIXEL:
                        texelsPerFrame = std::round(texelsPerFrame);
                        break;
                    case TranslationAlign::SUBPIXEL:
                        texelsPerFrame = std::floor(texelsPerFrame) + 0.5;
                        break;
                }
                Viewport const& vp = app->view->getViewport();
                // Render, not output, resolution: under upscaling TAA resolves at vp/upscaling
                // and this speed is meant to be comparable across upscaling factors, matching the
                // "render texel" convention the rest of the harness (and taa.mat) already uses.
                double const renderHeight = double(vp.height) / double(run.upscaling);
                double const worldUnitsPerTexel =
                        2.0 * distance * std::tan(0.5 * fov) / renderHeight;
                double const offset = texelsPerFrame * double(shot.motionFrameIndex)
                        * worldUnitsPerTexel;
                eye = baseEye + right * float(offset);
                // Translate the look-at point by the same vector: view DIRECTION stays fixed,
                // only position moves, which is what makes this a pure dolly rather than a pan
                // that happens to be phrased as a moving eye.
                lookAtPoint = app->target + right * float(offset);
            }
            break;
        case Regime::OBJECT:
            animationTime = shot.motionTime * o.objectSpeed;
            break;
    }
    camera.lookAt(eye, lookAtPoint, up);

    if (app->instance) {
        Animator& animator = *app->instance->getAnimator();
        if (size_t const count = animator.getAnimationCount(); count > 0) {
            animator.applyAnimation(std::min(size_t(app->options.animation), count - 1),
                    float(animationTime));
            animator.updateBoneMatrices();
        }
    }

    app->view->setMultiSampleAntiAliasingOptions({
            .enabled = shot.msaaSamples > 1, .sampleCount = shot.msaaSamples });

    TemporalAntiAliasingOptions taa = app->options.taaOptions;
    taa.jitterPattern = app->options.jitterPattern;
    taa.enabled = shot.taaEnabled;
    taa.upscaling = run.upscaling;
    app->view->setTemporalAntiAliasingOptions(taa);

    // Camera::setShift takes a translation in NDC, so a jitter of j texels is j/size -- the
    // convention its documentation states. Do not "fix" this to 2/size to match the arithmetic in
    // PostProcessManager: that path is subtracting from a projection matrix column, which is not
    // the same operation, and using its factor here over-jitters the reference by 2x. It is a
    // silent error -- the reference still looks like a plausible converged image, it is just
    // integrated over twice the footprint -- so it shows up as a large edge error floor rather
    // than as anything obviously broken. reference_alignment_x/y is the guard against it.
    //
    // Applied only for reference accumulation: when TAA is on it installs its own jitter, and
    // this must stay at zero or the two would compose.
    if (shot.destination == Destination::REFERENCE) {
        Viewport const& vp = app->view->getViewport();
        camera.setShift(double2{
                double(shot.jitter.x) / double(vp.width),
                double(shot.jitter.y) / double(vp.height) });
    } else {
        camera.setShift({ 0.0, 0.0 });
    }
}

// ------------------------------------------------------------------------------------------------
// Metrics
// ------------------------------------------------------------------------------------------------

struct Record {
    std::string regime;
    float upscaling;
    std::string metric;
    double value;
};

void appendMetrics(App* app, Run& run, std::vector<Record>* records) {
    Options const& o = app->options;
    auto emit = [&](std::string const& name, double const value) {
        records->push_back({ toString(run.regime), run.upscaling, name, value });
    };
    auto referenceFor = [&run](size_t const k) -> LinearImage const& {
        return run.references[run.references.size() == 1 ? 0 : k];
    };

    for (size_t r = 0; r < run.references.size(); r++) {
        double const total = run.referenceWeight[r];
        if (total == 0.0) {
            continue;
        }
        LinearImage& image = run.references[r];
        float* p = image.getPixelRef();
        size_t const count = size_t(image.getWidth()) * image.getHeight() * 3;
        for (size_t i = 0; i < count; i++) {
            p[i] /= float(total);
        }
    }

    // ONE mask for the whole run: the union of the silhouettes across every reference, i.e. every
    // texel the silhouette occupies at any point in the sequence. Two things make this the right
    // choice over a per-frame mask. A frame-0 mask is wrong the moment anything moves -- the
    // object walks out of the measured region and the masked error FALLS, which reads as an
    // improvement. And a mask that changes per frame changes the denominator of the RMSE with it,
    // so the per-frame series stops being comparable frame to frame, which is the one thing the
    // series exists to be. A fixed union covers both where the edge was -- where stale history
    // sits -- and where it went.
    std::vector<uint8_t> mask = edgeMask(referenceFor(0), o.edgeThreshold);
    for (size_t r = 1; r < run.references.size(); r++) {
        std::vector<uint8_t> const current = edgeMask(run.references[r], o.edgeThreshold);
        for (size_t i = 0; i < mask.size(); i++) {
            mask[i] |= current[i];
        }
    }
    size_t const maskedTexels = size_t(std::count(mask.begin(), mask.end(), uint8_t(1)));
    emit("edge_fraction", double(maskedTexels) / double(mask.size()));

    double const referenceEnergy = highFrequencyEnergy(referenceFor(0));

    // Per-frame error from motion onset. Frame 0 is the instant the scene changes, so its value
    // is how wrong the converged history was; the rest of the curve is how fast that is cleaned
    // up. Reported whole rather than reduced to one number, because "ghosting" and "how long
    // ghosting lasts" are different questions and conflating them is what prose reports do.
    double errorSum = 0.0;
    double edgeErrorSum = 0.0;
    double sharpnessSum = 0.0;
    double edgeSharpnessSum = 0.0;
    for (size_t k = 0; k < run.captures.size(); k++) {
        LinearImage const& reference = referenceFor(k);
        double const e = rmse(run.captures[k], reference, nullptr);
        double const edgeError = rmse(run.captures[k], reference, &mask);
        double const sharpness = referenceEnergy > 0.0
                ? highFrequencyEnergy(run.captures[k]) / referenceEnergy : 0.0;
        // Unlike sharpness_mean's fixed frame-0 denominator, this is normalized against THIS
        // frame's own reference, both restricted to the same (fixed, whole-run) mask -- because
        // that mask is a union across the whole sweep (see above), most of it is background or
        // object-interior at any single frame, not a silhouette; frame 0's own edge content
        // there would be a poor stand-in for what the silhouette actually looks like once the
        // sweep reaches it. edge_rmse_frame_k makes the same choice for the same reason.
        double const edgeReferenceEnergy = highFrequencyEnergy(reference, &mask);
        double const edgeSharpness = edgeReferenceEnergy > 0.0
                ? highFrequencyEnergy(run.captures[k], &mask) / edgeReferenceEnergy : 0.0;
        emit("rmse_frame_" + std::to_string(k), e);
        emit("edge_rmse_frame_" + std::to_string(k), edgeError);
        emit("sharpness_frame_" + std::to_string(k), sharpness);
        emit("edge_sharpness_frame_" + std::to_string(k), edgeSharpness);
        errorSum += e;
        edgeErrorSum += edgeError;
        sharpnessSum += sharpness;
        edgeSharpnessSum += edgeSharpness;
    }
    double const n = double(run.captures.size());
    emit("rmse_mean", errorSum / n);
    emit("edge_rmse_mean", edgeErrorSum / n);
    emit("sharpness_mean", sharpnessSum / n);
    emit("edge_sharpness_mean", edgeSharpnessSum / n);

    math::double3 const alignment = referenceAlignment(run.captures.front(), referenceFor(0), mask);
    emit("reference_alignment_x", alignment.x);
    emit("reference_alignment_y", alignment.y);
    emit("reference_alignment_gain", alignment.z);
    // Warn on the gain, not on the offset: a large offset that removes almost no error is the
    // search wandering across a flat minimum, and warning about it trains the reader to ignore
    // the warning.
    if (alignment.z > 0.25) {
        fprintf(stderr, "taa_harness: WARNING -- shifting the reference by (%.2f, %.2f) texels "
                        "removes %.0f%% of its error against the TAA output; it is converged on "
                        "the wrong point and every metric below carries that as a floor\n",
                alignment.x, alignment.y, alignment.z * 100.0);
    }

    // The MSAA cross-check, restricted to silhouettes -- the only place MSAA is a valid
    // reference at all, since it antialiases coverage and not shading. Three numbers, answering
    // different questions: whether our reference and an independently produced one agree at all,
    // whether they are aligned with each other, and -- the useful one -- where the TAA output
    // sits relative to a reference built without any of our jitter arithmetic. If TAA/MSAA and
    // TAA/accumulated disagree about the alignment, the fault is ours.
    if (run.msaaReference.getWidth() != 0) {
        emit("msaa_vs_reference_edge_rmse", rmse(run.msaaReference, referenceFor(0), &mask));
        math::double3 const toReference =
                referenceAlignment(run.msaaReference, referenceFor(0), mask);
        emit("msaa_vs_reference_alignment_x", toReference.x);
        emit("msaa_vs_reference_alignment_y", toReference.y);
        emit("msaa_vs_reference_alignment_gain", toReference.z);
        math::double3 const toCapture =
                referenceAlignment(run.captures.front(), run.msaaReference, mask);
        emit("msaa_alignment_x", toCapture.x);
        emit("msaa_alignment_y", toCapture.y);
        emit("msaa_alignment_gain", toCapture.z);
        emit("msaa_edge_rmse", rmse(run.captures.front(), run.msaaReference, &mask));
    }

    // How far the converged TAA output is from a plain, fully aliased render at output
    // resolution. Normally this is large and means nothing -- an antialiased image SHOULD differ
    // from an aliased one -- but there is one configuration where it is an exact test.
    //
    // UNIFORM_HELIX_X4 jitters by a quarter of a render texel. At 2:1 a render texel spans two
    // output pixels, so a quarter texel is half an output pixel, and the four offsets land
    // exactly on the four output pixel centres inside that render texel. A 2:1 upscale that
    // reconstructs correctly has therefore sampled precisely the points a native output-
    // resolution render samples, and the two should agree. Any gap is the reconstruction, and it
    // is measured against something that shares no code with the accumulated reference.
    if (run.aliasedReference.getWidth() != 0) {
        emit("aliased_edge_rmse", rmse(run.captures.front(), run.aliasedReference, &mask));
        emit("aliased_rmse", rmse(run.captures.front(), run.aliasedReference, nullptr));
    }

    // Flicker. Averaged over the capture window, which is one full jitter cycle by default, so
    // this is temporal instability and not jitter phase.
    emit("flicker_variance", temporalVariance(run.captures, nullptr));
    emit("edge_flicker_variance", temporalVariance(run.captures, &mask));

    // Frame 0 is the converged, pre-motion state, so its edge error is this run's floor -- the
    // part of the error that belongs to the reference definition rather than to any artifact.
    // Everything above it is the artifact, which is what these two report: how far above the
    // floor the worst frame gets, and how many frames after motion STOPS the error takes to come
    // back within 25% of it. -1 means it never did within the capture window, which is the
    // failure this is here to catch.
    size_t const motionFrames = std::max<size_t>(1, run.captures.size() / 2);
    std::vector<double> edgeErrors;
    edgeErrors.reserve(run.captures.size());
    for (size_t k = 0; k < run.captures.size(); k++) {
        edgeErrors.push_back(rmse(run.captures[k], referenceFor(k), &mask));
    }
    double const floor = edgeErrors.front();
    double const peak = *std::max_element(edgeErrors.begin(), edgeErrors.end());
    emit("edge_ghost_peak", peak - floor);

    // Recovery is measured against the TRANSIENT, not against the floor: the floor is large and
    // is dominated by the reference definition, so "within 25% of the floor" is satisfied by
    // frames that are still visibly ghosted. A quarter of the way down from the peak is a
    // threshold that scales with the artifact actually present.
    double recovery = 0.0;
    if (peak - floor > 0.0) {
        double const threshold = floor + 0.25 * (peak - floor);
        recovery = -1.0;
        for (size_t k = motionFrames; k < edgeErrors.size(); k++) {
            if (edgeErrors[k] <= threshold) {
                recovery = double(k - motionFrames);
                break;
            }
        }
    }
    emit("edge_recovery_frames", recovery);

    if (!o.dumpDirectory.isEmpty()) {
        // One directory per run, holding everything needed to see what a number is describing:
        // what TAA produced, what it was compared against, where they differ, and which texels
        // were counted. The diff is the one a human should look at first -- the numbers say a run
        // got worse, the diff says where.
        std::string const name = std::string(toString(run.regime)) + "_x"
                + std::to_string(int(run.upscaling));
        Path const directory = Path(o.dumpDirectory + name);
        directory.mkdirRecursive();
        writePng(maskImage(mask, referenceFor(0).getWidth(), referenceFor(0).getHeight()),
                Path(directory + "mask.png"));
        if (run.aliasedReference.getWidth() != 0) {
            writePng(run.aliasedReference, Path(directory + "aliased_reference.png"));
            writePng(differenceImage(run.captures.front(), run.aliasedReference, o.diffGain),
                    Path(directory + "aliased_diff.png"));
        }
        if (run.msaaReference.getWidth() != 0) {
            writePng(run.msaaReference, Path(directory + "msaa_reference.png"));
            writePng(differenceImage(run.msaaReference, referenceFor(0), o.diffGain),
                    Path(directory + "msaa_vs_reference_diff.png"));
        }
        char file[64];
        for (size_t k = 0; k < run.captures.size(); k++) {
            snprintf(file, sizeof(file), "capture_%02zu.png", k);
            writePng(run.captures[k], Path(directory + file));
            snprintf(file, sizeof(file), "reference_%02zu.png", k);
            writePng(referenceFor(k), Path(directory + file));
            snprintf(file, sizeof(file), "diff_%02zu.png", k);
            writePng(differenceImage(run.captures[k], referenceFor(k), o.diffGain),
                    Path(directory + file));
        }
        printf("taa_harness: wrote %s (diff gain %gx)\n", directory.c_str(), o.diffGain);
    }

    run.references.clear();
    run.captures.clear();
}

void writeJson(App* app, std::vector<Record> const& records) {
    Options const& o = app->options;
    std::ostringstream json;
    json << "{\n";
    json << "  \"model\": \"" << o.model.c_str() << "\",\n";
    json << "  \"backend\": \"" << int(app->engine->getBackend()) << "\",\n";
    json << "  \"size\": " << o.size << ",\n";
    json << "  \"warmup_frames\": " << o.warmupFrames << ",\n";
    json << "  \"capture_frames\": " << o.captureFrames << ",\n";
    json << "  \"reference_filter\": \""
         << (o.referenceFilter == ReferenceFilter::BOX ? "box"
                    : o.referenceFilter == ReferenceFilter::LANCZOS ? "lanczos"
                                                                    : "lanczos-filament")
         << "\",\n";
    json << "  \"reference_samples\": " << o.referenceSamples << ",\n";
    json << "  \"msaa\": " << int(o.msaaSamples) << ",\n";
    json << "  \"camera_motion\": \""
         << (o.cameraMotion == CameraMotion::ROTATE ? "rotate" : "translate") << "\",\n";
    if (o.cameraMotion == CameraMotion::TRANSLATE) {
        json << "  \"camera_texels_per_frame\": " << o.cameraTexelsPerFrame << ",\n";
        json << "  \"camera_translation_align\": \""
             << (o.cameraTranslationAlign == TranslationAlign::PIXEL ? "pixel"
                        : o.cameraTranslationAlign == TranslationAlign::SUBPIXEL ? "subpixel"
                                                                                  : "as-requested")
             << "\",\n";
    } else {
        json << "  \"camera_speed_deg_per_sec\": " << o.cameraRotateSpeedDegPerSec << ",\n";
    }
    json << "  \"records\": [\n";
    for (size_t i = 0; i < records.size(); i++) {
        Record const& r = records[i];
        json << "    { \"regime\": \"" << r.regime << "\", \"upscaling\": " << r.upscaling
             << ", \"metric\": \"" << r.metric << "\", \"value\": " << r.value << " }"
             << (i + 1 < records.size() ? "," : "") << "\n";
    }
    json << "  ]\n}\n";

    std::ofstream out(o.output.c_str(), std::ios::trunc);
    out << json.str();
    printf("taa_harness: wrote %zu records to %s\n", records.size(), o.output.c_str());
}

// ------------------------------------------------------------------------------------------------
// Callbacks
// ------------------------------------------------------------------------------------------------

App* g_app = nullptr;

void setup(Engine* engine, View* view, Scene* scene) {
    App* app = g_app;
    app->engine = engine;
    app->view = view;
    app->scene = scene;

    // Post-processing stays on -- TAA lives inside it -- but dithering must not, since it adds
    // per-frame noise that would land straight in the flicker metric.
    // Every frame this harness renders needs to be exactly what it claims to be: TAA's raw
    // resolve for a capture, a genuinely unfiltered point sample for the reference and aliased
    // accumulations, an untouched hardware resolve for the MSAA cross-check. View::AntiAliasing
    // defaults to FXAA, and FXAA runs unconditionally whenever post-processing is on, AFTER TAA's
    // own resolve -- it is not gated by whether TAA is enabled. Left on, it silently smooths every
    // one of those frames, including the ones with no antialiasing of any kind requested, which
    // is what made the "aliased" reference look filtered. Dithering adds its own per-frame noise
    // on top, which would land straight in the flicker metric. Both off; MSAA is handled per-shot
    // in applyShot instead, since it is the one antialiasing path this harness actually wants on
    // for exactly one purpose.
    view->setAntiAliasing(View::AntiAliasing::NONE);
    view->setDithering(View::Dithering::NONE);

    if (app->options.visualizeDisocclusion) {
        engine->getDebugRegistry().setProperty("d.taa.visualize_disocclusion", true);
    }

    app->names = new NameComponentManager(EntityManager::get());
    app->materials = createUbershaderProvider(engine,
            UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE);
    app->assetLoader = AssetLoader::create({ engine, app->materials, app->names });

    std::ifstream in(app->options.model.c_str(), std::ios::binary);
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
    if (buffer.empty()) {
        fprintf(stderr, "taa_harness: unable to read %s\n", app->options.model.c_str());
        exit(1);
    }
    app->asset = app->assetLoader->createAsset(buffer.data(), buffer.size());
    if (!app->asset) {
        fprintf(stderr, "taa_harness: unable to parse %s\n", app->options.model.c_str());
        exit(1);
    }
    app->instance = app->asset->getInstance();

    ResourceConfiguration configuration = {};
    configuration.engine = engine;
    CString const gltfPath(app->options.model.getAbsolutePath().c_str());
    configuration.gltfPath = gltfPath.c_str();
    configuration.normalizeSkinningWeights = true;
    app->resourceLoader = new gltfio::ResourceLoader(configuration);
    app->stbDecoder = createStbProvider(engine);
    app->ktxDecoder = createKtx2Provider(engine);
    app->resourceLoader->addTextureProvider("image/png", app->stbDecoder);
    app->resourceLoader->addTextureProvider("image/jpeg", app->stbDecoder);
    app->resourceLoader->addTextureProvider("image/ktx2", app->ktxDecoder);
    // Synchronous on purpose: an asset that finishes streaming partway through a run would
    // change the image under the measurement.
    app->resourceLoader->loadResources(app->asset);
    // Skinned assets ship bounds for the rest pose, which is not where the animation puts them.
    app->instance->recomputeBoundingBoxes();
    app->asset->releaseSourceData();

    scene->addEntities(app->asset->getEntities(), app->asset->getEntityCount());

    // Frame the whole asset from its bounding sphere, so the camera path is scale-independent
    // and the same for every model handed to the harness. FilamentAsset::getBoundingBox() is not
    // usable for this: it is the union of the glTF accessor min/max values, i.e. mesh-local and
    // with no node transforms applied, which for a model whose root node scales it is off by
    // that scale.
    Aabb const bounds = worldBounds(engine, app->asset);
    app->target = (bounds.min + bounds.max) * 0.5f;
    app->radius = length(bounds.max - bounds.min) * 0.5f;
    if (app->radius <= 0.0f) {
        app->radius = 1.0f;
    }

    // The IBL also supplies the skybox. A black background would be a poor test scene: most of
    // what TAA does wrong shows up where a silhouette meets a lit background, and against black
    // there is nothing to ghost onto.
    // Set through the app, not on the camera: this is the same state FilamentApp2 re-applies on
    // its own schedule, so the only way to make it stick is to change what it re-applies.
    app->filamentApp->setCameraNearFar(app->radius * 0.05f, app->radius * 20.0f);

    if (IBL* ibl = app->filamentApp->getIBL(); ibl) {
        scene->setIndirectLight(ibl->getIndirectLight());
        scene->setSkybox(ibl->getSkybox());
    }

    // A directional light as well as the IBL: the harness needs specular highlights and hard
    // silhouettes, which are where the artifacts being measured actually live.
    app->sunlight = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::SUN)
            .intensity(100000.0f)
            .direction({ 0.6f, -1.0f, -0.8f })
            .castShadows(true)
            .build(*engine, app->sunlight);
    scene->addEntity(app->sunlight);

    buildSchedule(app);
    printf("taa_harness: %zu runs, %zu scheduled frames\n", app->runs.size(), app->shots.size());
}

// One line on stderr, overwritten in place with \r rather than scrolling: how many of the
// scheduled frames are done and which run they belong to. Frame counts, not wall-clock time --
// Lanczos references alone can be an order of magnitude more frames than box ones (see
// referenceSamples above), so a frame count is the only estimate that stays honest across
// --reference-filter.
void printProgress(App* app, Shot const& completed) {
    if (!app->options.showProgress) {
        return;
    }
    size_t const total = app->shots.size();
    size_t const done = app->nextShot;
    Run const& run = app->runs[completed.runIndex];

    constexpr int kBarWidth = 30;
    int const filled = int(double(done) * kBarWidth / double(std::max<size_t>(total, 1)));
    char bar[kBarWidth + 1];
    for (int i = 0; i < kBarWidth; i++) {
        bar[i] = i < filled ? '=' : ' ';
    }
    bar[kBarWidth] = '\0';

    fprintf(stderr, "\rtaa_harness: [%s] %5zu/%-5zu  %s x%g", bar, done, total,
            toString(run.regime), double(run.upscaling));
    if (done >= total) {
        fprintf(stderr, "\n");
    }
    fflush(stderr);
}

// Everything about the framing that the host app can change underneath the harness, folded into
// one number so a change is detectable.
float cameraSignature(View* view) {
    Viewport const& vp = view->getViewport();
    return view->getCamera().getFieldOfViewInDegrees(Camera::Fov::VERTICAL)
            + float(vp.width) * 1e3f + float(vp.height) * 1e6f;
}

void preRender(Engine*, View* view, Scene*, Renderer*) {
    App* app = g_app;
    app->shotApplied = false;

    float const signature = cameraSignature(view);
    if (signature != app->cameraSignature) {
        if (app->nextShot > 0) {
            fprintf(stderr, "taa_harness: the view was reconfigured mid-run; results would mix "
                            "two framings, so nothing is reported\n");
            exit(1);
        }
        app->cameraSignature = signature;
        app->stableFrames = 0;
        return;
    }
    if (app->stableFrames < App::kRequiredStableFrames) {
        app->stableFrames++;
        return;
    }
    if (app->nextShot >= app->shots.size()) {
        return;
    }
    applyShot(app, app->shots[app->nextShot]);
    app->shotApplied = true;
}

void postRender(Engine* engine, View* view, Scene*, Renderer* renderer) {
    App* app = g_app;
    if (!app->shotApplied) {
        return;
    }

    Shot const& shot = app->shots[app->nextShot++];
    printProgress(app, shot);
    bool const wanted = shot.destination != Destination::NONE;

    if (wanted) {
        Viewport const& vp = view->getViewport();
        size_t const size = size_t(vp.width) * vp.height * 4;
        auto* pixels = new uint8_t[size];
        app->pendingReadbacks.fetch_add(1, std::memory_order_acq_rel);

        struct Capture {
            App* app;
            Shot shot;
            uint32_t width;
            uint32_t height;
        };

        backend::PixelBufferDescriptor descriptor(pixels, size,
                backend::PixelBufferDescriptor::PixelDataFormat::RGBA,
                backend::PixelBufferDescriptor::PixelDataType::UBYTE,
                [](void* buffer, size_t, void* user) {
                    auto* capture = static_cast<Capture*>(user);
                    auto const* rgba = static_cast<uint8_t const*>(buffer);
                    LinearImage image = decode(rgba, capture->width, capture->height);
                    Run& run = capture->app->runs[capture->shot.runIndex];
                    if (capture->shot.destination == Destination::MSAA) {
                        run.msaaReference = std::move(image);
                    } else if (capture->shot.destination == Destination::ALIASED) {
                        run.aliasedReference = std::move(image);
                    } else if (capture->shot.destination == Destination::CAPTURE) {
                        run.captures[capture->shot.slot] = std::move(image);
                    } else {
                        // Sum here and divide once the whole reference is in: accumulating in
                        // linear light is the point, a running mean would just add rounding.
                        LinearImage& accumulator = run.references[capture->shot.slot];
                        if (accumulator.getWidth() == 0) {
                            accumulator = LinearImage(capture->width, capture->height, 3);
                        }
                        float* dst = accumulator.getPixelRef();
                        float const* src = image.getPixelRef();
                        size_t const count = size_t(capture->width) * capture->height * 3;
                        float const weight = capture->shot.weight;
                        for (size_t i = 0; i < count; i++) {
                            dst[i] += src[i] * weight;
                        }
                        run.referenceWeight[capture->shot.slot] += double(weight);
                    }
                    delete[] static_cast<uint8_t*>(buffer);
                    capture->app->pendingReadbacks.fetch_sub(1, std::memory_order_acq_rel);
                    delete capture;
                },
                new Capture{ app, shot, vp.width, vp.height });

        renderer->readPixels(uint32_t(vp.left), uint32_t(vp.bottom), vp.width, vp.height,
                std::move(descriptor));
    }

    if (app->nextShot >= app->shots.size()) {
        engine->flushAndWait();
        app->filamentApp->close();
    }
}

void cleanup(Engine* engine, View*, Scene*) {
    App* app = g_app;

    // Everything is read back by now (the schedule waits a frame at a time), but the last frame's
    // callback still has to be delivered before the images can be read.
    engine->flushAndWait();

    if (!app->finished) {
        app->finished = true;
        std::vector<Record> records;
        for (Run& run: app->runs) {
            appendMetrics(app, run, &records);
        }
        writeJson(app, records);
    }

    if (app->sunlight) {
        engine->destroy(app->sunlight);
        EntityManager::get().destroy(app->sunlight);
    }
    if (app->asset) {
        app->assetLoader->destroyAsset(app->asset);
    }
    delete app->resourceLoader;
    delete app->stbDecoder;
    delete app->ktxDecoder;
    AssetLoader::destroy(&app->assetLoader);
    app->materials->destroyMaterials();
    delete app->materials;
    delete app->names;
}

// ------------------------------------------------------------------------------------------------
// Command line
// ------------------------------------------------------------------------------------------------

std::vector<std::string> split(std::string const& input) {
    std::vector<std::string> out;
    std::stringstream stream(input);
    std::string item;
    while (std::getline(stream, item, ',')) {
        if (!item.empty()) {
            out.push_back(item);
        }
    }
    return out;
}

} // anonymous namespace

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    static App app;
    g_app = &app;

    Options& o = app.options;
    o.model = Path(config.positionalArgs[0].c_str_safe());
    o.output = Path(config.getString("output").c_str_safe());
    CString const dump = config.getString("dump");
    if (!dump.empty()) {
        o.dumpDirectory = Path(dump.c_str());
        o.dumpDirectory.mkdirRecursive();
    }
    o.visualizeDisocclusion = config.getBool("visualize-disocclusion");
    o.size = uint32_t(config.getInt("size", 512));
    o.warmupFrames = uint32_t(config.getInt("warmup", 64));
    o.captureFrames = uint32_t(config.getInt("frames", 16));
    // Parsed before reference-samples: the sample-count default below depends on which filter was
    // requested, and reading it back the other way around silently defaulted every filter to
    // Lanczos's sample count regardless of -F, which is correct but wasteful for box.
    CString const filter = config.getString("reference-filter");
    if (filter == "box") {
        o.referenceFilter = ReferenceFilter::BOX;
    } else if (filter == "lanczos") {
        o.referenceFilter = ReferenceFilter::LANCZOS;
    } else if (filter == "lanczos-filament") {
        o.referenceFilter = ReferenceFilter::LANCZOS_FILAMENT;
    } else if (!filter.empty()) {
        fprintf(stderr, "taa_harness: unknown reference filter '%s'\n", filter.c_str());
        exit(1);
    }
    o.referenceSamples = uint32_t(config.getInt("reference-samples", 0));
    if (o.referenceSamples == 0) {
        // Measured, not guessed: against a 1024-sample accumulation, box is within 0.0007 RMSE by
        // 32 samples while Lanczos is still at 0.004+ there and reaches 0.0012 by 256. It
        // converges far more slowly because its support covers sixteen times the area and its
        // negative lobes make the estimator a noisier ratio. Both defaults put the reference's own
        // noise at least an order of magnitude below the errors being measured -- re-measure if
        // View::AntiAliasing or View::Dithering are ever reintroduced, since both were quietly
        // smoothing every accumulated sample and made convergence look better than it is.
        o.referenceSamples = o.referenceFilter == ReferenceFilter::BOX ? 32 : 256;
    }
    o.edgeThreshold = config.getFloat("edge-threshold", 0.08f);
    o.animation = uint32_t(config.getInt("animation", 0));
    o.objectSpeed = double(config.getFloat("object-speed", 1.0f));
    CString const motion = config.getString("camera-motion");
    if (motion == "rotate") {
        o.cameraMotion = CameraMotion::ROTATE;
    } else if (motion == "translate") {
        o.cameraMotion = CameraMotion::TRANSLATE;
    } else if (!motion.empty()) {
        fprintf(stderr, "taa_harness: unknown camera motion '%s'\n", motion.c_str());
        exit(1);
    }
    o.cameraRotateSpeedDegPerSec = double(config.getFloat("camera-speed", 90.0f));
    o.cameraTexelsPerFrame = double(config.getFloat("camera-texels-per-frame", 3.5f));
    CString const align = config.getString("camera-translation-align");
    if (align == "as-requested") {
        o.cameraTranslationAlign = TranslationAlign::AS_REQUESTED;
    } else if (align == "pixel") {
        o.cameraTranslationAlign = TranslationAlign::PIXEL;
    } else if (align == "subpixel") {
        o.cameraTranslationAlign = TranslationAlign::SUBPIXEL;
    } else if (!align.empty()) {
        fprintf(stderr, "taa_harness: unknown --camera-translation-align '%s'\n", align.c_str());
        exit(1);
    }
#if defined(_WIN32)
    o.showProgress = _isatty(_fileno(stderr)) != 0;
#else
    o.showProgress = isatty(fileno(stderr)) != 0;
#endif
    if (config.getBool("quiet")) {
        o.showProgress = false;
    }
    o.diffGain = config.getFloat("diff-gain", 8.0f);
    o.msaaSamples = uint8_t(config.getInt("msaa", 0));
    // The whole TemporalAntiAliasingOptions surface, in the same JSON the viewer already reads
    // and writes, rather than a growing pile of one-off flags. Everything except `enabled` and
    // `upscaling` is honoured; those two the schedule owns, since they are what a run IS.
    CString const settingsFile = config.getString("settings");
    if (!settingsFile.empty()) {
        std::ifstream in(settingsFile.c_str(), std::ios::binary);
        std::string const json((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
        if (json.empty()) {
            fprintf(stderr, "taa_harness: unable to read %s\n", settingsFile.c_str());
            exit(1);
        }
        viewer::Settings settings = {};
        viewer::JsonSerializer serializer;
        if (!serializer.readJson(json.c_str(), json.size(), &settings)) {
            fprintf(stderr, "taa_harness: unable to parse %s\n", settingsFile.c_str());
            exit(1);
        }
        o.taaOptions = settings.view.taa;
        o.jitterPattern = o.taaOptions.jitterPattern;
    }
    CString const pattern = config.getString("jitter-pattern");
    if (!pattern.empty() && !parseJitterPattern(pattern.c_str(), &o.jitterPattern)) {
        fprintf(stderr, "taa_harness: unknown jitter pattern '%s'\n", pattern.c_str());
        exit(1);
    }

    for (std::string const& name: split(config.getString("regimes").c_str_safe())) {
        Regime regime;
        if (!parseRegime(name, &regime)) {
            fprintf(stderr, "taa_harness: unknown regime '%s'\n", name.c_str());
            exit(1);
        }
        o.regimes.push_back(regime);
    }
    if (o.regimes.empty()) {
        o.regimes = { Regime::STATIC, Regime::CAMERA, Regime::OBJECT };
    }
    for (std::string const& value: split(config.getString("upscaling").c_str_safe())) {
        o.upscalings.push_back(std::stof(value));
    }
    if (o.upscalings.empty()) {
        // Both ratios, always. At upscaling > 1 the resolve runs at output resolution while
        // depth is still at render resolution, and that difference has shipped a regression
        // before precisely because it was only ever tested at 1:1.
        o.upscalings = { 1.0f, 2.0f };
    }

    if (config.iblDirectory.empty()) {
        config.iblDirectory = CString("assets/ibl/lightroom_14b");
    }
    config.width = o.size;
    config.height = o.size;
    config.headless = true;
    auto fApp = samples::getBuilder(config, dm, loader)
                        .setup(setup)
                        .cleanup(cleanup)
                        .preRender(preRender)
                        .postRender(postRender)
                        .build();
    app.filamentApp = fApp.get();
    return fApp;
}

samples::SampleParameters createAppParameters() {
    return {
        samples::Parameter::makeString("output", 'o', "Path of the metrics JSON to write",
                "taa_metrics.json"),
        samples::Parameter::makeString("dump", 'd',
                "Directory to write captured and reference PNGs into", ""),
        samples::Parameter::makeInt("size", 'S', "Output resolution (square)", 512, 16),
        samples::Parameter::makeInt("warmup", 'W',
                "Frames of converged TAA before motion begins (64 measured comfortably past "
                "feedback's convergence point; see Options::warmupFrames)", 64, 0),
        samples::Parameter::makeInt("frames", 'n',
                "Frames captured after motion onset; keep at a full jitter cycle", 16, 2),
        samples::Parameter::makeInt("reference-samples", 'R',
                "Jittered frames accumulated into each reference; 0 picks a converged default "
                "for the chosen --reference-filter (32 box, 256 lanczos)", 0, 0),
        samples::Parameter::makeString("regimes", 'r',
                "Comma-separated: static,camera,object", "static,camera,object"),
        samples::Parameter::makeString("upscaling", 'u',
                "Comma-separated TAA upscaling factors", "1,2"),
        // 4 and not 8: on hardware that does not support 8x -- Apple silicon, for one -- the
        // request is silently clamped and you get a 4x image believing it is 8x. 4x is the count
        // that is actually guaranteed.
        samples::Parameter::makeInt("msaa", 'M',
                "Cross-check the accumulated reference against an N-sample MSAA render", 0, 0, 4),
        samples::Parameter::makeString("reference-filter", 'F',
                "Kernel the reference integrates against: lanczos (what TAA aims at), "
                "lanczos-filament (what it actually evaluates), or box",
                "lanczos"),
        samples::Parameter::makeString("jitter-pattern", 'J',
                "rgss4, helix4, halton8, halton16, halton32 (overrides --settings)", ""),
        samples::Parameter::makeString("settings", 's',
                "JSON settings file; view.taa is applied to every run", ""),
        samples::Parameter::makeFloat("diff-gain", 'G',
                "Amplification applied to dumped difference images", 8.0f),
        samples::Parameter::makeInt("animation", 'A',
                "Index of the glTF animation the object regime plays", 0, 0),
        samples::Parameter::makeFloat("object-speed", '\0',
                "Animation playback speed multiplier, object regime", 1.0f),
        samples::Parameter::makeString("camera-motion", '\0',
                "rotate (orbit, re-aimed at the target) or translate (pure dolly, no rotation), "
                "camera regime", "rotate"),
        samples::Parameter::makeFloat("camera-speed", '\0',
                "Orbit speed in degrees/second, rotate mode", 90.0f),
        samples::Parameter::makeFloat("camera-texels-per-frame", '\0',
                "Parallax speed at the target's depth, in render-resolution texels/frame, "
                "translate mode", 3.5f),
        samples::Parameter::makeString("camera-translation-align", '\0',
                "as-requested, pixel (every reprojection an exact texel fetch), or subpixel "
                "(every reprojection a half-texel fetch) -- translate mode", "as-requested"),
        samples::Parameter::makeBool("quiet", 'q',
                "Suppress the progress bar even when stderr is a terminal", false),
        samples::Parameter::makeFloat("edge-threshold", 'E',
                "Luminance gradient above which a texel counts as a silhouette", 0.08f),
        samples::Parameter::makeBool("visualize-disocclusion", '\0',
                "Render taa.mat's disocclusion mask instead of the image (use with --dump; "
                "the metrics from such a run are meaningless)", false),
    };
}

#ifndef __ANDROID__
int main(int const argc, char* argv[]) {
    samples::CommandLineSpecification spec = {
        .sampleDescription =
                "SAMPLE_TAA_HARNESS measures temporal anti-aliasing artifacts\n\n"
                "Renders a fixed, reproducible frame schedule headlessly and reports masked\n"
                "per-regime metrics as JSON: error against a converged supersampled reference,\n"
                "the same error restricted to silhouettes, temporal variance (flicker),\n"
                "high-frequency energy (sharpness), and how many frames history takes to\n"
                "recover after motion begins. Every metric is produced at 1:1 and at the\n"
                "requested upscaling ratios.",
        .positionalArgsDescription = { "glTF/glb model" },
        .requiredPositionalArgCount = 1,
        .parameters = createAppParameters(),
    };

    SampleConfig config;
    samples::handleCommandLineArguments(argc, argv, &config, spec);
    auto dm = samples::getDisplayManager(config);

    Path const model(config.positionalArgs[0].c_str_safe());
    if (!model.exists()) {
        fprintf(stderr, "taa_harness: file %s not found\n", model.c_str());
        return 1;
    }

    config.title = "TAA Harness";
    config.headless = true;
    if (config.assetsPath.empty()) {
        config.assetsPath = CString(
                (Path::getCurrentExecutable().getParent() + "../../samples").c_str());
    }
    auto loader = samples::getAssetLoader(config);
    auto fApp = createSampleApp(config, dm.get(), loader.get());
    return samples::runApp(std::move(fApp), std::move(dm), std::move(loader));
}
#endif
