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

import android.content.res.AssetManager
import android.os.Handler
import android.os.HandlerThread
import android.os.Looper
import android.os.Process
import android.util.Log
import android.view.Choreographer
import android.view.Surface
import java.util.concurrent.CountDownLatch
import java.util.concurrent.atomic.AtomicInteger

/**
 * Runs the C++ samples on a dedicated render thread.
 *
 * Filament requires an Engine to be used and destroyed on the thread that created it, so the
 * sample is created, rendered, and destroyed on this thread, and every native call except window
 * acquire/release happens here. The public methods are called on the UI thread and only post
 * messages, so the UI stays responsive while a sample loads. Messages run in posting order.
 *
 * Surface handoff: once [surfaceDestroyed] returns, Android may free the surface, so the backend
 * must have stopped using it by then. The two threads share [surfaceLock], which guards the
 * window reference and [windowInUse]. The render thread sets [windowInUse] under the lock before
 * it creates a swapchain, so [surfaceDestroyed] either sees the window unused and releases it
 * immediately (even while a sample is still loading), or sees it in use and waits until the
 * render thread has destroyed the swapchain.
 *
 * @param onFirstFrame Called on the UI thread with the generation returned by [loadSample] once
 * that sample has rendered its first frame.
 * @param onLoadFailed Called on the UI thread with the generation of a sample that failed to load.
 */
