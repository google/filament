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

#ifndef TNT_FILAMENT_COLOR_SPACE_H
#define TNT_FILAMENT_COLOR_SPACE_H

#include <math/vec2.h>

namespace filament::color {

using namespace math;

/**
 * Holds the chromaticities of a color space's primaries as xy coordinates
 * in xyY (Y is assumed to be 1).
 */
struct Primaries {
    float2 r;
    float2 g;
    float2 b;

    bool operator==(const Primaries& rhs) const noexcept {
        return r == rhs.r && b == rhs.b && g == rhs.g;
    }
};

//! Reference white for a color space, defined as the xy coordinates in the xyY space.
using WhitePoint = float2;

/**
 * <p>Defines the parameters for the ICC parametric curve type 4, as
 * defined in ICC.1:2004-10, section 10.15.</p>
 *
 * <p>The EOTF is of the form:</p>
 *
 * \(\begin{equation}
 * Y = \begin{cases}c X + f & X \lt d \\\
 * \left( a X + b \right) ^{g} + e & X \ge d \end{cases}
 * \end{equation}\)
 *
 * <p>The corresponding OETF is simply the inverse function.</p>
 *
 * <p>The parameters defined by this class form a valid transfer
 * function only if all the following conditions are met:</p>
 * <ul>
 *     <li>No parameter is a NaN</li>
 *     <li>\(d\) is in the range \([0..1]\)</li>
 *     <li>The function is not constant</li>
 *     <li>The function is positive and increasing</li>
 * </ul>
 */
struct TransferFunction {
    /**
     * <p>Defines the parameters for the ICC parametric curve type 3, as
     * defined in ICC.1:2004-10, section 10.15.</p>
     *
     * <p>The EOTF is of the form:</p>
     *
     * \(\begin{equation}
     * Y = \begin{cases}c X & X \lt d \\\
     * \left( a X + b \right) ^{g} & X \ge d \end{cases}
     * \end{equation}\)
     *
     * <p>This constructor is equivalent to setting  \(e\) and \(f\) to 0.</p>
     *
     * @param a The value of \(a\) in the equation of the EOTF described above
     * @param b The value of \(b\) in the equation of the EOTF described above
     * @param c The value of \(c\) in the equation of the EOTF described above
     * @param d The value of \(d\) in the equation of the EOTF described above
     * @param g The value of \(g\) in the equation of the EOTF described above
     */
    constexpr TransferFunction(
            double a,
            double b,
            double c,
            double d,
            double e,
            double f,
            double g
    ) : a(a),
        b(b),
        c(c),
        d(d),
        e(e),
        f(f),
        g(g) {
    }

    constexpr TransferFunction(
            double a,
            double b,
            double c,
            double d,
            double g
    ) : TransferFunction(a, b, c, d, 0.0, 0.0, g) {
    }

    bool operator==(const TransferFunction& rhs) const noexcept {
        return
                a == rhs.a &&
                b == rhs.b &&
                c == rhs.c &&
                d == rhs.d &&
                e == rhs.e &&
                f == rhs.f &&
                g == rhs.g;
    }

