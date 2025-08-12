package android.dawn.helper

import android.dawn.*
import android.graphics.Bitmap
import java.nio.ByteBuffer

public fun Bitmap.createGpuTexture(device: Device): Texture {
    val size = Extent3D(width = width, height = height)
    return device.createTexture(
        TextureDescriptor(
            size = size,
            format = TextureFormat.RGBA8Unorm,
            usage = TextureUsage.TextureBinding or TextureUsage.CopyDst or
                    TextureUsage.RenderAttachment
        )
    ).also { texture ->
        ByteBuffer.allocateDirect(height * width * Int.SIZE_BYTES).let { pixels ->
            copyPixelsToBuffer(pixels)
            device.queue.writeTexture(
                dataLayout = TexelCopyBufferLayout(
                    bytesPerRow = width * Int.SIZE_BYTES,
                    rowsPerImage = height
                ),
                data = pixels,
                destination = TexelCopyTextureInfo(texture = texture),
                writeSize = size
            )
        }
    }
}

public suspend fun Texture.createBitmap(device: Device): Bitmap {
    if (width % 64 > 0) {
        throw DawnException("Texture must be a multiple of 64. Was ${width}")
    }

    val size = width * height * Int.SIZE_BYTES
    val readbackBuffer = device.createBuffer(
        BufferDescriptor(
            size = size.toLong(),
            usage = BufferUsage.CopyDst or BufferUsage.MapRead
        )
    )!!
    device.queue.submit(arrayOf(device.createCommandEncoder().let {
        it.copyTextureToBuffer(
            source = TexelCopyTextureInfo(texture = this),
            destination = TexelCopyBufferInfo(
                layout = TexelCopyBufferLayout(
                    offset = 0,
                    bytesPerRow = width * Int.SIZE_BYTES,
                    rowsPerImage = height
                ), buffer = readbackBuffer
            ),
            copySize = Extent3D(width = width, height = height)
        )
        it.finish()
    }))

    readbackBuffer.mapAsync(MapMode.Read, 0, size.toLong())

    return Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888).apply {
        copyPixelsFromBuffer(readbackBuffer.getConstMappedRange(size = readbackBuffer.size))
        readbackBuffer.unmap()
    }
}
