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
 * Engin engine         = Engine.create();
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
    private long mNativeObject;
    @NonNull private final TransformManager mTransformManager;
    @NonNull private final LightManager mLightManager;
    @NonNull private final RenderableManager mRenderableManager;

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

    private Engine(long nativeEngine) {
        mNativeObject = nativeEngine;
        mTransformManager = new TransformManager(nGetTransformManager(nativeEngine));
        mLightManager = new LightManager(nGetLightManager(nativeEngine));
        mRenderableManager = new RenderableManager(nGetRenderableManager(nativeEngine));
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
        long nativeEngine = nCreateEngine(0, 0);
        if (nativeEngine == 0) throw new IllegalStateException("Couldn't create Engine");
        return new Engine(nativeEngine);
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
        long nativeEngine = nCreateEngine(backend.ordinal(), 0);
        if (nativeEngine == 0) throw new IllegalStateException("Couldn't create Engine");
        return new Engine(nativeEngine);
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
        if (Platform.get().validateSharedContext(sharedContext)) {
            long nativeEngine = nCreateEngine(0,
                    Platform.get().getSharedContextNativeHandle(sharedContext));
            if (nativeEngine == 0) throw new IllegalStateException("Couldn't create Engine");
            return new Engine(nativeEngine);
        }
        throw new IllegalArgumentException("Invalid shared context " + sharedContext);
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
        return Backend.values()[(int) nGetBackend(getNativeObject())];
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
        return createSwapChain(surface, SwapChain.CONFIG_DEFAULT);
    }

    /**
     * Creates a {@link SwapChain} from the given OS native window handle.
     *
     * @param surface on Android, <b>must be</b> an instance of {@link android.view.Surface}
     *
     * @param flags configuration flags, see {@link SwapChain}
     *
     * @return a newly created {@link SwapChain} object
     *
     * @exception IllegalStateException can be thrown if the SwapChain couldn't be created
     *
     * @see SwapChain#CONFIG_DEFAULT
     * @see SwapChain#CONFIG_TRANSPARENT
     * @see SwapChain#CONFIG_READABLE
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
     * @param flags  configuration flags, see {@link SwapChain}
     *
     * @return a newly created {@link SwapChain} object
     *
     * @exception IllegalStateException can be thrown if the SwapChain couldn't be created
     *
     * @see SwapChain#CONFIG_DEFAULT
     * @see SwapChain#CONFIG_TRANSPARENT
     * @see SwapChain#CONFIG_READABLE
     *
     */
    @NonNull
    public SwapChain createSwapChain(int width, int height, long flags) {
        if (width >= 0 && height >= 0) {
            long nativeSwapChain = nCreateSwapChainHeadless(getNativeObject(), width, height, flags);
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
     * @param flags configuration flags, see {@link SwapChain}
     *
     * @return a newly created {@link SwapChain} object
     *
     * @exception IllegalStateException can be thrown if the {@link SwapChain} couldn't be created
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
     * Creates a new <code>entity</code> and adds a {@link Camera} component to it.
     *
     * @return A newly created {@link Camera}
     * @exception IllegalStateException can be thrown if the {@link Camera} couldn't be created
     */
    @NonNull
    public Camera createCamera() {
        long nativeCamera = nCreateCamera(getNativeObject());
        if (nativeCamera == 0) throw new IllegalStateException("Couldn't create Camera");
        return new Camera(nativeCamera);
    }

    /**
     * Creates and adds a {@link Camera} component to a given <code>entity</code>.
     *
     * @param entity <code>entity</code> to add the camera component to
     * @return A newly created {@link Camera}
     * @exception IllegalStateException can be thrown if the {@link Camera} couldn't be created
     */
    @NonNull
    public Camera createCamera(@Entity int entity) {
        long nativeCamera = nCreateCameraWithEntity(getNativeObject(), entity);
        if (nativeCamera == 0) throw new IllegalStateException("Couldn't create Camera");
        return new Camera(nativeCamera);
    }

    /**
     * Destroys a {@link Camera} component and frees all its associated resources.
     * @param camera the {@link Camera} to destroy
     */
    public void destroyCamera(@NonNull Camera camera) {
        nDestroyCamera(getNativeObject(), camera.getNativeObject());
        camera.clearNativeObject();
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
    public void destroySkybox(@NonNull ColorGrading colorGrading) {
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
     * Destroys an <code>entity</code> and all its components.
     * <p>
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
     * Kicks the hardware thread (e.g.: the OpenGL, Vulkan or Metal thread) and blocks until
     * all commands to this point are executed. Note that this doesn't guarantee that the
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

    @UsedByReflection("TextureHelper.java")
    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Engine");
        }
        return mNativeObject;
    }

    private void clearNativeObject() {
        mNativeObject = 0;
    }

    private static void assertDestroy(boolean success) {
        if (!success) {
            throw new IllegalStateException("Object couldn't be destoyed (double destroy()?)");
        }
    }

    private static native long nCreateEngine(long backend, long sharedContext);
    private static native void nDestroyEngine(long nativeEngine);
    private static native long nGetBackend(long nativeEngine);
    private static native long nCreateSwapChain(long nativeEngine, Object nativeWindow, long flags);
    private static native long nCreateSwapChainHeadless(long nativeEngine, int width, int height, long flags);
    private static native long nCreateSwapChainFromRawPointer(long nativeEngine, long pointer, long flags);
    private static native boolean nDestroySwapChain(long nativeEngine, long nativeSwapChain);
    private static native long nCreateView(long nativeEngine);
    private static native boolean nDestroyView(long nativeEngine, long nativeView);
    private static native long nCreateRenderer(long nativeEngine);
    private static native boolean nDestroyRenderer(long nativeEngine, long nativeRenderer);
    private static native long nCreateCamera(long nativeEngine);
    private static native long nCreateCameraWithEntity(long nativeEngine, int entity);
    private static native void nDestroyCamera(long nativeEngine, long nativeCamera);
    private static native long nCreateScene(long nativeEngine);
    private static native boolean nDestroyScene(long nativeEngine, long nativeScene);
    private static native long nCreateFence(long nativeEngine);
    private static native boolean nDestroyFence(long nativeEngine, long nativeFence);
    private static native boolean nDestroyStream(long nativeEngine, long nativeStream);
    private static native boolean nDestroyIndexBuffer(long nativeEngine, long nativeIndexBuffer);
    private static native boolean nDestroyVertexBuffer(long nativeEngine, long nativeVertexBuffer);
    private static native boolean nDestroyIndirectLight(long nativeEngine, long nativeIndirectLight);
    private static native boolean nDestroyMaterial(long nativeEngine, long nativeMaterial);
    private static native boolean nDestroyMaterialInstance(long nativeEngine, long nativeMaterialInstance);
    private static native boolean nDestroySkybox(long nativeEngine, long nativeSkybox);
    private static native boolean nDestroyColorGrading(long nativeEngine, long nativeColorGrading);
    private static native boolean nDestroyTexture(long nativeEngine, long nativeTexture);
    private static native boolean nDestroyRenderTarget(long nativeEngine, long nativeTarget);
    private static native void nDestroyEntity(long nativeEngine, int entity);
    private static native void nFlushAndWait(long nativeEngine);
    private static native long nGetTransformManager(long nativeEngine);
    private static native long nGetLightManager(long nativeEngine);
    private static native long nGetRenderableManager(long nativeEngine);
}
