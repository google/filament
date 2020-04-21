/*
 * Copyright (C) 2020 The Android Open Source Project
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

package com.google.android.filament.utils

import com.google.android.filament.Filament

object Utils {
    /**
     * Initializes the utils JNI layer. Must be called before using any utils functionality.
     */
    fun init() {
        // Load Filament first to ensure that the NioUtils Java class is available in the JNIEnv.
        Filament.init()
        System.loadLibrary("filament-utils-jni")
    }
}
