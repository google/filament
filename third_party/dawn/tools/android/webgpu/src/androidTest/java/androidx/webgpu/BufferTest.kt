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
import androidx.webgpu.WebGpuTestConstants.EMULATOR_TESTS_MIN_API_LEVEL
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import java.nio.ByteBuffer
import java.nio.ByteOrder
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertThrows
import org.junit.Test
import org.junit.After
import org.junit.Before
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Rule

@Suppress("UNUSED_VARIABLE")
class BufferTest {
  private lateinit var device: GPUDevice
  private lateinit var webGpu: WebGpu
  @get:Rule
  val apiSkipRule = ApiLevelSkipRule()

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

  /**
   * Test that calling getMappedRange() on a mapped buffer does not raise an exception.
   */
  @SmallTest
  @Test
  fun bufferMapTest() {
    device.createBuffer(
      GPUBufferDescriptor(
        usage = BufferUsage.Vertex,
        size = 1024,
        mappedAtCreation = true
      )
    ).apply {
     val unused = getMappedRange(size = size)
    }
  }

  /**
   * Test that calling getMappedRange() on a non-mapped buffer raises an exception.
   */
  @SmallTest
  @Test
  fun bufferMapFailureTest() {
    //TODO(b/452516879): Catch a more specific exception type.
    assertThrows(Error::class.java) {
      device.createBuffer(
        GPUBufferDescriptor(
          usage = BufferUsage.Vertex,
          size = 1024,
          mappedAtCreation = false
        )
      ).apply {
        val status = getMappedRange(size = size)
        assertEquals(MapAsyncStatus.Success, status)
      }
    }
  }

  /**
   * Tests that the size and usage properties of a buffer are correct.
   */
  @SmallTest
  @Test
  fun testBufferSizeAndUsage() {
    val bufferSize = 256L
    val bufferUsage = BufferUsage.Vertex or BufferUsage.CopyDst

    val buffer = device.createBuffer(
      GPUBufferDescriptor(
        usage = bufferUsage,
        size = bufferSize
      )
    )

    assertEquals(bufferSize, buffer.size)
    assertEquals(bufferUsage, buffer.usage)
  }

  /**
   * Tests that a buffer is no longer usable after being destroyed.
   */
  @SmallTest
  @Test
  fun testDestroy() {
    val buffer = device.createBuffer(
      GPUBufferDescriptor(
        usage = BufferUsage.MapRead,
        size = 16
      )
    )

    buffer.destroy()

    // After destroying the buffer, further operations should fail.
    assertThrows(ValidationException::class.java) {
      runBlocking {
        buffer.mapAndAwait(MapMode.Read, 0, 16)
      }
    }
  }

  /**
   * Verifies the data integrity of a full CPU -> GPU -> CPU round trip.
   */
  @MediumTest
  @Test
  @ApiRequirement(minApi = EMULATOR_TESTS_MIN_API_LEVEL, onlySkipOnEmulator = true)
  fun testWriteAndReadBuffer() {
    val queue = device.getQueue()

    val cpuData = floatArrayOf(1.1f, 2.2f, 3.3f, 4.4f, 5.5f)
    val bufferSize = cpuData.size * Float.SIZE_BYTES * 1L

    val cpuDataBuffer = ByteBuffer.allocateDirect(bufferSize.toInt()).order(ByteOrder.nativeOrder())
      .apply { asFloatBuffer().put(cpuData).rewind() }

    val gpuWriteBuffer = device.createBuffer(
      GPUBufferDescriptor(
        size = bufferSize,
        usage = BufferUsage.CopyDst or BufferUsage.CopySrc
      )
    )

    queue.writeBuffer(gpuWriteBuffer, 0, cpuDataBuffer)

    val gpuReadBuffer = device.createBuffer(
      GPUBufferDescriptor(
        size = bufferSize,
        usage = BufferUsage.CopyDst or BufferUsage.MapRead
      )
    )
    val commandEncoder = device.createCommandEncoder()
    commandEncoder.copyBufferToBuffer(
      source = gpuWriteBuffer,
      sourceOffset = 0,
      destination = gpuReadBuffer,
      destinationOffset = 0,
      size = bufferSize
    )
    val gpuCommand = commandEncoder.finish()
    queue.submit(arrayOf(gpuCommand))
    runBlocking { queue.onSubmittedWorkDone() }
    runBlocking { gpuReadBuffer.mapAndAwait(mode = MapMode.Read, size = bufferSize, offset = 0) }

    val arrayBuffer = gpuReadBuffer.getConstMappedRange(size = bufferSize).asFloatBuffer()
    gpuReadBuffer.unmap()
    val floatArray = FloatArray(arrayBuffer.remaining())
    arrayBuffer.get(floatArray)
    val delta = 0.00001f // Define an acceptable tolerance
    assertArrayEquals(
      "The GPU data does not match the original CPU data.",
      cpuData,
      floatArray,
      delta
    )
  }


  /**
   * Tests the full buffer mapping and unmapping lifecycle.
   */
  @MediumTest
  @Test
  fun testMapAsyncUnmap() {
    val bufferSize = 16L
    val buffer = device.createBuffer(
      GPUBufferDescriptor(
        usage = BufferUsage.MapRead or BufferUsage.CopyDst,
        size = bufferSize,
      )
    )

    assertEquals(BufferMapState.Unmapped, buffer.mapState)

    runBlocking { buffer.mapAndAwait(MapMode.Read, 0, bufferSize) }

    assertEquals(BufferMapState.Mapped, buffer.mapState)

    buffer.unmap()

    assertEquals(BufferMapState.Unmapped, buffer.mapState)

    //TODO(b/452516879): Catch a more specific exception type.
    // Should not be able to get the mapped range after unmapping.
    assertThrows(Error::class.java) {
      buffer.getMappedRange(0, bufferSize)
    }
  }

