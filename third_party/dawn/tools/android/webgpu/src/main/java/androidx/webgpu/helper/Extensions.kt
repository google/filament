/*
 * Copyright 2026 The Android Open Source Project
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
package androidx.webgpu.helper

import android.graphics.Color
import android.os.Build
import androidx.annotation.RequiresApi
import androidx.webgpu.GPUColor

/**
 * Converts an Android color integer to a [GPUColor].
 * @return The [GPUColor] representation of the integer color.
 */
public fun Int.toGPUColor(): GPUColor {
    val a = Color.alpha(this) / 255.0
    val r = Color.red(this) / 255.0
    val g = Color.green(this) / 255.0
    val b = Color.blue(this) / 255.0

    return GPUColor(r, g, b, a)
}

/**
 * Converts an Android color long to a [GPUColor].
 * @return The [GPUColor] representation of the long color.
 */
@RequiresApi(Build.VERSION_CODES.O)
public fun Long.toGPUColor(): GPUColor {
    val r = Color.red(this).toDouble()
    val g = Color.green(this).toDouble()
    val b = Color.blue(this).toDouble()
    val a = Color.alpha(this).toDouble()

    return GPUColor(r, g, b, a)
}