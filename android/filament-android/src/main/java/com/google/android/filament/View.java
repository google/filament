/*
 * Copyright (C) 2017 The Android Open Source Project
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

package com.google.android.filament;

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;

import java.util.EnumSet;

import static com.google.android.filament.Asserts.assertFloat3In;
import static com.google.android.filament.Asserts.assertFloat4In;
import static com.google.android.filament.Colors.LinearColor;

/**
 * Encompasses all the state needed for rendering a {@link Scene}.
 *
 * <p>
 * {@link Renderer#render} operates on <code>View</code> objects. These <code>View</code> objects
 * specify important parameters such as:
 * </p>
 *
 * <ul>
 * <li>The Scene</li>
 * <li>The Camera</li>
 * <li>The Viewport</li>
 * <li>Some rendering parameters</li>
 * </ul>
 *
 * <p>
 * <code>View</code> instances are heavy objects that internally cache a lot of data needed for
 * rendering. It is not advised for an application to use many View objects.
 * </p>
 *
 * <p>
 * For example, in a game, a <code>View</code> could be used for the main scene and another one for
 * the game's user interface. More <code>View</code> instances could be used for creating special
 * effects (e.g. a <code>View</code> is akin to a rendering pass).
 * </p>
 *
 * @see Renderer
 * @see Scene
 * @see Camera
 * @see RenderTarget
 */
public class View {
    private long mNativeObject;
    private String mName;
    private Scene mScene;
    private Camera mCamera;
    private Viewport mViewport = new Viewport(0, 0, 0, 0);
    private DynamicResolutionOptions mDynamicResolution;
    private RenderQuality mRenderQuality;
    private AmbientOcclusionOptions mAmbientOcclusionOptions;
    private BloomOptions mBloomOptions;
    private FogOptions mFogOptions;
    private RenderTarget mRenderTarget;
    private BlendMode mBlendMode;
    private DepthOfFieldOptions mDepthOfFieldOptions;
    private VignetteOptions mVignetteOptions;
    private ColorGrading mColorGrading;
    private TemporalAntiAliasingOptions mTemporalAntiAliasingOptions;
    private VsmShadowOptions mVsmShadowOptions;

    /**
     * Generic quality level.
     */
    public enum QualityLevel {
        LOW,
        MEDIUM,
        HIGH,
        ULTRA
    }

    public enum BlendMode {
        OPAQUE,
        TRANSLUCENT
    }

    /**
     * Dynamic resolution can be used to either reach a desired target frame rate by lowering the
     * resolution of a <code>View</code>, or to increase the quality when the rendering is faster
     * than the target frame rate.
     *
     * <p>
     * This structure can be used to specify the minimum scale factor used when lowering the
     * resolution of a <code>View</code>, and the maximum scale factor used when increasing the
     * resolution for higher quality rendering. The scale factors can be controlled on each X and Y
     * axis independently. By default, all scale factors are set to 1.0.
     * </p>
     *
     * <p>
     * Dynamic resolution is only supported on platforms where the time to render a frame can be
     * measured accurately. Dynamic resolution is currently only supported on Android.
     * </p>
     */
    public static class DynamicResolutionOptions {
        /**
         * Enables or disables dynamic resolution on a View.
         */
        public boolean enabled = false;

        /**
         * If false, the system scales the major axis first.
         */
        public boolean homogeneousScaling = false;

        /**
         * The minimum scale in X and Y this View should use.
         */
        public float minScale = 0.5f;

        /**
         * The maximum scale in X and Y this View should use.
         */
        public float maxScale = 1.0f;

        /**
         * Sharpness when QualityLevel.ULTRA is used [0, 2].
         * 0 is the sharpest setting, 2 is the smoothest setting.
         * The default is set to 0.2
         */
        public float sharpness = 0.2f;

        /**
         * Upscaling quality
         * LOW: bilinear filtered blit. Fastest, poor quality
         * MEDIUM: 16-tap optimized tent filter.
         * HIGH: 36-tap optimized tent filter.
         * ULTRA: AMD FidelityFX FSR1. Slowest, very high quality.
         *      Requires a well anti-aliased (MSAA or TAA), noise free scene.
         *
         * The default upscaling quality is set to LOW.
         */
        @NonNull
        public QualityLevel quality = QualityLevel.LOW;
    }

    /**
     * Options for screen space Ambient Occlusion
     */
    public static class AmbientOcclusionOptions {
        /**
         * Ambient Occlusion radius in meters, between 0 and ~10.
         */
        public float radius = 0.3f;

        /**
         * Self-occlusion bias in meters. Use to avoid self-occlusion. Between 0 and a few mm.
         */
        public float bias = 0.0005f;

        /**
         * Controls ambient occlusion's contrast. Must be positive. Default is 1.
         * Good values are between 0.5 and 3.
         */
        public float power = 1.0f;

        /**
         * How each dimension of the AO buffer is scaled. Must be either 0.5 or 1.0.
         */
        public float resolution = 0.5f;

        /**
         * Strength of the Ambient Occlusion effect. Must be positive.
         */
        public float intensity = 1.0f;

        /**
         * Depth distance that constitute an edge for filtering. Must be positive.
         * Default is 5cm.
         * This must be adjusted with the scene's scale and/or units.
         * A value too low will result in high frequency noise, while a value too high will
         * result in the loss of geometry edges. For AO, it is generally better to be too
         * blurry than not enough.
         */
        public float bilateralThreshold = 0.05f;

        /**
         * The quality setting controls the number of samples used for evaluating Ambient
         * occlusion. The default is QualityLevel.LOW which is sufficient for most mobile
         * applications.
         */
        @NonNull
        public QualityLevel quality = QualityLevel.LOW;

        /**
         * The lowPassFilter setting controls the quality of the low pass filter applied to
         * AO estimation. The default is QualityLevel.MEDIUM which is sufficient for most mobile
         * applications. QualityLevel.LOW disables the filter entirely.
         */
        @NonNull
        public QualityLevel lowPassFilter = QualityLevel.MEDIUM;

