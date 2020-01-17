/*
 * Copyright (C) 2020 The Android Open Source Project
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

package com.google.android.filament.utils

import com.google.android.filament.Engine
import com.google.android.filament.IndirectLight
import com.google.android.filament.Skybox
import com.google.android.filament.Texture

import java.nio.Buffer

/**
 * Utilities for consuming KTX files and producing Filament textures, IBLs, and sky boxes.
 *
 * KTX is a simple container format that makes it easy to bundle miplevels and cubemap faces
 * into a single file.
 */
object KtxLoader {
    class Options {
        var srgb = false
    }

    /**
     * Consumes the content of a KTX file and produces a [Texture] object.
     *
     * @param engine Gets passed to the builder.
     * @param buffer The content of the KTX File.
     * @param options Loader options.
     * @return The resulting Filament texture, or null on failure.
     */
    fun createTexture(engine: Engine, buffer: Buffer, options: Options): Texture {
        val nativeEngine = engine.nativeObject
        val nativeTexture = nCreateTexture(nativeEngine, buffer, buffer.remaining(), options.srgb)
        return Texture(nativeTexture)
    }

    /**
     * Consumes the content of a KTX file and produces an [IndirectLight] object.
     *
     * @param engine Gets passed to the builder.
     * @param buffer The content of the KTX File.
     * @param options Loader options.
     * @return The resulting Filament texture, or null on failure.
     */
    fun createIndirectLight(engine: Engine, buffer: Buffer, options: Options): IndirectLight {
        val nativeEngine = engine.nativeObject
        val nativeIndirectLight = nCreateIndirectLight(nativeEngine, buffer, buffer.remaining(), options.srgb)
        return IndirectLight(nativeIndirectLight)
    }

    /**
     * Consumes the content of a KTX file and produces a [Skybox] object.
     *
     * @param engine Gets passed to the builder.
     * @param buffer The content of the KTX File.
     * @param options Loader options.
     * @return The resulting Filament texture, or null on failure.
     */
    fun createSkybox(engine: Engine, buffer: Buffer, options: Options): Skybox {
        val nativeEngine = engine.nativeObject
        val nativeSkybox = nCreateSkybox(nativeEngine, buffer, buffer.remaining(), options.srgb)
        return Skybox(nativeSkybox)
    }

    private external fun nCreateTexture(nativeEngine: Long, buffer: Buffer, remaining: Int, srgb: Boolean): Long
    private external fun nCreateIndirectLight(nativeEngine: Long, buffer: Buffer, remaining: Int, srgb: Boolean): Long
    private external fun nCreateSkybox(nativeEngine: Long, buffer: Buffer, remaining: Int, srgb: Boolean): Long
}