/*
 * Copyright (C) 2021 The Android Open Source Project
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
import com.google.android.filament.Texture

import java.nio.Buffer

/**
 * Utility for decoding an HDR file and producing a Filament texture.
 */
object HDRLoader {
    class Options {
        var desiredFormat = Texture.InternalFormat.RGB16F
    }

    /**
     * Consumes the content of an HDR file and produces a [Texture] object.
     *
     * @param engine Gets passed to the builder.
     * @param buffer The content of the HDR File.
     * @param options Loader options.
     * @return The resulting Filament texture, or null on failure.
     */
    fun createTexture(engine: Engine, buffer: Buffer, options: Options = Options()): Texture? {
        val nativeEngine = engine.nativeObject
        val nativeTexture = nCreateHDRTexture(nativeEngine, buffer, buffer.remaining(), options.desiredFormat.ordinal)
        if (nativeTexture == 0L) {
            return null;
        }
        return Texture(nativeTexture)
    }

    private external fun nCreateHDRTexture(nativeEngine: Long, buffer: Buffer, remaining: Int, format: Int): Long
}