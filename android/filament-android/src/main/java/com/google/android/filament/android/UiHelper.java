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

import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.os.Build;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;

import com.google.android.filament.SwapChain;

/**
 * UiHelper is a simple class that can manage either a SurfaceView, TextureView, or a SurfaceHolder
 * so it can be used to render into with Filament.
 *
 * Here is a simple example with a SurfaceView. The code would be exactly the same with a
 * TextureView:
 *
 * <pre>
 * public class FilamentActivity extends Activity {
 *     private UiHelper mUiHelper;
 *     private SurfaceView mSurfaceView;
 *
 *     // Filament specific APIs
 *     private Engine mEngine;
 *     private Renderer mRenderer;
 *     private View mView; // com.google.android.filament.View, not android.view.View
 *     private SwapChain mSwapChain;
 *
 *     public void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *
 *         // Create a SurfaceView and add it to the activity
 *         mSurfaceView = new SurfaceView(this);
 *         setContentView(mSurfaceView);
 *
 *         // Create the Filament UI helper
 *         mUiHelper = new UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK);
 *
 *         // Attach the SurfaceView to the helper, you could do the same with a TextureView
 *         mUiHelper.attachTo(mSurfaceView);
 *
 *         // Set a rendering callback that we will use to invoke Filament
 *         mUiHelper.setRenderCallback(new UiHelper.RendererCallback() {
 *             public void onNativeWindowChanged(Surface surface) {
 *                 if (mSwapChain != null) mEngine.destroySwapChain(mSwapChain);
 *                 mSwapChain = mEngine.createSwapChain(surface, mUiHelper.getSwapChainFlags());
 *             }
 *
 *             // The native surface went away, we must stop rendering.
 *             public void onDetachedFromSurface() {
 *                 if (mSwapChain != null) {
 *                     mEngine.destroySwapChain(mSwapChain);
 *
 *                     // Required to ensure we don't return before Filament is done executing the
 *                     // destroySwapChain command, otherwise Android might destroy the Surface
 *                     // too early
 *                     mEngine.flushAndWait();
 *
 *                     mSwapChain = null;
 *                 }
 *             }
 *
 *             // The native surface has changed size. This is always called at least once
 *             // after the surface is created (after onNativeWindowChanged() is invoked).
 *             public void onResized(int width, int height) {
 *
 *                 // Wait for all pending frames to be processed before returning. This is to
 *                 // avoid a race between the surface being resized before pending frames are
 *                 // rendered into it.
 *                 Fence fence = mEngine.createFence();
 *                 fence.wait(Fence.Mode.FLUSH, Fence.WAIT_FOR_EVER);
 *                 mEngine.destroyFence(fence);
 *
 *                 // Compute camera projection and set the viewport on the view
 *             }
 *         });
 *
 *         mEngine = Engine.create();
 *         mRenderer = mEngine.createRenderer();
 *         mView = mEngine.createView();
 *         // Create scene, camera, etc.
 *     }
 *
 *     public void onDestroy() {
 *         super.onDestroy();
 *         // Always detach the surface before destroying the engine
 *         mUiHelper.detach();
 *
 *         mEngine.destroy();
 *     }
 *
 *     // This is an example of a render function. You will most likely invoke this from
 *     // a Choreographer callback to trigger rendering at vsync.
 *     public void render() {
 *         if (mUiHelper.isReadyToRender) {
 *             // If beginFrame() returns false you should skip the frame
 *             // This means you are sending frames too quickly to the GPU
 *             if (mRenderer.beginFrame(swapChain)) {
 *                 mRenderer.render(mView);
 *                 mRenderer.endFrame();
 *             }
 *         }
 *     }
 * }
 * </pre>
 */
public class UiHelper {
    private static final String LOG_TAG = "UiHelper";
    private static final boolean LOGGING = false;

    private int mDesiredWidth;
    private int mDesiredHeight;
    private Object mNativeWindow;

    private RendererCallback mRenderCallback;
    private boolean mHasSwapChain;

    private RenderSurface mRenderSurface;

    private boolean mOpaque = true;
    private boolean mOverlay = false;

    /**
     * Enum used to decide whether UiHelper should perform extra error checking.
     *
     * @see UiHelper#UiHelper(ContextErrorPolicy)
     */
    public enum ContextErrorPolicy {
        /** Check for extra errors. */
        CHECK,
        /** Do not check for extra errors. */
        DONT_CHECK
    }

    /**
     * Interface used to know when the native surface is created, destroyed or resized.
     *
     * @see #setRenderCallback(RendererCallback)
     */
    public interface RendererCallback {
        /**
         * Called when the underlying native window has changed.
         */
        void onNativeWindowChanged(Surface surface);

        /**
         * Called when the surface is going away. After this call <code>isReadyToRender()</code>
         * returns false. You MUST have stopped drawing when returning.
         * This is called from detach() or if the surface disappears on its own.
         */
        void onDetachedFromSurface();

        /**
         * Called when the underlying native window has been resized.
         */
        void onResized(int width, int height);
    }

    private interface RenderSurface {
        void resize(int width, int height);
        void detach();
    }

