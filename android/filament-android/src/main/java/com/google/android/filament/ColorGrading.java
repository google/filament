/*
 * Copyright (C) 2020 The Android Open Source Project
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

import androidx.annotation.NonNull;
import androidx.annotation.Size;

import static com.google.android.filament.Asserts.assertFloat3In;
import static com.google.android.filament.Asserts.assertFloat4In;

/**
 * <code>ColorGrading</code> is used to transform (either to modify or correct) the colors of the
 * HDR buffer rendered by Filament. Color grading transforms are applied after lighting, and after any
 * lens effects (bloom for instance), and include tone mapping.
 *
 * <h1>Creation, usage and destruction</h1>
 *
 * A ColorGrading object is created using the ColorGrading::Builder and destroyed by calling
 * Engine::destroy(const ColorGrading*). A ColorGrading object is meant to be set on a View.
 *
 * <pre>
 *  Engine engine = Engine.create();
 *
 *  ColorGrading colorGrading = ColorGrading.Builder()
 *              .toneMapping(ColorGrading.ToneMapping.ACES)
 *              .build(engine);
 *
 *  myView.setColorGrading(colorGrading);
 *
 *  engine.destroy(colorGrading);
 * </pre>
 *
 * <h1>Performance</h1>
 *
 * Creating a new ColorGrading object may be more expensive than other Filament objects as a LUT may
 * need to be generated. The generation of this LUT, if necessary, may happen on the CPU.
 *
 * <h1>Ordering</h1>
 *
 * The various transforms held by ColorGrading are applied in the following order:
 * <ul>
 * <li>Exposure</li>
 * <li>Night adaptation</li>
 * <li>White balance</li>
 * <li>Channel mixer</li>
 * <li>Shadows/mid-tones/highlights</li>
 * <li>Slope/offset/power (CDL)</li>
 * <li>Contrast</li>
 * <li>Vibrance</li>
 * <li>Saturation</li>
 * <li>Curves</li>
 * <li>Tone mapping</li>
 * <li>Luminance scaling</li>
 * <li>Gamut mapping</li>
 * </ul>
 *
 * <h1>Defaults</h1>
 *
 * Here are the default color grading options:
 * <ul>
 * <li>Exposure: 0.0</li>
 * <li>Night adaptation: 0.0</li>
 * <li>White balance: temperature <code>0.0</code>, and tint <code>0.0</code></li>
 * <li>Channel mixer: red <code>{1,0,0}</code>, green <code>{0,1,0}</code>, blue <code>{0,0,1}</code></li>
 * <li>Shadows/mid-tones/highlights: shadows <code>{1,1,1,0}</code>, mid-tones <code>{1,1,1,0}</code>,
 * highlights <code>{1,1,1,0}</code>, ranges <code>{0,0.333,0.550,1}</code></li>
 * <li>Slope/offset/power: slope <code>1.0</code>, offset <code>0.0</code>, and power <code>1.0</code></li>
 * <li>Contrast: <code>1.0</code></li>
 * <li>Vibrance: <code>1.0</code></li>
 * <li>Saturation: <code>1.0</code></li>
 * <li>Curves: gamma <code>{1,1,1}</code>, midPoint <code>{1,1,1}</code>, and scale <code>{1,1,1}</code></li>
 * <li>Tone mapping: {@link ToneMapper.ACESLegacy}</li>
 * <li>Luminance scaling: false</li>
 * <li>Gamut mapping: false</li>
 * </ul>
 *
 * @see View
 * @see ToneMapper
 */
public class ColorGrading {
    long mNativeObject;

    /**
     * Color grading quality level.
     */
    public enum QualityLevel {
        LOW,
        MEDIUM,
        HIGH,
        ULTRA
    }

    /**
     * Color grading LUT format.
     */
    public enum LutFormat {
        INTEGER,
        FLOAT
    }

    /**
     * List of available tone-mapping operators.
     *
     * @deprecated Use {@link ColorGrading.Builder#toneMapper(ToneMapper)}
     */
    @Deprecated
    public enum ToneMapping {
        /** Linear tone mapping (i.e. no tone mapping). */
        LINEAR,
        /** ACES tone mapping, with a brightness modifier to match Filament's legacy tone mapper. */
        ACES_LEGACY,
        /** ACES tone mapping. */
        ACES,
        /** Filmic tone mapping, modelled after ACES but applied in sRGB space. */
        FILMIC,
        /** Tone mapping used to validate/debug scene exposure. */
        DISPLAY_RANGE,
    }

    // NOTE: This constructor is public only so that filament-utils can use it.
    public ColorGrading(long colorGrading) {
        mNativeObject = colorGrading;
    }

