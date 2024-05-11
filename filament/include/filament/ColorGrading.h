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

//! \file

#ifndef TNT_FILAMENT_COLORGRADING_H
#define TNT_FILAMENT_COLORGRADING_H

#include <filament/FilamentAPI.h>
#include <filament/ToneMapper.h>

#include <utils/compiler.h>

#include <math/mathfwd.h>

#include <stdint.h>
#include <stddef.h>

namespace filament {

class Engine;
class FColorGrading;

namespace color {
class ColorSpace;
}

/**
 * ColorGrading is used to transform (either to modify or correct) the colors of the HDR buffer
 * rendered by Filament. Color grading transforms are applied after lighting, and after any lens
 * effects (bloom for instance), and include tone mapping.
 *
 * Creation, usage and destruction
 * ===============================
 *
 * A ColorGrading object is created using the ColorGrading::Builder and destroyed by calling
 * Engine::destroy(const ColorGrading*). A ColorGrading object is meant to be set on a View.
 *
 * ~~~~~~~~~~~{.cpp}
 *  filament::Engine* engine = filament::Engine::create();
 *
 *  filament::ColorGrading* colorGrading = filament::ColorGrading::Builder()
 *              .toneMapping(filament::ColorGrading::ToneMapping::ACES)
 *              .build(*engine);
 *
 *  myView->setColorGrading(colorGrading);
 *
 *  engine->destroy(colorGrading);
 * ~~~~~~~~~~~
 *
 * Performance
 * ===========
 *
 * Creating a new ColorGrading object may be more expensive than other Filament objects as a
 * 3D LUT may need to be generated. The generation of a 3D LUT, if necessary, may happen on
 * the CPU.
 *
 * Ordering
 * ========
 *
 * The various transforms held by ColorGrading are applied in the following order:
 * - Exposure
 * - Night adaptation
 * - White balance
 * - Channel mixer
 * - Shadows/mid-tones/highlights
 * - Slope/offset/power (CDL)
 * - Contrast
 * - Vibrance
 * - Saturation
 * - Curves
 * - Tone mapping
 * - Luminance scaling
 * - Gamut mapping
 *
 * Defaults
 * ========
 *
 * Here are the default color grading options:
 * - Exposure: 0.0
 * - Night adaptation: 0.0
 * - White balance: temperature 0, and tint 0
 * - Channel mixer: red {1,0,0}, green {0,1,0}, blue {0,0,1}
 * - Shadows/mid-tones/highlights: shadows {1,1,1,0}, mid-tones {1,1,1,0}, highlights {1,1,1,0},
 *   ranges {0,0.333,0.550,1}
 * - Slope/offset/power: slope 1.0, offset 0.0, and power 1.0
 * - Contrast: 1.0
 * - Vibrance: 1.0
 * - Saturation: 1.0
 * - Curves: gamma {1,1,1}, midPoint {1,1,1}, and scale {1,1,1}
 * - Tone mapping: ACESLegacyToneMapper
 * - Luminance scaling: false
 * - Gamut mapping: false
 * - Output color space: Rec709-sRGB-D65
 *
 * @see View
 */
class UTILS_PUBLIC ColorGrading : public FilamentAPI {
    struct BuilderDetails;
public:

    enum class QualityLevel : uint8_t {
        LOW,
        MEDIUM,
        HIGH,
        ULTRA
    };

    enum class LutFormat : uint8_t {
        INTEGER,    //!< 10 bits per component
        FLOAT,      //!< 16 bits per component (10 bits mantissa precision)
    };


    /**
     * List of available tone-mapping operators.
     *
     * @deprecated Use Builder::toneMapper(ToneMapper*) instead
     */
    enum class UTILS_DEPRECATED ToneMapping : uint8_t {
        LINEAR        = 0,     //!< Linear tone mapping (i.e. no tone mapping)
        ACES_LEGACY   = 1,     //!< ACES tone mapping, with a brightness modifier to match Filament's legacy tone mapper
        ACES          = 2,     //!< ACES tone mapping
        FILMIC        = 3,     //!< Filmic tone mapping, modelled after ACES but applied in sRGB space
        DISPLAY_RANGE = 4,     //!< Tone mapping used to validate/debug scene exposure
    };