        /**
         * The upsampling setting controls the quality of the ambient occlusion buffer upsampling.
         * The default is QualityLevel.LOW and uses bilinear filtering, a value of
         * QualityLevel.HIGH or more enables a better bilateral filter.
         */
        @NonNull
        public QualityLevel upsampling = QualityLevel.LOW;

        /**
         * enable or disable screen space ambient occlusion
         */
        public boolean enabled = false;

        /**
         * enables bent normals computation from AO, and specular AO
         */
        public boolean bentNormals = false;

        /**
         * Minimal angle to consider in radian. This is used to reduce the creases that can
         * appear due to insufficiently tessellated geometry.
         * For e.g. a good values to try could be around 0.2.
         */
        public float minHorizonAngleRad = 0.0f;


       /**
        * Full cone angle in radian, between 0 and pi/2. This affects the softness of the shadows,
        * as well as how far they are cast. A smaller angle yields to sharper and shorter shadows.
        * The default angle is about 60 degrees.
        */
       public float ssctLightConeRad = 1.0f;

       /**
        * Distance from where tracing starts.
        * This affects how far shadows are cast.
        */
       public float ssctStartTraceDistance = 0.01f;

       /**
        * Maximum contact distance with the cone. Intersections between the traced cone and
        * geometry samller than this distance are ignored.
        */
       public float ssctContactDistanceMax = 1.0f;

       /**
        * Intensity of the shadows.
        */
       public float ssctIntensity = 0.8f;

       /**
        * Light direction.
        */
       @NonNull @Size(min = 3)
       public float[] ssctLightDirection = { 0, -1, 0 };

       /**
        * Depth bias in world units (mitigate self shadowing)
        */
       public float ssctDepthBias = 0.01f;

       /**
        * Depth slope bias (mitigate self shadowing)
        */
       public float ssctDepthSlopeBias = 0.01f;

       /**
        * Tracing sample count, between 1 and 255. This affects the quality as well as the
        * distance of the shadows.
        */
       public int ssctSampleCount = 4;

       /**
        * Numbers of rays to trace, between 1 and 255. This affects the noise of the shadows.
        * Performance degrades quickly with this value.
        */
       public int ssctRayCount = 1;

       /**
        * Enables or disables SSCT.
        */
       public boolean ssctEnabled = false;
    }

    /**
     * Options for Temporal Anti-aliasing (TAA)
     * @see View#setTemporalAntiAliasingOptions
     */
    public static class TemporalAntiAliasingOptions {
        /** reconstruction filter width typically between 0 (sharper, aliased) and 1 (smoother) */
        public float filterWidth = 1.0f;

        /** history feedback, between 0 (maximum temporal AA) and 1 (no temporal AA). */
        public float feedback = 0.04f;

        /** enables or disables temporal anti-aliasing */
        public boolean enabled = false;
    };

    /**
     * Options for controlling the Bloom effect
     *
     * enabled:     Enable or disable the bloom post-processing effect. Disabled by default.
     * levels:      Number of successive blurs to achieve the blur effect, the minimum is 3 and the
     *              maximum is 11. This value together with resolution influences the spread of the
     *              blur effect. This value can be silently reduced to accommodate the original
     *              image size.
     * resolution:  Resolution of bloom's vertical axis. The minimum value is 2^levels and the
     *              the maximum is lower of the original resolution and 2048. This parameter is
     *              silently clamped to the minimum and maximum.
     *              It is highly recommended that this value be smaller than the target resolution
     *              after dynamic resolution is applied (horizontally and vertically).
     * strength:    how much of the bloom is added to the original image. Between 0 and 1.
     * blendMode:   Whether the bloom effect is purely additive (false) or mixed with the original
     *              image (true).
     * anamorphism: Bloom's aspect ratio (x/y), for artistic purposes.
     * threshold:   When enabled, a threshold at 1.0 is applied on the source image, this is
     *              useful for artistic reasons and is usually needed when a dirt texture is used.
     * dirt:        A dirt/scratch/smudges texture (that can be RGB), which gets added to the
     *              bloom effect. Smudges are visible where bloom occurs. Threshold must be
     *              enabled for the dirt effect to work properly.
     * dirtStrength: Strength of the dirt texture.
     *
     * @see View#setBloomOptions
     */
    public static class BloomOptions {

        public enum BlendingMode {
            ADD,
            INTERPOLATE
        }

        /**
         * User provided dirt texture
         */
        @Nullable
        public Texture dirt = null;

        /**
         * strength of the dirt texture
         */
        public float dirtStrength = 0.2f;

        /**
         * Strength of the bloom effect, between 0.0 and 1.0
         */
        public float strength = 0.10f;

        /**
         * Resolution of minor axis (2^levels to 2048)
         */
        public int resolution = 360;

        /**
         * Bloom x/y aspect-ratio (1/32 to 32)
         */
        public float anamorphism = 1.0f;

        /**
         * Number of blur levels (3 to 11)
         */
        public int levels = 6;

        /**
         * How the bloom effect is applied
         */
        public BlendingMode blendingMode = BlendingMode.ADD;

        /**
         * Whether to threshold the source
         */
        public boolean threshold = true;

        /**
         * enable or disable bloom
         */
        public boolean enabled = false;

        /**
         * limit highlights to this value before bloom. Use +inf for no limiting.
         * minimum value is 10.0.
         */
        public float highlight = 1000.0f;


        /**
         * enable screen-space lens flare
         */
        public boolean lensFlare = false;

        /**
         * enable starburst effect on lens flare
         */
        public boolean starburst = true;

        /**
         * amount of chromatic aberration
         */
        public float chromaticAberration = 0.005f;

        /**
         * number of flare "ghosts"
         */
        public int ghostCount = 4;

        /**
         * spacing of the ghost in screen units [0, 1[
         */
        public float ghostSpacing = 0.6f;

        /**
         * hdr threshold for the ghosts
         */
        public float ghostThreshold = 10.0f;

        /**
         * thickness of halo in vertical screen units, 0 to disable
         */
        public float haloThickness = 0.1f;

        /**
         * radius of halo in vertical screen units [0, 0.5]
         */
        public float haloRadius = 0.4f;

        /**
         * hdr threshold for the halo
         */
        public float haloThreshold = 10.0f;
    }