    /**
     * Use <code>Builder</code> to construct a <code>ColorGrading</code> object instance.
     */
    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"}) // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        /**
         * Use <code>Builder</code> to construct a <code>ColorGrading</code> object instance.
         */
        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Sets the quality level of the color grading. When color grading is implemented using
         * a 3D LUT, the quality level may impact the resolution and bit depth of the backing
         * 3D texture. For instance, a low quality level will use a 16x16x16 10 bit LUT, a medium
         * quality level will use a 32x32x32 10 bit LUT, a high quality will use a 32x32x32 16 bit
         * LUT, and a ultra quality will use a 64x64x64 16 bit LUT.
         *
         * This setting has no effect if generating a 1D LUT.
         *
         * This overrides the values set by format() and dimensions().
         *
         * The default quality is {@link QualityLevel#MEDIUM}.
         *
         * @param qualityLevel The desired quality of the color grading process
         *
         * @return This Builder, for chaining calls
         */
        public Builder quality(QualityLevel qualityLevel) {
            nBuilderQuality(mNativeBuilder, qualityLevel.ordinal());
            return this;
        }

        /**
         * When color grading is implemented using a 3D LUT, this sets the texture format of
         * of the LUT. This overrides the value set by quality().
         *
         * This setting has no effect if generating a 1D LUT.
         *
         * The default is INTEGER
         *
         * @param format The desired format of the 3D LUT.
         *
         * @return This Builder, for chaining calls
         */
        public Builder format(LutFormat format) {
            nBuilderFormat(mNativeBuilder, format.ordinal());
            return this;
        }

        /**
         * When color grading is implemented using a 3D LUT, this sets the dimension of the LUT.
         * This overrides the value set by quality().
         *
         * This setting has no effect if generating a 1D LUT.
         *
         * The default is 32
         *
         * @param dim The desired dimension of the LUT. Between 16 and 64.
         *
         * @return This Builder, for chaining calls
         */
        public Builder dimensions(int dim) {
            nBuilderDimensions(mNativeBuilder, dim);
            return this;
        }

        /**
         * Selects the tone mapping operator to apply to the HDR color buffer as the last
         * operation of the color grading post-processing step.
         *
         * The default tone mapping operator is {@link ToneMapper.ACESLegacy}.
         *
         * The specified tone mapper must have a lifecycle that exceeds the lifetime of
         * this builder. Since the build(Engine&) method is synchronous, it is safe to
         * delete the tone mapper object after that finishes executing.
         *
         * @param toneMapper The tone mapping operator to apply to the HDR color buffer
         *
         * @return This Builder, for chaining calls
         */
        public Builder toneMapper(ToneMapper toneMapper) {
            nBuilderToneMapper(mNativeBuilder, toneMapper.getNativeObject());
            return this;
        }

        /**
         * Selects the tone mapping operator to apply to the HDR color buffer as the last
         * operation of the color grading post-processing step.
         *
         * The default tone mapping operator is {@link ToneMapping#ACES_LEGACY}.
         *
         * @param toneMapping The tone mapping operator to apply to the HDR color buffer
         *
         * @return This Builder, for chaining calls
         *
         * @deprecated Use {@link #toneMapper(ToneMapper)}
         */
        @Deprecated
        public Builder toneMapping(ToneMapping toneMapping) {
            nBuilderToneMapping(mNativeBuilder, toneMapping.ordinal());
            return this;
        }

        /**
         * Enables or disables the luminance scaling component (LICH) from the exposure value
         * invariant luminance system (EVILS). When this setting is enabled, pixels with high
         * chromatic values will roll-off to white to offer a more natural rendering. This step
         * also helps avoid undesirable hue skews caused by out of gamut colors clipped
         * to the destination color gamut.
         *
         * When luminance scaling is enabled, tone mapping is performed on the luminance of each
         * pixel instead of per-channel.
         *
         * @param luminanceScaling Enables or disables EVILS post-tone mapping
         *
         * @return This Builder, for chaining calls
         */
        public Builder luminanceScaling(boolean luminanceScaling) {
            nBuilderLuminanceScaling(mNativeBuilder, luminanceScaling);
            return this;
        }

        /**
         * Enables or disables gamut mapping to the destination color space's gamut. When gamut
         * mapping is turned off, out-of-gamut colors are clipped to the destination's gamut,
         * which may produce hue skews (blue skewing to purple, green to yellow, etc.). When
         * gamut mapping is enabled, out-of-gamut colors are brought back in gamut by trying to
         * preserve the perceived chroma and lightness of the original values.
         *
         * @param gamutMapping Enables or disables gamut mapping
         *
         * @return This Builder, for chaining calls
         */
        public Builder gamutMapping(boolean gamutMapping) {
            nBuilderGamutMapping(mNativeBuilder, gamutMapping);
            return this;
        }

        /**
         * Adjusts the exposure of this image. The exposure is specified in stops:
         * each stop brightens (positive values) or darkens (negative values) the image by
         * a factor of 2. This means that an exposure of 3 will brighten the image 8 times
         * more than an exposure of 0 (2^3 = 8 and 2^0 = 1). Contrary to the camera's exposure,
         * this setting is applied after all post-processing (bloom, etc.) are applied.
         *
         * @param exposure Value in EV stops. Can be negative, 0, or positive.
         *
         * @return This Builder, for chaining calls
         */
        public Builder exposure(float exposure) {
            nBuilderExposure(mNativeBuilder, exposure);
            return this;
        }

        /**
         * Controls the amount of night adaptation to replicate a more natural representation of
         * low-light conditions as perceived by the human vision system. In low-light conditions,
         * peak luminance sensitivity of the eye shifts toward the blue end of the color spectrum:
         * darker tones appear brighter, reducing contrast, and colors are blue shifted (the darker
         * the more intense the effect).
         *
         * @param adaptation Amount of adaptation, between 0 (no adaptation) and 1 (full adaptation).
         *
         * @return This Builder, for chaining calls
         */
        public Builder nightAdaptation(float adaptation) {
            nBuilderNightAdaptation(mNativeBuilder, adaptation);
            return this;
        }

        /**
         * Adjusts the while balance of the image. This can be used to remove color casts
         * and correct the appearance of the white point in the scene, or to alter the
         * overall chromaticity of the image for artistic reasons (to make the image appear
         * cooler or warmer for instance).
         *
         * The while balance adjustment is defined with two values:
         * <ul>
         * <li>Temperature, to modify the color temperature. This value will modify the colors
         * on a blue/yellow axis. Lower values apply a cool color temperature, and higher
         * values apply a warm color temperature. The lowest value, -1.0f, is equivalent to
         * a temperature of 50,000K. The highest value, 1.0f, is equivalent to a temperature
         * of 2,000K.</li>
         * <li>Tint, to modify the colors on a green/magenta axis. The lowest value, -1.0f, will
         * apply a strong green cast, and the highest value, 1.0f, will apply a strong magenta
         * cast.</li>
         * </ul>
         *
         * Both values are expected to be in the range <code>[-1.0..+1.0]</code>. Values outside
         * of that range will be clipped to that range.
         *
         * @param temperature Modification on the blue/yellow axis, as a value between -1.0 and +1.0.
         * @param tint Modification on the green/magenta axis, as a value between -1.0 and +1.0.
         *
         * @return This Builder, for chaining calls
         */
        public Builder whiteBalance(float temperature, float tint) {
            nBuilderWhiteBalance(mNativeBuilder, temperature, tint);
            return this;
        }

        /**
         * The channel mixer adjustment modifies each output color channel using the specified
         * mix of the source color channels.
         *
         * By default each output color channel is set to use 100% of the corresponding source
         * channel and 0% of the other channels. For instance, the output red channel is set to
         * <code>{1.0, 0.0, 1.0}</code> or 100% red, 0% green and 0% blue.
         *
         * Each output channel can add or subtract data from the source channel by using values
         * in the range <code>[-2.0..+2.0]</code>. Values outside of that range will be clipped
         * to that range.
         *
         * Using the channel mixer adjustment you can for instance create a monochrome output
         * by setting all 3 output channels to the same mix. For instance:
         * </code>{0.4, 0.4, 0.2}</code> for all 3 output channels(40% red, 40% green and 20% blue).
         *
         * More complex mixes can be used to create more complex effects. For instance, here is
         * a mix that creates a sepia tone effect:
         * <ul>
         * <li><code>outRed   = {0.255, 0.858, 0.087}</code></li>
         * <li><code>outGreen = {0.213, 0.715, 0.072}</code></li>
         * <li><code>outBlue  = {0.170, 0.572, 0.058}</code></li>
         * </ul>
         *
         * @param outRed The mix of source RGB for the output red channel, between -2.0 and +2.0
         * @param outGreen The mix of source RGB for the output green channel, between -2.0 and +2.0
         * @param outBlue The mix of source RGB for the output blue channel, between -2.0 and +2.0
         *
         * @return This Builder, for chaining calls
         */
        public Builder channelMixer(
                @NonNull @Size(min = 3) float[] outRed,
                @NonNull @Size(min = 3) float[] outGreen,
                @NonNull @Size(min = 3) float[] outBlue) {

            assertFloat3In(outRed);
            assertFloat3In(outGreen);
            assertFloat3In(outBlue);

            nBuilderChannelMixer(mNativeBuilder, outRed, outGreen, outBlue);

            return this;
        }

        /**
         * Adjusts the colors separately in 3 distinct tonal ranges or zones: shadows, mid-tones,
         * and highlights.
         *
         * The tonal zones are by the ranges parameter: the x and y components define the beginning
         * and end of the transition from shadows to mid-tones, and the z and w components define
         * the beginning and end of the transition from mid-tones to highlights.
         *
         * A smooth transition is applied between the zones which means for instance that the
         * correction color of the shadows range will partially apply to the mid-tones, and the
         * other way around. This ensure smooth visual transitions in the final image.
         *
         * Each correction color is defined as a linear RGB color and a weight. The weight is a
         * value (which may be positive or negative) that is added to the linear RGB color before
         * mixing. This can be used to darken or brighten the selected tonal range.
         *
         * Shadows/mid-tones/highlights adjustment are performed linear space.
         *
         * @param shadows Linear RGB color (.rgb) and weight (.w) to apply to the shadows
         * @param midtones Linear RGB color (.rgb) and weight (.w) to apply to the mid-tones
         * @param highlights Linear RGB color (.rgb) and weight (.w) to apply to the highlights
         * @param ranges Range of the shadows (x and y), and range of the highlights (z and w)
         *
         * @return This Builder, for chaining calls
         */
        public Builder shadowsMidtonesHighlights(
                @NonNull @Size(min = 4) float[] shadows,
                @NonNull @Size(min = 4) float[] midtones,
                @NonNull @Size(min = 4) float[] highlights,
                @NonNull @Size(min = 4) float[] ranges) {

            assertFloat4In(shadows);
            assertFloat4In(midtones);
            assertFloat4In(highlights);
            assertFloat4In(ranges);

            nBuilderShadowsMidtonesHighlights(mNativeBuilder, shadows, midtones, highlights, ranges);

            return this;
        }

        /**
         * Applies a slope, offset, and power, as defined by the ASC CDL (American Society of
         * Cinematographers Color Decision List) to the image. The CDL can be used to adjust the
         * colors of different tonal ranges in the image.
         *
         * The ASC CDL is similar to the lift/gamma/gain controls found in many color grading tools.
         * Lift is equivalent to a combination of offset and slope, gain is equivalent to slope,
         * and gamma is equivalent to power.
         *
         * The slope and power values must be strictly positive. Values less than or equal to 0 will
         * be clamped to a small positive value, offset can be any positive or negative value.
         *
         * Version 1.2 of the ASC CDL adds saturation control, which is here provided as a separate
         * API. See the saturation() method for more information.
         *
         * Slope/offset/power adjustments are performed in log space.
         *
         * @param slope Multiplier of the input color, must be a strictly positive number
         * @param offset Added to the input color, can be a negative or positive number, including 0
         * @param power Power exponent of the input color, must be a strictly positive number
         *
         * @return This Builder, for chaining calls
         */
        public Builder slopeOffsetPower(
                @NonNull @Size(min = 3) float[] slope,
                @NonNull @Size(min = 3) float[] offset,
                @NonNull @Size(min = 3) float[] power) {

            assertFloat3In(slope);
            assertFloat3In(offset);
            assertFloat3In(power);

            nBuilderSlopeOffsetPower(mNativeBuilder, slope, offset, power);

            return this;
        }

        /**
         * Adjusts the contrast of the image. Lower values decrease the contrast of the image
         * (the tonal range is narrowed), and higher values increase the contrast of the image
         * (the tonal range is widened). A value of 1.0 has no effect.
         *
         * The contrast is defined as a value in the range <code>[0.0...2.0]</code>. Values
         * outside of that range will be clipped to that range.
         *
         * Contrast adjustment is performed in log space.
         *
         * @param contrast Contrast expansion, between 0.0 and 2.0. 1.0 leaves contrast unaffected
         *
         * @return This Builder, for chaining calls
         */
        public Builder contrast(float contrast) {
            nBuilderContrast(mNativeBuilder, contrast);
            return this;
        }

        /**
         * Adjusts the saturation of the image based on the input color's saturation level.
         * Colors with a high level of saturation are less affected than colors with low saturation
         * levels.
         *
         * Lower vibrance values decrease intensity of the colors present in the image, and
         * higher values increase the intensity of the colors in the image. A value of 1.0 has
         * no effect.
         *
         * The vibrance is defined as a value in the range <code>[0.0...2.0]</code>. Values outside
         * of that range will be clipped to that range.
         *
         * Vibrance adjustment is performed in linear space.
         *
         * @param vibrance Vibrance, between 0.0 and 2.0. 1.0 leaves vibrance unaffected
         *
         * @return This Builder, for chaining calls
         */
        public Builder vibrance(float vibrance) {
            nBuilderVibrance(mNativeBuilder, vibrance);
            return this;
        }

        /**
         * Adjusts the saturation of the image. Lower values decrease intensity of the colors
         * present in the image, and higher values increase the intensity of the colors in the
         * image. A value of 1.0 has no effect.
         *
         * The saturation is defined as a value in the range <code>[0.0...2.0]</code>.
         * Values outside of that range will be clipped to that range.
         *
         * Saturation adjustment is performed in linear space.
         *
         * @param saturation Saturation, between 0.0 and 2.0. 1.0 leaves saturation unaffected
         *
         * @return This Builder, for chaining calls
         */
        public Builder saturation(float saturation) {
            nBuilderSaturation(mNativeBuilder, saturation);
            return this;
        }

        /**
         * Applies a curve to each RGB channel of the image. Each curve is defined by 3 values:
         * a gamma value applied to the shadows only, a mid-point indicating where shadows stop
         * and highlights start, and a scale factor for the highlights.
         *
         * The gamma and mid-point must be strictly positive values. If they are not, they will be
         * clamped to a small positive value. The scale can be any negative of positive value.
         *
         * Curves are applied in linear space.
         *
         * @param shadowGamma Power value to apply to the shadows, must be strictly positive
         * @param midPoint Mid-point defining where shadows stop and highlights start, must be strictly positive
         * @param highlightScale Scale factor for the highlights, can be any negative or positive value
         *
         * @return This Builder, for chaining calls
         */
        public Builder curves(
                @NonNull @Size(min = 3) float[] shadowGamma,
                @NonNull @Size(min = 3) float[] midPoint,
                @NonNull @Size(min = 3) float[] highlightScale) {

            assertFloat3In(shadowGamma);
            assertFloat3In(midPoint);
            assertFloat3In(highlightScale);

            nBuilderCurves(mNativeBuilder, shadowGamma, midPoint, highlightScale);

            return this;
        }

        /**
         * Creates the IndirectLight object and returns a pointer to it.
         *
         * @param engine The {@link Engine} to associate this <code>IndirectLight</code> with.
         *
         * @return A newly created <code>IndirectLight</code>
         *
         * @exception IllegalStateException if a parameter to a builder function was invalid.
         */
        @NonNull
        public ColorGrading build(@NonNull Engine engine) {
            long nativeColorGrading = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeColorGrading == 0) throw new IllegalStateException("Couldn't create ColorGrading");
            return new ColorGrading(nativeColorGrading);
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;

            BuilderFinalizer(long nativeObject) { mNativeObject = nativeObject; }

            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) { // Ignore
                } finally {
                    nDestroyBuilder(mNativeObject);
                }
            }
        }
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed ColorGrading");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);

    private static native void nBuilderQuality(long nativeBuilder, int quality);
    private static native void nBuilderFormat(long nativeBuilder, int format);
    private static native void nBuilderDimensions(long nativeBuilder, int dim);
    private static native void nBuilderToneMapper(long nativeBuilder, long toneMapper);
    private static native void nBuilderToneMapping(long nativeBuilder, int toneMapping);
    private static native void nBuilderLuminanceScaling(long nativeBuilder, boolean luminanceScaling);
    private static native void nBuilderGamutMapping(long nativeBuilder, boolean gamutMapping);
    private static native void nBuilderExposure(long nativeBuilder, float exposure);
    private static native void nBuilderNightAdaptation(long nativeBuilder, float adaptation);
    private static native void nBuilderWhiteBalance(long nativeBuilder, float temperature, float tint);
    private static native void nBuilderChannelMixer(long nativeBuilder, float[] outRed, float[] outGreen, float[] outBlue);
    private static native void nBuilderShadowsMidtonesHighlights(long nativeBuilder, float[] shadows, float[] midtones, float[] highlights, float[] ranges);
    private static native void nBuilderSlopeOffsetPower(long nativeBuilder, float[] slope, float[] offset, float[] power);
    private static native void nBuilderContrast(long nativeBuilder, float contrast);
    private static native void nBuilderVibrance(long nativeBuilder, float vibrance);
    private static native void nBuilderSaturation(long nativeBuilder, float saturation);
    private static native void nBuilderCurves(long nativeBuilder, float[] gamma, float[] midPoint, float[] scale);

    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);
}