  /**
   * Verifies that `readMappedRange` successfully reads data from a mapped buffer.
   */
  @SmallTest
  @Test
  fun testReadMappedRangeSucceedsAndDataIsCorrect() {
    val initialData = floatArrayOf(1f, 2f, 3f, 4f)
    val bufferSize = initialData.size * Float.SIZE_BYTES * 1L

    val byteBuffer = ByteBuffer.allocateDirect(bufferSize.toInt()).order(ByteOrder.nativeOrder())
      .apply { asFloatBuffer().put(initialData).rewind() }

    val buffer = device.createBuffer(
      GPUBufferDescriptor(
        size = bufferSize,
        usage = BufferUsage.MapRead or BufferUsage.CopyDst
      )
    )
    device.getQueue().writeBuffer(buffer, 0, byteBuffer)

    runBlocking {
      val unusedBufferMapReturn = buffer.mapAndAwait(MapMode.Read, offset = 0, size = bufferSize)
    }

    val readByteBuffer =
      ByteBuffer.allocateDirect(bufferSize.toInt()).order(ByteOrder.nativeOrder())
    // The testcase will fail in case readMappedRange throws WebGpuException
    buffer.readMappedRange(0, readByteBuffer)
    val readByteBufferFloat = readByteBuffer.asFloatBuffer()
    val readData = FloatArray(readByteBufferFloat.remaining())

    readByteBufferFloat.get(readData)

    val delta = 0.00001f // Define an acceptable tolerance
    assertArrayEquals(
      "The GPU data does not match the original CPU data.",
      initialData,
      readData,
      delta
    )
  }

  /**
   * Verifies that `writeMappedRange` fails when the buffer is not mapped.
   */
  @SmallTest
  @Test
  fun testWriteMappedRangeFailsWhenNotMapped() {
    val buffer = device.createBuffer(
      GPUBufferDescriptor(size = 16, usage = BufferUsage.MapWrite)
    )
    val byteBuffer = ByteBuffer.allocateDirect(16)

    assertThrows(WebGpuException::class.java) {
      buffer.writeMappedRange(0, byteBuffer)
    }
  }

  @SmallTest
  @Test
  fun getMappedRangeWithDefaultValuesReturnsFullBuffer() {
    val bufferSize = 1024L
    device.createBuffer(
      GPUBufferDescriptor(
        usage = BufferUsage.MapWrite,
        size = bufferSize,
        mappedAtCreation = true
      )
    ).use { buffer ->
      // Call with defaults: offset = 0, size = -1L (WGPU_WHOLE_MAP_SIZE).
      val byteBuffer = buffer.getMappedRange()
      buffer.unmap()

      assertEquals(bufferSize, byteBuffer.capacity().toLong())
    }
  }

  @SmallTest
  @Test
  fun getMappedRangeWithOffsetAndDefaultSizeReturnsPartialBuffer() {
    val bufferSize = 1024L
    val offset = 256L
    device.createBuffer(
      GPUBufferDescriptor(
        usage = BufferUsage.MapWrite,
        size = bufferSize,
        mappedAtCreation = true
      )
    ).use { buffer ->
      // Call with offset and default size: size = -1L (WGPU_WHOLE_MAP_SIZE).
      val byteBuffer = buffer.getMappedRange(offset = offset)
      buffer.unmap()

      val expectedSize = bufferSize - offset
      assertEquals(expectedSize, byteBuffer.capacity().toLong())
    }
  }

  @SmallTest
  @Test
  fun getMappedRangeWithOffsetAndExplicitSizeReturnsPartialBuffer() {
    val bufferSize = 1024L
    val offset = 256L
    val size = 512L
    device.createBuffer(
      GPUBufferDescriptor(
        usage = BufferUsage.MapWrite,
        size = bufferSize,
        mappedAtCreation = true
      )
    ).use { buffer ->
      val byteBuffer = buffer.getMappedRange(offset = offset, size = size)
      buffer.unmap()

      assertEquals(size, byteBuffer.capacity().toLong())
    }
  }

  @SmallTest
  @Test
  fun getConstMappedRangeWithDefaultValuesReturnsFullBuffer() {
    val bufferSize = 1024L
    device.createBuffer(
      GPUBufferDescriptor(
        usage = BufferUsage.MapRead,
        size = bufferSize,
        mappedAtCreation = true
      )
    ).use { buffer ->
      val byteBuffer = buffer.getConstMappedRange()
      buffer.unmap()

      assertEquals(bufferSize, byteBuffer.capacity().toLong())
    }
  }

  @SmallTest
  @Test
  fun getConstMappedRangeWithOffsetAndDefaultSizeReturnsPartialBuffer() {
    val bufferSize = 1024L
    val offset = 128L
    device.createBuffer(
      GPUBufferDescriptor(
        usage = BufferUsage.MapRead,
        size = bufferSize,
        mappedAtCreation = true
      )
    ).use { buffer ->
      // Call with offset and default size.
      val byteBuffer = buffer.getConstMappedRange(offset = offset)
      buffer.unmap()

      val expectedSize = bufferSize - offset
      assertEquals(expectedSize, byteBuffer.capacity().toLong())
    }
  }
}