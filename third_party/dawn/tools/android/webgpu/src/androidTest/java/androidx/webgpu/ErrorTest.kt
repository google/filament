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
import androidx.webgpu.ValidationException
import androidx.webgpu.helper.createWebGpu
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertThrows
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
@SmallTest
class ErrorTest {
    @Test
    /**
     * Test that an invalid parameter raises an error that is converted to a Kotlin exception by
     * the adapter in DawnTestLauncher.
     */
    fun errorTest() {
        runBlocking {
            val webGpu = createWebGpu()
            val device = webGpu.device
            assertThrows(ValidationException::class.java) {
                device.createTexture(
                    GPUTextureDescriptor(
                        usage = TextureUsage.None,
                        size = GPUExtent3D(0)
                    )
                )
            }
        }
    }
}
