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
import androidx.webgpu.helper.createWebGpu
import java.util.concurrent.Executor
import kotlinx.coroutines.runBlocking
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
@SmallTest
class FeaturesTest {
    /**
     * Test that the features requested match the features in the adapter are present on the device.
     */
    @Test
    fun featuresTest() {
        val requiredFeatures = intArrayOf(FeatureName.TextureCompressionASTC)
        runBlocking {
            val webGpu =
                createWebGpu(
                    deviceDescriptor = GPUDeviceDescriptor(
                        requiredFeatures = requiredFeatures,
                        deviceLostCallback = null,
                        deviceLostCallbackExecutor = Executor(Runnable::run),
                        uncapturedErrorCallback = null,
                        uncapturedErrorCallbackExecutor = Executor(Runnable::run)
                    )
                )
            val device = webGpu.device
            val deviceFeatures = device.getFeatures().features
            requiredFeatures.forEach {
                assert(deviceFeatures.contains(it)) { "Requested feature $it available on device" }
            }
        }
    }
}
