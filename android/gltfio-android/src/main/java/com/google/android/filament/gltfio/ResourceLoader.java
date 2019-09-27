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

import android.support.annotation.NonNull;
import android.util.Log;

import com.google.android.filament.Engine;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.Buffer;

/**
 * Uploads vertex buffers and textures to the GPU and computes tangents.
 *
 * <p>For a usage example, see the documentation for {@link AssetLoader}.</p>
 *
 * @see AssetLoader
 * @see FilamentAsset
 */
public class ResourceLoader {
    private final long mNativeObject;

    private static Method sEngineGetNativeObject;

    static {
        try {
            sEngineGetNativeObject = Engine.class.getDeclaredMethod("getNativeObject");
            sEngineGetNativeObject.setAccessible(true);
        } catch (NoSuchMethodException e) {
            // Cannot happen
        }
    }

    /**
     * Constructs a resource loader tied to the given Filament engine.
     *
     * @param engine the engine that gets passed to all builder methods
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     */
    public ResourceLoader(@NonNull Engine engine) throws IllegalAccessException, InvocationTargetException {
        long nativeEngine = (long) sEngineGetNativeObject.invoke(engine);
        mNativeObject = nCreateResourceLoader(nativeEngine);
    }

    /**
     * Frees all memory associated with the native resource loader.
     */
    public void destroy() {
        nDestroyResourceLoader(mNativeObject);
    }

    /**
     * Feeds the binary content of an external resource into the loader's URI cache.
     *
     * <p><code>ResourceLoader</code> does not know how to download external resources on its own
     * (for example, external resources might come from a filesystem, a database, or the internet)
     * so this method allows clients to download external resources and push them to the loader.</p>
     *
     * <p>When loading GLB files (as opposed to JSON-based glTF files), clients typically do not
     * need to call this method.</p>
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
     * Iterates through all external buffers and images and creates corresponding Filament objects
     * (vertex buffers, textures, etc), which become owned by the asset.
     *
     * <p>This is the main entry point for <code>ResourceLoader</code>, and only needs to be called
     * once.</p>
     *
     * @param asset the Filament asset that contains URI-based resources
     * @return self (for daisy chaining)
     */
    @NonNull
    public ResourceLoader loadResources(@NonNull FilamentAsset asset) {
        nLoadResources(mNativeObject, asset.getNativeObject());
        return this;
    }

    private static native long nCreateResourceLoader(long nativeEngine);
    private static native void nDestroyResourceLoader(long nativeLoader);
    private static native void nAddResourceData(long nativeLoader, String url, Buffer buffer,
            int remaining);
    private static native void nLoadResources(long nativeLoader, long nativeAsset);
}
