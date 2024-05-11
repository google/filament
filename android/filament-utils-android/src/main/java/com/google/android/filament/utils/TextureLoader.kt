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

package com.google.android.filament.utils

import android.content.res.Resources
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import com.google.android.filament.Engine
import com.google.android.filament.Texture
import com.google.android.filament.android.TextureHelper
import java.lang.IllegalArgumentException
import java.nio.ByteBuffer

const val SKIP_BITMAP_COPY = true

enum class TextureType {
    COLOR,
    NORMAL,
    DATA
}

fun loadTexture(engine: Engine, resources: Resources, resourceId: Int, type: TextureType): Texture {
    val options = BitmapFactory.Options()
    // Color is the only type of texture we want to pre-multiply with the alpha channel
    // Pre-multiplication is the default behavior, so we need to turn it off here
    options.inPremultiplied = type == TextureType.COLOR

    val bitmap = BitmapFactory.decodeResource(resources, resourceId, options)

    val texture = Texture.Builder()
            .width(bitmap.width)
            .height(bitmap.height)
            .sampler(Texture.Sampler.SAMPLER_2D)
            .format(internalFormat(type))
            // This tells Filament to figure out the number of mip levels
            .levels(0xff)
            .build(engine)

    // TextureHelper offers a method that skips the copy of the bitmap into a ByteBuffer
    if (SKIP_BITMAP_COPY) {
        TextureHelper.setBitmap(engine, texture, 0, bitmap)
    } else {
        val buffer = ByteBuffer.allocateDirect(bitmap.byteCount)
        bitmap.copyPixelsToBuffer(buffer)
        // Do not forget to rewind the buffer!
        buffer.flip()

        val descriptor = Texture.PixelBufferDescriptor(
                buffer,
                format(bitmap),
                type(bitmap))

        texture.setImage(engine, 0, descriptor)
    }

    texture.generateMipmaps(engine)

    return texture
}

private fun internalFormat(type: TextureType) = when (type) {
    TextureType.COLOR  -> Texture.InternalFormat.SRGB8_A8
    TextureType.NORMAL -> Texture.InternalFormat.RGBA8
    TextureType.DATA   -> Texture.InternalFormat.RGBA8
}

// Not required when SKIP_BITMAP_COPY is true
// Use String representation for compatibility across API levels
private fun format(bitmap: Bitmap) = when (bitmap.config.name) {
    "ALPHA_8"   -> Texture.Format.ALPHA
    "RGB_565"   -> Texture.Format.RGB
    "ARGB_8888" -> Texture.Format.RGBA
    "RGBA_F16"  -> Texture.Format.RGBA
    else -> throw IllegalArgumentException("Unknown bitmap configuration")
}

// Not required when SKIP_BITMAP_COPY is true
private fun type(bitmap: Bitmap) = when (bitmap.config.name) {
    "ALPHA_8"   -> Texture.Type.USHORT
    "RGB_565"   -> Texture.Type.USHORT_565
    "ARGB_8888" -> Texture.Type.UBYTE
    "RGBA_F16"  -> Texture.Type.HALF
    else -> throw IllegalArgumentException("Unsupported bitmap configuration")
}