    /**
     * Options to control fog in the scene
     *
     * @see View#setFogOptions
     */
    public static class FogOptions {
        /**
         * distance in world units from the camera where the fog starts ( >= 0.0 )
         */
        public float distance = 0.0f;

        /**
         * fog's maximum opacity between 0 and 1
         */
        public float maximumOpacity = 1.0f;

        /**
         * fog's floor in world units
         */
        public float height = 0.0f;

        /**
         * how fast fog dissipates with altitude
         */
        public float heightFalloff = 1.0f;

        /**
         * Fog's color as a linear RGB color.
         */
        @NonNull
        @Size(min = 3)
        public float[] color = { 0.5f, 0.5f, 0.5f };

        /**
         * fog's density at altitude given by 'height'
         */
        public float density = 0.1f;

        /**
         * distance in world units from the camera where in-scattering starts
         */
        public float inScatteringStart = 0.0f;

        /**
         * size of in-scattering (>0 to activate). Good values are >> 1 (e.g. ~10 - 100)
         */
        public float inScatteringSize = -1.0f;

        /**
         * fog color will be modulated by the IBL color in the view direction
         */
        public boolean fogColorFromIbl = false;

        /**
         * enable or disable fog
         */
        public boolean enabled = false;
    }

    /**
     * Options to control Depth of Field (DoF) effect in the scene
     *
     * @see View#setDepthOfFieldOptions
     */
    public static class DepthOfFieldOptions {

        public enum Filter {
            NONE,
            MEDIAN
        }

        /**
         * circle of confusion scale factor (amount of blur)
         *
         * <p>cocScale can be used to set the depth of field blur independently from the camera
         * aperture, e.g. for artistic reasons. This can be achieved by setting:</p>
         * <code>
         *      cocScale = cameraAperture / desiredDoFAperture
         * </code>
         *
         */
        public float cocScale = 1.0f;

        /** maximum aperture diameter in meters (zero to disable bokeh rotation) */
        public float maxApertureDiameter = 0.01f;

        /** enable or disable Depth of field effect */
        public boolean enabled = false;

        /** filter to use for filling gaps in the kernel */
        @NonNull
        public Filter filter = Filter.MEDIAN;

        /** perform DoF processing at native resolution */
        public boolean nativeResolution = false;

        /**
         * <p>Number of of rings used by the foreground kernel. The number of rings affects quality
         * and performance. The actual number of sample per pixel is defined
         * as (ringCount * 2 - 1)^2. Here are a few commonly used values:</p>
         *       3 rings :   25 ( 5x 5 grid)
         *       4 rings :   49 ( 7x 7 grid)
         *       5 rings :   81 ( 9x 9 grid)
         *      17 rings : 1089 (33x33 grid)
         *
         * <p>With a maximum circle-of-confusion of 32, it is never necessary to use more than 17 rings.</p>
         *
         * <p>Usually all three settings below are set to the same value, however, it is often
         * acceptable to use a lower ring count for the "fast tiles", which improves performance.
         * Fast tiles are regions of the screen where every pixels have a similar
         * circle-of-confusion radius.</p>
         *
         * <p>A value of 0 means default, which is 5 on desktop and 3 on mobile.</p>
         */
        public int foregroundRingCount = 0;

        /**
         * Number of of rings used by the background kernel. The number of rings affects quality
         * and performance.
         * @see #foregroundRingCount
         */
        public int backgroundRingCount = 0;

        /**
         * Number of of rings used by the fast gather kernel. The number of rings affects quality
         * and performance.
         * @see #foregroundRingCount
         */
        public int fastGatherRingCount = 0;

        /**
         * maximum circle-of-confusion in pixels for the foreground, must be in [0, 32] range.
         * A value of 0 means default, which is 32 on desktop and 24 on mobile.
         */
        public int maxForegroundCOC = 0;

        /**
         * maximum circle-of-confusion in pixels for the background, must be in [0, 32] range.
         * A value of 0 means default, which is 32 on desktop and 24 on mobile.
         */
        public int maxBackgroundCOC = 0;
    };

    /**
     * Options to control the vignetting effect.
     */
    public static class VignetteOptions {
        /**
         * High values restrict the vignette closer to the corners, between 0 and 1.
         */
        public float midPoint = 0.5f;

        /**
         * Controls the shape of the vignette, from a rounded rectangle (0.0), to an oval (0.5),
         * to a circle (1.0). The value must be between 0 and 1.
         */
        public float roundness = 0.5f;

        /**
         * Softening amount of the vignette effect, between 0 and 1.
         */
        public float feather = 0.5f;

        /**
         * Color of the vignette effect as a linear RGBA color. The alpha channel is currently
         * ignored.
         */
        @NonNull
        @Size(min = 4)
        public float[] color = { 0.0f, 0.0f, 0.0f, 1.0f };

        /**
         * Enables or disables the vignette effect.
         */
        public boolean enabled = false;
    }

    /**
     * Structure used to set the color precision for the rendering of a <code>View</code>.
     *
     * <p>
     * This structure offers separate quality settings for different parts of the rendering
     * pipeline.
     * </p>
     *
     * @see #setRenderQuality
     * @see #getRenderQuality
     */
    public static class RenderQuality {
        /**
          * <p>
          * A quality of <code>HIGH</code> or <code>ULTRA</code> means using an RGB16F or RGBA16F color
          * buffer. This means colors in the LDR range (0..1) have 10 bit precision. A quality of
          * <code>LOW</code> or <code>MEDIUM</code> means using an R11G11B10F opaque color buffer or an
          * RGBA16F transparent color buffer. With R11G11B10F colors in the LDR range have a precision of
          * either 6 bits (red and green channels) or 5 bits (blue channel).
          * </p>
          */
        public QualityLevel hdrColorBuffer = QualityLevel.HIGH;
    }

    /**
     * List of available ambient occlusion techniques.
     * @deprecated use setAmbientOcclusionOptions instead
     * @see #setAmbientOcclusion
     */
    @Deprecated
    public enum AmbientOcclusion {
        NONE,
        SSAO
    }