    private static class SurfaceViewHandler implements RenderSurface {
        private final SurfaceView mSurfaceView;

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

    private static class SurfaceHolderHandler implements RenderSurface {
        private final SurfaceHolder mSurfaceHolder;

        SurfaceHolderHandler(SurfaceHolder surface) {
            mSurfaceHolder = surface;
        }

        @Override
        public void resize(int width, int height) {
            mSurfaceHolder.setFixedSize(width, height);
        }

        @Override
        public void detach() {
        }
    }

    private class TextureViewHandler implements RenderSurface {
        private final TextureView mTextureView;
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

        public Surface getSurface() {
            return mSurface;
        }
    }

    /**
     *  Creates a UiHelper which will help manage the native surface provided by a
     *  SurfaceView or a TextureView.
     */
    public UiHelper() {
        this(ContextErrorPolicy.CHECK);
    }

    /**
     * Creates a UiHelper which will help manage the native surface provided by a
     * SurfaceView or a TextureView.
     *
     * @param policy The error checking policy to use.
     */
    public UiHelper(ContextErrorPolicy policy) {
        // TODO: do something with policy
    }

    /**
     * Sets the renderer callback that will be notified when the native surface is
     * created, destroyed or resized.
     *
     * @param renderCallback The callback to register.
     */
    public void setRenderCallback(@Nullable RendererCallback renderCallback) {
        mRenderCallback = renderCallback;
    }

    /**
     * Returns the current render callback associated with this UiHelper.
     */
    @Nullable
    public RendererCallback getRenderCallback() {
        return mRenderCallback;
    }

