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

#include <stdint.h>

namespace filament {

class Texture;

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
 * This structure can be used to specify the minimum scale factor used when
 * lowering the resolution of a View, and the maximum scale factor used when
 * increasing the resolution for higher quality rendering. The scale factors
 * can be controlled on each X and Y axis independently. By default, all scale
 * factors are set to 1.0.
 *
 * enabled:   enable or disables dynamic resolution on a View
 *
 * homogeneousScaling: by default the system scales the major axis first. Set this to true
 *                     to force homogeneous scaling.
 *
 * minScale:  the minimum scale in X and Y this View should use
 *
 * maxScale:  the maximum scale in X and Y this View should use
 *
 * quality:   upscaling quality.
 *            LOW: 1 bilinear tap, Medium: 4 bilinear taps, High: 9 bilinear taps (tent)
 *
 * \note
 * Dynamic resolution is only supported on platforms where the time to render
 * a frame can be measured accurately. Dynamic resolution is currently only
 * supported on Android.
 *
 * @see Renderer::FrameRateOptions
 *
 */
struct DynamicResolutionOptions {
    math::float2 minScale = math::float2(0.5f);     //!< minimum scale factors in x and y
    math::float2 maxScale = math::float2(1.0f);     //!< maximum scale factors in x and y
    float sharpness = 0.9f;                         //!< sharpness when QualityLevel::MEDIUM or higher is used [0 (disabled), 1 (sharpest)]
    bool enabled = false;                           //!< enable or disable dynamic resolution
    bool homogeneousScaling = false;                //!< set to true to force homogeneous scaling

    /**
     * Upscaling quality
     * LOW:    bilinear filtered blit. Fastest, poor quality
     * MEDIUM: AMD FidelityFX FSR1 w/ mobile optimizations
     * HIGH:   AMD FidelityFX FSR1 w/ mobile optimizations
     * ULTRA:  AMD FidelityFX FSR1
     *      FSR1 require a well anti-aliased (MSAA or TAA), noise free scene.
     *
     * The default upscaling quality is set to LOW.
     */
    QualityLevel quality = QualityLevel::LOW;
};

/**
 * Options to control the bloom effect
 *
 * enabled:     Enable or disable the bloom post-processing effect. Disabled by default.
 *
 * levels:      Number of successive blurs to achieve the blur effect, the minimum is 3 and the
 *              maximum is 12. This value together with resolution influences the spread of the
 *              blur effect. This value can be silently reduced to accommodate the original
 *              image size.
 *
 * resolution:  Resolution of bloom's minor axis. The minimum value is 2^levels and the
 *              the maximum is lower of the original resolution and 4096. This parameter is
 *              silently clamped to the minimum and maximum.
 *              It is highly recommended that this value be smaller than the target resolution
 *              after dynamic resolution is applied (horizontally and vertically).
 *
 * strength:    how much of the bloom is added to the original image. Between 0 and 1.
 *
 * blendMode:   Whether the bloom effect is purely additive (false) or mixed with the original
 *              image (true).
 *
 * anamorphism: Bloom's aspect ratio (x/y), for artistic purposes.
 *
 * threshold:   When enabled, a threshold at 1.0 is applied on the source image, this is
 *              useful for artistic reasons and is usually needed when a dirt texture is used.
 *
 * dirt:        A dirt/scratch/smudges texture (that can be RGB), which gets added to the
 *              bloom effect. Smudges are visible where bloom occurs. Threshold must be
 *              enabled for the dirt effect to work properly.
 *
 * dirtStrength: Strength of the dirt texture.
 */
struct BloomOptions {
    enum class BlendMode : uint8_t {
        ADD,           //!< Bloom is modulated by the strength parameter and added to the scene
        INTERPOLATE    //!< Bloom is interpolated with the scene using the strength parameter
    };
    Texture* dirt = nullptr;                //!< user provided dirt texture
    float dirtStrength = 0.2f;              //!< strength of the dirt texture
    float strength = 0.10f;                 //!< bloom's strength between 0.0 and 1.0
    uint32_t resolution = 360;              //!< resolution of vertical axis (2^levels to 2048)
    float anamorphism = 1.0f;               //!< bloom x/y aspect-ratio (1/32 to 32)
    uint8_t levels = 6;                     //!< number of blur levels (3 to 11)
    BlendMode blendMode = BlendMode::ADD;   //!< how the bloom effect is applied
    bool threshold = true;                  //!< whether to threshold the source
    bool enabled = false;                   //!< enable or disable bloom
    float highlight = 1000.0f;              //!< limit highlights to this value before bloom [10, +inf]

