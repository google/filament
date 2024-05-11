/*
 * Copyright (C) 2023 The Android Open Source Project
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

import com.google.android.filament.Engine;
import com.google.android.filament.Fence;

public class FilamentHelper {

    /**
     * Wait for all pending frames to be processed before returning. This is to avoid a race
     * between the surface being resized before pending frames are rendered into it.
     * <p>
     * For {@link android.view.TextureView} this must be called before the texture's size is
     * reconfigured, which unfortunately is done by the Android framework before
     * {@link UiHelper} listeners are invoked. Therefore <code>synchronizePendingFrames</code>
     * cannot be called from
     * {@link android.view.TextureView.SurfaceTextureListener#onSurfaceTextureSizeChanged}; instead
     * a subclass of {@link android.view.TextureView} must be used in order to call it from
     * {@link android.view.TextureView#onSizeChanged}:
     * </p>
     * <pre>
     * public class MyTextureView extends TextureView {
     *     private Engine engine;
     *     protected void onSizeChanged(int w, int h, int oldw, int oldh) {
     *         FilamentHelper.synchronizePendingFrames(engine);
     *         super.onSizeChanged(w, h, oldw, oldh);
     *     }
     * }
     * </pre>
     *
     * Otherwise, this is typically called from {@link UiHelper.RendererCallback#onResized},
     * {@link android.view.SurfaceHolder.Callback#surfaceChanged}.
     *
     * @param engine Filament engine to synchronize
     *
     * @see UiHelper.RendererCallback#onResized
     * @see android.view.SurfaceHolder.Callback#surfaceChanged
     * @see android.view.TextureView#onSizeChanged
     */
    static public void synchronizePendingFrames(Engine engine) {
        Fence fence = engine.createFence();
        fence.wait(Fence.Mode.FLUSH, Fence.WAIT_FOR_EVER);
        engine.destroyFence(fence);
    }
}
