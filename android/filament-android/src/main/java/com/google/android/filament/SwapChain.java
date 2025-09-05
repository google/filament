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

import androidx.annotation.NonNull;

/**
 * A <code>SwapChain</code> represents an Operating System's <b>native</b> renderable surface.
 *
 * <p>Typically it's a native window or a view. Because a <code>SwapChain</code> is initialized
 * from a native object, it is given to filament as an <code>Object</code>, which must be of the
 * proper type for each platform filament is running on.</p>
 *
 * <code>
 * SwapChain swapChain = engine.createSwapChain(nativeWindow);
 * </code>
 *
 * <p>The <code>nativeWindow</code> parameter above must be of type:</p>
 *
 * <center>
 * <table border="1">
 *     <tr><th> Platform </th><th> nativeWindow type </th></tr>
 *     <tr><td> Android </td><td>{@link android.view.Surface Surface}</td></tr>
 * </table>
 * </center>
 * <p>
 *
 * <h1>Examples</h1>
 *
 * <h2>Android</h2>
 *
 *
 * <p>A {@link android.view.Surface Surface} can be retrieved from a
 * {@link android.view.SurfaceView SurfaceView} or {@link android.view.SurfaceHolder SurfaceHolder}
 * easily using {@link android.view.SurfaceHolder#getSurface SurfaceHolder.getSurface()} and/or
 * {@link android.view.SurfaceView#getHolder SurfaceView.getHolder()}.</p>
 *
 * <p>To use a {@link android.view.TextureView Textureview} as a <code>SwapChain</code>, it is
 * necessary to first get its {@link android.graphics.SurfaceTexture SurfaceTexture},
 * for instance using {@link android.view.TextureView.SurfaceTextureListener SurfaceTextureListener}
 * and then create a {@link android.view.Surface Surface}:</p>
 *
 * <pre>
 *  // using a TextureView.SurfaceTextureListener:
 *  public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
 *      mSurface = new Surface(surfaceTexture);
 *      // mSurface can now be used with Engine.createSwapChain()
 *  }
 * </pre>
 *
 * @see Engine
 */
public class SwapChain {
    private final Object mSurface;
    private long mNativeObject;

    SwapChain(long nativeSwapChain, Object surface) {
        mNativeObject = nativeSwapChain;
        mSurface = surface;
    }

    /**
     * Return whether createSwapChain supports the CONFIG_PROTECTED_CONTENT flag.
     * The default implementation returns false.
     *
     * @param engine A reference to the filament Engine
     * @return true if CONFIG_PROTECTED_CONTENT is supported, false otherwise.
     * @see SwapChainFlags#CONFIG_PROTECTED_CONTENT
     */
    public static boolean isProtectedContentSupported(@NonNull Engine engine) {
        return nIsProtectedContentSupported(engine.getNativeObject());
    }

    /**
     * Return whether createSwapChain supports the CONFIG_SRGB_COLORSPACE flag.
     * The default implementation returns false.
     *
     * @param engine A reference to the filament Engine
     * @return true if CONFIG_SRGB_COLORSPACE is supported, false otherwise.
     * @see SwapChainFlags#CONFIG_SRGB_COLORSPACE
     */
    public static boolean isSRGBSwapChainSupported(@NonNull Engine engine) {
        return nIsSRGBSwapChainSupported(engine.getNativeObject());
    }

    /**
     * @return the native <code>Object</code> this <code>SwapChain</code> was created from or null
     *         for a headless SwapChain.
     */
    public Object getNativeWindow() {
        return mSurface;
    }

