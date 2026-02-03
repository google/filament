/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_OPTIONS_H
#define TNT_FILAMENT_OPTIONS_H

#include <filament/Color.h>

#include <math/vec2.h>
#include <math/vec3.h>

#include <math.h>

#include <stdint.h>

namespace filament {

class Texture;

/**
 * Generic quality level.
 */
enum class QualityLevel : uint8_t {
    LOW,
    MEDIUM,
    HIGH,
    ULTRA
};

enum class BlendMode : uint8_t {
    OPAQUE,
    TRANSLUCENT
};

/**
 * Dynamic resolution can be used to either reach a desired target frame rate
 * by lowering the resolution of a View, or to increase the quality when the
 * rendering is faster than the target frame rate.
 *
 * <p>This structure can be used to specify the minimum scale factor used when
 * lowering the resolution of a View, and the maximum scale factor used when
 * increasing the resolution for higher quality rendering. The scale factors
 * can be controlled on each X and Y axis independently. By default, all scale
 * factors are set to 1.0.</p>
 *
 * <ul>
 * <li>enabled:   enable or disables dynamic resolution on a View</li>
 *
 * <li>homogeneousScaling: by default the system scales the major axis first. Set this to true
 *                     to force homogeneous scaling.</li>
 *
 * <li>minScale:  the minimum scale in X and Y this View should use</li>
 *
 * <li>maxScale:  the maximum scale in X and Y this View should use</li>
 *
 * <li>quality:   upscaling quality.
 *            LOW: 1 bilinear tap, Medium: 4 bilinear taps, High: 9 bilinear taps (tent)</li>
 * </ul>
 *
 * <p>Note:
 * Dynamic resolution is only supported on platforms where the time to render
 * a frame can be measured accurately. On platforms where this is not supported,
 * Dynamic Resolution can't be enabled unless <code>minScale == maxScale</code>.</p>
 *
 * @see Renderer.FrameRateOptions
 *
 */
struct DynamicResolutionOptions {
    /** minimum scale factors in x and y */
    math::float2 minScale = {0.5f, 0.5f};           // %codegen_java_float%
    /** maximum scale factors in x and y */
    math::float2 maxScale = {1.0f, 1.0f};           // %codegen_java_float%
    /** sharpness when QualityLevel::MEDIUM or higher is used [0 (disabled), 1 (sharpest)] */
    float sharpness = 0.9f;
    /** enable or disable dynamic resolution */
    bool enabled = false;
    /** set to true to force homogeneous scaling */
    bool homogeneousScaling = false;

