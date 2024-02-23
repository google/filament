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

    public static final long CONFIG_DEFAULT = 0x0;

    /**
     * This flag indicates that the <code>SwapChain</code> must be allocated with an
     * alpha-channel.
     */
    public static final long CONFIG_TRANSPARENT = 0x1;

    /**
     * This flag indicates that the <code>SwapChain</code> may be used as a source surface
     * for reading back render results.  This config must be set when creating
     * any <code>SwapChain</code>  that will be used as the source for a blit operation.
     *
     * @see Renderer#copyFrame
     */
    public static final long CONFIG_READABLE = 0x2;

    /**
     * Indicates that the native X11 window is an XCB window rather than an XLIB window.
     * This is ignored on non-Linux platforms and in builds that support only one X11 API.
     */
    public static final long CONFIG_ENABLE_XCB = 0x4;

    /**
     * Indicates that the SwapChain must automatically perform linear to sRGB encoding.
     *
     * This flag is ignored if isSRGBSwapChainSupported() is false.
     *
     * When using this flag, post-processing should be disabled.
     *
     * @see SwapChain#isSRGBSwapChainSupported
     * @see View#setPostProcessingEnabled
     */
    public static final long CONFIG_SRGB_COLORSPACE = 0x10;

    /**
     * Indicates that this SwapChain should allocate a stencil buffer in addition to a depth buffer.
     *
     * This flag is necessary when using View::setStencilBufferEnabled and rendering directly into
     * the SwapChain (when post-processing is disabled).
     *
     * The specific format of the stencil buffer depends on platform support. The following pixel
     * formats are tried, in order of preference:
     *
     * Depth only (without CONFIG_HAS_STENCIL_BUFFER):
     * - DEPTH32F
     * - DEPTH24
     *
     * Depth + stencil (with CONFIG_HAS_STENCIL_BUFFER):
     * - DEPTH32F_STENCIL8
     * - DEPTH24F_STENCIL8
     *
     * Note that enabling the stencil buffer may hinder depth precision and should only be used if
     * necessary.
     *
     * @see View#setStencilBufferEnabled
     * @see View#setPostProcessingEnabled
     */
    public static final long CONFIG_HAS_STENCIL_BUFFER = 0x20;

    /**
     * The SwapChain contains protected content. Only supported when isProtectedContentSupported()
     * is true.
     */
    public static final long CONFIG_PROTECTED_CONTENT   = 0x40;

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
     */
    public static boolean isProtectedContentSupported(@NonNull Engine engine) {
        return nIsProtectedContentSupported(engine.getNativeObject());
    }

    /**
     * Return whether createSwapChain supports the SWAP_CHAIN_CONFIG_SRGB_COLORSPACE flag.
     * The default implementation returns false.
     *
     * @param engine A reference to the filament Engine
     * @return true if SWAP_CHAIN_CONFIG_SRGB_COLORSPACE is supported, false otherwise.
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
    private static native boolean nIsSRGBSwapChainSupported(long nativeEngine);
    private static native boolean nIsProtectedContentSupported(long nativeEngine);
}
