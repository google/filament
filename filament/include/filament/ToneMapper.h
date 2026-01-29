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

#ifndef TNT_FILAMENT_TONEMAPPER_H
#define TNT_FILAMENT_TONEMAPPER_H

#include <utils/compiler.h>

#include <math/mathfwd.h>

#include <cstdint>

namespace filament {

/**
 * Interface for tone mapping operators. A tone mapping operator, or tone mapper,
 * is responsible for compressing the dynamic range of the rendered scene to a
 * dynamic range suitable for display.
 *
 * In Filament, tone mapping is a color grading step. ToneMapper instances are
 * created and passed to the ColorGrading::Builder to produce a 3D LUT that will
 * be used during post-processing to prepare the final color buffer for display.
 *
 * Filament provides several default tone mapping operators that fall into three
 * categories:
 *
 * - Configurable tone mapping operators
 *   - GenericToneMapper
 *   - AgXToneMapper
 * - Fixed-aesthetic tone mapping operators
 *   - ACESToneMapper
 *   - ACESLegacyToneMapper
 *   - FilmicToneMapper
 *   - PBRNeutralToneMapper
 *   - GT7ToneMapper
 * - Debug/validation tone mapping operators
 *   - LinearToneMapper
 *   - DisplayRangeToneMapper
 *
 * You can create custom tone mapping operators by subclassing ToneMapper.
 */
struct UTILS_PUBLIC ToneMapper {
    ToneMapper() noexcept;
    virtual ~ToneMapper() noexcept;

    /**
     * Maps an open domain (or "scene referred" values) color value to display
     * domain (or "display referred") color value. Both the input and output
     * color values are defined in the Rec.2020 color space, with no transfer
     * function applied ("linear Rec.2020").
     *
     * @param c Input color to tone map, in the Rec.2020 color space with no
     *          transfer function applied ("linear")
     *
     * @return A tone mapped color in the Rec.2020 color space, with no transfer
     *         function applied ("linear")
     */
    virtual math::float3 operator()(math::float3 c) const noexcept = 0;

    /**
     * If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x.b))
     *
     * This may be used to indicate that the color grading's LUT only requires a
     * 1D texture instead of a 3D texture, potentially saving a significant amount
     * of memory and generation time.
     */
    virtual bool isOneDimensional() const noexcept { return false; }

    /**
     * True if this tonemapper only works in low-dynamic-range.
     *
     * This may be used to indicate that the color grading's LUT doesn't need to be
     * log encoded.
     */
    virtual bool isLDR() const noexcept { return false; }
};

/**
 * Linear tone mapping operator that returns the input color but clamped to
 * the 0..1 range. This operator is mostly useful for debugging.
 */
struct UTILS_PUBLIC LinearToneMapper final : public ToneMapper {
    LinearToneMapper() noexcept;
    ~LinearToneMapper() noexcept final;

    math::float3 operator()(math::float3 c) const noexcept override;
    bool isOneDimensional() const noexcept override { return true; }
    bool isLDR() const noexcept override { return true; }
};

/**
 * ACES tone mapping operator. This operator is an implementation of the
 * ACES Reference Rendering Transform (RRT) combined with the Output Device
 * Transform (ODT) for sRGB monitors (dim surround, 100 nits).
 */
struct UTILS_PUBLIC ACESToneMapper final : public ToneMapper {
    ACESToneMapper() noexcept;
    ~ACESToneMapper() noexcept final;

    math::float3 operator()(math::float3 c) const noexcept override;
    bool isOneDimensional() const noexcept override { return false; }
    bool isLDR() const noexcept override { return false; }
};

/**
 * ACES tone mapping operator, modified to match the perceived brightness
 * of FilmicToneMapper. This operator is the same as ACESToneMapper but
 * applies a brightness multiplier of ~1.6 to the input color value to
 * target brighter viewing environments.
 */
struct UTILS_PUBLIC ACESLegacyToneMapper final : public ToneMapper {
    ACESLegacyToneMapper() noexcept;
    ~ACESLegacyToneMapper() noexcept final;

    math::float3 operator()(math::float3 c) const noexcept override;
    bool isOneDimensional() const noexcept override { return false; }
    bool isLDR() const noexcept override { return false; }
};

/**
 * "Filmic" tone mapping operator. This tone mapper was designed to
 * approximate the aesthetics of the ACES RRT + ODT for Rec.709
 * and historically Filament's default tone mapping operator. It exists
 * only for backward compatibility purposes and is not otherwise recommended.
 */
struct UTILS_PUBLIC FilmicToneMapper final : public ToneMapper {
    FilmicToneMapper() noexcept;
    ~FilmicToneMapper() noexcept final;

    math::float3 operator()(math::float3 x) const noexcept override;
    bool isOneDimensional() const noexcept override { return true; }
    bool isLDR() const noexcept override { return false; }
};

/**
 * Khronos PBR Neutral tone mapping operator. This tone mapper was designed
 * to preserve the appearance of materials across lighting conditions while
 * avoiding artifacts in the highlights in high dynamic range conditions.
 */
struct UTILS_PUBLIC PBRNeutralToneMapper final : public ToneMapper {
    PBRNeutralToneMapper() noexcept;
    ~PBRNeutralToneMapper() noexcept final;

    math::float3 operator()(math::float3 x) const noexcept override;
    bool isOneDimensional() const noexcept override { return false; }
    bool isLDR() const noexcept override { return false; }
};

/**
 * Gran Turismo 7 tone mapping operator. This tone mapper was designed
 * to preserve the appearance of materials across lighting conditions while
 * avoiding artifacts in the highlights in high dynamic range conditions.
 * This tone mapper targets an SDR paper white value of 250 nits, with a
 * reference luminance of 100 cd/m^2 (a value of 1.0 in the HDR framebuffer).
 */
struct UTILS_PUBLIC GT7ToneMapper final : public ToneMapper {
    GT7ToneMapper() noexcept;
    ~GT7ToneMapper() noexcept final;

