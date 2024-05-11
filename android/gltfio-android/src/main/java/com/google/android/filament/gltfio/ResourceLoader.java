/*
 * Copyright (C) 2019 The Android Open Source Project
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

package com.google.android.filament.gltfio;

import androidx.annotation.NonNull;

import com.google.android.filament.Engine;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.Buffer;

/**
 * Prepares and uploads vertex buffers and textures to the GPU.
 *
 * <p>For a usage example, see the documentation for {@link AssetLoader}.
 * All methods should be called from the main thread.</p>
 *
 * @see AssetLoader
 * @see FilamentAsset
 */
public class ResourceLoader {
    private final long mNativeObject;
    private final long mNativeStbProvider;
    private final long mNativeKtx2Provider;

    /**
     * Constructs a resource loader tied to the given Filament engine.
     *
     * @param engine the engine that gets passed to all builder methods
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     */
    public ResourceLoader(@NonNull Engine engine) {
        long nativeEngine = engine.getNativeObject();
        mNativeObject = nCreateResourceLoader(nativeEngine, false);
        mNativeStbProvider = nCreateStbProvider(nativeEngine);
        mNativeKtx2Provider = nCreateKtx2Provider(nativeEngine);
        nAddTextureProvider(mNativeObject, "image/jpeg", mNativeStbProvider);
        nAddTextureProvider(mNativeObject, "image/png", mNativeStbProvider);
        nAddTextureProvider(mNativeObject, "image/ktx2", mNativeKtx2Provider);
    }

    /**
     * Constructs a resource loader tied to the given Filament engine.
     *
     * @param engine the engine that gets passed to all builder methods
     * @param normalizeSkinningWeights scale non-conformant skinning weights so they sum to 1
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     */
    public ResourceLoader(@NonNull Engine engine, boolean normalizeSkinningWeights) {
        long nativeEngine = engine.getNativeObject();
        mNativeObject = nCreateResourceLoader(nativeEngine, normalizeSkinningWeights);
        mNativeStbProvider = nCreateStbProvider(nativeEngine);
        mNativeKtx2Provider = nCreateKtx2Provider(nativeEngine);
        nAddTextureProvider(mNativeObject, "image/jpeg", mNativeStbProvider);
        nAddTextureProvider(mNativeObject, "image/png", mNativeStbProvider);
        nAddTextureProvider(mNativeObject, "image/ktx2", mNativeKtx2Provider);
    }

    /**
     * Frees all memory associated with the native resource loader.
     */
    public void destroy() {
        nDestroyResourceLoader(mNativeObject);
        nDestroyTextureProvider(mNativeStbProvider);
        nDestroyTextureProvider(mNativeKtx2Provider);
    }

    /**
     * Feeds the binary content of an external resource into the loader's URI cache.
     *
     * On some platforms, `ResourceLoader` does not know how to download external resources on its
     * own (external resources might come from a filesystem, a database, or the internet) so this
     * method allows clients to download external resources and push them to the loader.
     *
     * Every resource should be passed in before calling [loadResources] or [asyncBeginLoad]. See
     * also [FilamentAsset#getResourceUris].
     *
     * When loading GLB files (as opposed to JSON-based glTF files), clients typically do not
     * need to call this method.
     *
     * @param uri the string path that matches an image URI or buffer URI in the glTF
     * @param buffer the binary blob corresponding to the given URI
     * @return self (for daisy chaining)
     */
    @NonNull
    public ResourceLoader addResourceData(@NonNull String uri, @NonNull Buffer buffer) {
        nAddResourceData(mNativeObject, uri, buffer, buffer.remaining());
        return this;
    }

    /**
     * Checks if the given resource has already been added to the URI cache.
     */
    public boolean hasResourceData(@NonNull String uri) {
        return nHasResourceData(mNativeObject, uri);
    }

    /**
     * Frees memory by evicting the URI cache that was populated via addResourceData.
     *
     * This can be called only after a model is fully loaded or after loading has been cancelled.
     */
    public void evictResourceData() {
        nEvictResourceData(mNativeObject);
    }

    /**
     * Iterates through all external buffers and images and creates corresponding Filament objects
     * (vertex buffers, textures, etc), which become owned by the asset.
     *
     * NOTE: this is a synchronous API, please see [asyncBeginLoad] as an alternative.
     *
     * @param asset the Filament asset that contains URI-based resources
     * @return self (for daisy chaining)
     */
    @NonNull
    public ResourceLoader loadResources(@NonNull FilamentAsset asset) {
        nLoadResources(mNativeObject, asset.getNativeObject());
        return this;
    }

    /**
     * Starts an asynchronous resource load.
     *
     * Returns false if the loading process was unable to start.
     *
     * This is an alternative to #loadResources and requires periodic calls to #asyncUpdateLoad.
     * On multi-threaded systems this creates threads for texture decoding.
     */
    public boolean asyncBeginLoad(@NonNull FilamentAsset asset) {
        return nAsyncBeginLoad(mNativeObject, asset.getNativeObject());
    }

    /**
     * Gets the status of an asynchronous resource load as a percentage in [0,1].
     */
    public float asyncGetLoadProgress() {
        return nAsyncGetLoadProgress(mNativeObject);
    }

    /**
     * Updates an asynchronous load by performing any pending work that must take place
     * on the main thread.
     *
     * Clients must periodically call this until #asyncGetLoadProgress returns 100%.
     * After progress reaches 100%, calling this is harmless; it just does nothing.
     */
    public void asyncUpdateLoad() {
        nAsyncUpdateLoad(mNativeObject);
    }

    /**
     * Cancels pending decoder jobs and frees all CPU-side texel data.
     *
     * Calling this is only necessary if the asyncBeginLoad API was used
     * and cancellation is required before progress reaches 100%.
     */
    public void asyncCancelLoad() {
        nAsyncCancelLoad(mNativeObject);
    }

    private static native long nCreateResourceLoader(long nativeEngine,
            boolean normalizeSkinningWeights);
    private static native void nDestroyResourceLoader(long nativeLoader);
    private static native void nAddResourceData(long nativeLoader, String url, Buffer buffer,
            int remaining);
    private static native void nEvictResourceData(long nativeLoader);
    private static native boolean nHasResourceData(long nativeLoader, String url);
    private static native void nLoadResources(long nativeLoader, long nativeAsset);
    private static native boolean nAsyncBeginLoad(long nativeLoader, long nativeAsset);
    private static native float nAsyncGetLoadProgress(long nativeLoader);
    private static native void nAsyncUpdateLoad(long nativeLoader);
    private static native void nAsyncCancelLoad(long nativeLoader);

    private static native long nCreateStbProvider(long nativeEngine);
    private static native long nCreateKtx2Provider(long nativeEngine);
    private static native void nAddTextureProvider(long nativeLoader, String url, long nativeProvider);
    private static native void nDestroyTextureProvider(long nativeProvider);
}
