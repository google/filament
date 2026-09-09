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
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import java.nio.ByteOrder
import java.util.concurrent.Executor
import java.util.concurrent.Executors
import junit.framework.TestCase
import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.ExecutorCoroutineDispatcher
import kotlinx.coroutines.asCoroutineDispatcher
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assume
import org.junit.Before
import org.junit.Rule
import org.junit.Test

@Suppress("UNUSED_VARIABLE")
@SmallTest
class QuerySetTest {

  private val dispatcher: CoroutineDispatcher = Executors.newSingleThreadExecutor { runnable ->
    Thread(runnable, "Test-WebGPU-Thread")
  }.asCoroutineDispatcher()
  private val testScope = CoroutineScope(dispatcher)
  private lateinit var device: GPUDevice
  private lateinit var webGpu: WebGpu

  @get:Rule
  val apiSkipRule = ApiLevelSkipRule()

  @Before
  fun setup(): Unit = runBlocking {
    val gpu = createWebGpu(
      dispatcher,
      deviceDescriptor = GPUDeviceDescriptor(
        requiredFeatures = intArrayOf(FeatureName.TimestampQuery),
        deviceLostCallbackExecutor = Executor(Runnable::run),
        deviceLostCallback = null,
        uncapturedErrorCallbackExecutor = Executor(Runnable::run),
        uncapturedErrorCallback = null
      ),
    )  // Request timestamp feature if available.
    webGpu = gpu
    device = gpu.device
    testScope.launch {
      webGpu.processEventsLoop()
    }
  }

  @After
  fun teardown() {
    if (::webGpu.isInitialized) {
      webGpu.close()
    }
    testScope.cancel()
    (dispatcher as? ExecutorCoroutineDispatcher)?.close()
  }

  companion object {
    private const val QUERY_COUNT = 2
  }


  /** Helper to create a destination buffer for resolveQuerySet */
  private fun createResolveBuffer(size: Long): GPUBuffer {
    return device.createBuffer(
      GPUBufferDescriptor(
        size = size,
        usage = BufferUsage.QueryResolve or BufferUsage.CopySrc  // CopySrc for potential readback.
      )
    )
  }

  @Test
  fun testCreateOcclusionQuerySet() {
    runBlocking {
      val unused = webGpu.execute {
        val querySet = device.createQuerySet(
          GPUQuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
        )
        TestCase.assertNotNull(querySet)
        assertEquals(QueryType.Occlusion, querySet.type)
        assertEquals(QUERY_COUNT, querySet.count)
        querySet.destroy()
      }
    }
  }

  @Test
  fun testCreateTimestampQuerySet() {
    runBlocking {
      val unused = webGpu.execute {
        // Timestamp query requires a specific feature.
        Assume.assumeTrue(
          "testCreateTimestampQuerySet: TimestampQuery feature not supported.",
          device.hasFeature(FeatureName.TimestampQuery)
        )

        val querySet = device.createQuerySet(
          GPUQuerySetDescriptor(type = QueryType.Timestamp, count = QUERY_COUNT)
        )
        TestCase.assertNotNull(querySet)
        assertEquals(QueryType.Timestamp, querySet.type)
        assertEquals(QUERY_COUNT, querySet.count)
        querySet.destroy()
      }
    }
  }

  @Test
  fun testCreateQuerySetWithNegativeCountFails() {
    runBlocking {
      val unused = webGpu.execute {
        // Attempting to create a QuerySet with count -1 should fail validation.
        device.pushErrorScope(ErrorFilter.Validation)
        val unusedQuerySet =
          device.createQuerySet(GPUQuerySetDescriptor(type = QueryType.Occlusion, count = -1))
        val unusedAssert = assertThrowsSuspend(ValidationException::class.java) {
          val unusedPop = device.popErrorScope()
        }
      }
    }
  }

