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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.android.filament.proguard.UsedByReflection;

/**
 * Engine is filament's main entry-point.
 * <p>
 * An Engine instance main function is to keep track of all resources created by the user and
 * manage the rendering thread as well as the hardware renderer.
 * <p>
 * To use filament, an Engine instance must be created first:
 *
 * <pre>
 * import com.google.android.filament.*
 *
 * Engine engine = Engine.create();
 * </pre>
 * <p>
 * Engine essentially represents (or is associated to) a hardware context
 * (e.g. an OpenGL ES context).
 * <p>
 * Rendering typically happens in an operating system's window (which can be full screen), such
 * window is managed by a {@link Renderer}.
 * <p>
 * A typical filament render loop looks like this:
 *
 *
 * <pre>
 * import com.google.android.filament.*
 *
 * Engine engine        = Engine.create();
 * SwapChain swapChain  = engine.createSwapChain(nativeWindow);
 * Renderer renderer    = engine.createRenderer();
 * Scene scene          = engine.createScene();
 * View view            = engine.createView();
 *
 * view.setScene(scene);
 *
 * do {
 *     // typically we wait for VSYNC and user input events
 *     if (renderer.beginFrame(swapChain)) {
 *         renderer.render(view);
 *         renderer.endFrame();
 *     }
 * } while (!quit);
 *
 * engine.destroyView(view);
 * engine.destroyScene(scene);
 * engine.destroyRenderer(renderer);
 * engine.destroySwapChain(swapChain);
 * engine.destroy();
 * </pre>
 *
 * <h1><u>Resource Tracking</u></h1>
 * <p>
 * Each <code>Engine</code> instance keeps track of all objects created by the user, such as vertex
 * and index buffers, lights, cameras, etc...
 * The user is expected to free those resources, however, leaked resources are freed when the
 * engine instance is destroyed and a warning is emitted in the console.
 *
 * <h1><u>Thread safety</u></h1>
 * <p>
 * An <code>Engine</code> instance is not thread-safe. The implementation makes no attempt to
 * synchronize calls to an <code>Engine</code> instance methods.
 * If multi-threading is needed, synchronization must be external.
 *
 * <h1><u>Multi-threading</u></h1>
 * <p>
 * When created, the <code>Engine</code> instance starts a render thread as well as multiple worker
 * threads, these threads have an elevated priority appropriate for rendering, based on the
 * platform's best practices. The number of worker threads depends on the platform and is
 * automatically chosen for best performance.
 * <p>
 * On platforms with asymmetric cores (e.g. ARM's Big.Little), <code>Engine</code> makes some
 * educated guesses as to which cores to use for the render thread and worker threads. For example,
 * it'll try to keep an OpenGL ES thread on a Big core.
 *
 * <h1><u>Swap Chains</u></h1>
 * <p>
 * A swap chain represents an Operating System's <b>native</b> renderable surface.
 * Typically it's a window or a view. Because a {@link SwapChain} is initialized from a native
 * object, it is given to filament as an <code>Object</code>, which must be of the proper type for
 * each platform filament is running on.
 * <p>
 *
 * @see SwapChain
 * @see Renderer
 */
public class Engine {
    private static final Backend[] sBackendValues = Backend.values();
    private static final FeatureLevel[] sFeatureLevelValues = FeatureLevel.values();

    private long mNativeObject;

    private Config mConfig;

    @NonNull private final TransformManager mTransformManager;
    @NonNull private final LightManager mLightManager;
    @NonNull private final RenderableManager mRenderableManager;
    @NonNull private final EntityManager mEntityManager;

    /**
     * Denotes a backend
     */
    public enum Backend {
        /**
         * Automatically selects an appropriate driver for the platform.
         */
        DEFAULT,
        /**
         * Selects the OpenGL driver (which supports OpenGL ES as well).
         */
        OPENGL,
        /**
         * Selects the Vulkan driver if the platform supports it.
         */
        VULKAN,
        /**
         * Selects the Metal driver if the platform supports it.
         */
        METAL,
        /**
         * Selects the no-op driver for testing purposes.
         */
        NOOP,
    }

    /**
     * Defines the backend's feature levels.
     */
    public enum FeatureLevel {
        /** Reserved, don't use */
        FEATURE_LEVEL_0,
        /** OpenGL ES 3.0 features (default) */
        FEATURE_LEVEL_1,
        /** OpenGL ES 3.1 features + 16 textures units + cubemap arrays */
        FEATURE_LEVEL_2,
        /** OpenGL ES 3.1 features + 31 textures units + cubemap arrays */
        FEATURE_LEVEL_3,
    };

    /**
     * The type of technique for stereoscopic rendering
     */
    public enum StereoscopicType {
        /** Stereoscopic rendering is performed using instanced rendering technique. */
        INSTANCED,
        /** Stereoscopic rendering is performed using the multiview feature from the graphics backend. */
        MULTIVIEW,
    };

    /**
     * Constructs <code>Engine</code> objects using a builder pattern.
     */
    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;
        private Config mConfig;

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Sets the {@link Backend} for the Engine.
         *
         * @param backend Driver backend to use
         * @return A reference to this Builder for chaining calls.
         */
        public Builder backend(Backend backend) {
            nSetBuilderBackend(mNativeBuilder, backend.ordinal());
            return this;
        }

