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

import android.support.annotation.IntRange;
import android.support.annotation.NonNull;

import java.nio.Buffer;
import java.nio.BufferOverflowException;
import java.nio.ReadOnlyBufferException;

/**
 * A <code>Renderer</code> instance represents an operating system's window.
 *
 * <p>
 * Typically, applications create a <code>Renderer</code> per window. The <code>Renderer</code> generates
 * drawing commands for the render thread and manages frame latency.
 * <br>
 * A Renderer generates drawing commands from a View, itself containing a Scene description.
 * </p>
 *
 * <h1>Creation and Destruction</h1>
 *
 * <p>A <code>Renderer</code> is created using {@link Engine#createRenderer} and destroyed
 * using {@link Engine#destroyRenderer}.</p>
 *
 * @see Engine
 * @see View
 */
public class Renderer {
    private final Engine mEngine;
    private long mNativeObject;

    /**
     * Indicates that the <code>dstSwapChain</code> passed into {@link #copyFrame} should be
     * committed after the frame has been copied.
     *
     * @see #copyFrame
     */
    public static final int MIRROR_FRAME_FLAG_COMMIT = 0x1;

    /**
     * Indicates that the presentation time should be set on the <code>dstSwapChain</code>
     * passed into {@link #copyFrame} to the monotonic clock time when the frame is
     * copied.
     *
     * @see #copyFrame
     */
    public static final int MIRROR_FRAME_FLAG_SET_PRESENTATION_TIME = 0x2;

    /**
     * Indicates that the <code>dstSwapChain</code> passed into {@link #copyFrame} should be
     * cleared to black before the frame is copied into the specified viewport.
     *
     * @see #copyFrame
     */
    public static final int MIRROR_FRAME_FLAG_CLEAR = 0x4;

    Renderer(@NonNull Engine engine, long nativeRenderer) {
        mEngine = engine;
        mNativeObject = nativeRenderer;
    }

    /**
     * Gets the {@link Engine} that created this <code>Renderer</code>.
     *
     * @return {@link Engine} instance this <code>Renderer</code> is associated to.
     */
    @NonNull
    public Engine getEngine() {
        return mEngine;
    }

    /**
     * Sets up a frame for this <code>Renderer</code>.
     * <p><code>beginFrame</code> manages frame pacing, and returns whether or not a frame should be
     * drawn. The goal of this is to skip frames when the GPU falls behind in order to keep the frame
     * latency low.</p>
     *
     * <p>If a given frame takes too much time in the GPU, the CPU will get ahead of the GPU. The
     * display will draw the same frame twice producing a stutter. At this point, the CPU is
     * ahead of the GPU and depending on how many frames are buffered, latency increases.
     * beginFrame() attempts to detect this situation and returns <code>false</code> in that case,
     * indicating to the caller to skip the current frame.</p>
     *
     * <p>All calls to render() must happen <b>after</b> beginFrame().</p>
     *
     * @param swapChain the {@link SwapChain} instance to use
     *
     * @return <code>false</code> if the current frame must be skipped<br>
     *         When skipping a frame, the whole frame is canceled, and {@link #endFrame} must not
     *         be called.
     *
     * @see #endFrame
     * @see #render
     */
    public boolean beginFrame(@NonNull SwapChain swapChain) {
        return nBeginFrame(getNativeObject(), swapChain.getNativeObject());
    }

    /**
     * Finishes the current frame and schedules it for display.
     * <p>
     * <code>endFrame()</code> schedules the current frame to be displayed on the
     * <code>Renderer</code>'s window.
     * </p>
     *
     * <br><p>All calls to render() must happen <b>before</b> endFrame().</p>
     *
     * @see #beginFrame
     * @see #render
     */
    public void endFrame() {
        nEndFrame(getNativeObject());
    }

