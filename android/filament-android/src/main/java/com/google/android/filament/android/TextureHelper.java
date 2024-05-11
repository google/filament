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

package com.google.android.filament.android;

import android.graphics.Bitmap;
import androidx.annotation.IntRange;
import androidx.annotation.NonNull;

import com.google.android.filament.Engine;
import com.google.android.filament.Texture;

public final class TextureHelper {
    // Keep in sync with Texture.cpp
    private static final int BITMAP_CONFIG_ALPHA_8    = 0;
    private static final int BITMAP_CONFIG_RGB_565    = 1;
    private static final int BITMAP_CONFIG_RGBA_4444  = 2;
    private static final int BITMAP_CONFIG_RGBA_8888  = 3;
    private static final int BITMAP_CONFIG_RGBA_F16   = 4;
    private static final int BITMAP_CONFIG_HARDWARE   = 5;

    private TextureHelper() {
    }

    public static void setBitmap(@NonNull Engine engine,
            @NonNull Texture texture, @IntRange(from = 0) int level, @NonNull Bitmap bitmap) {
        setBitmap(engine, texture,
                level, 0, 0, texture.getWidth(level), texture.getHeight(level), bitmap);
    }

    public static void setBitmap(@NonNull Engine engine,
            @NonNull Texture texture, @IntRange(from = 0) int level, @NonNull Bitmap bitmap,
            Object handler, Runnable callback) {
        setBitmap(engine, texture,
                level, 0, 0, texture.getWidth(level), texture.getHeight(level), bitmap,
                handler, callback);
    }

    public static void setBitmap(@NonNull Engine engine,
            @NonNull Texture texture, @IntRange(from = 0) int level,
            @IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height,
            @NonNull Bitmap bitmap) {

        int format = toNativeFormat(bitmap.getConfig());
        if (format == BITMAP_CONFIG_RGBA_4444 || format == BITMAP_CONFIG_HARDWARE) {
            throw new IllegalArgumentException("Unsupported config: ARGB_4444 or HARDWARE");
        }

        long nativeTexture = texture.getNativeObject();
        long nativeEngine = engine.getNativeObject();
        nSetBitmap(nativeTexture, nativeEngine, level, xoffset, yoffset, width, height,
                bitmap, format);
    }

    public static void setBitmap(@NonNull Engine engine,
            @NonNull Texture texture, @IntRange(from = 0) int level,
            @IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height,
            @NonNull Bitmap bitmap, Object handler, Runnable callback) {

        int format = toNativeFormat(bitmap.getConfig());
        if (format == BITMAP_CONFIG_RGBA_4444 || format == BITMAP_CONFIG_HARDWARE) {
            throw new IllegalArgumentException("Unsupported config: ARGB_4444 or HARDWARE");
        }

        long nativeTexture = texture.getNativeObject();
        long nativeEngine = engine.getNativeObject();
        nSetBitmapWithCallback(nativeTexture, nativeEngine, level, xoffset, yoffset, width, height,
                bitmap, format, handler, callback);
    }

    private static int toNativeFormat(Bitmap.Config config) {
        switch (config) {
            case ALPHA_8:   return BITMAP_CONFIG_ALPHA_8;
            case RGB_565:   return BITMAP_CONFIG_RGB_565;
            case ARGB_4444: return BITMAP_CONFIG_RGBA_4444;
            case ARGB_8888: return BITMAP_CONFIG_RGBA_8888;
            case RGBA_F16:  return BITMAP_CONFIG_RGBA_F16;
            case HARDWARE:  return BITMAP_CONFIG_HARDWARE;
        }
        return BITMAP_CONFIG_RGBA_8888;
    }

    private static native void nSetBitmap(long nativeTexture, long nativeEngine,
            int level, int xoffset, int yoffset, int width, int height, Bitmap bitmap, int format);

    private static native void nSetBitmapWithCallback(long nativeTexture, long nativeEngine,
            int level, int xoffset, int yoffset, int width, int height, Bitmap bitmap, int format,
            Object handler, Runnable callback);
}
