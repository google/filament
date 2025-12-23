/*
 * Copyright (C) 2017 The Android Open Source Project
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

package com.google.android.filament;

public class Filament {
    static {
        Platform.get();
        System.loadLibrary("filament-jni");
    }

    private Filament() {
    }

    @SuppressWarnings("unused")
    public static void init() {
    }

    /**
     * Checks for exceptions from worker threads and throws them.
     *
     * <p>This method polls for exceptions that occurred on Filament's background worker threads.
     * Worker thread exceptions cannot be thrown immediately because they don't have access to
     * the JNI environment. Instead, they are stored and surfaced when this method is called.</p>
     *
     * <p>This method is automatically called in {@link Renderer#beginFrame} and {@link Engine#destroy}
     * to ensure worker thread exceptions are detected. You can also call it manually at any time
     * to check for errors.</p>
     *
     * <p><b>Recommended usage:</b></p>
     * <pre>
     * // Automatic checking (recommended)
     * renderer.beginFrame(swapChain); // healthCheck() is called internally
     *
     * // Manual checking
     * Filament.healthCheck(); // Throws if worker thread exception occurred
     * </pre>
     *
     * @throws FilamentPanicException if a Filament panic occurred on a worker thread
     * @throws FilamentNativeException if a native C++ exception occurred on a worker thread
     * @see FilamentPanicException
     * @see FilamentNativeException
     */
    public static void healthCheck() {
        nHealthCheck();
    }

    private static native void nHealthCheck();
}