        /**
         * Sets a sharedContext for the Engine.
         *
         * @param sharedContext  A platform-dependant OpenGL context used as a shared context
         *                       when creating filament's internal context. On Android this parameter
         *                       <b>must be</b> an instance of {@link android.opengl.EGLContext}.
         * @return A reference to this Builder for chaining calls.
         */
        public Builder sharedContext(Object sharedContext) {
            if (Platform.get().validateSharedContext(sharedContext)) {
                nSetBuilderSharedContext(mNativeBuilder,
                        Platform.get().getSharedContextNativeHandle(sharedContext));
                return this;
            }
            throw new IllegalArgumentException("Invalid shared context " + sharedContext);
        }

        /**
         * Configure the Engine with custom parameters.
         *
         * @param config A {@link Config} object
         * @return A reference to this Builder for chaining calls.
         */
        public Builder config(Config config) {
            mConfig = config;
            nSetBuilderConfig(mNativeBuilder, config.commandBufferSizeMB,
                    config.perRenderPassArenaSizeMB, config.driverHandleArenaSizeMB,
                    config.minCommandBufferSizeMB, config.perFrameCommandsSizeMB,
                    config.jobSystemThreadCount,
                    config.textureUseAfterFreePoolSize, config.disableParallelShaderCompile,
                    config.stereoscopicType.ordinal(), config.stereoscopicEyeCount,
                    config.resourceAllocatorCacheSizeMB, config.resourceAllocatorCacheMaxAge);
            return this;
        }

        /**
         * Sets the initial featureLevel for the Engine.
         *
         * @param featureLevel The feature level at which initialize Filament.
         * @return A reference to this Builder for chaining calls.
         */
        public Builder featureLevel(FeatureLevel featureLevel) {
            nSetBuilderFeatureLevel(mNativeBuilder, featureLevel.ordinal());
            return this;
        }

        /**
         * Sets the initial paused state of the rendering thread.
         *
         * @param paused Whether to start the rendering thread paused.
         * @return A reference to this Builder for chaining calls.
         * @warning Experimental.
         */
        public Builder paused(boolean paused) {
            nSetBuilderPaused(mNativeBuilder, paused);
            return this;
        }

