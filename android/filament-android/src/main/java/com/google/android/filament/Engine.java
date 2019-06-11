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

import com.google.android.filament.proguard.UsedByReflection;

import android.support.annotation.NonNull;

public class Engine {
    private long mNativeObject;
    @NonNull private final TransformManager mTransformManager;
    @NonNull private final LightManager mLightManager;
    @NonNull private final RenderableManager mRenderableManager;

    public enum Backend {
        DEFAULT,  // Automatically selects an appropriate driver for the platform.
        OPENGL,   // Selects the OpenGL ES driver.
        VULKAN,   // Selects the experimental Vulkan driver.
        NOOP,     // Selects the no-op driver for testing purposes.
    }

    private Engine(long nativeEngine) {
        mNativeObject = nativeEngine;
        mTransformManager = new TransformManager(nGetTransformManager(nativeEngine));
        mLightManager = new LightManager(nGetLightManager(nativeEngine));
        mRenderableManager = new RenderableManager(nGetRenderableManager(nativeEngine));
    }

    @NonNull
    public static Engine create() {
        long nativeEngine = nCreateEngine(0, 0);
        if (nativeEngine == 0) throw new IllegalStateException("Couldn't create Engine");
        return new Engine(nativeEngine);
    }

    @NonNull
    public static Engine create(@NonNull Backend backend) {
        long nativeEngine = nCreateEngine(backend.ordinal(), 0);
        if (nativeEngine == 0) throw new IllegalStateException("Couldn't create Engine");
        return new Engine(nativeEngine);
    }

    /**
     * Valid shared context:
     * - Android: EGLContext
     * - Other: none
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

    public boolean isValid() {
        return mNativeObject != 0;
    }

    public void destroy() {
        nDestroyEngine(getNativeObject());
        clearNativeObject();
    }

    public Backend getBackend() {
        return Backend.values()[(int) nGetBackend(getNativeObject())];
    }

    // SwapChain

    /**
     * Valid surface types:
     * - Android: Surface
     * - Other: none
     */
    public SwapChain createSwapChain(@NonNull Object surface) {
        return createSwapChain(surface, SwapChain.CONFIG_DEFAULT);
    }

    /**
     * Valid surface types:
     * - Android: Surface
     * - Other: none
     *
     * Flags: see CONFIG flags in SwapChain.
     *
     * @see SwapChain#CONFIG_DEFAULT
     * @see SwapChain#CONFIG_TRANSPARENT
     * @see SwapChain#CONFIG_READABLE
     */
    public SwapChain createSwapChain(@NonNull Object surface, long flags) {
        if (Platform.get().validateSurface(surface)) {
            long nativeSwapChain = nCreateSwapChain(getNativeObject(), surface, flags);
            if (nativeSwapChain == 0) throw new IllegalStateException("Couldn't create SwapChain");
            return new SwapChain(nativeSwapChain, surface);
        }
        throw new IllegalArgumentException("Invalid surface " + surface);
    }

    public SwapChain createSwapChainFromNativeSurface(@NonNull NativeSurface surface, long flags) {
        long nativeSwapChain =
                nCreateSwapChainFromRawPointer(getNativeObject(), surface.getNativeObject(), flags);
        if (nativeSwapChain == 0) throw new IllegalStateException("Couldn't create SwapChain");
        return new SwapChain(nativeSwapChain, surface);
    }

    public void destroySwapChain(@NonNull SwapChain swapChain) {
        nDestroySwapChain(getNativeObject(), swapChain.getNativeObject());
        swapChain.clearNativeObject();
    }

    // View

    @NonNull
    public View createView() {
        long nativeView = nCreateView(getNativeObject());
        if (nativeView == 0) throw new IllegalStateException("Couldn't create View");
        return new View(nativeView);
    }

    public void destroyView(@NonNull View view) {
        nDestroyView(getNativeObject(), view.getNativeObject());
        view.clearNativeObject();
    }

    // Renderer

    @NonNull
    public Renderer createRenderer() {
        long nativeRenderer = nCreateRenderer(getNativeObject());
        if (nativeRenderer == 0) throw new IllegalStateException("Couldn't create Renderer");
        return new Renderer(this, nativeRenderer);
    }

    public void destroyRenderer(@NonNull Renderer renderer) {
        nDestroyRenderer(getNativeObject(), renderer.getNativeObject());
        renderer.clearNativeObject();
    }

    // Camera

    @NonNull
    public Camera createCamera() {
        long nativeCamera = nCreateCamera(getNativeObject());
        if (nativeCamera == 0) throw new IllegalStateException("Couldn't create Camera");
        return new Camera(nativeCamera);
    }

    @NonNull
    public Camera createCamera(@Entity int entity) {
        long nativeCamera = nCreateCameraWithEntity(getNativeObject(), entity);
        if (nativeCamera == 0) throw new IllegalStateException("Couldn't create Camera");
        return new Camera(nativeCamera);
    }

    public void destroyCamera(@NonNull Camera camera) {
        nDestroyCamera(getNativeObject(), camera.getNativeObject());
        camera.clearNativeObject();
    }

    // Scene

    @NonNull
    public Scene createScene() {
        long nativeScene = nCreateScene(getNativeObject());
        if (nativeScene == 0) throw new IllegalStateException("Couldn't create Scene");
        return new Scene(nativeScene);
    }