    /**
     * List of available post-processing anti-aliasing techniques.
     *
     * @see #setAntiAliasing
     * @see #getAntiAliasing
     */
    public enum AntiAliasing {
        /**
         * No anti aliasing performed as part of post-processing.
         */
        NONE,

        /**
         * FXAA is a low-quality but very efficient type of anti-aliasing. (default).
         */
        FXAA
    }

    /**
     * List of available tone-mapping operators
     *
     * @deprecated Use ColorGrading instead
     */
    @Deprecated
    public enum ToneMapping {
        /**
         * Equivalent to disabling tone-mapping.
         */
        LINEAR,

        /**
         * The Academy Color Encoding System (ACES).
         */
        ACES
    }

    /**
     * List of available post-processing dithering techniques.
     */
    public enum Dithering {
        NONE,
        TEMPORAL
    }

    /**
     * List of available shadow mapping techniques.
     *
     * @see #setShadowType
     */
    public enum ShadowType {
        /**
         * Percentage-closer filtered shadows (default).
         */
        PCF,

        /**
         * Variance shadows.
         */
        VSM
    }

    /**
     * View-level options for VSM shadowing.
     *
     * <strong>Warning: This API is still experimental and subject to change.</strong>
     *
     * @see View#setVsmShadowOptions
     */
    public static class VsmShadowOptions {
        /**
         * Sets the number of anisotropic samples to use when sampling a VSM shadow map. If greater
         * than 0, mipmaps will automatically be generated each frame for all lights.
         * This implies mipmapping below.
         *
         * <p>
         * The number of anisotropic samples = 2 ^ vsmAnisotropy.
         * </p>
         *
         */
        public int anisotropy = 0;

        /**
         * Whether to generate mipmaps for all VSM shadow maps.
         */
        public boolean mipmapping = false;

        /**
         * EVSM exponent
         * The maximum value permissible is 5.54 for a shadow map in fp16, or 42.0 for a
         * shadow map in fp32. Currently the shadow map bit depth is always fp16.
         */
        public float exponent = 5.54f;

        /**
         * VSM minimum variance scale, must be positive.
         */
        public float minVarianceScale = 1.0f;

        /**
         * VSM light bleeding reduction amount, between 0 and 1.
         */
        public float lightBleedReduction = 0.2f;
    }

    /**
     * Used to select buffers.
     */
    public enum TargetBufferFlags {
        /**
         * Color 0 buffer selected.
         */
        COLOR0(0x1),
        /**
         * Color 1 buffer selected.
         */
        COLOR1(0x2),
        /**
         * Color 2 buffer selected.
         */
        COLOR2(0x4),
        /**
         * Color 3 buffer selected.
         */
        COLOR3(0x8),
        /**
         * Depth buffer selected.
         */
        DEPTH(0x10),
        /**
         * Stencil buffer selected.
         */
        STENCIL(0x20);

        /*
         * No buffer selected
         */
        public static EnumSet<TargetBufferFlags> NONE = EnumSet.noneOf(TargetBufferFlags.class);

        /*
         * All color buffers selected
         */
        public static EnumSet<TargetBufferFlags> ALL_COLOR =
                EnumSet.of(COLOR0, COLOR1, COLOR2, COLOR3);
        /**
         * Depth and stencil buffer selected.
         */
        public static EnumSet<TargetBufferFlags> DEPTH_STENCIL = EnumSet.of(DEPTH, STENCIL);
        /**
         * All buffers are selected.
         */
        public static EnumSet<TargetBufferFlags> ALL = EnumSet.range(COLOR0, STENCIL);

        private int mFlags;

        TargetBufferFlags(int flags) {
            mFlags = flags;
        }

        static int flags(EnumSet<TargetBufferFlags> flags) {
            int result = 0;
            for (TargetBufferFlags flag : flags) {
                result |= flag.mFlags;
            }
            return result;
        }
    }

    View(long nativeView) {
        mNativeObject = nativeView;
    }

    /**
     * Sets the View's name. Only useful for debugging.
     */
    public void setName(@NonNull String name) {
        mName = name;
        nSetName(getNativeObject(), name);
    }

    /**
     * Returns the View's name.
     */
    @Nullable
    public String getName() {
        return mName;
    }

    /**
     * Sets this View instance's Scene.
     *
     * <p>
     * This method associates the specified Scene with this View. Note that a particular scene can
     * be associated with several View instances. To remove an existing association, simply pass
     * null.
     * </p>
     *
     * <p>
     * The View does not take ownership of the Scene pointer. Before destroying a Scene, be sure
     * to remove it from all assoicated Views.
     * </p>
     *
     * @see #getScene
     */
    public void setScene(@Nullable Scene scene) {
        mScene = scene;
        nSetScene(getNativeObject(), scene == null ? 0 : scene.getNativeObject());
    }

    /**
     * Gets this View's associated Scene, or null if none has been assigned.
     *
     * @see #setScene
     */
    @Nullable
    public Scene getScene() {
        return mScene;
    }

    /**
     * Sets this View's Camera.
     *
     * <p>
     * This method associates the specified Camera with this View. A Camera can be associated with
     * several View instances. To remove an existing association, simply pass
     * null.
     * </p>
     *
     * <p>
     * The View does not take ownership of the Scene pointer. Before destroying a Camera, be sure
     * to remove it from all assoicated Views.
     * </p>
     *
     * @see #getCamera
     */
    public void setCamera(@Nullable Camera camera) {
        mCamera = camera;
        nSetCamera(getNativeObject(), camera == null ? 0 : camera.getNativeObject());
    }

    /**
     * Gets this View's associated Camera, or null if none has been assigned.
     *
     * @see #setCamera
     */
    @Nullable
    public Camera getCamera() {
        return mCamera;
    }

    /**
     * Specifies the rectangular rendering area.
     *
     * <p>
     * The viewport specifies where the content of the View (i.e. the Scene) is rendered in
     * the render target. The render target is automatically clipped to the Viewport.
     * </p>
     *
     * <p>
     * If you wish subsequent changes to take effect please call this method again in order to
     * propagate the changes down to the native layer.
     * </p>
     *
     * @param viewport  The Viewport to render the Scene into.
     */
    public void setViewport(@NonNull Viewport viewport) {
        mViewport = viewport;
        nSetViewport(getNativeObject(),
                mViewport.left, mViewport.bottom, mViewport.width, mViewport.height);
    }