    /**
     * Upscaling quality
     * <ul>
     * <li>LOW:    bilinear filtered blit. Fastest, poor quality</li>
     * <li>MEDIUM: Qualcomm Snapdragon Game Super Resolution (SGSR) 1.0</li>
     * <li>HIGH:   AMD FidelityFX FSR1 w/ mobile optimizations</li>
     * <li>ULTRA:  AMD FidelityFX FSR1</li>
     * </ul>
     *      FSR1 and SGSR require a well anti-aliased (MSAA or TAA), noise free scene.
     *      Avoid FXAA and dithering.
     *
     * <p>The default upscaling quality is set to LOW.</p>
     *
     * <p>caveat: currently, <code>quality</code> is always set to LOW if the View is TRANSLUCENT.</p>
     */
    QualityLevel quality = QualityLevel::LOW;
};

/**
 * Options to control the bloom effect
 *
 * <ul>
 * <li>enabled:     Enable or disable the bloom post-processing effect. Disabled by default.</li>
 *
 * <li>levels:      Number of successive blurs to achieve the blur effect, the minimum is 3 and the
 *              maximum is 12. This value together with resolution influences the spread of the
 *              blur effect. This value can be silently reduced to accommodate the original
 *              image size.</li>
 *
 * <li>resolution:  Resolution of bloom's minor axis. The minimum value is 2^levels and the
 *              the maximum is lower of the original resolution and 4096. This parameter is
 *              silently clamped to the minimum and maximum.
 *              It is highly recommended that this value be smaller than the target resolution
 *              after dynamic resolution is applied (horizontally and vertically).</li>
 *
 * <li>strength:    how much of the bloom is added to the original image. Between 0 and 1.</li>
 *
 * <li>blendMode:   Whether the bloom effect is purely additive (false) or mixed with the original
 *              image (true).</li>
 *
 * <li>threshold:   When enabled, a threshold at 1.0 is applied on the source image, this is
 *              useful for artistic reasons and is usually needed when a dirt texture is used.</li>
 *
 * <li>dirt:        A dirt/scratch/smudges texture (that can be RGB), which gets added to the
 *              bloom effect. Smudges are visible where bloom occurs. Threshold must be
 *              enabled for the dirt effect to work properly.</li>
 *
 * <li>dirtStrength: Strength of the dirt texture.</li>
 * </ul>
 */
struct BloomOptions {
    enum class BlendMode : uint8_t {
        /** Bloom is modulated by the strength parameter and added to the scene */
        ADD,
        /** Bloom is interpolated with the scene using the strength parameter */
        INTERPOLATE
    };
    /** user provided dirt texture */
    Texture* dirt = nullptr;                // %codegen_skip_json% %codegen_skip_javascript%
    /** strength of the dirt texture */
    float dirtStrength = 0.2f;              // %codegen_skip_json% %codegen_skip_javascript%
    /** bloom's strength between 0.0 and 1.0 */
    float strength = 0.10f;
    /** resolution of vertical axis (2^levels to 2048) */
    uint32_t resolution = 384;
    /** number of blur levels (1 to 11) */
    uint8_t levels = 6;
    /** how the bloom effect is applied */
    BlendMode blendMode = BlendMode::ADD;
    /** whether to threshold the source */
    bool threshold = true;
    /** enable or disable bloom */
    bool enabled = false;
    /** limit highlights to this value before bloom [10, +inf] */
    float highlight = 1000.0f;

    /**
     * Bloom quality level.
     * <ul>
     * <li>LOW (default): use a more optimized down-sampling filter, however there can be artifacts
     *      with dynamic resolution, this can be alleviated by using the homogenous mode.</li>
     * <li>MEDIUM: Good balance between quality and performance.</li>
     * <li>HIGH: In this mode the bloom resolution is automatically increased to avoid artifacts.
     *      This mode can be significantly slower on mobile, especially at high resolution.
     *      This mode greatly improves the anamorphic bloom.</li>
     * </ul>
     */
    QualityLevel quality = QualityLevel::LOW;

    /** enable screen-space lens flare */
    bool lensFlare = false;
    /** enable starburst effect on lens flare */
    bool starburst = true;
    /** amount of chromatic aberration */
    float chromaticAberration = 0.005f;
    /** number of flare "ghosts" */
    uint8_t ghostCount = 4;
    /** spacing of the ghost in screen units [0, 1[ */
    float ghostSpacing = 0.6f;
    /** hdr threshold for the ghosts */
    float ghostThreshold = 10.0f;
    /** thickness of halo in vertical screen units, 0 to disable */
    float haloThickness = 0.1f;
    /** radius of halo in vertical screen units [0, 0.5] */
    float haloRadius = 0.4f;
    /** hdr threshold for the halo */
    float haloThreshold = 10.0f;
};

/**
 * Options to control large-scale fog in the scene. Materials can enable the <code>linearFog</code> property,
 * which uses a simplified, linear equation for fog calculation; in this mode, the heightFalloff
 * is ignored as well as the mipmap selection in IBL or skyColor mode.
 */
struct FogOptions {
    /**
     * Distance in world units [m] from the camera to where the fog starts ( >= 0.0 )
     */
    float distance = 0.0f;

    /**
     * Distance in world units [m] after which the fog calculation is disabled.
     * This can be used to exclude the skybox, which is desirable if it already contains clouds or
     * fog. The default value is +infinity which applies the fog to everything.
     *
     * <p>Note: The SkyBox is typically at a distance of 1e19 in world space (depending on the near
     * plane distance and projection used though).</p>
     */
    float cutOffDistance = INFINITY;

    /**
     * fog's maximum opacity between 0 and 1. Ignored in <code>linearFog</code> mode.
     */
    float maximumOpacity = 1.0f;

