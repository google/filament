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
package androidx.webgpu

import androidx.test.filters.SmallTest
import androidx.webgpu.DawnException
import androidx.webgpu.ValidationException
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createBitmap
import androidx.webgpu.helper.createWebGpu
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Before
import org.junit.Test

@Suppress("UNUSED_VARIABLE")
@SmallTest
class TextureTest {
  private lateinit var webGpu: WebGpu
  private lateinit var device: GPUDevice

  @Before
  fun setup() = runBlocking {
    webGpu = createWebGpu()
    device = webGpu.device
  }

  @After
  fun teardown() {
    runCatching { device.destroy() }
    webGpu.close()
  }

  @Test
  fun testTextureProperties() {
    val testTextureDescriptor = GPUTextureDescriptor(
      size = GPUExtent3D(width = 32, height = 16, depthOrArrayLayers = 1),
      format = TextureFormat.RGBA8Unorm,
      usage = TextureUsage.TextureBinding or TextureUsage.CopyDst,
      mipLevelCount = 1,
      sampleCount = 1,
      dimension = TextureDimension._2D
    )
    val texture = device.createTexture(testTextureDescriptor)
    try {
      assertEquals(testTextureDescriptor.size.width, texture.width)
      assertEquals(testTextureDescriptor.size.height, texture.height)
      assertEquals(testTextureDescriptor.size.depthOrArrayLayers, texture.depthOrArrayLayers)
      assertEquals(testTextureDescriptor.format, texture.format)
      assertEquals(testTextureDescriptor.usage, texture.usage)
      assertEquals(testTextureDescriptor.mipLevelCount, texture.mipLevelCount)
      assertEquals(testTextureDescriptor.sampleCount, texture.sampleCount)
      assertEquals(testTextureDescriptor.dimension, texture.dimension)
    } finally {
      texture.destroy()
    }
  }

  /**
   * Verifies that creating a texture with an invalid combination of usage flags
   * (e.g., multisampling with storage binding) fails validation.
   */
  @Test
  fun createTexture_withInvalidUsageCombination_fails() {
    // According to the WebGPU specification, a multisampled texture (sampleCount > 1)
    // cannot have the StorageBinding usage flag. This is a guaranteed validation error.
    val invalidDescriptor = GPUTextureDescriptor(
      size = GPUExtent3D(width = 32, height = 16, depthOrArrayLayers = 1),
      format = TextureFormat.RGBA8Unorm,  // A standard format is fine.
      sampleCount = 4,  // Multisampled.
      usage = TextureUsage.RenderAttachment or TextureUsage.StorageBinding // Invalid combination
    )

    assertThrows(ValidationException::class.java) {
      val unusedGPUTexture = device.createTexture(invalidDescriptor)
    }
  }

  /**
   * Verifies that using a view from a destroyed texture generates an uncaptured error
   * upon submission to the queue.
   */
  @Test
  fun useOfDestroyedTextureView_firesUncapturedError() {
    val textureDescriptor = GPUTextureDescriptor(
      size = GPUExtent3D(width = 32, height = 16, depthOrArrayLayers = 1),
      format = TextureFormat.RGBA8Unorm,
      usage = TextureUsage.RenderAttachment
    )

    val texture = device.createTexture(textureDescriptor)
    texture.destroy()

    val invalidView = texture.createView()

    val encoder = device.createCommandEncoder()
    try {
      val passEncoder = encoder.beginRenderPass(
        GPURenderPassDescriptor(
          colorAttachments = arrayOf(
            GPURenderPassColorAttachment(
              view = invalidView,
              loadOp = LoadOp.Clear,
              storeOp = StoreOp.Store,
              clearValue = GPUColor(r = 1.0, g = 0.0, b = 0.0, a = 1.0)
            )
          )
        )
      )
      passEncoder.end()
      val commandBuffer = encoder.finish()

      val queue = device.getQueue()
      assertThrows(ValidationException::class.java) {
        queue.submit(arrayOf(commandBuffer))
        runBlocking {
          val unusedQueueWorkDoneReturn = queue.onSubmittedWorkDone()
        }
      }
    } finally {
      encoder.close()
    }
  }

  /**
   * Verifies that creating a bitmap from a texture with an invalid width fails.
   */
  @Test
  fun createBitmap_withInvalidWidth_fails() {
    val descriptor = GPUTextureDescriptor(
      size = GPUExtent3D(width = 65, height = 16, depthOrArrayLayers = 1),
      format = TextureFormat.RGBA8Unorm,
      usage = TextureUsage.CopySrc
    )
    val texture = device.createTexture(descriptor)
    assertThrows(DawnException::class.java) {
      runBlocking {
        texture.createBitmap(device)
      }
    }
  }
}