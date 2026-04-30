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

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.SmallTest
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import junit.framework.TestCase.assertEquals
import androidx.webgpu.ValidationException
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.Assert.assertThrows
import org.junit.Assume
import org.junit.Before

@Suppress("UNUSED_VARIABLE")
@RunWith(AndroidJUnit4::class)
@SmallTest
class DeviceTest {
  private lateinit var device: GPUDevice
  private lateinit var webGpu: WebGpu

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
  @SmallTest
  fun testHasFeature() {
    runBlocking {
      // This test ensures the API is callable.
      Assume.assumeTrue(device.hasFeature(FeatureName.TimestampQuery))
    }
  }

  @Test
  @SmallTest
  fun testErrorScope() {
    device.pushErrorScope(ErrorFilter.Validation)

    // Intentionally create an invalid buffer to trigger a validation error.
    // A buffer size must be a multiple of 4.
    val unusedBuffer = device.createBuffer(
      GPUBufferDescriptor(
        size = 1, usage = BufferUsage.Vertex, mappedAtCreation = true
      )
    )

    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
  }


  @Test
  @SmallTest
  fun testCreateBuffer() {
    val buffer = device.createBuffer(
      GPUBufferDescriptor(
        size = 4, usage = BufferUsage.Vertex
      )
    )
    assertEquals(buffer.usage, BufferUsage.Vertex)
  }


  @Test
  @SmallTest
  fun testCreateTexture() {
    val texture = device.createTexture(
      GPUTextureDescriptor(
        size = GPUExtent3D(1, 1, 1),
        format = TextureFormat.RGBA8Unorm,
        usage = TextureUsage.TextureBinding
      )
    )
    assertEquals(texture.usage, TextureUsage.TextureBinding)
  }

  @Test
  @SmallTest
  fun testCreateComputePipeline_withInvalidEntryPoint_throwsException() {
    val shaderModule = device.createShaderModule(
      GPUShaderModuleDescriptor(
        shaderSourceWGSL = GPUShaderSourceWGSL(
          code = "@compute @workgroup_size(1) fn main() {}"
        )
      )
    )

    assertThrows(ValidationException::class.java) {
      device.createComputePipeline(
        GPUComputePipelineDescriptor(
          compute = GPUComputeState(
            module = shaderModule, entryPoint = "non_existent_entry_point"
          )
        )
      )
    }
  }

  @Test
  @SmallTest
  fun testCreateShaderModule_withInvalidShader_throwsException() {
    // This shader has a syntax error ("fu" instead of "fn")
    val badShaderCode = "@compute @workgroup_size(1) fu main() {}"

    // Creating the shader module itself should fail
    assertThrows(ValidationException::class.java) {
      device.createShaderModule(
        GPUShaderModuleDescriptor(shaderSourceWGSL = GPUShaderSourceWGSL(code = badShaderCode))
      )
    }
  }

  /**
   * Verifies that createRenderPipeline fails validation if the entry point is incorrect.
   */
  @Test
  @SmallTest
  fun testCreateRenderPipeline_withInvalidEntryPoint_failsValidation() {
    val shaderModule = device.createShaderModule(
      GPUShaderModuleDescriptor(
        shaderSourceWGSL = GPUShaderSourceWGSL(
          code = "@vertex fn main() -> @builtin(position) vec4<f32> { return vec4<f32>(0.0); }"
        )
      )
    )

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderPipeline = device.createRenderPipeline(
      GPURenderPipelineDescriptor(
        vertex = GPUVertexState(
          module = shaderModule, entryPoint = "non_existent_entry_point" // Invalid
        )
      )
    )
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
  }

  @Test
  @SmallTest
  fun testCreateBindGroupLayout_withDuplicateBindings_failsValidation() {
    device.pushErrorScope(ErrorFilter.Validation)
    val unusedBindGroupLayout = device.createBindGroupLayout(
      GPUBindGroupLayoutDescriptor(
        entries = arrayOf(
          GPUBindGroupLayoutEntry(
            binding = 0, // Duplicate
            visibility = ShaderStage.Fragment,
            buffer = GPUBufferBindingLayout(type = BufferBindingType.Storage)
          ), GPUBindGroupLayoutEntry(
            binding = 0, // Duplicate
            visibility = ShaderStage.Fragment,
            buffer = GPUBufferBindingLayout(type = BufferBindingType.Storage)
          )
        )
      )
    )
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
  }

  /**
   * Verifies that createBindGroup fails validation if the buffer's usage is incorrect.
   */
  @Test
  @SmallTest
  fun testCreateBindGroup_withMismatchedBufferUsage_failsValidation() {
    val layout = device.createBindGroupLayout(
      GPUBindGroupLayoutDescriptor(
        entries = arrayOf(
          GPUBindGroupLayoutEntry(
            binding = 0,
            visibility = ShaderStage.Compute,
            buffer = GPUBufferBindingLayout(type = BufferBindingType.Uniform)
          )
        )
      )
    )

    // Create a buffer WITHOUT the required `Uniform` usage.
    val buffer = device.createBuffer(
      GPUBufferDescriptor(size = 16, usage = BufferUsage.CopySrc) // Invalid usage
    )

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedBindGroup = device.createBindGroup(
      GPUBindGroupDescriptor(
        layout = layout, entries = arrayOf(
          GPUBindGroupEntry(binding = 0, buffer = buffer)
        )
      )
    )
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
  }

  @Test
  @SmallTest
  fun testCreateQuerySet_withInvalidCount_failsValidation() {
    device.pushErrorScope(ErrorFilter.Validation)
    val unusedQuerySet = device.createQuerySet(
      GPUQuerySetDescriptor(
        type = QueryType.Occlusion, count = -1 // Invalid: count must be > 0.
      )
    )
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
  }

  @Test
  fun validationError_withoutActiveErrorScope_throwsValidationException() {
    val invalidDescriptor = GPUQuerySetDescriptor(
      type = QueryType.Occlusion,
      count = -1 // Invalid parameter
    )
    assertThrows(ValidationException::class.java) {
      device.createQuerySet(invalidDescriptor)
    }
  }

  @Test
  @SmallTest
  fun testCreateSampler_withInvalidLodClamp_failsValidation() {
    runBlocking {
      val invalidDescriptor = GPUSamplerDescriptor(
        lodMinClamp = 10.0f, lodMaxClamp = 1.0f
      )

      device.pushErrorScope(ErrorFilter.Validation)
      val unusedSampler = device.createSampler(invalidDescriptor)

      assertThrows(ValidationException::class.java) {
        runBlocking {
          device.popErrorScope()
        }
      }
    }
  }
}
