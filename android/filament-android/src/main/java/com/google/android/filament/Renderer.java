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

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;

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
    private DisplayInfo mDisplayInfo;
    private FrameRateOptions mFrameRateOptions;
    private ClearOptions mClearOptions;

    /**
     * Information about the display this renderer is associated to
     */
    public static class DisplayInfo {
        /**
         * Refresh rate of the display in Hz. Set to 0 for offscreen or turn off frame-pacing.
         * On Android you can use {@link android.view.Display#getRefreshRate()}.
         */
        public float refreshRate = 60.0f;

        /**
         * How far in advance a buffer must be queued for presentation at a given time in ns
         * On Android you can use {@link android.view.Display#getPresentationDeadlineNanos()}.
         * @deprecated this value is ignored
         */
        @Deprecated
        public long presentationDeadlineNanos = 0;

        /**
         * Offset by which vsyncSteadyClockTimeNano provided in beginFrame() is offset in ns
         * On Android you can use {@link android.view.Display#getAppVsyncOffsetNanos()}.
         * @deprecated this value is ignored
         */
        @Deprecated
        public long vsyncOffsetNanos = 0;
    };

    /**
     * Use FrameRateOptions to set the desired frame rate and control how quickly the system
     * reacts to GPU load changes.
     *
     * interval: desired frame interval in multiple of the refresh period, set in DisplayInfo
     *           (as 1 / DisplayInfo.refreshRate)
     *
     * The parameters below are relevant when some Views are using dynamic resolution scaling:
     *
     * headRoomRatio: additional headroom for the GPU as a ratio of the targetFrameTime.
     *                Useful for taking into account constant costs like post-processing or
     *                GPU drivers on different platforms.
     * history:   History size. higher values, tend to filter more (clamped to 30)
     * scaleRate: rate at which the gpu load is adjusted to reach the target frame rate
     *            This value can be computed as 1 / N, where N is the number of frames
     *            needed to reach 64% of the target scale factor.
     *            Higher values make the dynamic resolution react faster.
     *
     * @see View.DynamicResolutionOptions
     * @see Renderer.DisplayInfo
     *
     */
    public static class FrameRateOptions {
        /**
         * Desired frame interval in unit of 1 / DisplayInfo.refreshRate.
         */
        public float interval = 1.0f;

        /**
         * Additional headroom for the GPU as a ratio of the targetFrameTime.
         */
        public float headRoomRatio = 0.0f;

        /**
         * Rate at which the scale will change to reach the target frame rate.
         */
        public float scaleRate = 1.0f / 15.0f;

        /**
         * History size. higher values, tend to filter more (clamped to 31).
         */
        public int history = 15;
    }

    /**
     * ClearOptions are used at the beginning of a frame to clear or retain the SwapChain content.
     */
    public static class ClearOptions {
        /**
         * Color (sRGB linear) to use to clear the RenderTarget (typically the SwapChain).
         *
         * The RenderTarget is cleared using this color, which won't be tone-mapped since
         * tone-mapping is part of View rendering (this is not).
         *
         * When a View is rendered, there are 3 scenarios to consider:
         * - Pixels rendered by the View replace the clear color (or blend with it in
         *   `BlendMode.TRANSLUCENT` mode).
         *
         * - With blending mode set to `BlendMode.TRANSLUCENT`, Pixels untouched by the View
         *   are considered fulling transparent and let the clear color show through.
         *
         * - With blending mode set to `BlendMode.OPAQUE`, Pixels untouched by the View
         *   are set to the clear color. However, because it is now used in the context of a View,
         *   it will go through the post-processing stage, which includes tone-mapping.
         *
         * For consistency, it is recommended to always use a Skybox to clear an opaque View's
         * background, or to use black or fully-transparent (i.e. {0,0,0,0}) as the clear color.
         */
        @NonNull
        public float[] clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

        /**
         * Whether the SwapChain should be cleared using the clearColor. Use this if translucent
         * View will be drawn, for instance.
         */
        public boolean clear = false;

        /**
         * Whether the SwapChain content should be discarded. clear implies discard. Set this
         * to false (along with clear to false as well) if the SwapChain already has content that
         * needs to be preserved
         */
        public boolean discard = true;
    };

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
     * Information about the display this Renderer is associated to. This information is needed
     * to accurately compute dynamic-resolution scaling and for frame-pacing.
     */
    public void setDisplayInfo(@NonNull DisplayInfo info) {
        mDisplayInfo = info;
        nSetDisplayInfo(getNativeObject(), info.refreshRate);
    }

    /**
     * Returns the DisplayInfo object set in {@link #setDisplayInfo} or a new instance otherwise.
     * @return a DisplayInfo instance
     */
    @NonNull
    public DisplayInfo getDisplayInfo() {
        if (mDisplayInfo == null) {
            mDisplayInfo = new DisplayInfo();
        }
        return mDisplayInfo;
    }

    /**
     * Set options controlling the desired frame-rate.
     */
    public void setFrameRateOptions(@NonNull FrameRateOptions options) {
        mFrameRateOptions = options;
        nSetFrameRateOptions(getNativeObject(),
                options.interval, options.headRoomRatio, options.scaleRate, options.history);
    }

    /**
     * Returns the FrameRateOptions object set in {@link #setFrameRateOptions} or a new instance
     * otherwise.
     * @return a FrameRateOptions instance
     */
    @NonNull
    public FrameRateOptions getFrameRateOptions() {
        if (mFrameRateOptions == null) {
            mFrameRateOptions = new FrameRateOptions();
        }
        return mFrameRateOptions;
    }

    /**
     * Set ClearOptions which are used at the beginning of a frame to clear or retain the
     * SwapChain content.
     */
    public void setClearOptions(@NonNull ClearOptions options) {
        mClearOptions = options;
        nSetClearOptions(getNativeObject(),
                options.clearColor[0], options.clearColor[1], options.clearColor[2], options.clearColor[3],
                options.clear, options.discard);
    }

    /**
     * Returns the ClearOptions object set in {@link #setClearOptions} or a new instance
     * otherwise.
     * @return a ClearOptions instance
     */
    @NonNull
    public ClearOptions getClearOptions() {
        if (mClearOptions == null) {
            mClearOptions = new ClearOptions();
        }
        return mClearOptions;
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
     * Set the time at which the frame must be presented to the display.
     * <p>
     * This must be called between {@link #beginFrame} and {@link #endFrame}.
     * </p>
     *
     * @param monotonicClockNanos  The time in nanoseconds corresponding to the system monotonic
     *                             up-time clock. The presentation time is typically set in the
     *                             middle of the period of interest and cannot be too far in the
     *                             future as it is limited by how many buffers are available in
     *                             the display sub-system. Typically it is set to 1 or 2 vsync
     *                             periods away.
     */
    public void setPresentationTime(long monotonicClockNanos) {
        nSetPresentationTime(getNativeObject(), monotonicClockNanos);
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
     * @param frameTimeNanos The time in nanoseconds when the frame started being rendered,
     *                       in the {@link System#nanoTime()} timebase. Divide this value by 1000000 to
     *                       convert it to the {@link android.os.SystemClock#uptimeMillis()}
     *                       time base. This typically comes from
     *                       {@link android.view.Choreographer.FrameCallback}.
     *
     * @return <code>true</code>: the current frame must be drawn, and {@link #endFrame} must be called<br>
     *         <code>false</code>: the current frame should be skipped, when skipping a frame,
     *         the whole frame is canceled, and {@link #endFrame} must not
     *         be called. However, the user can choose to proceed as though <code>true</code> was
     *         returned and produce a frame anyways, by making calls to {@link #render(View)},
     *         in which case {@link #endFrame} must be called.
     *
     * @see #endFrame
     * @see #render
     */
    public boolean beginFrame(@NonNull SwapChain swapChain, long frameTimeNanos) {
        return nBeginFrame(getNativeObject(), swapChain.getNativeObject(), frameTimeNanos);
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
     * <li>Shadow map passes, if needed</li>
     * <li>Depth pre-pass</li>
     * <li>SSAO pass, if enabled</li>
     * <li>Color pass</li>
     * <li>Post-processing pass</li>
     * </ul>
     * <p>
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
     * <li><code>render()</code> must be called <b>after</b> {@link #beginFrame} and <b>before</b>
     * {@link #endFrame}.</li>
     *
     * <li><code>render()</code> must be called from the {@link Engine}'s main thread
     * (or external synchronization must be provided). In particular, calls to <code>render()</code>
     * on different <code>Renderer</code> instances <b>must</b> be synchronized.</li>
     *
     * <li><code>render()</code> performs potentially heavy computations and cannot be multi-threaded.
     * However, internally, it is highly multi-threaded to both improve performance and mitigate
     * the call's latency.</li>
     *
     * <li><code>render()</code> is typically called once per frame (but not necessarily).</li>
     * </ul>
     *
     * @param view the {@link View} to render
     * @see #beginFrame
     * @see #endFrame
     * @see View
     */
    public void render(@NonNull View view) {
        nRender(getNativeObject(), view.getNativeObject());
    }

    /**
     * Renders a standalone {@link View} into its associated <code>RenderTarget</code>.
     *
     * <p>
     * This call is mostly equivalent to calling {@link #render(View)} inside a
     * {@link #beginFrame} / {@link #endFrame} block, but incurs less overhead. It can be used
     * as a poor man's compute API. 
     * </p>
     *
     * <ul>
     * <li><code>renderStandaloneView()</code> must be called <b>outside</b> of
     * {@link #beginFrame} / {@link #endFrame}.</li>
     *
     * <li><code>renderStandaloneView()</code> must be called from the {@link Engine}'s main thread
     * (or external synchronization must be provided). In particular, calls to
     * <code>renderStandaloneView()</code> on different <code>Renderer</code> instances <b>must</b>
     * be synchronized.</li>
     *
     * <li><code>renderStandaloneView()</code> performs potentially heavy computations and cannot be
     * multi-threaded. However, internally, it is highly multi-threaded to both improve performance
     * and mitigate the call's latency.</li>
     *
     * @param view the {@link View} to render. This View must have an associated {@link RenderTarget}
     * @see View
     */
    public void renderStandaloneView(@NonNull View view) {
        nRenderStandaloneView(getNativeObject(), view.getNativeObject());
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
     * <p><code>readPixels</code> must be called within a frame, meaning after {@link #beginFrame}
     * and before {@link #endFrame}. Typically, <code>readPixels</code> will be called after
     * {@link #render}.</p>
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
     * <p>OpenGL only: if issuing a <code>readPixels</code> on a {@link RenderTarget} backed by a
     * {@link Texture} that had data uploaded to it via {@link Texture#setImage}, the data returned
     * from <code>readPixels</code> will be y-flipped with respect to the {@link Texture#setImage}
     * call.</p>
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
     * <code>Activity.onPause</code> in Android.</p>
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

    private static native void nSetPresentationTime(long nativeObject, long monotonicClockNanos);
    private static native boolean nBeginFrame(long nativeRenderer, long nativeSwapChain, long frameTimeNanos);
    private static native void nEndFrame(long nativeRenderer);
    private static native void nRender(long nativeRenderer, long nativeView);
    private static native void nRenderStandaloneView(long nativeRenderer, long nativeView);
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
    private static native void nSetDisplayInfo(long nativeRenderer, float refreshRate);
    private static native void nSetFrameRateOptions(long nativeRenderer,
            float interval, float headRoomRatio, float scaleRate, int history);
    private static native void nSetClearOptions(long nativeRenderer,
            float r, float g, float b, float a, boolean clear, boolean discard);
}
