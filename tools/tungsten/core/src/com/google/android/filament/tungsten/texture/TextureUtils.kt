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

package com.google.android.filament.tungsten.texture

import com.google.android.filament.Engine
import com.google.android.filament.Texture

import javax.imageio.ImageIO
import java.awt.image.BufferedImage
import java.awt.image.DataBufferByte
import java.io.File
import java.io.IOException
import java.nio.ByteBuffer
import java.nio.ByteOrder

private fun Texture.InternalFormat.toSrgb(): Texture.InternalFormat {
    return when (this) {
        Texture.InternalFormat.RGB8 -> Texture.InternalFormat.SRGB8
        Texture.InternalFormat.RGBA8 -> Texture.InternalFormat.SRGB8_A8
        else -> this
    }
}

object TextureUtils {

    fun createDefaultTexture(engine: Engine): Texture {
        val stream = javaClass.classLoader.getResourceAsStream("checkerboard.png")
        val texture = stream?.let { s ->
            ImageIO.read(s)?.let {
                img -> loadTextureFromImage(engine, img)
            }
        }
        return texture ?: throw RuntimeException("Could not load default texture")
    }

    fun loadTexture(engine: Engine, file: File): Texture? {
        val img = try {
            ImageIO.read(file) ?: return null
        } catch (e: IOException) {
            System.err.println("Could not read image ${file.canonicalPath}.")
            e.printStackTrace()
            return null
        }

        return loadTextureFromImage(engine, img)
    }

    /**
     * Based on the number of components the image has and its color space, decide which
     * texture formats to use.
     */
    private fun decideTextureFormat(
        img: BufferedImage
    ): Pair<Texture.InternalFormat, Texture.Format> {
        val isSrgb = img.colorModel.colorSpace.isCS_sRGB
        val (internalFormat, textureFormat) = when (img.colorModel.numComponents) {
            1 -> Texture.InternalFormat.R8 to Texture.Format.R
            2 -> Texture.InternalFormat.RG8 to Texture.Format.RG
            3 -> Texture.InternalFormat.RGB8 to Texture.Format.RGB
            4 -> Texture.InternalFormat.RGBA8 to Texture.Format.RGBA
            else -> Texture.InternalFormat.RGBA8 to Texture.Format.RGBA
        }
        return (if (isSrgb) internalFormat.toSrgb() else internalFormat) to textureFormat
    }

    private fun loadTextureFromImage(engine: Engine, img: BufferedImage): Texture? {
        val data = img.data.dataBuffer as DataBufferByte
        val pixels = data.data

        val (internalFormat, textureFormat) = decideTextureFormat(img)

        flipComponents(img.colorModel.numComponents, pixels)

        val texture = Texture.Builder()
                .width(img.width)
                .height(img.height)
                .format(internalFormat)
                .sampler(Texture.Sampler.SAMPLER_2D)
                .build(engine)

        val buf = ByteBuffer.wrap(pixels)
        buf.order(ByteOrder.BIG_ENDIAN)

        val desc = Texture.PixelBufferDescriptor(buf, textureFormat, Texture.Type.UBYTE)

        texture.setImage(engine, Texture.BASE_LEVEL, desc)

        return texture
    }

    private fun flipComponents(numComponents: Int, pixels: ByteArray) {
        if (numComponents == 4) {
            // ABGR -> RGBA
            assert(pixels.size % 4 == 0)
            var i = 0
            while (i < pixels.size) {
                val A = pixels[i]
                val B = pixels[i + 1]
                val G = pixels[i + 2]
                val R = pixels[i + 3]
                pixels[i] = R
                pixels[i + 1] = G
                pixels[i + 2] = B
                pixels[i + 3] = A
                i += 4
            }
        }
        if (numComponents == 3) {
            // ABGR -> RGBA
            assert(pixels.size % 3 == 0)
            var i = 0
            while (i < pixels.size) {
                val B = pixels[i]
                val G = pixels[i + 1]
                val R = pixels[i + 2]
                pixels[i] = R
                pixels[i + 1] = G
                pixels[i + 2] = B
                i += 3
            }
        }
    }
}
