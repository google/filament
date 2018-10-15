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
import android.support.annotation.IntRange;
import android.support.annotation.NonNull;

import com.google.android.filament.Engine;
import com.google.android.filament.Texture;

import java.lang.reflect.Method;

public final class TextureHelper {
    private static final int BITMAP_CONFIG_HARDWARE = 7;

    private static Method sEngineGetNativeObject;
    private static Method sTextureGetNativeObject;

    static {
        try {
            sEngineGetNativeObject = Engine.class.getDeclaredMethod("getNativeObject");
            sTextureGetNativeObject = Texture.class.getDeclaredMethod("getNativeObject");
        } catch (NoSuchMethodException e) {
            // Cannot happen
        }
    }

    private TextureHelper() {
    }

    public static void setBitmap(@NonNull Engine engine,
            @NonNull Texture texture, @IntRange(from = 0) int level, @NonNull Bitmap bitmap) {
        setBitmap(engine, texture,
                level, 0, 0, texture.getWidth(level), texture.getHeight(level), bitmap);
    }

    public static void setBitmap(@NonNull Engine engine,
            @NonNull Texture texture, @IntRange(from = 0) int level,
            @IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height,
            @NonNull Bitmap bitmap) {
        Bitmap.Config config = bitmap.getConfig();
        if (config == Bitmap.Config.ARGB_4444 ||
                config.ordinal() == BITMAP_CONFIG_HARDWARE) {
            throw new IllegalArgumentException("Unsupported config: ARGB_4444 or HARDWARE");
        }

        try {
            long nativeTexture = (Long) sTextureGetNativeObject.invoke(texture);
            long nativeEngine = (Long) sEngineGetNativeObject.invoke(engine);
            nSetBitmap(nativeTexture, nativeEngine, level, xoffset, yoffset, width, height,
                    bitmap, bitmap.getConfig().ordinal());
        } catch (Exception e) {
            // Ignored
        }
    }

    private static native void nSetBitmap(long nativeTexture, long nativeEngine,
            int level, int xoffset, int yoffset, int width, int height, Bitmap bitmap, int format);
}
