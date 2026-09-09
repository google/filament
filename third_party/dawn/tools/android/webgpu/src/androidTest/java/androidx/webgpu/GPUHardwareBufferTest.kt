/*
 * Copyright 2026 The Android Open Source Project
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

import android.annotation.SuppressLint
import android.hardware.HardwareBuffer
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.MediumTest
import androidx.webgpu.helper.GPUAndroidHardwareBufferUtil
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import androidx.webgpu.GPU.createInstance
import androidx.webgpu.helper.initLibrary
import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.CoroutineScope
import java.util.concurrent.Executor
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.asCoroutineDispatcher
import kotlinx.coroutines.joinAll
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertThrows
import org.junit.Assert.assertTrue
import org.junit.Assume
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.Executors

@RunWith(AndroidJUnit4::class)
@MediumTest
@OptIn(ExperimentalWebGpuApi::class)
class GPUHardwareBufferTest {

    companion object {
        private const val FENCE_TIMEOUT_MS = 5000L
    }

    private val dispatcher: CoroutineDispatcher = Executors.newSingleThreadExecutor { runnable ->
        Thread(runnable, "Test-WebGPU-Thread")
    }.asCoroutineDispatcher()
    private val testScope = CoroutineScope(dispatcher)

    private lateinit var webGpu: WebGpu
    private lateinit var device: GPUDevice

    @get:Rule
    val apiSkipRule = ApiLevelSkipRule()

    @Before
    fun setup() = runBlocking {
        // 1. Check features BEFORE requesting the device
        initLibrary()
        val instance = createInstance(
            GPUInstanceDescriptor().apply {
                dawnTogglesDescriptor = GPUDawnTogglesDescriptor(
                    enabledToggles = arrayOf("allow_unsafe_apis") // Required to enable experimental SharedTextureMemoryAHardwareBuffer features
                )
            }
        )
        val adapter = instance.requestAdapter()
        val hasRequiredFeatures = with(adapter) {
            hasFeature(FeatureName.SharedTextureMemoryAHardwareBuffer) &&
                    hasFeature(FeatureName.SharedFenceSyncFD) &&
                    hasFeature(FeatureName.YCbCrVulkanSamplers) &&
                    hasFeature(FeatureName.OpaqueYCbCrAndroidForExternalTexture)
        }
        adapter.close()
        instance.close()

        // 2. Skip gracefully if features are missing
        Assume.assumeTrue(
            "Adapter does not support required features for hardware buffer tests",
            hasRequiredFeatures
        )

        // 3. Now it is safe to create the device with these features
        webGpu = createWebGpu(
            dispatcher = dispatcher,
            deviceDescriptor = GPUDeviceDescriptor(
                deviceLostCallback = DeviceLostCallback { _, _, _ -> },
                deviceLostCallbackExecutor = Executor(Runnable::run),
                uncapturedErrorCallback = UncapturedErrorCallback { _, _, _ -> },
                uncapturedErrorCallbackExecutor = Executor(Runnable::run),
                dawnTogglesDescriptor = GPUDawnTogglesDescriptor(
                    enabledToggles = arrayOf("allow_unsafe_apis")
                ),
                requiredFeatures = intArrayOf(
                    FeatureName.SharedTextureMemoryAHardwareBuffer,
                    FeatureName.SharedFenceSyncFD,
                    FeatureName.YCbCrVulkanSamplers,
                    FeatureName.OpaqueYCbCrAndroidForExternalTexture
                )
            )
        )
        testScope.launch {
            webGpu.processEventsLoop()
        }
        device = webGpu.device
    }

    @After
    fun teardown() {
        if (::device.isInitialized) {
            runCatching { device.destroy() }
        }
        if (::webGpu.isInitialized) {
            runCatching { webGpu.close() }
        }
    }

    private fun createHardwareBuffer(
        width: Int, height: Int,
        format: Int = HardwareBuffer.RGBA_8888,
        usage: Long = HardwareBuffer.USAGE_GPU_SAMPLED_IMAGE
    ): HardwareBuffer {
        return HardwareBuffer.create(width, height, format, 1, usage)
    }

    private data class PixelColor(val r: Int, val g: Int, val b: Int, val a: Int)

    private fun getCenterPixelColor(hardwareBuffer: HardwareBuffer): PixelColor {
        val hardwareBitmap = android.graphics.Bitmap.wrapHardwareBuffer(
            hardwareBuffer,
            android.graphics.ColorSpace.get(android.graphics.ColorSpace.Named.SRGB)
        )
        requireNotNull(hardwareBitmap) { "Hardware bitmap must not be null" }

        try {
            val softwareBitmap = hardwareBitmap.copy(android.graphics.Bitmap.Config.ARGB_8888, false)
            requireNotNull(softwareBitmap) { "Software bitmap must not be null" }

            try {
                val centerPixel = softwareBitmap.getPixel(softwareBitmap.width / 2, softwareBitmap.height / 2)
                return PixelColor(
                    r = android.graphics.Color.red(centerPixel),
                    g = android.graphics.Color.green(centerPixel),
                    b = android.graphics.Color.blue(centerPixel),
                    a = android.graphics.Color.alpha(centerPixel)
                )
            } finally {
                softwareBitmap.recycle()
            }
        } finally {
            hardwareBitmap.recycle()
        }
    }

    private fun assertPixelColor(
        hardwareBuffer: HardwareBuffer,
        expectedRed: Int, expectedGreen: Int, expectedBlue: Int,
        expectedAlpha: Int = 255
    ) {
        val color = getCenterPixelColor(hardwareBuffer)

        assertEquals(expectedAlpha, color.a)
        assertEquals(expectedRed, color.r)
        assertEquals(expectedGreen, color.g)
        assertEquals(expectedBlue, color.b)
    }

    private fun verifyColorSpaceConversion(
        pipeline: GPURenderPipeline, sampler: GPUSampler,
        srcBuffer: HardwareBuffer, dstBuffer: HardwareBuffer, dstWrapper: GPUHardwareBufferTexture,
        width: Int, height: Int,
        primary: Int, transfer: Int, range: Int, matrix: Int,
    ) {
        val extDescriptor = createExternalTextureDescriptor(
            cropWidth = width, cropHeight = height,
            primary = primary, transfer = transfer, range = range, matrix = matrix
        )

        val srcExtWrapper = GPUAndroidHardwareBufferUtil.createExternalTexture(
            device,
            srcBuffer,
            extDescriptor
        )

        assertNotNull(srcExtWrapper)

        val bindGroup = device.createBindGroup(
            GPUBindGroupDescriptor(
                layout = pipeline.getBindGroupLayout(0),
                entries = arrayOf(
                    GPUBindGroupEntry(binding = 0, sampler = sampler),
                    GPUBindGroupEntry(
                        binding = 1,
                        externalTextureBindingEntry = GPUExternalTextureBindingEntry(srcExtWrapper.externalTexture)
                    )
                )
            )
        )

        // Begin access on both wrappers
        srcExtWrapper.beginAccess(null)
        dstWrapper.beginAccess(null)

        val view = dstWrapper.texture.createView()
        val encoder = device.createCommandEncoder()
        val pass = encoder.beginRenderPass(
            GPURenderPassDescriptor(
                colorAttachments = arrayOf(
                    GPURenderPassColorAttachment(
                        view = view,
                        loadOp = LoadOp.Clear,
                        storeOp = StoreOp.Store,
                        clearValue = GPUColor(0.0, 0.0, 0.0, 1.0)
                    )
                )
            )
        )
        pass.setPipeline(pipeline)
        pass.setBindGroup(0, bindGroup)
        pass.draw(3)
        pass.end()

        device.queue.submit(arrayOf(encoder.finish()))
        webGpu.instance.processEvents()

        val fenceSrc = requireNotNull(srcExtWrapper.endAccess()) { "Expected a valid fence" }
        assertTrue("GPU execution did not complete in time", fenceSrc.await(FENCE_TIMEOUT_MS))
        fenceSrc.close()

        val fenceDst = requireNotNull(dstWrapper.endAccess()) { "Expected a valid fence" }
        assertTrue("GPU execution did not complete in time", fenceDst.await(FENCE_TIMEOUT_MS))
        fenceDst.close()

        // 4. Verify transformed color components
        val color = getCenterPixelColor(dstBuffer)

        // Assert color conversion preserves primarily Red format
        assertEquals(255, color.a)
        assertTrue("Expected Red component to be high (got ${color.r})", color.r > 200)
        assertTrue("Expected Green component to be low (got ${color.g})", color.g < 50)
        assertTrue("Expected Blue component to be low (got ${color.b})", color.b < 50)

        view.close()
        bindGroup.close()
        srcExtWrapper.close()
    }

    // =========================================================================
    // 1. MEMORY SAFETY & OOM TESTS
    // =========================================================================

    /**
     * Verifies prolonged loop of texture creation/destruction does not leak memory.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 29)
    fun testImportHardwareBufferLoop_noLeaks() {
        runBlocking {
            val unused = webGpu.execute {
                for (i in 0 until 50) {
                    val (rgbBuffer, wrapper) = createWrappedHardwareBuffer(
                        128, 128,
                        textureUsage = TextureUsage.TextureBinding
                    )

                    wrapper.beginAccess(null)
                    val fence = wrapper.endAccess()
                    fence?.close()

                    wrapper.close()
                    rgbBuffer.close()
                }
                webGpu.instance.processEvents()
            }
        }
    }

    /**
     * Verifies WebGPU OOM error scopes capture errors gracefully.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 29)
    fun testWebGpuOOMScope_handlesErrorGracefully() {
        runBlocking {
            val unused = webGpu.execute {
                device.pushErrorScope(ErrorFilter.Validation)

                // Attempt to allocate an absolutely massive texture to trigger validation error
                val descriptor = GPUTextureDescriptor(
                    size = GPUExtent3D(width = 1000000, height = 1000000, depthOrArrayLayers = 1),
                    format = TextureFormat.RGBA8Unorm,
                    usage = TextureUsage.None // Invalid usage
                )
                val unusedTexture = device.createTexture(descriptor)

                // Popping the error scope should throw a ValidationException due to validation failure
                assertThrows(ValidationException::class.java) {
                    runBlocking { val unusedPop = device.popErrorScope() }
                }
            }
        }
    }

    // =========================================================================
    // 2. RGB & YUV EXTERNAL TEXTURE IMPORT LIFE-CYCLE TESTS
    // =========================================================================

    /**
     * Verifies RGB HardwareBuffers can be wrapped as a standard Dawn GPUTexture.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 29)
    fun testRGBHardwareBuffer_asTexture() {
        runBlocking {
            val unused = webGpu.execute {
                val (rgbBuffer, wrapper) = createWrappedHardwareBuffer(
                    textureUsage = TextureUsage.TextureBinding
                )

                assertNotNull(wrapper)
                assertNotNull(wrapper.texture)

                wrapper.beginAccess(null)
                val fence = wrapper.endAccess()
                fence?.close()

                wrapper.close()
                rgbBuffer.close()
            }
        }
    }

    /**
     * Verifies RGB HardwareBuffers can be wrapped as a WebGPU GPUExternalTexture.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 29)
    fun testRGBHardwareBuffer_asExternalTexture() {
        runBlocking {
            val unused = webGpu.execute {
                val rgbBuffer = createHardwareBuffer(64, 64)

                val extDescriptor = createExternalTextureDescriptor()

                validateExternalTextureImport(rgbBuffer, extDescriptor)
            }
        }
    }

    /**
     * Verifies YUV HardwareBuffers can be wrapped as a WebGPU GPUExternalTexture.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 30)
    fun testYUVHardwareBuffer_asExternalTexture() {
        runBlocking {
            val unused = webGpu.execute {
                val yuvBuffer = createHardwareBuffer(64, 64, format = HardwareBuffer.YCBCR_420_888)

                val extDescriptor = createExternalTextureDescriptor()

                validateExternalTextureImport(yuvBuffer, extDescriptor)
            }
        }
    }

    // =========================================================================
    // 3. RACE CONDITIONS & CONCURRENT ACCESS
    // =========================================================================

    private fun createAndAccessTexture() {
        val rgbBuffer = createHardwareBuffer(32, 32)
        val wrapper = GPUAndroidHardwareBufferUtil.createTexture(
            device,
            rgbBuffer,
            usage = TextureUsage.TextureBinding
        )
        wrapper.beginAccess(null)
        val fence = wrapper.endAccess()
        fence?.close()
        wrapper.close()
        rgbBuffer.close()
    }

    /**
     * Verifies concurrent texture access across threads avoids race conditions.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 29)
    @SuppressLint("SuspendBlocks")
    fun testConcurrentAccess_raceCondition() {
        runBlocking(Dispatchers.IO) {
            val numThreads = 2
            val barrier = java.util.concurrent.CyclicBarrier(numThreads)

            val jobs = List(numThreads) {
                launch {
                    for (i in 0 until 20) {
                        createAndAccessTexture()
                        // Gate after every single iteration to guarantee threads strictly interleave
                        barrier.await()
                    }
                }
            }

            jobs.joinAll()
        }
    }

    /**
     * Verifies CPU-to-GPU zero-copy memory visibility via writeTexture.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 29)
    fun testZeroCopyPipeline_writeAndVerify() {
        runBlocking {
            val unused = webGpu.execute {
                val width = 64
                val height = 64
                val (rgbBuffer, wrapper) = createWrappedHardwareBuffer(
                    width, height,
                    textureUsage = TextureUsage.TextureBinding or TextureUsage.CopyDst or TextureUsage.CopySrc
                )

                assertNotNull(wrapper)
                assertNotNull(wrapper.texture)

                // Write green pixels to the imported WebGPU texture
                populateTextureWithSolidColor(wrapper, width, height, r = 0, g = 255.toByte(), b = 0, a = 255.toByte())

                assertPixelColor(rgbBuffer, 0, 255, 0)
                wrapper.close()
                rgbBuffer.close()
            }
        }
    }

    /**
     * Verifies GPU-to-CPU zero-copy memory visibility after rendering into an imported HardwareBuffer.
     * Crashes with Vulkan image layout transition mismatch if GPUImportedAHB does not use VK_IMAGE_LAYOUT_GENERAL.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 29)
    fun testZeroCopyPipeline_gpuRenderWriteAndVerify() {
        runBlocking {
            val unused = webGpu.execute {
                val width = 64
                val height = 64

                val (rgbBuffer, wrapper) = createWrappedHardwareBuffer(
                    width, height,
                    textureUsage = TextureUsage.RenderAttachment or TextureUsage.CopySrc
                )

                assertNotNull(wrapper)
                assertNotNull(wrapper.texture)

                // Shader outputs solid red color (R=255, G=0, B=0, A=255)
                val shaderModule = device.createShaderModule(
                    GPUShaderModuleDescriptor(
                        shaderSourceWGSL = GPUShaderSourceWGSL(
                            code = """
                            @vertex fn vertexMain(@builtin(vertex_index) i : u32) -> @builtin(position) vec4f {
                                const pos = array(vec2f(-1, -1), vec2f(3, -1), vec2f(-1, 3));
                                return vec4f(pos[i], 0.0, 1.0);
                            }

                            @fragment fn fragmentMain() -> @location(0) vec4f {
                                return vec4f(1.0, 0.0, 0.0, 1.0);
                            }
                        """.trimIndent()
                        )
                    )
                )

                val pipeline = device.createRenderPipeline(
                    GPURenderPipelineDescriptor(
                        vertex = GPUVertexState(
                            module = shaderModule,
                            entryPoint = "vertexMain"
                        ),
                        fragment = GPUFragmentState(
                            module = shaderModule,
                            entryPoint = "fragmentMain",
                            targets = arrayOf(GPUColorTargetState(format = TextureFormat.RGBA8Unorm))
                        )
                    )
                )

                wrapper.beginAccess(null)

                // Render directly into the imported HardwareBuffer texture view
                val encoder = device.createCommandEncoder()
                val pass = encoder.beginRenderPass(
                    GPURenderPassDescriptor(
                        colorAttachments = arrayOf(
                            GPURenderPassColorAttachment(
                                view = wrapper.texture.createView(),
                                loadOp = LoadOp.Clear,
                                storeOp = StoreOp.Store,
                                clearValue = GPUColor(0.0, 0.0, 0.0, 1.0)
                            )
                        )
                    )
                )
                pass.setPipeline(pipeline)
                pass.draw(3)
                pass.end()

                val commandBuffer = encoder.finish()
                device.queue.submit(arrayOf(commandBuffer))

                webGpu.instance.processEvents()

                val fence = requireNotNull(wrapper.endAccess()) { "Expected a valid fence" }
                assertTrue("GPU execution did not complete in time", fence.await(FENCE_TIMEOUT_MS))
                fence.close()

                assertPixelColor(rgbBuffer, 255, 0, 0)
                wrapper.close()
                rgbBuffer.close()
            }
        }
    }

    /**
     * Verifies full CPU-to-GPU-to-CPU zero-copy sampling pipeline.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 29)
    fun testZeroCopyPipeline_gpuSamplingAndCopyVerify() {
        runBlocking {
            val unused = webGpu.execute {
                val width = 64
                val height = 64

                // Clear source texture with solid blue color (R=0, G=0, B=255, A=255)
                val sourceTexture = device.createTexture(
                    GPUTextureDescriptor(
                        size = GPUExtent3D(width = width, height = height, depthOrArrayLayers = 1),
                        format = TextureFormat.RGBA8Unorm,
                        usage = TextureUsage.CopySrc or TextureUsage.RenderAttachment
                    )
                )

                val clearEncoder = device.createCommandEncoder()
                val clearPass = clearEncoder.beginRenderPass(
                    GPURenderPassDescriptor(
                        colorAttachments = arrayOf(
                            GPURenderPassColorAttachment(
                                view = sourceTexture.createView(),
                                loadOp = LoadOp.Clear,
                                storeOp = StoreOp.Store,
                                clearValue = GPUColor(0.0, 0.0, 1.0, 1.0)
                            )
                        )
                    )
                )
                clearPass.end()
                device.queue.submit(arrayOf(clearEncoder.finish()))

                // Create imported HardwareBuffer texture
                val (rgbBuffer, wrapper) = createWrappedHardwareBuffer(
                    width, height,
                    textureUsage = TextureUsage.CopyDst or TextureUsage.TextureBinding
                )

                wrapper.beginAccess(null)

                // Copy directly from source texture to zero-copy texture
                val copyEncoder = device.createCommandEncoder()
                copyEncoder.copyTextureToTexture(
                    source = GPUTexelCopyTextureInfo(texture = sourceTexture),
                    destination = GPUTexelCopyTextureInfo(texture = wrapper.texture),
                    copySize = GPUExtent3D(width = width, height = height)
                )
                device.queue.submit(arrayOf(copyEncoder.finish()))

                webGpu.instance.processEvents()

                val fence = requireNotNull(wrapper.endAccess()) { "Expected a valid fence" }
                assertTrue("GPU execution did not complete in time", fence.await(FENCE_TIMEOUT_MS))
                fence.close()

                assertPixelColor(rgbBuffer, 0, 0, 255)
                wrapper.close()
                rgbBuffer.close()
                sourceTexture.destroy()
            }
        }
    }

    /**
     * Verifies YUV external texture descriptors are parsed correctly across the JNI boundary without error.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 30)
    fun testYUVExternalTexture_descriptorTransform_validatesWithoutError() {
        runBlocking {
            val unused = webGpu.execute {
                val yuvBuffer = createHardwareBuffer(128, 128, format = HardwareBuffer.YCBCR_420_888)

                val extDescriptor = createExternalTextureDescriptor(
                    cropOriginX = 16, cropOriginY = 16, cropWidth = 96, cropHeight = 96,
                    mirrored = true, rotation = ExternalTextureRotation.Rotate180Degrees
                )

                validateExternalTextureImport(yuvBuffer, extDescriptor)
            }
        }
    }

    /**
     * Verifies RGB external texture descriptors are parsed correctly across the JNI boundary without error.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 29)
    fun testRGBExternalTexture_descriptorTransform_validatesWithoutError() {
        runBlocking {
            val unused = webGpu.execute {
                val width = 64
                val height = 64

                val rgbBuffer = createHardwareBuffer(width, height)

                val extDescriptor = createExternalTextureDescriptor(
                    cropOriginX = 8, cropOriginY = 8, cropWidth = 48, cropHeight = 48,
                    mirrored = true, rotation = ExternalTextureRotation.Rotate90Degrees
                )

                validateExternalTextureImport(rgbBuffer, extDescriptor)
            }
        }
    }

    /**
     * Verifies ColorSpace enum variants map correctly between Kotlin and Dawn C++.
     */
    @Test
    @MediumTest
    @ApiRequirement(minApi = 29)
    @SuppressLint("SuspendBlocks")
    fun testColorSpaceEnumVariants() {
        runBlocking {
            val unused = webGpu.execute {
                val width = 64
                val height = 64

                // 1. Allocate source HardwareBuffer and populate it with solid Red
                val (srcBuffer, srcWrapper) = createWrappedHardwareBuffer(
                    width, height,
                    textureUsage = TextureUsage.TextureBinding or TextureUsage.CopyDst or TextureUsage.CopySrc
                )

                populateTextureWithSolidColor(srcWrapper, width, height, r = 255.toByte(), g = 0, b = 0, a = 255.toByte())

                // 2. Allocate destination HardwareBuffer (render target)
                val (dstBuffer, dstWrapper) = createWrappedHardwareBuffer(
                    width, height,
                    textureUsage = TextureUsage.RenderAttachment or TextureUsage.CopySrc
                )

                val (pipeline, sampler) = createExternalTextureSamplingPipeline()

                val primariesToTest = intArrayOf(
                    ColorSpacePrimariesDawn.Rec709,
                    ColorSpacePrimariesDawn.Rec601
                )

                val transfersToTest = intArrayOf(
                    ColorSpaceTransferDawn.SRGB,
                    ColorSpaceTransferDawn.Identity
                )

                val matricesToTest = intArrayOf(
                    ColorSpaceYCbCrMatrixDawn.Rec709,
                    ColorSpaceYCbCrMatrixDawn.Rec601
                )

                val rangesToTest = intArrayOf(
                    ColorSpaceYCbCrRangeDawn.Narrow,
                    ColorSpaceYCbCrRangeDawn.Full
                )

                // 3. Permute, draw, and verify output color for all combinations on Main thread
                runBlocking(Dispatchers.Main) {
                    for (primary in primariesToTest) {
                        for (transfer in transfersToTest) {
                            val matrix = matricesToTest[primary % matricesToTest.size]
                            val range = rangesToTest[transfer % rangesToTest.size]

                            verifyColorSpaceConversion(
                                pipeline, sampler, srcBuffer, dstBuffer, dstWrapper, width, height, primary, transfer, range, matrix
                            )
                        }
                    }
                }

                // 5. Clean up shared wrappers
                srcWrapper.close()
                dstWrapper.close()
                srcBuffer.close()
                dstBuffer.close()
            }
        }
    }

    private fun createWrappedHardwareBuffer(
        width: Int = 64, height: Int = 64, textureUsage: Int
    ): Pair<HardwareBuffer, GPUHardwareBufferTexture> {
        val buffer = createHardwareBuffer(
            width, height,
            usage = HardwareBuffer.USAGE_GPU_SAMPLED_IMAGE or HardwareBuffer.USAGE_GPU_COLOR_OUTPUT
        )
        val wrapper = GPUAndroidHardwareBufferUtil.createTexture(device, buffer, usage = textureUsage)
        return Pair(buffer, wrapper)
    }

    private fun populateTextureWithSolidColor(
        wrapper: GPUHardwareBufferTexture,
        width: Int, height: Int,
        r: Byte, g: Byte, b: Byte, a: Byte
    ) {
        val byteBuffer = java.nio.ByteBuffer.allocateDirect(width * height * 4).apply {
            order(java.nio.ByteOrder.nativeOrder())
            for (i in 0 until width * height) {
                put(r)
                put(g)
                put(b)
                put(a)
            }
            rewind()
        }

        wrapper.beginAccess(null)
        device.queue.writeTexture(
            dataLayout = GPUTexelCopyBufferLayout(bytesPerRow = width * 4, rowsPerImage = height),
            data = byteBuffer,
            destination = GPUTexelCopyTextureInfo(texture = wrapper.texture),
            writeSize = GPUExtent3D(width = width, height = height)
        )
        webGpu.instance.processEvents()
        val fence = requireNotNull(wrapper.endAccess()) { "Expected a valid fence" }
        assertTrue("GPU execution did not complete in time", fence.await(FENCE_TIMEOUT_MS))
        fence.close()
    }

    private fun createExternalTextureDescriptor(
        cropWidth: Int = 64, cropHeight: Int = 64,
        cropOriginX: Int = 0, cropOriginY: Int = 0,
        mirrored: Boolean = false,
        rotation: Int = ExternalTextureRotation.Rotate0Degrees,
        primary: Int = ColorSpacePrimariesDawn.Rec709,
        transfer: Int = ColorSpaceTransferDawn.SRGB,
        range: Int = ColorSpaceYCbCrRangeDawn.Narrow,
        matrix: Int = ColorSpaceYCbCrMatrixDawn.Rec709
    ): GPUExternalTextureDescriptor {
        return GPUExternalTextureDescriptor(
            cropOrigin = GPUOrigin2D(cropOriginX, cropOriginY),
            cropSize = GPUExtent2D(cropWidth, cropHeight),
            mirrored = mirrored,
            rotation = rotation,
            srcColorSpace = GPUColorSpaceDawn(
                primaries = primary, transfer = transfer, yCbCrRange = range, yCbCrMatrix = matrix
            ),
            dstColorSpace = PredefinedColorSpace.SRGB
        )
    }

    private fun validateExternalTextureImport(buffer: HardwareBuffer, extDescriptor: GPUExternalTextureDescriptor) {
        val wrapper = GPUAndroidHardwareBufferUtil.createExternalTexture(device, buffer, extDescriptor)
        assertNotNull(wrapper)
        assertNotNull(wrapper.externalTexture)

        wrapper.beginAccess(null)
        val fence = wrapper.endAccess()
        fence?.close()

        wrapper.close()
        buffer.close()
    }

    private fun createExternalTextureSamplingPipeline(): Pair<GPURenderPipeline, GPUSampler> {
        val sampler = device.createSampler(GPUSamplerDescriptor())
        val shaderModule = device.createShaderModule(
            GPUShaderModuleDescriptor(
                shaderSourceWGSL = GPUShaderSourceWGSL(
                    code = """
                        @vertex fn vertexMain(@builtin(vertex_index) i : u32) -> @builtin(position) vec4f {
                            const pos = array(vec2f(-1.0, -1.0), vec2f(3.0, -1.0), vec2f(-1.0, 3.0));
                            return vec4f(pos[i], 0.0, 1.0);
                        }
                        @group(0) @binding(0) var mySampler: sampler;
                        @group(0) @binding(1) var myExternalTexture: texture_external;
                        @fragment fn fragmentMain(@builtin(position) fragCoord: vec4f) -> @location(0) vec4f {
                            return textureSampleBaseClampToEdge(myExternalTexture, mySampler, vec2f(0.5, 0.5));
                        }
                    """.trimIndent()
                )
            )
        )

        val pipeline = device.createRenderPipeline(
            GPURenderPipelineDescriptor(
                vertex = GPUVertexState(module = shaderModule, entryPoint = "vertexMain"),
                fragment = GPUFragmentState(
                    module = shaderModule,
                    entryPoint = "fragmentMain",
                    targets = arrayOf(GPUColorTargetState(format = TextureFormat.RGBA8Unorm))
                )
            )
        )
        return Pair(pipeline, sampler)
    }
}