    public void destroyScene(@NonNull Scene scene) {
        nDestroyScene(getNativeObject(), scene.getNativeObject());
        scene.clearNativeObject();
    }

    // Stream

    public void destroyStream(@NonNull Stream stream) {
        nDestroyStream(getNativeObject(), stream.getNativeObject());
        stream.clearNativeObject();
    }

    // Fence

    @NonNull
    public Fence createFence(@NonNull Fence.Type type) {
        long nativeFence = nCreateFence(getNativeObject(), type.ordinal());
        if (nativeFence == 0) throw new IllegalStateException("Couldn't create Fence");
        return new Fence(nativeFence);
    }

    public void destroyFence(@NonNull Fence fence) {
        nDestroyFence(getNativeObject(), fence.getNativeObject());
        fence.clearNativeObject();
    }

    // others...

    public void destroyIndexBuffer(@NonNull IndexBuffer indexBuffer) {
        nDestroyIndexBuffer(getNativeObject(), indexBuffer.getNativeObject());
        indexBuffer.clearNativeObject();
    }

    public void destroyVertexBuffer(@NonNull VertexBuffer vertexBuffer) {
        nDestroyVertexBuffer(getNativeObject(), vertexBuffer.getNativeObject());
        vertexBuffer.clearNativeObject();
    }

    public void destroyIndirectLight(@NonNull IndirectLight ibl) {
        nDestroyIndirectLight(getNativeObject(), ibl.getNativeObject());
        ibl.clearNativeObject();
    }

    public void destroyMaterial(@NonNull Material material) {
        nDestroyMaterial(getNativeObject(), material.getNativeObject());
        material.clearNativeObject();
    }

    public void destroyMaterialInstance(@NonNull MaterialInstance materialInstance) {
        nDestroyMaterialInstance(getNativeObject(), materialInstance.getNativeObject());
        materialInstance.clearNativeObject();
    }

    public void destroySkybox(@NonNull Skybox skybox) {
        nDestroySkybox(getNativeObject(), skybox.getNativeObject());
        skybox.clearNativeObject();
    }

    public void destroyTexture(@NonNull Texture texture) {
        nDestroyTexture(getNativeObject(), texture.getNativeObject());
        texture.clearNativeObject();
    }

    public void destroyRenderTarget(@NonNull RenderTarget target) {
        nDestroyRenderTarget(getNativeObject(), target.getNativeObject());
        target.clearNativeObject();
    }

    public void destroyEntity(@Entity int entity) {
        nDestroyEntity(getNativeObject(), entity);
    }

    // Managers

    @NonNull
    public TransformManager getTransformManager() {
        return mTransformManager;
    }

    @NonNull
    public LightManager getLightManager() {
        return mLightManager;
    }

    @NonNull
    public RenderableManager getRenderableManager() {
        return mRenderableManager;
    }

    public void flushAndWait() {
        Fence.waitAndDestroy(createFence(Fence.Type.HARD), Fence.Mode.FLUSH);
    }

    @UsedByReflection("TextureHelper.java")
    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Engine");
        }
        return mNativeObject;
    }

    private void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateEngine(long backend, long sharedContext);
    private static native void nDestroyEngine(long nativeEngine);
    private static native long nGetBackend(long nativeEngine);
    private static native long nCreateSwapChain(long nativeEngine, Object nativeWindow, long flags);
    private static native long nCreateSwapChainFromRawPointer(long nativeEngine, long pointer, long flags);
    private static native void nDestroySwapChain(long nativeEngine, long nativeSwapChain);
    private static native long nCreateView(long nativeEngine);
    private static native void nDestroyView(long nativeEngine, long nativeView);
    private static native long nCreateRenderer(long nativeEngine);
    private static native void nDestroyRenderer(long nativeEngine, long nativeRenderer);
    private static native long nCreateCamera(long nativeEngine);
    private static native long nCreateCameraWithEntity(long nativeEngine, int entity);
    private static native void nDestroyCamera(long nativeEngine, long nativeCamera);
    private static native long nCreateScene(long nativeEngine);
    private static native void nDestroyScene(long nativeEngine, long nativeScene);
    private static native long nCreateFence(long nativeEngine, int fenceType);
    private static native void nDestroyFence(long nativeEngine, long nativeFence);
    private static native void nDestroyStream(long nativeEngine, long nativeStream);
    private static native void nDestroyIndexBuffer(long nativeEngine, long nativeIndexBuffer);
    private static native void nDestroyVertexBuffer(long nativeEngine, long nativeVertexBuffer);
    private static native void nDestroyIndirectLight(long nativeEngine, long nativeIndirectLight);
    private static native void nDestroyMaterial(long nativeEngine, long nativeMaterial);
    private static native void nDestroyMaterialInstance(long nativeEngine, long nativeMaterialInstance);
    private static native void nDestroySkybox(long nativeEngine, long nativeSkybox);
    private static native void nDestroyTexture(long nativeEngine, long nativeTexture);
    private static native void nDestroyRenderTarget(long nativeEngine, long nativeTarget);
    private static native void nDestroyEntity(long nativeEngine, int entity);
    private static native long nGetTransformManager(long nativeEngine);
    private static native long nGetLightManager(long nativeEngine);
    private static native long nGetRenderableManager(long nativeEngine);
}
