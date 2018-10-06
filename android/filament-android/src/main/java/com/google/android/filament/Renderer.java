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

public class Renderer {
    private final Engine mEngine;
    private long mNativeObject;

    public static final int MIRROR_FRAME_FLAG_COMMIT = 0x1;
    public static final int MIRROR_FRAME_FLAG_SET_PRESENTATION_TIME = 0x2;
    public static final int MIRROR_FRAME_FLAG_CLEAR = 0x4;

    Renderer(@NonNull Engine engine, long nativeRenderer) {
        mEngine = engine;
        mNativeObject = nativeRenderer;
    }

    @NonNull
    public Engine getEngine() {
        return mEngine;
    }

    public boolean beginFrame(@NonNull SwapChain swapChain) {
        return nBeginFrame(getNativeObject(), swapChain.getNativeObject());
    }

    public void endFrame() {
        nEndFrame(getNativeObject());
    }

    public void render(@NonNull View view) {
        nRender(getNativeObject(), view.getNativeObject());
    }

    /**
     * This method MUST be called before endFrame.
     */
    public void mirrorFrame(
            @NonNull SwapChain dstSwapChain, @NonNull Viewport dstViewport,
            @NonNull Viewport srcViewport, int flags) {
        nMirrorFrame(getNativeObject(), dstSwapChain.getNativeObject(),
                dstViewport.left, dstViewport.bottom, dstViewport.width, dstViewport.height,
                srcViewport.left, srcViewport.bottom, srcViewport.width, srcViewport.height,
                flags);
    }

    /**
     * This method MUST be called before endFrame.
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

    long getNativeObject() {
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
    private static native void nMirrorFrame(long nativeRenderer, long nativeDstSwapChain,
            int dstLeft, int dstBottom, int dstWidth, int dstHeight,
            int srcLeft, int srcBottom, int srcWidth, int srcHeight,
            int flags);
    private static native int nReadPixels(long nativeRenderer, long nativeEngine,
            int xoffset, int yoffset, int width, int height,
            Buffer storage, int remaining,
            int left, int top, int type, int alignment, int stride, int format,
            Object handler, Runnable callback);
}
