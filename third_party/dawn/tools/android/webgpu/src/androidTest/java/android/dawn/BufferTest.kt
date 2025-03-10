package android.dawn

import org.junit.Assert.assertThrows
import org.junit.Test

class BufferTest {
    @Test
    /**
     * Test that calling getMappedRange() on a mapped buffer does not raise an exception.
     */
    fun bufferMapTest() {
        dawnTestLauncher() { device ->
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
        dawnTestLauncher() { device ->
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
