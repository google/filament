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
 * How the object regime moves
 * ----------------------------
 * --object-motion picks what the object regime actually moves:
 *
 *   - animation: the glTF clip (-A), played through gltfio's Animator. --object-speed multiplies
 *     playback time, for when the authored clip itself moves too little to measure. A skinned
 *     asset (Fox) DEFORMS here, which per-object motion vectors do not cover, so this mode cannot
 *     show what view.motionVectors changes.
 *   - translate: the asset's root entity dollies screen-right through the TransformManager, by
 *     --object-texels-per-frame render-resolution texels at the target's depth, with the same
 *     --camera-translation-align policy as the camera dolly. The mirror image of camera translate:
 *     identical screen-space speed, but now it is the object that moved, which is exactly what the
 *     motion vectors pass reports and camera reprojection alone cannot see.
 *   - spin: the root entity rotates about the vertical axis through its bounds centre at
 *     --object-speed degrees/second. Screen-space speed varies across the model, and specular
 *     highlights slide over the surface, which is the content camera-only reprojection smears.
 *
 * translate and spin are pure functions of the shot, like the camera modes, so the per-frame
 * references track them for free.
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

#include "taa_metrics.h"

#include "common/arguments.h"

#include "materials/uberarchive.h"

#include <viewer/Settings.h>

#include <imageio-lite/ImageEncoder.h>

#if !defined(__ANDROID__)
#include <imageio/ImageEncoder.h>
#endif

#include <image/LinearImage.h>

#include <gltfio/Animator.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/FilamentInstance.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

#include <filamentapp/AssetLoader.h>
#include <filamentapp/DesktopAssetLoader.h>
#include <filamentapp/FilamentApp2.h>
#include <filamentapp/HeadlessDisplayManager.h>
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

#if defined(__ANDROID__)
#include <private/backend/VirtualMachineEnv.h>
#endif

#include <backend/PixelBufferDescriptor.h>

#if defined(__ANDROID__)
#include <utils/android/ThermalManager.h>
#endif
#include <utils/EntityManager.h>
#include <utils/getopt.h>
#include <utils/NameComponentManager.h>
#include <utils/Path.h>
#include <utils/Systrace.h>

#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#if defined(__ANDROID__)
#include <android/log.h>
#endif
#if defined(_WIN32)
#include <io.h>
#endif
#if defined(__ANDROID__)
#include <jni.h>
#endif
#include <samples/SampleConfig.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#if !defined(_WIN32)
#include <unistd.h>
#endif

using namespace filament;
using namespace filament::math;
using namespace filament::gltfio;
using namespace filament::tools::taa;
using namespace image;
using namespace utils;

