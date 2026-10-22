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

public class NativeSurface {
    private final int mWidth;
    private final int mHeight;
    private final long mNativeObject;

    public NativeSurface(int width, int height) {
        mWidth = width;
        mHeight = height;
        mNativeObject = nCreateSurface(width, height);
    }

    public void dispose() {
        nDestroySurface(mNativeObject);
    }

    public int getWidth() {
        return mWidth;
    }

    public int getHeight() {
        return mHeight;
    }

    public long getNativeObject() {
        return mNativeObject;
    }

    private static native long nCreateSurface(int width, int height);
    private static native void nDestroySurface(long nativeObject);
}