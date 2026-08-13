/*
 * Copyright (C) 2026 The Android Open Source Project
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

import android.view.SurfaceView

/**
 * A Kotlin wrapper class for the native AndroidDisplayManager.
 *
 * This class bridges an Android SurfaceView directly to the C++ DisplayManager, allowing C++
 * to autonomously extract the ANativeWindow buffer using JNI reflection when the OS makes it valid.
 */
class AndroidDisplayManager(val surfaceView: SurfaceView) {
    val nativeDm: Long

    init {
        nativeDm = nCreate(surfaceView)
    }

    /**
     * Cleans up the native C++ AndroidDisplayManager object.
     */
    fun destroy() {
        if (nativeDm != 0L) {
            nDestroy(nativeDm)
        }
    }

    private external fun nCreate(surfaceView: SurfaceView): Long
    private external fun nDestroy(nativeDm: Long)

    companion object {
        init {
            System.loadLibrary("filament-utils-jni")
        }
    }
}