namespace {

constexpr double kBaseAzimuthRadians = 60.0 * F_PI / 180.0;

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
    // Scene animates, camera still. Per-object motion vectors are off by default (and do not yet
    // cover skinning/morphing), so interior ghosting is EXPECTED here and is reported, not failed;
    // what matters is that the dis/occluded edges recover.
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

enum class RunKind : uint8_t {
    QUALITY,
    PERF_BASELINE,
    PERF_UPSCALER,
};

enum class PerfPhase : uint8_t {
    NONE,
    WARMUP,
    TIMED,
    DRAIN,
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
    PerfPhase perfPhase = PerfPhase::NONE;
    uint32_t perfFrameIndex = 0;
};

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

// What the OBJECT regime moves. See "How the object regime moves" at the top of the file.
enum class ObjectMotion : uint8_t {
    // The glTF clip, through gltfio's Animator. A skinned asset deforms, which per-object motion
    // vectors do not cover.
    ANIMATION,
    // The asset's root entity dollies screen-right through the TransformManager, the mirror image
    // of CameraMotion::TRANSLATE: same texel speed, same alignment policy, opposite reference
    // frame. The motion is rigid, so it is exactly what the motion vectors pass reports.
    TRANSLATE,
    // The root entity rotates about the vertical axis through its bounds centre.
    SPIN,
};

struct RunConfig {
    std::string label;
    std::string pairedBaselineLabel;
    RunKind kind = RunKind::QUALITY;
    int assetIndex = 0;
    Regime regime = Regime::STATIC;
    float upscaling = 1.0f;
    float nominalUpscaling = 1.0f;
    uint32_t width = 512;
    uint32_t height = 512;
    CameraMotion cameraMotion = CameraMotion::ROTATE;
    double cameraRotateSpeedDegPerSec = 90.0;
    double cameraTexelsPerFrame = 3.5;
    TranslationAlign cameraTranslationAlign = TranslationAlign::AS_REQUESTED;
    ObjectMotion objectMotion = ObjectMotion::ANIMATION;
    double objectTexelsPerFrame = 3.5;
    double objectSpeed = 1.0;
    double animationStartTime = 0.0;
    uint32_t animation = 0;
    bool motionVectorsEnabled = false;
    bool ssrEnabled = false;
    bool deduplicateStaticHoldReferences = false;
    bool disableSubpasses = false;
};

struct RunMetrics {
    double rmseMean = 0.0;
    double edgeRmseMean = 0.0;
    double gmsdMean = 0.0;
    double hotspotTop10Rmse = 0.0;
    double hotspotPeakRmse = 0.0;
    double sharpnessMean = 0.0;
    double edgeSharpnessMean = 0.0;
    double detailTransferMean = 0.0;
    double edgeDetailTransferMean = 0.0;
    double spuriousHfMean = 0.0;
    double flickerVariance = 0.0;
    double edgeFlickerVariance = 0.0;
    double flickerTop1PctRms = 0.0;
    double residualFlickerTop1PctRms = 0.0;
    double subpixelRingFlickerRms = 0.0;
    bool hasSubpixelRings = false;
    double edgeGhostPeak = 0.0;
    double edgeTailError = 0.0;
    double edgeEndRmse = 0.0;
    double edgeRecoveryFrames = 0.0;
    double wakeGhostingRms = 0.0;
    // GPU performance metrics (milliseconds, lower is better)
    double frameGpuMsP50 = 0.0;
    double frameGpuMsP95 = 0.0;
    double frameGpuMsIqr = 0.0;
    double frameGpuMsMean = 0.0;
    double frameGpuMsStddev = 0.0;
    double baselineGpuMsP50 = 0.0;
    double baselineGpuMsMean = 0.0;
    double upscalerGpuMsP50 = 0.0;
    double upscalerGpuMsP95 = 0.0;
    double upscalerGpuMsMean = 0.0;
    double fenceFrameMsP50 = 0.0;
    double fenceBaselineMsP50 = 0.0;
    double fenceUpscalerMsP50 = 0.0;
    double windowWallFrameMs = 0.0;
    double windowWallBaselineMs = 0.0;
    double windowWallUpscalerMs = 0.0;
    double timingDiscrepancyMs = 0.0;
    bool postCheckPassed = false;
    double postCheckAliasedRmse = 0.0;
    double postCheckLumaMean = 0.0;
    uint32_t timedFramesCount = 0;
};

struct Run {
    RunConfig config;
    RunMetrics summary;
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
    std::vector<CapturePose> capturePoses;
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
    // Performance window state
    std::string systraceWindowName;
    int64_t timedWindowStartNs = 0;
    int64_t timedWindowEndNs = 0;
    int64_t timedWindowGpuDoneNs = 0;
    bool systraceWindowOpen = false;
    uint32_t lastHarvestedFrameId = 0;
    std::vector<Renderer::FrameInfo> timedFrameInfos;
};

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

void writePng(LinearImage const& img, Path const& path) {
    std::ofstream out(path.c_str(), std::ios::binary | std::ios::trunc);
#if defined(__ANDROID__)
    imageio_lite::ImageEncoder::encode(
            out, imageio_lite::ImageEncoder::Format::TIFF, img, "", CString(path.c_str()));
#else
    ImageEncoder::encode(out, ImageEncoder::Format::PNG, img, "", path.c_str());
#endif
}

int64_t steadyClockNowNs() noexcept {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

char const* thermalStatusName(int const status) noexcept {
    switch (status) {
        case 0: return "NONE";
        case 1: return "LIGHT";
        case 2: return "MODERATE";
        case 3: return "SEVERE";
        case 4: return "CRITICAL";
        case 5: return "EMERGENCY";
        case 6: return "SHUTDOWN";
        default: return "UNKNOWN";
    }
}

bool queryAndroidFixedPerformanceMode() noexcept {
    if (char const* env = std::getenv("TAA_HARNESS_FIXED_PERF_MODE")) {
        if (std::strcmp(env, "1") == 0 || std::strcmp(env, "true") == 0) {
            return true;
        }
    }
    return false;
}

bool parsePerfResolution(std::string_view const spec, uint32_t* outW, uint32_t* outH) noexcept {
    if (spec == "720p") {
        *outW = 1280;
        *outH = 720;
        return true;
    }
    if (spec == "1080p") {
        *outW = 1920;
        *outH = 1080;
        return true;
    }
    if (spec == "1440p") {
        *outW = 2560;
        *outH = 1440;
        return true;
    }
    if (spec == "4k" || spec == "2160p") {
        *outW = 3840;
        *outH = 2160;
        return true;
    }
    size_t const xPos = spec.find_first_of("xX");
    if (xPos == std::string_view::npos || xPos == 0 || xPos + 1 >= spec.size()) {
        return false;
    }
    unsigned int w = 0;
    unsigned int h = 0;
    std::string const s(spec);
    char extra = '\0';
    if (std::sscanf(s.c_str(), "%ux%u%c", &w, &h, &extra) != 2 &&
            std::sscanf(s.c_str(), "%uX%u%c", &w, &h, &extra) != 2) {
        return false;
    }
    if (w < 16 || h < 16 || w > 16384 || h > 16384) {
        return false;
    }
    *outW = w;
    *outH = h;
    return true;
}

// ------------------------------------------------------------------------------------------------
// Application state
// ------------------------------------------------------------------------------------------------

struct Options {
    Path model;
    Path output = "taa_metrics.json";
    Path dumpDirectory;
    std::string label;
    std::string settingsLabelSuffix;
    bool userSetLabel = false;
    std::vector<Path> compareFiles;
    bool benchmark = false;
    bool perf = false;
    uint32_t perfWidth = 1920;
    uint32_t perfHeight = 1080;
    uint32_t perfWarmupFrames = 64;
    uint32_t perfFrames = 128;
    int thermalStatusStart = 0;
    int thermalStatusEnd = 0;
    bool fixedPerformanceMode = false;
    bool ssrEnabled = false;
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
    // Shared by the camera dolly and ObjectMotion::TRANSLATE: the question it answers (exact vs
    // half-texel history fetches) is the same whichever side moved.
    TranslationAlign cameraTranslationAlign = TranslationAlign::AS_REQUESTED;
    ObjectMotion objectMotion = ObjectMotion::ANIMATION;
    // ObjectMotion::TRANSLATE's speed, with the camera dolly's convention and default, so a
    // translate/translate pair of runs differs only in which side moved.
    double objectTexelsPerFrame = 3.5;
    // ANIMATION: multiplies elapsed animation time. The animation itself sets the natural time
    // scale, so unlike the camera there is no principled default speed -- 1.0 is "play it as
    // authored" and this exists for when that turns out to move too little to measure, the same
    // complaint camera rotation had at its old default.
    // SPIN: degrees per second, defaulting to the camera orbit's speed (see --camera-speed).
    double objectSpeed = 1.0;
    float edgeThreshold = 0.08f;
    float diffGain = 8.0f;
    uint8_t msaaSamples = 0;
    TemporalAntiAliasingOptions::JitterPattern jitterPattern =
            TemporalAntiAliasingOptions::JitterPattern::HALTON_23_X16;
    TemporalAntiAliasingOptions taaOptions = {};
    MotionVectorsOptions motionVectorsOptions = {};
    ReferenceFilter referenceFilter = ReferenceFilter::LANCZOS;
    // Which glTF animation the object regime plays. It matters: a survey/idle clip barely moves,
    // and a regime that does not move measures nothing.
    uint32_t animation = 0;
    double animationStartTime = 0.0;
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

struct LoadedAsset {
    Path path;
    FilamentAsset* asset = nullptr;
    FilamentInstance* instance = nullptr;
    // The asset's root transform as loaded. ObjectMotion::TRANSLATE/SPIN compose their motion
    // onto this every shot, and every other mode restores it, so a run never inherits the pose
    // the previous run left behind.
    mat4f rootRestTransform;
    // Framing, derived once from the asset bounds so the camera path is scale-independent.
    float3 target = {};
    float radius = 1.0f;
    bool isSubpixelStar = false;
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
    std::vector<LoadedAsset> assets;
    int activeAssetIndex = -1;
    uint32_t activeWidth = 0;
    uint32_t activeHeight = 0;
    Entity sunlight;
#if defined(__ANDROID__)
    ThermalManager thermalManager;
#endif

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
    bool timedFrameSystraceOpen = false;
    bool thermalStartRecorded = false;

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
    int exitCode = 0;
};

// ------------------------------------------------------------------------------------------------
// Schedule construction
// ------------------------------------------------------------------------------------------------

static constexpr uint32_t kPerfDrainFrames = 4;

void addScheduledRun(App* app, RunConfig const& cfg) {
    Options const& o = app->options;
    int const runIndex = int(app->runs.size());
    Run run;
    run.config = cfg;
    run.config.kind = RunKind::QUALITY;
    run.config.width = o.size;
    run.config.height = o.size;
    run.config.nominalUpscaling = cfg.upscaling;

    // With nothing moving there is only one scene state, so one reference serves every captured
    // frame. Moving regimes advance for the first half of `captureFrames` and then hold still, so
    // `motionFrames + 1` unique poses cover the entire capture window when deduplicating the
    // static hold.
    size_t const motionFrames = std::max<size_t>(1, o.captureFrames / 2);
    size_t const movingRefs = cfg.deduplicateStaticHoldReferences
            ? std::min<size_t>(o.captureFrames, motionFrames + 1)
            : size_t(o.captureFrames);
    size_t const referenceCount = (cfg.regime == Regime::STATIC) ? 1 : movingRefs;
    run.references.resize(referenceCount);
    run.referenceWeight.assign(referenceCount, 0.0);
    run.captures.resize(o.captureFrames);
    run.capturePoses.resize(o.captureFrames);
    app->runs.push_back(std::move(run));

    // SSR reads the previous frame's colour history, so prime one uncaptured frame at the initial
    // pose before any MSAA, aliased, or reference sample is read back.
    if (cfg.ssrEnabled) {
        Shot prime;
        prime.taaEnabled = false;
        prime.runIndex = runIndex;
        prime.destination = Destination::NONE;
        app->shots.push_back(prime);
    }

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
        double const motionTime = motionTimeForFrame(r, o.captureFrames, o.stepSeconds);
        size_t const frameIndex = motionFrameIndex(r, o.captureFrames);
        if (cfg.ssrEnabled && r > 0) {
            Shot prime;
            prime.motionTime = motionTime;
            prime.motionFrameIndex = frameIndex;
            prime.taaEnabled = false;
            prime.runIndex = runIndex;
            prime.destination = Destination::NONE;
            app->shots.push_back(prime);
        }
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

void addScheduledPerfPair(App* app, RunConfig const& upscalerCfg) {
    Options const& o = app->options;
    uint32_t const outW = o.perfWidth;
    uint32_t const outH = o.perfHeight;
    uint32_t const renderW = std::max<uint32_t>(
            1u, uint32_t(std::floor(float(outW) / upscalerCfg.upscaling)));
    uint32_t const renderH = std::max<uint32_t>(
            1u, uint32_t(std::floor(float(outH) / upscalerCfg.upscaling)));

    std::string const baseLabel = upscalerCfg.label + "_baseline";

    // 1. Baseline run at render resolution (no TAA/SR, no motion vectors, subpasses disabled so
    // the colour pass and post-process tone mapping stay split into separate render passes just as
    // they are when TAA/SR sits between them).
    int const baseRunIndex = int(app->runs.size());
    {
        Run baseRun;
        baseRun.config = upscalerCfg;
        baseRun.config.kind = RunKind::PERF_BASELINE;
        baseRun.config.label = baseLabel;
        baseRun.config.upscaling = 1.0f;
        baseRun.config.nominalUpscaling = upscalerCfg.upscaling;
        baseRun.config.width = renderW;
        baseRun.config.height = renderH;
        baseRun.config.motionVectorsEnabled = false;
        baseRun.config.disableSubpasses = true;
        char winName[160];
        snprintf(winName, sizeof(winName), "taa_harness::timed_window::%s_x%g",
                baseLabel.c_str(), double(upscalerCfg.upscaling));
        baseRun.systraceWindowName = winName;
        app->runs.push_back(std::move(baseRun));

        for (uint32_t w = 0; w < o.perfWarmupFrames; w++) {
            Shot shot;
            shot.runIndex = baseRunIndex;
            shot.taaEnabled = false;
            shot.perfPhase = PerfPhase::WARMUP;
            shot.perfFrameIndex = w;
            shot.motionFrameIndex = w;
            shot.motionTime = double(w) * o.stepSeconds;
            app->shots.push_back(shot);
        }
        for (uint32_t k = 0; k < o.perfFrames; k++) {
            Shot shot;
            shot.runIndex = baseRunIndex;
            shot.taaEnabled = false;
            shot.perfPhase = PerfPhase::TIMED;
            shot.perfFrameIndex = k;
            shot.motionFrameIndex = o.perfWarmupFrames + k;
            shot.motionTime = double(o.perfWarmupFrames + k) * o.stepSeconds;
            app->shots.push_back(shot);
        }
        size_t const finalMotionIdx =
                size_t(o.perfWarmupFrames) + std::max<uint32_t>(1u, o.perfFrames) - 1u;
        double const finalMotionTime = double(finalMotionIdx) * o.stepSeconds;
        for (uint32_t d = 0; d < kPerfDrainFrames; d++) {
            Shot shot;
            shot.runIndex = baseRunIndex;
            shot.taaEnabled = false;
            shot.perfPhase = PerfPhase::DRAIN;
            shot.perfFrameIndex = d;
            shot.motionFrameIndex = finalMotionIdx;
            shot.motionTime = finalMotionTime;
            app->shots.push_back(shot);
        }
    }

    // 2. Upscaler run at output resolution (TAA/SR enabled at nominal upscaling factor u, motion
    // vectors enabled if configured). Zero readPixels occur during WARMUP and TIMED; after the
    // timed window is flushed and closed, drain frame 0 captures the final upscaled frame and
    // drain frame 1 captures the native aliased render at the identical pose for anti-cheat
    // verification.
    int const upRunIndex = int(app->runs.size());
    {
        Run upRun;
        upRun.config = upscalerCfg;
        upRun.config.kind = RunKind::PERF_UPSCALER;
        upRun.config.pairedBaselineLabel = baseLabel;
        upRun.config.nominalUpscaling = upscalerCfg.upscaling;
        upRun.config.width = outW;
        upRun.config.height = outH;
        upRun.config.disableSubpasses = false;
        upRun.captures.resize(1);
        char winName[160];
        snprintf(winName, sizeof(winName), "taa_harness::timed_window::%s_x%g",
                upscalerCfg.label.c_str(), double(upscalerCfg.upscaling));
        upRun.systraceWindowName = winName;
        app->runs.push_back(std::move(upRun));

        for (uint32_t w = 0; w < o.perfWarmupFrames; w++) {
            Shot shot;
            shot.runIndex = upRunIndex;
            shot.taaEnabled = true;
            shot.perfPhase = PerfPhase::WARMUP;
            shot.perfFrameIndex = w;
            shot.motionFrameIndex = w;
            shot.motionTime = double(w) * o.stepSeconds;
            app->shots.push_back(shot);
        }
        for (uint32_t k = 0; k < o.perfFrames; k++) {
            Shot shot;
            shot.runIndex = upRunIndex;
            shot.taaEnabled = true;
            shot.perfPhase = PerfPhase::TIMED;
            shot.perfFrameIndex = k;
            shot.motionFrameIndex = o.perfWarmupFrames + k;
            shot.motionTime = double(o.perfWarmupFrames + k) * o.stepSeconds;
            app->shots.push_back(shot);
        }
        size_t const finalMotionIdx =
                size_t(o.perfWarmupFrames) + std::max<uint32_t>(1u, o.perfFrames) - 1u;
        double const finalMotionTime = double(finalMotionIdx) * o.stepSeconds;
        for (uint32_t d = 0; d < kPerfDrainFrames; d++) {
            Shot shot;
            shot.runIndex = upRunIndex;
            shot.perfPhase = PerfPhase::DRAIN;
            shot.perfFrameIndex = d;
            shot.motionFrameIndex = finalMotionIdx;
            shot.motionTime = finalMotionTime;
            if (d == 0) {
                shot.taaEnabled = true;
                shot.destination = Destination::CAPTURE;
                shot.slot = 0;
            } else if (d == 1) {
                shot.taaEnabled = false;
                shot.destination = Destination::ALIASED;
            } else {
                shot.taaEnabled = false;
                shot.destination = Destination::NONE;
            }
            app->shots.push_back(shot);
        }
    }
}

void buildSchedule(App* app) {
    Options const& o = app->options;
    if (!o.benchmark && !o.perf) {
        for (float const upscaling: o.upscalings) {
            for (Regime const regime: o.regimes) {
                RunConfig cfg;
                cfg.label = toString(regime);
                cfg.assetIndex = 0;
                cfg.regime = regime;
                cfg.upscaling = upscaling;
                cfg.cameraMotion = o.cameraMotion;
                cfg.cameraRotateSpeedDegPerSec = o.cameraRotateSpeedDegPerSec;
                cfg.cameraTexelsPerFrame = o.cameraTexelsPerFrame;
                cfg.cameraTranslationAlign = o.cameraTranslationAlign;
                cfg.objectMotion = o.objectMotion;
                cfg.objectTexelsPerFrame = o.objectTexelsPerFrame;
                cfg.objectSpeed = o.objectSpeed;
                cfg.animationStartTime = o.animationStartTime;
                cfg.animation = o.animation;
                cfg.motionVectorsEnabled = o.motionVectorsOptions.enabled;
                cfg.ssrEnabled = o.ssrEnabled;
                cfg.deduplicateStaticHoldReferences = false;
                addScheduledRun(app, cfg);
            }
        }
        return;
    }

    // Benchmark suite: 11 curated scenarios grouped by asset (0 = Fox, 1 = DamagedHelmet,
    // 2 = SubpixelStar, 3 = SSRReflection, 4 = BusterDrone) to minimize scene switches while
    // covering all three axes of the TAA trilemma (Spatial Quality, Temporal Stability, and
    // Tracked/Unmodeled Motion).
    if (o.benchmark) {
        for (float const upscaling: o.upscalings) {
            auto makeBase = [&](char const* label, int const assetIndex, Regime const regime) {
                RunConfig cfg;
                cfg.label = label;
                cfg.assetIndex = assetIndex;
                cfg.regime = regime;
                cfg.upscaling = upscaling;
                cfg.motionVectorsEnabled = true;
                cfg.deduplicateStaticHoldReferences = true;
                return cfg;
            };

            // Asset 0: Fox.glb (hard silhouettes, rigid translation & skinned Run animation)
            addScheduledRun(app, makeBase("fox_static", 0, Regime::STATIC));
            {
                RunConfig cfg = makeBase("fox_drift", 0, Regime::CAMERA);
                cfg.cameraMotion = CameraMotion::TRANSLATE;
                cfg.cameraTexelsPerFrame = 0.1;
                cfg.cameraTranslationAlign = TranslationAlign::AS_REQUESTED;
                addScheduledRun(app, cfg);
            }
            {
                RunConfig cfg = makeBase("fox_obj_translate", 0, Regime::OBJECT);
                cfg.objectMotion = ObjectMotion::TRANSLATE;
                cfg.objectTexelsPerFrame = 3.5;
                cfg.cameraTranslationAlign = TranslationAlign::AS_REQUESTED;
                addScheduledRun(app, cfg);
            }
            {
                RunConfig cfg = makeBase("fox_skinned_run", 0, Regime::OBJECT);
                cfg.objectMotion = ObjectMotion::ANIMATION;
                cfg.animation = 2; // Run clip (vertex deformation not covered by rigid MVs)
                cfg.objectSpeed = 1.0;
                addScheduledRun(app, cfg);
            }

            // Asset 1: DamagedHelmet.glb (normal maps, specular highlights, camera & spin motion)
            addScheduledRun(app, makeBase("helmet_static", 1, Regime::STATIC));
            {
                RunConfig cfg = makeBase("helmet_cam_rotate", 1, Regime::CAMERA);
                cfg.cameraMotion = CameraMotion::ROTATE;
                cfg.cameraRotateSpeedDegPerSec = 90.0;
                addScheduledRun(app, cfg);
            }
            {
                RunConfig cfg = makeBase("helmet_cam_dolly", 1, Regime::CAMERA);
                cfg.cameraMotion = CameraMotion::TRANSLATE;
                cfg.cameraTexelsPerFrame = 3.5;
                cfg.cameraTranslationAlign = TranslationAlign::SUBPIXEL;
                addScheduledRun(app, cfg);
            }
            {
                RunConfig cfg = makeBase("helmet_obj_spin", 1, Regime::OBJECT);
                cfg.objectMotion = ObjectMotion::SPIN;
                cfg.objectSpeed = 90.0;
                addScheduledRun(app, cfg);
            }

            // Asset 2: SubpixelStar.glb (Siemens star sub-pixel geometry resolution & ring flicker)
            addScheduledRun(app, makeBase("star_static", 2, Regime::STATIC));

            // Asset 3: SSRReflection.glb (view-dependent reflection sliding across a static floor)
            {
                RunConfig cfg = makeBase("ssr_reflection", 3, Regime::CAMERA);
                cfg.cameraMotion = CameraMotion::TRANSLATE;
                cfg.cameraTexelsPerFrame = 3.5;
                cfg.cameraTranslationAlign = TranslationAlign::AS_REQUESTED;
                cfg.ssrEnabled = true;
                addScheduledRun(app, cfg);
            }

            // Asset 4: BusterDrone.glb (articulated multi-node rigid animation with per-object MVs)
            {
                RunConfig cfg = makeBase("buster_drone_anim", 4, Regime::OBJECT);
                cfg.objectMotion = ObjectMotion::ANIMATION;
                cfg.animation = 0;
                cfg.animationStartTime = 10.0;
                cfg.objectSpeed = 2.0;
                addScheduledRun(app, cfg);
            }
        }
    }

    if (o.perf) {
        bool const useSuite = o.benchmark || o.model.isEmpty();
        for (float const upscaling: o.upscalings) {
            if (!useSuite) {
                for (Regime const regime: o.regimes) {
                    RunConfig cfg;
                    cfg.label = std::string("perf_") + toString(regime);
                    cfg.assetIndex = 0;
                    cfg.regime = regime;
                    cfg.upscaling = upscaling;
                    cfg.cameraMotion = o.cameraMotion;
                    cfg.cameraRotateSpeedDegPerSec = o.cameraRotateSpeedDegPerSec;
                    cfg.cameraTexelsPerFrame = o.cameraTexelsPerFrame;
                    cfg.cameraTranslationAlign = o.cameraTranslationAlign;
                    cfg.objectMotion = o.objectMotion;
                    cfg.objectTexelsPerFrame = o.objectTexelsPerFrame;
                    cfg.objectSpeed = o.objectSpeed;
                    cfg.animationStartTime = o.animationStartTime;
                    cfg.animation = o.animation;
                    cfg.motionVectorsEnabled = true;
                    cfg.ssrEnabled = o.ssrEnabled;
                    addScheduledPerfPair(app, cfg);
                }
                continue;
            }

            auto makePerfBase = [&](char const* label, int const assetIndex, Regime const regime) {
                RunConfig cfg;
                cfg.label = label;
                cfg.assetIndex = assetIndex;
                cfg.regime = regime;
                cfg.upscaling = upscaling;
                cfg.motionVectorsEnabled = true;
                return cfg;
            };

            // 1. Skinned animation (Asset 0: Fox.glb)
            {
                RunConfig cfg = makePerfBase("perf_fox_skinned_run", 0, Regime::OBJECT);
                cfg.objectMotion = ObjectMotion::ANIMATION;
                cfg.animation = 2;
                cfg.objectSpeed = 1.0;
                addScheduledPerfPair(app, cfg);
            }
            // 2. Sub-texel camera dolly (Asset 1: DamagedHelmet.glb)
            {
                RunConfig cfg = makePerfBase("perf_helmet_cam_dolly", 1, Regime::CAMERA);
                cfg.cameraMotion = CameraMotion::TRANSLATE;
                cfg.cameraTexelsPerFrame = 3.5;
                cfg.cameraTranslationAlign = TranslationAlign::SUBPIXEL;
                addScheduledPerfPair(app, cfg);
            }
            // 3. Per-object rigid motion vectors (Asset 1: DamagedHelmet.glb)
            {
                RunConfig cfg = makePerfBase("perf_helmet_obj_spin", 1, Regime::OBJECT);
                cfg.objectMotion = ObjectMotion::SPIN;
                cfg.objectSpeed = 90.0;
                addScheduledPerfPair(app, cfg);
            }
            // 4. Screen-space reflections + camera translation (Asset 3: SSRReflection.glb)
            {
                RunConfig cfg = makePerfBase("perf_ssr_reflection", 3, Regime::CAMERA);
                cfg.cameraMotion = CameraMotion::TRANSLATE;
                cfg.cameraTexelsPerFrame = 3.5;
                cfg.cameraTranslationAlign = TranslationAlign::AS_REQUESTED;
                cfg.ssrEnabled = true;
                addScheduledPerfPair(app, cfg);
            }
            // 5. Articulated multi-node rigid animation + motion vectors (Asset 4: BusterDrone.glb)
            {
                RunConfig cfg = makePerfBase("perf_buster_drone_anim", 4, Regime::OBJECT);
                cfg.objectMotion = ObjectMotion::ANIMATION;
                cfg.animation = 0;
                cfg.animationStartTime = 10.0;
                cfg.objectSpeed = 2.0;
                addScheduledPerfPair(app, cfg);
            }
        }
    }
}

// Places camera, animation and TAA settings for one scheduled frame. Runs from preRender, which
// is after FilamentApp2 has pushed the camera manipulator's pose, so these writes win.
void applyShot(App* app, Shot const& shot, bool const updateScene = true) {
    Run const& run = app->runs[shot.runIndex];
    RunConfig const& rc = run.config;
    LoadedAsset const& loaded = app->assets[rc.assetIndex];
    Camera& camera = app->view->getCamera();

    app->view->setViewport({ 0, 0, rc.width, rc.height });
    double const aspect = double(rc.width) / double(std::max<uint32_t>(1u, rc.height));
    camera.setLensProjection(28.0, aspect,
            double(loaded.radius) * 0.05, double(loaded.radius) * 20.0);

    // Frame from the camera's ACTUAL field of view rather than one this function installs.
    // FilamentApp2 re-applies its own lens projection and viewport right after preRender whenever
    // it thinks the window changed, so a projection written here survives some frames and not
    // others -- which silently rendered the reference phase and the capture phase of a run at two
    // different zoom levels. Reading the fov instead is immune to when that reconfiguration lands.
    double const fov = double(camera.getFieldOfViewInDegrees(Camera::Fov::VERTICAL)) * F_PI / 180.0;
    double const distance = double(loaded.radius) / std::sin(0.5 * fov) * 1.15;

    // The motion-free pose: fixed off-axis azimuth (seen end-on, a model presents almost no
    // silhouette, and silhouettes are where everything being measured happens), and the basis
    // TRANSLATE mode dollies along.
    float3 const baseEye = loaded.target + float3{
            float(std::sin(kBaseAzimuthRadians) * distance), float(loaded.radius) * 0.35f,
            float(std::cos(kBaseAzimuthRadians) * distance) };
    float3 const up = { 0.0f, 1.0f, 0.0f };
    float3 const right = normalize(cross(normalize(loaded.target - baseEye), up));

    // texels/frame -> world units/frame at the target's depth, then a straight multiple of the
    // (integer) motion frame count -- see TranslationAlign and motionFrameIndex() for why an
    // index rather than motionTime / stepSeconds. Shared by the camera dolly and the object
    // translation so the two are the same motion seen from the other side.
    auto const translationOffset = [&](double texelsPerFrame) {
        switch (rc.cameraTranslationAlign) {
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
        // Render, not output, resolution: under upscaling TAA resolves at vp/upscaling and this
        // speed is meant to be comparable across upscaling factors, matching the "render texel"
        // convention the rest of the harness (and taa.mat) already uses.
        double const renderHeight = double(vp.height) / double(rc.upscaling);
        double const worldUnitsPerTexel = 2.0 * distance * std::tan(0.5 * fov) / renderHeight;
        return texelsPerFrame * double(shot.motionFrameIndex) * worldUnitsPerTexel;
    };

    float3 eye = baseEye;
    float3 lookAtPoint = loaded.target;
    double animationTime = rc.animationStartTime;
    // Composed onto the root's rest transform below. Identity unless the object regime moves
    // the root, so every other regime (and the reference/warm-up phases) renders the rest pose.
    mat4f objectTransform;
    switch (rc.regime) {
        case Regime::STATIC:
            break;
        case Regime::CAMERA:
            if (rc.cameraMotion == CameraMotion::ROTATE) {
                double const angle = kBaseAzimuthRadians
                        + shot.motionTime * rc.cameraRotateSpeedDegPerSec * (F_PI / 180.0);
                eye = loaded.target + float3{ float(std::sin(angle) * distance),
                        float(loaded.radius) * 0.35f, float(std::cos(angle) * distance) };
            } else {
                double const offset = translationOffset(rc.cameraTexelsPerFrame);
                eye = baseEye + right * float(offset);
                // Translate the look-at point by the same vector: view DIRECTION stays fixed,
                // only position moves, which is what makes this a pure dolly rather than a pan
                // that happens to be phrased as a moving eye.
                lookAtPoint = loaded.target + right * float(offset);
            }
            break;
        case Regime::OBJECT:
            switch (rc.objectMotion) {
                case ObjectMotion::ANIMATION:
                    animationTime = rc.animationStartTime + shot.motionTime * rc.objectSpeed;
                    break;
                case ObjectMotion::TRANSLATE:
                    // The camera dolly moves the eye along `right`; moving the object the same
                    // way gives the same screen-space motion in the opposite direction, which
                    // is irrelevant to every metric and keeps the two modes one line apart.
                    objectTransform = mat4f::translation(
                            right * float(translationOffset(rc.objectTexelsPerFrame)));
                    break;
                case ObjectMotion::SPIN: {
                    float const angle = float(shot.motionTime * rc.objectSpeed * (F_PI / 180.0));
                    objectTransform = mat4f::translation(loaded.target)
                            * mat4f::rotation(angle, up)
                            * mat4f::translation(-loaded.target);
                    break;
                }
            }
            break;
    }
    camera.lookAt(eye, lookAtPoint, up);
    if (!updateScene) {
        return;
    }

    if (loaded.instance) {
        Animator& animator = *loaded.instance->getAnimator();
        if (size_t const count = animator.getAnimationCount(); count > 0) {
            animator.applyAnimation(std::min(size_t(rc.animation), count - 1),
                    float(animationTime));
            animator.updateBoneMatrices();
        }
    }

    // After the Animator, which only writes the nodes the clip targets and so never the root's
    // rest pose this composes onto. Written every shot, including when objectTransform is the
    // identity: setTransform() with an unchanged value is what a static object looks like to the
    // motion vectors pass (a dirty bit and a bitwise-equal transform), so a static run exercises
    // that path rather than a never-touched one.
    {
        TransformManager& tcm = app->engine->getTransformManager();
        tcm.setTransform(tcm.getInstance(loaded.asset->getRoot()),
                objectTransform * loaded.rootRestTransform);
    }

    app->engine->getDebugRegistry().setProperty(
            "d.renderer.disable_subpasses", rc.disableSubpasses);

    app->view->setMultiSampleAntiAliasingOptions({
            .enabled = shot.msaaSamples > 1, .sampleCount = shot.msaaSamples });
    app->view->setScreenSpaceReflectionsOptions({
            .maxDistance = 6.0f, .enabled = rc.ssrEnabled });

    TemporalAntiAliasingOptions taa = app->options.taaOptions;
    taa.jitterPattern = app->options.jitterPattern;
    taa.enabled = shot.taaEnabled;
    taa.upscaling = rc.upscaling;
    app->view->setTemporalAntiAliasingOptions(taa);
    app->view->setMotionVectorsOptions({
            .enabled = rc.motionVectorsEnabled && shot.taaEnabled });

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
        if (shot.destination == Destination::CAPTURE && shot.slot >= 0) {
            Run& mutRun = app->runs[shot.runIndex];
            if (size_t(shot.slot) < mutRun.capturePoses.size()) {
                CapturePose& pose = mutRun.capturePoses[size_t(shot.slot)];
                mat4 const proj = camera.getProjectionMatrix();
                mat4 const viewMat = camera.getViewMatrix();
                pose.viewProj = proj * viewMat;
                pose.invViewProj = inverse(pose.viewProj);
                pose.objectTransform = mat4(objectTransform);
                pose.invObjectTransform = inverse(pose.objectTransform);
                pose.eye = double3(eye);
                pose.target = double3(loaded.target);
                pose.radius = double(loaded.radius);
                pose.distance = distance;
                pose.valid = true;
            }
        }
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

struct BenchmarkSummary {
    float upscaling = 2.0f;
    bool hasQuality = false;
    bool hasPerf = false;
    double spatialQualityDb = 0.0;
    double spatialRmse = 0.0;
    double spatialGmsd = 0.0;
    double detailTransferPct = 0.0;
    double spuriousHfPct = 0.0;
    double spatialHotspotTop10Rmse = 0.0;
    double spatialHotspotPeakRmse = 0.0;
    double temporalStabilityDb = 0.0;
    double temporalFlickerRms = 0.0;
    double subpixelFlickerRms = 0.0;
    double slowDriftFlickerRms = 0.0;
    double motionRobustnessDb = 0.0;
    double motionRobustnessRmse = 0.0;
    double trackedMotionDb = 0.0;
    double trackedMotionRmse = 0.0;
    double unmodeledMotionDb = 0.0;
    double unmodeledMotionRmse = 0.0;
    double motionWakeGhostingRms = 0.0;
    double motionHotspotTop10Rmse = 0.0;
    double motionHotspotPeakRmse = 0.0;
    // 4. GPU Performance (milliseconds, lower is better)
    double perfUpscalerGpuMs = 0.0;
    double perfUpscalerGpuP95Ms = 0.0;
    double perfFrameGpuMs = 0.0;
    double perfFrameGpuP95Ms = 0.0;
    double perfFrameGpuIqrMs = 0.0;
    double perfBaselineGpuMs = 0.0;
    double perfTimingDiscrepancyMs = 0.0;
    bool perfPostCheckPassed = true;
    bool ctsValid = true;
};

std::string escapeJson(std::string const& s) {
    std::string out;
    out.reserve(s.size());
    for (char const c: s) {
        if (c == '"' || c == '\\') {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    return out;
}

void appendMetrics(App* app, Run& run, std::vector<Record>* records) {
    Options const& o = app->options;
    auto emit = [&](std::string const& name, double const value) {
        records->push_back({ run.config.label, run.config.upscaling, name, value });
    };
    auto referenceFor = [&run](size_t const k) -> LinearImage const& {
        return run.references[std::min(k, run.references.size() - 1)];
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
    std::vector<std::vector<uint8_t>> perRefEdgeMasks;
    perRefEdgeMasks.reserve(run.references.size());
    for (size_t r = 0; r < run.references.size(); r++) {
        perRefEdgeMasks.push_back(edgeMask(run.references[r], o.edgeThreshold));
    }
    std::vector<uint8_t> mask = perRefEdgeMasks.front();
    for (size_t r = 1; r < perRefEdgeMasks.size(); r++) {
        for (size_t i = 0; i < mask.size(); i++) {
            mask[i] |= perRefEdgeMasks[r][i];
        }
    }
    size_t const maskedTexels = size_t(std::count(mask.begin(), mask.end(), uint8_t(1)));
    emit("edge_fraction", double(maskedTexels) / double(mask.size()));

    std::vector<uint8_t> fgMask = foregroundMask(referenceFor(0), mask);
    for (size_t r = 1; r < run.references.size(); r++) {
        std::vector<uint8_t> const currentFg = foregroundMask(run.references[r], mask);
        for (size_t i = 0; i < fgMask.size(); i++) {
            fgMask[i] |= currentFg[i];
        }
    }

    double const referenceEnergy = highFrequencyEnergy(referenceFor(0));

    // Per-frame error from motion onset. Frame 0 is the instant the scene changes, so its value
    // is how wrong the converged history was; the rest of the curve is how fast that is cleaned
    // up. Reported whole rather than reduced to one number, because "ghosting" and "how long
    // ghosting lasts" are different questions and conflating them is what prose reports do.
    double errorSum = 0.0;
    double edgeErrorSum = 0.0;
    double gmsdSum = 0.0;
    double hotspotTop10Sum = 0.0;
    double hotspotPeakSum = 0.0;
    double sharpnessSum = 0.0;
    double edgeSharpnessSum = 0.0;
    double detailTransferSum = 0.0;
    double edgeDetailTransferSum = 0.0;
    double spuriousHfSum = 0.0;
    for (size_t k = 0; k < run.captures.size(); k++) {
        LinearImage const& reference = referenceFor(k);
        double const e = rmse(run.captures[k], reference, nullptr);
        double const edgeError = rmse(run.captures[k], reference, &mask);
        double const gmsdVal = computeGmsd(run.captures[k], reference, &fgMask);
        HotspotMetrics const hotspots = computeInnerGridHotspots(run.captures[k], reference, fgMask);
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
        LaplacianComparison const lap = compareLaplacians(run.captures[k], reference, nullptr);
        LaplacianComparison const edgeLap = compareLaplacians(run.captures[k], reference, &mask);
        emit("rmse_frame_" + std::to_string(k), e);
        emit("edge_rmse_frame_" + std::to_string(k), edgeError);
        emit("gmsd_frame_" + std::to_string(k), gmsdVal);
        emit("hotspot_top10_rmse_frame_" + std::to_string(k), hotspots.top10Rmse);
        emit("sharpness_frame_" + std::to_string(k), sharpness);
        emit("edge_sharpness_frame_" + std::to_string(k), edgeSharpness);
        errorSum += e;
        edgeErrorSum += edgeError;
        gmsdSum += gmsdVal;
        hotspotTop10Sum += hotspots.top10Rmse;
        hotspotPeakSum += hotspots.peakRmse;
        sharpnessSum += sharpness;
        edgeSharpnessSum += edgeSharpness;
        detailTransferSum += lap.detailTransfer;
        edgeDetailTransferSum += edgeLap.detailTransfer;
        spuriousHfSum += lap.spuriousHfRms;
    }
    double const n = double(run.captures.size());
    run.summary.rmseMean = errorSum / n;
    run.summary.edgeRmseMean = edgeErrorSum / n;
    run.summary.gmsdMean = gmsdSum / n;
    run.summary.hotspotTop10Rmse = hotspotTop10Sum / n;
    run.summary.hotspotPeakRmse = hotspotPeakSum / n;
    run.summary.sharpnessMean = sharpnessSum / n;
    run.summary.edgeSharpnessMean = edgeSharpnessSum / n;
    run.summary.detailTransferMean = detailTransferSum / n;
    run.summary.edgeDetailTransferMean = edgeDetailTransferSum / n;
    run.summary.spuriousHfMean = spuriousHfSum / n;
    emit("rmse_mean", run.summary.rmseMean);
    emit("edge_rmse_mean", run.summary.edgeRmseMean);
    emit("gmsd_mean", run.summary.gmsdMean);
    emit("hotspot_top10_rmse", run.summary.hotspotTop10Rmse);
    emit("hotspot_peak_rmse", run.summary.hotspotPeakRmse);
    emit("sharpness_mean", run.summary.sharpnessMean);
    emit("edge_sharpness_mean", run.summary.edgeSharpnessMean);
    emit("detail_transfer_mean", run.summary.detailTransferMean);
    emit("edge_detail_transfer_mean", run.summary.edgeDetailTransferMean);
    emit("spurious_hf_mean", run.summary.spuriousHfMean);

    math::double3 const alignment = referenceAlignment(run.captures.front(), referenceFor(0), mask);
    emit("reference_alignment_x", alignment.x);
    emit("reference_alignment_y", alignment.y);
    emit("reference_alignment_gain", alignment.z);
    // Warn on the gain, not on the offset: a large offset that removes almost no error is the
    // search wandering across a flat minimum, and warning about it trains the reader to ignore
    // the warning. Under upscaling, SSR raymarches against the render-resolution depth pyramid
    // rather than output resolution, which introduces a ~0.26 gain on reflected silhouettes.
    double const warnGain = (run.config.ssrEnabled && run.config.upscaling > 1.0f) ? 0.35 : 0.25;
    if (alignment.z > warnGain) {
        fprintf(stderr, "taa_harness: WARNING (%s x%g) -- shifting the reference by "
                        "(%.2f, %.2f) texels removes %.0f%% of its error against the TAA output; "
                        "it is converged on the wrong point and every metric below carries that "
                        "as a floor\n",
                run.config.label.c_str(), double(run.config.upscaling),
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
    run.summary.flickerVariance = temporalVariance(run.captures, nullptr);
    run.summary.edgeFlickerVariance = temporalVariance(run.captures, &mask);
    run.summary.flickerTop1PctRms = topFractionTemporalRms(run.captures, nullptr);
    run.summary.residualFlickerTop1PctRms = topFractionTemporalRms(run.captures, &run.references);
    emit("flicker_variance", run.summary.flickerVariance);
    emit("edge_flicker_variance", run.summary.edgeFlickerVariance);
    emit("flicker_top1pct_rms", run.summary.flickerTop1PctRms);
    emit("residual_flicker_top1pct_rms", run.summary.residualFlickerTop1PctRms);

    if (app->assets[run.config.assetIndex].isSubpixelStar && run.config.regime == Regime::STATIC) {
        SubpixelRingMetrics const rings = subpixelRingFlicker(run.captures);
        run.summary.hasSubpixelRings = true;
        run.summary.subpixelRingFlickerRms = rings.rms;
        emit("subpixel_flicker_0_22px", rings.ringStd[0]);
        emit("subpixel_flicker_0_34px", rings.ringStd[1]);
        emit("subpixel_flicker_0_53px", rings.ringStd[2]);
        emit("subpixel_flicker_0_80px", rings.ringStd[3]);
        emit("subpixel_ring_flicker_rms", rings.rms);
    }

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
    MotionCurveMetrics const motionCurve = computeMotionCurveMetrics(edgeErrors, motionFrames);
    run.summary.edgeGhostPeak = motionCurve.edgeGhostPeak;
    run.summary.edgeTailError = motionCurve.edgeTailError;
    run.summary.edgeEndRmse = motionCurve.edgeEndRmse;
    run.summary.edgeRecoveryFrames = motionCurve.edgeRecoveryFrames;
    emit("edge_ghost_peak", run.summary.edgeGhostPeak);
    emit("edge_tail_error", run.summary.edgeTailError);
    emit("edge_recovery_frames", run.summary.edgeRecoveryFrames);

    if (run.config.regime != Regime::STATIC && run.captures.size() >= 2) {
        double wakeGhostSqSum = 0.0;
        size_t wakeFrameCount = 0;
        size_t const maxMovingFrame = std::min(motionFrames, run.captures.size() - 1);
        bool const isSpinObject = run.config.regime == Regime::OBJECT &&
                run.config.objectMotion == ObjectMotion::SPIN;
        for (size_t k = 1; k <= maxMovingFrame; k++) {
            size_t const refCurrIdx = std::min(k, run.references.size() - 1);
            size_t const refPrevIdx = std::min(k - 1, run.references.size() - 1);
            bool const hasPoses = k < run.capturePoses.size() &&
                    run.capturePoses[k].valid && run.capturePoses[k - 1].valid;
            CapturePose const* poseCurr = hasPoses ? &run.capturePoses[k] : nullptr;
            CapturePose const* posePrev = hasPoses ? &run.capturePoses[k - 1] : nullptr;
            double const wGhost = computeWakeGhostingFrame(
                    false, isSpinObject, poseCurr, posePrev,
                    run.captures[k], run.references[refCurrIdx], run.references[refPrevIdx],
                    perRefEdgeMasks[refCurrIdx], perRefEdgeMasks[refPrevIdx]);
            emit("wake_ghosting_frame_" + std::to_string(k), wGhost);
            wakeGhostSqSum += wGhost * wGhost;
            wakeFrameCount++;
        }
        run.summary.wakeGhostingRms = wakeFrameCount > 0
                ? std::sqrt(wakeGhostSqSum / double(wakeFrameCount))
                : 0.0;
    } else {
        run.summary.wakeGhostingRms = 0.0;
    }
    emit("wake_ghosting_rms", run.summary.wakeGhostingRms);

    if (!o.dumpDirectory.isEmpty()) {
        // One directory per run, holding everything needed to see what a number is describing:
        // what TAA produced, what it was compared against, where they differ, and which texels
        // were counted. The diff is the one a human should look at first -- the numbers say a run
        // got worse, the diff says where.
        char dirName[96];
        snprintf(dirName, sizeof(dirName), "%s_x%g", run.config.label.c_str(),
                double(run.config.upscaling));
        Path const directory = Path(o.dumpDirectory + dirName);
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
    run.capturePoses.clear();
}

void appendPerfMetrics(App* app, Run& run, std::vector<Record>* records) {
    Options const& o = app->options;
    float const nominalUpscaling = run.config.nominalUpscaling;
    auto emit = [&](std::string const& name, double const value) {
        records->push_back({ run.config.label, nominalUpscaling, name, value });
    };

    std::sort(run.timedFrameInfos.begin(), run.timedFrameInfos.end(),
            [](Renderer::FrameInfo const& a, Renderer::FrameInfo const& b) {
                return a.beginFrame < b.beginFrame;
            });

    uint32_t const expectedFrames = std::max<uint32_t>(1u, o.perfFrames);
    if (run.timedWindowGpuDoneNs > run.timedWindowStartNs) {
        run.summary.windowWallFrameMs =
                double(run.timedWindowGpuDoneNs - run.timedWindowStartNs) * 1e-6 /
                double(expectedFrames);
    }

    std::vector<double> gpuTimesMs;
    gpuTimesMs.reserve(run.timedFrameInfos.size());
    std::vector<double> fenceTimesMs;
    fenceTimesMs.reserve(run.timedFrameInfos.size());

    for (size_t i = 0; i < run.timedFrameInfos.size(); i++) {
        Renderer::FrameInfo const& info = run.timedFrameInfos[i];
        if (info.gpuFrameDuration > 0 && info.gpuFrameDuration != Renderer::FrameInfo::INVALID) {
            gpuTimesMs.push_back(double(info.gpuFrameDuration) * 1e-6);
        } else if (info.gpuFrameComplete != Renderer::FrameInfo::INVALID &&
                   info.beginFrame != Renderer::FrameInfo::INVALID &&
                   info.gpuFrameComplete > info.beginFrame) {
            gpuTimesMs.push_back(double(info.gpuFrameComplete - info.beginFrame) * 1e-6);
        }
        if (i > 0) {
            Renderer::FrameInfo const& prev = run.timedFrameInfos[i - 1];
            if (info.gpuFrameComplete != Renderer::FrameInfo::INVALID &&
                    prev.gpuFrameComplete != Renderer::FrameInfo::INVALID &&
                    prev.gpuFrameComplete > 0 &&
                    info.gpuFrameComplete > prev.gpuFrameComplete) {
                fenceTimesMs.push_back(
                        double(info.gpuFrameComplete - prev.gpuFrameComplete) * 1e-6);
            }
        }
    }

    if (gpuTimesMs.empty() && run.summary.windowWallFrameMs > 0.0) {
        gpuTimesMs.assign(expectedFrames, run.summary.windowWallFrameMs);
    }

    computeDistributionStats(gpuTimesMs,
            &run.summary.frameGpuMsP50,
            &run.summary.frameGpuMsP95,
            &run.summary.frameGpuMsIqr,
            &run.summary.frameGpuMsMean,
            &run.summary.frameGpuMsStddev);
    run.summary.timedFramesCount = uint32_t(gpuTimesMs.size());

    if (!fenceTimesMs.empty()) {
        std::sort(fenceTimesMs.begin(), fenceTimesMs.end());
        run.summary.fenceFrameMsP50 = sortedPercentile(fenceTimesMs, 0.50);
    } else {
        run.summary.fenceFrameMsP50 = run.summary.windowWallFrameMs;
    }

    for (size_t k = 0; k < gpuTimesMs.size(); k++) {
        emit("frame_gpu_ms_" + std::to_string(k), gpuTimesMs[k]);
    }
    emit("timed_frames_count", double(run.summary.timedFramesCount));
    emit("frame_gpu_ms_p50", run.summary.frameGpuMsP50);
    emit("frame_gpu_ms_p95", run.summary.frameGpuMsP95);
    emit("frame_gpu_ms_iqr", run.summary.frameGpuMsIqr);
    emit("frame_gpu_ms_mean", run.summary.frameGpuMsMean);
    emit("frame_gpu_ms_stddev", run.summary.frameGpuMsStddev);
    emit("fence_frame_ms_p50", run.summary.fenceFrameMsP50);
    emit("window_wall_frame_ms", run.summary.windowWallFrameMs);

    if (run.config.kind == RunKind::PERF_BASELINE) {
        return;
    }

    // Locate the paired render-resolution baseline run to compute the net upscaler GPU cost.
    for (Run const& candidate : app->runs) {
        if (candidate.config.kind == RunKind::PERF_BASELINE &&
                candidate.config.label == run.config.pairedBaselineLabel &&
                std::abs(candidate.config.nominalUpscaling - nominalUpscaling) < 1e-4f) {
            run.summary.baselineGpuMsP50 = candidate.summary.frameGpuMsP50;
            run.summary.baselineGpuMsMean = candidate.summary.frameGpuMsMean;
            run.summary.fenceBaselineMsP50 = candidate.summary.fenceFrameMsP50;
            run.summary.windowWallBaselineMs = candidate.summary.windowWallFrameMs;
            break;
        }
    }

    run.summary.upscalerGpuMsP50 = run.summary.frameGpuMsP50 - run.summary.baselineGpuMsP50;
    run.summary.upscalerGpuMsP95 = run.summary.frameGpuMsP95 - run.summary.baselineGpuMsP50;
    run.summary.upscalerGpuMsMean = run.summary.frameGpuMsMean - run.summary.baselineGpuMsMean;
    run.summary.fenceUpscalerMsP50 = run.summary.fenceFrameMsP50 - run.summary.fenceBaselineMsP50;
    run.summary.windowWallUpscalerMs =
            run.summary.windowWallFrameMs - run.summary.windowWallBaselineMs;

    // Cross-validation against timestamp spoofing:
    // 1. Under-reporting the upscaler's GPU cost causes wall-clock delta to exceed timer-query
    //    delta when GPU-bound (`windowWallUpscalerMs - upscalerGpuMsP50`).
    // 2. Over-reporting the baseline or upscaler timer query above total wall-clock frame time is
    //    physically impossible across a flushed N-frame window.
    double const underReportDiscrepancy =
            std::max(0.0, run.summary.windowWallUpscalerMs - run.summary.upscalerGpuMsP50);
    double const overReportBaselineDiscrepancy =
            std::max(0.0, run.summary.baselineGpuMsP50 - run.summary.windowWallBaselineMs);
    double const overReportUpscalerDiscrepancy =
            std::max(0.0, run.summary.frameGpuMsP50 - run.summary.windowWallFrameMs);
    run.summary.timingDiscrepancyMs = std::max({
            underReportDiscrepancy,
            overReportBaselineDiscrepancy,
            overReportUpscalerDiscrepancy });

    // Post-window output verification: verify that the upscaled output captured on drain frame 0
    // is non-empty, finite, non-black, and structurally consistent with the native aliased render
    // captured at the identical final pose on drain frame 1.
    bool postCheckOk = false;
    if (!run.captures.empty() &&
            run.captures[0].getWidth() == run.config.width &&
            run.captures[0].getHeight() == run.config.height &&
            run.aliasedReference.getWidth() == run.config.width &&
            run.aliasedReference.getHeight() == run.config.height) {
        LinearImage const& cap = run.captures[0];
        float const* px = cap.getPixelRef();
        size_t const pixelCount = size_t(cap.getWidth()) * cap.getHeight();
        double lumaSum = 0.0;
        bool hasInvalid = false;
        for (size_t i = 0; i < pixelCount; i++) {
            float const r = px[i * 3 + 0];
            float const g = px[i * 3 + 1];
            float const b = px[i * 3 + 2];
            if (!std::isfinite(r) || !std::isfinite(g) || !std::isfinite(b)) {
                hasInvalid = true;
                break;
            }
            lumaSum += double(luma(&px[i * 3]));
        }
        run.summary.postCheckLumaMean = pixelCount > 0 ? lumaSum / double(pixelCount) : 0.0;
        run.summary.postCheckAliasedRmse = rmse(cap, run.aliasedReference, nullptr);
        postCheckOk = !hasInvalid &&
                run.summary.postCheckLumaMean > 1e-4 &&
                run.summary.postCheckAliasedRmse < 0.25;
    }
    run.summary.postCheckPassed = postCheckOk;

    emit("baseline_gpu_ms_p50", run.summary.baselineGpuMsP50);
    emit("baseline_gpu_ms_mean", run.summary.baselineGpuMsMean);
    emit("upscaler_gpu_ms_p50", run.summary.upscalerGpuMsP50);
    emit("upscaler_gpu_ms_p95", run.summary.upscalerGpuMsP95);
    emit("upscaler_gpu_ms_mean", run.summary.upscalerGpuMsMean);
    emit("fence_baseline_ms_p50", run.summary.fenceBaselineMsP50);
    emit("fence_upscaler_ms_p50", run.summary.fenceUpscalerMsP50);
    emit("window_wall_baseline_ms", run.summary.windowWallBaselineMs);
    emit("window_wall_upscaler_ms", run.summary.windowWallUpscalerMs);
    emit("timing_discrepancy_ms", run.summary.timingDiscrepancyMs);
    emit("post_check_passed", run.summary.postCheckPassed ? 1.0 : 0.0);
    emit("post_check_aliased_rmse", run.summary.postCheckAliasedRmse);
    emit("post_check_luma_mean", run.summary.postCheckLumaMean);

    if (!o.dumpDirectory.isEmpty() && !run.captures.empty() && run.captures[0].getWidth() != 0) {
        char dirName[96];
        snprintf(dirName, sizeof(dirName), "%s_x%g", run.config.label.c_str(),
                double(nominalUpscaling));
        Path const directory = Path(o.dumpDirectory + dirName);
        directory.mkdirRecursive();
        writePng(run.captures[0], Path(directory + "perf_capture.png"));
        if (run.aliasedReference.getWidth() != 0) {
            writePng(run.aliasedReference, Path(directory + "perf_aliased_reference.png"));
            writePng(differenceImage(run.captures[0], run.aliasedReference, o.diffGain),
                    Path(directory + "perf_aliased_diff.png"));
        }
    }
    run.captures.clear();
}

std::vector<BenchmarkSummary> computeBenchmarkSummaries(App const* app,
        std::vector<Record>* records) {
    std::vector<BenchmarkSummary> summaries;
    if (!app->options.benchmark && !app->options.perf) {
        return summaries;
    }

    for (float const upscaling: app->options.upscalings) {
        auto findQualityRun = [&](char const* label) -> RunMetrics const* {
            for (Run const& run: app->runs) {
                if (run.config.kind == RunKind::QUALITY &&
                        run.config.label == label &&
                        std::abs(run.config.upscaling - upscaling) < 1e-4f) {
                    return &run.summary;
                }
            }
            return nullptr;
        };

        BenchmarkSummary s;
        s.upscaling = upscaling;

        auto emitBench = [&](char const* metric, double const value) {
            records->push_back({ "benchmark", upscaling, metric, value });
        };

        if (app->options.benchmark) {
            s.hasQuality = true;

            // 1. Spatial Quality: static converged L2 accuracy + Prewitt GMSD on real content
            // (`fox_static`, `helmet_static`) + phase-correlated Laplacian detail across all
            // static scenes. (`star_static` is excluded from raw L2 MSE because its sub-texel
            // Siemens star core lies past the 2x-upscaling Nyquist limit, where a 0.5 grey blur
            // beats sharp reconstruction in L2 by 2x, whereas phase-correlated `detailTransferMean`
            // cancels out Moire cleanly.)
            char const* const kSpatialMseRuns[] = { "fox_static", "helmet_static" };
            double spatialMseSum = 0.0;
            double gmsdSum = 0.0;
            double spatialHotspotTop10SqSum = 0.0;
            double spatialHotspotPeakMax = 0.0;
            int spatialMseCount = 0;
            for (char const* name: kSpatialMseRuns) {
                if (RunMetrics const* m = findQualityRun(name)) {
                    spatialMseSum += 0.40 * m->edgeRmseMean * m->edgeRmseMean
                            + 0.35 * m->rmseMean * m->rmseMean
                            + 0.25 * m->gmsdMean * m->gmsdMean;
                    gmsdSum += m->gmsdMean;
                    spatialHotspotTop10SqSum += m->hotspotTop10Rmse * m->hotspotTop10Rmse;
                    spatialHotspotPeakMax = std::max(spatialHotspotPeakMax, m->hotspotPeakRmse);
                    spatialMseCount++;
                }
            }
            if (spatialMseCount > 0) {
                double const mse = spatialMseSum / double(spatialMseCount);
                s.spatialRmse = std::sqrt(mse);
                s.spatialQualityDb = mseToDb(mse);
                s.spatialGmsd = gmsdSum / double(spatialMseCount);
                s.spatialHotspotTop10Rmse =
                        std::sqrt(spatialHotspotTop10SqSum / double(spatialMseCount));
                s.spatialHotspotPeakRmse = spatialHotspotPeakMax;
            }

            char const* const kStaticDetailRuns[] = {
                "fox_static", "helmet_static", "star_static"
            };
            double detailSum = 0.0;
            double spuriousSum = 0.0;
            int detailCount = 0;
            for (char const* name: kStaticDetailRuns) {
                if (RunMetrics const* m = findQualityRun(name)) {
                    detailSum += m->detailTransferMean;
                    spuriousSum += m->spuriousHfMean;
                    detailCount++;
                }
            }
            if (detailCount > 0) {
                s.detailTransferPct = 100.0 * detailSum / double(detailCount);
                s.spuriousHfPct = 100.0 * spuriousSum / double(detailCount);
            }

            // 2. Temporal Stability: top-1% temporal shimmer on real scenes + dedicated sub-pixel
            // ring flicker (`0.22-0.80 px`) on `SubpixelStar` + slow `0.1` tx/frame camera drift.
            char const* const kStabilityRuns[] = {
                "fox_static", "helmet_static", "star_static", "fox_drift"
            };
            double stabilityVarSum = 0.0;
            int stabilityCount = 0;
            for (char const* name: kStabilityRuns) {
                if (RunMetrics const* m = findQualityRun(name)) {
                    double const rms = m->hasSubpixelRings ? m->subpixelRingFlickerRms
                                                           : m->residualFlickerTop1PctRms;
                    if (m->hasSubpixelRings) {
                        s.subpixelFlickerRms = m->subpixelRingFlickerRms;
                    }
                    stabilityVarSum += rms * rms;
                    stabilityCount++;
                }
            }
            if (RunMetrics const* drift = findQualityRun("fox_drift")) {
                s.slowDriftFlickerRms = drift->residualFlickerTop1PctRms;
            }
            if (stabilityCount > 0) {
                double const var = stabilityVarSum / double(stabilityCount);
                s.temporalFlickerRms = std::sqrt(var);
                s.temporalStabilityDb = mseToDb(var);
            }

            // 3. Motion Robustness: dynamic edge RMSE + transient ghost peak + unrecovered tail
            // error + analytical backward-motion wake ghosting RMS
            auto motionMse = [](RunMetrics const& m) {
                double const peak = std::max(0.0, m.edgeGhostPeak);
                double const tail = std::max(0.0, m.edgeTailError);
                double const wake = std::max(0.0, m.wakeGhostingRms);
                return 0.40 * m.edgeRmseMean * m.edgeRmseMean
                        + 0.30 * peak * peak
                        + 0.15 * tail * tail
                        + 0.15 * wake * wake;
            };

            char const* const kTrackedRuns[] = {
                "helmet_cam_rotate",
                "helmet_cam_dolly",
                "fox_obj_translate",
                "helmet_obj_spin",
                "buster_drone_anim"
            };
            double trackedSum = 0.0;
            double wakeSqSum = 0.0;
            double motionHotspotTop10SqSum = 0.0;
            double motionHotspotPeakMax = 0.0;
            int trackedCount = 0;
            for (char const* name: kTrackedRuns) {
                if (RunMetrics const* m = findQualityRun(name)) {
                    trackedSum += motionMse(*m);
                    wakeSqSum += m->wakeGhostingRms * m->wakeGhostingRms;
                    motionHotspotTop10SqSum += m->hotspotTop10Rmse * m->hotspotTop10Rmse;
                    motionHotspotPeakMax = std::max(motionHotspotPeakMax, m->hotspotPeakRmse);
                    trackedCount++;
                }
            }
            double const trackedMse = trackedCount > 0 ? trackedSum / double(trackedCount) : 0.0;
            s.trackedMotionRmse = std::sqrt(trackedMse);
            s.trackedMotionDb = mseToDb(trackedMse);

            char const* const kUnmodeledRuns[] = { "fox_skinned_run", "ssr_reflection" };
            double unmodeledSum = 0.0;
            int unmodeledCount = 0;
            for (char const* name: kUnmodeledRuns) {
                if (RunMetrics const* m = findQualityRun(name)) {
                    unmodeledSum += motionMse(*m);
                    wakeSqSum += m->wakeGhostingRms * m->wakeGhostingRms;
                    motionHotspotTop10SqSum += m->hotspotTop10Rmse * m->hotspotTop10Rmse;
                    motionHotspotPeakMax = std::max(motionHotspotPeakMax, m->hotspotPeakRmse);
                    unmodeledCount++;
                }
            }
            double const unmodeledMse =
                    unmodeledCount > 0 ? unmodeledSum / double(unmodeledCount) : 0.0;
            s.unmodeledMotionRmse = std::sqrt(unmodeledMse);
            s.unmodeledMotionDb = mseToDb(unmodeledMse);

            int const totalMotion = trackedCount + unmodeledCount;
            if (totalMotion > 0) {
                double const combined = (trackedSum + unmodeledSum) / double(totalMotion);
                s.motionRobustnessRmse = std::sqrt(combined);
                s.motionRobustnessDb = mseToDb(combined);
                s.motionWakeGhostingRms = std::sqrt(wakeSqSum / double(totalMotion));
                s.motionHotspotTop10Rmse = std::sqrt(motionHotspotTop10SqSum / double(totalMotion));
                s.motionHotspotPeakRmse = motionHotspotPeakMax;
            }

            emitBench("spatial_quality_db", s.spatialQualityDb);
            emitBench("spatial_rmse", s.spatialRmse);
            emitBench("spatial_gmsd", s.spatialGmsd);
            emitBench("spatial_detail_transfer_pct", s.detailTransferPct);
            emitBench("spatial_spurious_hf_pct", s.spuriousHfPct);
            emitBench("spatial_hotspot_top10_rmse", s.spatialHotspotTop10Rmse);
            emitBench("spatial_hotspot_peak_rmse", s.spatialHotspotPeakRmse);
            emitBench("temporal_stability_db", s.temporalStabilityDb);
            emitBench("temporal_flicker_rms", s.temporalFlickerRms);
            emitBench("temporal_subpixel_flicker_rms", s.subpixelFlickerRms);
            emitBench("temporal_drift_flicker_rms", s.slowDriftFlickerRms);
            emitBench("motion_robustness_db", s.motionRobustnessDb);
            emitBench("motion_robustness_rmse", s.motionRobustnessRmse);
            emitBench("motion_wake_ghosting_rms", s.motionWakeGhostingRms);
            emitBench("motion_hotspot_top10_rmse", s.motionHotspotTop10Rmse);
            emitBench("motion_hotspot_peak_rmse", s.motionHotspotPeakRmse);
            emitBench("motion_tracked_db", s.trackedMotionDb);
            emitBench("motion_tracked_rmse", s.trackedMotionRmse);
            emitBench("motion_unmodeled_db", s.unmodeledMotionDb);
            emitBench("motion_unmodeled_rmse", s.unmodeledMotionRmse);
        }

        if (app->options.perf) {
            double upscalerP50Sum = 0.0;
            double upscalerP95Sum = 0.0;
            double frameP50Sum = 0.0;
            double frameP95Sum = 0.0;
            double frameIqrSum = 0.0;
            double baselineP50Sum = 0.0;
            double maxDiscrepancy = 0.0;
            bool allPostChecksPassed = true;
            int perfCount = 0;

            for (Run const& run : app->runs) {
                if (run.config.kind == RunKind::PERF_UPSCALER &&
                        std::abs(run.config.nominalUpscaling - upscaling) < 1e-4f) {
                    RunMetrics const& m = run.summary;
                    upscalerP50Sum += m.upscalerGpuMsP50;
                    upscalerP95Sum += m.upscalerGpuMsP95;
                    frameP50Sum += m.frameGpuMsP50;
                    frameP95Sum += m.frameGpuMsP95;
                    frameIqrSum += m.frameGpuMsIqr;
                    baselineP50Sum += m.baselineGpuMsP50;
                    maxDiscrepancy = std::max(maxDiscrepancy, m.timingDiscrepancyMs);
                    allPostChecksPassed = allPostChecksPassed && m.postCheckPassed;
                    perfCount++;
                }
            }

            if (perfCount > 0) {
                s.hasPerf = true;
                double const inv = 1.0 / double(perfCount);
                s.perfUpscalerGpuMs = upscalerP50Sum * inv;
                s.perfUpscalerGpuP95Ms = upscalerP95Sum * inv;
                s.perfFrameGpuMs = frameP50Sum * inv;
                s.perfFrameGpuP95Ms = frameP95Sum * inv;
                s.perfFrameGpuIqrMs = frameIqrSum * inv;
                s.perfBaselineGpuMs = baselineP50Sum * inv;
                s.perfTimingDiscrepancyMs = maxDiscrepancy;
                s.perfPostCheckPassed = allPostChecksPassed;

                double const discrepancyLimit =
                        std::max(0.5, 0.25 * std::max(0.0, s.perfFrameGpuMs));
                bool const timingValid =
                        s.perfFrameGpuMs > 0.0 && s.perfTimingDiscrepancyMs <= discrepancyLimit;
                bool const thermalValid = app->options.thermalStatusEnd <= 1;
#if defined(__ANDROID__)
                bool const fixedPerfValid = app->options.fixedPerformanceMode;
#else
                bool const fixedPerfValid = true;
#endif
                s.ctsValid = s.perfPostCheckPassed && timingValid && thermalValid && fixedPerfValid;

                emitBench("perf_upscaler_gpu_ms", s.perfUpscalerGpuMs);
                emitBench("perf_upscaler_gpu_p95_ms", s.perfUpscalerGpuP95Ms);
                emitBench("perf_frame_gpu_ms", s.perfFrameGpuMs);
                emitBench("perf_frame_gpu_p95_ms", s.perfFrameGpuP95Ms);
                emitBench("perf_frame_gpu_iqr_ms", s.perfFrameGpuIqrMs);
                emitBench("perf_baseline_gpu_ms", s.perfBaselineGpuMs);
                emitBench("perf_timing_discrepancy_ms", s.perfTimingDiscrepancyMs);
                emitBench("perf_post_check_passed", s.perfPostCheckPassed ? 1.0 : 0.0);
                emitBench("perf_cts_valid", s.ctsValid ? 1.0 : 0.0);
            }
        }

        summaries.push_back(s);
    }
    return summaries;
}

void printBenchmarkScorecard(App const* app, std::vector<BenchmarkSummary> const& summaries) {
    Options const& o = app->options;
    char const* refName = referenceFilterName(o.referenceFilter);
    for (BenchmarkSummary const& s: summaries) {
        printf("\n");
        printf("================================================================================"
               "================\n");
        printf("  Upscaler Qualification Benchmark  --  %s (x%g, ref: %s)\n",
                o.label.c_str(), double(s.upscaling), refName);
        printf("================================================================================"
               "================\n");
        if (s.hasQuality) {
            printf("  1. Spatial Quality     : %6.2f dB   "
                   "(RMSE: %.4f | GMSD: %.4f | Detail: %5.1f%% | Ringing: %4.1f%%)\n",
                    s.spatialQualityDb, s.spatialRmse, s.spatialGmsd, s.detailTransferPct,
                    s.spuriousHfPct);
            printf("       - Spatial Hotspots:             "
                   "(8x8 Core Top-10 P95: %.4f | Peak Cell P95: %.4f)\n",
                    s.spatialHotspotTop10Rmse, s.spatialHotspotPeakRmse);
            printf("  2. Temporal Stability  : %6.2f dB   "
                   "(Top-1%% RMS: %.4f | Star: %.4f | Drift: %.4f)\n",
                    s.temporalStabilityDb, s.temporalFlickerRms,
                    s.subpixelFlickerRms, s.slowDriftFlickerRms);
            printf("  3. Motion Robustness   : %6.2f dB   "
                   "(Dynamic Edge RMSE: %.4f | Wake Ghosting RMS: %.4f)\n",
                    s.motionRobustnessDb, s.motionRobustnessRmse, s.motionWakeGhostingRms);
            printf("       - Tracked Motion  : %6.2f dB   "
                   "(Camera + Rigid MVs, RMSE: %.4f)\n",
                    s.trackedMotionDb, s.trackedMotionRmse);
            printf("       - Unmodeled Motion: %6.2f dB   "
                   "(Skinned + SSR,      RMSE: %.4f)\n",
                    s.unmodeledMotionDb, s.unmodeledMotionRmse);
            printf("       - Motion Hotspots :             "
                   "(8x8 Core Top-10 P95: %.4f | Peak Cell P95: %.4f)\n",
                    s.motionHotspotTop10Rmse, s.motionHotspotPeakRmse);
        }
        if (s.hasPerf) {
            printf("  4. GPU Performance     : %6.2f ms   "
                   "(SR Cost p50 | p95: %.2f ms | IQR: %.2f ms)\n",
                    s.perfUpscalerGpuMs, s.perfUpscalerGpuP95Ms, s.perfFrameGpuIqrMs);
            printf("       - Full Frame GPU  : %6.2f ms   "
                   "(%ux%u p50 | Baseline: %.2f ms)\n",
                    s.perfFrameGpuMs, o.perfWidth, o.perfHeight, s.perfBaselineGpuMs);
            printf("       - CTS Integrity   :   %-6s    "
                   "(FixedPerf: %s | Thermal: %s | PostCheck: %s)\n",
                    s.ctsValid ? "PASS" : "FAIL",
                    o.fixedPerformanceMode ? "ON" : "OFF",
                    thermalStatusName(o.thermalStatusEnd),
                    s.perfPostCheckPassed ? "PASS" : "FAIL");
        }
        if (s.hasQuality) {
            printf("----------------------------------------------------------------------------"
                   "--------------------\n");
            printf("  %-18s %9s %8s %9s %9s %11s %10s %10s\n",
                    "Scenario", "Edge RMSE", "GMSD", "Hotspot10", "Detail %", "Flicker RMS",
                    "Ghost Peak", "Wake RMS");
            for (Run const& run: app->runs) {
                if (run.config.kind != RunKind::QUALITY ||
                        std::abs(run.config.upscaling - s.upscaling) >= 1e-4f) {
                    continue;
                }
                RunMetrics const& m = run.summary;
                bool const isStatic = run.config.regime == Regime::STATIC;
                bool const isDrift = run.config.label == "fox_drift";
                char flickerBuf[24] = "-";
                if (isStatic || isDrift) {
                    snprintf(flickerBuf, sizeof(flickerBuf), "%.4f",
                            m.hasSubpixelRings ? m.subpixelRingFlickerRms
                                               : m.residualFlickerTop1PctRms);
                }
                char ghostBuf[24] = "-";
                char wakeBuf[24] = "-";
                if (!isStatic) {
                    snprintf(ghostBuf, sizeof(ghostBuf), "%.4f", m.edgeGhostPeak);
                    snprintf(wakeBuf, sizeof(wakeBuf), "%.4f", m.wakeGhostingRms);
                }
                printf("  %-18s %9.4f %8.4f %9.4f %8.1f%% %11s %10s %10s\n",
                        run.config.label.c_str(), m.edgeRmseMean, m.gmsdMean, m.hotspotTop10Rmse,
                        m.detailTransferMean * 100.0, flickerBuf, ghostBuf, wakeBuf);
            }
        }
        if (s.hasPerf) {
            printf("----------------------------------------------------------------------------"
                   "--------------------\n");
            printf("  %-22s %11s %11s %11s %11s %9s %9s\n",
                    "Perf Scenario", "SR p50 ms", "SR p95 ms", "Frame p50", "Base p50", "IQR ms",
                    "PostCheck");
            for (Run const& run : app->runs) {
                if (run.config.kind != RunKind::PERF_UPSCALER ||
                        std::abs(run.config.nominalUpscaling - s.upscaling) >= 1e-4f) {
                    continue;
                }
                RunMetrics const& m = run.summary;
                printf("  %-22s %11.3f %11.3f %11.3f %11.3f %9.3f %9s\n",
                        run.config.label.c_str(),
                        m.upscalerGpuMsP50,
                        m.upscalerGpuMsP95,
                        m.frameGpuMsP50,
                        m.baselineGpuMsP50,
                        m.frameGpuMsIqr,
                        m.postCheckPassed ? "PASS" : "FAIL");
            }
        }
        printf("================================================================================"
               "================\n");
        if (s.hasQuality && s.hasPerf) {
            printf("  (Higher dB is better on quality axes 1-3; lower ms is better on GPU "
                   "performance)\n\n");
        } else if (s.hasPerf) {
            printf("  (Lower ms is better on GPU performance; SR Cost = Full Frame GPU - "
                   "Render Baseline)\n\n");
        } else {
            printf("  (Higher dB is better on all 3 axes: +3 dB = 2x less MSE, +6 dB = 2x lower "
                   "RMSE)\n\n");
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Normative Protocol Version & SHA-256 Schedule Signature
// ------------------------------------------------------------------------------------------------

char const* jitterPatternName(TemporalAntiAliasingOptions::JitterPattern const pattern) {
    using JP = TemporalAntiAliasingOptions::JitterPattern;
    switch (pattern) {
        case JP::RGSS_X4: return "rgss_x4";
        case JP::UNIFORM_HELIX_X4: return "uniform_helix_x4";
        case JP::HALTON_23_X8: return "halton_23_x8";
        case JP::HALTON_23_X16: return "halton_23_x16";
        case JP::HALTON_23_X32: return "halton_23_x32";
    }
    return "halton_23_x16";
}

std::string formatUpscalingsList(std::vector<float> const& upscalings) {
    std::ostringstream oss;
    for (size_t i = 0; i < upscalings.size(); i++) {
        if (i > 0) {
            oss << ",";
        }
        char buf[32];
        snprintf(buf, sizeof(buf), "%.4f", double(upscalings[i]));
        oss << buf;
    }
    return oss.str();
}

std::string formatScenarioLabels(std::vector<Run> const& runs, RunKind const kind) {
    std::vector<std::string> seen;
    std::ostringstream oss;
    for (Run const& run: runs) {
        if (run.config.kind != kind) {
            continue;
        }
        if (std::find(seen.begin(), seen.end(), run.config.label) != seen.end()) {
            continue;
        }
        if (!seen.empty()) {
            oss << ",";
        }
        seen.push_back(run.config.label);
        oss << run.config.label;
    }
    return oss.str();
}

std::string buildQualityScheduleDescriptor(App const* app) {
    Options const& o = app->options;
    char buf[1024];
    std::string const ups = formatUpscalingsList(o.upscalings);
    std::string const scenarios = formatScenarioLabels(app->runs, RunKind::QUALITY);
    int written = snprintf(buf, sizeof(buf),
            "protocol=%s;mode=%s;scenarios=%s;size=%u;warmup_frames=%u;capture_frames=%u;"
            "reference_filter=%s;reference_samples=%u;jitter_pattern=%s;"
            "edge_threshold=%.4f;upscalings=%s",
            kBenchmarkProtocolVersion,
            o.benchmark ? "benchmark_suite" : "custom",
            scenarios.c_str(),
            o.size,
            o.warmupFrames,
            o.captureFrames,
            referenceFilterName(o.referenceFilter),
            o.referenceSamples,
            jitterPatternName(o.taaOptions.jitterPattern),
            double(o.edgeThreshold),
            ups.c_str());
    std::string out(buf, written > 0 ? size_t(std::min<int>(written, int(sizeof(buf) - 1))) : 0u);
    if (!o.benchmark && std::abs(o.animationStartTime) > 1e-6) {
        char startBuf[64];
        snprintf(startBuf, sizeof(startBuf), ";animation_start=%.4f", o.animationStartTime);
        out += startBuf;
    }
    return out;
}

std::string buildPerfScheduleDescriptor(App const* app) {
    Options const& o = app->options;
    char buf[1024];
    std::string const ups = formatUpscalingsList(o.upscalings);
    std::string const scenarios = formatScenarioLabels(app->runs, RunKind::PERF_UPSCALER);
    snprintf(buf, sizeof(buf),
            "protocol=%s;scenarios=%s;perf_width=%u;perf_height=%u;perf_warmup_frames=%u;"
            "perf_frames=%u;upscalings=%s",
            kBenchmarkProtocolVersion,
            scenarios.c_str(),
            o.perfWidth,
            o.perfHeight,
            o.perfWarmupFrames,
            o.perfFrames,
            ups.c_str());
    return std::string(buf);
}

std::string computeDumpDirForJson(Path const& outputPath, Path const& dumpDirectory) {
    if (dumpDirectory.isEmpty()) {
        return {};
    }
    Path const absOutDir = outputPath.getAbsolutePath().getParent();
    Path const absDump = dumpDirectory.getAbsolutePath();
    std::string const outDirStr = absOutDir.getPath();
    std::string const dumpStr = absDump.getPath();
    if (dumpStr == outDirStr) {
        return ".";
    }
    if (absOutDir.isAncestorOf(absDump) && dumpStr.size() > outDirStr.size()) {
        size_t const offset = outDirStr.back() == '/' ? outDirStr.size() : outDirStr.size() + 1u;
        if (offset < dumpStr.size()) {
            return dumpStr.substr(offset);
        }
    }
    Path const outGrandParent = absOutDir.getParent();
    if (!outGrandParent.isEmpty() && outGrandParent.getPath() == absDump.getParent().getPath() &&
            !absDump.getName().empty()) {
        return std::string("../") + absDump.getName();
    }
    return dumpDirectory.getPath();
}

std::string serializeBenchmarkJson(App* app, std::vector<Record> const& records,
        std::vector<BenchmarkSummary> const& summaries, std::string const& dumpDirField) {
    Options const& o = app->options;
    bool const hasQualitySchedule = o.benchmark || !o.perf;
    bool const hasPerfSchedule = o.perf;
    std::string const qualityDesc =
            hasQualitySchedule ? buildQualityScheduleDescriptor(app) : std::string();
    std::string const qualitySig =
            hasQualitySchedule ? sha256Hex(qualityDesc) : std::string();
    std::string const perfDesc =
            hasPerfSchedule ? buildPerfScheduleDescriptor(app) : std::string();
    std::string const perfSig =
            hasPerfSchedule ? sha256Hex(perfDesc) : std::string();
    std::string const combinedDesc = (hasQualitySchedule && hasPerfSchedule)
            ? (qualityDesc + "|" + perfDesc)
            : (hasQualitySchedule ? qualityDesc : perfDesc);
    std::string const combinedSig = sha256Hex(combinedDesc);

    std::ostringstream json;
    json << "{\n";
    json << "  \"protocol_version\": \"" << kBenchmarkProtocolVersion << "\",\n";
    json << "  \"schedule_signature\": \"" << combinedSig << "\",\n";
    json << "  \"schedule_descriptor\": \"" << escapeJson(combinedDesc) << "\",\n";
    if (hasQualitySchedule) {
        json << "  \"quality_schedule_signature\": \"" << qualitySig << "\",\n";
        json << "  \"quality_schedule_descriptor\": \"" << escapeJson(qualityDesc) << "\",\n";
    }
    if (hasPerfSchedule) {
        json << "  \"perf_schedule_signature\": \"" << perfSig << "\",\n";
        json << "  \"perf_schedule_descriptor\": \"" << escapeJson(perfDesc) << "\",\n";
    }
    if (!o.label.empty()) {
        json << "  \"label\": \"" << escapeJson(o.label) << "\",\n";
    }
    if (!dumpDirField.empty()) {
        json << "  \"dump_directory\": \"" << escapeJson(dumpDirField) << "\",\n";
    }
    json << "  \"benchmark\": " << (o.benchmark ? "true" : "false") << ",\n";
    json << "  \"perf\": " << (o.perf ? "true" : "false") << ",\n";
    if (o.perf) {
        json << "  \"perf_width\": " << o.perfWidth << ",\n";
        json << "  \"perf_height\": " << o.perfHeight << ",\n";
        json << "  \"perf_warmup_frames\": " << o.perfWarmupFrames << ",\n";
        json << "  \"perf_frames\": " << o.perfFrames << ",\n";
        json << "  \"fixed_performance_mode\": " << (o.fixedPerformanceMode ? "true" : "false")
             << ",\n";
        json << "  \"thermal_status_start\": \"" << thermalStatusName(o.thermalStatusStart)
             << "\",\n";
        json << "  \"thermal_status_end\": \"" << thermalStatusName(o.thermalStatusEnd) << "\",\n";
    }
    bool const isSuite = o.benchmark || (o.perf && o.model.isEmpty());
    json << "  \"model\": \"" << escapeJson(isSuite ? "benchmark_suite" : o.model.c_str())
         << "\",\n";
    json << "  \"backend\": \"" << int(app->engine->getBackend()) << "\",\n";
    json << "  \"size\": " << o.size << ",\n";
    json << "  \"warmup_frames\": " << o.warmupFrames << ",\n";
    json << "  \"capture_frames\": " << o.captureFrames << ",\n";
    json << "  \"reference_filter\": \"" << referenceFilterName(o.referenceFilter) << "\",\n";
    json << "  \"reference_samples\": " << o.referenceSamples << ",\n";
    json << "  \"msaa\": " << int(o.msaaSamples) << ",\n";
    if (!isSuite) {
        json << "  \"camera_motion\": \""
             << (o.cameraMotion == CameraMotion::ROTATE ? "rotate" : "translate") << "\",\n";
        if (o.cameraMotion == CameraMotion::TRANSLATE) {
            json << "  \"camera_texels_per_frame\": " << o.cameraTexelsPerFrame << ",\n";
            json << "  \"camera_translation_align\": \""
                 << (o.cameraTranslationAlign == TranslationAlign::PIXEL ? "pixel"
                            : o.cameraTranslationAlign == TranslationAlign::SUBPIXEL
                                    ? "subpixel" : "as-requested")
                 << "\",\n";
        } else {
            json << "  \"camera_speed_deg_per_sec\": " << o.cameraRotateSpeedDegPerSec << ",\n";
        }
        json << "  \"object_motion\": \""
             << (o.objectMotion == ObjectMotion::ANIMATION ? "animation"
                        : o.objectMotion == ObjectMotion::TRANSLATE ? "translate" : "spin")
             << "\",\n";
        switch (o.objectMotion) {
            case ObjectMotion::ANIMATION:
                json << "  \"animation\": " << o.animation << ",\n";
                if (std::abs(o.animationStartTime) > 1e-6) {
                    json << "  \"animation_start\": " << o.animationStartTime << ",\n";
                }
                json << "  \"object_speed\": " << o.objectSpeed << ",\n";
                break;
            case ObjectMotion::TRANSLATE:
                json << "  \"object_texels_per_frame\": " << o.objectTexelsPerFrame << ",\n";
                break;
            case ObjectMotion::SPIN:
                json << "  \"object_speed_deg_per_sec\": " << o.objectSpeed << ",\n";
                break;
        }
    }
    if (!summaries.empty()) {
        json << "  \"benchmark_summary\": [\n";
        for (size_t i = 0; i < summaries.size(); i++) {
            BenchmarkSummary const& s = summaries[i];
            json << "    {\n"
                 << "      \"upscaling\": " << s.upscaling << ",\n"
                 << "      \"spatial_quality_db\": " << s.spatialQualityDb << ",\n"
                 << "      \"spatial_rmse\": " << s.spatialRmse << ",\n"
                 << "      \"spatial_gmsd\": " << s.spatialGmsd << ",\n"
                 << "      \"spatial_detail_transfer_pct\": " << s.detailTransferPct << ",\n"
                 << "      \"spatial_spurious_hf_pct\": " << s.spuriousHfPct << ",\n"
                 << "      \"spatial_hotspot_top10_rmse\": " << s.spatialHotspotTop10Rmse << ",\n"
                 << "      \"spatial_hotspot_peak_rmse\": " << s.spatialHotspotPeakRmse << ",\n"
                 << "      \"temporal_stability_db\": " << s.temporalStabilityDb << ",\n"
                 << "      \"temporal_flicker_rms\": " << s.temporalFlickerRms << ",\n"
                 << "      \"temporal_subpixel_flicker_rms\": " << s.subpixelFlickerRms << ",\n"
                 << "      \"temporal_drift_flicker_rms\": " << s.slowDriftFlickerRms << ",\n"
                 << "      \"motion_robustness_db\": " << s.motionRobustnessDb << ",\n"
                 << "      \"motion_robustness_rmse\": " << s.motionRobustnessRmse << ",\n"
                 << "      \"motion_wake_ghosting_rms\": " << s.motionWakeGhostingRms << ",\n"
                 << "      \"motion_hotspot_top10_rmse\": " << s.motionHotspotTop10Rmse << ",\n"
                 << "      \"motion_hotspot_peak_rmse\": " << s.motionHotspotPeakRmse << ",\n"
                 << "      \"motion_tracked_db\": " << s.trackedMotionDb << ",\n"
                 << "      \"motion_tracked_rmse\": " << s.trackedMotionRmse << ",\n"
                 << "      \"motion_unmodeled_db\": " << s.unmodeledMotionDb << ",\n"
                 << "      \"motion_unmodeled_rmse\": " << s.unmodeledMotionRmse << ",\n"
                 << "      \"perf_upscaler_gpu_ms\": " << s.perfUpscalerGpuMs << ",\n"
                 << "      \"perf_upscaler_gpu_p95_ms\": " << s.perfUpscalerGpuP95Ms << ",\n"
                 << "      \"perf_frame_gpu_ms\": " << s.perfFrameGpuMs << ",\n"
                 << "      \"perf_frame_gpu_p95_ms\": " << s.perfFrameGpuP95Ms << ",\n"
                 << "      \"perf_frame_gpu_iqr_ms\": " << s.perfFrameGpuIqrMs << ",\n"
                 << "      \"perf_baseline_gpu_ms\": " << s.perfBaselineGpuMs << ",\n"
                 << "      \"perf_timing_discrepancy_ms\": " << s.perfTimingDiscrepancyMs << ",\n"
                 << "      \"perf_post_check_passed\": "
                 << (s.perfPostCheckPassed ? "true" : "false") << ",\n"
                 << "      \"cts_valid\": " << (s.ctsValid ? "true" : "false") << "\n"
                 << "    }" << (i + 1 < summaries.size() ? "," : "") << "\n";
        }
        json << "  ],\n";
    }
    json << "  \"records\": [\n";
    for (size_t i = 0; i < records.size(); i++) {
        Record const& r = records[i];
        json << "    { \"regime\": \"" << escapeJson(r.regime) << "\", \"upscaling\": "
             << r.upscaling << ", \"metric\": \"" << escapeJson(r.metric) << "\", \"value\": "
             << r.value << " }" << (i + 1 < records.size() ? "," : "") << "\n";
    }
    json << "  ]\n}\n";
    return json.str();
}

void writeJson(App* app, std::vector<Record> const& records,
        std::vector<BenchmarkSummary> const& summaries) {
    Options const& o = app->options;
    std::string const dumpField = computeDumpDirForJson(o.output, o.dumpDirectory);
    std::string const mainPayload = serializeBenchmarkJson(app, records, summaries, dumpField);

    std::ofstream out(o.output.c_str(), std::ios::trunc);
    out << mainPayload;
    printf("taa_harness: wrote %zu records to %s\n", records.size(), o.output.c_str());

    if (!o.dumpDirectory.isEmpty()) {
        Path const dumpJson = o.dumpDirectory.concat("results.json");
        if (dumpJson.getAbsolutePath().getPath() != o.output.getAbsolutePath().getPath()) {
            std::ofstream dumpOut(dumpJson.c_str(), std::ios::trunc);
            dumpOut << serializeBenchmarkJson(app, records, summaries, ".");
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Side-by-side benchmark comparison (--compare)
// ------------------------------------------------------------------------------------------------

struct ParsedBenchmarkFile {
    Path path;
    std::string label;
    std::string protocolVersion;
    std::string scheduleSignature;
    std::string scheduleDescriptor;
    std::string qualityScheduleSignature;
    std::string qualityScheduleDescriptor;
    std::string perfScheduleSignature;
    std::string perfScheduleDescriptor;
    std::string referenceFilter;
    std::vector<BenchmarkSummary> summaries;
    // Key: regime + "@" + upscaling (formatted "%.4f") -> per-frame edge_rmse_frame_k series
    std::map<std::string, std::vector<double>> edgeRmseFrames;
};

struct JsonCursor {
    std::string const& s;
    size_t pos = 0;

    void skipWs() {
        while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) {
            pos++;
        }
    }

    bool consume(char const c) {
        skipWs();
        if (pos < s.size() && s[pos] == c) {
            pos++;
            return true;
        }
        return false;
    }

    bool parseString(std::string* out) {
        skipWs();
        if (pos >= s.size() || s[pos] != '"') {
            return false;
        }
        pos++;
        out->clear();
        while (pos < s.size()) {
            char const c = s[pos++];
            if (c == '"') {
                return true;
            }
            if (c == '\\' && pos < s.size()) {
                out->push_back(s[pos++]);
            } else {
                out->push_back(c);
            }
        }
        return false;
    }

    bool parseNumber(double* out) {
        skipWs();
        if (pos >= s.size()) {
            return false;
        }
        char const* start = s.c_str() + pos;
        char* end = nullptr;
        double const v = std::strtod(start, &end);
        if (end == start) {
            return false;
        }
        pos = size_t(end - s.c_str());
        *out = v;
        return true;
    }

    bool skipValue() {
        skipWs();
        if (pos >= s.size()) {
            return false;
        }
        char const c = s[pos];
        if (c == '"') {
            std::string dummy;
            return parseString(&dummy);
        }
        if (c == '{') {
            pos++;
            if (consume('}')) {
                return true;
            }
            do {
                std::string k;
                if (!parseString(&k) || !consume(':') || !skipValue()) {
                    return false;
                }
            } while (consume(','));
            return consume('}');
        }
        if (c == '[') {
            pos++;
            if (consume(']')) {
                return true;
            }
            do {
                if (!skipValue()) {
                    return false;
                }
            } while (consume(','));
            return consume(']');
        }
        if (std::isalpha(static_cast<unsigned char>(c))) {
            while (pos < s.size() && std::isalpha(static_cast<unsigned char>(s[pos]))) {
                pos++;
            }
            return true;
        }
        double dummy = 0.0;
        return parseNumber(&dummy);
    }
};

std::string makeScenarioUpscalingKey(std::string const& regime, float const upscaling) {
    char buf[64];
    snprintf(buf, sizeof(buf), "@%.4f", double(upscaling));
    return regime + buf;
}

bool loadBenchmarkFile(Path const& path, ParsedBenchmarkFile* out) {
    std::ifstream in(path.c_str(), std::ios::binary);
    std::string const text((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
    if (text.empty()) {
        fprintf(stderr, "taa_harness: unable to read %s\n", path.c_str());
        return false;
    }

    out->path = path;
    out->label = path.getNameWithoutExtension();

    JsonCursor cur{ text, 0 };
    if (!cur.consume('{')) {
        fprintf(stderr, "taa_harness: invalid JSON in %s\n", path.c_str());
        return false;
    }

    auto getOrCreateSummary = [&](float const upscaling) -> BenchmarkSummary& {
        for (BenchmarkSummary& s: out->summaries) {
            if (std::abs(s.upscaling - upscaling) < 1e-4f) {
                return s;
            }
        }
        BenchmarkSummary s;
        s.upscaling = upscaling;
        out->summaries.push_back(s);
        return out->summaries.back();
    };

    if (!cur.consume('}')) {
        do {
            std::string key;
            if (!cur.parseString(&key) || !cur.consume(':')) {
                fprintf(stderr, "taa_harness: malformed key in %s\n", path.c_str());
                return false;
            }
            if (key == "label") {
                std::string label;
                if (!cur.parseString(&label)) {
                    return false;
                }
                if (!label.empty()) {
                    out->label = label;
                }
            } else if (key == "protocol_version") {
                if (!cur.parseString(&out->protocolVersion)) {
                    return false;
                }
            } else if (key == "schedule_signature") {
                if (!cur.parseString(&out->scheduleSignature)) {
                    return false;
                }
            } else if (key == "schedule_descriptor") {
                if (!cur.parseString(&out->scheduleDescriptor)) {
                    return false;
                }
            } else if (key == "quality_schedule_signature") {
                if (!cur.parseString(&out->qualityScheduleSignature)) {
                    return false;
                }
            } else if (key == "quality_schedule_descriptor") {
                if (!cur.parseString(&out->qualityScheduleDescriptor)) {
                    return false;
                }
            } else if (key == "perf_schedule_signature") {
                if (!cur.parseString(&out->perfScheduleSignature)) {
                    return false;
                }
            } else if (key == "perf_schedule_descriptor") {
                if (!cur.parseString(&out->perfScheduleDescriptor)) {
                    return false;
                }
            } else if (key == "reference_filter") {
                if (!cur.parseString(&out->referenceFilter)) {
                    return false;
                }
            } else if (key == "records") {
                if (!cur.consume('[')) {
                    return false;
                }
                if (!cur.consume(']')) {
                    do {
                        if (!cur.consume('{')) {
                            return false;
                        }
                        std::string regime;
                        std::string metric;
                        double upscaling = 0.0;
                        double value = 0.0;
                        if (!cur.consume('}')) {
                            do {
                                std::string rKey;
                                if (!cur.parseString(&rKey) || !cur.consume(':')) {
                                    return false;
                                }
                                if (rKey == "regime") {
                                    if (!cur.parseString(&regime)) return false;
                                } else if (rKey == "metric") {
                                    if (!cur.parseString(&metric)) return false;
                                } else if (rKey == "upscaling") {
                                    if (!cur.parseNumber(&upscaling)) return false;
                                } else if (rKey == "value") {
                                    if (!cur.parseNumber(&value)) return false;
                                } else if (!cur.skipValue()) {
                                    return false;
                                }
                            } while (cur.consume(','));
                            if (!cur.consume('}')) {
                                return false;
                            }
                        }
                        if (regime == "benchmark") {
                            BenchmarkSummary& s = getOrCreateSummary(float(upscaling));
                            if (metric == "spatial_quality_db") {
                                s.spatialQualityDb = value;
                                s.hasQuality = true;
                            } else if (metric == "spatial_rmse") {
                                s.spatialRmse = value;
                            } else if (metric == "spatial_gmsd") {
                                s.spatialGmsd = value;
                            } else if (metric == "spatial_detail_transfer_pct") {
                                s.detailTransferPct = value;
                            } else if (metric == "spatial_spurious_hf_pct") {
                                s.spuriousHfPct = value;
                            } else if (metric == "spatial_hotspot_top10_rmse") {
                                s.spatialHotspotTop10Rmse = value;
                            } else if (metric == "spatial_hotspot_peak_rmse") {
                                s.spatialHotspotPeakRmse = value;
                            } else if (metric == "temporal_stability_db") {
                                s.temporalStabilityDb = value;
                            } else if (metric == "temporal_flicker_rms") {
                                s.temporalFlickerRms = value;
                            } else if (metric == "temporal_subpixel_flicker_rms") {
                                s.subpixelFlickerRms = value;
                            } else if (metric == "temporal_drift_flicker_rms") {
                                s.slowDriftFlickerRms = value;
                            } else if (metric == "motion_robustness_db") {
                                s.motionRobustnessDb = value;
                            } else if (metric == "motion_robustness_rmse") {
                                s.motionRobustnessRmse = value;
                            } else if (metric == "motion_wake_ghosting_rms") {
                                s.motionWakeGhostingRms = value;
                            } else if (metric == "motion_hotspot_top10_rmse") {
                                s.motionHotspotTop10Rmse = value;
                            } else if (metric == "motion_hotspot_peak_rmse") {
                                s.motionHotspotPeakRmse = value;
                            } else if (metric == "motion_tracked_db") {
                                s.trackedMotionDb = value;
                            } else if (metric == "motion_tracked_rmse") {
                                s.trackedMotionRmse = value;
                            } else if (metric == "motion_unmodeled_db") {
                                s.unmodeledMotionDb = value;
                            } else if (metric == "motion_unmodeled_rmse") {
                                s.unmodeledMotionRmse = value;
                            } else if (metric == "perf_upscaler_gpu_ms") {
                                s.perfUpscalerGpuMs = value;
                                s.hasPerf = true;
                            } else if (metric == "perf_upscaler_gpu_p95_ms") {
                                s.perfUpscalerGpuP95Ms = value;
                            } else if (metric == "perf_frame_gpu_ms") {
                                s.perfFrameGpuMs = value;
                                s.hasPerf = true;
                            } else if (metric == "perf_frame_gpu_p95_ms") {
                                s.perfFrameGpuP95Ms = value;
                            } else if (metric == "perf_frame_gpu_iqr_ms") {
                                s.perfFrameGpuIqrMs = value;
                            } else if (metric == "perf_baseline_gpu_ms") {
                                s.perfBaselineGpuMs = value;
                            } else if (metric == "perf_timing_discrepancy_ms") {
                                s.perfTimingDiscrepancyMs = value;
                            } else if (metric == "perf_post_check_passed") {
                                s.perfPostCheckPassed = value >= 0.5;
                            } else if (metric == "perf_cts_valid") {
                                s.ctsValid = value >= 0.5;
                            }
                        } else {
                            constexpr char const* kEdgeRmseFramePrefix = "edge_rmse_frame_";
                            constexpr size_t kPrefixLen = 16u;
                            if (metric.compare(0, kPrefixLen, kEdgeRmseFramePrefix) == 0) {
                                char* endPtr = nullptr;
                                unsigned long const frameIdx = std::strtoul(
                                        metric.c_str() + kPrefixLen, &endPtr, 10);
                                if (endPtr != metric.c_str() + kPrefixLen && *endPtr == '\0' &&
                                        frameIdx < 4096ul) {
                                    std::string const sKey =
                                            makeScenarioUpscalingKey(regime, float(upscaling));
                                    std::vector<double>& vec = out->edgeRmseFrames[sKey];
                                    if (vec.size() <= size_t(frameIdx)) {
                                        vec.resize(size_t(frameIdx) + 1u, 0.0);
                                    }
                                    vec[size_t(frameIdx)] = value;
                                }
                            }
                        }
                    } while (cur.consume(','));
                    if (!cur.consume(']')) {
                        return false;
                    }
                }
            } else if (!cur.skipValue()) {
                return false;
            }
        } while (cur.consume(','));
    }

    if (out->summaries.empty()) {
        fprintf(stderr,
                "taa_harness: %s contains no benchmark records (run with --benchmark or --perf)\n",
                path.c_str());
        return false;
    }
    return true;
}

void printDescriptorMismatches(std::string const& descA, std::string const& descB) {
    auto splitFields = [](std::string const& desc) {
        std::map<std::string, std::string> fields;
        size_t start = 0;
        while (start < desc.size()) {
            size_t const semi = desc.find(';', start);
            std::string const token = (semi == std::string::npos)
                    ? desc.substr(start)
                    : desc.substr(start, semi - start);
            size_t const eq = token.find('=');
            if (eq != std::string::npos) {
                fields[token.substr(0, eq)] = token.substr(eq + 1);
            }
            if (semi == std::string::npos) {
                break;
            }
            start = semi + 1;
        }
        return fields;
    };

    auto const fieldsA = splitFields(descA);
    auto const fieldsB = splitFields(descB);
    for (auto const& [k, vA]: fieldsA) {
        auto const it = fieldsB.find(k);
        if (it == fieldsB.end()) {
            fprintf(stderr, "  - parameter '%s': '%s' vs <missing>\n", k.c_str(), vA.c_str());
        } else if (it->second != vA) {
            fprintf(stderr, "  - parameter '%s': '%s' vs '%s'\n",
                    k.c_str(), vA.c_str(), it->second.c_str());
        }
    }
    for (auto const& [k, vB]: fieldsB) {
        if (fieldsA.find(k) == fieldsA.end()) {
            fprintf(stderr, "  - parameter '%s': <missing> vs '%s'\n", k.c_str(), vB.c_str());
        }
    }
}

bool compareBenchmarkFiles(std::vector<Path> const& files) {
    if (files.empty()) {
        return false;
    }
    std::vector<ParsedBenchmarkFile> parsed;
    parsed.reserve(files.size());
    for (Path const& f: files) {
        ParsedBenchmarkFile p;
        if (!loadBenchmarkFile(f, &p)) {
            return false;
        }
        parsed.push_back(std::move(p));
    }

    // Disambiguate duplicate labels using each file's stem.
    for (size_t i = 0; i < parsed.size(); i++) {
        bool duplicate = false;
        for (size_t j = 0; j < parsed.size(); j++) {
            if (i != j && parsed[i].label == parsed[j].label) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) {
            parsed[i].label += " (" + parsed[i].path.getNameWithoutExtension() + ")";
        }
    }

    // Normative protocol version & schedule signature verification.
    auto fileHasQuality = [](ParsedBenchmarkFile const& p) {
        for (BenchmarkSummary const& s: p.summaries) {
            if (s.hasQuality) {
                return true;
            }
        }
        return false;
    };
    auto fileHasPerf = [](ParsedBenchmarkFile const& p) {
        for (BenchmarkSummary const& s: p.summaries) {
            if (s.hasPerf) {
                return true;
            }
        }
        return false;
    };

    ParsedBenchmarkFile const* refQualityFile = nullptr;
    ParsedBenchmarkFile const* refPerfFile = nullptr;
    for (ParsedBenchmarkFile const& p: parsed) {
        if (p.protocolVersion != kBenchmarkProtocolVersion) {
            fprintf(stderr,
                    "taa_harness: ERROR -- protocol_version mismatch in '%s' "
                    "(expected '%s', got '%s')\n",
                    p.path.c_str(), kBenchmarkProtocolVersion,
                    p.protocolVersion.empty() ? "<missing>" : p.protocolVersion.c_str());
            return false;
        }
        if (fileHasQuality(p)) {
            std::string const& sig = !p.qualityScheduleSignature.empty()
                    ? p.qualityScheduleSignature
                    : p.scheduleSignature;
            if (sig.empty()) {
                fprintf(stderr,
                        "taa_harness: ERROR -- missing quality schedule_signature in '%s'\n",
                        p.path.c_str());
                return false;
            }
            if (!refQualityFile) {
                refQualityFile = &p;
            } else {
                std::string const& refSig = !refQualityFile->qualityScheduleSignature.empty()
                        ? refQualityFile->qualityScheduleSignature
                        : refQualityFile->scheduleSignature;
                if (sig != refSig) {
                    fprintf(stderr,
                            "taa_harness: ERROR -- quality schedule_signature mismatch between "
                            "'%s' (%s) and '%s' (%s)\n",
                            refQualityFile->label.c_str(), refSig.c_str(),
                            p.label.c_str(), sig.c_str());
                    printDescriptorMismatches(
                            refQualityFile->qualityScheduleDescriptor,
                            p.qualityScheduleDescriptor);
                    return false;
                }
            }
        }
        if (fileHasPerf(p)) {
            std::string const& sig = !p.perfScheduleSignature.empty()
                    ? p.perfScheduleSignature
                    : p.scheduleSignature;
            if (sig.empty()) {
                fprintf(stderr,
                        "taa_harness: ERROR -- missing perf schedule_signature in '%s'\n",
                        p.path.c_str());
                return false;
            }
            if (!refPerfFile) {
                refPerfFile = &p;
            } else {
                std::string const& refSig = !refPerfFile->perfScheduleSignature.empty()
                        ? refPerfFile->perfScheduleSignature
                        : refPerfFile->scheduleSignature;
                if (sig != refSig) {
                    fprintf(stderr,
                            "taa_harness: ERROR -- perf schedule_signature mismatch between "
                            "'%s' (%s) and '%s' (%s)\n",
                            refPerfFile->label.c_str(), refSig.c_str(),
                            p.label.c_str(), sig.c_str());
                    printDescriptorMismatches(
                            refPerfFile->perfScheduleDescriptor,
                            p.perfScheduleDescriptor);
                    return false;
                }
            }
        }
    }

    std::string const& baseFilter =
            refQualityFile ? refQualityFile->referenceFilter : parsed.front().referenceFilter;

    for (BenchmarkSummary const& baseSummary: parsed.front().summaries) {
        float const upscaling = baseSummary.upscaling;
        std::vector<BenchmarkSummary const*> cols;
        bool anyQuality = false;
        bool anyPerf = false;
        for (ParsedBenchmarkFile const& p: parsed) {
            BenchmarkSummary const* match = nullptr;
            for (BenchmarkSummary const& s: p.summaries) {
                if (std::abs(s.upscaling - upscaling) < 1e-4f) {
                    match = &s;
                    break;
                }
            }
            if (!match) {
                break;
            }
            anyQuality = anyQuality || match->hasQuality;
            anyPerf = anyPerf || match->hasPerf;
            cols.push_back(match);
        }
        if (cols.size() != parsed.size()) {
            continue;
        }

        int const colWidth = 18;
        size_t const lineWidth = 34 + cols.size() * size_t(colWidth + 1);
        std::string const sepEq(lineWidth, '=');
        std::string const sepDash(lineWidth, '-');

        printf("\n%s\n", sepEq.c_str());
        printf("  Upscaler Qualification Comparison (x%g, ref: %s, protocol: v%s)\n",
                double(upscaling), baseFilter.empty() ? "-" : baseFilter.c_str(),
                kBenchmarkProtocolVersion);
        printf("%s\n", sepEq.c_str());
        printf("  %-32s", "Metric");
        for (ParsedBenchmarkFile const& p: parsed) {
            std::string hdr = p.label;
            if (hdr.size() > size_t(colWidth)) {
                size_t const suffixPos = hdr.rfind(" [");
                if (suffixPos != std::string::npos && hdr.size() - suffixPos < size_t(colWidth)) {
                    size_t const suffixLen = hdr.size() - suffixPos;
                    hdr = hdr.substr(0, size_t(colWidth) - suffixLen) + hdr.substr(suffixPos);
                } else {
                    hdr = hdr.substr(0, size_t(colWidth));
                }
            }
            printf(" %*s", colWidth, hdr.c_str());
        }
        printf("\n%s\n", sepDash.c_str());

        auto printDbRow = [&](char const* title, auto getter) {
            double best = -1e30;
            for (BenchmarkSummary const* c: cols) {
                if (c->hasQuality) {
                    best = std::max(best, getter(*c));
                }
            }
            printf("  %-32s", title);
            for (BenchmarkSummary const* c: cols) {
                if (!c->hasQuality) {
                    printf(" %*s", colWidth, "-  ");
                    continue;
                }
                double const v = getter(*c);
                bool const isBest = cols.size() > 1 && std::abs(v - best) < 0.005;
                char cell[32];
                snprintf(cell, sizeof(cell), "%.2f dB%s", v, isBest ? " *" : "  ");
                printf(" %*s", colWidth, cell);
            }
            printf("\n");
        };

        auto printMsLowerBetterRow = [&](char const* title, auto getter) {
            double best = 1e30;
            for (BenchmarkSummary const* c: cols) {
                if (c->hasPerf) {
                    best = std::min(best, getter(*c));
                }
            }
            printf("  %-32s", title);
            for (BenchmarkSummary const* c: cols) {
                if (!c->hasPerf) {
                    printf(" %*s", colWidth, "-  ");
                    continue;
                }
                double const v = getter(*c);
                bool const isBest = cols.size() > 1 && std::abs(v - best) < 0.001;
                char cell[32];
                snprintf(cell, sizeof(cell), "%.3f ms%s", v, isBest ? " *" : "  ");
                printf(" %*s", colWidth, cell);
            }
            printf("\n");
        };

        auto printValRow = [&](char const* title, char const* fmt, auto getter) {
            printf("  %-32s", title);
            for (BenchmarkSummary const* c: cols) {
                char cell[32];
                snprintf(cell, sizeof(cell), fmt, getter(*c));
                printf(" %*s", colWidth, cell);
            }
            printf("\n");
        };

        if (anyQuality) {
            printDbRow("1. Spatial Quality (dB)",
                    [](BenchmarkSummary const& s) { return s.spatialQualityDb; });
            printValRow("     Static Spatial RMSE", "%.4f  ",
                    [](BenchmarkSummary const& s) { return s.spatialRmse; });
            printValRow("     Interior Texture GMSD", "%.4f  ",
                    [](BenchmarkSummary const& s) { return s.spatialGmsd; });
            printValRow("     Correlated Detail (%)", "%.1f%%  ",
                    [](BenchmarkSummary const& s) { return s.detailTransferPct; });
            printValRow("     Spurious HF / Ringing (%)", "%.1f%%  ",
                    [](BenchmarkSummary const& s) { return s.spuriousHfPct; });
            printValRow("     Hotspot Top-10 P95 RMSE", "%.4f  ",
                    [](BenchmarkSummary const& s) { return s.spatialHotspotTop10Rmse; });
            printValRow("     Hotspot Peak Cell P95 RMSE", "%.4f  ",
                    [](BenchmarkSummary const& s) { return s.spatialHotspotPeakRmse; });

            printDbRow("2. Temporal Stability (dB)",
                    [](BenchmarkSummary const& s) { return s.temporalStabilityDb; });
            printValRow("     Top-1% Flicker RMS", "%.4f  ",
                    [](BenchmarkSummary const& s) { return s.temporalFlickerRms; });
            printValRow("     Sub-pixel Star Ring RMS", "%.4f  ",
                    [](BenchmarkSummary const& s) { return s.subpixelFlickerRms; });
            printValRow("     Slow Camera Drift RMS", "%.4f  ",
                    [](BenchmarkSummary const& s) { return s.slowDriftFlickerRms; });

            printDbRow("3. Motion Robustness (dB)",
                    [](BenchmarkSummary const& s) { return s.motionRobustnessDb; });
            printDbRow("     - Tracked Motion (dB)",
                    [](BenchmarkSummary const& s) { return s.trackedMotionDb; });
            printDbRow("     - Unmodeled Motion (dB)",
                    [](BenchmarkSummary const& s) { return s.unmodeledMotionDb; });
            printValRow("     Dynamic Edge RMSE", "%.4f  ",
                    [](BenchmarkSummary const& s) { return s.motionRobustnessRmse; });
            printValRow("     Wake Ghosting RMS", "%.4f  ",
                    [](BenchmarkSummary const& s) { return s.motionWakeGhostingRms; });
            printValRow("     Hotspot Top-10 P95 RMSE", "%.4f  ",
                    [](BenchmarkSummary const& s) { return s.motionHotspotTop10Rmse; });
            printValRow("     Hotspot Peak Cell P95 RMSE", "%.4f  ",
                    [](BenchmarkSummary const& s) { return s.motionHotspotPeakRmse; });
        }

        if (anyPerf) {
            printMsLowerBetterRow("4. GPU SR Cost p50 (ms)",
                    [](BenchmarkSummary const& s) { return s.perfUpscalerGpuMs; });
            printValRow("     SR Cost p95 (ms)", "%.3f ms  ",
                    [](BenchmarkSummary const& s) { return s.perfUpscalerGpuP95Ms; });
            printMsLowerBetterRow("     Full Frame GPU p50 (ms)",
                    [](BenchmarkSummary const& s) { return s.perfFrameGpuMs; });
            printValRow("     Render Baseline p50 (ms)", "%.3f ms  ",
                    [](BenchmarkSummary const& s) { return s.perfBaselineGpuMs; });
            printValRow("     Frame GPU IQR (ms)", "%.3f ms  ",
                    [](BenchmarkSummary const& s) { return s.perfFrameGpuIqrMs; });
            printf("  %-32s", "     CTS Integrity");
            for (BenchmarkSummary const* c: cols) {
                char const* status = !c->hasPerf ? "-  " : (c->ctsValid ? "PASS  " : "FAIL  ");
                printf(" %*s", colWidth, status);
            }
            printf("\n");
        }

        printf("%s\n", sepEq.c_str());

        if (anyQuality && cols.size() > 1) {
            // 1. Hotspot Rank-Reversal Detector (Improvement 5.1)
            for (size_t i = 0; i < cols.size(); i++) {
                if (!cols[i]->hasQuality) {
                    continue;
                }
                for (size_t j = 0; j < cols.size(); j++) {
                    if (i == j || !cols[j]->hasQuality) {
                        continue;
                    }
                    double const spatialDbGain =
                            cols[j]->spatialQualityDb - cols[i]->spatialQualityDb;
                    double const spatialHotspotBase = cols[i]->spatialHotspotTop10Rmse;
                    double const spatialHotspotDelta =
                            cols[j]->spatialHotspotTop10Rmse - spatialHotspotBase;
                    if (spatialDbGain >= 0.05 && spatialHotspotBase > 0.0 &&
                            spatialHotspotDelta > std::max(0.0005, 0.02 * spatialHotspotBase)) {
                        printf("  WARNING [HOTSPOT_RANK_REVERSAL] %s vs %s (Spatial Quality): "
                               "global score improved (+%.2f dB) while 8x8 inner-core Top-10 "
                               "hotspot P95 RMSE worsened (%.4f -> %.4f, +%.1f%%)\n",
                                parsed[j].label.c_str(), parsed[i].label.c_str(),
                                spatialDbGain, spatialHotspotBase,
                                cols[j]->spatialHotspotTop10Rmse,
                                100.0 * spatialHotspotDelta / spatialHotspotBase);
                    }

                    double const motionDbGain =
                            cols[j]->motionRobustnessDb - cols[i]->motionRobustnessDb;
                    double const motionHotspotBase = cols[i]->motionHotspotTop10Rmse;
                    double const motionHotspotDelta =
                            cols[j]->motionHotspotTop10Rmse - motionHotspotBase;
                    if (motionDbGain >= 0.05 && motionHotspotBase > 0.0 &&
                            motionHotspotDelta > std::max(0.0005, 0.02 * motionHotspotBase)) {
                        printf("  WARNING [HOTSPOT_RANK_REVERSAL] %s vs %s (Motion Robustness): "
                               "global score improved (+%.2f dB) while 8x8 inner-core Top-10 "
                               "hotspot P95 RMSE worsened (%.4f -> %.4f, +%.1f%%)\n",
                                parsed[j].label.c_str(), parsed[i].label.c_str(),
                                motionDbGain, motionHotspotBase,
                                cols[j]->motionHotspotTop10Rmse,
                                100.0 * motionHotspotDelta / motionHotspotBase);
                    }
                }
            }

            // 2. Per-Frame Worst-Window ([t_start..t_end]) & Peak-Frame (t*) Localization
            //    against parsed[0] (Improvement 5.5)
            char upsSuffix[64];
            snprintf(upsSuffix, sizeof(upsSuffix), "@%.4f", double(upscaling));
            std::string const suffixStr(upsSuffix);

            struct ScenarioWindowLocalization {
                std::string scenario;
                size_t worstWinStart = 0;
                size_t worstWinEnd = 0;
                double worstWinMeanDelta = 0.0;
                size_t worstPeakFrame = 0;
                double worstPeakDelta = 0.0;
                size_t bestWinStart = 0;
                size_t bestWinEnd = 0;
                double bestWinMeanDelta = 0.0;
                size_t bestPeakFrame = 0;
                double bestPeakDelta = 0.0;
            };

            for (size_t c = 1; c < parsed.size(); c++) {
                if (!cols[0]->hasQuality || !cols[c]->hasQuality) {
                    continue;
                }
                std::vector<ScenarioWindowLocalization> locs;
                for (auto const& [key, baseFrames]: parsed[0].edgeRmseFrames) {
                    if (key.size() <= suffixStr.size() ||
                            key.compare(key.size() - suffixStr.size(), suffixStr.size(),
                                    suffixStr) != 0) {
                        continue;
                    }
                    auto const it = parsed[c].edgeRmseFrames.find(key);
                    if (it == parsed[c].edgeRmseFrames.end()) {
                        continue;
                    }
                    std::vector<double> const& candFrames = it->second;
                    size_t const n = std::min(baseFrames.size(), candFrames.size());
                    if (n == 0) {
                        continue;
                    }

                    ScenarioWindowLocalization loc;
                    loc.scenario = key.substr(0, key.size() - suffixStr.size());
                    loc.worstPeakDelta = -1e30;
                    loc.bestPeakDelta = 1e30;

                    std::vector<double> deltas(n, 0.0);
                    for (size_t k = 0; k < n; k++) {
                        double const d = candFrames[k] - baseFrames[k];
                        deltas[k] = d;
                        if (d > loc.worstPeakDelta) {
                            loc.worstPeakDelta = d;
                            loc.worstPeakFrame = k;
                        }
                        if (d < loc.bestPeakDelta) {
                            loc.bestPeakDelta = d;
                            loc.bestPeakFrame = k;
                        }
                    }

                    size_t const winLen = std::min<size_t>(4u, n);
                    loc.worstWinMeanDelta = -1e30;
                    loc.bestWinMeanDelta = 1e30;
                    for (size_t sIdx = 0; sIdx + winLen <= n; sIdx++) {
                        double sum = 0.0;
                        for (size_t w = 0; w < winLen; w++) {
                            sum += deltas[sIdx + w];
                        }
                        double const meanDelta = sum / double(winLen);
                        if (meanDelta > loc.worstWinMeanDelta) {
                            loc.worstWinMeanDelta = meanDelta;
                            loc.worstWinStart = sIdx;
                            loc.worstWinEnd = sIdx + winLen - 1u;
                        }
                        if (meanDelta < loc.bestWinMeanDelta) {
                            loc.bestWinMeanDelta = meanDelta;
                            loc.bestWinStart = sIdx;
                            loc.bestWinEnd = sIdx + winLen - 1u;
                        }
                    }
                    locs.push_back(std::move(loc));
                }

                if (!locs.empty()) {
                    printf("  Per-Frame Edge RMSE Localization: %s vs %s\n",
                            parsed[c].label.c_str(), parsed[0].label.c_str());

                    std::vector<ScenarioWindowLocalization> worstSorted = locs;
                    std::sort(worstSorted.begin(), worstSorted.end(),
                            [](ScenarioWindowLocalization const& a,
                                    ScenarioWindowLocalization const& b) {
                                return a.worstWinMeanDelta > b.worstWinMeanDelta;
                            });
                    size_t printedRegressions = 0;
                    for (ScenarioWindowLocalization const& l: worstSorted) {
                        if (l.worstWinMeanDelta <= 1e-5 && l.worstPeakDelta <= 1e-5) {
                            break;
                        }
                        if (printedRegressions >= 3u) {
                            break;
                        }
                        printf("    Regression  %-18s window [%2zu..%2zu] mean %+.4f "
                               "| peak t*=%2zu (%+.4f)\n",
                                l.scenario.c_str(), l.worstWinStart, l.worstWinEnd,
                                l.worstWinMeanDelta, l.worstPeakFrame, l.worstPeakDelta);
                        printedRegressions++;
                    }

                    std::vector<ScenarioWindowLocalization> bestSorted = locs;
                    std::sort(bestSorted.begin(), bestSorted.end(),
                            [](ScenarioWindowLocalization const& a,
                                    ScenarioWindowLocalization const& b) {
                                return a.bestWinMeanDelta < b.bestWinMeanDelta;
                            });
                    size_t printedImprovements = 0;
                    for (ScenarioWindowLocalization const& l: bestSorted) {
                        if (l.bestWinMeanDelta >= -1e-5 && l.bestPeakDelta >= -1e-5) {
                            break;
                        }
                        if (printedImprovements >= 3u) {
                            break;
                        }
                        printf("    Improvement %-18s window [%2zu..%2zu] mean %+.4f "
                               "| peak t*=%2zu (%+.4f)\n",
                                l.scenario.c_str(), l.bestWinStart, l.bestWinEnd,
                                l.bestWinMeanDelta, l.bestPeakFrame, l.bestPeakDelta);
                        printedImprovements++;
                    }
                }
            }
        }

        if (cols.size() > 1) {
            printf("  (* marks best score: highest dB on quality axes 1-3, lowest ms on GPU "
                   "performance)\n\n");
        }
    }
    return true;
}

// ------------------------------------------------------------------------------------------------
// Callbacks
// ------------------------------------------------------------------------------------------------

App* g_app = nullptr;

Path resolveRepoAsset(char const* relativePath) {
    Path direct(relativePath);
    if (direct.exists()) {
        return direct;
    }
    Path dir = Path::getCurrentExecutable().getParent();
    for (int depth = 0; depth <= 6 && !dir.isEmpty(); depth++) {
        Path const candidate = dir + relativePath;
        if (candidate.exists()) {
            return candidate;
        }
        dir = dir.getParent();
    }
    return direct;
}

void loadAssetIntoApp(App* app, Path const& modelPath) {
    Engine* engine = app->engine;
    std::ifstream in(modelPath.c_str(), std::ios::binary);
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
    if (buffer.empty()) {
        fprintf(stderr, "taa_harness: unable to read %s\n", modelPath.c_str());
        exit(1);
    }
    FilamentAsset* asset = app->assetLoader->createAsset(buffer.data(), buffer.size());
    if (!asset) {
        fprintf(stderr, "taa_harness: unable to parse %s\n", modelPath.c_str());
        exit(1);
    }
    FilamentInstance* instance = asset->getInstance();

    ResourceConfiguration configuration = {};
    configuration.engine = engine;
    CString const gltfPath(modelPath.getAbsolutePath().c_str());
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    configuration.gltfPath = gltfPath.c_str();
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    configuration.normalizeSkinningWeights = true;
    if (!app->resourceLoader) {
        app->resourceLoader = new gltfio::ResourceLoader(configuration);
        app->stbDecoder = createStbProvider(engine);
        app->ktxDecoder = createKtx2Provider(engine);
        app->resourceLoader->addTextureProvider("image/png", app->stbDecoder);
        app->resourceLoader->addTextureProvider("image/jpeg", app->stbDecoder);
        app->resourceLoader->addTextureProvider("image/ktx2", app->ktxDecoder);
    } else {
        app->resourceLoader->setConfiguration(configuration);
    }
    // Synchronous on purpose: an asset that finishes streaming partway through a run would
    // change the image under the measurement.
    app->resourceLoader->loadResources(asset);
    app->resourceLoader->evictResourceData();
    // Skinned assets ship bounds for the rest pose, which is not where the animation puts them.
    instance->recomputeBoundingBoxes();
    asset->releaseSourceData();

    TransformManager const& tcm = engine->getTransformManager();
    mat4f const rootRest = tcm.getTransform(tcm.getInstance(asset->getRoot()));

    // Frame the whole asset from its bounding sphere, so the camera path is scale-independent
    // and the same for every model handed to the harness. FilamentAsset::getBoundingBox() is not
    // usable for this: it is the union of the glTF accessor min/max values, i.e. mesh-local and
    // with no node transforms applied, which for a model whose root node scales it is off by
    // that scale.
    Aabb const bounds = worldBounds(engine, asset);
    float3 const target = (bounds.min + bounds.max) * 0.5f;
    float radius = length(bounds.max - bounds.min) * 0.5f;
    if (radius <= 0.0f) {
        radius = 1.0f;
    }

    LoadedAsset loaded;
    loaded.path = modelPath;
    loaded.asset = asset;
    loaded.instance = instance;
    loaded.rootRestTransform = rootRest;
    loaded.target = target;
    loaded.radius = radius;
    loaded.isSubpixelStar = (modelPath.getNameWithoutExtension() == "SubpixelStar");
    app->assets.push_back(std::move(loaded));
}

void activateAsset(App* app, int const assetIndex) {
    if (app->activeAssetIndex == assetIndex) {
        return;
    }
    if (app->activeAssetIndex >= 0) {
        FilamentAsset* prev = app->assets[app->activeAssetIndex].asset;
        app->scene->removeEntities(prev->getEntities(), prev->getEntityCount());
    }
    LoadedAsset const& next = app->assets[assetIndex];
    app->scene->addEntities(next.asset->getEntities(), next.asset->getEntityCount());
    // Set through the app, not on the camera: this is the same state FilamentApp2 re-applies on
    // its own schedule, so the only way to make it stick is to change what it re-applies.
    app->filamentApp->setCameraNearFar(next.radius * 0.05f, next.radius * 20.0f);
    app->activeAssetIndex = assetIndex;
}

void setup(Engine* engine, View* view, Scene* scene) {
    App* app = g_app;
    app->engine = engine;
    app->view = view;
    app->scene = scene;

    SYSTRACE_ENABLE();

    if (app->options.perf) {
        app->options.fixedPerformanceMode = queryAndroidFixedPerformanceMode();
#if defined(__ANDROID__)
        app->options.thermalStatusStart = int(app->thermalManager.getCurrentThermalStatus());
        app->options.thermalStatusEnd = app->options.thermalStatusStart;
        if (!app->options.fixedPerformanceMode) {
            fprintf(stderr,
                    "taa_harness: WARNING: Android ADPF Fixed Performance Mode is NOT enabled "
                    "(run `adb shell cmd power set-fixed-performance-mode-enabled true` before "
                    "benchmarking; results will be marked cts_valid=false).\n");
        }
#endif
    }

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

    if (app->options.benchmark || (app->options.perf && app->options.model.isEmpty())) {
        char const* const kBenchmarkModels[] = {
            "third_party/models/Fox/Fox.glb",
            "third_party/models/DamagedHelmet/DamagedHelmet.glb",
            "tools/taa/models/SubpixelStar/SubpixelStar.glb",
            "tools/taa/models/SSRReflection/SSRReflection.glb",
            "third_party/models/BusterDrone/BusterDrone.glb",
        };
        for (char const* rel: kBenchmarkModels) {
            Path const resolved = resolveRepoAsset(rel);
            if (!resolved.exists()) {
                fprintf(stderr, "taa_harness: benchmark asset %s not found\n", rel);
                exit(1);
            }
            loadAssetIntoApp(app, resolved);
        }
    } else {
        loadAssetIntoApp(app, app->options.model);
    }

    // The IBL also supplies the skybox. A black background would be a poor test scene: most of
    // what TAA does wrong shows up where a silhouette meets a lit background, and against black
    // there is nothing to ghost onto.
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
    if (!app->shots.empty()) {
        RunConfig const& firstCfg = app->runs[app->shots[0].runIndex].config;
        activateAsset(app, firstCfg.assetIndex);
        app->activeWidth = firstCfg.width;
        app->activeHeight = firstCfg.height;
    }
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

    fprintf(stderr, "\rtaa_harness: [%s] %5zu/%-5zu  %-22s x%-4g", bar, done, total,
            run.config.label.c_str(), double(run.config.nominalUpscaling));
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

void harvestPerfFrameInfo(Run& run, Renderer const* renderer) {
    if (run.timedWindowStartNs <= 0 || !renderer) {
        return;
    }
    auto const history = renderer->getFrameInfoHistory(renderer->getMaxFrameHistorySize());
    for (Renderer::FrameInfo const& info : history) {
        if (info.beginFrame == Renderer::FrameInfo::INVALID) {
            continue;
        }
        int64_t const beginNs = int64_t(info.beginFrame);
        if (beginNs < run.timedWindowStartNs) {
            continue;
        }
        if (run.timedWindowEndNs > 0 && beginNs > run.timedWindowEndNs) {
            continue;
        }
        bool found = false;
        for (Renderer::FrameInfo& existing : run.timedFrameInfos) {
            if (existing.frameId == info.frameId || existing.beginFrame == info.beginFrame) {
                if (info.gpuFrameDuration != Renderer::FrameInfo::INVALID &&
                        info.gpuFrameDuration > 0) {
                    existing.gpuFrameDuration = info.gpuFrameDuration;
                }
                if (info.denoisedGpuFrameDuration != Renderer::FrameInfo::INVALID &&
                        info.denoisedGpuFrameDuration > 0) {
                    existing.denoisedGpuFrameDuration = info.denoisedGpuFrameDuration;
                }
                if (info.gpuFrameComplete != Renderer::FrameInfo::INVALID &&
                        info.gpuFrameComplete > 0) {
                    existing.gpuFrameComplete = info.gpuFrameComplete;
                }
                if (info.endFrame != Renderer::FrameInfo::INVALID && info.endFrame > 0) {
                    existing.endFrame = info.endFrame;
                }
                found = true;
                break;
            }
        }
        if (!found) {
            run.timedFrameInfos.push_back(info);
        }
    }
}

void preRender(Engine* engine, View* view, Scene*, Renderer* renderer) {
    App* app = g_app;
    if (app->finished) {
        app->filamentApp->close();
        return;
    }

    // In headless mode, FilamentApp2::doFrame() runs in an unthrottled loop. Whenever the CPU gets
    // ahead of the GPU, Renderer::beginFrame() returns false (FrameSkipper) and skips render(),
    // postRender(), and endFrame(). If preRender() already prepared the next shot on the previous
    // doFrame() iteration and postRender() has not yet consumed it, only restore the camera pose
    // (which FilamentApp2::doFrame() overwrites via mMainCameraMan before invoking mPreRender) and
    // sleep briefly instead of re-uploading bone matrices or emitting duplicate Systrace events.
    if (app->shotApplied) {
        if (app->nextShot < app->shots.size()) {
            applyShot(app, app->shots[app->nextShot], /*updateScene=*/false);
        }
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        return;
    }

    static bool sRendererConfigured = false;
    if (!sRendererConfigured && renderer) {
        renderer->setFrameRateOptions({ .history = 31 });
        sRendererConfigured = true;
    }

    if (app->nextShot >= app->shots.size()) {
        return;
    }

    Shot const& shot = app->shots[app->nextShot];
    Run& run = app->runs[shot.runIndex];
    RunConfig const& rc = run.config;

    int const wantedAsset = rc.assetIndex;
    bool const resolutionChanged =
            rc.width != app->activeWidth || rc.height != app->activeHeight;
    if (wantedAsset != app->activeAssetIndex || resolutionChanged) {
        engine->flushAndWait();
        activateAsset(app, wantedAsset);
        app->activeWidth = rc.width;
        app->activeHeight = rc.height;
        applyShot(app, shot);
        // FilamentApp2 applies setCameraNearFar() right after preRender returns (see
        // FilamentApp2::configureCamerasForWindow). Resetting stableFrames consumes uncaptured
        // warmup frames so both the new near/far planes, new viewport, and any async ubershader
        // variants (e.g. SSR) finish compiling before the run begins.
        app->stableFrames = 0;
        return;
    }

    if (app->stableFrames > 0) {
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
    }

    if (app->stableFrames < App::kRequiredStableFrames) {
        engine->flushAndWait();
        applyShot(app, shot);
        app->cameraSignature = cameraSignature(view);
        app->stableFrames++;
        return;
    }

    if (shot.perfPhase == PerfPhase::TIMED) {
        if (shot.perfFrameIndex == 0) {
            // Drain all warmup GPU work before starting the timed window so no warmup command
            // buffers bleed into the timed measurement or Systrace slice.
            engine->flushAndWait();
#if defined(__ANDROID__)
            int const therm = int(app->thermalManager.getCurrentThermalStatus());
            if (!app->thermalStartRecorded) {
                app->options.thermalStatusStart = therm;
                app->thermalStartRecorded = true;
            }
            app->options.thermalStatusEnd = std::max(app->options.thermalStatusEnd, therm);
#endif
            run.timedWindowStartNs = steadyClockNowNs();
            SYSTRACE_CONTEXT();
            SYSTRACE_ASYNC_BEGIN(run.systraceWindowName.c_str(), shot.runIndex + 1);
            run.systraceWindowOpen = true;
        }
        SYSTRACE_CONTEXT();
        SYSTRACE_NAME_BEGIN("taa_harness::timed_frame");
        app->timedFrameSystraceOpen = true;
    } else if (shot.perfPhase == PerfPhase::DRAIN) {
        // Wait for the preceding frame's endFrame() submission to complete on the GPU before
        // FRenderer::beginFrame() polls FrameInfoManager. On drain frame 0 this drains the final
        // timed frame and closes the Systrace timed_window slice.
        engine->flushAndWait();
        if (shot.perfFrameIndex == 0) {
            run.timedWindowGpuDoneNs = steadyClockNowNs();
            if (run.systraceWindowOpen) {
                SYSTRACE_CONTEXT();
                SYSTRACE_ASYNC_END(run.systraceWindowName.c_str(), shot.runIndex + 1);
                run.systraceWindowOpen = false;
            }
#if defined(__ANDROID__)
            int const therm = int(app->thermalManager.getCurrentThermalStatus());
            app->options.thermalStatusEnd = std::max(app->options.thermalStatusEnd, therm);
#endif
        }
    }

    applyShot(app, shot);
    app->shotApplied = true;
}

void postRender(Engine* engine, View* view, Scene*, Renderer* renderer) {
    App* app = g_app;
    if (app->timedFrameSystraceOpen) {
        SYSTRACE_CONTEXT();
        SYSTRACE_NAME_END();
        app->timedFrameSystraceOpen = false;
    }
    if (app->finished) {
        app->filamentApp->close();
        return;
    }
    if (!app->shotApplied) {
        return;
    }
    app->shotApplied = false;

    Shot const& shot = app->shots[app->nextShot++];
    Run& run = app->runs[shot.runIndex];
    printProgress(app, shot);

    if (shot.perfPhase == PerfPhase::TIMED) {
        if (shot.perfFrameIndex + 1 == app->options.perfFrames) {
            // FRenderer::beginFrame() for the final timed frame has already recorded its
            // beginFrame timestamp; cap timedWindowEndNs now so subsequent drain frames'
            // beginFrame timestamps are excluded from run.timedFrameInfos.
            run.timedWindowEndNs = steadyClockNowNs();
        }
        harvestPerfFrameInfo(run, renderer);
    } else if (shot.perfPhase == PerfPhase::DRAIN) {
        harvestPerfFrameInfo(run, renderer);
    }

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
                    Run& r = capture->app->runs[capture->shot.runIndex];
                    if (capture->shot.destination == Destination::MSAA) {
                        r.msaaReference = std::move(image);
                    } else if (capture->shot.destination == Destination::ALIASED) {
                        r.aliasedReference = std::move(image);
                    } else if (capture->shot.destination == Destination::CAPTURE) {
                        r.captures[capture->shot.slot] = std::move(image);
                    } else {
                        // Sum here and divide once the whole reference is in: accumulating in
                        // linear light is the point, a running mean would just add rounding.
                        LinearImage& accumulator = r.references[capture->shot.slot];
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
                        r.referenceWeight[capture->shot.slot] += double(weight);
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
            if (run.config.kind == RunKind::QUALITY) {
                appendMetrics(app, run, &records);
            } else {
                appendPerfMetrics(app, run, &records);
            }
        }
        std::vector<BenchmarkSummary> const summaries = computeBenchmarkSummaries(app, &records);
        writeJson(app, records, summaries);
        if (app->options.benchmark || app->options.perf) {
            printBenchmarkScorecard(app, summaries);
            if (!app->options.compareFiles.empty()) {
                std::vector<Path> files = app->options.compareFiles;
                std::string const selfPath = app->options.output.getAbsolutePath().getPath();
                bool alreadyListed = false;
                for (Path const& f: files) {
                    if (f.getAbsolutePath().getPath() == selfPath) {
                        alreadyListed = true;
                        break;
                    }
                }
                if (!alreadyListed) {
                    files.push_back(app->options.output);
                }
                compareBenchmarkFiles(files);
            }
        }
    }

    if (app->sunlight) {
        engine->destroy(app->sunlight);
        EntityManager::get().destroy(app->sunlight);
    }
    for (LoadedAsset& loaded: app->assets) {
        if (loaded.asset) {
            app->assetLoader->destroyAsset(loaded.asset);
        }
    }
    app->assets.clear();
    delete app->resourceLoader;
    delete app->stbDecoder;
    delete app->ktxDecoder;
    if (app->assetLoader) {
        AssetLoader::destroy(&app->assetLoader);
    }
    if (app->materials) {
        app->materials->destroyMaterials();
        delete app->materials;
    }
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
    static std::unique_ptr<App> appHolder;
    appHolder = std::make_unique<App>();
    App& app = *appHolder;
    g_app = &app;

    Options& o = app.options;
    o.benchmark = config.getBool("benchmark");
    o.perf = config.getBool("perf");
    o.ssrEnabled = config.getBool("ssr");
    if (!config.positionalArgs.empty()) {
        o.model = Path(config.positionalArgs[0].c_str_safe());
    }
    o.output = Path(config.getString("output").c_str_safe());
    CString const dump = config.getString("dump");
    if (!dump.empty()) {
        o.dumpDirectory = Path(dump.c_str());
        o.dumpDirectory.mkdirRecursive();
    }
    for (std::string const& item: split(config.getString("compare").c_str_safe())) {
        o.compareFiles.emplace_back(item);
    }
    o.visualizeDisocclusion = config.getBool("visualize-disocclusion");
    o.size = uint32_t(config.getInt("size", 512));
    o.warmupFrames = uint32_t(config.getInt("warmup", 64));
    o.captureFrames = uint32_t(config.getInt("frames", 16));
    CString const perfRes = config.getString("perf-resolution");
    if (!perfRes.empty() && !parsePerfResolution(perfRes.c_str(), &o.perfWidth, &o.perfHeight)) {
        fprintf(stderr,
                "taa_harness: invalid --perf-resolution '%s' (expected WxH or 720p/1080p/1440p/4k)"
                "\n",
                perfRes.c_str());
        exit(1);
    }
    o.perfWarmupFrames = uint32_t(config.getInt("perf-warmup", 64));
    o.perfFrames = uint32_t(config.getInt("perf-frames", 128));
    // Parsed before reference-samples: the sample-count default below depends on which filter was
    // requested, and reading it back the other way around silently defaulted every filter to
    // Lanczos's sample count regardless of -F, which is correct but wasteful for box.
    CString const filter = config.getString("reference-filter");
    if (filter.empty()) {
        o.referenceFilter =
                (o.benchmark || o.perf) ? ReferenceFilter::BOX : ReferenceFilter::LANCZOS;
    } else if (filter == "box") {
        o.referenceFilter = ReferenceFilter::BOX;
    } else if (filter == "lanczos") {
        o.referenceFilter = ReferenceFilter::LANCZOS;
    } else if (filter == "lanczos-filament") {
        o.referenceFilter = ReferenceFilter::LANCZOS_FILAMENT;
    } else {
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
    o.animationStartTime = double(config.getFloat("animation-start", 0.0f));
    CString const objectMotion = config.getString("object-motion");
    if (objectMotion == "animation") {
        o.objectMotion = ObjectMotion::ANIMATION;
    } else if (objectMotion == "translate") {
        o.objectMotion = ObjectMotion::TRANSLATE;
    } else if (objectMotion == "spin") {
        o.objectMotion = ObjectMotion::SPIN;
    } else if (!objectMotion.empty()) {
        fprintf(stderr, "taa_harness: unknown --object-motion '%s'\n", objectMotion.c_str());
        exit(1);
    }
    double const objectSpeed = double(config.getFloat("object-speed", 0.0f));
    o.objectSpeed = objectSpeed > 0.0 ? objectSpeed
                                      : (o.objectMotion == ObjectMotion::SPIN ? 90.0 : 1.0);
    o.objectTexelsPerFrame = double(config.getFloat("object-texels-per-frame", 3.5f));
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
        o.motionVectorsOptions = settings.view.motionVectors;
        o.jitterPattern = o.taaOptions.jitterPattern;
    }
    o.settingsLabelSuffix = !settingsFile.empty()
            ? (" (" + Path(settingsFile.c_str()).getNameWithoutExtension() + ")")
            : "";
    o.label = config.getString("label").c_str_safe();
    o.userSetLabel = !o.label.empty();
    if (!o.userSetLabel) {
        o.label = "default" + o.settingsLabelSuffix;
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
        // Benchmark and perf modes qualify upscaling (2x by default); single-model regression runs
        // keep both 1:1 and 2:1 so neither path can regress unnoticed.
        o.upscalings = (o.benchmark || o.perf)
                ? std::vector<float>{ 2.0f }
                : std::vector<float>{ 1.0f, 2.0f };
    }

    if (config.iblDirectory.empty()) {
        config.iblDirectory = CString("assets/ibl/lightroom_14b");
    }
    uint32_t const maxW = o.perf ? std::max(o.size, o.perfWidth) : o.size;
    uint32_t const maxH = o.perf ? std::max(o.size, o.perfHeight) : o.size;
    config.width = maxW;
    config.height = maxH;
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
        samples::Parameter::makeBool("benchmark", 'b',
                "Run the 11-scenario upscaler qualification benchmark and print a scorecard "
                "(defaults to -u 2 -F box)", false),
        samples::Parameter::makeBool("perf", '\0',
                "Run GPU performance benchmark windows (paired render-baseline vs upscaler) and "
                "report GPU cost in milliseconds", false),
        samples::Parameter::makeString("perf-resolution", '\0',
                "Output resolution for --perf runs: WxH or 720p/1080p/1440p/4k", "1920x1080"),
        samples::Parameter::makeInt("perf-warmup", '\0',
                "Untimed warmup frames before each --perf measurement window", 64, 0),
        samples::Parameter::makeInt("perf-frames", '\0',
                "Timed frames per --perf measurement window", 128, 8),
        samples::Parameter::makeString("label", '\0',
                "Display name for this configuration in benchmark scorecards and --compare", ""),
        samples::Parameter::makeString("compare", '\0',
                "Comma-separated benchmark JSON files to compare side-by-side", ""),
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
                "Comma-separated TAA upscaling factors (default: 2 in --benchmark/--perf, "
                "1,2 otherwise)", ""),
        // 4 and not 8: on hardware that does not support 8x -- Apple silicon, for one -- the
        // request is silently clamped and you get a 4x image believing it is 8x. 4x is the count
        // that is actually guaranteed.
        samples::Parameter::makeInt("msaa", 'M',
                "Cross-check the accumulated reference against an N-sample MSAA render", 0, 0, 4),
        samples::Parameter::makeString("reference-filter", 'F',
                "Kernel the reference integrates against: lanczos (what TAA aims at), "
                "lanczos-filament (what it actually evaluates), or box "
                "(default: box in --benchmark, lanczos otherwise)", ""),
        samples::Parameter::makeString("jitter-pattern", 'J',
                "rgss4, helix4, halton8, halton16, halton32 (overrides --settings)", ""),
        samples::Parameter::makeString("settings", 's',
                "JSON settings file; view.taa and view.motionVectors are applied to every run", ""),
        samples::Parameter::makeFloat("diff-gain", 'G',
                "Amplification applied to dumped difference images", 8.0f),
        samples::Parameter::makeInt("animation", 'A',
                "Index of the glTF animation the object regime plays", 0, 0),
        samples::Parameter::makeFloat("animation-start", '\0',
                "Start offset in seconds within the glTF animation clip", 0.0f),
        samples::Parameter::makeString("object-motion", '\0',
                "animation (glTF clip), translate (root entity dollies screen-right), or spin "
                "(root entity rotates about vertical axis), object regime", "animation"),
        samples::Parameter::makeFloat("object-speed", '\0',
                "Animation playback speed multiplier (animation) or degrees/second (spin), "
                "0 picks mode default (1.0 / 90.0)", 0.0f),
        samples::Parameter::makeFloat("object-texels-per-frame", '\0',
                "Screen-right speed at the target's depth, in render-resolution texels/frame, "
                "translate mode", 3.5f),
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
        samples::Parameter::makeBool("ssr", '\0',
                "Enable screen-space reflections (automatically enabled on ssr_reflection in "
                "--benchmark)", false),
        samples::Parameter::makeBool("quiet", 'q',
                "Suppress the progress bar even when stderr is a terminal", false),
        samples::Parameter::makeFloat("edge-threshold", 'E',
                "Luminance gradient above which a texel counts as a silhouette", 0.08f),
        samples::Parameter::makeBool("visualize-disocclusion", '\0',
                "Render taa.mat's disocclusion mask instead of the image (use with --dump; "
                "the metrics from such a run are meaningless)", false),
    };
}

extern "C" int taa_harness_main(int const argc, char* argv[]) {
    samples::CommandLineSpecification spec = {
        .sampleDescription =
                "SAMPLE_TAA_HARNESS measures temporal anti-aliasing and upscaler quality & "
                "GPU performance\n\n"
                "Renders a fixed, reproducible frame schedule headlessly and reports masked\n"
                "per-regime metrics as JSON. Pass --benchmark (-b) to run the 11-scenario\n"
                "upscaler qualification suite and output a 3-metric quality scorecard, --perf\n"
                "to run paired render-baseline vs upscaler GPU performance benchmarks (ms),\n"
                "or --compare to compare saved benchmark JSON files side-by-side.",
        .positionalArgsDescription = { "glTF/glb model" },
        .requiredPositionalArgCount = 0,
        .parameters = createAppParameters(),
    };

    utils::getopt::optind = 1;
    SampleConfig config;
    samples::handleCommandLineArguments(argc, argv, &config, spec);

    bool const benchmark = config.getBool("benchmark");
    bool const perf = config.getBool("perf");
    CString const compareArg = config.getString("compare");
    if (!compareArg.empty() && !benchmark && !perf &&
            config.positionalArgs.empty()) {
        std::vector<Path> files;
        for (std::string const& item: split(compareArg.c_str())) {
            files.emplace_back(item);
        }
        return compareBenchmarkFiles(files) ? 0 : 1;
    }

    if (benchmark) {
        if (!config.positionalArgs.empty()) {
            fprintf(stderr, "taa_harness: --benchmark uses its built-in 4-scene suite and takes "
                            "no positional model argument\n");
            return 1;
        }
    } else if (perf && config.positionalArgs.empty()) {
        // Uses the built-in 4-scene performance suite.
    } else {
        if (config.positionalArgs.empty()) {
            fprintf(stderr, "taa_harness: missing glTF/glb model path "
                            "(or pass --benchmark / --perf / --compare)\n");
            return 1;
        }
        Path const model(config.positionalArgs[0].c_str_safe());
        if (!model.exists()) {
            fprintf(stderr, "taa_harness: file %s not found\n", model.c_str());
            return 1;
        }
    }

    config.title = "TAA Harness";
    config.headless = true;
    Path const exeDir = Path::getCurrentExecutable().getParent();
    Path const cwd = Path::getCurrentDirectory();
    if (config.assetsPath.empty()) {
        if ((cwd + "assets/ibl/lightroom_14b").exists()) {
            config.assetsPath = CString(cwd.c_str());
        } else if ((exeDir + "assets/ibl/lightroom_14b").exists()) {
            config.assetsPath = CString(exeDir.c_str());
        } else {
            config.assetsPath = CString((exeDir + "../../samples").c_str());
        }
    }
    auto dm = std::make_unique<filament::app::HeadlessDisplayManager>();
    auto loader = std::make_unique<filament::app::DesktopAssetLoader>(
            utils::Path(config.assetsPath.c_str_safe()));
    auto fApp = createSampleApp(config, dm.get(), loader.get());
    int const rc = samples::runApp(std::move(fApp), std::move(dm), std::move(loader));
    return (g_app && g_app->exitCode != 0) ? g_app->exitCode : rc;
}

#if defined(__ANDROID__)

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    ::filament::VirtualMachineEnv::JNI_OnLoad(vm);
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_taaharness_TaaHarnessNative_nRunHarness(
        JNIEnv* env, jclass, jstring jWorkingDir, jobjectArray jArgs,
        jboolean jFixedPerfMode, jobjectArray jOutOutput) {
    setenv("TAA_HARNESS_FIXED_PERF_MODE", jFixedPerfMode ? "1" : "0", 1);

    if (jWorkingDir != nullptr) {
        char const* workDir = env->GetStringUTFChars(jWorkingDir, nullptr);
        if (workDir != nullptr && *workDir != '\0') {
            chdir(workDir);
        }
        if (workDir != nullptr) {
            env->ReleaseStringUTFChars(jWorkingDir, workDir);
        }
    }

    jsize const argCount = jArgs ? env->GetArrayLength(jArgs) : 0;
    std::vector<std::string> argStrings;
    argStrings.reserve(size_t(argCount) + 1u);
    argStrings.emplace_back("taa_harness");
    for (jsize i = 0; i < argCount; ++i) {
        auto jStr = static_cast<jstring>(env->GetObjectArrayElement(jArgs, i));
        if (jStr != nullptr) {
            char const* utf = env->GetStringUTFChars(jStr, nullptr);
            if (utf != nullptr) {
                argStrings.emplace_back(utf);
                env->ReleaseStringUTFChars(jStr, utf);
            }
            env->DeleteLocalRef(jStr);
        }
    }

    std::vector<char*> argv;
    argv.reserve(argStrings.size() + 1u);
    for (std::string& s : argStrings) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    fflush(stdout);
    fflush(stderr);
    int const savedStdout = dup(STDOUT_FILENO);
    int const savedStderr = dup(STDERR_FILENO);
    int pipeFds[2] = { -1, -1 };
    std::string captured;
    std::thread readerThread;

    if (pipe(pipeFds) == 0) {
        dup2(pipeFds[1], STDOUT_FILENO);
        dup2(pipeFds[1], STDERR_FILENO);
        close(pipeFds[1]);
        pipeFds[1] = -1;

        int const readFd = pipeFds[0];
        readerThread = std::thread([readFd, &captured]() {
            char buf[1024];
            std::string lineBuf;
            ssize_t n = 0;
            while ((n = read(readFd, buf, sizeof(buf) - 1)) > 0) {
                buf[n] = '\0';
                captured.append(buf, size_t(n));
                lineBuf.append(buf, size_t(n));
                size_t pos = 0;
                while ((pos = lineBuf.find('\n')) != std::string::npos) {
                    std::string const line = lineBuf.substr(0, pos);
                    __android_log_print(ANDROID_LOG_INFO, "TaaHarness", "%s", line.c_str());
                    lineBuf.erase(0, pos + 1);
                }
            }
            if (!lineBuf.empty()) {
                __android_log_print(ANDROID_LOG_INFO, "TaaHarness", "%s", lineBuf.c_str());
            }
            close(readFd);
        });
    }

    int const rc = taa_harness_main(int(argStrings.size()), argv.data());

    fflush(stdout);
    fflush(stderr);
    if (savedStdout >= 0) {
        dup2(savedStdout, STDOUT_FILENO);
        close(savedStdout);
    }
    if (savedStderr >= 0) {
        dup2(savedStderr, STDERR_FILENO);
        close(savedStderr);
    }
    if (readerThread.joinable()) {
        readerThread.join();
    }

    if (jOutOutput != nullptr && env->GetArrayLength(jOutOutput) > 0) {
        jstring jOut = env->NewStringUTF(captured.c_str());
        env->SetObjectArrayElement(jOutOutput, 0, jOut);
        env->DeleteLocalRef(jOut);
    }
    return rc;
}

#else

int main(int const argc, char* argv[]) {
    return taa_harness_main(argc, argv);
}

#endif
