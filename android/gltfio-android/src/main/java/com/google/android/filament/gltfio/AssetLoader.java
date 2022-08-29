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
import androidx.annotation.Nullable;

import com.google.android.filament.Engine;
import com.google.android.filament.EntityManager;

import java.nio.Buffer;

/**
 * Consumes a blob of glTF 2.0 content (either JSON or GLB) and produces a {@link FilamentAsset}
 * object, which is a bundle of Filament textures, vertex buffers, index buffers, etc. An asset is
 * composed of 1 or more FilamentInstance objects which contain entities and components.
 *
 * <p>AssetLoader does not fetch external buffer data or create textures on its own. Clients can use
 * the provided {@link ResourceLoader} class for this, which obtains the URI list from the asset.
 * This is demonstrated in the Kotlin snippet below.</p>
 *
 * <pre>
 *
 * companion object {
 *     init {
 *        Gltfio.init() // or, use Utils.init() if depending on filament-utils
 *    }
 * }
 *
 * override fun onCreate(savedInstanceState: Bundle?) {
 *
 *     ...
 *
 *     materialProvider = UbershaderProvider(engine)
 *     assetLoader = AssetLoader(engine, materialProvider, EntityManager.get())
 *
 *     filamentAsset = assets.open("models/lucy.gltf").use { input -&gt;
 *         val bytes = ByteArray(input.available())
 *         input.read(bytes)
 *         assetLoader.createAsset(ByteBuffer.wrap(bytes))!!
 *     }
 *
 *     val resourceLoader = ResourceLoader(engine)
 *     for (uri in filamentAsset.resourceUris) {
 *         val buffer = loadResource(uri)
 *         resourceLoader.addResourceData(uri, buffer)
 *     }
 *     resourceLoader.loadResources(filamentAsset)
 *     resourceLoader.destroy()
 *     filamentAsset.releaseSourceData();
 *
 *     scene.addEntities(filamentAsset.entities)
 * }
 *
 * private fun loadResource(uri: String): Buffer {
 *     TODO("Load your asset here (e.g. using Android's AssetManager API)")
 * }
 * </pre>
 *
 * @see Animator
 * @see FilamentAsset
 * @see ResourceLoader
 */
public class AssetLoader {
    private long mNativeObject;
    private Engine mEngine;
    private MaterialProvider mMaterialCache;

    /**
     * Constructs an <code>AssetLoader</code> that can be used to create and destroy instances of
     * {@link FilamentAsset}.
     *
     * @param engine the engine that the loader should pass to builder objects
     * @param provider an object that provides Filament materials corresponding to glTF materials
     * @param entities the EntityManager that should be used to create entities
     */
    public AssetLoader(@NonNull Engine engine, @NonNull MaterialProvider provider,
            @NonNull EntityManager entities) {

        long nativeEngine = engine.getNativeObject();
        long nativeEntities = entities.getNativeObject();
        mNativeObject = nCreateAssetLoader(nativeEngine, provider, nativeEntities);

        if (mNativeObject == 0) {
            throw new IllegalStateException("Unable to parse glTF asset.");
        }

        mEngine = engine;
        mMaterialCache = provider;
    }

    /**
     * Frees all memory consumed by the native <code>AssetLoader</code>
     *
     * This does not not automatically free the cache of materials, nor
     * does it free the entities for created assets (see destroyAsset).
     */
    public void destroy() {
        nDestroyAssetLoader(mNativeObject);
        mNativeObject = 0;
    }

    /**
     * Creates a {@link FilamentAsset} from the contents of a GLB or GLTF file.
     */
    @Nullable
    public FilamentAsset createAsset(@NonNull Buffer buffer) {
        long nativeAsset = nCreateAsset(mNativeObject, buffer, buffer.remaining());
        return nativeAsset != 0 ? new FilamentAsset(mEngine, nativeAsset) : null;
    }

    /**
     * Consumes the contents of a glTF 2.0 file and produces a primary asset with one or more
     * instances.
     *
     * The given instance array must be sized to the desired number of instances. If successful,
     * this method will populate the array with secondary instances whose resources are shared with
     * the primary asset.
     */
    @Nullable
    @SuppressWarnings("unused")
    public FilamentAsset createInstancedAsset(@NonNull Buffer buffer,
            @NonNull FilamentInstance[] instances) {
        long[] nativeInstances = new long[instances.length];
        long nativeAsset = nCreateInstancedAsset(mNativeObject, buffer, buffer.remaining(),
                nativeInstances);
        if (nativeAsset == 0) {
            return null;
        }
        FilamentAsset asset = new FilamentAsset(mEngine, nativeAsset);
        for (int i = 0; i < nativeInstances.length; i++) {
            instances[i] = new FilamentInstance(asset, nativeInstances[i]);
        }
        return asset;
    }

    /**
     * Adds a new instance to the asset.
     *
     * Use this with caution. It is more efficient to pre-allocate a max number of instances, and
     * gradually add them to the scene as needed. Instances can also be "recycled" by removing and
     * re-adding them to the scene.
     *
     * NOTE: destroyInstance() does not exist because gltfio favors flat arrays for storage of
     * entity lists and instance lists, which would be slow to shift. We also wish to discourage
     * create/destroy churn, as noted above.
     *
     * This cannot be called after FilamentAsset#releaseSourceData().
     * See also AssetLoader#createInstancedAsset().
     */
    @Nullable
    @SuppressWarnings("unused")
    public FilamentInstance createInstance(@NonNull FilamentAsset asset) {
        long nativeInstance = nCreateInstance(mNativeObject, asset.getNativeObject());
        if (nativeInstance == 0) {
            return null;
        }
        return new FilamentInstance(asset, nativeInstance);
    }

    /**
     * Allows clients to enable diagnostic shading on newly-loaded assets.
     */
    @SuppressWarnings("unused")
    public void enableDiagnostics(boolean enable) {
        nEnableDiagnostics(mNativeObject, enable);
    }

    /**
     * Frees all memory associated with the given {@link FilamentAsset}.
     */
    public void destroyAsset(@NonNull FilamentAsset asset) {
        nDestroyAsset(mNativeObject, asset.getNativeObject());
        asset.clearNativeObject();
    }

    private static native long nCreateAssetLoader(long nativeEngine, Object provider,
            long nativeEntities);
    private static native void nDestroyAssetLoader(long nativeLoader);
    private static native long nCreateAsset(long nativeLoader, Buffer buffer, int remaining);
    private static native long nCreateInstancedAsset(long nativeLoader, Buffer buffer, int remaining,
            long[] nativeInstances);
    private static native long nCreateInstance(long nativeLoader, long nativeAsset);
    private static native void nEnableDiagnostics(long nativeLoader, boolean enable);
    private static native void nDestroyAsset(long nativeLoader, long nativeAsset);
}
