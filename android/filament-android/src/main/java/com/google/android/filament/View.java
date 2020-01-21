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
    private DepthPrepass mDepthPrepass = DepthPrepass.DEFAULT;
    private AmbientOcclusionOptions mAmbientOcclusionOptions;
    private RenderTarget mRenderTarget;

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
         * Desired frame time in milliseconds.
         */
        public float targetFrameTimeMilli = 1000.0f / 60.0f;

        /**
         * Additional headroom for the GPU as a ratio of the targetFrameTime.
         */
        public float headRoomRatio = 0.0f;

        /**
         * Rate at which the scale will change to reach the target frame rate.
         */
        public float scaleRate = 0.125f;

        /**
         * The minimum scale in X and Y this View should use.
         */
        public float minScale = 0.5f;

        /**
         * The maximum scale in X and Y this View should use.
         */
        public float maxScale = 1.0f;

        /**
         * History size. higher values, tend to filter more (clamped to 30).
         */
        public int history = 9;
    }

    /**
     * Options for Ambient Occlusion
     * @see #setAmbientOcclusion
     */
    public static class AmbientOcclusionOptions {
        /**
         * Ambient Occlusion radius in meters, between 0 and ~10.
         */
        public float radius = 0.3f;

        /**
         * Self-occlusion bias in meters. Use to avoid self-occlusion. Between 0 and a few mm.
         */
        public float bias = 0.005f;

        /**
         * Controls ambient occlusion's contrast. Between 0 (linear) and 1 (squared)
         */
        public float power = 0.0f;

        /**
         * How each dimension of the AO buffer is scaled. Must be positive and <= 1.
         */
        public float resolution = 0.5f;

        /**
         * Strength of the Ambient Occlusion effect. Must be positive.
         */
        public float intensity = 1.0f;
    }

    /**
     * Sets the quality of the HDR color buffer.
     *
     * <p>
     * A quality of <code>HIGH</code> or <code>ULTRA</code> means using an RGB16F or RGBA16F color
     * buffer. This means colors in the LDR range (0..1) have 10 bit precision. A quality of
     * <code>LOW</code> or <code>MEDIUM</code> means using an R11G11B10F opaque color buffer or an
     * RGBA16F transparent color buffer. With R11G11B10F colors in the LDR range have a precision of
     * either 6 bits (red and green channels) or 5 bits (blue channel).
     * </p>
     */
    public enum QualityLevel {
        LOW,
        MEDIUM,
        HIGH,
        ULTRA
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
        public QualityLevel hdrColorBuffer = QualityLevel.HIGH;
    }

    /**
     * List of available ambient occlusion techniques.
     *
     * @see #setAmbientOcclusion
    */
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
     */
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

    /** @see #setDepthPrepass */
    public enum DepthPrepass {
        DEFAULT(-1),
        DISABLED(0),
        ENABLED(1);

        final int value;

        DepthPrepass(int value) {
            this.value = value;
        }
    };

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
     * Sets the color used to clear the Viewport when rendering this View.
     *
     * <p>This is ignored if a {@link Skybox} is present or if clearing has been disabled
     * via {@link #setClearTargets}. Defaults to black.</p>
     *
     * @see #getClearColor
     */
    public void setClearColor(
            @LinearColor float r, @LinearColor float g, @LinearColor float b, float a) {
        nSetClearColor(getNativeObject(), r, g, b, a);
    }

    /**
     * Returns the View clear color in a provided 4-tuple.
     *
     * @return A reference to the passed-in array.
     *
     * @see #setClearColor
     */
    @NonNull @Size(min = 4)
    public float[] getClearColor(@NonNull @Size(min = 4) float[] out) {
        out = Asserts.assertFloat4(out);
        nGetClearColor(getNativeObject(), out);
        return out;
    }

    /**
     * Sets which targets to clear (default: true, true, false)
     *
     * @see #setClearColor
     */
    public void setClearTargets(boolean color, boolean depth, boolean stencil) {
        nSetClearTargets(getNativeObject(), color, depth, stencil);
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
    public void setShadowsEnabled(boolean enabled) {
        nSetShadowsEnabled(getNativeObject(), enabled);
    }

    /**
     * Specifies an offscreen render target to render into.
     *
     * <p>
     * By default, the view's associated render target is null, which corresponds to the
     * SwapChain associated with the engine.
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
     * Enables or disables tone-mapping in the post-processing stage. Enabled by default.
     *
     * @param type Tone-mapping function.
     */
    public void setToneMapping(@NonNull ToneMapping type) {
        nSetToneMapping(getNativeObject(), type.ordinal());
    }

    /**
     * Returns the tone-mapping function.
     * @return tone-mapping function.
     */
    @NonNull
    public ToneMapping getToneMapping() {
        return ToneMapping.values()[nGetToneMapping(getNativeObject())];
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
                options.targetFrameTimeMilli,
                options.headRoomRatio,
                options.scaleRate,
                options.minScale,
                options.maxScale,
                options.history);
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
     * Checks if this view is rendered with a depth-only prepass.
     *
     * @return the value set by {@link #setDepthPrepass}.
     */
    @NonNull
    public DepthPrepass getDepthPrepass() {
        return mDepthPrepass;
    }

    /**
     * Sets whether this view is rendered with or without a depth pre-pass.
     *
     * <p><b>This setting is ignored and will be removed in future versions of Filament.</b></p>
     *
     * <p>
     * By default, the system picks the most appropriate strategy for your platform; this method
     * lets you override that strategy.
     * </p>
     *
     * <p>
     * When the depth pre-pass is enabled, the renderer will first draw all objects in the
     * depth buffer from front to back, and then draw the objects again but sorted to minimize
     * state changes. With the depth pre-pass disabled, objects are drawn only once, but it may
     * result in more state changes or more overdraw.
     * </p>
     *
     * <p>
     * The best strategy may depend on the scene and/or GPU.
     * </p>
     *
     * <ul>
     * <li>DepthPrepass::DEFAULT uses the most appropriate strategy</li>
     * <li>DepthPrepass::DISABLED disables the depth pre-pass</li>
     * <li>DepthPrepass::ENABLE enables the depth pre-pass</li>
     * </ul>
     */
    @Deprecated
    public void setDepthPrepass(@NonNull DepthPrepass depthPrepass) {
        mDepthPrepass = depthPrepass;
        nSetDepthPrepass(getNativeObject(), depthPrepass.value);
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
     * Activates or deactivates ambient occlusion.
     *
     * @param ao Type of ambient occlusion to use.
     */
    public void setAmbientOcclusion(@NonNull AmbientOcclusion ao) {
        nSetAmbientOcclusion(getNativeObject(), ao.ordinal());
    }

    /**
     * Queries the type of ambient occlusion active for this View.
     *
     * @return ambient occlusion type.
     */
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
                options.resolution, options.intensity);
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
    private static native void nSetClearColor(long nativeView, float r, float g, float b, float a);
    private static native void nGetClearColor(long nativeView, float[] out);
    private static native void nSetClearTargets(long nativeView, boolean color, boolean depth, boolean stencil);
    private static native void nSetVisibleLayers(long nativeView, int select, int value);
    private static native void nSetShadowsEnabled(long nativeView, boolean enabled);
    private static native void nSetRenderTarget(long nativeView, long nativeRenderTarget);
    private static native void nSetSampleCount(long nativeView, int count);
    private static native int nGetSampleCount(long nativeView);
    private static native void nSetAntiAliasing(long nativeView, int type);
    private static native int nGetAntiAliasing(long nativeView);
    private static native void nSetToneMapping(long nativeView, int type);
    private static native int nGetToneMapping(long nativeView);
    private static native void nSetDithering(long nativeView, int dithering);
    private static native int nGetDithering(long nativeView);
    private static native void nSetDynamicResolutionOptions(long nativeView,
            boolean enabled, boolean homogeneousScaling,
            float targetFrameTimeMilli, float headRoomRatio, float scaleRate,
            float minScale, float maxScale, int history);
    private static native void nSetRenderQuality(long nativeView, int hdrColorBufferQuality);
    private static native void nSetDynamicLightingOptions(long nativeView, float zLightNear, float zLightFar);
    private static native void nSetDepthPrepass(long nativeView, int value);
    private static native void nSetPostProcessingEnabled(long nativeView, boolean enabled);
    private static native boolean nIsPostProcessingEnabled(long nativeView);
    private static native void nSetFrontFaceWindingInverted(long nativeView, boolean inverted);
    private static native boolean nIsFrontFaceWindingInverted(long nativeView);
    private static native void nSetAmbientOcclusion(long nativeView, int ordinal);
    private static native int nGetAmbientOcclusion(long nativeView);
    private static native void nSetAmbientOcclusionOptions(long nativeView, float radius, float bias, float power, float resolution, float intensity);
}
