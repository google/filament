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

import androidx.test.filters.MediumTest
import androidx.test.filters.SmallTest
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import java.nio.ByteBuffer
import java.nio.ByteOrder
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Before
import org.junit.Test

@Suppress("UNUSED_VARIABLE")
@SmallTest
class RenderPassEncoderTest {
  private var webGpu: WebGpu? = null
  private lateinit var device: GPUDevice
  private lateinit var defaultColorPipeline: GPURenderPipeline
  private lateinit var renderTarget: GPUTexture
  private lateinit var renderTargetDepth: GPUTexture
  private lateinit var renderTargetView: GPUTextureView
  private lateinit var renderTargetDepthView: GPUTextureView
  private lateinit var shaderModule: GPUShaderModule
  private lateinit var layout: GPUPipelineLayout
  private val kDepthFormat = TextureFormat.Depth24Plus

  @Before
  fun setup() = runBlocking {
    val gpu = createWebGpu()
    webGpu = gpu
    device = gpu.device

    renderTarget = device.createTexture(
      GPUTextureDescriptor(
        size = GPUExtent3D(1, 1, 1),
        format = TextureFormat.RGBA8Unorm,
        usage = TextureUsage.RenderAttachment
      )
    )
    renderTargetView = renderTarget.createView()

    renderTargetDepth = device.createTexture(
      GPUTextureDescriptor(
        size = GPUExtent3D(1, 1, 1),
        format = kDepthFormat,
        usage = TextureUsage.RenderAttachment
      )
    )
    renderTargetDepthView = renderTargetDepth.createView()

    shaderModule = device.createShaderModule(
      GPUShaderModuleDescriptor(
        shaderSourceWGSL = GPUShaderSourceWGSL(
          """
                        @vertex fn vsMain() -> @builtin(position) vec4<f32> {
                            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
                        }
                        @fragment fn fsMain() -> @location(0) vec4<f32> {
                            return vec4<f32>(1.0, 0.0, 0.0, 1.0);
                        }
                        """.trimIndent()
        )
      )
    )

    layout = device.createPipelineLayout(GPUPipelineLayoutDescriptor())

    defaultColorPipeline = device.createRenderPipeline(
      GPURenderPipelineDescriptor(
        layout = layout,
        vertex = GPUVertexState(module = shaderModule, entryPoint = "vsMain"),
        fragment = GPUFragmentState(
          module = shaderModule,
          entryPoint = "fsMain",
          targets = arrayOf(
            GPUColorTargetState(format = TextureFormat.RGBA8Unorm)
          )
        ),
        primitive = GPUPrimitiveState(topology = PrimitiveTopology.TriangleList),
      )
    )
  }

  @After
  fun teardown() {
    defaultColorPipeline.close()
    renderTargetView.close()
    renderTarget.destroy()

    renderTargetDepthView.close()
    renderTargetDepth.destroy()

    shaderModule.close()
    layout.close()

    runCatching { device.destroy() }
    webGpu?.close()
    webGpu = null
  }

  /**
   * Helper function to begin a standard (color-only) render pass.
   */
  private fun beginDefaultRenderPass(encoder: GPUCommandEncoder): GPURenderPassEncoder {
    return encoder.beginRenderPass(
      GPURenderPassDescriptor(
        colorAttachments = arrayOf(
          GPURenderPassColorAttachment(
            view = renderTargetView,
            loadOp = LoadOp.Clear,
            storeOp = StoreOp.Store,
            clearValue = GPUColor(0.0, 0.0, 0.0, 1.0)
          )
        )
      )
    )
  }

  /**
   * Helper to create a buffer for index data with 4-byte padding.
   */
  private fun createIndexBuffer(indices: ShortArray): GPUBuffer {
    val dataSize = (indices.size * Short.SIZE_BYTES).toLong()
    val paddedSize = (dataSize + 3) and -4L  // Aligns to 4 bytes.

    val buffer = device.createBuffer(
      GPUBufferDescriptor(
        size = paddedSize,
        usage = BufferUsage.Index or BufferUsage.CopyDst
      )
    )

    val data = ByteBuffer.allocateDirect(paddedSize.toInt()).order(ByteOrder.nativeOrder())
    data.asShortBuffer().put(indices)
    device.queue.writeBuffer(buffer, 0, data)
    return buffer
  }

  /**
   * Helper to create a buffer for indirect draw calls.
   */
  private fun createIndirectBuffer(data: IntArray): GPUBuffer {
    val byteBuffer = ByteBuffer.allocateDirect(data.size * Int.SIZE_BYTES)
      .order(ByteOrder.nativeOrder())
    byteBuffer.asIntBuffer().put(data)

    val buffer = device.createBuffer(
      GPUBufferDescriptor(
        size = byteBuffer.capacity().toLong(),
        usage = BufferUsage.Indirect or BufferUsage.CopyDst
      )
    )
    device.queue.writeBuffer(buffer, 0, byteBuffer)
    return buffer
  }

