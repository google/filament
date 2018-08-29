/*
 * Copyright (C) 2015 The Android Open Source Project
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

package com.google.android.filament.android;

import android.graphics.SurfaceTexture;

import android.os.Build;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;

public class UiHelper {
    private static final String LOG_TAG = "UiHelper";
    private static final boolean LOGGING = false;

    private int mDesiredWidth;
    private int mDesiredHeight;
    private Object mNativeWindow;

    private RendererCallback mRenderCallback;
    private boolean mHasSwapChain;

    public enum ContextErrorPolicy {
        CHECK, DONT_CHECK
    }

    public interface RendererCallback {

        // called when the underlying native window has changed
        // NOTE: this could be called from UiHelper's constructor.
        void onNativeWindowChanged(Surface surface);

        // called when the surface is going away. after this call isReadyToRender() returns false.
        // you MUST have stopped drawing when returning.
        // This is called from detach() or if the surface disappears on its own.
        void onDetachedFromSurface();

        // called when the underlying native window has been resized
        void onResized(int width, int height);
    }

    // --------------------------------------------------------------------------------------------

    private interface RenderSurface {
        void resize(int width, int height);
        void detach();
    }

    private class SurfaceViewHandler implements RenderSurface {
        private SurfaceView mSurfaceView;

        SurfaceViewHandler(SurfaceView surface) {
            mSurfaceView = surface;
        }

        @Override
        public void resize(int width, int height) {
            mSurfaceView.getHolder().setFixedSize(width, height);
        }

        @Override
        public void detach() {
        }
    }

    private class TextureViewHandler implements RenderSurface {
        private TextureView mTextureView;
        private Surface mSurface;

        TextureViewHandler(TextureView surface) { mTextureView = surface; }

        @Override
        public void resize(int width, int height) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1) {
                mTextureView.getSurfaceTexture().setDefaultBufferSize(width, height);
            }
            // the call above won't cause TextureView.onSurfaceTextureSizeChanged()
            mRenderCallback.onResized(width, height);
        }

        @Override
        public void detach() {
            setSurface(null);
        }

        void setSurface(Surface surface) {
            if (surface == null) {
                if (mSurface != null) {
                    mSurface.release();
                }
            }
            mSurface = surface;
        }
    }
    // --------------------------------------------------------------------------------------------

    private RenderSurface mRenderSurface;

    /**
     *  Creates a UiHelper which will help manage the EGL context
     *  and EGL surfaces. UiHelper handles SurfaceView and TextureView.
     *
     *  When this call returns, OpenGL ES is ready to use.
     */
    public UiHelper() {
        this(ContextErrorPolicy.CHECK);
    }

    public UiHelper(ContextErrorPolicy policy) {
        // TODO: do something with policy
    }

    public UiHelper(int sampleCount, RendererCallback renderCallback) {
        this();
        setRenderCallback(renderCallback);
    }

    public void setRenderCallback(RendererCallback renderCallback) {
        mRenderCallback = renderCallback;
    }

    public RendererCallback getRenderCallback() {
        return mRenderCallback;
    }

    /**
     * Free resources associated to the native window specified in attachTo().
     */
    public void detach() {
        destroySwapChain();
        mNativeWindow = null;
        mRenderSurface = null;
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        // TODO: check that detach() has been called
        // TODO: call Surface.release() if not already done
    }

    /**
     * Checks whether we are ready to render into the attached surface.
     *
     * Using OpenGL ES when this returns true, will result in drawing commands being lost,
     * HOWEVER, GLES state will be preserved. This is useful to initialize the engine.
     *
     * @return true: rendering is possible, false: rendering is not possible.
     */
    public boolean isReadyToRender() {
        return mHasSwapChain;
    }

    /**
     * Set the size of the underlying buffers
     */
    public void setDesiredSize(int width, int height) {
        mDesiredWidth = width;
        mDesiredHeight = height;
        if (mRenderSurface != null) {
            mRenderSurface.resize(width, height);
        }
    }

    public int getDesiredWidth() {
        return mDesiredWidth;
    }

    public int getDesiredHeight() {
        return mDesiredHeight;
    }

    /**
     * Associate UiHelper with a SurfaceView.
     *
     * As soon as SurfaceView is ready (i.e. has a Surface), we'll create the
     * EGL resources needed, and call user callbacks if needed.
     */
    public void attachTo(SurfaceView view) {
        final SurfaceHolder.Callback callback = new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                if (LOGGING) Log.d(LOG_TAG, "surfaceCreated()");
                createSwapChain(holder.getSurface());
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                // Note: this is always called at least once after surfaceCreated()
                if (LOGGING) Log.d(LOG_TAG, "surfaceChanged(" + width + ", " + height + ")");
                mRenderCallback.onResized(width, height);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                if (LOGGING) Log.d(LOG_TAG, "surfaceDestroyed()");
                destroySwapChain();
            }
        };

        if (attach(view)) {
            mRenderSurface = new SurfaceViewHandler(view);
            SurfaceHolder holder = view.getHolder();
            holder.addCallback(callback);
            holder.setFixedSize(mDesiredWidth, mDesiredHeight);
            // in case the SurfaceView's surface already existed
            final Surface surface = holder.getSurface();
            if (surface != null && surface.isValid()) {
                callback.surfaceCreated(holder);
            }
        }
    }

    /**
     * Associate UiHelper with a TextureView.
     *
     * As soon as TextureView is ready (i.e. has a buffer), we'll create the
     * EGL resources needed, and call user callbacks if needed.
     */
    public void attachTo(TextureView view) {
        final TextureView.SurfaceTextureListener listener = new TextureView.SurfaceTextureListener() {
            @Override
            public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
                if (LOGGING) Log.d(LOG_TAG, "onSurfaceTextureAvailable()");
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1) {
                    surfaceTexture.setDefaultBufferSize(mDesiredWidth, mDesiredHeight);
                }
                Surface surface = new Surface(surfaceTexture);
                TextureViewHandler textureViewHandler = (TextureViewHandler)mRenderSurface;
                textureViewHandler.setSurface(surface);
                createSwapChain(surface);
            }

            @Override
            public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {
                if (LOGGING) Log.d(LOG_TAG, "onSurfaceTextureSizeChanged()");
                mRenderCallback.onResized(width, height);
            }

            @Override
            public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
                if (LOGGING) Log.d(LOG_TAG, "onSurfaceTextureDestroyed()");
                destroySwapChain();
                return true;
            }

            @Override
            public void onSurfaceTextureUpdated(SurfaceTexture surface) { }
        };

        if (attach(view)) {
            mRenderSurface = new TextureViewHandler(view);
            view.setSurfaceTextureListener(listener);
            // in case the View's SurfaceTexture already existed
            if (view.isAvailable()) {
                final SurfaceTexture surfaceTexture = view.getSurfaceTexture();
                listener.onSurfaceTextureAvailable(surfaceTexture, mDesiredWidth, mDesiredHeight);
            }
        }
    }

    private boolean attach(Object nativeWindow) {
        if (mNativeWindow != null) {
            // we are already attached to a native window
            if (mNativeWindow == nativeWindow) {
                // nothing to do
                return false;
            }
            destroySwapChain();
        }
        mNativeWindow = nativeWindow;
        return true;
    }

    private void createSwapChain(Surface sur) {
        mRenderCallback.onNativeWindowChanged(sur);
        mHasSwapChain = true;
    }

    private void destroySwapChain() {
        if (mRenderSurface != null) {
            mRenderSurface.detach();
        }
        mRenderCallback.onDetachedFromSurface();
        mHasSwapChain = false;
    }
}