    /**
     * Fog's floor in world units [m]. This sets the "sea level".
     */
    float height = 0.0f;

    /**
     * How fast the fog dissipates with the altitude. heightFalloff has a unit of [1/m].
     * It can be expressed as 1/H, where H is the altitude change in world units [m] that causes a
     * factor 2.78 (e) change in fog density.
     *
     * <p>A falloff of 0 means the fog density is constant everywhere and may result is slightly
     * faster computations.</p>
     *
     * <p>In <code>linearFog</code> mode, only use to compute the slope of the linear equation. Completely
     * ignored if set to 0.</p>
     */
    float heightFalloff = 1.0f;

    /**
     *  Fog's color is used for ambient light in-scattering, a good value is
     *  to use the average of the ambient light, possibly tinted towards blue
     *  for outdoors environments. Color component's values should be between 0 and 1, values
     *  above one are allowed but could create a non energy-conservative fog (this is dependant
     *  on the IBL's intensity as well).
     *
     *  <p>We assume that our fog has no absorption and therefore all the light it scatters out
     *  becomes ambient light in-scattering and has lost all directionality, i.e.: scattering is
     *  isotropic. This somewhat simulates Rayleigh scattering.</p>
     *
     *  <p>This value is used as a tint instead, when fogColorFromIbl is enabled.</p>
     *
     *  @see #fogColorFromIbl
     */
    LinearColor color = { 1.0f, 1.0f, 1.0f };

    /**
     * Extinction factor in [1/m] at an altitude 'height'. The extinction factor controls how much
     * light is absorbed and out-scattered per unit of distance. Each unit of extinction reduces
     * the incoming light to 37% of its original value.
     *
     * <p>Note: The extinction factor is related to the fog density, it's usually some constant K times
     * the density at sea level (more specifically at fog height). The constant K depends on
     * the composition of the fog/atmosphere.</p>
     *
     * <p>For historical reason this parameter is called <code>density</code>.</p>
     *
     * <p>In <code>linearFog</code> mode this is the slope of the linear equation if heightFalloff is set to 0.
     * Otherwise, heightFalloff affects the slope calculation such that it matches the slope of
     * the standard equation at the camera height.</p>
     */
    float density = 0.1f;

    /**
     * Distance in world units [m] from the camera where the Sun in-scattering starts.
     * Ignored in <code>linearFog</code> mode.
     */
    float inScatteringStart = 0.0f;

    /**
     * Very inaccurately simulates the Sun's in-scattering. That is, the light from the sun that
     * is scattered (by the fog) towards the camera.
     * Size of the Sun in-scattering (>0 to activate). Good values are >> 1 (e.g. ~10 - 100).
     * Smaller values result is a larger scattering size.
     * Ignored in <code>linearFog</code> mode.
     */
    float inScatteringSize = -1.0f;

    /**
     * The fog color will be sampled from the IBL in the view direction and tinted by <code>color</code>.
     * Depending on the scene this can produce very convincing results.
     *
     * <p>This simulates a more anisotropic phase-function.</p>
     *
     * <p><code>fogColorFromIbl</code> is ignored when skyTexture is specified.</p>
     *
     * @see #skyColor
     */
    bool fogColorFromIbl = false;

    /**
     * skyTexture must be a mipmapped cubemap. When provided, the fog color will be sampled from
     * this texture, higher resolution mip levels will be used for objects at the far clip plane,
     * and lower resolution mip levels for objects closer to the camera. The skyTexture should
     * typically be heavily blurred; a typical way to produce this texture is to blur the base
     * level with a strong gaussian filter or even an irradiance filter and then generate mip
     * levels as usual. How blurred the base level is somewhat of an artistic decision.
     *
     * <p>This simulates a more anisotropic phase-function.</p>
     *
     * <p><code>fogColorFromIbl</code> is ignored when skyTexture is specified.</p>
     *
     * <p>In <code>linearFog</code> mode mipmap level 0 is always used.</p>
     *
     * @see Texture
     * @see #fogColorFromIbl
     */
    Texture* skyColor = nullptr;    // %codegen_skip_json% %codegen_skip_javascript%