    /**
     * Renders a {@link View} into this <code>Renderer</code>'s window.
     *
     * <p>
     * This is filament's main rendering method, most of the CPU-side heavy lifting is performed
     * here. The purpose of the <code>render()</code> function is to generate render commands which
     * are asynchronously executed by the {@link Engine}'s render thread.
     * </p>
     *
     * <p><code>render()</code> generates commands for each of the following stages:</p>
     * <ul>
     * <li>Shadow map pass, if needed (currently only a single shadow map is supported)</li>
     * <li>Depth pre-pass</li>
     * <li>SSAO pass, if enabled</li>
     * <li>Color pass</li>
     * <li>Post-processing pass</li>
     * </ul>
     *
     * A typical render loop looks like this:
     *
     * <pre>
     * void renderLoop(Renderer renderer, SwapChain swapChain) {
     *     do {
     *         // typically we wait for VSYNC and user input events
     *         if (renderer.beginFrame(swapChain)) {
     *             renderer.render(mView);
     *             renderer.endFrame();
     *         }
     *     } while (!quit());
     * }
     * </pre>
     *
     * <ul>
     *<li><code>render()</code> must be called <b>after</b> {@link #beginFrame} and <b>before</b>
     *{@link #endFrame}.</li>
     *
     *<li><code>render()</code> must be called from the {@link Engine}'s main thread
     *(or external synchronization must be provided). In particular, calls to <code>render()</code>
     *on different <code>Renderer</code> instances <b>must</b> be synchronized.</li>
     *
     *<li><code>render()</code> performs potentially heavy computations and cannot be multi-threaded.
     *However, internally, it is highly multi-threaded to both improve performance and mitigate
     *the call's latency.</li>
     *
     *<li><code>render()</code> is typically called once per frame (but not necessarily).</li>
     * </ul>
     *
     * @param view the {@link View} to render
     *
     * @see #beginFrame
     * @see #endFrame
     * @see View
     *
     */
    public void render(@NonNull View view) {
        nRender(getNativeObject(), view.getNativeObject());
    }

    /**
     * Copies the currently rendered {@link View} to the indicated {@link SwapChain}, using the
     * indicated source and destination rectangle.
     *
     * <p><code>copyFrame()</code> should be called after a frame is rendered using {@link #render}
     * but before {@link #endFrame} is called.</p>
     *
     * @param dstSwapChain the {@link SwapChain} into which the frame should be copied
     * @param dstViewport the destination rectangle in which to draw the view
     * @param srcViewport the source rectangle to be copied
     * @param flags one or more <code>CopyFrameFlag</code> behavior configuration flags
     */
    public void copyFrame(
            @NonNull SwapChain dstSwapChain, @NonNull Viewport dstViewport,
            @NonNull Viewport srcViewport, int flags) {
        nCopyFrame(getNativeObject(), dstSwapChain.getNativeObject(),
                dstViewport.left, dstViewport.bottom, dstViewport.width, dstViewport.height,
                srcViewport.left, srcViewport.bottom, srcViewport.width, srcViewport.height,
                flags);
    }

    @Deprecated
    public void mirrorFrame(
            @NonNull SwapChain dstSwapChain, @NonNull Viewport dstViewport,
            @NonNull Viewport srcViewport, int flags) {
        copyFrame(dstSwapChain, dstViewport, srcViewport, flags);
    }

