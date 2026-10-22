/*
 * Copyright 2025 The Android Open Source Project
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
@file:JvmName("TexturesUtils")

package androidx.webgpu.helper

import android.graphics.Bitmap
import androidx.webgpu.*
import java.nio.ByteBuffer

public fun Bitmap.createGpuTexture(device: GPUDevice): GPUTexture {
    val size = GPUExtent3D(width = width, height = height)
    return device.createTexture(
        GPUTextureDescriptor(
            size = size,
            format = TextureFormat.RGBA8Unorm,
            usage = TextureUsage.TextureBinding or TextureUsage.CopyDst or
                    TextureUsage.RenderAttachment
        )
    ).also { texture ->
        ByteBuffer.allocateDirect(height * width * Int.SIZE_BYTES).let { pixels ->
            copyPixelsToBuffer(pixels)
            device.queue.writeTexture(
                dataLayout = GPUTexelCopyBufferLayout(
                    bytesPerRow = width * Int.SIZE_BYTES,
                    rowsPerImage = height
                ),
                data = pixels,
                destination = GPUTexelCopyTextureInfo(texture = texture),
                writeSize = size
            )
        }
    }
}

public suspend fun GPUTexture.createBitmap(device: GPUDevice): Bitmap {
    if (width % 64 > 0) {
        throw DawnException("Texture must be a multiple of 64. Was ${width}")
    }

    val size = width * height * Int.SIZE_BYTES
    val readbackBuffer = device.createBuffer(
        GPUBufferDescriptor(
            size = size.toLong(),
            usage = BufferUsage.CopyDst or BufferUsage.MapRead
        )
    )
    device.queue.submit(arrayOf(device.createCommandEncoder().let {
        it.copyTextureToBuffer(
            source = GPUTexelCopyTextureInfo(texture = this),
            destination = GPUTexelCopyBufferInfo(
                layout = GPUTexelCopyBufferLayout(
                    offset = 0,
                    bytesPerRow = width * Int.SIZE_BYTES,
                    rowsPerImage = height
                ), buffer = readbackBuffer
            ),
            copySize = GPUExtent3D(width = width, height = height)
        )
        it.finish()
    }))

    readbackBuffer.mapAndAwait(MapMode.Read, 0, size.toLong())

    return Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888).apply {
        copyPixelsFromBuffer(readbackBuffer.getConstMappedRange(size = readbackBuffer.size))
        readbackBuffer.unmap()
    }
}
