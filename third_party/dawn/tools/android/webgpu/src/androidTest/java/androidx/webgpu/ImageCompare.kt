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

import android.graphics.Bitmap
import kotlin.math.pow
import kotlin.math.sqrt

fun Int.floatFrom(pos: Int) = (this shr 8 * pos and 255).toFloat() / 255

val Int.blue get() = floatFrom(0)
val Int.green get() = floatFrom(1)
val Int.red get() = floatFrom(2)
val Int.alpha get() = floatFrom(3)

fun imageSimilarity(a: Bitmap, b: Bitmap): Float {
    if (a.width != b.width || a.height != b.height) {
        return 0f
    }

    var sumSimilarity = 0f

    for (y in 0 until a.height) {
        for (x in 0 until a.width) {
            val ac = a.getPixel(x, y)
            val bc = b.getPixel(x, y)
            sumSimilarity += (1 - sqrt(
                (ac.red - bc.red).pow(2.0f) +
                        (ac.green - bc.green).pow(2.0f) +
                        (ac.blue - bc.blue).pow(2.0f) +
                        (ac.alpha - bc.alpha).pow(2.0f)
            ) / 2)
        }
    }

    return sumSimilarity / (a.width * a.height)
}