    //! Use Builder to construct a ColorGrading object instance
    class Builder : public BuilderBase<BuilderDetails> {
        friend struct BuilderDetails;
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        /**
         * Sets the quality level of the color grading. When color grading is implemented using
         * a 3D LUT, the quality level may impact the resolution and bit depth of the backing
         * 3D texture. For instance, a low quality level will use a 16x16x16 10 bit LUT, a medium
         * quality level will use a 32x32x32 10 bit LUT, a high quality will use a 32x32x32 16 bit
         * LUT, and a ultra quality will use a 64x64x64 16 bit LUT.
         * This overrides the values set by format() and dimensions().
         *
         * The default quality is medium.
         *
         * @param qualityLevel The desired quality of the color grading process
         *
         * @return This Builder, for chaining calls
         */
        Builder& quality(QualityLevel qualityLevel) noexcept;

        /**
         * When color grading is implemented using a 3D LUT, this sets the texture format of
         * of the LUT. This overrides the value set by quality().
         *
         * The default is INTEGER
         *
         * @param format The desired format of the 3D LUT.
         *
         * @return This Builder, for chaining calls
         */
        Builder& format(LutFormat format) noexcept;

        /**
         * When color grading is implemented using a 3D LUT, this sets the dimension of the LUT.
         * This overrides the value set by quality().
         *
         * The default is 32
         *
         * @param dim The desired dimension of the LUT. Between 16 and 64.
         *
         * @return This Builder, for chaining calls
         */
        Builder& dimensions(uint8_t dim) noexcept;

        /**
         * Selects the tone mapping operator to apply to the HDR color buffer as the last
         * operation of the color grading post-processing step.
         *
         * The default tone mapping operator is ACESLegacyToneMapper.
         *
         * The specified tone mapper must have a lifecycle that exceeds the lifetime of
         * this builder. Since the build(Engine&) method is synchronous, it is safe to
         * delete the tone mapper object after that finishes executing.
         *
         * @param toneMapper The tone mapping operator to apply to the HDR color buffer
         *
         * @return This Builder, for chaining calls
         */
        Builder& toneMapper(ToneMapper const* UTILS_NULLABLE toneMapper) noexcept;

        /**
         * Selects the tone mapping operator to apply to the HDR color buffer as the last
         * operation of the color grading post-processing step.
         *
         * The default tone mapping operator is ACES_LEGACY.
         *
         * @param toneMapping The tone mapping operator to apply to the HDR color buffer
         *
         * @return This Builder, for chaining calls
         *
         * @deprecated Use toneMapper(ToneMapper*) instead
         */
        UTILS_DEPRECATED
        Builder& toneMapping(ToneMapping toneMapping) noexcept;

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
         * @param luminanceScaling Enables or disables luminance scaling post-tone mapping
         *
         * @return This Builder, for chaining calls
         */
        Builder& luminanceScaling(bool luminanceScaling) noexcept;

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
        Builder& gamutMapping(bool gamutMapping) noexcept;

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
        Builder& exposure(float exposure) noexcept;

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
        Builder& nightAdaptation(float adaptation) noexcept;

        /**
         * Adjusts the while balance of the image. This can be used to remove color casts
         * and correct the appearance of the white point in the scene, or to alter the
         * overall chromaticity of the image for artistic reasons (to make the image appear
         * cooler or warmer for instance).
         *
         * The while balance adjustment is defined with two values:
         * - Temperature, to modify the color temperature. This value will modify the colors
         *   on a blue/yellow axis. Lower values apply a cool color temperature, and higher
         *   values apply a warm color temperature. The lowest value, -1.0f, is equivalent to
         *   a temperature of 50,000K. The highest value, 1.0f, is equivalent to a temperature
         *   of 2,000K.
         * - Tint, to modify the colors on a green/magenta axis. The lowest value, -1.0f, will
         *   apply a strong green cast, and the highest value, 1.0f, will apply a strong magenta
         *   cast.
         *
         * Both values are expected to be in the range [-1.0..+1.0]. Values outside of that
         * range will be clipped to that range.
         *
         * @param temperature Modification on the blue/yellow axis, as a value between -1.0 and +1.0.
         * @param tint Modification on the green/magenta axis, as a value between -1.0 and +1.0.
         *
         * @return This Builder, for chaining calls
         */
        Builder& whiteBalance(float temperature, float tint) noexcept;

