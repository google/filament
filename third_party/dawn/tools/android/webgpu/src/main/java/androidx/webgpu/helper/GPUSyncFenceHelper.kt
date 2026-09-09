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

import android.hardware.SyncFence
import android.opengl.EGL14
import android.opengl.EGLConfig
import android.opengl.EGLContext
import android.opengl.EGLDisplay
import android.opengl.EGLExt
import android.opengl.EGLSurface
import android.opengl.EGL15
import android.opengl.EGLSync
import android.os.Build
import android.os.ParcelFileDescriptor
import androidx.annotation.RequiresApi
import androidx.webgpu.ExperimentalWebGpuApi
import androidx.webgpu.GPUSyncFence

/**
 * A thread-safe helper object to convert raw file descriptors to Android `SyncFence`
 * objects using EGL.
 *
 * This object maintains a single persistent EGL context and pbuffer surface to avoid
 * the overhead of creating and destroying them on every conversion.
 * It also ensures that the calling thread's existing EGL state is preserved.
 */
@RequiresApi(Build.VERSION_CODES.TIRAMISU)
internal object GPUSyncFenceHelper {

    private var eglDisplay: EGLDisplay = EGL14.EGL_NO_DISPLAY
    private var eglContext: EGLContext = EGL14.EGL_NO_CONTEXT
    private var eglSurface: EGLSurface = EGL14.EGL_NO_SURFACE

    private val lock = Any()
    private var isSupported: Boolean? = null

    /**
     * Ensures that a dedicated EGL context and a pbuffer surface are initialized.
     * This ensures that a current EGL context exists on the calling thread,
     * which is required by some drivers for EGLSync operations.
     */
    private fun ensureEGL() {
        if (eglDisplay != EGL14.EGL_NO_DISPLAY) return
        // 1. Get the default EGL display
        eglDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY)
        check(eglDisplay != EGL14.EGL_NO_DISPLAY) { "Failed to get EGL display" }

        // 2. Initialize EGL
        val version = IntArray(2)
        check(EGL14.eglInitialize(eglDisplay, version, 0, version, 1)) {
            "Failed to initialize EGL"
        }

        // 3. Choose a configuration that supports Pbuffer and OpenGL ES2
        val configAttribs = intArrayOf(
            EGL14.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT,
            EGL14.EGL_SURFACE_TYPE, EGL14.EGL_PBUFFER_BIT,
            EGL14.EGL_NONE
        )
        val configs = arrayOfNulls<EGLConfig>(1)
        val numConfigs = IntArray(1)
        check(EGL14.eglChooseConfig(eglDisplay, configAttribs, 0, configs, 0, 1, numConfigs, 0)) {
            "Failed to choose EGL config"
        }
        check(numConfigs[0] > 0) { "No EGL config found" }

        // 4. Create an EGL context
        val contextAttribs = intArrayOf(
            EGL14.EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL14.EGL_NONE
        )
        eglContext = EGL14.eglCreateContext(
            eglDisplay, configs[0], EGL14.EGL_NO_CONTEXT, contextAttribs, 0
        )
        check(eglContext != EGL14.EGL_NO_CONTEXT) { "Failed to create EGL context" }

