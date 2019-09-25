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
import android.support.annotation.Nullable;

import com.google.android.filament.Engine;
import com.google.android.filament.IndirectLight;
import com.google.android.filament.Skybox;
import com.google.android.filament.Texture;

import java.nio.Buffer;

/**
 * Utilities for consuming KTX files and producing Filament textures, IBLs, and sky boxes.
 *
 * KTX is a simple container format that makes it easy to bundle miplevels and cubemap faces
 * into a single file.
 */
public class KtxLoader {

    public static class Options {
        public boolean srgb;
    }

    /**
     * Consumes the content of a KTX file and produces a Filament Texture object.
     *
     * @param engine Gets passed to the builder.
     * @param buffer The content of the KTX File.
     * @param options Loader options.
     * @return The resulting Filament texture, or null on failure.
     */
    @Nullable
    public static Texture createTexture(@NonNull Engine engine, @NonNull Buffer buffer, @NonNull Options options) {
        try {
            long nativeEngine = (long) AssetLoader.sEngineGetNativeObject.invoke(engine);
            long nativeTexture = nCreateTexture(nativeEngine, buffer, buffer.remaining(), options.srgb);
            return AssetLoader.sTextureConstructor.newInstance(nativeTexture);
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Consumes the content of a KTX file and produces a Filament IndirectLight object.
     *
     * @param engine Gets passed to the builder.
     * @param buffer The content of the KTX File.
     * @param options Loader options.
     * @return The resulting Filament texture, or null on failure.
     */
    @Nullable
    public static IndirectLight createIndirectLight(@NonNull Engine engine, @NonNull Buffer buffer, @NonNull Options options) {
        try {
            long nativeEngine = (long) AssetLoader.sEngineGetNativeObject.invoke(engine);
            long nativeIndirectLight = nCreateIndirectLight(nativeEngine, buffer, buffer.remaining(), options.srgb);
            return AssetLoader.sIndirectLightConstructor.newInstance(nativeIndirectLight);
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Consumes the content of a KTX file and produces a Filament Skybox object.
     *
     * @param engine Gets passed to the builder.
     * @param buffer The content of the KTX File.
     * @param options Loader options.
     * @return The resulting Filament texture, or null on failure.
     */
    @Nullable
    public static Skybox createSkybox(@NonNull Engine engine, @NonNull Buffer buffer, @NonNull Options options) {
        try {
            long nativeEngine = (long) AssetLoader.sEngineGetNativeObject.invoke(engine);
            long nativeSkybox = nCreateSkybox(nativeEngine, buffer, buffer.remaining(), options.srgb);
            return AssetLoader.sSkyboxConstructor.newInstance(nativeSkybox);
        } catch (Exception e) {
            return null;
        }
    }

    private static native long nCreateTexture(long nativeEngine, Buffer buffer, int remaining, boolean srgb);
    private static native long nCreateIndirectLight(long nativeEngine, Buffer buffer, int remaining, boolean srgb);
    private static native long nCreateSkybox(long nativeEngine, Buffer buffer, int remaining, boolean srgb);
}
