/*
 * Copyright 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package androidx.webgpu

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.MediumTest
import androidx.webgpu.WebGpuTestConstants.EMULATOR_TESTS_MIN_API_LEVEL
import androidx.webgpu.helper.createWebGpu
import androidx.webgpu.helper.WebGpu
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
@MediumTest
class MultisampleStateTest {
    companion object {
        private const val MSAA_COUNT = 4
        private const val TEXTURE_FORMAT = TextureFormat.RGBA8Unorm
        private const val WIDTH = 4
        private const val HEIGHT = 4
        private const val BYTES_PER_ROW = 256
    }

    private lateinit var device: GPUDevice
    private lateinit var webGpu: WebGpu
    @get:Rule val apiSkipRule = ApiLevelSkipRule()

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
     * Helper function to run the MSAA test with a specific sample mask.
     * returns the Red channel value of the first pixel (0-255).
     */
    private fun executeMsaaTest(multiSampleState: GPUMultisampleState): Int {
        // 1. Create Textures
        val msaaTexture = device.createTexture(
            GPUTextureDescriptor(
                size = GPUExtent3D(WIDTH, HEIGHT, 1),
                format = TEXTURE_FORMAT,
                usage = TextureUsage.RenderAttachment,
                sampleCount = MSAA_COUNT
            )
        )

        val resolveTexture = device.createTexture(
            GPUTextureDescriptor(
                size = GPUExtent3D(WIDTH, HEIGHT, 1),
                format = TEXTURE_FORMAT,
                usage = TextureUsage.RenderAttachment or TextureUsage.CopySrc,
                sampleCount = 1
            )
        )

        // 2. Create Output Buffer
        val outputBufferSize = (HEIGHT * BYTES_PER_ROW).toLong()
        val outputBuffer = device.createBuffer(
            GPUBufferDescriptor(
                size = outputBufferSize,
                usage = BufferUsage.CopyDst or BufferUsage.MapRead
            )
        )

        // 3. Create Pipeline with the specific Mask
        val shaderModule = device.createShaderModule(
            GPUShaderModuleDescriptor(
                shaderSourceWGSL = GPUShaderSourceWGSL(
                    code = """
                        @vertex fn vs(@builtin(vertex_index) vi: u32) -> @builtin(position) vec4f {
                            var pos = array<vec2f, 3>(
                                vec2f(-1.0, -1.0), vec2f(3.0, -1.0), vec2f(-1.0, 3.0)
                            );
                            return vec4f(pos[vi], 0.0, 1.0);
                        }
                        @fragment fn fs() -> @location(0) vec4f {
                            return vec4f(1.0, 1.0, 1.0, 1.0); // Output White
                        }
                    """
                )
            )
        )

        val pipeline = device.createRenderPipeline(
            GPURenderPipelineDescriptor(
                vertex = GPUVertexState(module = shaderModule, entryPoint = "vs"),
                fragment = GPUFragmentState(
                    module = shaderModule,
                    entryPoint = "fs",
                    targets = arrayOf(GPUColorTargetState(format = TEXTURE_FORMAT))
                ),
                primitive = GPUPrimitiveState(topology = PrimitiveTopology.TriangleList),
                multisample = multiSampleState
            )
        )

        // 4. Render Pass
        val encoder = device.createCommandEncoder()
        val pass = encoder.beginRenderPass(
            GPURenderPassDescriptor(
                colorAttachments = arrayOf(
                    GPURenderPassColorAttachment(
                        view = msaaTexture.createView(),
                        resolveTarget = resolveTexture.createView(),
                        loadOp = LoadOp.Clear,
                        clearValue = GPUColor(0.0, 0.0, 0.0, 1.0), // Clear to Black
                        storeOp = StoreOp.Discard
                    )
                )
            )
        )
        pass.setPipeline(pipeline)
        pass.draw(3)
        pass.end()

        // 5. Copy & Resolve
        encoder.copyTextureToBuffer(
            source = GPUTexelCopyTextureInfo(texture = resolveTexture),
            destination = GPUTexelCopyBufferInfo(
                buffer = outputBuffer,
                layout = GPUTexelCopyBufferLayout(offset = 0, bytesPerRow = BYTES_PER_ROW)
            ),
            copySize = GPUExtent3D(WIDTH, HEIGHT, 1)
        )

        device.queue.submit(arrayOf(encoder.finish()))

        runBlocking {
            device.queue.onSubmittedWorkDone()
            outputBuffer.mapAndAwait(MapMode.Read, 0, outputBufferSize)
        }

        // 6. Read Result
        val data = outputBuffer.getConstMappedRange()
        val r = data.get(0).toInt() and 0xFF

        outputBuffer.unmap()
        outputBuffer.destroy()
        resolveTexture.destroy()
        msaaTexture.destroy()

        return r
    }

    @Test
    @ApiRequirement(minApi = EMULATOR_TESTS_MIN_API_LEVEL, onlySkipOnEmulator = true)
    fun verifyDefaultMaskEnablesAllSamplesInMSAARender() {
        // Default mask (0xFFFFFFFF or -1) should allow drawing.
        // We draw White on Black background -> Expect White (255).
        val actualValue = executeMsaaTest(
            GPUMultisampleState(
                count = MSAA_COUNT,
            )
        )
        assertEquals("Should be White (255)", 255, actualValue)
    }

    @Test
    fun verifyZeroMaskDisablesUpdates() {
        // Zero mask (0x0) should block all samples from being updated.
        // We draw White on Black background -> Expect Black (0) because the draw was masked out.
        val actualValue = executeMsaaTest(
            GPUMultisampleState(
                count = MSAA_COUNT,
                mask = 0
            )
        )
        assertEquals("Should be Black (0) because mask prevented write", 0, actualValue)
    }
}