    /**
     * Returns the rectangular rendering area.
     *
     * @see #setViewport
     */
    @NonNull
    public Viewport getViewport() {
        return mViewport;
    }

    /**
     * Sets the blending mode used to draw the view into the SwapChain.
     *
     * @param blendMode either {@link BlendMode#OPAQUE} or {@link BlendMode#TRANSLUCENT}
     * @see #getBlendMode
     */
    public void setBlendMode(BlendMode blendMode) {
        mBlendMode = blendMode;
        nSetBlendMode(getNativeObject(), blendMode.ordinal());
    }

    /**
     *
     * @return blending mode set by setBlendMode
     * @see #setBlendMode
     */
    public BlendMode getBlendMode() {
        return mBlendMode;
    }

    /**
     * Sets which layers are visible.
     *
     * <p>
     * Renderable objects can have one or several layers associated to them. Layers are
     * represented with an 8-bits bitmask, where each bit corresponds to a layer.
     * By default all layers are visible.
     * </p>
     *
     * @see RenderableManager#setLayerMask
     *
     * @param select    a bitmask specifying which layer to set or clear using <code>values</code>.
     * @param values    a bitmask where each bit sets the visibility of the corresponding layer
     *                  (1: visible, 0: invisible), only layers in <code>select</code> are affected.
     */
    public void setVisibleLayers(
            @IntRange(from = 0, to = 255) int select,
            @IntRange(from = 0, to = 255) int values) {
        nSetVisibleLayers(getNativeObject(), select & 0xFF, values & 0xFF);
    }

    /**
     * Enables or disables shadow mapping. Enabled by default.
     *
     * @see LightManager.Builder#castShadows
     * @see RenderableManager.Builder#receiveShadows
     * @see RenderableManager.Builder#castShadows
     */
    public void setShadowingEnabled(boolean enabled) {
        nSetShadowingEnabled(getNativeObject(), enabled);
    }

    /**
     * @return whether shadowing is enabled
     */
    boolean isShadowingEnabled() {
        return nIsShadowingEnabled(getNativeObject());
    }

    /**
     * Enables or disables screen space refraction. Enabled by default.
     *
     * @param enabled true enables screen space refraction, false disables it.
     */
    public void setScreenSpaceRefractionEnabled(boolean enabled) {
        nSetScreenSpaceRefractionEnabled(getNativeObject(), enabled);
    }

    /**
     * @return whether screen space refraction is enabled
     */
    boolean isScreenSpaceRefractionEnabled() {
        return nIsScreenSpaceRefractionEnabled(getNativeObject());
    }

    /**
     * Specifies an offscreen render target to render into.
     *
     * <p>
     * By default, the view's associated render target is null, which corresponds to the
     * SwapChain associated with the engine.
     * </p>
     *
     * <p>
     * A view with a custom render target cannot rely on Renderer.ClearOptions, which only applies
     * to the SwapChain. Such view can use a Skybox instead.
     * </p>
     *
     * @param target render target associated with view, or null for the swap chain
     */
    public void setRenderTarget(@Nullable RenderTarget target) {
        mRenderTarget = target;
        nSetRenderTarget(getNativeObject(), target != null ? target.getNativeObject() : 0);
    }

    /**
     * Gets the offscreen render target associated with this view.
     *
     * Returns null if the render target is the swap chain (which is default).
     *
     * @see #setRenderTarget
     */
    @Nullable
    public RenderTarget getRenderTarget() {
        return mRenderTarget;
    }

    /**
     * Sets how many samples are to be used for MSAA in the post-process stage.
     * Default is 1 and disables MSAA.
     *
     * <p>
     * Note that anti-aliasing can also be performed in the post-processing stage, generally at
     * lower cost. See the FXAA option in {@link #setAntiAliasing}.
     * </p>
     *
     * @param count number of samples to use for multi-sampled anti-aliasing.
     */
    public void setSampleCount(int count) {
        nSetSampleCount(getNativeObject(), count);
    }

    /**
     * Returns the effective MSAA sample count.
     *
     * <p>
     * A value of 0 or 1 means MSAA is disabled.
     * </p>
     *
     * @return value set by {@link #setSampleCount}
     */
    public int getSampleCount() {
        return nGetSampleCount(getNativeObject());
    }

    /**
     * Enables or disables anti-aliasing in the post-processing stage. Enabled by default.
     *
     * <p>
     * For MSAA anti-aliasing, see {@link #setSampleCount}.
     * </p>
     *
     * @param type FXAA for enabling, NONE for disabling anti-aliasing.
     */
    public void setAntiAliasing(@NonNull AntiAliasing type) {
        nSetAntiAliasing(getNativeObject(), type.ordinal());
    }

    /**
     * Queries whether anti-aliasing is enabled during the post-processing stage. To query
     * whether MSAA is enabled, see {@link #getSampleCount}.
     *
     * @return The post-processing anti-aliasing method.
     */
    @NonNull
    public AntiAliasing getAntiAliasing() {
        return AntiAliasing.values()[nGetAntiAliasing(getNativeObject())];
    }

    /**
     * Enables or disable temporal anti-aliasing (TAA). Disabled by default.
     *
     * @param options temporal anti-aliasing options
     */
    public void setTemporalAntiAliasingOptions(@NonNull TemporalAntiAliasingOptions options) {
        mTemporalAntiAliasingOptions = options;
        nSetTemporalAntiAliasingOptions(getNativeObject(),
                options.feedback, options.filterWidth, options.enabled);
    }

    /**
     * Returns temporal anti-aliasing options.
     *
     * @return temporal anti-aliasing options
     */
    @NonNull
    public TemporalAntiAliasingOptions getTemporalAntiAliasingOptions() {
        if (mTemporalAntiAliasingOptions == null) {
            mTemporalAntiAliasingOptions = new TemporalAntiAliasingOptions();
        }
        return mTemporalAntiAliasingOptions;
    }