  @Test
  fun testGetCount() {
    runBlocking {
      val unused = webGpu.execute {
        val querySet = device.createQuerySet(
          GPUQuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
        )

        assertEquals(QUERY_COUNT, querySet.count)
      }
    }
  }

  @Test
  fun testGetType() {
    runBlocking {
      val unused = webGpu.execute {
        val querySet = device.createQuerySet(
          GPUQuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
        )
        assertEquals(QueryType.Occlusion, querySet.type)
        querySet.destroy()

        Assume.assumeTrue(
          "testGetType: TimestampQuery feature not supported.",
          device.hasFeature(FeatureName.TimestampQuery)
        )
        val tsQuerySet = device.createQuerySet(
          GPUQuerySetDescriptor(type = QueryType.Timestamp, count = QUERY_COUNT)
        )
        assertEquals(QueryType.Timestamp, tsQuerySet.type)
        tsQuerySet.destroy()
      }
    }
  }


  @Test
  fun testResolveQuerySetValid() {
    runBlocking {
      val unused = webGpu.execute {
        val querySet = device.createQuerySet(
          GPUQuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
        )
        // Each individual query result is stored as a 64-bit unsigned integer.
        val destinationBuffer = createResolveBuffer((QUERY_COUNT * Long.SIZE_BYTES).toLong())

        val encoder = device.createCommandEncoder()
        encoder.resolveQuerySet(querySet, 0, QUERY_COUNT, destinationBuffer, 0)

        device.pushErrorScope(ErrorFilter.Validation)
        val unusedCommandBuffer = encoder.finish()
        val error = device.popErrorScope()

        assertEquals(ErrorType.NoError, error)

        querySet.destroy()
        destinationBuffer.destroy()
      }
    }
  }

  @Test
  fun testResolveQuerySetInvalidDestinationUsage() {
    runBlocking {
      val unused = webGpu.execute {
        val querySet = device.createQuerySet(
          GPUQuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
        )
        // Create buffer *without* QueryResolve usage.
        val invalidBuffer = device.createBuffer(
          GPUBufferDescriptor(size = 8, usage = BufferUsage.CopySrc)
        )

        val encoder = device.createCommandEncoder()
        encoder.resolveQuerySet(querySet, 0, QUERY_COUNT, invalidBuffer, 0)  // Invalid usage.

        device.pushErrorScope(ErrorFilter.Validation)
        val unusedCommandBuffer = encoder.finish()
        val unusedAssert = assertThrowsSuspend(ValidationException::class.java) {
          val unusedPop = device.popErrorScope()
        }
        invalidBuffer.destroy()
        querySet.destroy()
      }
    }
  }

  @Test
  fun testResolveQuerySetDestinationTooSmall() {
    runBlocking {
      val unused = webGpu.execute {
        val querySet = device.createQuerySet(
          GPUQuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
        )
        // GPUBuffer only has space for 1 result (8 bytes), but we try to resolve 2.
        val smallBuffer = createResolveBuffer((1 * Long.SIZE_BYTES).toLong())

        val encoder = device.createCommandEncoder()
        encoder.resolveQuerySet(querySet, 0, QUERY_COUNT, smallBuffer, 0)  // Invalid size.

        device.pushErrorScope(ErrorFilter.Validation)
        val unusedCommandBuffer = encoder.finish()
        val unusedAssert = assertThrowsSuspend(ValidationException::class.java) {
          val unusedPop = device.popErrorScope()
        }
        smallBuffer.destroy()
        querySet.destroy()
      }
    }
  }

  @Test
  fun testResolveQuerySetIndexOutOfBounds() {
    runBlocking {
      val unused = webGpu.execute {
        val querySet = device.createQuerySet(
          GPUQuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
        )
        // Each individual query result is stored as a 64-bit unsigned integer.
        val destinationBuffer = createResolveBuffer(QUERY_COUNT * 8L)

        val encoder = device.createCommandEncoder()
        // Try to resolve starting at index 1, count 2 (goes past end).
        encoder.resolveQuerySet(querySet, 1, QUERY_COUNT, destinationBuffer, 0)  // Invalid range.

        device.pushErrorScope(ErrorFilter.Validation)
        val unusedCommandBuffer = encoder.finish()
        val unusedAssert = assertThrowsSuspend(ValidationException::class.java) {
          val unusedPop = device.popErrorScope()
        }
        destinationBuffer.destroy()
        querySet.destroy()
      }
    }
  }

