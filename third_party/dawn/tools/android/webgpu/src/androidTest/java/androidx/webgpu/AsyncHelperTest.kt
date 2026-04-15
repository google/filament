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
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Assume.assumeFalse
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.atomic.AtomicBoolean

@RunWith(AndroidJUnit4::class)
@SmallTest
class AsyncHelperTest {

    private lateinit var webGpu: WebGpu
    private lateinit var device: GPUDevice

    private val BASIC_SHADER = """
        @vertex fn vertexMain(@builtin(vertex_index) i : u32) ->
        @builtin(position) vec4f {
            return vec4f();
        }
        @fragment fn fragmentMain() -> @location(0) vec4f {
            return vec4f();
        } """

    @Before
    fun setup() {
        runBlocking {
            webGpu = createWebGpu()
            device = webGpu.device
        }
    }

    @Test
    fun asyncMethodTest() {
        runBlocking {
            /* Set up a shader module to support the async call. */
            val shaderModule = device.createShaderModule(
                GPUShaderModuleDescriptor(shaderSourceWGSL = GPUShaderSourceWGSL(""))
            )

            val exception = assertThrows(WebGpuException::class.java) {
                runBlocking {
                    /* Call an asynchronous method, converted from a callback pattern by a helper. */
                    device.createRenderPipelineAndAwait(
                        GPURenderPipelineDescriptor(vertex = GPUVertexState(module = shaderModule))
                    )
                }
            }

            assertEquals(
                """Create render pipeline (async) should fail when no shader entry point exists.
                   The result was: ${exception.status}""",
                CreatePipelineAsyncStatus.ValidationError,
                exception.status
            )
        }
    }

    @Test
    fun asyncMethodTestValidationPasses() {
        runBlocking {
            /* Set up a valid shader module and descriptor */
            val shaderModule = device.createShaderModule(
                GPUShaderModuleDescriptor(shaderSourceWGSL = GPUShaderSourceWGSL(BASIC_SHADER))
            )

            /* Call an asynchronous method, converted from a callback pattern by a helper. */
            val unused = device.createRenderPipelineAndAwait(
                GPURenderPipelineDescriptor(
                    vertex = GPUVertexState(module = shaderModule), fragment = GPUFragmentState(
                        module = shaderModule,
                        targets = arrayOf(GPUColorTargetState(format = TextureFormat.RGBA8Unorm))
                    )
                )
            )

          /* Create render pipeline (async) should pass with a simple shader.. */
        }
    }

    private fun baseCancellationTest(doCancel: Boolean): Boolean {
        val hasReturned = AtomicBoolean(false)

        runBlocking {
            val shaderModule = device.createShaderModule(
                GPUShaderModuleDescriptor(shaderSourceWGSL = GPUShaderSourceWGSL(BASIC_SHADER))
            )

            /* Launch the function in a new coroutine, giving us a job handle we can cancel. */
            val job = launch {
                var unused = device.createRenderPipelineAndAwait(
                    GPURenderPipelineDescriptor(vertex = GPUVertexState(module = shaderModule),
                        fragment = GPUFragmentState(
                            module = shaderModule,
                            targets = arrayOf(GPUColorTargetState(format = TextureFormat.RGBA8Unorm))
                        )
                    )
                )
                hasReturned.set(true)
            }
            assumeFalse("The job completed before we could test it", hasReturned.get())

            if (doCancel) {
                job.cancel()
            }
            job.join()
        }
        return hasReturned.get()
    }

    /**
     * Test that the async-based job will complete if it's not cancelled.
     */
    @Test
    fun asyncMethodCancellationTestControl() {
        assertEquals(
            "The async job should have completed but it failed to do so.",
            true,
            baseCancellationTest(false)
        )
    }

    /**
     * Test that the async-based job will not complete if it is cancelled.
     */
    @Test
    fun asyncMethodCancellationTest() {
        assertEquals(
            "The async job should have been cancelled but it completed.",
            false,
            baseCancellationTest(true)
        )
    }
}