    /**
     * Enables or disables tone-mapping in the post-processing stage. Enabled by default.
     *
     * @param type Tone-mapping function.
     *
     * @deprecated Use {@link #setColorGrading(com.google.android.filament.ColorGrading)}
     */
    @Deprecated
    public void setToneMapping(@NonNull ToneMapping type) {
    }

    /**
     * Returns the tone-mapping function.
     * @return tone-mapping function.
     *
     * @deprecated Use {@link #getColorGrading()}. This always returns {@link ToneMapping#ACES}
     */
    @Deprecated
    @NonNull
    public ToneMapping getToneMapping() {
        return ToneMapping.ACES;
    }

    /**
     * Sets this View's color grading transforms.
     *
     * @param colorGrading Associate the specified {@link ColorGrading} to this view. A ColorGrading
     *                     can be associated to several View instances. Can be null to dissociate
     *                     the currently set ColorGrading from this View. Doing so will revert to
     *                     the use of the default color grading transforms.
     */
    public void setColorGrading(@Nullable ColorGrading colorGrading) {
        nSetColorGrading(getNativeObject(),
                colorGrading != null ? colorGrading.getNativeObject() : 0);
        mColorGrading = colorGrading;
    }

    /**
     * Returns the {@link ColorGrading} associated to this view.
     *
     * @return A {@link ColorGrading} or null if the default {@link ColorGrading} is in use
     */
    public ColorGrading getColorGrading() {
        return mColorGrading;
    }

    /**
     * Enables or disables dithering in the post-processing stage. Enabled by default.
     *
     * @param dithering dithering type
     */
    public void setDithering(@NonNull Dithering dithering) {
        nSetDithering(getNativeObject(), dithering.ordinal());
    }

    /**
     * Queries whether dithering is enabled during the post-processing stage.
     *
     * @return the current dithering type for this view.
     */
    @NonNull
    public Dithering getDithering() {
        return Dithering.values()[nGetDithering(getNativeObject())];
    }

    /**
     * Sets the dynamic resolution options for this view.
     *
     * <p>
     * Dynamic resolution options controls whether dynamic resolution is enabled, and if it is,
     * how it behaves.
     * </p>
     *
     * <p>
     * If you wish subsequent changes to take effect please call this method again in order to
     * propagate the changes down to the native layer.
     * </p>
     *
     * @param options The dynamic resolution options to use on this view
     */
    public void setDynamicResolutionOptions(@NonNull DynamicResolutionOptions options) {
        mDynamicResolution = options;
        nSetDynamicResolutionOptions(getNativeObject(),
                options.enabled,
                options.homogeneousScaling,
                options.minScale,
                options.maxScale,
                options.sharpness,
                options.quality.ordinal());
    }

    /**
     * Returns the dynamic resolution options associated with this view.
     * @return value set by {@link #setDynamicResolutionOptions}.
     */
    @NonNull
    public DynamicResolutionOptions getDynamicResolutionOptions() {
        if (mDynamicResolution == null) {
            mDynamicResolution = new DynamicResolutionOptions();
        }
        return mDynamicResolution;
    }

    /**
     * Sets the rendering quality for this view (e.g. color precision).
     *
     * @param renderQuality The render quality to use on this view
     */
    public void setRenderQuality(@NonNull RenderQuality renderQuality) {
        mRenderQuality = renderQuality;
        nSetRenderQuality(getNativeObject(), renderQuality.hdrColorBuffer.ordinal());
    }

    /**
     * Returns the render quality used by this view.
     * @return value set by {@link #setRenderQuality}.
     */
    @NonNull
    public RenderQuality getRenderQuality() {
        if (mRenderQuality == null) {
            mRenderQuality = new RenderQuality();
        }
        return mRenderQuality;
    }

    /**
     * Returns true if post-processing is enabled.
     *
     * @see #setPostProcessingEnabled
     */
    public boolean isPostProcessingEnabled() {
        return nIsPostProcessingEnabled(getNativeObject());
    }

    /**
     * Enables or disables post processing. Enabled by default.
     *
     * <p>Post-processing includes:</p>
     * <ul>
     * <li>Tone-mapping & gamma encoding</li>
     * <li>Dithering</li>
     * <li>MSAA</li>
     * <li>FXAA</li>
     * <li>Dynamic scaling</li>
     * </ul>
     *
     * <p>
     * Disabling post-processing forgoes color correctness as well as anti-aliasing and
     * should only be used experimentally (e.g., for UI overlays).
     * </p>
     *
     * @param enabled true enables post processing, false disables it
     *
     * @see #setToneMapping
     * @see #setAntiAliasing
     * @see #setDithering
     * @see #setSampleCount
     */
    public void setPostProcessingEnabled(boolean enabled) {
        nSetPostProcessingEnabled(getNativeObject(), enabled);
    }

    /**
     * Returns true if post-processing is enabled.
     *
     * @see #setPostProcessingEnabled
     */
    public boolean isFrontFaceWindingInverted() {
        return nIsFrontFaceWindingInverted(getNativeObject());
    }

    /**
     * Inverts the winding order of front faces. By default front faces use a counter-clockwise
     * winding order. When the winding order is inverted, front faces are faces with a clockwise
     * winding order.
     *
     * Changing the winding order will directly affect the culling mode in materials
     * (see Material#getCullingMode).
     *
     * Inverting the winding order of front faces is useful when rendering mirrored reflections
     * (water, mirror surfaces, front camera in AR, etc.).
     *
     * @param inverted True to invert front faces, false otherwise.
     */
    public void setFrontFaceWindingInverted(boolean inverted) {
        nSetFrontFaceWindingInverted(getNativeObject(), inverted);
    }

