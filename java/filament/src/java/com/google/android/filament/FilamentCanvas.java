/*
 * Copyright (C) 2018 The Android Open Source Project
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

import java.awt.Canvas;
import java.awt.Dimension;
import java.awt.Graphics;
import com.google.android.filament.Engine;
import com.google.android.filament.SwapChain;
import com.google.android.filament.View;
import com.google.android.filament.Renderer;
import androidx.annotation.NonNull;

public class FilamentCanvas extends Canvas implements FilamentTarget  {
    private SwapChain mSwapChain;
    private boolean mHasPainted;

    public FilamentCanvas() {
        super();
        mHasPainted = false;
    }

    @Override
    public void paint(@NonNull Graphics g) {
        // On windows, allowing paint more than once will lock the canvas filament renderer.
        // Not allowing it at all will trigger weird behavior where maximizing and returning
        // to previous window size prevent SwapBuffer from working properly. super.paint() must
        // be allowed exactly once.
        if (!mHasPainted) {
            super.paint(g);
            mHasPainted = true;
        }
    }

    public boolean beginFrame(@NonNull Engine engine, @NonNull Renderer renderer) {
        return renderer.beginFrame(getSwapChain(engine));
    }

    public void endFrame(Renderer renderer) {
        renderer.endFrame();
    }

    @NonNull
    private SwapChain getSwapChain(@NonNull Engine engine) {
        if (mSwapChain == null) {
            mSwapChain = engine.createSwapChain(this, 0);
        }
        return mSwapChain;
    }

    public void destroy(@NonNull Engine engine) {
        if (mSwapChain == null) {
            return;
        }
        engine.destroySwapChain(mSwapChain);
        mSwapChain = null;
    }
}