        /**
         * Creates an instance of Engine
         *
         * @return A newly created <code>Engine</code>, or <code>null</code> if the GPU driver couldn't
         *         be initialized, for instance if it doesn't support the right version of OpenGL or
         *         OpenGL ES.
         *
         * @exception IllegalStateException can be thrown if there isn't enough memory to
         * allocate the command buffer.
         */
        public Engine build() {
            long nativeEngine = nBuilderBuild(mNativeBuilder);
            if (nativeEngine == 0) throw new IllegalStateException("Couldn't create Engine");
            return new Engine(nativeEngine, mConfig);
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;

            BuilderFinalizer(long nativeObject) {
                mNativeObject = nativeObject;
            }

            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) { // Ignore
                } finally {
                    nDestroyBuilder(mNativeObject);
                }
            }
        }
    }

    /**
     * Parameters for customizing the initialization of {@link Engine}.
     */
    public static class Config {

        // #defines in Engine.h
        private static final long FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB = 3;
        private static final long FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB = 2;
        private static final long FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB = 1;
        private static final long FILAMENT_COMMAND_BUFFER_SIZE_IN_MB =
                FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB * 3;

        /**
         * Size in MiB of the low-level command buffer arena.
         *
         * Each new command buffer is allocated from here. If this buffer is too small the program
         * might terminate or rendering errors might occur.
         *
         * This is typically set to minCommandBufferSizeMB * 3, so that up to 3 frames can be
         * batched-up at once.
         *
         * This value affects the application's memory usage.
         */
        public long commandBufferSizeMB = FILAMENT_COMMAND_BUFFER_SIZE_IN_MB;

        /**
         * Size in MiB of the per-frame data arena.
         *
         * This is the main arena used for allocations when preparing a frame.
         * e.g.: Froxel data and high-level commands are allocated from this arena.
         *
         * If this size is too small, the program will abort on debug builds and have undefined
         * behavior otherwise.
         *
         * This value affects the application's memory usage.
         */
        public long perRenderPassArenaSizeMB = FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB;

        /**
         * Size in MiB of the backend's handle arena.
         *
         * Backends will fallback to slower heap-based allocations when running out of space and
         * log this condition.
         *
         * If 0, then the default value for the given platform is used
         *
         * This value affects the application's memory usage.
         */
        public long driverHandleArenaSizeMB = 0;

        /**
         * Minimum size in MiB of a low-level command buffer.
         *
         * This is how much space is guaranteed to be available for low-level commands when a new
         * buffer is allocated. If this is too small, the engine might have to stall to wait for
         * more space to become available, this situation is logged.
         *
         * This value does not affect the application's memory usage.
         */
        public long minCommandBufferSizeMB = FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB;

        /**
         * Size in MiB of the per-frame high level command buffer.
         *
         * This buffer is related to the number of draw calls achievable within a frame, if it is
         * too small, the program will abort on debug builds and have undefined behavior otherwise.
         *
         * It is allocated from the 'per-render-pass arena' above. Make sure that at least 1 MiB is
         * left in the per-render-pass arena when deciding the size of this buffer.
         *
         * This value does not affect the application's memory usage.
         */
        public long perFrameCommandsSizeMB = FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB;

        /**
         * Number of threads to use in Engine's JobSystem.
         *
         * Engine uses a utils::JobSystem to carry out paralleization of Engine workloads. This
         * value sets the number of threads allocated for JobSystem. Configuring this value can be
         * helpful in CPU-constrained environments where too many threads can cause contention of
         * CPU and reduce performance.
         *
         * The default value is 0, which implies that the Engine will use a heuristic to determine
         * the number of threads to use.
         */
        public long jobSystemThreadCount = 0;

        /**
         * Number of most-recently destroyed textures to track for use-after-free.
         *
         * This will cause the backend to throw an exception when a texture is freed but still bound
         * to a SamplerGroup and used in a draw call. 0 disables completely.
         *
         * Currently only respected by the Metal backend.
         */
        public long textureUseAfterFreePoolSize = 0;

        /**
         * Set to `true` to forcibly disable parallel shader compilation in the backend.
         * Currently only honored by the GL backend.
         */
        public boolean disableParallelShaderCompile = false;

        /**
         * The type of technique for stereoscopic rendering.
         *
         * This setting determines the algorithm used when stereoscopic rendering is enabled. This
         * decision applies to the entire Engine for the lifetime of the Engine. E.g., multiple
         * Views created from the Engine must use the same stereoscopic type.
         *
         * Each view can enable stereoscopic rendering via the StereoscopicOptions::enable flag.
         *
         * @see View#setStereoscopicOptions
         */
        public StereoscopicType stereoscopicType = StereoscopicType.INSTANCED;

        /**
         * The number of eyes to render when stereoscopic rendering is enabled. Supported values are
         * between 1 and Engine#getMaxStereoscopicEyes() (inclusive).
         *
         * @see View#setStereoscopicOptions
         * @see Engine#getMaxStereoscopicEyes
         */
        public long stereoscopicEyeCount = 2;

        /*
         * @Deprecated This value is no longer used.
         */
        public long resourceAllocatorCacheSizeMB = 64;

        /*
         * This value determines for how many frames are texture entries kept in the cache.
         */
        public long resourceAllocatorCacheMaxAge = 2;
    }

    private Engine(long nativeEngine, Config config) {
        mNativeObject = nativeEngine;
        mTransformManager = new TransformManager(nGetTransformManager(nativeEngine));
        mLightManager = new LightManager(nGetLightManager(nativeEngine));
        mRenderableManager = new RenderableManager(nGetRenderableManager(nativeEngine));
        mEntityManager = new EntityManager(nGetEntityManager(nativeEngine));
        mConfig = config;
    }

    /**
     * Creates an instance of Engine using the default {@link Backend}
     * <p>
     * This method is one of the few thread-safe methods.
     *
     * @return A newly created <code>Engine</code>, or <code>null</code> if the GPU driver couldn't
     *         be initialized, for instance if it doesn't support the right version of OpenGL or
     *         OpenGL ES.
     *
     * @exception IllegalStateException can be thrown if there isn't enough memory to
     * allocate the command buffer.
     *
     */
    @NonNull
    public static Engine create() {
        return new Builder().build();
    }

    /**
     * Creates an instance of Engine using the specified {@link Backend}
     * <p>
     * This method is one of the few thread-safe methods.
     *
     * @param backend           driver backend to use
     *
     * @return A newly created <code>Engine</code>, or <code>null</code> if the GPU driver couldn't
     *         be initialized, for instance if it doesn't support the right version of OpenGL or
     *         OpenGL ES.
     *
     * @exception IllegalStateException can be thrown if there isn't enough memory to
     * allocate the command buffer.
     *
     */
    @NonNull
    public static Engine create(@NonNull Backend backend) {
        return new Builder()
            .backend(backend)
            .build();
    }

    /**
     * Creates an instance of Engine using the {@link Backend#OPENGL} and a shared OpenGL context.
     * <p>
     * This method is one of the few thread-safe methods.
     *
     * @param sharedContext  A platform-dependant OpenGL context used as a shared context
     *                       when creating filament's internal context. On Android this parameter
     *                       <b>must be</b> an instance of {@link android.opengl.EGLContext}.
     *
     * @return A newly created <code>Engine</code>, or <code>null</code> if the GPU driver couldn't
     *         be initialized, for instance if it doesn't support the right version of OpenGL or
     *         OpenGL ES.
     *
     * @exception IllegalStateException can be thrown if there isn't enough memory to
     * allocate the command buffer.
     *
     */
    @NonNull
    public static Engine create(@NonNull Object sharedContext) {
        return new Builder()
            .sharedContext(sharedContext)
            .build();
    }

    /**
     * @return <code>true</code> if this <code>Engine</code> is initialized properly.
     */
    public boolean isValid() {
        return mNativeObject != 0;
    }

    /**
     * Destroy the <code>Engine</code> instance and all associated resources.
     * <p>
     * This method is one of the few thread-safe methods.
     * <p>
     * {@link Engine#destroy()} should be called last and after all other resources have been
     * destroyed, it ensures all filament resources are freed.
     * <p>
     * <code>Destroy</code> performs the following tasks:
     * <li>Destroy all internal software and hardware resources.</li>
     * <li>Free all user allocated resources that are not already destroyed and logs a warning.
     *     <p>This indicates a "leak" in the user's code.</li>
     * <li>Terminate the rendering engine's thread.</li>
     *
     * <pre>
     * Engine engine = Engine.create();
     * engine.destroy();
     * </pre>
     */
    public void destroy() {
        nDestroyEngine(getNativeObject());
        clearNativeObject();
    }

    /**
     * @return the backend used by this <code>Engine</code>
     */
    @NonNull
    public Backend getBackend() {
        return sBackendValues[(int) nGetBackend(getNativeObject())];
    }

    /**
     * Helper to enable accurate translations.
     * If you need this Engine to handle a very large world space, one way to achieve this
     * automatically is to enable accurate translations in the TransformManager. This helper
     * provides a convenient way of doing that.
     * This is typically called once just after creating the Engine.
     */
    public void enableAccurateTranslations() {
        getTransformManager().setAccurateTranslationsEnabled(true);
    }

    /**
     * Query the feature level supported by the selected backend.
     *
     * A specific feature level needs to be set before the corresponding features can be used.
     *
     * @return FeatureLevel supported the selected backend.
     * @see #setActiveFeatureLevel
     */
    @NonNull
    public FeatureLevel getSupportedFeatureLevel() {
        return sFeatureLevelValues[(int) nGetSupportedFeatureLevel(getNativeObject())];
    }

    /**
     * Activate all features of a given feature level. If an explicit feature level is not specified
     * at Engine initialization time via {@link Builder#featureLevel}, the default feature level is
     * {@link FeatureLevel#FEATURE_LEVEL_0} on devices not compatible with GLES 3.0; otherwise, the
     * default is {@link FeatureLevel::FEATURE_LEVEL_1}. The selected feature level must not be
     * higher than the value returned by {@link #getActiveFeatureLevel} and it's not possible lower
     * the active feature level. Additionally, it is not possible to modify the feature level at all
     * if the Engine was initialized at {@link FeatureLevel#FEATURE_LEVEL_0}.
     *
     * @param featureLevel the feature level to activate. If featureLevel is lower than {@link
     *                     #getActiveFeatureLevel}, the current (higher) feature level is kept. If
     *                     featureLevel is higher than {@link #getSupportedFeatureLevel}, or if the
     *                     engine was initialized at feature level 0, an exception is thrown, or the
     *                     program is terminated if exceptions are disabled.
     *
     * @return the active feature level.
     *
     * @see Builder#featureLevel
     * @see #getSupportedFeatureLevel
     * @see #getActiveFeatureLevel
     */
    @NonNull
    public FeatureLevel setActiveFeatureLevel(@NonNull FeatureLevel featureLevel) {
        return sFeatureLevelValues[(int) nSetActiveFeatureLevel(getNativeObject(), featureLevel.ordinal())];
    }

    /**
     * Returns the currently active feature level.
     * @return currently active feature level
     * @see #getSupportedFeatureLevel
     * @see #setActiveFeatureLevel
     */
    @NonNull
    public FeatureLevel getActiveFeatureLevel() {
        return sFeatureLevelValues[(int) nGetActiveFeatureLevel(getNativeObject())];
    }

    /**
     * Enables or disables automatic instancing of render primitives. Instancing of render primitive
     * can greatly reduce CPU overhead but requires the instanced primitives to be identical
     * (i.e. use the same geometry) and use the same MaterialInstance. If it is known that the
     * scene doesn't contain any identical primitives, automatic instancing can have some
     * overhead and it is then best to disable it.
     *
     * Disabled by default.
     *
     * @param enable true to enable, false to disable automatic instancing.
     *
     * @see RenderableManager
     * @see MaterialInstance
     */
    public void setAutomaticInstancingEnabled(boolean enable) {
        nSetAutomaticInstancingEnabled(getNativeObject(), enable);
    }

    /**
     * @return true if automatic instancing is enabled, false otherwise.
     * @see #setAutomaticInstancingEnabled
     */
    public boolean isAutomaticInstancingEnabled() {
        return nIsAutomaticInstancingEnabled(getNativeObject());
    }

    /**
     * Retrieves the configuration settings of this {@link Engine}.
     *
     * This method returns the configuration object that was supplied to the Engine's {@link
     * Builder#config} method during the creation of this Engine. If the {@link Builder::config}
     * method was not explicitly called (or called with null), this method returns the default
     * configuration settings.
     *
     * @return a {@link Config} object with this Engine's configuration
     * @see Builder#config
     */
    @NonNull
    public Config getConfig() {
        if (mConfig == null) {
            mConfig = new Config();
        }
        return mConfig;
    }

    /**
     * Returns the maximum number of stereoscopic eyes supported by Filament. The actual number of
     * eyes rendered is set at Engine creation time with the {@link
     * Engine#Config#stereoscopicEyeCount} setting.
     *
     * @return the max number of stereoscopic eyes supported
     * @see Engine#Config#stereoscopicEyeCount
     */
    public long getMaxStereoscopicEyes() {
        return nGetMaxStereoscopicEyes(getNativeObject());
    }


    // SwapChain

    /**
     * Creates an opaque {@link SwapChain} from the given OS native window handle.
     *
     * @param surface on Android, <b>must be</b> an instance of {@link android.view.Surface}
     *
     * @return a newly created {@link SwapChain} object
     *
     * @exception IllegalStateException can be thrown if the SwapChain couldn't be created
     */
    @NonNull
    public SwapChain createSwapChain(@NonNull Object surface) {
        return createSwapChain(surface, SwapChainFlags.CONFIG_DEFAULT);
    }

    /**
     * Creates a {@link SwapChain} from the given OS native window handle.
     *
     * @param surface on Android, <b>must be</b> an instance of {@link android.view.Surface}
     *
     * @param flags configuration flags, see {@link SwapChainFlags}
     *
     * @return a newly created {@link SwapChain} object
     *
     * @exception IllegalStateException can be thrown if the SwapChain couldn't be created
     *
     * @see SwapChainFlags#CONFIG_DEFAULT
     * @see SwapChainFlags#CONFIG_TRANSPARENT
     * @see SwapChainFlags#CONFIG_READABLE
     *
     */
    @NonNull
    public SwapChain createSwapChain(@NonNull Object surface, long flags) {
        if (Platform.get().validateSurface(surface)) {
            long nativeSwapChain = nCreateSwapChain(getNativeObject(), surface, flags);
            if (nativeSwapChain == 0) throw new IllegalStateException("Couldn't create SwapChain");
            return new SwapChain(nativeSwapChain, surface);
        }
        throw new IllegalArgumentException("Invalid surface " + surface);
    }

    /**
     * Creates a headless {@link SwapChain}
     *
     * @param width  width of the rendering buffer
     * @param height height of the rendering buffer
     * @param flags  configuration flags, see {@link SwapChainFlags}
     *
     * @return a newly created {@link SwapChain} object
     *
     * @exception IllegalStateException can be thrown if the SwapChain couldn't be created
     *
     * @see SwapChainFlags#CONFIG_DEFAULT
     * @see SwapChainFlags#CONFIG_TRANSPARENT
     * @see SwapChainFlags#CONFIG_READABLE
     *
     */
    @NonNull
    public SwapChain createSwapChain(int width, int height, long flags) {
        if (width >= 0 && height >= 0) {
            long nativeSwapChain =
                nCreateSwapChainHeadless(getNativeObject(), width, height, flags);
            if (nativeSwapChain == 0) throw new IllegalStateException("Couldn't create SwapChain");
            return new SwapChain(nativeSwapChain, null);
        }
        throw new IllegalArgumentException("Invalid parameters");
    }

    /**
     * Creates a {@link SwapChain} from a {@link NativeSurface}.
     *
     * @param surface a properly initialized {@link NativeSurface}
     *
     * @param flags configuration flags, see {@link SwapChainFlags}
     *
     * @return a newly created {@link SwapChain} object
     *
     * @exception IllegalStateException can be thrown if the {@link SwapChainFlags} couldn't be
     *            created
     */
    @NonNull
    public SwapChain createSwapChainFromNativeSurface(@NonNull NativeSurface surface, long flags) {
        long nativeSwapChain =
                nCreateSwapChainFromRawPointer(getNativeObject(), surface.getNativeObject(), flags);
        if (nativeSwapChain == 0) throw new IllegalStateException("Couldn't create SwapChain");
        return new SwapChain(nativeSwapChain, surface);
    }

    /**
     * Destroys a {@link SwapChain} and frees all its associated resources.
     * @param swapChain the {@link SwapChain} to destroy
     */
    public void destroySwapChain(@NonNull SwapChain swapChain) {
        assertDestroy(nDestroySwapChain(getNativeObject(), swapChain.getNativeObject()));
        swapChain.clearNativeObject();
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidRenderer(@NonNull Renderer object) {
        return nIsValidRenderer(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidView(@NonNull View object) {
        return nIsValidView(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidScene(@NonNull Scene object) {
        return nIsValidScene(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidFence(@NonNull Fence object) {
        return nIsValidFence(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidStream(@NonNull Stream object) {
        return nIsValidStream(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidIndexBuffer(@NonNull IndexBuffer object) {
        return nIsValidIndexBuffer(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidVertexBuffer(@NonNull VertexBuffer object) {
        return nIsValidVertexBuffer(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidSkinningBuffer(@NonNull SkinningBuffer object) {
        return nIsValidSkinningBuffer(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidIndirectLight(@NonNull IndirectLight object) {
        return nIsValidIndirectLight(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidMaterial(@NonNull Material object) {
        return nIsValidMaterial(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidSkybox(@NonNull Skybox object) {
        return nIsValidSkybox(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidColorGrading(@NonNull ColorGrading object) {
        return nIsValidColorGrading(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidTexture(@NonNull Texture object) {
        return nIsValidTexture(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidRenderTarget(@NonNull RenderTarget object) {
        return nIsValidRenderTarget(getNativeObject(), object.getNativeObject());
    }

    /**
     * Returns whether the object is valid.
     * @param object Object to check for validity
     * @return returns true if the specified object is valid.
     */
    public boolean isValidSwapChain(@NonNull SwapChain object) {
        return nIsValidSwapChain(getNativeObject(), object.getNativeObject());
    }

    // View

    /**
     * Creates a {@link View}.
     * @return a newly created {@link View}
     * @exception IllegalStateException can be thrown if the {@link View} couldn't be created
     */
    @NonNull
    public View createView() {
        long nativeView = nCreateView(getNativeObject());
        if (nativeView == 0) throw new IllegalStateException("Couldn't create View");
        return new View(nativeView);
    }

    /**
     * Destroys a {@link View} and frees all its associated resources.
     * @param view the {@link View} to destroy
     */
    public void destroyView(@NonNull View view) {
        assertDestroy(nDestroyView(getNativeObject(), view.getNativeObject()));
        view.clearNativeObject();
    }

    // Renderer

    /**
     * Creates a {@link Renderer}.
     * @return a newly created {@link Renderer}
     * @exception IllegalStateException can be thrown if the {@link Renderer} couldn't be created
     */
    @NonNull
    public Renderer createRenderer() {
        long nativeRenderer = nCreateRenderer(getNativeObject());
        if (nativeRenderer == 0) throw new IllegalStateException("Couldn't create Renderer");
        return new Renderer(this, nativeRenderer);
    }

    /**
     * Destroys a {@link Renderer} and frees all its associated resources.
     * @param renderer the {@link Renderer} to destroy
     */
    public void destroyRenderer(@NonNull Renderer renderer) {
        assertDestroy(nDestroyRenderer(getNativeObject(), renderer.getNativeObject()));
        renderer.clearNativeObject();
    }

    // Camera

    /**
     * Creates and adds a {@link Camera} component to a given <code>entity</code>.
     *
     * @param entity <code>entity</code> to add the camera component to
     * @return A newly created {@link Camera}
     * @exception IllegalStateException can be thrown if the {@link Camera} couldn't be created
     */
    @NonNull
    public Camera createCamera(@Entity int entity) {
        long nativeCamera = nCreateCamera(getNativeObject(), entity);
        if (nativeCamera == 0) throw new IllegalStateException("Couldn't create Camera");
        return new Camera(nativeCamera, entity);
    }

    /**
     * Returns the Camera component of the given <code>entity</code>.
     *
     * @param entity An <code>entity</code>.
     * @return the Camera component for this entity or null if the entity doesn't have a Camera
     *         component
     */
    @Nullable
    public Camera getCameraComponent(@Entity int entity) {
        long nativeCamera = nGetCameraComponent(getNativeObject(), entity);
        if (nativeCamera == 0) return null;
        return new Camera(nativeCamera, entity);
    }

    /**
     * Destroys the {@link Camera} component associated with the given entity.
     *
     * @param entity an entity
     */
    public void destroyCameraComponent(@Entity int entity) {
        nDestroyCameraComponent(getNativeObject(), entity);
    }

    // Scene

    /**
     * Creates a {@link Scene}.
     * @return a newly created {@link Scene}
     * @exception IllegalStateException can be thrown if the {@link Scene} couldn't be created
     */
    @NonNull
    public Scene createScene() {
        long nativeScene = nCreateScene(getNativeObject());
        if (nativeScene == 0) throw new IllegalStateException("Couldn't create Scene");
        return new Scene(nativeScene);
    }

    /**
     * Destroys a {@link Scene} and frees all its associated resources.
     * @param scene the {@link Scene} to destroy
     */
    public void destroyScene(@NonNull Scene scene) {
        assertDestroy(nDestroyScene(getNativeObject(), scene.getNativeObject()));
        scene.clearNativeObject();
    }

    // Stream

    /**
     * Destroys a {@link Stream} and frees all its associated resources.
     * @param stream the {@link Stream} to destroy
     */
    public void destroyStream(@NonNull Stream stream) {
        assertDestroy(nDestroyStream(getNativeObject(), stream.getNativeObject()));
        stream.clearNativeObject();
    }

    // Fence

    /**
     * Creates a {@link Fence}.
     * @return a newly created {@link Fence}
     * @exception IllegalStateException can be thrown if the {@link Fence} couldn't be created
     */
    @NonNull
    public Fence createFence() {
        long nativeFence = nCreateFence(getNativeObject());
        if (nativeFence == 0) throw new IllegalStateException("Couldn't create Fence");
        return new Fence(nativeFence);
    }

    /**
     * Destroys a {@link Fence} and frees all its associated resources.
     * @param fence the {@link Fence} to destroy
     */
    public void destroyFence(@NonNull Fence fence) {
        assertDestroy(nDestroyFence(getNativeObject(), fence.getNativeObject()));
        fence.clearNativeObject();
    }

    // others...

    /**
     * Destroys a {@link IndexBuffer} and frees all its associated resources.
     * @param indexBuffer the {@link IndexBuffer} to destroy
     */
    public void destroyIndexBuffer(@NonNull IndexBuffer indexBuffer) {
        assertDestroy(nDestroyIndexBuffer(getNativeObject(), indexBuffer.getNativeObject()));
        indexBuffer.clearNativeObject();
    }

    /**
     * Destroys a {@link VertexBuffer} and frees all its associated resources.
     * @param vertexBuffer the {@link VertexBuffer} to destroy
     */
    public void destroyVertexBuffer(@NonNull VertexBuffer vertexBuffer) {
        assertDestroy(nDestroyVertexBuffer(getNativeObject(), vertexBuffer.getNativeObject()));
        vertexBuffer.clearNativeObject();
    }

    /**
     * Destroys a {@link SkinningBuffer} and frees all its associated resources.
     * @param skinningBuffer the {@link SkinningBuffer} to destroy
     */
    public void destroySkinningBuffer(@NonNull SkinningBuffer skinningBuffer) {
        assertDestroy(nDestroySkinningBuffer(getNativeObject(), skinningBuffer.getNativeObject()));
        skinningBuffer.clearNativeObject();
    }

    /**
     * Destroys a {@link IndirectLight} and frees all its associated resources.
     * @param ibl the {@link IndirectLight} to destroy
     */
    public void destroyIndirectLight(@NonNull IndirectLight ibl) {
        assertDestroy(nDestroyIndirectLight(getNativeObject(), ibl.getNativeObject()));
        ibl.clearNativeObject();
    }

    /**
     * Destroys a {@link Material} and frees all its associated resources.
     * <p>
     * All {@link MaterialInstance} of the specified {@link Material} must be destroyed before
     * destroying it; if some {@link MaterialInstance} remain, this method fails silently.
     *
     * @param material the {@link Material} to destroy
     */
    public void destroyMaterial(@NonNull Material material) {
        assertDestroy(nDestroyMaterial(getNativeObject(), material.getNativeObject()));
        material.clearNativeObject();
    }

    /**
     * Destroys a {@link MaterialInstance} and frees all its associated resources.
     * @param materialInstance the {@link MaterialInstance} to destroy
     */
    public void destroyMaterialInstance(@NonNull MaterialInstance materialInstance) {
        assertDestroy(nDestroyMaterialInstance(getNativeObject(), materialInstance.getNativeObject()));
        materialInstance.clearNativeObject();
    }

    /**
     * Destroys a {@link Skybox} and frees all its associated resources.
     * @param skybox the {@link Skybox} to destroy
     */
    public void destroySkybox(@NonNull Skybox skybox) {
        assertDestroy(nDestroySkybox(getNativeObject(), skybox.getNativeObject()));
        skybox.clearNativeObject();
    }

    /**
     * Destroys a {@link ColorGrading} and frees all its associated resources.
     * @param colorGrading the {@link ColorGrading} to destroy
     */
    public void destroyColorGrading(@NonNull ColorGrading colorGrading) {
        assertDestroy(nDestroyColorGrading(getNativeObject(), colorGrading.getNativeObject()));
        colorGrading.clearNativeObject();
    }

    /**
     * Destroys a {@link Texture} and frees all its associated resources.
     * @param texture the {@link Texture} to destroy
     */
    public void destroyTexture(@NonNull Texture texture) {
        assertDestroy(nDestroyTexture(getNativeObject(), texture.getNativeObject()));
        texture.clearNativeObject();
    }

    /**
     * Destroys a {@link RenderTarget} and frees all its associated resources.
     * @param target the {@link RenderTarget} to destroy
     */
    public void destroyRenderTarget(@NonNull RenderTarget target) {
        nDestroyRenderTarget(getNativeObject(), target.getNativeObject());
        target.clearNativeObject();
    }

    /**
     * Destroys all Filament-known components from this <code>entity</code>.
     * <p>
     * This method destroys Filament components only, not the <code>entity</code> itself. To destroy
     * the <code>entity</code> use <code>EntityManager#destroy</code>.
     *
     * It is recommended to destroy components individually before destroying their
     * <code>entity</code>, this gives more control as to when the destruction really happens.
     * Otherwise, orphaned components are garbage collected, which can happen at a later time.
     * Even when component are garbage collected, the destruction of their <code>entity</code>
     * terminates their participation immediately.
     *
     * @param entity the <code>entity</code> to destroy
     */
    public void destroyEntity(@Entity int entity) {
        nDestroyEntity(getNativeObject(), entity);
    }

    // Managers

    /**
     * @return the {@link TransformManager} used by this {@link Engine}
     */
    @NonNull
    public TransformManager getTransformManager() {
        return mTransformManager;
    }

    /**
     * @return the {@link LightManager} used by this {@link Engine}
     */
    @NonNull
    public LightManager getLightManager() {
        return mLightManager;
    }

    /**
     * @return the {@link RenderableManager} used by this {@link Engine}
     */
    @NonNull
    public RenderableManager getRenderableManager() {
        return mRenderableManager;
    }

    /**
     * @return the {@link EntityManager} used by this {@link Engine}
     */
    @NonNull
    public EntityManager getEntityManager() {
        return mEntityManager;
    }

    /**
     * Kicks the hardware thread (e.g.: the OpenGL, Vulkan or Metal thread) and blocks until
     * all commands to this point are executed. Note that this does guarantee that the
     * hardware is actually finished.
     *
     * <p>This is typically used right after destroying the <code>SwapChain</code>,
     * in cases where a guarantee about the SwapChain destruction is needed in a timely fashion,
     * such as when responding to Android's
     * {@link  android.view.SurfaceHolder.Callback#surfaceDestroyed surfaceDestroyed}.</p>
     */
    public void flushAndWait() {
        nFlushAndWait(getNativeObject());
    }

    /**
     * Kicks the hardware thread (e.g. the OpenGL, Vulkan or Metal thread) but does not wait
     * for commands to be either executed or the hardware finished.
     *
     * <p>This is typically used after creating a lot of objects to start draining the command
     * queue which has a limited size.</p>
     */
    public void flush() {
        nFlush(getNativeObject());
    }

    /**
     * Pause or resume the rendering thread.
     * @warning Experimental.
     */
    public void setPaused(boolean paused) {
        nSetPaused(getNativeObject(), paused);
    }

    @UsedByReflection("TextureHelper.java")
    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Engine");
        }
        return mNativeObject;
    }

    @UsedByReflection("MaterialBuilder.java")
    public long getNativeJobSystem() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Engine");
        }
        return nGetJobSystem(getNativeObject());
    }

    private void clearNativeObject() {
        mNativeObject = 0;
    }

    private static void assertDestroy(boolean success) {
        if (!success) {
            throw new IllegalStateException("Object couldn't be destroyed (double destroy()?)");
        }
    }

    private static native void nDestroyEngine(long nativeEngine);
    private static native long nGetBackend(long nativeEngine);
    private static native long nCreateSwapChain(long nativeEngine, Object nativeWindow, long flags);
    private static native long nCreateSwapChainHeadless(long nativeEngine, int width, int height, long flags);
    private static native long nCreateSwapChainFromRawPointer(long nativeEngine, long pointer, long flags);
    private static native long nCreateView(long nativeEngine);
    private static native long nCreateRenderer(long nativeEngine);
    private static native long nCreateCamera(long nativeEngine, int entity);
    private static native long nGetCameraComponent(long nativeEngine, int entity);
    private static native void nDestroyCameraComponent(long nativeEngine, int entity);
    private static native long nCreateScene(long nativeEngine);
    private static native long nCreateFence(long nativeEngine);

    private static native boolean nDestroyRenderer(long nativeEngine, long nativeRenderer);
    private static native boolean nDestroyView(long nativeEngine, long nativeView);
    private static native boolean nDestroyScene(long nativeEngine, long nativeScene);
    private static native boolean nDestroyFence(long nativeEngine, long nativeFence);
    private static native boolean nDestroyStream(long nativeEngine, long nativeStream);
    private static native boolean nDestroyIndexBuffer(long nativeEngine, long nativeIndexBuffer);
    private static native boolean nDestroyVertexBuffer(long nativeEngine, long nativeVertexBuffer);
    private static native boolean nDestroySkinningBuffer(long nativeEngine, long nativeSkinningBuffer);
    private static native boolean nDestroyIndirectLight(long nativeEngine, long nativeIndirectLight);
    private static native boolean nDestroyMaterial(long nativeEngine, long nativeMaterial);
    private static native boolean nDestroyMaterialInstance(long nativeEngine, long nativeMaterialInstance);
    private static native boolean nDestroySkybox(long nativeEngine, long nativeSkybox);
    private static native boolean nDestroyColorGrading(long nativeEngine, long nativeColorGrading);
    private static native boolean nDestroyTexture(long nativeEngine, long nativeTexture);
    private static native boolean nDestroyRenderTarget(long nativeEngine, long nativeTarget);
    private static native boolean nDestroySwapChain(long nativeEngine, long nativeSwapChain);
    private static native boolean nIsValidRenderer(long nativeEngine, long nativeRenderer);
    private static native boolean nIsValidView(long nativeEngine, long nativeView);
    private static native boolean nIsValidScene(long nativeEngine, long nativeScene);
    private static native boolean nIsValidFence(long nativeEngine, long nativeFence);
    private static native boolean nIsValidStream(long nativeEngine, long nativeStream);
    private static native boolean nIsValidIndexBuffer(long nativeEngine, long nativeIndexBuffer);
    private static native boolean nIsValidVertexBuffer(long nativeEngine, long nativeVertexBuffer);
    private static native boolean nIsValidSkinningBuffer(long nativeEngine, long nativeSkinningBuffer);
    private static native boolean nIsValidIndirectLight(long nativeEngine, long nativeIndirectLight);
    private static native boolean nIsValidMaterial(long nativeEngine, long nativeMaterial);
    private static native boolean nIsValidSkybox(long nativeEngine, long nativeSkybox);
    private static native boolean nIsValidColorGrading(long nativeEngine, long nativeColorGrading);
    private static native boolean nIsValidTexture(long nativeEngine, long nativeTexture);
    private static native boolean nIsValidRenderTarget(long nativeEngine, long nativeTarget);
    private static native boolean nIsValidSwapChain(long nativeEngine, long nativeSwapChain);
    private static native void nDestroyEntity(long nativeEngine, int entity);
    private static native void nFlushAndWait(long nativeEngine);
    private static native void nFlush(long nativeEngine);
    private static native void nSetPaused(long nativeEngine, boolean paused);
    private static native long nGetTransformManager(long nativeEngine);
    private static native long nGetLightManager(long nativeEngine);
    private static native long nGetRenderableManager(long nativeEngine);
    private static native long nGetJobSystem(long nativeEngine);
    private static native long nGetEntityManager(long nativeEngine);
    private static native void nSetAutomaticInstancingEnabled(long nativeEngine, boolean enable);
    private static native boolean nIsAutomaticInstancingEnabled(long nativeEngine);
    private static native long nGetMaxStereoscopicEyes(long nativeEngine);
    private static native int nGetSupportedFeatureLevel(long nativeEngine);
    private static native int nSetActiveFeatureLevel(long nativeEngine, int ordinal);
    private static native int nGetActiveFeatureLevel(long nativeEngine);

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);
    private static native void nSetBuilderBackend(long nativeBuilder, long backend);
    private static native void nSetBuilderConfig(long nativeBuilder, long commandBufferSizeMB,
            long perRenderPassArenaSizeMB, long driverHandleArenaSizeMB,
            long minCommandBufferSizeMB, long perFrameCommandsSizeMB, long jobSystemThreadCount,
            long textureUseAfterFreePoolSize, boolean disableParallelShaderCompile,
            int stereoscopicType, long stereoscopicEyeCount,
            long resourceAllocatorCacheSizeMB, long resourceAllocatorCacheMaxAge);
    private static native void nSetBuilderFeatureLevel(long nativeBuilder, int ordinal);
    private static native void nSetBuilderSharedContext(long nativeBuilder, long sharedContext);
    private static native void nSetBuilderPaused(long nativeBuilder, boolean paused);
    private static native long nBuilderBuild(long nativeBuilder);
}
