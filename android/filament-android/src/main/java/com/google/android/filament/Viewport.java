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

import androidx.annotation.IntRange;

/**
 * Specifies a rectangular region within a render target in terms of pixel coordinates.
 *
 * <p>
 * The rectangle spans from <code>(left,bottom)</code> to <code>(left+width-1, top+height-1)</code>,
 * inclusive. Width and height must be non-negative.
 * </p>
 *
 * @see View#setViewport
 */
public class Viewport {
    public Viewport(int left, int bottom, @IntRange(from = 0) int width, @IntRange(from = 0) int height) {
        this.left = left;
        this.bottom = bottom;
        this.width = width;
        this.height = height;
    }

    // left coordinate in pixels
    public int left;

    // bottom coordinate in pixels
    public int bottom;

    // width in pixels
    @IntRange(from = 0)
    public int width;

    // height in pixels
    @IntRange(from = 0)
    public int height;
}
