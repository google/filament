package android.dawn

import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertThrows
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class ErrorTest {
    @Test
    /**
     * Test that an invalid parameter raises an error that is converted to a Kotlin exception by
     * the adapter in DawnTestLauncher.
     */
    fun errorTest() {
        dawnTestLauncher { device ->
            assertThrows(UncapturedErrorException::class.java) {
                device.createTexture(
                    TextureDescriptor(
                        usage = TextureUsage.None,
                        size = Extent3D(0),
                        format = TextureFormat.Undefined  // Invalid parameter for createTexture().
                    )
                )
            }
        }
    }
}