    /**
     * Reads back the content of the {@link SwapChain} associated with this <code>Renderer</code>.
     *
     *<pre>
     *
     *  Framebuffer as seen on         User buffer (PixelBufferDescriptor)
     *  screen
     *  +--------------------+
     *  |                    |                .stride         .alignment
     *  |                    |         ----------------------->-->
     *  |                    |         O----------------------+--+   low addresses
     *  |                    |         |          |           |  |
     *  |             w      |         |          | .top      |  |
     *  |       <--------->  |         |          V           |  |
     *  |       +---------+  |         |     +---------+      |  |
     *  |       |     ^   |  | ======> |     |         |      |  |
     *  |   x   |    h|   |  |         |.left|         |      |  |
     *  +------>|     v   |  |         +---->|         |      |  |
     *  |       +.........+  |         |     +.........+      |  |
     *  |            ^       |         |                      |  |
     *  |          y |       |         +----------------------+--+  high addresses
     *  O------------+-------+
     *
     *</pre>
     *
     *
     * <p>Typically <code>readPixels</code> will be called after {@link #render} and before
     * {@link #endFrame}.</p>
     * <br>
     * <p>After calling this method, the callback associated with <code>buffer</code>
     * will be invoked on the main thread, indicating that the read-back has completed.
     * Typically, this will happen after multiple calls to {@link #beginFrame},
     * {@link #render}, {@link #endFrame}.</p>
     * <br>
     * <p><code>readPixels</code> is intended for debugging and testing.
     * It will impact performance significantly.</p>
     *
     * @param xoffset   left offset of the sub-region to read back
     * @param yoffset   bottom offset of the sub-region to read back
     * @param width     width of the sub-region to read back
     * @param height    height of the sub-region to read back
     * @param buffer    client-side buffer where the read-back will be written
     *
     *                  <p>
     *                  The following format are always supported:
     *                      <li>{@link Texture.Format#RGBA}</li>
     *                      <li>{@link Texture.Format#RGBA_INTEGER}</li>
     *                  </p>
     *
     *                  <p>
     *                  The following types are always supported:
     *                      <li>{@link Texture.Type#UBYTE}</li>
     *                      <li>{@link Texture.Type#UINT}</li>
     *                      <li>{@link Texture.Type#INT}</li>
     *                      <li>{@link Texture.Type#FLOAT}</li>
     *                  </p>
     *
     *                  <p>Other combination of format/type may be supported. If a combination is
     *                  not supported, this operation may fail silently. Use a DEBUG build
     *                  to get some logs about the failure.</p>
     *
     * @exception BufferOverflowException if the specified parameters would result in reading
     * outside of <code>buffer</code>.
     */
    public void readPixels(
            @IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height,
            @NonNull Texture.PixelBufferDescriptor buffer) {

        if (buffer.storage.isReadOnly()) {
            throw new ReadOnlyBufferException();
        }

        int result = nReadPixels(getNativeObject(), mEngine.getNativeObject(),
                xoffset, yoffset, width, height,
                buffer.storage, buffer.storage.remaining(),
                buffer.left, buffer.top, buffer.type.ordinal(), buffer.alignment,
                buffer.stride, buffer.format.ordinal(),
                buffer.handler, buffer.callback);

        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    /**
     * Reads back the content of a specified {@link RenderTarget}.
     *
     *<pre>
     *
     *  Framebuffer as seen on         User buffer (PixelBufferDescriptor)
     *  screen
     *  +--------------------+
     *  |                    |                .stride         .alignment
     *  |                    |         ----------------------->-->
     *  |                    |         O----------------------+--+   low addresses
     *  |                    |         |          |           |  |
     *  |             w      |         |          | .top      |  |
     *  |       <--------->  |         |          V           |  |
     *  |       +---------+  |         |     +---------+      |  |
     *  |       |     ^   |  | ======> |     |         |      |  |
     *  |   x   |    h|   |  |         |.left|         |      |  |
     *  +------>|     v   |  |         +---->|         |      |  |
     *  |       +.........+  |         |     +.........+      |  |
     *  |            ^       |         |                      |  |
     *  |          y |       |         +----------------------+--+  high addresses
     *  O------------+-------+
     *
     *</pre>
     *
     *
     * <p>Typically <code>readPixels</code> will be called after {@link #render} and before
     * {@link #endFrame}.</p>
     * <br>
     * <p>After calling this method, the callback associated with <code>buffer</code>
     * will be invoked on the main thread, indicating that the read-back has completed.
     * Typically, this will happen after multiple calls to {@link #beginFrame},
     * {@link #render}, {@link #endFrame}.</p>
     * <br>
     * <p><code>readPixels</code> is intended for debugging and testing.
     * It will impact performance significantly.</p>
     *
     * @param renderTarget  {@link RenderTarget} to read back from
     * @param xoffset       left offset of the sub-region to read back
     * @param yoffset       bottom offset of the sub-region to read back
     * @param width         width of the sub-region to read back
     * @param height        height of the sub-region to read back
     * @param buffer        client-side buffer where the read-back will be written
     *
     *                  <p>
     *                  The following format are always supported:
     *                      <li>{@link Texture.Format#RGBA}</li>
     *                      <li>{@link Texture.Format#RGBA_INTEGER}</li>
     *                  </p>
     *
     *                  <p>
     *                  The following types are always supported:
     *                      <li>{@link Texture.Type#UBYTE}</li>
     *                      <li>{@link Texture.Type#UINT}</li>
     *                      <li>{@link Texture.Type#INT}</li>
     *                      <li>{@link Texture.Type#FLOAT}</li>
     *                  </p>
     *
     *                  <p>Other combination of format/type may be supported. If a combination is
     *                  not supported, this operation may fail silently. Use a DEBUG build
     *                  to get some logs about the failure.</p>
     *
     * @exception BufferOverflowException if the specified parameters would result in reading
     * outside of <code>buffer</code>.
     */
    public void readPixels(
            @NonNull RenderTarget renderTarget,
            @IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height,
            @NonNull Texture.PixelBufferDescriptor buffer) {

        if (buffer.storage.isReadOnly()) {
            throw new ReadOnlyBufferException();
        }

        int result = nReadPixelsEx(getNativeObject(), mEngine.getNativeObject(),
                renderTarget.getNativeObject(),
                xoffset, yoffset, width, height,
                buffer.storage, buffer.storage.remaining(),
                buffer.left, buffer.top, buffer.type.ordinal(), buffer.alignment,
                buffer.stride, buffer.format.ordinal(),
                buffer.handler, buffer.callback);

        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    /**
     * Returns a timestamp (in seconds) for the last call to {@link #beginFrame}. This value is
     * constant for all {@link View views} rendered during a frame. The epoch is set with
     * {@link #resetUserTime}.
     * <br>
     * <p>In materials, this value can be queried using <code>vec4 getUserTime()</code>. The value
     * returned is a <code>highp vec4</code> encoded as follows:</p>
     * <pre>
     *      time.x = (float)Renderer.getUserTime();
     *      time.y = Renderer.getUserTime() - time.x;
     * </pre>
     *
     * It follows that the following invariants are true:
     * <pre>
     *      (double)time.x + (double)time.y == Renderer.getUserTime()
     *      time.x == (float)Renderer.getUserTime()
     * </pre>
     *
     * This encoding allows the shader code to perform high precision (i.e. double) time
     * calculations when needed despite the lack of double precision in the shader, e.g.:
     * <br>
     *      To compute <code>(double)time * vertex</code> in the material, use the following construct:
     * <pre>
     *              vec3 result = time.x * vertex + time.y * vertex;
     * </pre>
     *
     * Most of the time, high precision computations are not required, but be aware that the
     * precision of <code>time.x</code> rapidly diminishes as time passes:
     *
     * <center>
     * <table border="1">
     *     <tr align="center"><th> time    </th><th> precision </th></tr>
     *     <tr align="center"><td> 16.7s   </td><td> us        </td></tr>
     *     <tr align="center"><td> 4h39.7s </td><td> ms        </td></tr>
     *     <tr align="center"><td> 77h     </td><td> 1/60s     </td></tr>
     * </table>
     * </center>
     * <p>
     *
     * In other words, it is only possible to get microsecond accuracy for about 16s or millisecond
     * accuracy for just under 5h. This problem can be mitigated by calling {@link #resetUserTime},
     * or using high precision time as described above.
     *
     * @return the time in seconds since {@link #resetUserTime} was last called
     *
     * @see #resetUserTime
     */
    public double getUserTime() {
        return nGetUserTime(getNativeObject());
    }

    /**
     * Sets the user time epoch to now, i.e. resets the user time to zero.
     * <br>
     * <p>Use this method used to keep the precision of time high in materials, in practice it should
     * be called at least when the application is paused, e.g.
     * {@link android.app.Activity#onPause() Activity.onPause} in Android.</p>
     *
     * @see #getUserTime
     */
    public void resetUserTime() {
        nResetUserTime(getNativeObject());
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Renderer");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native boolean nBeginFrame(long nativeRenderer, long nativeSwapChain);
    private static native void nEndFrame(long nativeRenderer);
    private static native void nRender(long nativeRenderer, long nativeView);
    private static native void nCopyFrame(long nativeRenderer, long nativeDstSwapChain,
            int dstLeft, int dstBottom, int dstWidth, int dstHeight,
            int srcLeft, int srcBottom, int srcWidth, int srcHeight,
            int flags);
    private static native int nReadPixels(long nativeRenderer, long nativeEngine,
            int xoffset, int yoffset, int width, int height,
            Buffer storage, int remaining,
            int left, int top, int type, int alignment, int stride, int format,
            Object handler, Runnable callback);
    private static native int nReadPixelsEx(long nativeRenderer, long nativeEngine,
            long nativeRenderTarget,
            int xoffset, int yoffset, int width, int height,
            Buffer storage, int remaining,
            int left, int top, int type, int alignment, int stride, int format,
            Object handler, Runnable callback);
    private static native double nGetUserTime(long nativeRenderer);
    private static native void nResetUserTime(long nativeRenderer);
}
