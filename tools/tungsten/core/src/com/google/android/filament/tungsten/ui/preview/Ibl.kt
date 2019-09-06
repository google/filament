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

package com.google.android.filament.tungsten.ui.preview

import com.google.android.filament.Engine
import com.google.android.filament.Skybox
import com.google.android.filament.Texture
import com.google.android.filament.tungsten.texture.TextureCache
import com.google.android.filament.tungsten.texture.TextureUtils
import java.io.InputStream
import java.lang.RuntimeException
import java.nio.ByteBuffer
import kotlin.math.log2

private const val float = """([0-9-.]+)"""
private val pattern = Regex("""\(\s*$float\s*,\s*$float\s*,\s*$float\s*\);""")

internal fun parseSphereHarmonics(harmonics: String): FloatArray {
    return harmonics.lines().fold(FloatArray(0)) { acc, line ->
        val match = pattern.find(line)
        val floats = match?.groups?.mapNotNull { group ->
            group?.value?.toFloatOrNull()
        }
        floats?.toFloatArray()?.let {
            acc + it
        } ?: acc
    }
}

internal class Ibl(val engine: Engine, private val pathPrefix: String) {

    val environmentMap: Texture
    val skyboxTexture: Texture
    val skybox: Skybox
    val irradiance: FloatArray

    init {
        environmentMap = loadCubemapLevel(null, 0, "m0_")
        TextureCache.addTextureForRemoval(environmentMap)
        for (i in 1 until environmentMap.levels) {
            println("Loading level $i")
            loadCubemapLevel(environmentMap, i, "m${i}_")
        }

        // Use non-prefixed images as skybox textures.
        skyboxTexture = loadCubemapLevel(null, 0, "")
        TextureCache.addTextureForRemoval(skyboxTexture)
        skybox = loadSkybox()

        irradiance = loadSphereHarmonics()
    }

    private fun loadSkybox(): Skybox {
        return Skybox.Builder()
                .environment(skyboxTexture)
                .showSun(true)
                .build(engine)
    }

    private fun loadSphereHarmonics(): FloatArray {
        val path = "$pathPrefix/sh.txt"
        val stream: InputStream = javaClass.classLoader.getResourceAsStream(path)
                ?: throw RuntimeException("Could not get stream for sphere harmonics at $path.")
        val contents = stream.bufferedReader().use { it.readText() }
        return parseSphereHarmonics(contents)
    }

    private fun loadCubemapLevel(texture: Texture?, level: Int, facePrefix: String): Texture {
        require(texture != null || level == 0)

        val cubemapFaces = listOf("px", "nx", "py", "ny", "pz", "nz")

        val faceOffsets = IntArray(6)

        val rawBuffers = cubemapFaces.map { face ->
            val path = "$pathPrefix/$facePrefix$face.rgb32f"
            val stream: InputStream = javaClass.classLoader.getResourceAsStream(path)
                    ?: throw RuntimeException("Could not get stream for cubemap face $path.")

            val bufferAndInfo = TextureUtils.loadImageBufferFromStream(stream)
                    ?: throw RuntimeException("Could not load cubemap face $path.")

            val (_, info) = bufferAndInfo
            if (info.width != info.height) {
                throw RuntimeException("Cubemap face $pathPrefix width != height")
            }

            bufferAndInfo
        }

        val firstFace = rawBuffers[0]
        val (_, firstFaceInfo) = firstFace
        val size = firstFaceInfo.width

        // Assuming that size is a PO2
        val levels = if (facePrefix != "") {
            (log2(size.toFloat()) + 1).toInt()
        } else {
            1
        }

        // Allocate a byte buffer large enough to hold all the faces
        val buffer = ByteBuffer.allocate(size * size * 4 * 6)
        for ((rawBuffer, _) in rawBuffers) {
            buffer.put(rawBuffer)
        }
        buffer.position(0)

        val bufferDescriptor =
                Texture.PixelBufferDescriptor(buffer, Texture.Format.RGB, Texture.Type.UINT_10F_11F_11F_REV)

        // If the texture hasn't been created yet, create it.
        val resultTexture = texture ?: Texture.Builder()
                .width(size)
                .height(size)
                .levels(levels)
                .format(Texture.InternalFormat.R11F_G11F_B10F)
                .sampler(Texture.Sampler.SAMPLER_CUBEMAP)
                .build(engine)

        for (i in 0..5) {
            faceOffsets[i] = size * size * 4 * i
        }

        resultTexture.setImage(engine, level, bufferDescriptor, faceOffsets)
        return resultTexture
    }
}