    /**
     * Enable or disable large-scale fog
     */
    bool enabled = false;
};

/**
 * Options to control Depth of Field (DoF) effect in the scene.
 *
 * <p>cocScale can be used to set the depth of field blur independently of the camera
 * aperture, e.g. for artistic reasons. This can be achieved by setting:
 *      cocScale = cameraAperture / desiredDoFAperture</p>
 *
 * @see Camera
 */
struct DepthOfFieldOptions {
    enum class Filter : uint8_t {
        NONE,
        UNUSED,
        MEDIAN
    };
    /** circle of confusion scale factor (amount of blur) */
    float cocScale = 1.0f;
    /** width/height aspect ratio of the circle of confusion (simulate anamorphic lenses) */
    float cocAspectRatio = 1.0f;
    /** maximum aperture diameter in meters (zero to disable rotation) */
    float maxApertureDiameter = 0.01f;
    /** enable or disable depth of field effect */
    bool enabled = false;
    /** filter to use for filling gaps in the kernel */
    Filter filter = Filter::MEDIAN;
    /** perform DoF processing at native resolution */
    bool nativeResolution = false;
    /**
     * Number of of rings used by the gather kernels. The number of rings affects quality
     * and performance. The actual number of sample per pixel is defined
     * as (ringCount * 2 - 1)^2. Here are a few commonly used values:
     * <ul>
     *       <li>3 rings :   25 ( 5x 5 grid)</li>
     *       <li>4 rings :   49 ( 7x 7 grid)</li>
     *       <li>5 rings :   81 ( 9x 9 grid)</li>
     *      <li>17 rings : 1089 (33x33 grid)</li>
     * </ul>
     *
     * <p>With a maximum circle-of-confusion of 32, it is never necessary to use more than 17 rings.</p>
     *
     * <p>Usually all three settings below are set to the same value, however, it is often
     * acceptable to use a lower ring count for the "fast tiles", which improves performance.
     * Fast tiles are regions of the screen where every pixels have a similar
     * circle-of-confusion radius.</p>
     *
     * <p>A value of 0 means default, which is 5 on desktop and 3 on mobile.</p>
     *
     * @see #backgroundRingCount
     * @see #fastGatherRingCount
     */
    /** number of kernel rings for foreground tiles */
    uint8_t foregroundRingCount = 0;
    /** number of kernel rings for background tiles */
    uint8_t backgroundRingCount = 0;
    /** number of kernel rings for fast tiles */
    uint8_t fastGatherRingCount = 0;

    /**
     * maximum circle-of-confusion in pixels for the foreground, must be in [0, 32] range.
     * A value of 0 means default, which is 32 on desktop and 24 on mobile.
     */
    uint16_t maxForegroundCOC = 0;

    /**
     * maximum circle-of-confusion in pixels for the background, must be in [0, 32] range.
     * A value of 0 means default, which is 32 on desktop and 24 on mobile.
     */
    uint16_t maxBackgroundCOC = 0;
};

/**
 * Options to control the vignetting effect.
 */
struct VignetteOptions {
    /** high values restrict the vignette closer to the corners, between 0 and 1 */
    float midPoint = 0.5f;
    /** controls the shape of the vignette, from a rounded rectangle (0.0), to an oval (0.5), to a circle (1.0) */
    float roundness = 0.5f;
    /** softening amount of the vignette effect, between 0 and 1 */
    float feather = 0.5f;
    /** color of the vignette effect, alpha is currently ignored */
    LinearColorA color = {0.0f, 0.0f, 0.0f, 1.0f};
    /** enables or disables the vignette effect */
    bool enabled = false;
};

/**
 * Structure used to set the precision of the color buffer and related quality settings.
 *
 * @see #setRenderQuality
 * @see #getRenderQuality
 */
struct RenderQuality {
    /**
     * Sets the quality of the HDR color buffer.
     *
     * <p>A quality of HIGH or ULTRA means using an RGB16F or RGBA16F color buffer. This means
     * colors in the LDR range (0..1) have a 10 bit precision. A quality of LOW or MEDIUM means
     * using an R11G11B10F opaque color buffer or an RGBA16F transparent color buffer. With
     * R11G11B10F colors in the LDR range have a precision of either 6 bits (red and green
     * channels) or 5 bits (blue channel).</p>
     */
    QualityLevel hdrColorBuffer = QualityLevel::HIGH;
};

/**
 * Options for screen space Ambient Occlusion (SSAO) and Screen Space Cone Tracing (SSCT)
 * @see #setAmbientOcclusionOptions
 */
struct AmbientOcclusionOptions {
    enum class AmbientOcclusionType : uint8_t {
        /** use Scalable Ambient Occlusion */
        SAO,
        /** use Ground Truth-Based Ambient Occlusion */
        GTAO,
    };