  @Test
  fun testResolveQuerySetOffsetAlignment() {
    runBlocking {
      val unused = webGpu.execute {
        val querySet = device.createQuerySet(
          GPUQuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
        )

        // Each individual query result is stored as a 64-bit unsigned integer.
        val destinationBuffer = createResolveBuffer(QUERY_COUNT * 8L + 8L)

        val encoder = device.createCommandEncoder()
        // Try to resolve starting at offset 4 (invalid alignment).
        encoder.resolveQuerySet(querySet, 0, QUERY_COUNT, destinationBuffer, 4)  // Invalid offset.

        device.pushErrorScope(ErrorFilter.Validation)
        val unusedCommandBuffer = encoder.finish()
        val unusedAssert = assertThrowsSuspend(ValidationException::class.java) {
          val unusedPop = device.popErrorScope()
        }
        destinationBuffer.destroy()
        querySet.destroy()
      }
    }
  }

  /**
   * Helper function to execute a render pass, resolve an occlusion query,
   * and read the result back from the GPU.
   *
   * @param drawAction A lambda to execute drawing commands within the render pass.
   * @param expectedResult The expected long value (sample count) after resolving the query.
   */
  private suspend fun executeQueryResolveTest(
    drawAction: (GPURenderPassEncoder) -> Unit,
    expectedResult: Long,
  ) {
    // 1. Create resources: textures, render pipelines.
    val renderTarget = device.createTexture(
      GPUTextureDescriptor(
        size = GPUExtent3D(1, 1, 1),
        format = TextureFormat.RGBA8Unorm,
        usage = TextureUsage.RenderAttachment or TextureUsage.CopySrc
      )
    )
    val colorView = renderTarget.createView()

    val depthTexture = device.createTexture(
      GPUTextureDescriptor(
        size = GPUExtent3D(1, 1, 1),
        format = TextureFormat.Depth24Plus,
        usage = TextureUsage.RenderAttachment
      )
    )
    val depthView = depthTexture.createView()

    val vertexShaderCode = """
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32)
             -> @builtin(position) vec4<f32> {
            var positions = array<vec2<f32>, 3>(
                vec2<f32>(0.0, 0.5),
                vec2<f32>(-0.5, -0.5),
                vec2<f32>(0.5, -0.5)
            );
            let pos = positions[VertexIndex];
            return vec4<f32>(pos, 0.0, 1.0);
        }
    """.trimIndent()

    val fragmentShaderCode = """
        @fragment
        fn main() -> @location(0) vec4<f32> {
            return vec4<f32>(1.0, 0.0, 0.0, 1.0);
        }
    """.trimIndent()

    val shaderModuleVert = device.createShaderModule(
      GPUShaderModuleDescriptor(shaderSourceWGSL = GPUShaderSourceWGSL(vertexShaderCode))
    )
    val shaderModuleFrag = device.createShaderModule(
      GPUShaderModuleDescriptor(shaderSourceWGSL = GPUShaderSourceWGSL(fragmentShaderCode))
    )

    val pipeline = device.createRenderPipeline(
      GPURenderPipelineDescriptor(
        vertex = GPUVertexState(module = shaderModuleVert, entryPoint = "main"),
        fragment = GPUFragmentState(
          module = shaderModuleFrag,
          entryPoint = "main",
          targets = arrayOf(GPUColorTargetState(format = TextureFormat.RGBA8Unorm))
        ),
        primitive = GPUPrimitiveState(topology = PrimitiveTopology.TriangleList),
        depthStencil = GPUDepthStencilState(
          format = TextureFormat.Depth24Plus,
          depthWriteEnabled = OptionalBool.True,
          depthCompare = CompareFunction.Less
        )
      )
    )

    // 2. Create buffers: one for query results resolving, and a map-read buffer to read back.
    val queryCount = 1
    val resolveBufferSize = queryCount * Long.SIZE_BYTES.toLong()
    val resolveBuffer = createResolveBuffer(resolveBufferSize)

    val readbackBuffer = device.createBuffer(
      GPUBufferDescriptor(
        size = resolveBufferSize,
        usage = BufferUsage.MapRead or BufferUsage.CopyDst
      )
    )

    val querySet = device.createQuerySet(
      GPUQuerySetDescriptor(type = QueryType.Occlusion, count = queryCount)
    )

    // 3. Record rendering commands, resolving query results.
    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginRenderPass(
      GPURenderPassDescriptor(
        colorAttachments = arrayOf(
          GPURenderPassColorAttachment(
            view = colorView,
            loadOp = LoadOp.Clear,
            storeOp = StoreOp.Store,
            clearValue = GPUColor(0.0, 0.0, 0.0, 1.0)
          )
        ),
        depthStencilAttachment = GPURenderPassDepthStencilAttachment(
          view = depthView,
          depthLoadOp = LoadOp.Clear,
          depthStoreOp = StoreOp.Store,
          depthClearValue = 1.0f
        ),
        occlusionQuerySet = querySet
      )
    )

    // Execute the draw logic provided by the caller (drawAction).
    passEncoder.beginOcclusionQuery(0)
    passEncoder.setPipeline(pipeline)
    drawAction(passEncoder) // The action determines the query result
    passEncoder.endOcclusionQuery()
    passEncoder.end()

    // Resolve the query result from the QuerySet into the resolveBuffer.
    encoder.resolveQuerySet(querySet, firstQuery = 0, queryCount = queryCount, resolveBuffer, 0)
    // Copy the resolved data into the readbackBuffer for CPU access.
    encoder.copyBufferToBuffer(resolveBuffer, 0, readbackBuffer, 0, resolveBufferSize)

    val commandBuffer = encoder.finish()
    device.queue.submit(arrayOf(commandBuffer))

    device.queue.onSubmittedWorkDone()
    readbackBuffer.mapAndAwait(MapMode.Read, 0, resolveBufferSize)

    val mappedBuffer = readbackBuffer.getConstMappedRange(size = resolveBufferSize)
    // WebGPU resolves query data as a little-endian unsigned 64-bit integer.
    val result = mappedBuffer.order(ByteOrder.LITTLE_ENDIAN).getLong(0)

    assertEquals(expectedResult, result)

    readbackBuffer.unmap()
    readbackBuffer.destroy()
    resolveBuffer.destroy()
    querySet.destroy()
    renderTarget.destroy()
    depthTexture.destroy()
  }

  /**
   * Test case: Draw a triangle to get a positive occlusion query result.
   * Since the test uses a 1x1 render target with 1 sample per pixel,
   * and the triangle successfully draws, the query must return the exact sample count.
   */
  @Test
  @ApiRequirement(minApi = 35, onlySkipOnEmulator = true)
  fun testResolveQuerySetAndReadback() {
    runBlocking {
      val unused = webGpu.execute {
        executeQueryResolveTest(
          drawAction = { passEncoder -> passEncoder.draw(3) },
          expectedResult = 1L
        )
      }
    }
  }

  /**
   * Perform no drawing, which should result in an occlusion query count of 0.
   */
  @Test
  @ApiRequirement(minApi = 35, onlySkipOnEmulator = true)
  fun testResolveQuerySetWithZeroResult() {
    runBlocking {
      val unused = webGpu.execute {
        executeQueryResolveTest(
          drawAction = { },
          expectedResult = 0L
        )
      }
    }
  }
}