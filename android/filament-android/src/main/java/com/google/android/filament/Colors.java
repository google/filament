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
import android.support.annotation.Size;

import java.lang.annotation.Retention;
import java.lang.annotation.Target;

import static java.lang.annotation.ElementType.FIELD;
import static java.lang.annotation.ElementType.LOCAL_VARIABLE;
import static java.lang.annotation.ElementType.METHOD;
import static java.lang.annotation.ElementType.PARAMETER;
import static java.lang.annotation.RetentionPolicy.SOURCE;

public class Colors {
    private Colors() {
    }

    @Retention(SOURCE)
    @Target({PARAMETER, METHOD, LOCAL_VARIABLE, FIELD})
    public @interface LinearColor {
    }

    public enum RgbType {
        SRGB,
        LINEAR
    }

    public enum RgbaType {
        SRGB,
        LINEAR,
        PREMULTIPLIED_SRGB,
        PREMULTIPLIED_LINEAR
    }

    public enum Conversion {
        ACCURATE,
        FAST
    }

    @NonNull
    @Size(3)
    @LinearColor
    public static float[] toLinear(@NonNull RgbType type, float r, float g, float b) {
        return toLinear(type, new float[] { r, g, b });
    }

    @NonNull
    @Size(min = 3)
    @LinearColor
    public static float[] toLinear(@NonNull RgbType type, @NonNull @Size(min = 3) float[] rgb) {
        return (type == RgbType.LINEAR) ? rgb : toLinear(Conversion.ACCURATE, rgb);
    }

    @NonNull
    @Size(4)
    @LinearColor
    public static float[] toLinear(@NonNull RgbaType type, float r, float g, float b, float a) {
        return toLinear(type, new float[] { r, g, b, a });
    }

    @NonNull
    @Size(min = 4)
    @LinearColor
    public static float[] toLinear(@NonNull RgbaType type, @NonNull @Size(min = 4) float[] rgba) {
        switch (type) {
            case SRGB:
                toLinear(Conversion.ACCURATE, rgba);
                // fall through
            case LINEAR:
                float a = rgba[3];
                rgba[0] *= a;
                rgba[1] *= a;
                rgba[2] *= a;
                return rgba;
            case PREMULTIPLIED_SRGB:
                return toLinear(Conversion.ACCURATE, rgba);
            case PREMULTIPLIED_LINEAR:
                return rgba;
        }
        return rgba;
    }

    @NonNull
    @LinearColor
    public static float[] toLinear(@NonNull Conversion conversion, @NonNull @Size(min = 3) float[] rgb) {
        switch (conversion) {
            case ACCURATE:
                for (int i = 0; i < 3; i++) {
                    rgb[i] = (rgb[i] <= 0.04045f) ?
                            rgb[i] / 12.92f : (float) Math.pow((rgb[i] + 0.055f) / 1.055f, 2.4f);
                }
                break;
            case FAST:
                for (int i = 0; i < 3; i++) {
                    rgb[i] = (float) Math.sqrt(rgb[i]);
                }
                break;
        }
        return rgb;
    }

    @NonNull
    @Size(3)
    @LinearColor
    public static float[] cct(float temperature) {
        float[] color = new float[3];
        nCct(temperature, color);
        return color;
    }

    @NonNull
    @Size(3)
    @LinearColor
    public static float[] illuminantD(float temperature) {
        float[] color = new float[3];
        nIlluminantD(temperature, color);
        return color;
    }

    private static native void nCct(float temperature, @NonNull @Size(3) float[] color);
    private static native void nIlluminantD(float temperature, @NonNull @Size(3) float[] color);
}
