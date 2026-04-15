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

import android.graphics.BitmapFactory
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.MediumTest
import androidx.test.platform.app.InstrumentationRegistry
import androidx.webgpu.WebGpuTestConstants.EMULATOR_TESTS_MIN_API_LEVEL
import androidx.webgpu.helper.asString
import androidx.webgpu.helper.createBitmap
import androidx.webgpu.helper.createWebGpu
import junit.framework.TestCase.assertEquals
import kotlinx.coroutines.runBlocking
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class ImageTest {
    private val appContext = InstrumentationRegistry.getInstrumentation().targetContext
    private val storage = StorageFactory.createStore(appContext)
    @get:Rule
    val apiSkipRule = ApiLevelSkipRule()

    @Test
    @MediumTest
    @ApiRequirement(minApi = EMULATOR_TESTS_MIN_API_LEVEL, onlySkipOnEmulator = true)
    fun imageCompareGreen() {
        triangleTest(GPUColor(0.2, 0.9, 0.1, 1.0), "green.png")
    }

    @Test
    @MediumTest
    @ApiRequirement(minApi = EMULATOR_TESTS_MIN_API_LEVEL, onlySkipOnEmulator = true)
    fun imageCompareRed() {
        triangleTest(GPUColor(0.9, 0.1, 0.2, 1.0), "red.png")
    }

    private fun triangleTest(color: GPUColor, imageName: String) {
        runBlocking {
            val webGpu = createWebGpu()
            val device = webGpu.device

            val shaderModule = device.createShaderModule(
                GPUShaderModuleDescriptor(
                    shaderSourceWGSL = GPUShaderSourceWGSL(
                        code = appContext.assets.open("triangle/shader.wgsl").asString()
                    )
                )
            )

            val testTexture = device.createTexture(
                GPUTextureDescriptor(
                    size = GPUExtent3D(256, 256),
                    format = TextureFormat.RGBA8Unorm,
                    usage = TextureUsage.CopySrc or TextureUsage.RenderAttachment
                )
            )

            with(device.queue) {
                submit(device.createCommandEncoder().use {
                    with(
                        it.beginRenderPass(
                            GPURenderPassDescriptor(
                                colorAttachments = arrayOf(
                                    GPURenderPassColorAttachment(
                                        loadOp = LoadOp.Clear,
                                        storeOp = StoreOp.Store,
                                        clearValue = color,
                                        view = testTexture.createView()
                                    )
                                )
                            )
                        )
                    ) {
                        setPipeline(
                            device.createRenderPipeline(
                                GPURenderPipelineDescriptor(
                                    vertex = GPUVertexState(module = shaderModule),
                                    primitive = GPUPrimitiveState(
                                        topology = PrimitiveTopology.TriangleList
                                    ),
                                    fragment = GPUFragmentState(
                                        module = shaderModule,
                                        targets = arrayOf(
                                            GPUColorTargetState(
                                                format = TextureFormat.RGBA8Unorm
                                            )
                                        )
                                    )
                                )
                            )
                        )
                        draw(3)
                        end()
                    }

                    arrayOf(it.finish())
                })
            }

            val bitmap = testTexture.createBitmap(device)

            // Write the generated bitmap to test storage for inspection in the event of test
            // failures.
            storage.writeImage("generated_image.png", bitmap)

            val testAssets = appContext.assets
            val matched = testAssets.list("compare")!!.filter {
                imageSimilarity(
                    bitmap,
                    BitmapFactory.decodeStream(testAssets.open("compare/$it"))
                ) > 0.99
            }

            assertEquals(listOf(imageName), matched)
        }
    }
}