    /**
     * Free resources associated to the native window specified in {@link #attachTo(SurfaceView)},
     * {@link #attachTo(TextureView)}, or {@link #attachTo(SurfaceHolder)}.
     */
    public void detach() {
        destroySwapChain();
        mNativeWindow = null;
        mRenderSurface = null;
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
     * Set the size of the render target buffers of the native surface.
     */
    public void setDesiredSize(int width, int height) {
        mDesiredWidth = width;
        mDesiredHeight = height;
        if (mRenderSurface != null) {
            mRenderSurface.resize(width, height);
        }
    }

    /**
     * Returns the requested width for the native surface.
     */
    public int getDesiredWidth() {
        return mDesiredWidth;
    }

    /**
     * Returns the requested height for the native surface.
     */
    public int getDesiredHeight() {
        return mDesiredHeight;
    }

    /**
     * Returns true if the render target is opaque.
     */
    public boolean isOpaque() {
        return mOpaque;
    }

    /**
     * Controls whether the render target (SurfaceView or TextureView) is opaque or not.
     * The render target is considered opaque by default.
     *
     * Must be called before calling {@link #attachTo(SurfaceView)}, {@link #attachTo(TextureView)},
     * or {@link #attachTo(SurfaceHolder)}.
     *
     * @param opaque Indicates whether the render target should be opaque. True by default.
     */
    public void setOpaque(boolean opaque) {
        mOpaque = opaque;
    }

    /**
     * Returns true if the SurfaceView used as a render target should be positioned above
     * other surfaces but below the activity's surface. False by default.
     */
    public boolean isMediaOverlay() {
        return mOverlay;
    }

    /**
     * Controls whether the surface of the SurfaceView used as a render target should be
     * positioned above other surfaces but below the activity's surface. This property
     * only has an effect when used in combination with {@link #setOpaque(boolean) setOpaque(false)}
     * and does not affect TextureView targets.
     *
     * Must be called before calling {@link #attachTo(SurfaceView)}
     * or {@link #attachTo(TextureView)}.
     *
     * Has no effect when using {@link #attachTo(SurfaceHolder)}.
     *
     * @param overlay Indicates whether the render target should be rendered below the activity's
     *                surface when transparent.
     */
    public void setMediaOverlay(boolean overlay) {
        mOverlay = overlay;
    }

    /**
     * Returns the flags to pass to
     * {@link com.google.android.filament.Engine#createSwapChain(Object, long)} to honor all
     * the options set on this UiHelper.
     */
    public long getSwapChainFlags() {
        return isOpaque() ? SwapChain.CONFIG_DEFAULT : SwapChain.CONFIG_TRANSPARENT;
    }

    /**
     * Associate UiHelper with a SurfaceView.
     *
     * As soon as SurfaceView is ready (i.e. has a Surface), we'll create the
     * EGL resources needed, and call user callbacks if needed.
     */
    public void attachTo(@NonNull SurfaceView view) {
        if (attach(view)) {
            boolean translucent = !isOpaque();
            // setZOrderOnTop() and setZOrderMediaOverlay() override each other,
            // we must only call one of them
            if (isMediaOverlay()) {
                view.setZOrderMediaOverlay(translucent);
            } else {
                view.setZOrderOnTop(translucent);
            }

            int format = isOpaque() ? PixelFormat.OPAQUE : PixelFormat.TRANSLUCENT;
            view.getHolder().setFormat(format);

            mRenderSurface = new SurfaceViewHandler(view);

            final SurfaceHolder.Callback callback = new SurfaceHolder.Callback() {
                @Override
                public void surfaceCreated(SurfaceHolder holder) {
                    if (LOGGING) Log.d(LOG_TAG, "surfaceCreated()");
                    createSwapChain(holder.getSurface());
                }

                @Override
                public void surfaceChanged(
                        SurfaceHolder holder, int format, int width, int height) {
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

            SurfaceHolder holder = view.getHolder();
            holder.addCallback(callback);
            if (mDesiredWidth > 0 && mDesiredHeight > 0) {
                holder.setFixedSize(mDesiredWidth, mDesiredHeight);
            }

            // in case the SurfaceView's surface already existed
            final Surface surface = holder.getSurface();
            if (surface != null && surface.isValid()) {
                callback.surfaceCreated(holder);
                callback.surfaceChanged(holder, format,
                        holder.getSurfaceFrame().width(), holder.getSurfaceFrame().height());
            }
        }
    }

    /**
     * Associate UiHelper with a TextureView.
     *
     * As soon as TextureView is ready (i.e. has a buffer), we'll create the
     * EGL resources needed, and call user callbacks if needed.
     */
    public void attachTo(@NonNull TextureView view) {
        if (attach(view)) {
            view.setOpaque(isOpaque());

            mRenderSurface = new TextureViewHandler(view);

            TextureView.SurfaceTextureListener listener = new TextureView.SurfaceTextureListener() {
                @Override
                public void onSurfaceTextureAvailable(
                        SurfaceTexture surfaceTexture, int width, int height) {
                    if (LOGGING) Log.d(LOG_TAG, "onSurfaceTextureAvailable()");

                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1) {
                        if (mDesiredWidth > 0 && mDesiredHeight > 0) {
                            surfaceTexture.setDefaultBufferSize(mDesiredWidth, mDesiredHeight);
                        }
                    }

                    Surface surface = new Surface(surfaceTexture);
                    TextureViewHandler textureViewHandler = (TextureViewHandler) mRenderSurface;
                    textureViewHandler.setSurface(surface);

                    createSwapChain(surface);

                    // Call this the first time because onSurfaceTextureSizeChanged()
                    // isn't called at initialization time
                    mRenderCallback.onResized(width, height);
                }

                @Override
                public void onSurfaceTextureSizeChanged(
                        SurfaceTexture surfaceTexture, int width, int height) {
                    if (LOGGING) Log.d(LOG_TAG, "onSurfaceTextureSizeChanged()");
                    if (mDesiredWidth > 0 && mDesiredHeight > 0) {
                        surfaceTexture.setDefaultBufferSize(mDesiredWidth, mDesiredHeight);
                        mRenderCallback.onResized(mDesiredWidth, mDesiredHeight);
                    } else {
                        mRenderCallback.onResized(width, height);
                    }
                    // We must recreate the SwapChain to guarantee that it sees the new size.
                    // More precisely, for an EGL client, the EGLSurface must be recreated. For
                    // a Vulkan client, the SwapChain must be recreated. Calling
                    // onNativeWindowChanged() will accomplish that.
                    // This requirement comes from SurfaceTexture.setDefaultBufferSize()
                    // documentation.
                    TextureViewHandler textureViewHandler = (TextureViewHandler) mRenderSurface;
                    mRenderCallback.onNativeWindowChanged(textureViewHandler.getSurface());
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

            view.setSurfaceTextureListener(listener);

            // in case the View's SurfaceTexture already existed
            if (view.isAvailable()) {
                SurfaceTexture surfaceTexture = view.getSurfaceTexture();
                listener.onSurfaceTextureAvailable(surfaceTexture, mDesiredWidth, mDesiredHeight);
            }
        }
    }

    /**
     * Associate UiHelper with a SurfaceHolder.
     *
     * As soon as a Surface is created, we'll create the
     * EGL resources needed, and call user callbacks if needed.
     */
    public void attachTo(@NonNull SurfaceHolder holder) {
        if (attach(holder)) {
            int format = isOpaque() ? PixelFormat.OPAQUE : PixelFormat.TRANSLUCENT;
            holder.setFormat(format);

            mRenderSurface = new SurfaceHolderHandler(holder);

            final SurfaceHolder.Callback callback = new SurfaceHolder.Callback() {
                @Override
                public void surfaceCreated(SurfaceHolder surfaceHolder) {
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
                public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
                    if (LOGGING) Log.d(LOG_TAG, "surfaceDestroyed()");
                    destroySwapChain();
                }
            };

            holder.addCallback(callback);
            if (mDesiredWidth > 0 && mDesiredHeight > 0) {
                holder.setFixedSize(mDesiredWidth, mDesiredHeight);
            }

            // in case the SurfaceHolder's surface already existed
            final Surface surface = holder.getSurface();
            if (surface != null && surface.isValid()) {
                callback.surfaceCreated(holder);
                callback.surfaceChanged(holder, format,
                        holder.getSurfaceFrame().width(), holder.getSurfaceFrame().height());
            }
        }
    }

    private boolean attach(@NonNull Object nativeWindow) {
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

    private void createSwapChain(@NonNull Surface surface) {
        mRenderCallback.onNativeWindowChanged(surface);
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
