/*
 * Copyright (C) 2019 The Android Open Source Project
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
import android.support.annotation.Nullable;
import android.support.annotation.Size;

final class Asserts {
    private Asserts() {
    }

    @NonNull @Size(min = 9)
    static float[] assertMat3f(@Nullable float[] out) {
        if (out == null) out = new float[9];
        else if (out.length < 9) {
            throw new ArrayIndexOutOfBoundsException("Array length must be at least 9");
        }
        return out;
    }

    static void assertMat3fIn(@NonNull @Size(min = 9) float[] in) {
        if (in.length < 9) {
            throw new ArrayIndexOutOfBoundsException("Array length must be at least 9");
        }
    }

    @NonNull @Size(min = 16)
    static double[] assertMat4d(@Nullable double[] out) {
        if (out == null) out = new double[16];
        else if (out.length < 16) {
            throw new ArrayIndexOutOfBoundsException("Array length must be at least 16");
        }
        return out;
    }

    static void assertMat4dIn(@NonNull @Size(min = 16) double[] in) {
        if (in.length < 16) {
            throw new ArrayIndexOutOfBoundsException("Array length must be at least 16");
        }
    }

    @NonNull @Size(min = 16)
    static float[] assertMat4f(@Nullable float[] out) {
        if (out == null) out = new float[16];
        else if (out.length < 16) {
            throw new ArrayIndexOutOfBoundsException("Array length must be at least 16");
        }
        return out;
    }

    static void assertMat4fIn(@NonNull @Size(min = 16) float[] in) {
        if (in.length < 16) {
            throw new ArrayIndexOutOfBoundsException("Array length must be at least 16");
        }
    }

    @NonNull @Size(min = 3)
    static float[] assertFloat3(@Nullable float[] out) {
        if (out == null) out = new float[3];
        else if (out.length < 3) {
            throw new ArrayIndexOutOfBoundsException("Array length must be at least 3");
        }
        return out;
    }

    @NonNull @Size(min = 4)
    static float[] assertFloat4(@Nullable float[] out) {
        if (out == null) out = new float[4];
        else if (out.length < 4) {
            throw new ArrayIndexOutOfBoundsException("Array length must be at least 4");
        }
        return out;
    }
}
