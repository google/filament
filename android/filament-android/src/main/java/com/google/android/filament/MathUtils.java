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

    /**
     * Packs the tangent frame represented by the specified tangent, bitangent, and normal into a
     * quaternion.
     *
     * <p>
     * Reflection is preserved by encoding it as the sign of the w component in the resulting
     * quaternion. Since -0 cannot always be represented on the GPU, this function computes a bias
     * to ensure values are always either positive or negative, never 0. The bias is computed based
     * on a per-element storage size of 2 bytes, making the resulting quaternion suitable for
     * storage into an SNORM16 vector.
     * </p>
     *
     * @param tangentX   the X component of the tangent
     * @param tangentY   the Y component of the tangent
     * @param tangentZ   the Z component of the tangent
     * @param bitangentX the X component of the bitangent
     * @param bitangentY the Y component of the bitangent
     * @param bitangentZ the Z component of the bitangent
     * @param normalX    the X component of the normal
     * @param normalY    the Y component of the normal
     * @param normalZ    the Z component of the normal
     * @param quaternion a float array of at least size 4 for the quaternion result to be stored
     */
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

    /**
     * Packs the tangent frame represented by the specified tangent, bitangent, and normal into a
     * quaternion.
     *
     * <p>
     * Reflection is preserved by encoding it as the sign of the w component in the resulting
     * quaternion. Since -0 cannot always be represented on the GPU, this function computes a bias
     * to ensure values are always either positive or negative, never 0. The bias is computed based
     * on a per-element storage size of 2 bytes, making the resulting quaternion suitable for
     * storage into an SNORM16 vector.
     * </p>
     *
     * @param tangentX   the X component of the tangent
     * @param tangentY   the Y component of the tangent
     * @param tangentZ   the Z component of the tangent
     * @param bitangentX the X component of the bitangent
     * @param bitangentY the Y component of the bitangent
     * @param bitangentZ the Z component of the bitangent
     * @param normalX    the X component of the normal
     * @param normalY    the Y component of the normal
     * @param normalZ    the Z component of the normal
     * @param quaternion a float array of at least size 4 for the quaternion result to be stored
     * @param offset     offset, in elements, into the quaternion array to store the results
     */
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
