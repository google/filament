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
 * <code>Stream</code> is used to attach a native video stream to a filament {@link Texture}.
 *
 * Stream supports three different configurations:
 *
 * <dl>
 * <dt>TEXTURE_ID</dt>   <dd>takes an OpenGL texture ID and incurs a copy</dd>
 * <dt>ACQUIRED</dt>     <dd>connects to an Android AHardwareBuffer</dd>
 * <dt>NATIVE</dt>       <dd>connects to an Android SurfaceTexture</dd>
 * </dl>
 *
 * <p>
 * Before explaining these different configurations, let's review the high-level structure of an
 * AR or video application that uses Filament.
 * </p>
 *
 * <pre>
 * while (true) {
 *
 *     // Misc application work occurs here, such as:
 *     // - Writing the image data for a video frame into a Stream
 *     // - Moving the Filament Camera
 *
 *     if (renderer.beginFrame(swapChain)) {
 *         renderer.render(view);
 *         renderer.endFrame();
 *     }
 * }
 * </pre>
 *
 * <p>
 * Let's say that the video image data at the time of a particular invocation of beginFrame
 * becomes visible to users at time A. The 3D scene state (including the camera) at the time of
 * that same invocation becomes apparent to users at time B.
 * </p>
 *
 * <ul>
 * <li>If time A matches time B, we say that the stream is <em>synchronized</em>.</li>
 * <li>Filament invokes low-level graphics commands on the <em>driver thread</em>.</li>
 * <li>The thread that calls beginFrame is called the <em>main thread</em>.</li>
 * </ul>
 *
 * <p>
 * The <b>TEXTURE_ID</b> configuration achieves synchronization automatically. In this mode,
 * Filament performs a copy on the main thread during beginFrame by blitting the external image into
 * an internal round-robin queue of images. This copy has a run-time cost.
 * </p>
 *
 * <p>
 * For <b>ACQUIRED</b> streams, there is no need to perform the copy because Filament explictly
 * acquires the stream, then releases it later via a callback function. This configuration is
 * especially useful when the Vulkan backend is enabled.
 * </p>
 *
 * <p>
 * For <b>NATIVE</b> streams, Filament does not make any synchronization guarantee. However they are
 * simple to use and do not incur a copy. These are often appropriate in video applications.
 * </p>
 *
 * <p>
 * Please see <code>sample-stream-test</code> and <code>sample-hello-camera</code> for usage
 * examples.
 * </p>
 *
 * @see Texture#setExternalStream
 * @see Engine#destroyStream
 */
public class Stream {
    private long mNativeObject;
    private long mNativeEngine;

    /**
     * Represents the immutable stream type.
     */
    public enum StreamType {
        /** Not synchronized but copy-free. Good for video. */
        NATIVE,

        /** Synchronized, but GL-only and incurs copies. Good for AR on devices before API 26. */
        TEXTURE_ID,

        /** Synchronized, copy-free, and take a release callback. Good for AR but requires API 26+. */
        ACQUIRED,
    };

    Stream(long nativeStream, Engine engine) {
        mNativeObject = nativeStream;
        mNativeEngine = engine.getNativeObject();
    }

