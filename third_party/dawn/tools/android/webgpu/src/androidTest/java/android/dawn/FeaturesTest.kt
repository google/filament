package android.dawn

import android.dawn.helper.createWebGpu
import androidx.test.ext.junit.runners.AndroidJUnit4
import kotlinx.coroutines.runBlocking
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class FeaturesTest {
    /**
     * Test that the features requested match the features in the adapter are present on the device.
     */
    @Test
    fun featuresTest() {
        val requiredFeatures = arrayOf(FeatureName.TextureCompressionASTC)
        runBlocking {
            val webGpu = createWebGpu(requiredFeatures = requiredFeatures)
            val device = webGpu.device
            val deviceFeatures = device.getFeatures().features
            requiredFeatures.forEach {
                assert(deviceFeatures.contains(it)) { "Requested feature $it available on device" }
            }
        }
    }
}