    /** Type of ambient occlusion algorithm. */
    AmbientOcclusionType aoType = AmbientOcclusionType::SAO;
    /** Ambient Occlusion radius in meters, between 0 and ~10. */
    float radius = 0.3f;
    /** Controls ambient occlusion's contrast. Must be positive. */
    float power = 1.0f;

    /**
     * Self-occlusion bias in meters. Use to avoid self-occlusion.
     * Between 0 and a few mm. No effect when aoType set to GTAO
     */
    float bias = 0.0005f;

    /** How each dimension of the AO buffer is scaled. Must be either 0.5 or 1.0. */
    float resolution = 0.5f;
    /** Strength of the Ambient Occlusion effect. */
    float intensity = 1.0f;
    /** depth distance that constitute an edge for filtering */
    float bilateralThreshold = 0.05f;
    /** affects # of samples used for AO and params for filtering */
    QualityLevel quality = QualityLevel::LOW;
    /** affects AO smoothness. Recommend setting to HIGH when aoType set to GTAO. */
    QualityLevel lowPassFilter = QualityLevel::MEDIUM;
    /** affects AO buffer upsampling quality */
    QualityLevel upsampling = QualityLevel::LOW;
    /** enables or disables screen-space ambient occlusion */
    bool enabled = false;
    /** enables bent normals computation from AO, and specular AO */
    bool bentNormals = false;
    /** min angle in radian to consider. No effect when aoType set to GTAO. */
    float minHorizonAngleRad = 0.0f;
    /**
     * Screen Space Cone Tracing (SSCT) options
     * Ambient shadows from dominant light
     */
    struct Ssct {
        /** full cone angle in radian, between 0 and pi/2 */
        float lightConeRad = 1.0f;
        /** how far shadows can be cast */
        float shadowDistance = 0.3f;
        /** max distance for contact */
        float contactDistanceMax = 1.0f;
        /** intensity */
        float intensity = 0.8f;
        /** light direction */
        math::float3 lightDirection = { 0, -1, 0 };
        /** depth bias in world units (mitigate self shadowing) */
        float depthBias = 0.01f;
        /** depth slope bias (mitigate self shadowing) */
        float depthSlopeBias = 0.01f;
        /** tracing sample count, between 1 and 255 */
        uint8_t sampleCount = 4;
        /** # of rays to trace, between 1 and 255 */
        uint8_t rayCount = 1;
        /** enables or disables SSCT */
        bool enabled = false;
    };
    Ssct ssct;                           // %codegen_skip_javascript% %codegen_java_flatten%

    /**
     * Ground Truth-base Ambient Occlusion (GTAO) options
     */
    struct Gtao {
        /** # of slices. Higher value makes less noise. */
        uint8_t sampleSliceCount = 4;
        /** # of steps the radius is divided into for integration. Higher value makes less bias. */
        uint8_t sampleStepsPerSlice = 3;
        /** thickness heuristic, should be closed to 0. No effect when useVisibilityBitmasks sets to true. */
        float thicknessHeuristic = 0.004f;

        /**
         * Enables or disables visibility bitmasks mode. Notes that bent normal doesn't work under this mode.
         * Caution: Changing this option at runtime is very expensive as it may trigger a shader re-compilation.
         */
        bool useVisibilityBitmasks = false;
        /** constant thickness value of objects on the screen in world space. Only take effect when useVisibilityBitmasks is set to true. */
        float constThickness = 0.5f;

