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

/**
 * Utilities to manipulate and convert colors.
 */
public class Colors {
    private Colors() {
    }

    @Retention(SOURCE)
    @Target({PARAMETER, METHOD, LOCAL_VARIABLE, FIELD})
    public @interface LinearColor {
    }

    /**
     * Types of RGB colors.
     */
    public enum RgbType {
        /** The color is defined in sRGB space. */
        SRGB,

        /** The color is defined in linear space. */
        LINEAR
    }

    /**
     * Types of RGBA colors.
     */
    public enum RgbaType {
        /**
         * The color is defined in sRGB space and the RGB values have not been premultiplied by the
         * alpha (for instance, a 50% transparent red is <1,0,0,0.5>).
         */
        SRGB,

        /**
         * The color is defined in linear space and the RGB values have not been premultiplied by
         * the alpha (for instance, a 50% transparent red is <1,0,0,0.5>).
         */
        LINEAR,

        /**
         * The color is defined in sRGB space and the RGB values have been premultiplied by the
         * alpha (for instance, a 50% transparent red is <0.5,0,0,0.5>).
         */
        PREMULTIPLIED_SRGB,

        /**
         * The color is defined in linear space and the RGB values have been premultiplied by the
         * alpha (for instance, a 50% transparent red is <0.5,0,0,0.5>).
         */
        PREMULTIPLIED_LINEAR
    }

    /**
     * Type of color conversion to use when converting to/from sRGB and linear spaces.
     */
    public enum Conversion {
        /** Accurate conversion using the sRGB standard. */
        ACCURATE,

        /** Fast conversion using a simple gamma 2.2 curve. */
        FAST
    }

    /**
     * Converts an RGB color to linear space, the conversion depends on the specified type.
     *
     * @param type the color space of the RGB color values provided
     * @param r    the red component
     * @param g    the green component
     * @param b    the blue component
     *
     * @return an RGB float array of size 3 with the result of the conversion
     */
    @NonNull
    @Size(3)
    @LinearColor
    public static float[] toLinear(@NonNull RgbType type, float r, float g, float b) {
        return toLinear(type, new float[] { r, g, b });
    }

    /**
     * Converts an RGB color to linear space, the conversion depends on the specified type.
     *
     * @param type the color space of the RGB color values provided
     * @param rgb  an RGB float array of size 3, will be modified
     *
     * @return the passed-in <code>rgb</code> array, after applying the conversion
     */
    @NonNull
    @Size(min = 3)
    @LinearColor
    public static float[] toLinear(@NonNull RgbType type, @NonNull @Size(min = 3) float[] rgb) {
        return (type == RgbType.LINEAR) ? rgb : toLinear(Conversion.ACCURATE, rgb);
    }

    /**
     * Converts an RGBA color to linear space, with pre-multiplied alpha.
     *
     * @param type the color space and type of RGBA color values provided
     * @param r    the red component
     * @param g    the green component
     * @param b    the blue component
     * @param a    the alpha component
     *
     * @return an RGBA float array of size 4 with the result of the conversion
     */
    @NonNull
    @Size(4)
    @LinearColor
    public static float[] toLinear(@NonNull RgbaType type, float r, float g, float b, float a) {
        return toLinear(type, new float[] { r, g, b, a });
    }

    /**
     * Converts an RGBA color to linear space, with pre-multiplied alpha.
     *
     * @param type the color space of the RGBA color values provided
     * @param rgba an RGBA float array of size 4, will be modified
     *
     * @return the passed-in <code>rgba</code> array, after applying the conversion
     */
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

    /**
     * Converts an RGB color in sRGB space to an RGB color in linear space.
     *
     * @param conversion the conversion algorithm to use
     * @param rgb        an RGB float array of at least size 3, will be modified
     *
     * @return the passed-in <code>rgb</code> array, after applying the conversion. The alpha
     *         channel, if present, is left unmodified.
     */
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

    /**
     * Converts a correlated color temperature to a linear RGB color in sRGB space. The temperature
     * must be expressed in Kelvin and must be in the range 1,000K to 15,000K.
     *
     * @param temperature the temperature, in Kelvin
     *
     * @return an RGB float array of size 3 with the result of the conversion
     */
    @NonNull
    @Size(3)
    @LinearColor
    public static float[] cct(float temperature) {
        float[] color = new float[3];
        nCct(temperature, color);
        return color;
    }

    /**
     * Converts a CIE standard illuminant series D to a linear RGB color in sRGB space. The
     * temperature must be expressed in Kelvin and must be in the range 4,000K to 25,000K.
     *
     * @param temperature the temperature, in Kelvin
     *
     * @return an RGB float array of size 3 with the result of the conversion
     */
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