    /**
     * FrameCompletedCallback is a callback function that notifies an application when a frame's
     * contents have completed rendering on the GPU.
     *
     * <p>
     * Use setFrameCompletedCallback to set a callback on an individual SwapChain. Each time a frame
     * completes GPU rendering, the callback will be called.
     * </p>
     *
     * <p>
     * Warning: Only Filament's Metal backend supports frame callbacks. Other backends ignore the
     * callback (which will never be called) and proceed normally.
     * </p>
     *
     * @param handler     A {@link java.util.concurrent.Executor Executor}.
     * @param callback    The Runnable callback to invoke.
     */
    public void setFrameCompletedCallback(@NonNull Object handler, @NonNull Runnable callback) {
        nSetFrameCompletedCallback(getNativeObject(), handler, callback);
    }

    /**
     * FrameScheduledCallback is a callback function that notifies an application about the status
     * of a frame after Filament has finished its processing.
     *
     * <p>
     * The exact timing and semantics of this callback differ depending on the graphics backend in
     * use.
     * </p>
     *
     * <h3>Metal Backend</h3>
     * <p>
     * With the Metal backend, this callback signifies that Filament has completed all CPU-side
     * processing for a frame and the frame is ready to be scheduled for presentation.
     * </p>
     *
     * <p>
     * Typically, Filament is responsible for scheduling the frame's presentation to the SwapChain.
     * If a FrameScheduledCallback is set, however, the application bears the responsibility of
     * scheduling the frame for presentation by calling the PresentCallable passed to the callback
     * function. In this mode, Filament will not automatically schedule the frame for presentation.
     * </p>
     *
     * <p>
     * When using the Metal backend, if your application delays the call to the PresentCallable
     * (e.g., by invoking it on a separate thread), you must ensure all PresentCallables have been
     * called before shutting down the Filament Engine. You can guarantee this by calling
     * Engine.flushAndWait() before Engine.shutdown(). This is necessary to ensure the Engine has
     * a chance to clean up all memory related to frame presentation.
     * </p>
     *
     * <h3>Other Backends (OpenGL, Vulkan, WebGPU)</h3>
     * <p>
     * On other backends, this callback serves as a notification that Filament has completed all
     * CPU-side processing for a frame. Filament proceeds with its normal presentation logic
     * automatically, and the PresentCallable passed to the callback is a no-op that can be safely
     * ignored.
     * </p>
     *
     * <h3>General Behavior</h3>
     * <p>
     * A FrameScheduledCallback can be set on an individual SwapChain through
     * setFrameScheduledCallback. Each SwapChain can have only one callback set per frame. If
     * setFrameScheduledCallback is called multiple times on the same SwapChain before
     * Renderer.endFrame(), the most recent call effectively overwrites any previously set callback.
     * </p>
     *
     * <p>
     * The callback set by setFrameScheduledCallback is "latched" when Renderer.endFrame() is
     * executed. At this point, the callback is fixed for the frame that was just encoded.
     * Subsequent calls to setFrameScheduledCallback after endFrame() will apply to the next frame.
     * </p>
     *
     * <p>
     * Use setFrameScheduledCallback() (with default arguments) to unset the callback.
     * </p>
     *
     * @param handler     A {@link java.util.concurrent.Executor Executor}.
     * @param callback    The Runnable callback to invoke when frame processing is complete.
     */
    public void setFrameScheduledCallback(@NonNull Object handler, @NonNull Runnable callback) {
        nSetFrameScheduledCallback(getNativeObject(), handler, callback);
    }

    /**
     * Returns whether this SwapChain currently has a FrameScheduledCallback set.
     *
     * @return true, if the last call to setFrameScheduledCallback set a callback
     */
    public boolean isFrameScheduledCallbackSet() {
        return nIsFrameScheduledCallbackSet(getNativeObject());
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed SwapChain");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native void nSetFrameCompletedCallback(long nativeSwapChain, Object handler, Runnable callback);
    private static native void nSetFrameScheduledCallback(long nativeSwapChain, Object handler, Runnable callback);
    private static native boolean nIsFrameScheduledCallbackSet(long nativeSwapChain);
    private static native boolean nIsSRGBSwapChainSupported(long nativeEngine);
    private static native boolean nIsProtectedContentSupported(long nativeEngine);
}