        /**
         * Increase thickness with distance to maintain detail on distant surfaces.
         * Caution: Changing this option at runtime is very expensive as it may trigger a shader re-compilation.
         */
        bool linearThickness = false;
    };
    Gtao gtao;                           // %codegen_skip_javascript% %codegen_java_flatten%
};

/**
 * Options for Multi-Sample Anti-aliasing (MSAA)
 * @see #setMultiSampleAntiAliasingOptions
 */
struct MultiSampleAntiAliasingOptions {
    /** enables or disables msaa */
    bool enabled = false;

    /**
     * sampleCount number of samples to use for multi-sampled anti-aliasing.<br>
     *              0: treated as 1
     *              1: no anti-aliasing
     *              n: sample count. Effective sample could be different depending on the
     *                 GPU capabilities.
     */
    uint8_t sampleCount = 4;

    /**
     * custom resolve improves quality for HDR scenes, but may impact performance.
     */
    bool customResolve = false;
};

/**
 * Options for Temporal Anti-aliasing (TAA)
 * Most TAA parameters are extremely costly to change, as they will trigger the TAA post-process
 * shaders to be recompiled. These options should be changed or set during initialization.
 * `filterWidth`, `feedback` and `jitterPattern`, however, can be changed at any time.
 *
 * <p><code>feedback</code> of 0.1 effectively accumulates a maximum of 19 samples in steady state.
 * see "A Survey of Temporal Antialiasing Techniques" by Lei Yang and all for more information.</p>
 *
 * @see #setTemporalAntiAliasingOptions
 */
struct TemporalAntiAliasingOptions {
    /** @deprecated has no effect. */
    float filterWidth = 1.0f;
    /** history feedback, between 0 (maximum temporal AA) and 1 (no temporal AA). */
    float feedback = 0.12f;
    /** texturing lod bias (typically -1 or -2) */
    float lodBias = -1.0f;
    /** post-TAA sharpen, especially useful when upscaling is true. */
    float sharpness = 0.0f;
    /** enables or disables temporal anti-aliasing */
    bool enabled = false;
    /** Upscaling factor. Disables Dynamic Resolution. [BETA] */
    float upscaling = 1.0f;

    enum class BoxType : uint8_t {
        /** use an AABB neighborhood */
        AABB,
        /** use both AABB and variance */
        AABB_VARIANCE
    };

    enum class BoxClipping : uint8_t {
        /** Accurate box clipping */
        ACCURATE,
        /** clamping */
        CLAMP,
        /** no rejections (use for debugging) */
        NONE
    };

    enum class JitterPattern : uint8_t {
        /** 4-samples, rotated grid sampling */
        RGSS_X4,
        /** 4-samples, uniform grid in helix sequence */
        UNIFORM_HELIX_X4,
        /** 8-samples of halton 2,3 */
        HALTON_23_X8,
        /** 16-samples of halton 2,3 */
        HALTON_23_X16,
        /** 32-samples of halton 2,3 */
        HALTON_23_X32
    };

    /** whether to filter the history buffer */
    bool filterHistory = true;
    /** whether to apply the reconstruction filter to the input */
    bool filterInput = true;
    /** whether to use the YcoCg color-space for history rejection */
    bool useYCoCg = false;
    /** set to true for HDR content */
    bool hdr = true;
    /** type of color gamut box */
    BoxType boxType = BoxType::AABB;
    /** clipping algorithm */
    BoxClipping boxClipping = BoxClipping::ACCURATE;
    /** Jitter Pattern */
    JitterPattern jitterPattern = JitterPattern::HALTON_23_X16;
    /** High values increases ghosting artefact, lower values increases jittering, range [0.75, 1.25] */
    float varianceGamma = 1.0f;