        /**
         * The channel mixer adjustment modifies each output color channel using the specified
         * mix of the source color channels.
         *
         * By default each output color channel is set to use 100% of the corresponding source
         * channel and 0% of the other channels. For instance, the output red channel is set to
         * {1.0, 0.0, 1.0} or 100% red, 0% green and 0% blue.
         *
         * Each output channel can add or subtract data from the source channel by using values
         * in the range [-2.0..+2.0]. Values outside of that range will be clipped to that range.
         *
         * Using the channel mixer adjustment you can for instance create a monochrome output
         * by setting all 3 output channels to the same mix. For instance: {0.4, 0.4, 0.2} for
         * all 3 output channels(40% red, 40% green and 20% blue).
         *
         * More complex mixes can be used to create more complex effects. For instance, here is
         * a mix that creates a sepia tone effect:
         * - outRed   = {0.255, 0.858, 0.087}
         * - outGreen = {0.213, 0.715, 0.072}
         * - outBlue  = {0.170, 0.572, 0.058}
         *
         * @param outRed The mix of source RGB for the output red channel, between -2.0 and +2.0
         * @param outGreen The mix of source RGB for the output green channel, between -2.0 and +2.0
         * @param outBlue The mix of source RGB for the output blue channel, between -2.0 and +2.0
         *
         * @return This Builder, for chaining calls
         */
        Builder& channelMixer(
                math::float3 outRed, math::float3 outGreen, math::float3 outBlue) noexcept;

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
        Builder& shadowsMidtonesHighlights(
                math::float4 shadows, math::float4 midtones, math::float4 highlights,
                math::float4 ranges) noexcept;

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
        Builder& slopeOffsetPower(math::float3 slope, math::float3 offset, math::float3 power) noexcept;

        /**
         * Adjusts the contrast of the image. Lower values decrease the contrast of the image
         * (the tonal range is narrowed), and higher values increase the contrast of the image
         * (the tonal range is widened). A value of 1.0 has no effect.
         *
         * The contrast is defined as a value in the range [0.0...2.0]. Values outside of that
         * range will be clipped to that range.
         *
         * Contrast adjustment is performed in log space.
         *
         * @param contrast Contrast expansion, between 0.0 and 2.0. 1.0 leaves contrast unaffected
         *
         * @return This Builder, for chaining calls
         */
        Builder& contrast(float contrast) noexcept;

        /**
         * Adjusts the saturation of the image based on the input color's saturation level.
         * Colors with a high level of saturation are less affected than colors with low saturation
         * levels.
         *
         * Lower vibrance values decrease intensity of the colors present in the image, and
         * higher values increase the intensity of the colors in the image. A value of 1.0 has
         * no effect.
         *
         * The vibrance is defined as a value in the range [0.0...2.0]. Values outside of that
         * range will be clipped to that range.
         *
         * Vibrance adjustment is performed in linear space.
         *
         * @param vibrance Vibrance, between 0.0 and 2.0. 1.0 leaves vibrance unaffected
         *
         * @return This Builder, for chaining calls
         */
        Builder& vibrance(float vibrance) noexcept;

        /**
         * Adjusts the saturation of the image. Lower values decrease intensity of the colors
         * present in the image, and higher values increase the intensity of the colors in the
         * image. A value of 1.0 has no effect.
         *
         * The saturation is defined as a value in the range [0.0...2.0]. Values outside of that
         * range will be clipped to that range.
         *
         * Saturation adjustment is performed in linear space.
         *
         * @param saturation Saturation, between 0.0 and 2.0. 1.0 leaves saturation unaffected
         *
         * @return This Builder, for chaining calls
         */
        Builder& saturation(float saturation) noexcept;

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
        Builder& curves(math::float3 shadowGamma, math::float3 midPoint, math::float3 highlightScale) noexcept;

        /**
         * Sets the output color space for this ColorGrading object. After all color grading steps
         * have been applied, the final color will be converted in the desired color space.
         *
         * NOTE: Currently the output color space must be one of Rec709-sRGB-D65 or
         *       Rec709-Linear-D65. Only the transfer function is taken into account.
         *
         * @param colorSpace The output color space.
         *
         * @return This Builder, for chaining calls
         */
        Builder& outputColorSpace(const color::ColorSpace& colorSpace) noexcept;

        /**
         * Creates the ColorGrading object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this ColorGrading with.
         *
         * @return pointer to the newly created object.
         */
        ColorGrading* UTILS_NONNULL build(Engine& engine);

    private:
        friend class FColorGrading;
    };

protected:
    // prevent heap allocation
    ~ColorGrading() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_COLORGRADING_H