class SampleRenderThread(
    private val assets: AssetManager,
    private val onFirstFrame: (generation: Int) -> Unit,
    private val onLoadFailed: (generation: Int) -> Unit,
) {
    companion object {
        private const val TAG = "CppViewer"

        init {
            System.loadLibrary("sample-cpp-viewer-jni")
        }
    }

    private val thread = HandlerThread("FilamentRender", Process.THREAD_PRIORITY_DISPLAY)
        .apply { start() }
    private val handler = Handler(thread.looper)
    private val uiHandler = Handler(Looper.getMainLooper())

    // The generation of the latest loadSample() request. Requests superseded before they start
    // are skipped.
    private val latestGeneration = AtomicInteger(0)

    // ---- Shared between the UI and render threads, guarded by surfaceLock. ----
    private val surfaceLock = Object()
    // ANativeWindow reference acquired in surfaceCreated(), or 0.
    private var window = 0L
    // False once surfaceDestroyed() has run for the surface backing `window`.
    private var surfaceValid = false
    private var surfaceWidth = 0
    private var surfaceHeight = 0
    // True while the render thread may use `window`, i.e., from just before the swapchain is
    // created until it has been destroyed. Written only by the render thread.
    private var windowInUse = false
    // Counted down by the render thread once it stops using an invalidated window.
    private var windowReleased: CountDownLatch? = null

    // ---- Confined to the render thread. ----
    private lateinit var choreographer: Choreographer
    // Native AndroidAssetLoader, or 0.
    private var assetLoader = 0L
    // Native handle owning the running sample and its display manager, or 0.
    private var sample = 0L
    private var generation = 0
    private var firstFrameReported = false
    private var resumed = false
    private var frameScheduled = false
    private val frameCallback = Choreographer.FrameCallback { renderFrame() }

    init {
        handler.post {
            // Choreographer.getInstance() binds to the calling thread's Looper, so frame
            // callbacks are delivered on the render thread.
            choreographer = Choreographer.getInstance()
            assetLoader = nCreateAssetLoader(assets)
        }
    }

    // ---- UI thread API. ----

    /**
     * Replaces the current sample with [name]. Returns the generation identifying this request,
     * which is passed back to [onFirstFrame] and [onLoadFailed].
     */
    fun loadSample(name: String): Int {
        val gen = latestGeneration.incrementAndGet()
        handler.post {
            if (gen != latestGeneration.get()) {
                return@post
            }
            destroySample()
            createSample(name, gen)
        }
        return gen
    }

    fun surfaceCreated(surface: Surface) {
        val nativeWindow = nAcquireNativeWindow(surface)
        synchronized(surfaceLock) {
            // surfaceDestroyed() released the previous window before returning.
            check(window == 0L && !windowInUse)
            window = nativeWindow
            surfaceValid = nativeWindow != 0L
        }
        handler.post { attachSurface() }
    }

    fun surfaceChanged(width: Int, height: Int) {
        synchronized(surfaceLock) {
            surfaceWidth = width
            surfaceHeight = height
        }
        handler.post { resizeSurface() }
    }

    /** Returns once the render thread no longer uses the surface. */
    fun surfaceDestroyed() {
        val released: CountDownLatch
        synchronized(surfaceLock) {
            surfaceValid = false
            if (!windowInUse) {
                releaseWindowLocked()
                return
            }
            released = CountDownLatch(1)
            windowReleased = released
        }
        // If the looper is quitting, this post is dropped, but the pending destroySample() from
        // quit() detaches the window and counts the latch down.
        handler.post { detachSurface() }
        awaitUninterruptibly(released)
    }

    fun touchEvent(action: Int, x: Float, y: Float) {
        handler.post {
            if (sample != 0L) {
                nOnTouchEvent(sample, action, x, y)
            }
        }
    }

    fun resume() {
        handler.post {
            resumed = true
            scheduleFrame()
        }
    }

    fun pause() {
        handler.post {
            resumed = false
            cancelFrame()
        }
    }

    /** Destroys the current sample and stops the render thread, without blocking on either. */
    fun quit() {
        latestGeneration.incrementAndGet()
        handler.post {
            destroySample()
            if (assetLoader != 0L) {
                nDestroyAssetLoader(assetLoader)
                assetLoader = 0L
            }
        }
        thread.quitSafely()
    }

    // ---- Render thread. ----

    private fun createSample(name: String, gen: Int) {
        val handle = nCreateSample(name, assetLoader)
        if (handle == 0L) {
            Log.e(TAG, "Failed to create sample '$name'")
            uiHandler.post { onLoadFailed(gen) }
            return
        }
        sample = handle
        generation = gen
        firstFrameReported = false
        attachSurface()
    }

    private fun destroySample() {
        if (sample == 0L) {
            return
        }
        cancelFrame()
        detachSurface()
        nDestroySample(sample)
        sample = 0L
    }

    private fun attachSurface() {
        if (sample == 0L) {
            return
        }
        val nativeWindow: Long
        val width: Int
        val height: Int
        synchronized(surfaceLock) {
            if (windowInUse || !surfaceValid) {
                return
            }
            // From here on, surfaceDestroyed() waits for detachSurface() before returning, so the
            // window stays valid outside the lock.
            windowInUse = true
            nativeWindow = window
            width = surfaceWidth
            height = surfaceHeight
        }
        nOnSurfaceCreated(sample, nativeWindow)
        nOnSurfaceChanged(sample, width, height)
        scheduleFrame()
    }

    private fun detachSurface() {
        val inUse = synchronized(surfaceLock) { windowInUse }
        if (!inUse) {
            return
        }
        cancelFrame()
        // windowInUse implies that a sample exists. This destroys the swapchain and waits for the
        // backend to finish with the window.
        nOnSurfaceDestroyed(sample)
        synchronized(surfaceLock) {
            windowInUse = false
            if (!surfaceValid) {
                releaseWindowLocked()
                windowReleased?.countDown()
                windowReleased = null
            }
        }
    }

    private fun resizeSurface() {
        if (sample == 0L) {
            return
        }
        val width: Int
        val height: Int
        synchronized(surfaceLock) {
            if (!windowInUse) {
                return
            }
            width = surfaceWidth
            height = surfaceHeight
        }
        nOnSurfaceChanged(sample, width, height)
    }

    private fun scheduleFrame() {
        // windowInUse is written only on this thread, so it can be read without the lock here.
        if (!resumed || frameScheduled || sample == 0L || !windowInUse) {
            return
        }
        choreographer.postFrameCallback(frameCallback)
        frameScheduled = true
    }

    private fun cancelFrame() {
        if (frameScheduled) {
            choreographer.removeFrameCallback(frameCallback)
            frameScheduled = false
        }
    }

    private fun renderFrame() {
        frameScheduled = false
        if (sample == 0L) {
            return
        }
        if (nDoFrame(sample)) {
            Log.i(TAG, "Sample closed itself; stopping the frame loop")
            return
        }
        if (!firstFrameReported) {
            firstFrameReported = true
            val gen = generation
            uiHandler.post { onFirstFrame(gen) }
        }
        scheduleFrame()
    }

    // ---- Helpers. ----

    private fun releaseWindowLocked() {
        if (window != 0L) {
            nReleaseNativeWindow(window)
            window = 0L
        }
    }

    private fun awaitUninterruptibly(latch: CountDownLatch) {
        var interrupted = false
        while (true) {
            try {
                latch.await()
                break
            } catch (e: InterruptedException) {
                interrupted = true
            }
        }
        if (interrupted) {
            Thread.currentThread().interrupt()
        }
    }

    // ---- Native methods, implemented in SampleAppJni.cpp. ----

    // Window acquire/release may be called from any thread. All other calls happen on the render
    // thread.
    private external fun nAcquireNativeWindow(surface: Surface): Long
    private external fun nReleaseNativeWindow(window: Long)

    private external fun nCreateAssetLoader(assets: AssetManager): Long
    private external fun nDestroyAssetLoader(loader: Long)

    // Returns a handle owning the sample and its display manager, or 0 on failure.
    private external fun nCreateSample(name: String, loader: Long): Long
    private external fun nDestroySample(sample: Long)

    private external fun nOnSurfaceCreated(sample: Long, window: Long)
    private external fun nOnSurfaceChanged(sample: Long, width: Int, height: Int)
    private external fun nOnSurfaceDestroyed(sample: Long)
    private external fun nOnTouchEvent(sample: Long, action: Int, x: Float, y: Float)

    // Returns true once the sample has asked to close.
    private external fun nDoFrame(sample: Long): Boolean
}