    double a;
    double b;
    double c;
    double d;
    double e;
    double f;
    double g;
};

/**
 * <p>A color space in Filament is always an RGB color space. A specific RGB color space
 * is defined by the following properties:</p>
 * <ul>
 *     <li>Three chromaticities of the red, green and blue primaries, which
 *     define the gamut of the color space.</li>
 *     <li>A white point chromaticity that defines the stimulus to which
 *     color space values are normalized (also just called "white").</li>
 *     <li>An opto-electronic transfer function, also called opto-electronic
 *     conversion function or often, and approximately, gamma function.</li>
 *     <li>An electro-optical transfer function, also called electo-optical
 *     conversion function or often, and approximately, gamma function.</li>
 * </ul>
 *
 * <h3>Primaries and white point chromaticities</h3>
 * <p>In this implementation, the chromaticity of the primaries and the white
 * point of an RGB color space is defined in the CIE xyY color space. This
 * color space separates the chromaticity of a color, the x and y components,
 * and its luminance, the Y component. Since the primaries and the white
 * point have full brightness, the Y component is assumed to be 1 and only
 * the x and y components are needed to encode them.</p>
 *
 * <h3>Transfer functions</h3>
 * <p>A transfer function is a color component conversion function, defined as
 * a single variable, monotonic mathematical function. It is applied to each
 * individual component of a color. They are used to perform the mapping
 * between linear tristimulus values and non-linear electronic signal value.</p>
 * <p>The <em>opto-electronic transfer function</em> (OETF or OECF) encodes
 * tristimulus values in a scene to a non-linear electronic signal value.</p>
 */
class ColorSpace {
public:
    constexpr ColorSpace(
            const Primaries primaries,
            const TransferFunction transferFunction,
            const WhitePoint whitePoint
    ) : mPrimaries(primaries),
        mTransferFunction(transferFunction),
        mWhitePoint(whitePoint) {
    }

    bool operator==(const ColorSpace& rhs) const noexcept {
        return mPrimaries == rhs.mPrimaries &&
                mTransferFunction == rhs.mTransferFunction &&
                mWhitePoint == rhs.mWhitePoint;
    }

    constexpr const Primaries& getPrimaries() const { return mPrimaries; }
    constexpr const TransferFunction& getTransferFunction() const { return mTransferFunction; }
    constexpr const WhitePoint& getWhitePoint() const { return mWhitePoint; }

private:
    Primaries mPrimaries;
    TransferFunction mTransferFunction;
    WhitePoint mWhitePoint;
};

/**
 * Intermediate class used when building a color space using the "-" syntax:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * // Declares a "linear sRGB" color space.
 * ColorSpace myColorSpace = Rec709-Linear-D65;
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class PartialColorSpace {
public:
    constexpr ColorSpace operator-(const WhitePoint& whitePoint) const {
        return { mPrimaries, mTransferFunction, whitePoint };
    }

private:
    constexpr PartialColorSpace(
            const Primaries primaries,
            const TransferFunction transferFunction
    ) : mPrimaries(primaries),
        mTransferFunction(transferFunction) {
    }

    Primaries mPrimaries;
    TransferFunction mTransferFunction;

    friend class Gamut;
};

/**
 * Defines the chromaticities of the primaries for a color space. The chromaticities
 * are expressed as three pairs of xy coordinates (in xyY) for the red, green, and blue
 * chromaticities.
 */
class Gamut {
public:
    constexpr explicit Gamut(const Primaries primaries) : mPrimaries(primaries) {
    }

    constexpr Gamut(float2 r, float2 g, float2 b) : Gamut(Primaries{ r, g, b }) {
    }

    constexpr PartialColorSpace operator-(const TransferFunction& transferFunction) const {
        return { mPrimaries, transferFunction };
    }

    constexpr const Primaries& getPrimaries() const { return mPrimaries; }

private:
    Primaries mPrimaries;
};

//! Rec.709 color gamut, used in the sRGB and DisplayP3 color spaces.
constexpr Gamut Rec709 = {{ 0.640f, 0.330f },
                          { 0.300f, 0.600f },
                          { 0.150f, 0.060f }};

//! Linear transfer function.
constexpr TransferFunction Linear = { 1.0, 0.0, 0.0, 0.0, 1.0 };

//! sRGB transfer function.
constexpr TransferFunction sRGB = { 1.0 / 1.055, 0.055 / 1.055, 1.0 / 12.92, 0.04045, 2.4 };

//! Standard CIE 1931 2° illuminant D65. This illuminant has a color temperature of 6504K.
constexpr WhitePoint D65 = { 0.31271f, 0.32902f };

} // namespace filament::color

#endif // TNT_FILAMENT_COLOR_SPACE_H
