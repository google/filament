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

import android.support.annotation.IntRange;
import android.support.annotation.NonNull;
import android.support.annotation.Size;

public final class MathUtils {
    private MathUtils() { }

    public static void packTangentFrame(
            float tangentX, float tangentY, float tangentZ,
            float bitangentX, float bitangentY, float bitangentZ,
            float normalX, float normalY, float normalZ,
            @NonNull @Size(min = 4) float[] quaternion) {
        nPackTangentFrame(
            tangentX, tangentY, tangentZ,
            bitangentX, bitangentY, bitangentZ,
            normalX, normalY, normalZ, quaternion, 0);
    }

    public static void packTangentFrame(
            float tangentX, float tangentY, float tangentZ,
            float bitangentX, float bitangentY, float bitangentZ,
            float normalX, float normalY, float normalZ,
            @NonNull @Size(min = 4) float[] quaternion, @IntRange(from = 0) int offset) {
        nPackTangentFrame(tangentX, tangentY, tangentZ,
            bitangentX, bitangentY, bitangentZ,
            normalX, normalY, normalZ, quaternion, offset);
    }

    private static native void nPackTangentFrame(
        float tangentX, float tangentY, float tangentZ,
        float bitangentX, float bitangentY, float bitangentZ,
        float normalX, float normalY, float normalZ,
        @NonNull @Size(min = 4) float[] quaternion, @IntRange(from = 0) int offset);
}
