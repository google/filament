package android.dawn

import android.dawn.FeatureName
import android.dawn.dawnTestLauncher
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class FeaturesTest {
    /**
     * Test that the features requested match the features in the adapter are present on the device.
     */
    @Test
    fun featuresTest() {
        val requiredFeatures =
            arrayOf(FeatureName.BGRA8UnormStorage, FeatureName.TextureCompressionASTC)
        dawnTestLauncher(requiredFeatures) { device ->
            val deviceFeatures = device.getFeatures().features
            requiredFeatures.forEach {
                assert(deviceFeatures.contains(it)) { "Requested feature $it available on device" }
            }
        }
    }
}
