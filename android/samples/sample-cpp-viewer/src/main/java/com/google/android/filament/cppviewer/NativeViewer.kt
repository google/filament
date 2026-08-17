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

package com.google.android.filament.cppviewer

import android.view.MotionEvent
import android.view.Surface

/**
 * A Kotlin wrapper class for handling the Android UI and lifecycle events for native Filament applications.
 *
 * This class uses a C++ JNI bridge to map Android lifecycle methods (such as surface creation,
 * resizing, destruction, and touch events) to native FilamentApp2 callbacks.
 */
class NativeViewer(val nativeApp: Long = 0L) {
    companion object {
        init {
            System.loadLibrary("sample-cpp-viewer-jni")
        }
    }

    /**
     * Called when the Android surface is created.
     * Maps to the native FilamentApp2 callback to initialize graphics resources for the surface.
     *
     * @param surface The newly created Android [Surface].
     */
    fun onSurfaceCreated(surface: Surface) {
        nOnSurfaceCreated(nativeApp, surface)
    }

    /**
     * Called when the Android surface is changed (e.g., resized or rotated).
     * Maps to the native FilamentApp2 callback to handle screen dimensions updates.
     *
     * @param width The new width of the surface in pixels.
     * @param height The new height of the surface in pixels.
     */
    fun onSurfaceChanged(width: Int, height: Int) {
        nOnSurfaceChanged(nativeApp, width, height)
    }

    /**
     * Called when the Android surface is destroyed.
     * Maps to the native FilamentApp2 callback to clean up the swap chain and graphics resources.
     */
    fun onSurfaceDestroyed() {
        nOnSurfaceDestroyed(nativeApp)
    }

    /**
     * Passes raw touch event data to the native FilamentApp2.
     *
     * @param action The masked action of the motion event (e.g., ACTION_DOWN, ACTION_MOVE).
     * @param x The X coordinate of the touch event.
     * @param y The Y coordinate of the touch event.
     */
    fun onTouchEvent(action: Int, x: Float, y: Float) {
        nOnTouchEvent(nativeApp, action, x, y)
    }

    /**
     * Advances and renders a single frame.
     * Maps to the native FilamentApp2 callback to execute rendering.
     *
     * @return True if the app requested termination or closed.
     */
    fun doFrame(): Boolean {
        return nDoFrame(nativeApp)
    }

    /**
     * Helper method to process a [MotionEvent] from the Android View system.
     * Extracts the relevant action and coordinates and forwards them to the native implementation.
     *
     * @param event The [MotionEvent] received from the view.
     * @return True to indicate the event was handled.
     */
    fun onTouchEvent(event: MotionEvent): Boolean {
        onTouchEvent(event.actionMasked, event.x, event.y)
        return true
    }

    /**
     * Destroys the native FilamentApp2 instance, waiting for it to safely shut down.
     */
    fun destroy() {
        if (nativeApp != 0L) {
            nDestroy(nativeApp)
        }
    }

    private external fun nOnSurfaceCreated(nativeApp: Long, surface: Surface)
    private external fun nOnSurfaceChanged(nativeApp: Long, width: Int, height: Int)
    private external fun nOnSurfaceDestroyed(nativeApp: Long)
    private external fun nOnTouchEvent(nativeApp: Long, action: Int, x: Float, y: Float)
    private external fun nDoFrame(nativeApp: Long): Boolean
    private external fun nDestroy(nativeApp: Long)
}