    /** adjust the feedback dynamically to reduce flickering */
    bool preventFlickering = false;
    /** whether to apply history reprojection (debug option) */
    bool historyReprojection = true;
};

/**
 * Options for Screen-space Reflections.
 * @see #setScreenSpaceReflectionsOptions
 */
struct ScreenSpaceReflectionsOptions {
    /** ray thickness, in world units */
    float thickness = 0.1f;
    /** bias, in world units, to prevent self-intersections */
    float bias = 0.01f;
    /** maximum distance, in world units, to raycast */
    float maxDistance = 3.0f;
    /** stride, in texels, for samples along the ray. */
    float stride = 2.0f;
    bool enabled = false;
};

/**
 * Options for the  screen-space guard band.
 * <p>A guard band can be enabled to avoid some artifacts towards the edge of the screen when
 * using screen-space effects such as SSAO. Enabling the guard band reduces performance slightly.
 * Currently the guard band can only be enabled or disabled.</p>
 */
struct GuardBandOptions {
    bool enabled = false;
};

/**
 * List of available post-processing anti-aliasing techniques.
 * @see #setAntiAliasing
 * @see #getAntiAliasing
 * @see #setSampleCount
 */
enum class AntiAliasing : uint8_t {
    /** no anti aliasing performed as part of post-processing */
    NONE,
    /** FXAA is a low-quality but very efficient type of anti-aliasing. (default). */
    FXAA
};

/**
 * List of available post-processing dithering techniques.
 */
enum class Dithering : uint8_t {
    /** No dithering */
    NONE,
    /** Temporal dithering (default) */
    TEMPORAL
};

/**
 * List of available shadow mapping techniques.
 * @see #setShadowType
 */
enum class ShadowType : uint8_t {
    /** percentage-closer filtered shadows (default) */
    PCF,
    /** variance shadows */
    VSM,
    /** PCF with contact hardening simulation */
    DPCF,
    /** PCF with soft shadows and contact hardening */
    PCSS,
    // for debugging only, don't use.
    PCFd,
};

/**
 * View-level options for VSM Shadowing.
 * @see #setVsmShadowOptions
 * <b>Warning:</b> This API is still experimental and subject to change.
 */
struct VsmShadowOptions {
    /**
     * Sets the number of anisotropic samples to use when sampling a VSM shadow map. If greater
     * than 0, mipmaps will automatically be generated each frame for all lights.
     *
     * <p>The number of anisotropic samples = 2 ^ vsmAnisotropy.</p>
     */
    uint8_t anisotropy = 0;

    /**
     * Whether to generate mipmaps for all VSM shadow maps.
     */
    bool mipmapping = false;

    /**
     * The number of MSAA samples to use when rendering VSM shadow maps.
     * Must be a power-of-two and greater than or equal to 1. A value of 1 effectively turns
     * off MSAA.
     * Higher values may not be available depending on the underlying hardware.
     */
    uint8_t msaaSamples = 1;

    /**
     * Whether to use a 32-bits or 16-bits texture format for VSM shadow maps. 32-bits
     * precision is rarely needed, but it does reduces light leaks as well as "fading"
     * of the shadows in some situations. Setting highPrecision to true for a single
     * shadow map will double the memory usage of all shadow maps.
     */
    bool highPrecision = false;

    /**
     * VSM minimum variance scale, must be positive.
     */
    float minVarianceScale = 0.5f;

    /**
     * VSM light bleeding reduction amount, between 0 and 1.
     */
    float lightBleedReduction = 0.15f;
};

/**
 * View-level options for DPCF and PCSS Shadowing.
 * @see #setSoftShadowOptions
 * <b>Warning:</b> This API is still experimental and subject to change.
 */
struct SoftShadowOptions {
    /**
     * Globally scales the penumbra of all DPCF and PCSS shadows
     * Acceptable values are greater than 0
     */
    float penumbraScale = 1.0f;

    /**
     * Globally scales the computed penumbra ratio of all DPCF and PCSS shadows.
     * This effectively controls the strength of contact hardening effect and is useful for
     * artistic purposes. Higher values make the shadows become softer faster.
     * Acceptable values are equal to or greater than 1.
     */
    float penumbraRatioScale = 1.0f;
};

/**
 * Options for stereoscopic (multi-eye) rendering.
 */
struct StereoscopicOptions {
    bool enabled = false;
};

} // namespace filament

#endif //TNT_FILAMENT_OPTIONS_H