    /**
     * Sets options relative to dynamic lighting for this view.
     *
     * <p>
     * Together <code>zLightNear</code> and <code>zLightFar</code> must be chosen so that the
     * visible influence of lights is spread between these two values.
     * </p>
     *
     * @param zLightNear Distance from the camera where the lights are expected to shine.
     *                   This parameter can affect performance and is useful because depending
     *                   on the scene, lights that shine close to the camera may not be
     *                   visible -- in this case, using a larger value can improve performance.
     *                   e.g. when standing and looking straight, several meters of the ground
     *                   isn't visible and if lights are expected to shine there, there is no
     *                   point using a short zLightNear. (Default 5m).
     *
     * @param zLightFar Distance from the camera after which lights are not expected to be visible.
     *                  Similarly to zLightNear, setting this value properly can improve
     *                  performance. (Default 100m).
     *
     */
    public void setDynamicLightingOptions(float zLightNear, float zLightFar) {
        nSetDynamicLightingOptions(getNativeObject(), zLightNear, zLightFar);
    }

    /**
     * Sets the shadow mapping technique this View uses.
     *
     * The ShadowType affects all the shadows seen within the View.
     *
     * <p>
     * {@link ShadowType#VSM} imposes a restriction on marking renderables as only shadow receivers
     * (but not casters). To ensure correct shadowing with VSM, all shadow participant renderables
     * should be marked as both receivers and casters. Objects that are guaranteed to not cast
     * shadows on themselves or other objects (such as flat ground planes) can be set to not cast
     * shadows, which might improve shadow quality.
     * </p>
     *
     * <strong>Warning: This API is still experimental and subject to change.</strong>
     */
    public void setShadowType(ShadowType type) {
        nSetShadowType(getNativeObject(), type.ordinal());
    }

    /**
     * Sets VSM shadowing options that apply across the entire View.
     *
     * Additional light-specific VSM options can be set with
     * {@link LightManager.Builder#shadowOptions}.
     *
     * Only applicable when shadow type is set to ShadowType::VSM.
     *
     * <strong>Warning: This API is still experimental and subject to change.</strong>
     *
     * @param options Options for shadowing.
     * @see #setShadowType
     */
    public void setVsmShadowOptions(@NonNull VsmShadowOptions options) {
        mVsmShadowOptions = options;
        nSetVsmShadowOptions(getNativeObject(), options.anisotropy, options.mipmapping,
                options.exponent, options.minVarianceScale, options.lightBleedReduction);
    }

    /**
     * Gets the VSM shadowing options.
     * @see #setVsmShadowOptions
     * @return VSM shadow options currently set.
     */
    @NonNull
    public VsmShadowOptions getVsmShadowOptions() {
        if (mVsmShadowOptions == null) {
            mVsmShadowOptions = new VsmShadowOptions();
        }
        return mVsmShadowOptions;
    }

    /**
     * Activates or deactivates ambient occlusion.
     * @see #setAmbientOcclusionOptions
     * @param ao Type of ambient occlusion to use.
     */
    @Deprecated
    public void setAmbientOcclusion(@NonNull AmbientOcclusion ao) {
        nSetAmbientOcclusion(getNativeObject(), ao.ordinal());
    }

    /**
     * Queries the type of ambient occlusion active for this View.
     * @see #getAmbientOcclusionOptions
     * @return ambient occlusion type.
     */
    @Deprecated
    @NonNull
    public AmbientOcclusion getAmbientOcclusion() {
        return AmbientOcclusion.values()[nGetAmbientOcclusion(getNativeObject())];
    }

    /**
     * Sets ambient occlusion options.
     *
     * @param options Options for ambient occlusion.
     */
    public void setAmbientOcclusionOptions(@NonNull AmbientOcclusionOptions options) {
        mAmbientOcclusionOptions = options;
        nSetAmbientOcclusionOptions(getNativeObject(), options.radius, options.bias, options.power,
                options.resolution, options.intensity, options.bilateralThreshold,
                options.quality.ordinal(), options.lowPassFilter.ordinal(), options.upsampling.ordinal(),
                options.enabled, options.bentNormals, options.minHorizonAngleRad);
        nSetSSCTOptions(getNativeObject(), options.ssctLightConeRad, options.ssctStartTraceDistance,
                options.ssctContactDistanceMax,  options.ssctIntensity,
                options.ssctLightDirection[0], options.ssctLightDirection[1], options.ssctLightDirection[2],
                options.ssctDepthBias, options.ssctDepthSlopeBias, options.ssctSampleCount,
                options.ssctRayCount, options.ssctEnabled);
    }

    /**
     * Gets the ambient occlusion options.
     *
     * @return ambient occlusion options currently set.
     */
    @NonNull
    public AmbientOcclusionOptions getAmbientOcclusionOptions() {
        if (mAmbientOcclusionOptions == null) {
            mAmbientOcclusionOptions = new AmbientOcclusionOptions();
        }
        return mAmbientOcclusionOptions;
    }

    /**
     * Sets bloom options.
     *
     * @param options Options for bloom.
     * @see #getBloomOptions
     */
    public void setBloomOptions(@NonNull BloomOptions options) {
        mBloomOptions = options;
        nSetBloomOptions(getNativeObject(), options.dirt != null ? options.dirt.getNativeObject() : 0,
                options.dirtStrength, options.strength, options.resolution,
                options.anamorphism, options.levels, options.blendingMode.ordinal(),
                options.threshold, options.enabled, options.highlight,
                options.lensFlare, options.starburst, options.chromaticAberration,
                options.ghostCount, options.ghostSpacing, options.ghostThreshold,
                options.haloThickness, options.haloRadius, options.haloThreshold);
    }

    /**
     * Gets the bloom options
     * @see #setBloomOptions
     *
     * @return bloom options currently set.
     */
    @NonNull
    public BloomOptions getBloomOptions() {
        if (mBloomOptions == null) {
            mBloomOptions = new BloomOptions();
        }
        return mBloomOptions;
    }

    /**
     * Sets vignette options.
     *
     * @param options Options for vignetting.
     * @see #getVignetteOptions
     */
    public void setVignetteOptions(@NonNull VignetteOptions options) {
        assertFloat4In(options.color);
        mVignetteOptions = options;
        nSetVignetteOptions(getNativeObject(),
                options.midPoint, options.roundness, options.feather,
                options.color[0], options.color[1], options.color[2], options.color[3],
                options.enabled);
    }