    math::float3 operator()(math::float3 x) const noexcept override;
    bool isOneDimensional() const noexcept override { return false; }
    bool isLDR() const noexcept override { return false; }

private:
    struct State;
    State* mState;
};

/**
 * AgX tone mapping operator.
 */
struct UTILS_PUBLIC AgxToneMapper final : public ToneMapper {
    enum class AgxLook : uint8_t {
        NONE = 0,   //!< Base contrast with no look applied
        PUNCHY,     //!< A punchy and more chroma laden look for sRGB displays
        GOLDEN      //!< A golden tinted, slightly washed look for BT.1886 displays
    };

    /**
     * Builds a new AgX tone mapper.
     *
     * @param look an optional creative adjustment to contrast and saturation
     */
    explicit AgxToneMapper(AgxLook look = AgxLook::NONE) noexcept;
    ~AgxToneMapper() noexcept final;

    math::float3 operator()(math::float3 x) const noexcept override;
    bool isOneDimensional() const noexcept override { return false; }
    bool isLDR() const noexcept override { return false; }

    AgxLook look;
};

/**
 * Generic tone mapping operator that gives control over the tone mapping
 * curve. This operator can be used to control the aesthetics of the final
 * image. This operator also allows to control the dynamic range of the
 * scene referred values.
 *
 * The tone mapping curve is defined by 5 parameters:
 * - contrast: controls the contrast of the curve
 * - midGrayIn: sets the input middle gray
 * - midGrayOut: sets the output middle gray
 * - hdrMax: defines the maximum input value that will be mapped to
 *           output white
 */
struct UTILS_PUBLIC GenericToneMapper final : public ToneMapper {
    /**
     * Builds a new generic tone mapper. The default values of the
     * constructor parameters approximate an ACES tone mapping curve
     * and the maximum input value is set to 10.0.
     *
     * @param contrast controls the contrast of the curve, must be > 0.0, values
     *                 in the range 0.5..2.0 are recommended.
     * @param midGrayIn sets the input middle gray, between 0.0 and 1.0.
     * @param midGrayOut sets the output middle gray, between 0.0 and 1.0.
     * @param hdrMax defines the maximum input value that will be mapped to
     *               output white. Must be >= 1.0.
     */
    explicit GenericToneMapper(
            float contrast = 1.55f,
            float midGrayIn = 0.18f,
            float midGrayOut = 0.215f,
            float hdrMax = 10.0f
    ) noexcept;
    ~GenericToneMapper() noexcept final;

    GenericToneMapper(GenericToneMapper const&) = delete;
    GenericToneMapper& operator=(GenericToneMapper const&) = delete;
    GenericToneMapper(GenericToneMapper&& rhs)  noexcept;
    GenericToneMapper& operator=(GenericToneMapper&& rhs) noexcept;

    math::float3 operator()(math::float3 x) const noexcept override;
    bool isOneDimensional() const noexcept override { return true; }
    bool isLDR() const noexcept override { return false; }

    /** Returns the contrast of the curve as a strictly positive value. */
    float getContrast() const noexcept;

    /** Returns the middle gray point for input values as a value between 0.0 and 1.0. */
    float getMidGrayIn() const noexcept;

    /** Returns the middle gray point for output values as a value between 0.0 and 1.0. */
    float getMidGrayOut() const noexcept;

    /** Returns the maximum input value that will map to output white, as a value >= 1.0. */
    float getHdrMax() const noexcept;

    /** Sets the contrast of the curve, must be > 0.0, values in the range 0.5..2.0 are recommended. */
    void setContrast(float contrast) noexcept;

    /** Sets the input middle gray, between 0.0 and 1.0. */
    void setMidGrayIn(float midGrayIn) noexcept;

    /** Sets the output middle gray, between 0.0 and 1.0. */
    void setMidGrayOut(float midGrayOut) noexcept;

    /** Defines the maximum input value that will be mapped to output white. Must be >= 1.0. */
    void setHdrMax(float hdrMax) noexcept;

private:
    struct Options;
    Options* mOptions;
};

/**
 * A tone mapper that converts the input HDR RGB color into one of 16 debug colors
 * that represent the pixel's exposure. When the output is cyan, the input color
 * represents  middle gray (18% exposure). Every exposure stop above or below middle
 * gray causes a color shift.
 *
 * The relationship between exposures and colors is:
 *
 * - -5EV  black
 * - -4EV  darkest blue
 * - -3EV  darker blue
 * - -2EV  dark blue
 * - -1EV  blue
 * -  OEV  cyan
 * - +1EV  dark green
 * - +2EV  green
 * - +3EV  yellow
 * - +4EV  yellow-orange
 * - +5EV  orange
 * - +6EV  bright red
 * - +7EV  red
 * - +8EV  magenta
 * - +9EV  purple
 * - +10EV white
 *
 * This tone mapper is useful to validate and tweak scene lighting.
 */
struct UTILS_PUBLIC DisplayRangeToneMapper final : public ToneMapper {
    DisplayRangeToneMapper() noexcept;
    ~DisplayRangeToneMapper() noexcept override;

    math::float3 operator()(math::float3 c) const noexcept override;
    bool isOneDimensional() const noexcept override { return false; }
    bool isLDR() const noexcept override { return false; }
};

} // namespace filament

#endif // TNT_FILAMENT_TONEMAPPER_H
