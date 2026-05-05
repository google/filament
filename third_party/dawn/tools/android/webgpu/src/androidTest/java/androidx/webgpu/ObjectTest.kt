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
import androidx.webgpu.helper.Util
import androidx.webgpu.GPU.createInstance
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
@SmallTest
class ObjectTest {
    init {
        Util  // Hack to force library initialization.
    }

    @Test
    fun sameObjectCompare() {
        val surface = createInstance().createSurface(GPUSurfaceDescriptor(
            surfaceSourceAndroidNativeWindow =
                GPUSurfaceSourceAndroidNativeWindow(0)
        ))

        val texture1 = surface.getCurrentTexture().texture

        val texture2 = surface.getCurrentTexture().texture

        assert(texture1 == texture2) {
            "== matches two Kotlin objects representing the same Dawn object"
        }

        assert(texture1.equals(texture2)) {
            ".equals() matches two Kotlin objects representing the same Dawn object"
        }
    }

    @Test
    fun differentObjectCompare() {
        val instance = createInstance()

        val surfaceDescriptor = GPUSurfaceDescriptor(
            surfaceSourceAndroidNativeWindow =
                GPUSurfaceSourceAndroidNativeWindow(0)
        )

        val surface1 = instance.createSurface(surfaceDescriptor)

        val surface2 = instance.createSurface(surfaceDescriptor)

        assert(!(surface1 == surface2)) {
            "== fails to match two Kotlin objects representing different Dawn objects"
        }

        assert(!surface1.equals(surface2)) {
            ".equals fails to match two Kotlin objects representing different Dawn objects"
        }
    }
}
