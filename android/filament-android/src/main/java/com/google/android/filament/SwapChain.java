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

import android.support.annotation.NonNull;

public class SwapChain {
    private final Object mSurface;
    private long mNativeObject;

    public static final long CONFIG_DEFAULT = 0x0;
    public static final long CONFIG_TRANSPARENT = 0x1;
    public static final long CONFIG_READABLE = 0x2;

    SwapChain(long nativeSwapChain, @NonNull Object surface) {
        mNativeObject = nativeSwapChain;
        mSurface = surface;
    }

    @NonNull
    public Object getNativeWindow() {
        return mSurface;
    }

    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed SwapChain");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }
}