        // 5. Create a 1x1 Pbuffer surface (needed to make the context current)
        val pbufferAttribs = intArrayOf(
            EGL14.EGL_WIDTH, 1,
            EGL14.EGL_HEIGHT, 1,
            EGL14.EGL_NONE
        )
        eglSurface = EGL14.eglCreatePbufferSurface(eglDisplay, configs[0], pbufferAttribs, 0)
        check(eglSurface != EGL14.EGL_NO_SURFACE) { "Failed to create EGL pbuffer surface" }
    }

    /**
     * Checks if the EGL_ANDROID_native_fence_sync extension is supported.
     */
    internal fun isNativeFenceSyncSupported(): Boolean {
        synchronized(lock) {
            isSupported?.let { return it }

            ensureEGL()
            if (eglDisplay == EGL14.EGL_NO_DISPLAY) {
                isSupported = false
                return false
            }
            val extensions = EGL14.eglQueryString(eglDisplay, EGL14.EGL_EXTENSIONS)
            isSupported = extensions?.contains("EGL_ANDROID_native_fence_sync") == true
            return isSupported!!
        }
    }

    /**
     * Creates an Android SyncFence from a raw file descriptor using EGL.
     *
     * **Ownership Transfer Rules:**
     * 1. The caller transfers ownership of the raw file descriptor (`syncFd`) to this function.
     * 2. If the conversion fails at any early-exit check, this function manually closes `syncFd`
     *    to prevent file descriptor exhaustion leaks.
     * 3. Once `EGL15.eglCreateSync` is successfully called, the underlying EGL driver takes absolute
     *    ownership of the file descriptor and guarantees its closure upon sync object destruction.
     *
     * @param syncFd The raw file descriptor representing a synchronization fence.
     * @return A valid [SyncFence] duplicated from the EGL sync object, or null if unsupported/failed.
     */
    @OptIn(ExperimentalWebGpuApi::class)
    internal fun createSyncFenceFromFd(pfd: ParcelFileDescriptor): SyncFence? {
        // Ensures all EGL context operations and driver calls are perfectly synchronized across threads.
        synchronized(lock) {
            if (!isNativeFenceSyncSupported()) {
                // EGL extension is missing. Wait for the fence to signal before closing to prevent visual artifacts.
                GPUSyncFence.fromParcelFileDescriptor(pfd).use { val unused = it.await(SYNC_FENCE_TIMEOUT_MS) }
                return null
            }

            // Preserve the calling thread's existing EGL context state to ensure rendering continuity.
            val oldDisplay = EGL14.eglGetCurrentDisplay()
            val oldContext = EGL14.eglGetCurrentContext()
            val oldDrawSurface = EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW)
            val oldReadSurface = EGL14.eglGetCurrentSurface(EGL14.EGL_READ)

            try {
                // Bind our warm helper context and Pbuffer to the calling thread.
                check(EGL14.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
                    "Failed to make EGL context current"
                }
            } catch (e: Exception) {
                // Binding failed. Close PFD directly.
                pfd.close()
                throw e
            }

            try {
                // Detach ownership of the file descriptor from Java, as EGL is about to assume ownership.
                val syncFd = pfd.detachFd()
                if (syncFd < 0) return null

                // 1. Prepare the attributes to import the native fence fd into an EGLSync object.
                val attributes = longArrayOf(
                    EGL_SYNC_NATIVE_FENCE_FD_ANDROID.toLong(), syncFd.toLong(),
                    EGL14.EGL_NONE.toLong()
                )

                // 2. Create the EGLSync object. EGL driver now assumes ownership of the raw fd.
                val eglSync: EGLSync? = EGL15.eglCreateSync(
                    eglDisplay,
                    EGL_SYNC_NATIVE_FENCE_ANDROID,
                    attributes,
                    0
                )

                if (eglSync == null || eglSync == EGL15.EGL_NO_SYNC) {
                    // EGL failed to take ownership. We must adopt and close since it was already detached.
                    ParcelFileDescriptor.adoptFd(syncFd).close()
                    return null
                }

                try {
                    // 3. Safely duplicate the EGL native fence into a platform SyncFence.
                    var fence = EGLExt.eglDupNativeFenceFDANDROID(eglDisplay, eglSync)
                    if (!fence.isValid) {
                        // Workaround for driver bugs (b/18052459): glFlush and try again
                        android.opengl.GLES20.glFlush()
                        fence = EGLExt.eglDupNativeFenceFDANDROID(eglDisplay, eglSync)
                    }
                    return fence
                } finally {
                    // 4. Destroy the temporary EGLSync object immediately after duplication.
                    EGL15.eglDestroySync(eglDisplay, eglSync)
                }
            } finally {
                // Guaranteed Restoration: Reset the calling thread to its original EGL display and context.
                if (oldDisplay != EGL14.EGL_NO_DISPLAY) {
                    EGL14.eglMakeCurrent(oldDisplay, oldDrawSurface, oldReadSurface, oldContext)
                } else {
                    EGL14.eglMakeCurrent(eglDisplay, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_CONTEXT)
                }
            }
        }
    }


    private const val EGL_SYNC_NATIVE_FENCE_ANDROID = 0x3144
    private const val EGL_SYNC_NATIVE_FENCE_FD_ANDROID = 0x3145
    private const val SYNC_FENCE_TIMEOUT_MS = 3000L
}
