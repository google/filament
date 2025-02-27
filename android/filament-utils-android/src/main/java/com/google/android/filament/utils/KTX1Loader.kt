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
 * Utilities for consuming KTX1 files and producing Filament textures, IBLs, and sky boxes.
 *
 * KTX is a simple container format that makes it easy to bundle miplevels and cubemap faces
 * into a single file.
 */
object KTX1Loader {
    class Options {
        var srgb = false
    }

    class IndirectLightBundle(val indirectLight: IndirectLight? = null, val cubemap: Texture? = null) {
    }

    class SkyboxBundle(val skybox: Skybox? = null, val cubemap: Texture? = null) {
    }

    /**
     * Consumes the content of a KTX file and produces a [Texture] object.
     *
     * @param engine Gets passed to the builder.
     * @param buffer The content of the KTX File.
     * @param options Loader options.
     * @return The resulting Filament texture, or null on failure.
     */
    fun createTexture(engine: Engine, buffer: Buffer, options: Options = Options()): Texture {
        val nativeEngine = engine.nativeObject
        val nativeTexture = nCreateKTXTexture(nativeEngine, buffer, buffer.remaining(), options.srgb)
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
    fun createIndirectLight(engine: Engine, buffer: Buffer, options: Options = Options()): IndirectLightBundle {
        val nativeEngine = engine.nativeObject
        val sphericalHarmonics = getSphericalHarmonics(buffer)
        if (sphericalHarmonics == null) {
            return IndirectLightBundle()
        }
        val ktxTexture = createTexture(engine, buffer, options)
        val nativeIndirectLight = nCreateIndirectLight(nativeEngine, ktxTexture.nativeObject, sphericalHarmonics)
        return IndirectLightBundle(IndirectLight(nativeIndirectLight), ktxTexture)
    }

    /**
     * Consumes the content of a KTX file and produces a [Skybox] object.
     *
     * @param engine Gets passed to the builder.
     * @param buffer The content of the KTX File.
     * @param options Loader options.
     * @return The resulting Filament texture, or null on failure.
     */
    fun createSkybox(engine: Engine, buffer: Buffer, options: Options = Options()): SkyboxBundle {
        val nativeEngine = engine.nativeObject
        val ktxTexture = createTexture(engine, buffer, options)
        val nativeSkybox = nCreateSkybox(nativeEngine, ktxTexture.nativeObject)
        return SkyboxBundle(Skybox(nativeSkybox), ktxTexture)
    }

    /**
     * Retrieves spherical harmonics from the content of a KTX file.
     *
     * @param buffer The content of the KTX File.
     * @return The resulting array of 9 * 3 floats, or null on failure.
     */
    fun getSphericalHarmonics(buffer: Buffer): FloatArray? {
        val sphericalHarmonics = FloatArray(9 * 3)
        val success = nGetSphericalHarmonics(buffer, buffer.remaining(), sphericalHarmonics)
        return if (success) sphericalHarmonics else null
    }

    private external fun nCreateKTXTexture(nativeEngine: Long, buffer: Buffer, remaining: Int, srgb: Boolean): Long
    private external fun nCreateIndirectLight(nativeEngine: Long, ktxTexture: Long, sphericalHarmonics: FloatArray) : Long
    private external fun nGetSphericalHarmonics(buffer: Buffer, remaining: Int, outSphericalHarmonics: FloatArray): Boolean
    private external fun nCreateSkybox(nativeEngine: Long, ktxTexture: Long) : Long
}
