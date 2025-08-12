package android.dawn

import android.dawn.helper.createWebGpu
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertThrows
import org.junit.Test

class BufferTest {
    @Test
    /**
     * Test that calling getMappedRange() on a mapped buffer does not raise an exception.
     */
    fun bufferMapTest() {
        runBlocking {
            val webGpu = createWebGpu()
            val device = webGpu.device
            device.createBuffer(
                BufferDescriptor(
                    usage = BufferUsage.Vertex,
                    size = 1024,
                    mappedAtCreation = true
                )
            ).apply {
                getMappedRange(size = size)
            }
        }
    }

    @Test
    /**
     * Test that calling getMappedRange() on a non-mapped buffer raises an exception.
     */
    fun bufferMapFailureTest() {
        runBlocking {
            val webGpu = createWebGpu()
            val device = webGpu.device
            assertThrows(Error::class.java) {
                device.createBuffer(
                    BufferDescriptor(
                        usage = BufferUsage.Vertex,
                        size = 1024,
                        mappedAtCreation = false
                    )
                ).apply {
                    getMappedRange(size = size)
                }
            }
        }
    }
}
