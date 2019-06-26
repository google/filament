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

#include <ibl/CubemapSH.h>

#include <ibl/Cubemap.h>
#include <ibl/CubemapUtils.h>
#include <ibl/utilities.h>

#include "CubemapUtilsImpl.h"

#include <utils/JobSystem.h>

#include <math/mat4.h>

using namespace filament::math;
using namespace utils;

namespace filament {
namespace ibl {

// -----------------------------------------------------------------------------------------------
// A few useful utilities
// -----------------------------------------------------------------------------------------------

/*
 * returns n! / d!
 */
static float factorial(size_t n, size_t d = 1) {
   d = std::max(size_t(1), d);
   n = std::max(size_t(1), n);
   float r = 1.0;
   if (n == d) {
       // intentionally left blank
   } else if (n > d) {
       for ( ; n>d ; n--) {
           r *= n;
       }
   } else {
       for ( ; d>n ; d--) {
           r *= d;
       }
       r = 1.0f / r;
   }
   return r;
}

// -----------------------------------------------------------------------------------------------

/*
 * SH scaling factors:
 *  returns sqrt((2*l + 1) / 4*pi) * sqrt( (l-|m|)! / (l+|m|)! )
 */
float CubemapSH::Kml(ssize_t m, size_t l) {
    m = std::abs(m);
    const float K = (2*l + 1) * factorial(size_t(l-m), size_t(l+m));
    return std::sqrt(K) * (M_2_SQRTPI * 0.25);
}

// < cos(theta) > SH coefficients pre-multiplied by 1 / K(0,l)
float CubemapSH::computeTruncatedCosSh(size_t l) {
    if (l == 0) {
        return M_PI;
    } else if (l == 1) {
        return 2 * M_PI / 3;
    } else if (l & 1) {
        return 0;
    }
    const size_t l_2 = l / 2;
    float A0 = ((l_2 & 1) ? 1.0f : -1.0f) / ((l + 2) * (l - 1));
    float A1 = factorial(l, l_2) / (factorial(l_2) * (1 << l));
    return 2 * M_PI * A0 * A1;
}

/*
 * Calculates non-normalized SH bases, i.e.:
 *  m > 0, cos(m*phi)   * P(m,l)
 *  m < 0, sin(|m|*phi) * P(|m|,l)
 *  m = 0, P(0,l)
 */
void CubemapSH::computeShBasis(
        float* UTILS_RESTRICT SHb,
        size_t numBands,
        const float3& s)
{
#if 0
    // Reference implementation
    float phi = atan2(s.x, s.y);
    for (size_t l = 0; l < numBands; l++) {
        SHb[SHindex(0, l)] = Legendre(l, 0, s.z);
        for (size_t m = 1; m <= l; m++) {
            float p = Legendre(l, m, s.z);
            SHb[SHindex(-m, l)] = std::sin(m * phi) * p;
            SHb[SHindex( m, l)] = std::cos(m * phi) * p;
        }
    }
#endif

    /*
     * TODO: all the Legendre computation below is identical for all faces, so it
     * might make sense to pre-compute it once. Also note that there is
     * a fair amount of symmetry within a face (which we could take advantage of
     * to reduce the pre-compute table).
     */

    /*
     * Below, we compute the associated Legendre polynomials using recursion.
     * see: http://mathworld.wolfram.com/AssociatedLegendrePolynomial.html
     *
     * Note [0]: s.z == cos(theta) ==> we only need to compute P(s.z)
     *
     * Note [1]: We in fact compute P(s.z) / sin(theta)^|m|, by removing
     * the "sqrt(1 - s.z*s.z)" [i.e.: sin(theta)] factor from the recursion.
     * This is later corrected in the ( cos(m*phi), sin(m*phi) ) recursion.
     */

    // s = (x, y, z) = (sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta))

    // handle m=0 separately, since it produces only one coefficient
    float Pml_2 = 0;
    float Pml_1 = 1;
    SHb[0] =  Pml_1;
    for (size_t l=1; l<numBands; l++) {
        float Pml = ((2*l-1.0f)*Pml_1*s.z - (l-1.0f)*Pml_2) / l;
        Pml_2 = Pml_1;
        Pml_1 = Pml;
        SHb[SHindex(0, l)] = Pml;
    }
    float Pmm = 1;
    for (size_t m=1 ; m<numBands ; m++) {
        Pmm = (1.0f - 2*m) * Pmm;      // See [1], divide by sqrt(1 - s.z*s.z);
        Pml_2 = Pmm;
        Pml_1 = (2*m + 1.0f)*Pmm*s.z;
        // l == m
        SHb[SHindex(-m, m)] = Pml_2;
        SHb[SHindex( m, m)] = Pml_2;
        if (m+1 < numBands) {
            // l == m+1
            SHb[SHindex(-m, m+1)] = Pml_1;
            SHb[SHindex( m, m+1)] = Pml_1;
            for (size_t l=m+2 ; l<numBands ; l++) {
                float Pml = ((2*l - 1.0f)*Pml_1*s.z - (l + m - 1.0f)*Pml_2) / (l-m);
                Pml_2 = Pml_1;
                Pml_1 = Pml;
                SHb[SHindex(-m, l)] = Pml;
                SHb[SHindex( m, l)] = Pml;
            }
        }
    }

    // At this point, SHb contains the associated Legendre polynomials divided
    // by sin(theta)^|m|. Below we compute the SH basis.
    //
    // ( cos(m*phi), sin(m*phi) ) recursion:
    // cos(m*phi + phi) == cos(m*phi)*cos(phi) - sin(m*phi)*sin(phi)
    // sin(m*phi + phi) == sin(m*phi)*cos(phi) + cos(m*phi)*sin(phi)
    // cos[m+1] == cos[m]*s.x - sin[m]*s.y
    // sin[m+1] == sin[m]*s.x + cos[m]*s.y
    //
    // Note that (d.x, d.y) == (cos(phi), sin(phi)) * sin(theta), so the
    // code below actually evaluates:
    //      (cos((m*phi), sin(m*phi)) * sin(theta)^|m|
    float Cm = s.x;
    float Sm = s.y;
    for (size_t m = 1; m <= numBands; m++) {
        for (size_t l = m; l < numBands; l++) {
            SHb[SHindex(-m, l)] *= Sm;
            SHb[SHindex( m, l)] *= Cm;
        }
        float Cm1 = Cm * s.x - Sm * s.y;
        float Sm1 = Sm * s.x + Cm * s.y;
        Cm = Cm1;
        Sm = Sm1;
    }
}

std::unique_ptr<float3[]> CubemapSH::computeSH(JobSystem& js, const Cubemap& cm, size_t numBands, bool irradiance) {

    const size_t numCoefs = numBands*numBands;
    std::unique_ptr<float3[]> SH(new float3[numCoefs]{});

    struct State {
        State() = default;
        explicit State(size_t numCoefs) : numCoefs(numCoefs) { }

        State& operator=(State const & rhs) {
            SH.reset(new float3[rhs.numCoefs]{}); // NOLINT(modernize-make-unique)
            SHb.reset(new float[rhs.numCoefs]{}); // NOLINT(modernize-make-unique)
            return *this;
        }
        size_t numCoefs = 0;
        std::unique_ptr<float3[]> SH;
        std::unique_ptr<float[]> SHb;
    } prototype(numCoefs);

    CubemapUtils::process<State>(const_cast<Cubemap&>(cm), js,
            [&](State& state, size_t y, Cubemap::Face f, Cubemap::Texel const* data, size_t dim) {
        for (size_t x=0 ; x<dim ; ++x, ++data) {

            float3 s(cm.getDirectionFor(f, x, y));

            // sample a color
            float3 color(Cubemap::sampleAt(data));

            // take solid angle into account
            color *= CubemapUtils::solidAngle(dim, x, y);

            computeShBasis(state.SHb.get(), numBands, s);

            // apply coefficients to the sampled color
            for (size_t i=0 ; i<numCoefs ; i++) {
                state.SH[i] += color * state.SHb[i];
            }
        }
    },
    [&](State& state) {
        for (size_t i=0 ; i<numCoefs ; i++) {
            SH[i] += state.SH[i];
        }
    }, prototype);

    // precompute the scaling factor K
    for (size_t l = 0; l < numBands; l++) {
        const float truncatedCosSh = irradiance ? (computeTruncatedCosSh(size_t(l)) * M_1_PI) : 1;
        float K = Kml(0, l);
        SH[SHindex(0, l)] *= K * truncatedCosSh;
        for (size_t m = 1; m <= l; m++) {
            K = M_SQRT2 * Kml(m, l);
            SH[SHindex(-m, l)] *= K * truncatedCosSh;
            SH[SHindex( m, l)] *= K * truncatedCosSh;
        }
    }

    return SH;
}

void CubemapSH::renderSH(JobSystem& js, Cubemap& cm,
        const std::unique_ptr<float3[]>& sh, size_t numBands)
{
    const size_t numCoefs = numBands*numBands;
    std::vector<float> K(numCoefs);
    // precompute the scaling factor K
    for (size_t l=0 ; l<numBands ; l++) {
        K[SHindex(0, l)] = Kml(0, l);
        for (size_t m=1 ; m<=l ; m++) {
            K[SHindex( m, l)] =
            K[SHindex(-m, l)] = M_SQRT2 * Kml(m, l);
        }
    }

    CubemapUtils::process<CubemapUtils::EmptyState>(cm, js,
            [&](CubemapUtils::EmptyState&, size_t y,
                    Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
       std::vector<float> SHb(numCoefs);
        for (size_t x=0 ; x<dim ; ++x, ++data) {
            float3 s(cm.getDirectionFor(f, x, y));
            computeShBasis(SHb.data(), numBands, s);
            float3 c = 0;
            for (size_t i=0 ; i<numCoefs ; i++) {
                c += sh[i] * (K[i] * SHb[i]);
            }
            Cubemap::writeAt(data, Cubemap::Texel(c));
        }
    });
}

/*
 * This computes the 3-bands SH coefficients of the Cubemap convoluted by the
 * truncated cos(theta) (i.e.: saturate(s.z)), pre-scaled by the reconstruction
 * factors.
 */
std::unique_ptr<float3[]> CubemapSH::computeIrradianceSH3Bands(JobSystem& js, const Cubemap& cm) {

    const size_t numCoefs = 9;

    std::unique_ptr<float3[]> SH(new float3[numCoefs]{});
    std::unique_ptr<float[]> A(new float[numCoefs]{});

    const float c0 = computeTruncatedCosSh(0);
    const float c1 = computeTruncatedCosSh(1);
    const float c2 = computeTruncatedCosSh(2);
    A[0] = (M_1_PI * M_1_PI / 4)       * c0;
    A[1] = (M_1_PI * M_1_PI / 4) * 3   * c1;
    A[2] = (M_1_PI * M_1_PI / 4) * 3   * c1;
    A[3] = (M_1_PI * M_1_PI / 4) * 3   * c1;
    A[4] = (M_1_PI * M_1_PI / 4) * 15  * c2;
    A[5] = (M_1_PI * M_1_PI / 4) * 15  * c2;
    A[6] = (M_1_PI * M_1_PI /16) * 5   * c2;
    A[7] = (M_1_PI * M_1_PI / 4) * 15  * c2;
    A[8] = (M_1_PI * M_1_PI /16) * 15  * c2;

    struct State {
        float3 SH[9] = { };
    };

    CubemapUtils::process<State>(const_cast<Cubemap&>(cm), js,
            [&](State& state, size_t y, Cubemap::Face f, Cubemap::Texel const* data, size_t dim) {
        for (size_t x=0 ; x<dim ; ++x, ++data) {

            float3 s(cm.getDirectionFor(f, x, y));

            // sample a color
            float3 color(Cubemap::sampleAt(data));

            // take solid angle into account
            color *= CubemapUtils::solidAngle(dim, x, y);

            state.SH[0] += color * A[0];
            state.SH[1] += color * A[1] * s.y;
            state.SH[2] += color * A[2] * s.z;
            state.SH[3] += color * A[3] * s.x;
            state.SH[4] += color * A[4] * s.y * s.x;
            state.SH[5] += color * A[5] * s.y * s.z;
            state.SH[6] += color * A[6] * (3 * s.z * s.z - 1);
            state.SH[7] += color * A[7] * s.z * s.x;
            state.SH[8] += color * A[8] * (s.x * s.x - s.y * s.y);
        }
    },
    [&](State& state) {
        for (size_t i=0 ; i<numCoefs ; i++) {
            SH[i] += state.SH[i];
        }
    });

    return SH;
}

void CubemapSH::renderPreScaledSH3Bands(JobSystem& js,
        Cubemap& cm, const std::unique_ptr<filament::math::float3[]>& sh) {
    CubemapUtils::process<CubemapUtils::EmptyState>(cm, js,
            [&](CubemapUtils::EmptyState&, size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
        for (size_t x=0 ; x<dim ; ++x, ++data) {
            float3 s(cm.getDirectionFor(f, x, y));
            float3 c = 0;
            c += sh[0];
            c += sh[1] * s.y;
            c += sh[2] * s.z;
            c += sh[3] * s.x;
            c += sh[4] * s.y * s.x;
            c += sh[5] * s.y * s.z;
            c += sh[6] * (3 * s.z * s.z - 1);
            c += sh[7] * s.z * s.x;
            c += sh[8] * (s.x * s.x - s.y * s.y);
            Cubemap::writeAt(data, Cubemap::Texel(c));
        }
    });
}

// -----------------------------------------------------------------------------------------------
// Only used for debugging
// -----------------------------------------------------------------------------------------------

float UTILS_UNUSED CubemapSH::Legendre(ssize_t l, ssize_t m, float x) {
    // evaluate an Associated Legendre Polynomial P(l,m,x) at x
    float pmm = 1.0;
    if (m > 0) {
        float somx2 = sqrt((1.0f - x) * (1.0f + x));
        float fact = 1.0f;
        for (int i = 1; i <= m; i++) {
            pmm *= (-fact) * somx2;
            fact += 2.0f;
        }
    }
    if (l == m)
        return pmm;
    float pmmp1 = x * (2.0f * m + 1.0f) * pmm;
    if (l == m + 1)
        return pmmp1;
    float pll = 0.0;
    for (ssize_t ll = m + 2; ll <= l; ++ll) {
        pll = ((2.0f * ll - 1.0f) * x * pmmp1 - (ll + m - 1.0f) * pmm) / (ll - m);
        pmm = pmmp1;
        pmmp1 = pll;
    }
    return pll;
}

// Only used for debugging
float UTILS_UNUSED CubemapSH::TSH(int l, int m, const float3& d) {
    if (l==0 && m==0) {
        return 1 / (2*sqrt(M_PI));
    } else if (l==1 && m==-1) {
        return -(sqrt(3)*d.y)/(2*sqrt(M_PI));
    } else if (l==1 && m==0) {
        return  (sqrt(3)*d.z)/(2*sqrt(M_PI));
    } else if (l==1 && m==1) {
        return -(sqrt(3)*d.x)/(2*sqrt(M_PI));
    } else if (l==2 && m==-2) {
        return (sqrt(15)*d.y*d.x)/(2*sqrt(M_PI));
    } else if (l==2 && m==-1) {
        return -(sqrt(15)*d.y*d.z)/(2*sqrt(M_PI));
    } else if (l==2 && m==0) {
        return (sqrt(5)*(3*d.z*d.z-1))/(4*sqrt(M_PI));
    } else if (l==2 && m==1) {
        return -(sqrt(15)*d.z*d.x)/(2*sqrt(M_PI));
    } else if (l==2 && m==2) {
        return (sqrt(15)*(d.x*d.x - d.y*d.y))/(4*sqrt(M_PI));
    }
    return 0;
}

void UTILS_UNUSED CubemapSH::printShBase(std::ostream& out, int l, int m) {
    if (l<3 && std::abs(m) <= l) {
        const char* d = nullptr;
        float c = 0;
        if (l==0 && m==0) {
            c = M_2_SQRTPI * 0.25;
            d = "               ";
        } else if (l==1 && m==-1) {
            c = -M_2_SQRTPI * sqrt(3) * 0.25;
            d = " * y;          ";
        } else if (l==1 && m==0) {
            c = M_2_SQRTPI * sqrt(3) * 0.25;
            d = " * z;          ";
        } else if (l==1 && m==1) {
            c = -M_2_SQRTPI * sqrt(3) * 0.25;
            d = " * x;          ";
        } else if (l==2 && m==-2) {
            c = M_2_SQRTPI * sqrt(15) * 0.25;
            d = " * y*x;        ";
        } else if (l==2 && m==-1) {
            c = -M_2_SQRTPI * sqrt(15) * 0.25;
            d = " * y*z;        ";
        } else if (l==2 && m==0) {
            c =  M_2_SQRTPI * sqrt(5) * 0.125;
            d = " * (3*z*z -1); ";
        } else if (l==2 && m==1) {
            c = -M_2_SQRTPI * sqrt(15) * 0.25;
            d = " * z*x;        ";
        } else if (l==2 && m==2) {
            c = M_2_SQRTPI * sqrt(15) * 0.125;
            d = " * (x*x - y*y);";
        }
        out << "SHb[" << SHindex(m, size_t(l)) << "] = ";
        out << std::fixed << std::setprecision(15) << std::setw(18) << c << d;
        out << " // L" << l << m;
    }
}

} // namespace ibl
} // namespace filament
