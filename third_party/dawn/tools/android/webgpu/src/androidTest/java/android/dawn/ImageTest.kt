package android.dawn

import android.dawn.helper.asString
import android.dawn.helper.createBitmap
import android.dawn.helper.createWebGpu
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import androidx.test.services.storage.TestStorage
import junit.framework.TestCase.assertEquals
import kotlinx.coroutines.runBlocking
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class ImageTest {
    @Test
    fun imageCompareGreen() {
        triangleTest(Color(0.2, 0.9, 0.1, 1.0), "green.png")
    }

    @Test
    fun imageCompareRed() {
        triangleTest(Color(0.9, 0.1, 0.2, 1.0), "red.png")
    }

    private fun triangleTest(color: Color, imageName: String) {
        runBlocking {
            val webGpu = createWebGpu()
            val device = webGpu.device
            val appContext = InstrumentationRegistry.getInstrumentation().targetContext

            val shaderModule = device.createShaderModule(
                ShaderModuleDescriptor(
                    shaderSourceWGSL = ShaderSourceWGSL(
                        code = appContext.assets.open("triangle/shader.wgsl").asString()
                    )
                )
            )

            val testTexture = device.createTexture(
                TextureDescriptor(
                    size = Extent3D(256, 256),
                    format = TextureFormat.RGBA8Unorm,
                    usage = TextureUsage.CopySrc or TextureUsage.RenderAttachment
                )
            )

            with(device.queue) {
                submit(device.createCommandEncoder().use {
                    with(
                        it.beginRenderPass(
                            RenderPassDescriptor(
                                colorAttachments = arrayOf(
                                    RenderPassColorAttachment(
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
                                RenderPipelineDescriptor(
                                    vertex = VertexState(module = shaderModule),
                                    primitive = PrimitiveState(
                                        topology = PrimitiveTopology.TriangleList
                                    ),
                                    fragment = FragmentState(
                                        module = shaderModule,
                                        targets = arrayOf(
                                            ColorTargetState(
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
            TestStorage(appContext.contentResolver).openOutputFile("generated_image.png").use {
                bitmap.compress(Bitmap.CompressFormat.PNG, 100, it)
            }

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
