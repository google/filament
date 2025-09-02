package android.dawn
import android.dawn.helper.createWebGpu
import androidx.test.ext.junit.runners.AndroidJUnit4
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class AsyncHelperTest {
    @Test
    fun asyncMethodTest() {
        runBlocking {
            val webGpu = createWebGpu()
            val device = webGpu.device
            /* Set up a shader module to support the async call. */
            val shaderModule = device.createShaderModule(
                ShaderModuleDescriptor(shaderSourceWGSL = ShaderSourceWGSL(""))
            )

            /* Call an asynchronous method, converted from a callback pattern by a helper. */
            val result = device.createRenderPipelineAsync(
                RenderPipelineDescriptor(vertex = VertexState(module = shaderModule))
            )

            assert(result.status == CreatePipelineAsyncStatus.ValidationError) {
                """Create render pipeline (async) should fail when no shader entry point exists.
                   The result was: ${result.status}"""
            }
        }
    }

    @Test
    fun asyncMethodTestValidationPasses() {
        runBlocking {
            val webGpu = createWebGpu()
            val device = webGpu.device
            /* Set up a shader module to support the async call. */
            val shaderModule = device.createShaderModule(
                ShaderModuleDescriptor(
                    shaderSourceWGSL = ShaderSourceWGSL(
                        """
@vertex fn vertexMain(@builtin(vertex_index) i : u32) ->
@builtin(position) vec4f {
    return vec4f();
}
@fragment fn fragmentMain() -> @location(0) vec4f {
    return vec4f();
}
                        """
                    )
                )
            )

            /* Call an asynchronous method, converted from a callback pattern by a helper. */
            val result = device.createRenderPipelineAsync(
                RenderPipelineDescriptor(
                    vertex = VertexState(module = shaderModule),
                    fragment = FragmentState(
                        module = shaderModule,
                        targets = arrayOf(ColorTargetState(format = TextureFormat.RGBA8Unorm))
                    )
                )
            )

          /* Create render pipeline (async) should pass with a simple shader.. */
          assertEquals(result.status, CreatePipelineAsyncStatus.Success)

        }
    }
}
