/*
 * Copyright (C) 2026 The Android Open Source Project
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

package com.google.android.filament.utils;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.graphics.Bitmap;

public class ImageDiff {
    public enum Mode {
        LEAF, AND, OR
    }

    public enum Swizzle {
        RGBA, BGRA
    }

    public static class Config {
        @NonNull
        public Mode mode = Mode.LEAF;
        @NonNull
        public Swizzle swizzle = Swizzle.RGBA;
        public int channelMask = 0xF;
        public float maxAbsDiff = 0.0f;
        public float maxFailingPixelsFraction = 0.0f;
        // Children not supported in this simple wrapper for now, can be added if needed
    }

    public static class Result {
        public enum Status {
            PASSED,
            SIZE_MISMATCH,
            PIXEL_DIFFERENCE
        }

        public Status status;
        public long failingPixelCount;
        public float[] maxDiffFound; // [R, G, B, A]
        public Bitmap diffImage; // Null if not generated
    }

    /**
     * Compares two bitmaps using a configuration object.
     *
     * @param reference Golden image
     * @param candidate Actual image
     * @param config    Comparison configuration
     * @param mask      Optional mask (grayscale)
     * @return Result of comparison
     */
    @NonNull
    public static Result compareBasic(@NonNull Bitmap reference, @NonNull Bitmap candidate,
            @NonNull Config config, @Nullable Bitmap mask) {
        return nCompareBasic(reference, candidate, config.mode.ordinal(), config.swizzle.ordinal(),
                config.channelMask, config.maxAbsDiff, config.maxFailingPixelsFraction, mask);
    }

    /**
     * Compares two bitmaps using a JSON configuration string.
     *
     * @param reference Golden image
     * @param candidate Actual image
     * @param jsonConfig Comparison configuration in JSON format
     * @param mask      Optional mask (grayscale)
     * @return Result of comparison
     */
    @NonNull
    public static Result compare(@NonNull Bitmap reference, @NonNull Bitmap candidate,
            @NonNull String jsonConfig, @Nullable Bitmap mask) {
        return nCompareJson(reference, candidate, jsonConfig, mask);
    }

    private static native Result nCompareBasic(Bitmap reference, Bitmap candidate, int mode, int swizzle,
            int channelMask, float maxAbsDiff, float maxFailingPixelsFraction, Bitmap mask);

    private static native Result nCompareJson(Bitmap reference, Bitmap candidate, String jsonConfig, Bitmap mask);
}