    bool lensFlare = false;                 //!< enable screen-space lens flare
    bool starburst = true;                  //!< enable starburst effect on lens flare
    float chromaticAberration = 0.005f;     //!< amount of chromatic aberration
    uint8_t ghostCount = 4;                 //!< number of flare "ghosts"
    float ghostSpacing = 0.6f;              //!< spacing of the ghost in screen units [0, 1[
    float ghostThreshold = 10.0f;           //!< hdr threshold for the ghosts
    float haloThickness = 0.1f;             //!< thickness of halo in vertical screen units, 0 to disable
    float haloRadius = 0.4f;                //!< radius of halo in vertical screen units [0, 0.5]
    float haloThreshold = 10.0f;            //!< hdr threshold for the halo
};

/**
 * Options to control fog in the scene
 */
struct FogOptions {
    float distance = 0.0f;              //!< distance in world units from the camera where the fog starts ( >= 0.0 )
    float maximumOpacity = 1.0f;        //!< fog's maximum opacity between 0 and 1
    float height = 0.0f;                //!< fog's floor in world units
    float heightFalloff = 1.0f;         //!< how fast fog dissipates with altitude
    LinearColor color{0.5f};            //!< fog's color (linear), see fogColorFromIbl
    float density = 0.1f;               //!< fog's density at altitude given by 'height'
    float inScatteringStart = 0.0f;     //!< distance in world units from the camera where in-scattering starts
    float inScatteringSize = -1.0f;     //!< size of in-scattering (>0 to activate). Good values are >> 1 (e.g. ~10 - 100).
    bool fogColorFromIbl = false;       //!< Fog color will be modulated by the IBL color in the view direction.
    bool enabled = false;               //!< enable or disable fog
};

/**
 * Options to control Depth of Field (DoF) effect in the scene.
 *
 * cocScale can be used to set the depth of field blur independently from the camera
 * aperture, e.g. for artistic reasons. This can be achieved by setting:
 *      cocScale = cameraAperture / desiredDoFAperture
 *
 * @see Camera
 */
struct DepthOfFieldOptions {
    enum class Filter : uint8_t {
        NONE = 0,
        MEDIAN = 2
    };
    float cocScale = 1.0f;              //!< circle of confusion scale factor (amount of blur)
    float maxApertureDiameter = 0.01f;  //!< maximum aperture diameter in meters (zero to disable rotation)
    bool enabled = false;               //!< enable or disable depth of field effect
    Filter filter = Filter::MEDIAN;     //!< filter to use for filling gaps in the kernel
    bool nativeResolution = false;      //!< perform DoF processing at native resolution
    /**
     * Number of of rings used by the gather kernels. The number of rings affects quality
     * and performance. The actual number of sample per pixel is defined
     * as (ringCount * 2 - 1)^2. Here are a few commonly used values:
     *       3 rings :   25 ( 5x 5 grid)
     *       4 rings :   49 ( 7x 7 grid)
     *       5 rings :   81 ( 9x 9 grid)
     *      17 rings : 1089 (33x33 grid)
     *
     * With a maximum circle-of-confusion of 32, it is never necessary to use more than 17 rings.
     *
     * Usually all three settings below are set to the same value, however, it is often
     * acceptable to use a lower ring count for the "fast tiles", which improves performance.
     * Fast tiles are regions of the screen where every pixels have a similar
     * circle-of-confusion radius.
     *
     * A value of 0 means default, which is 5 on desktop and 3 on mobile.
     *
     * @{
     */
    uint8_t foregroundRingCount = 0; //!< number of kernel rings for foreground tiles
    uint8_t backgroundRingCount = 0; //!< number of kernel rings for background tiles
    uint8_t fastGatherRingCount = 0; //!< number of kernel rings for fast tiles
    /** @}*/

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
    float midPoint = 0.5f;                      //!< high values restrict the vignette closer to the corners, between 0 and 1
    float roundness = 0.5f;                     //!< controls the shape of the vignette, from a rounded rectangle (0.0), to an oval (0.5), to a circle (1.0)
    float feather = 0.5f;                       //!< softening amount of the vignette effect, between 0 and 1
    LinearColorA color{0.0f, 0.0f, 0.0f, 1.0f}; //!< color of the vignette effect, alpha is currently ignored
    bool enabled = false;                       //!< enables or disables the vignette effect
};

/**
 * Structure used to set the precision of the color buffer and related quality settings.
 *
 * @see setRenderQuality, getRenderQuality
 */
struct RenderQuality {
    /**
     * Sets the quality of the HDR color buffer.
     *
     * A quality of HIGH or ULTRA means using an RGB16F or RGBA16F color buffer. This means
     * colors in the LDR range (0..1) have a 10 bit precision. A quality of LOW or MEDIUM means
     * using an R11G11B10F opaque color buffer or an RGBA16F transparent color buffer. With
     * R11G11B10F colors in the LDR range have a precision of either 6 bits (red and green
     * channels) or 5 bits (blue channel).
     */
    QualityLevel hdrColorBuffer = QualityLevel::HIGH;
};

/**
 * Options for screen space Ambient Occlusion (SSAO) and Screen Space Cone Tracing (SSCT)
 * @see setAmbientOcclusionOptions()
 */
struct AmbientOcclusionOptions {
    float radius = 0.3f;    //!< Ambient Occlusion radius in meters, between 0 and ~10.
    float power = 1.0f;     //!< Controls ambient occlusion's contrast. Must be positive.
    float bias = 0.0005f;   //!< Self-occlusion bias in meters. Use to avoid self-occlusion. Between 0 and a few mm.
    float resolution = 0.5f;//!< How each dimension of the AO buffer is scaled. Must be either 0.5 or 1.0.
    float intensity = 1.0f; //!< Strength of the Ambient Occlusion effect.
    float bilateralThreshold = 0.05f; //!< depth distance that constitute an edge for filtering
    QualityLevel quality = QualityLevel::LOW; //!< affects # of samples used for AO.
    QualityLevel lowPassFilter = QualityLevel::MEDIUM; //!< affects AO smoothness
    QualityLevel upsampling = QualityLevel::LOW; //!< affects AO buffer upsampling quality
    bool enabled = false;    //!< enables or disables screen-space ambient occlusion
    bool bentNormals = false; //!< enables bent normals computation from AO, and specular AO
    float minHorizonAngleRad = 0.0f;  //!< min angle in radian to consider
    /**
     * Screen Space Cone Tracing (SSCT) options
     * Ambient shadows from dominant light
     */
    struct Ssct {
        float lightConeRad = 1.0f;          //!< full cone angle in radian, between 0 and pi/2
        float shadowDistance = 0.3f;        //!< how far shadows can be cast
        float contactDistanceMax = 1.0f;    //!< max distance for contact
        float intensity = 0.8f;             //!< intensity
        math::float3 lightDirection{ 0, -1, 0 };    //!< light direction
        float depthBias = 0.01f;        //!< depth bias in world units (mitigate self shadowing)
        float depthSlopeBias = 0.01f;   //!< depth slope bias (mitigate self shadowing)
        uint8_t sampleCount = 4;        //!< tracing sample count, between 1 and 255
        uint8_t rayCount = 1;           //!< # of rays to trace, between 1 and 255
        bool enabled = false;           //!< enables or disables SSCT
    } ssct;
};

/**
 * Options for Temporal Anti-aliasing (TAA)
 * @see setTemporalAntiAliasingOptions()
 */
struct TemporalAntiAliasingOptions {
    float filterWidth = 1.0f;   //!< reconstruction filter width typically between 0 (sharper, aliased) and 1 (smoother)
    float feedback = 0.04f;     //!< history feedback, between 0 (maximum temporal AA) and 1 (no temporal AA).
    bool enabled = false;       //!< enables or disables temporal anti-aliasing
};

/**
 * List of available post-processing anti-aliasing techniques.
 * @see setAntiAliasing, getAntiAliasing, setSampleCount
 */
enum class AntiAliasing : uint8_t {
    NONE = 0,   //!< no anti aliasing performed as part of post-processing
    FXAA = 1    //!< FXAA is a low-quality but very efficient type of anti-aliasing. (default).
};

/**
 * List of available post-processing dithering techniques.
 */
enum class Dithering : uint8_t {
    NONE = 0,       //!< No dithering
    TEMPORAL = 1    //!< Temporal dithering (default)
};

/**
 * List of available shadow mapping techniques.
 * @see setShadowType
 */
enum class ShadowType : uint8_t {
    PCF,        //!< percentage-closer filtered shadows (default)
    VSM         //!< variance shadows
};

/**
 * View-level options for VSM Shadowing.
 * @see setVsmShadowOptions()
 * @warning This API is still experimental and subject to change.
 */
struct VsmShadowOptions {
    /**
     * Sets the number of anisotropic samples to use when sampling a VSM shadow map. If greater
     * than 0, mipmaps will automatically be generated each frame for all lights.
     *
     * The number of anisotropic samples = 2 ^ vsmAnisotropy.
     */
    uint8_t anisotropy = 0;

    /**
     * Whether to generate mipmaps for all VSM shadow maps.
     */
    bool mipmapping = false;

    /**
     * EVSM exponent.
     * The maximum value permissible is 5.54 for a shadow map in fp16, or 42.0 for a
     * shadow map in fp32. Currently the shadow map bit depth is always fp16.
     */
    float exponent = 5.54f;

    /**
     * VSM minimum variance scale, must be positive.
     */
    float minVarianceScale = 0.5f;

    /**
     * VSM light bleeding reduction amount, between 0 and 1.
     */
    float lightBleedReduction = 0.15f;
};

} // namespace filament

#endif //TNT_FILAMENT_OPTIONS_H
