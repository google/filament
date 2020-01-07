package com.google.android.filament;
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

import androidx.annotation.NonNull;

public class Fence {
    private long mNativeObject;

    Fence(long nativeFence) {
        mNativeObject = nativeFence;
    }

    public static final long WAIT_FOR_EVER = -1;

    public enum Mode {
        FLUSH,
        DONT_FLUSH
    }

    public enum FenceStatus {
        ERROR,
        CONDITION_SATISFIED,
        TIMEOUT_EXPIRED
    }

    /**
     * Blocks the current thread until the Fence signals.
     */
    public FenceStatus wait(@NonNull Mode mode, long timeoutNanoSeconds) {
        int nativeResult = nWait(getNativeObject(), mode.ordinal(), timeoutNanoSeconds);
        switch (nativeResult) {
            case -1:
                return FenceStatus.ERROR;
            case 0:
                return FenceStatus.CONDITION_SATISFIED;
            case 1:
                return FenceStatus.TIMEOUT_EXPIRED;
            default:
                // this should never happend
                return FenceStatus.ERROR;
        }
    }

    public static FenceStatus waitAndDestroy(@NonNull Fence fence, @NonNull Mode mode) {
        int nativeResult = nWaitAndDestroy(fence.getNativeObject(), mode.ordinal());
        switch (nativeResult) {
            case -1:
                return FenceStatus.ERROR;
            case 0:
                return FenceStatus.CONDITION_SATISFIED;
            default:
                // this should never happen
                return FenceStatus.ERROR;
        }
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Fence");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native int nWait(long nativeFence, int mode, long timeoutNanoSeconds);
    private static native int nWaitAndDestroy(long nativeFence, int mode);
}