    /**
     * Use <code>Builder</code> to construct an Stream object instance.
     *
     * By default, Stream objects are {@link StreamType#ACQUIRED ACQUIRED} and must have external images pushed to them via
     * {@link #setAcquiredImage}.
     *
     * To create a {@link StreamType#NATIVE NATIVE} or {@link StreamType#TEXTURE_ID TEXTURE_ID} stream, call one of the <pre>stream</pre> methods
     * on the builder.
     */
    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"}) // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        /**
         * Use <code>Builder</code> to construct an Stream object instance.
         */
        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Creates a {@link StreamType#NATIVE NATIVE} stream. Native streams can sample data
         * directly from an opaque platform object such as a {@link android.graphics.SurfaceTexture SurfaceTexture}
         * on Android.
         *
         * @param streamSource an opaque native stream handle, e.g.: on Android this must be a
         *                     {@link android.graphics.SurfaceTexture SurfaceTexture} object
         * @return This Builder, for chaining calls.
         * @see Texture#setExternalStream
         */
        @NonNull
        public Builder stream(@NonNull Object streamSource) {
            if (Platform.get().validateStreamSource(streamSource)) {
                nBuilderStreamSource(mNativeBuilder, streamSource);
                return this;
            }
            throw new IllegalArgumentException("Invalid stream source: " + streamSource);
        }

        /**
         * Creates a {@link StreamType#TEXTURE_ID TEXTURE_ID} stream. A copy stream will sample data from the supplied
         * external texture and copy it into an internal private texture.
         *
         * <p>Currently only OpenGL external texture ids are supported.</p>
         *
         * @param externalTextureId An opaque texture id (typically a GLuint created with
         *                          <code>glGenTextures()</code>) in a context shared with
         *                          filament -- in that case this texture's target must be
         *                          <code>GL_TEXTURE_EXTERNAL_OES.</code>
         * @return This Builder, for chaining calls.
         * @see Texture#setExternalStream
         */
        @NonNull
        public Builder stream(long externalTextureId) {
            nBuilderStream(mNativeBuilder, externalTextureId);
            return this;
        }

        /**
         * @param width initial width of the incoming stream. Whether this value is used is
         *              stream dependent. On Android, it must be set when using
         *              {@link #stream(long)}
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder width(int width) {
            nBuilderWidth(mNativeBuilder, width);
            return this;
        }

        /**
         * @param height initial height of the incoming stream. Whether this value is used is
         *              stream dependent. On Android, it must be set when using
         *              {@link #stream(long)}
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder height(int height) {
            nBuilderHeight(mNativeBuilder, height);
            return this;
        }

        /**
         * Creates a new <code>Stream</code> object instance.
         *
         * @param engine {@link Engine} instance to associate this <code>Stream</code> with.
         *
         * @return newly created <code>Stream</code> object
         * @exception IllegalStateException if the <code>Stream</code> couldn't be created
         */
        @NonNull
        public Stream build(@NonNull Engine engine) {
            long nativeStream = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeStream == 0) throw new IllegalStateException("Couldn't create Stream");
            return new Stream(nativeStream, engine);
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;

            BuilderFinalizer(long nativeObject) { mNativeObject = nativeObject; }

            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) { // Ignore
                } finally {
                    nDestroyBuilder(mNativeObject);
                }
            }

        }
    }

    /**
     * Indicates whether this <code>Stream</code> is NATIVE, TEXTURE_ID, or ACQUIRED.
     */
    public StreamType getStreamType() {
        return StreamType.values()[nGetStreamType(getNativeObject())];
    }

    /**
     * Updates an <pre>ACQUIRED</pre> stream with an image that is guaranteed to be used in the next frame.
     *
     * This method tells Filament to immediately "acquire" the image and trigger a callback
     * when it is done with it. This should be called by the user outside of beginFrame / endFrame,
     * and should be called only once per frame. If the user pushes images to the same stream
     * multiple times in a single frame, only the final image is honored, but all callbacks are
     * invoked.
     *
     * This method should be called on the same thread that calls {@link Renderer#beginFrame}, which is
     * also where the callback is invoked. This method can only be used for streams that were
     * constructed without calling the {@link Builder.stream} method.
     *
     * See {@link Stream} for more information about NATIVE, TEXTURE_ID, and ACQUIRED configurations.
     *
     * @param hwbuffer {@link android.hardware.HardwareBuffer HardwareBuffer} (requires API level 26)
     * @param handler {@link java.util.concurrent.Executor Executor} or {@link android.os.Handler Handler}.
     * @param callback a callback invoked by <code>handler</code> when the <code>hwbuffer</code> can be released.
     */
    public void setAcquiredImage(Object hwbuffer, Object handler, Runnable callback) {
        nSetAcquiredImage(getNativeObject(), mNativeEngine, hwbuffer, handler, callback);
    }

    /**
     * Updates the size of the incoming stream. Whether this value is used is
     * stream dependent. On Android, it must be set when using
     * {@link Builder#stream(long)}
     *
     * @param width  new width of the incoming stream
     * @param height new height of the incoming stream
     */
    public void setDimensions(@IntRange(from = 0) int width, @IntRange(from = 0) int height) {
        nSetDimensions(getNativeObject(), width, height);
    }

    /**
     * Reads back the content of the last frame of a <code>Stream</code> since the last call to
     * {@link Renderer#beginFrame}.
     *
     * <p>The Stream must be a copy stream, which can be checked with {@link #getStreamType()}.
     * This function is a no-op otherwise.</p>
     *
     * <pre>
     *
     *  Stream buffer                  User buffer (PixelBufferDescriptor)
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
     * </pre>
     *
     * <p>Typically readPixels() will be called after {@link Renderer#beginFrame}.</p>
     *
     * <p>After calling this method, the callback associated with <code>buffer</code>
     * will be invoked on the main thread, indicating that the read-back has completed.
     * Typically, this will happen after multiple calls to {@link Renderer#beginFrame},
     * {@link Renderer#render}, {@link Renderer#endFrame}.</p>
     *
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

        int result = nReadPixels(getNativeObject(), mNativeEngine,
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
     * Returns the presentation time of the currently displayed frame in nanosecond.
     *
     * This value can change at any time.
     *
     * @return timestamp in nanosecond.
     */
    public long getTimestamp() {
        return nGetTimestamp(getNativeObject());
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Stream");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeStreamBuilder);
    private static native void nBuilderStreamSource(long nativeStreamBuilder, Object streamSource);
    private static native void nBuilderStream(long nativeStreamBuilder, long externalTextureId);
    private static native void nBuilderWidth(long nativeStreamBuilder, int width);
    private static native void nBuilderHeight(long nativeStreamBuilder, int height);
    private static native long nBuilderBuild(long nativeStreamBuilder, long nativeEngine);

    private static native int nGetStreamType(long nativeStream);
    private static native void nSetDimensions(long nativeStream, int width, int height);
    private static native int nReadPixels(long nativeStream, long nativeEngine,
            int xoffset, int yoffset, int width, int height,
            Buffer storage, int remaining,
            int left, int top, int type, int alignment, int stride, int format,
            Object handler, Runnable callback);
    private static native long nGetTimestamp(long nativeStream);
    private static native void nSetAcquiredImage(long nativeStream, long nativeEngine,
            Object hwbuffer, Object handler, Runnable callback);
}
