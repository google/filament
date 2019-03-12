/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_FILAMENT_COLOR_H
#define TNT_FILAMENT_COLOR_H

#include <utils/compiler.h>

#include <math/vec3.h>
#include <math/vec4.h>

namespace filament {

//! RGB color in linear space
using LinearColor = filament::math::float3;

//! RGB color in sRGB space
using sRGBColor  = filament::math::float3;

//! RGBA color in linear space, with alpha
using LinearColorA = filament::math::float4;

//! RGBA color in sRGB space, with alpha
using sRGBColorA  = filament::math::float4;

//! types of RGB colors
enum class UTILS_PUBLIC RgbType : uint8_t {
    sRGB,   //!< the color is defined in sRGB space
    LINEAR, //!< the color is defined in linear space
};

//! types of RGBA colors
enum class UTILS_PUBLIC RgbaType : uint8_t {
    /**
     * the color is defined in sRGB space and the RGB values
     * have not been premultiplied by the alpha (for instance, a 50%
     * transparent red is <1,0,0,0.5>)
     */
    sRGB,
    /**
     * the color is defined in linear space and the RGB values
     * have not been premultiplied by the alpha (for instance, a 50%
     * transparent red is <1,0,0,0.5>)
     */
    LINEAR,
    /**
     * the color is defined in sRGB space and the RGB values
     * have been premultiplied by the alpha (for instance, a 50%
     * transparent red is <0.5,0,0,0.5>)
     */
    PREMULTIPLIED_sRGB,
    /**
     * the color is defined in linear space and the RGB values
     * have been premultiplied by the alpha (for instance, a 50%
     * transparent red is <0.5,0,0,0.5>)
     */
    PREMULTIPLIED_LINEAR
};

//! type of color conversion to use when converting to/from sRGB and linear spaces
enum UTILS_PUBLIC ColorConversion {
    ACCURATE,   //!< accurate conversion using the sRGB standard
    FAST        //!< fast conversion using a simple gamma 2.2 curve
};

/**
 * Utilities to manipulate and convert colors
 */
class UTILS_PUBLIC Color {
public:
    //! converts an RGB color to linear space, the conversion depends on the specified type
    static LinearColor toLinear(RgbType type, filament::math::float3 color);

    //! converts an RGBA color to linear space, the conversion depends on the specified type
    static LinearColorA toLinear(RgbaType type, filament::math::float4 color);

    //! converts an RGB color in sRGB space to an RGB color in linear space
    template<ColorConversion = ACCURATE>
    static LinearColor toLinear(sRGBColor const& color);

    //! converts an RGB color in linear space to an RGB color in sRGB space
    template<ColorConversion = ACCURATE>
    static sRGBColor toSRGB(LinearColor const& color);

    /**
     * converts an RGBA color in sRGB space to an RGBA color in linear space
     * the alpha component is left unmodified
     */
    template<ColorConversion = ACCURATE>
    static LinearColorA toLinear(sRGBColorA const& color);

    /**
     * converts an RGBA color in linear space to an RGBA color in sRGB space
     * the alpha component is left unmodified
     */
    template<ColorConversion = ACCURATE>
    static sRGBColorA toSRGB(LinearColorA const& color);

    /**
     * converts a correlated color temperature to a linear RGB color in sRGB
     * space the temperature must be expressed in kelvin and must be in the
     * range 1,000K to 15,000K
     */
    static LinearColor cct(float K);

    /**
     * converts a CIE standard illuminant series D to a linear RGB color in
     * sRGB space the temperature must be expressed in kelvin and must be in
     * the range 4,000K to 25,000K
     */
    static LinearColor illuminantD(float K);

private:
    static filament::math::float3 sRGBToLinear(filament::math::float3 color) noexcept;
    static filament::math::float3 linearToSRGB(filament::math::float3 color) noexcept;
};

// Use the default implementation from the header
template<>
inline LinearColor Color::toLinear<FAST>(sRGBColor const& color) {
    return pow(color, 2.2f);
}

template<>
inline LinearColorA Color::toLinear<FAST>(sRGBColorA const& color) {
    return LinearColorA{pow(color.rgb, 2.2f), color.a};
}

template<>
inline LinearColor Color::toLinear<ACCURATE>(sRGBColor const& color) {
    return sRGBToLinear(color);
}

template<>
inline LinearColorA Color::toLinear<ACCURATE>(sRGBColorA const& color) {
    return LinearColorA{sRGBToLinear(color.rgb), color.a};
}

// Use the default implementation from the header
template<>
inline sRGBColor Color::toSRGB<FAST>(LinearColor const& color) {
    return pow(color, 1.0f / 2.2f);
}

template<>
inline sRGBColorA Color::toSRGB<FAST>(LinearColorA const& color) {
    return sRGBColorA{pow(color.rgb, 1.0f / 2.2f), color.a};
}

template<>
inline sRGBColor Color::toSRGB<ACCURATE>(LinearColor const& color) {
    return linearToSRGB(color);
}

template<>
inline sRGBColorA Color::toSRGB<ACCURATE>(LinearColorA const& color) {
    return sRGBColorA{linearToSRGB(color.rgb), color.a};
}

inline LinearColor Color::toLinear(RgbType type, filament::math::float3 color) {
    return (type == RgbType::LINEAR) ? color : Color::toLinear<ACCURATE>(color);
}

// converts an RGBA color to linear space
// the conversion depends on the specified type
inline LinearColorA Color::toLinear(RgbaType type, filament::math::float4 color) {
    switch (type) {
        case RgbaType::sRGB:
            return Color::toLinear<ACCURATE>(color) * filament::math::float4{color.a, color.a, color.a, 1};
        case RgbaType::LINEAR:
            return color * filament::math::float4{color.a, color.a, color.a, 1};
        case RgbaType::PREMULTIPLIED_sRGB:
            return Color::toLinear<ACCURATE>(color);
        case RgbaType::PREMULTIPLIED_LINEAR:
            return color;
    }
}

} // namespace filament

#endif // TNT_FILAMENT_COLOR_H