    /**
     * Gets the vignette options
     * @see #setVignetteOptions
     *
     * @return vignetting options currently set.
     */
    @NonNull
    public VignetteOptions getVignetteOptions() {
        if (mVignetteOptions == null) {
            mVignetteOptions = new VignetteOptions();
        }
        return mVignetteOptions;
    }

    /**
     * Sets fog options.
     *
     * @param options Options for fog.
     * @see #getFogOptions
     */
    public void setFogOptions(@NonNull FogOptions options) {
        assertFloat3In(options.color);
        mFogOptions = options;
        nSetFogOptions(getNativeObject(), options.distance, options.maximumOpacity, options.height,
                options.heightFalloff, options.color[0], options.color[1], options.color[2],
                options.density, options.inScatteringStart, options.inScatteringSize,
                options.fogColorFromIbl,
                options.enabled);
    }

    /**
     * Gets the fog options
     *
     * @return fog options currently set.
     * @see #setFogOptions
     */
    @NonNull
    public FogOptions getFogOptions() {
        if (mFogOptions == null) {
            mFogOptions = new FogOptions();
        }
        return mFogOptions;
    }


    /**
     * Sets Depth of Field options.
     *
     * @param options Options for depth of field effect.
     * @see #getDepthOfFieldOptions
     */
    public void setDepthOfFieldOptions(@NonNull DepthOfFieldOptions options) {
        mDepthOfFieldOptions = options;
        nSetDepthOfFieldOptions(getNativeObject(), options.cocScale,
                options.maxApertureDiameter, options.enabled, options.filter.ordinal(),
                options.nativeResolution, options.foregroundRingCount, options.backgroundRingCount,
                options.fastGatherRingCount, options.maxForegroundCOC, options.maxBackgroundCOC);
    }

    /**
     * Gets the Depth of Field options
     *
     * @return Depth of Field options currently set.
     * @see #setDepthOfFieldOptions
     */
    @NonNull
    public DepthOfFieldOptions getDepthOfFieldOptions() {
        if (mDepthOfFieldOptions == null) {
            mDepthOfFieldOptions = new DepthOfFieldOptions();
        }
        return mDepthOfFieldOptions;
    }


    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed View");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native void nSetName(long nativeView, String name);
    private static native void nSetScene(long nativeView, long nativeScene);
    private static native void nSetCamera(long nativeView, long nativeCamera);
    private static native void nSetViewport(long nativeView, int left, int bottom, int width, int height);
    private static native void nSetVisibleLayers(long nativeView, int select, int value);
    private static native void nSetShadowingEnabled(long nativeView, boolean enabled);
    private static native void nSetRenderTarget(long nativeView, long nativeRenderTarget);
    private static native void nSetSampleCount(long nativeView, int count);
    private static native int nGetSampleCount(long nativeView);
    private static native void nSetAntiAliasing(long nativeView, int type);
    private static native int nGetAntiAliasing(long nativeView);
    private static native void nSetDithering(long nativeView, int dithering);
    private static native int nGetDithering(long nativeView);
    private static native void nSetDynamicResolutionOptions(long nativeView, boolean enabled, boolean homogeneousScaling, float minScale, float maxScale, float sharpness, int quality);
    private static native void nSetRenderQuality(long nativeView, int hdrColorBufferQuality);
    private static native void nSetDynamicLightingOptions(long nativeView, float zLightNear, float zLightFar);
    private static native void nSetShadowType(long nativeView, int type);
    private static native void nSetVsmShadowOptions(long nativeView, int anisotropy, boolean mipmapping, float exponent, float minVarianceScale, float lightBleedReduction);
    private static native void nSetColorGrading(long nativeView, long nativeColorGrading);
    private static native void nSetPostProcessingEnabled(long nativeView, boolean enabled);
    private static native boolean nIsPostProcessingEnabled(long nativeView);
    private static native void nSetFrontFaceWindingInverted(long nativeView, boolean inverted);
    private static native boolean nIsFrontFaceWindingInverted(long nativeView);
    private static native void nSetAmbientOcclusion(long nativeView, int ordinal);
    private static native int nGetAmbientOcclusion(long nativeView);
    private static native void nSetAmbientOcclusionOptions(long nativeView, float radius, float bias, float power, float resolution, float intensity, float bilateralThreshold, int quality, int lowPassFilter, int upsampling, boolean enabled, boolean bentNormals, float minHorizonAngleRad);
    private static native void nSetSSCTOptions(long nativeView, float ssctLightConeRad, float ssctStartTraceDistance, float ssctContactDistanceMax, float ssctIntensity, float v, float v1, float v2, float ssctDepthBias, float ssctDepthSlopeBias, int ssctSampleCount, int ssctRayCount, boolean ssctEnabled);
    private static native void nSetBloomOptions(long nativeView, long dirtNativeObject, float dirtStrength, float strength, int resolution, float anamorphism, int levels, int blendMode, boolean threshold, boolean enabled, float highlight,
            boolean lensFlare, boolean starburst, float chromaticAberration, int ghostCount, float ghostSpacing, float ghostThreshold, float haloThickness, float haloRadius, float haloThreshold);
    private static native void nSetFogOptions(long nativeView, float distance, float maximumOpacity, float height, float heightFalloff, float v, float v1, float v2, float density, float inScatteringStart, float inScatteringSize, boolean fogColorFromIbl, boolean enabled);
    private static native void nSetBlendMode(long nativeView, int blendMode);
    private static native void nSetDepthOfFieldOptions(long nativeView, float cocScale, float maxApertureDiameter, boolean enabled, int filter,
            boolean nativeResolution, int foregroundRingCount, int backgroundRingCount, int fastGatherRingCount, int maxForegroundCOC, int maxBackgroundCOC);
    private static native void nSetVignetteOptions(long nativeView, float midPoint, float roundness, float feather, float r, float g, float b, float a, boolean enabled);
    private static native void nSetTemporalAntiAliasingOptions(long nativeView, float feedback, float filterWidth, boolean enabled);
    private static native boolean nIsShadowingEnabled(long nativeView);
    private static native void nSetScreenSpaceRefractionEnabled(long nativeView, boolean enabled);
    private static native boolean nIsScreenSpaceRefractionEnabled(long nativeView);
}
