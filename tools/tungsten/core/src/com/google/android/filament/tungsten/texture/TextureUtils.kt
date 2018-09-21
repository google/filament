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
import java.awt.image.BufferedImage.TYPE_3BYTE_BGR
import java.awt.image.BufferedImage.TYPE_4BYTE_ABGR
import java.awt.image.BufferedImage.TYPE_4BYTE_ABGR_PRE
import java.awt.image.BufferedImage.TYPE_INT_ARGB
import java.awt.image.BufferedImage.TYPE_INT_ARGB_PRE
import java.awt.image.BufferedImage.TYPE_INT_BGR
import java.awt.image.DataBufferByte
import java.io.File
import java.io.IOException
import java.io.InputStream
import java.nio.ByteBuffer

private fun Texture.InternalFormat.toSrgb(): Texture.InternalFormat {
    return when (this) {
        Texture.InternalFormat.RGB8 -> Texture.InternalFormat.SRGB8
        Texture.InternalFormat.RGBA8 -> Texture.InternalFormat.SRGB8_A8
        else -> this
    }
}

data class ImageInfo(val width: Int, val height: Int)

object TextureUtils {

    enum class ColorSpaceStrategy {
        // Use the profile present in the image file, defaulting to sRGB if none exists.
        USE_FILE_PROFILE,

        // Always assume an sRGB color space.
        FORCE_SRGB,

        // Always assume a non-sRGB color space.
        FORCE_LINEAR
    }

    fun createDefaultTexture(engine: Engine): Texture {
        val stream = javaClass.classLoader.getResourceAsStream("checkerboard.png")
        val texture = stream?.let { s ->
            ImageIO.read(s)?.let {
                img -> loadTextureFromImage(engine, img, ColorSpaceStrategy.FORCE_LINEAR)
            }
        }
        return texture ?: throw RuntimeException("Could not load default texture")
    }

    fun loadTextureFromFile(engine: Engine, file: File, colorSpace: ColorSpaceStrategy): Texture? {
        val img = try {
            ImageIO.read(file) ?: return null
        } catch (e: IOException) {
            System.err.println("Could not read image ${file.canonicalPath}.")
            e.printStackTrace()
            return null
        }

        return loadTextureFromImage(engine, img, colorSpace)
    }

    fun loadImageBufferFromStream(stream: InputStream): Pair<ByteBuffer, ImageInfo>? {
        val img = try {
            ImageIO.read(stream) ?: return null
        } catch (e: IOException) {
            System.err.println("Could not parse image from InputStream.")
            e.printStackTrace()
            return null
        }
        return Pair(loadImageBuffer(img), ImageInfo(img.width, img.height))
    }

    /**
     * Based on the number of components the image has and its color space, decide which
     * texture formats to use.
     */
    private fun decideTextureFormat(
        img: BufferedImage,
        colorSpace: ColorSpaceStrategy
    ): Pair<Texture.InternalFormat, Texture.Format> {
        val isSrgb = when (colorSpace) {
            ColorSpaceStrategy.FORCE_LINEAR -> false
            ColorSpaceStrategy.FORCE_SRGB -> true
            ColorSpaceStrategy.USE_FILE_PROFILE -> img.colorModel.colorSpace.isCS_sRGB
        }
        val (internalFormat, textureFormat) = when (img.colorModel.numComponents) {
            1 -> Texture.InternalFormat.R8 to Texture.Format.R
            2 -> Texture.InternalFormat.RG8 to Texture.Format.RG
            3 -> Texture.InternalFormat.RGB8 to Texture.Format.RGB
            4 -> Texture.InternalFormat.RGBA8 to Texture.Format.RGBA
            else -> Texture.InternalFormat.RGBA8 to Texture.Format.RGBA
        }
        return (if (isSrgb) internalFormat.toSrgb() else internalFormat) to textureFormat
    }

    private fun loadImageBuffer(img: BufferedImage): ByteBuffer {
        val data = img.raster.dataBuffer as DataBufferByte
        flipComponentsIfNecessary(img)
        return ByteBuffer.wrap(data.data)
    }

    private fun loadTextureFromImage(
        engine: Engine,
        img: BufferedImage,
        colorSpace: ColorSpaceStrategy
    ): Texture? {
        val data = img.raster.dataBuffer as DataBufferByte

        val (internalFormat, textureFormat) = decideTextureFormat(img, colorSpace)

        flipComponentsIfNecessary(img)

        val texture = Texture.Builder()
                .width(img.width)
                .height(img.height)
                .format(internalFormat)
                .sampler(Texture.Sampler.SAMPLER_2D)
                .build(engine)

        val buf = ByteBuffer.wrap(data.data)

        val desc = Texture.PixelBufferDescriptor(buf, textureFormat, Texture.Type.UBYTE)

        texture.setImage(engine, Texture.BASE_LEVEL, desc)

        return texture
    }

    private fun flipComponentsIfNecessary(img: BufferedImage) {
        val data = img.raster.dataBuffer as DataBufferByte
        val pixels = data.data
        val components = img.colorModel.numComponents

        // Only flip components if they are specified in non-RGBA order.
        val typesToFlip = listOf(
                TYPE_INT_ARGB,
                TYPE_INT_ARGB_PRE,
                TYPE_INT_BGR,
                TYPE_3BYTE_BGR,
                TYPE_4BYTE_ABGR,
                TYPE_4BYTE_ABGR_PRE
        )
        if (!typesToFlip.contains(img.type)) {
            return
        }

        if (components == 4) {
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
        if (components == 3) {
            // BGR -> RGB
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