  @Test
  fun testDebugMarker() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)

    passEncoder.insertDebugMarker("Drawing background")

    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val error = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, error)
  }

  @Test
  fun testPopDebugGroupWithoutPushFails() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.popDebugGroup()  // Invalid call.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
  }

  @Test
  fun testDrawWithoutPipelineFails() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.draw(3)  // Invalid: pipeline not set.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
  }

  @Test
  fun testSetVertexBufferInvalidUsageFails() {
    val invalidBuffer = device.createBuffer(
      GPUBufferDescriptor(size = 16, usage = BufferUsage.CopyDst)
    )
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setVertexBuffer(0, invalidBuffer)  // Invalid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
    invalidBuffer.destroy()
  }

  @Test
  fun testDrawIndexedWithoutIndexBufferFails() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setPipeline(defaultColorPipeline)
    passEncoder.drawIndexed(3)  // Invalid: index buffer not set.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
  }

  @Test
  fun testDrawIndexedValidSucceeds() {
    val indexBuffer = createIndexBuffer(shortArrayOf(0, 1, 2))
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setPipeline(defaultColorPipeline)
    passEncoder.setIndexBuffer(indexBuffer, IndexFormat.Uint16)  // Valid.
    passEncoder.drawIndexed(3)  // Valid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorType = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorType)
    indexBuffer.destroy()
  }

  @Test
  fun testDrawIndirectInvalidBufferFails() {
    val invalidBuffer = device.createBuffer(
      GPUBufferDescriptor(size = 16, usage = BufferUsage.CopyDst)  // 4 * Int.
    )
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setPipeline(defaultColorPipeline)
    passEncoder.drawIndirect(invalidBuffer, 0)  // Invalid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
    invalidBuffer.destroy()
  }

  @Test
  fun testDrawIndexedIndirectWithoutIndexBufferFails() {
    val indirectBuffer = createIndirectBuffer(intArrayOf(3, 1, 0, 0, 0))
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setPipeline(defaultColorPipeline)
    passEncoder.drawIndexedIndirect(indirectBuffer, 0)  // Invalid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
    indirectBuffer.destroy()
  }

  @Test
  @MediumTest
  fun testDrawIndexedIndirectValidSucceeds() {
    val indirectBuffer = createIndirectBuffer(intArrayOf(3, 1, 0, 0, 0))
    val indexBuffer = createIndexBuffer(shortArrayOf(0, 1, 2))
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setPipeline(defaultColorPipeline)
    passEncoder.setIndexBuffer(indexBuffer, IndexFormat.Uint16)  // Valid.
    passEncoder.drawIndexedIndirect(indirectBuffer, 0)  // Valid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val error = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, error)
    indirectBuffer.destroy()
    indexBuffer.destroy()
  }

  @Test
  fun testOcclusionQueryValidSucceeds() {
    // This test needs its OWN depth-enabled pipeline.
    // We can't use the class-level 'defaultColorPipeline' because it's color-only.

    val depthPipeline = device.createRenderPipeline(
      GPURenderPipelineDescriptor(
        layout = layout,
        vertex = GPUVertexState(module = shaderModule, entryPoint = "vsMain"),
        fragment = GPUFragmentState(
          module = shaderModule,
          entryPoint = "fsMain",
          targets = arrayOf(
            GPUColorTargetState(format = TextureFormat.RGBA8Unorm)
          )
        ),
        primitive = GPUPrimitiveState(topology = PrimitiveTopology.TriangleList),
        depthStencil = GPUDepthStencilState(
          format = kDepthFormat,
          depthWriteEnabled = OptionalBool.Companion.True,
          depthCompare = CompareFunction.Always
        )
      )
    )

    val querySet = device.createQuerySet(
      GPUQuerySetDescriptor(type = QueryType.Occlusion, count = 1)
    )
    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginRenderPass(
      GPURenderPassDescriptor(
        colorAttachments = arrayOf(
          GPURenderPassColorAttachment(
            view = renderTargetView,
            loadOp = LoadOp.Clear,
            storeOp = StoreOp.Store,
            clearValue = GPUColor(0.0, 0.0, 0.0, 1.0)
          )
        ),
        depthStencilAttachment = GPURenderPassDepthStencilAttachment(
          view = renderTargetDepthView,
          depthLoadOp = LoadOp.Clear,
          depthStoreOp = StoreOp.Store,
          depthClearValue = 1.0f
        ),
        occlusionQuerySet = querySet
      )
    )

    passEncoder.beginOcclusionQuery(0)
    passEncoder.setPipeline(depthPipeline)  // Use the local depth-pipeline.
    passEncoder.draw(3)
    passEncoder.endOcclusionQuery()
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val error = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, error)

    querySet.destroy()
    depthPipeline.close()
  }

  @Test
  fun testSetPassPropertiesSucceeds() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setViewport(0f, 0f, 1f, 1f, 0f, 1f)
    passEncoder.setScissorRect(0, 0, 1, 1)
    passEncoder.setBlendConstant(GPUColor(0.0, 0.0, 0.0, 0.0))
    passEncoder.setStencilReference(0)
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val error = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, error)
  }

  @Test
  fun testEndCalledTwiceFails() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.end()  // First call (valid).

    device.pushErrorScope(ErrorFilter.Validation)
    passEncoder.end()  // Second call (invalid).
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
  }

  @Test
  fun testExecuteBundlesSucceeds() {
    val bundleEncoder = device.createRenderBundleEncoder(
      GPURenderBundleEncoderDescriptor(
        colorFormats = intArrayOf(TextureFormat.RGBA8Unorm)
      )
    )
    bundleEncoder.setPipeline(defaultColorPipeline)  // Use the color-only pipeline.
    bundleEncoder.draw(3)
    val bundle = bundleEncoder.finish()

    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)  // Use the color-only pass.
    passEncoder.executeBundles(arrayOf(bundle))  // Valid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val error = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, error)

    bundle.close()
  }
}