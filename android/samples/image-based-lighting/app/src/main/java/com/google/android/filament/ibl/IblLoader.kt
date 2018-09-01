/*
 * Copyright (C) 2018 The Android Open Source Project
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

package com.google.android.filament.ibl

import android.content.res.AssetManager
import android.graphics.BitmapFactory

import com.google.android.filament.Engine
import com.google.android.filament.IndirectLight
import com.google.android.filament.Skybox
import com.google.android.filament.Texture
import java.io.BufferedReader
import java.io.InputStreamReader

import java.nio.ByteBuffer

import kotlin.math.log2

private fun peekSize(assets: AssetManager, name: String): Pair<Int, Int> {
    val input = assets.open(name)
    val opts = BitmapFactory.Options().apply { inJustDecodeBounds = true }
    BitmapFactory.decodeStream(input, null, opts)
    input.close()
    return opts.outWidth to opts.outHeight
}

fun loadIbl(assets: AssetManager, name: String, engine: Engine): IndirectLight {
    val (w, h) = peekSize(assets, "$name/nx.rgbm")
    val texture = Texture.Builder()
            .width(w)
            .height(h)
            .levels(log2(w.toFloat()).toInt() + 1)
            .format(Texture.InternalFormat.RGBM)
            .sampler(Texture.Sampler.SAMPLER_CUBEMAP)
            .build(engine)

    (0 until texture.levels).forEach {
        loadCubemap(texture, assets, name, engine, "m${it}_", it)
    }

    val sphericalHarmonics = loadSphericalHarmonics(assets, name)

    return IndirectLight.Builder()
            .reflections(texture)
            .irradiance(3, sphericalHarmonics)
            .intensity(30_000.0f)
            .build(engine)
}

private fun loadSphericalHarmonics(assets: AssetManager, name: String): FloatArray {
    // 3 bands = 9 RGB coefficients, so 9 * 3 floats
    val sphericalHarmonics = FloatArray(9 * 3)
    val input = BufferedReader(InputStreamReader(assets.open("$name/sh.txt")))
    val re = Regex("""\(\s*([+-]?\d+\.\d+),\s*([+-]?\d+\.\d+),\s*([+-]?\d+\.\d+)\);""")
    (0 until 9).forEach { i ->
        val line = input.readLine()
        re.find(line)?.let {
            sphericalHarmonics[i * 3] = it.groups[1]?.value?.toFloat() ?: 0.0f
            sphericalHarmonics[i * 3 + 1] = it.groups[2]?.value?.toFloat() ?: 0.0f
            sphericalHarmonics[i * 3 + 2] = it.groups[3]?.value?.toFloat() ?: 0.0f
        }
    }
    input.close()
    return sphericalHarmonics
}

fun loadSkybox(assets: AssetManager, name: String, engine: Engine): Skybox {
    val (w, h) = peekSize(assets, "$name/nx.rgbm")
    val texture = Texture.Builder()
            .width(w)
            .height(h)
            .levels(1)
            .format(Texture.InternalFormat.RGBM)
            .sampler(Texture.Sampler.SAMPLER_CUBEMAP)
            .build(engine)

    loadCubemap(texture, assets, name, engine)

    return Skybox.Builder().environment(texture).build(engine)
}

private fun loadCubemap(texture: Texture, assets: AssetManager, name: String,
        engine: Engine, prefix: String = "", level: Int = 0) {

    // RGBM is always 4 bytes per pixel
    val faceSize = texture.getWidth(level) * texture.getHeight(level) * 4
    val offsets = IntArray(6) { it * faceSize }

    val opts = BitmapFactory.Options().apply { inPremultiplied = false }

    val storage = ByteBuffer.allocateDirect(faceSize * 6)

    val suffix = arrayOf("px", "nx", "py", "ny", "pz", "nz")
    (0 until 6).forEach {
        val input = assets.open("$name/$prefix${suffix[it]}.rgbm")
        val bitmap = BitmapFactory.decodeStream(input, null, opts)
        input.close()

        bitmap?.copyPixelsToBuffer(storage)
    }

    // Rewind the texture buffer
    storage.flip()

    val buffer = Texture.PixelBufferDescriptor(storage, Texture.Format.RGBM, Texture.Type.UBYTE)
    texture.setImage(engine, level, buffer, offsets)
}
