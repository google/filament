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
 * Consumes a blob of glTF 2.0 content (either JSON or GLB) and produces {@link FilamentAsset}
 * objects, which are bundles of Filament entities, material instances, textures, vertex buffers,
 * and index buffers.
 *
 * <p>AssetLoader does not fetch external buffer data or create textures on its own. Clients can use
 * the provided {@link ResourceLoader} class for this, which obtains the URI list from the asset.
 * This is demonstrated in the Kotlin snippet below.</p>
 *
 * <pre>
 *
 * companion object {
 *     init {
 *        AssetLoader.init()
 *    }
 * }
 *
 * override fun onCreate(savedInstanceState: Bundle?) {
 *
 *     ...
 *
 *     assetLoader = AssetLoader(engine, MaterialProvider(engine), EntityManager.get())
 *
 *     filamentAsset = assets.open("models/lucy.glb").use { input ->
 *         val bytes = ByteArray(input.available())
 *         input.read(bytes)
 *         assetLoader.createAssetFromBinary(ByteBuffer.wrap(bytes))!!
 *     }
 *
 *     val resourceLoader = ResourceLoader(engine)
 *     resourceLoader.loadResources(filamentAsset)
 *     for (uri in filamentAsset.resourceUris) {
 *         val buffer = loadResource(uri)
 *         resourceLoader.addResourceData(uri, buffer)
 *     }
 *     resourceLoader.destroy()
 *     animator = asset.getAnimator()
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

    /**
     * Initializes the gltfio JNI layer. Must be called before using any gltfio functionality.
     */
    public static void init() {
        System.loadLibrary("gltfio-jni");
    }

    /**
     * Constructs an <code>AssetLoader </code>that can be used to create and destroy instances of
     * {@link FilamentAsset}.
     *
     * @param engine the engine that the loader should pass to builder objects
     * @param generator specifies if materials should be generated or loaded from a pre-built set
     * @param entities the EntityManager that should be used to create entities
     */
    public AssetLoader(@NonNull Engine engine, @NonNull MaterialProvider generator,
            @NonNull EntityManager entities) {

        long nativeEngine = engine.getNativeObject();
        long nativeMaterials = generator.getNativeObject();
        long nativeEntities = entities.getNativeObject();
        mNativeObject = nCreateAssetLoader(nativeEngine, nativeMaterials, nativeEntities);

        if (mNativeObject == 0) {
            throw new IllegalStateException("Unable to parse glTF asset.");
        }
    }

    /**
     * Frees all memory consumed by the native <code>AssetLoader</code> and its material cache.
     */
    public void destroy() {
        nDestroyAssetLoader(mNativeObject);
        mNativeObject = 0;
    }

    /**
     * Creates a {@link FilamentAsset} from the contents of a GLB file.
     */
    @Nullable
    public FilamentAsset createAssetFromBinary(@NonNull Buffer buffer) {
        long nativeAsset = nCreateAssetFromBinary(mNativeObject, buffer, buffer.remaining());
        return new FilamentAsset(nativeAsset);
    }

    /**
     * Creates a {@link FilamentAsset} from the contents of a GLTF file.
     */
    @Nullable
    public FilamentAsset createAssetFromJson(@NonNull Buffer buffer) {
        long nativeAsset = nCreateAssetFromJson(mNativeObject, buffer, buffer.remaining());
        return new FilamentAsset(nativeAsset);
    }

    /**
     * Allows clients to enable diagnostic shading on newly-loaded assets.
     */
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

    private static native long nCreateAssetLoader(long nativeEngine, long nativeGenerator,
            long nativeEntities);
    private static native void nDestroyAssetLoader(long nativeLoader);
    private static native long nCreateAssetFromBinary(long nativeLoader, Buffer buffer, int remaining);
    private static native long nCreateAssetFromJson(long nativeLoader, Buffer buffer, int remaining);
    private static native void nEnableDiagnostics(long nativeLoader, boolean enable);
    private static native void nDestroyAsset(long nativeLoader, long nativeAsset);
}
