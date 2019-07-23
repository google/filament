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

#ifndef IBL_CUBEMAPSH_H
#define IBL_CUBEMAPSH_H


#include <utils/compiler.h>

#include <math/mat3.h>
#include <math/vec3.h>

#include <memory>
#include <vector>

namespace utils {
class JobSystem;
} // namespace utils

namespace filament {
namespace ibl {

class Cubemap;

/**
 * Computes spherical harmonics
 */
class CubemapSH {
public:
    /**
     * Spherical Harmonics decomposition of the given cubemap
     * Optionally calculates irradiance by convolving with truncated cos.
     */
    static std::unique_ptr<math::float3[]> computeSH(
            utils::JobSystem& js, const Cubemap& cm, size_t numBands, bool irradiance);

    /**
     * Render given spherical harmonics into a cubemap
     */
    static void renderSH(utils::JobSystem& js, Cubemap& cm,
            const std::unique_ptr<math::float3[]>& sh, size_t numBands);

    static void windowSH(std::unique_ptr<math::float3[]>& sh, size_t numBands, float cutoff);

    /**
     * Compute spherical harmonics of the irradiance of the given cubemap.
     * The SH basis are pre-scaled for easier rendering by the shader. The resulting coefficients
     * are not spherical harmonics (as they're scalled by various factors). In particular they
     * cannot be rendered with renderSH() above. Instead use renderPreScaledSH3Bands() which
     * is exactly the code ran by our shader.
     */
    static void preprocessSHForShader(std::unique_ptr<math::float3[]>& sh);

    /**
     * Render pre-scaled irrandiance SH
     */
    static void renderPreScaledSH3Bands(utils::JobSystem& js, Cubemap& cm,
            const std::unique_ptr<math::float3[]>& sh);

    static constexpr size_t getShIndex(ssize_t m, size_t l) {
        return SHindex(m, l);
    }

private:
    class float5 {
        float v[5];
    public:
        float5() = default;
        constexpr float5(float a, float b, float c, float d, float e) : v{ a, b, c, d, e } {}
        constexpr float operator[](size_t i) const { return v[i]; }
        float& operator[](size_t i) { return v[i]; }
    };

    static inline const float5 multiply(const float5 M[5], float5 x) noexcept {
        return float5{
                M[0][0] * x[0] + M[1][0] * x[1] + M[2][0] * x[2] + M[3][0] * x[3] + M[4][0] * x[4],
                M[0][1] * x[0] + M[1][1] * x[1] + M[2][1] * x[2] + M[3][1] * x[3] + M[4][1] * x[4],
                M[0][2] * x[0] + M[1][2] * x[1] + M[2][2] * x[2] + M[3][2] * x[3] + M[4][2] * x[4],
                M[0][3] * x[0] + M[1][3] * x[1] + M[2][3] * x[2] + M[3][3] * x[3] + M[4][3] * x[4],
                M[0][4] * x[0] + M[1][4] * x[1] + M[2][4] * x[2] + M[3][4] * x[3] + M[4][4] * x[4]
        };
    };


    static inline constexpr size_t SHindex(ssize_t m, size_t l) {
        return l * (l + 1) + m;
    }

    static void computeShBasis(float* SHb, size_t numBands, const math::float3& s);

    static float Kml(ssize_t m, size_t l);

    static std::vector<float> Ki(size_t numBands);

    static constexpr float computeTruncatedCosSh(size_t l);

    static float sincWindow(size_t l, float w);

    static math::float3 rotateShericalHarmonicBand1(math::float3 band1, math::mat3f const& M);

    static float5 rotateShericalHarmonicBand2(float5 const& band2, math::mat3f const& M);

        // debugging only...
    static float Legendre(ssize_t l, ssize_t m, float x);
    static float TSH(int l, int m, const math::float3& d);
    static void printShBase(std::ostream& out, int l, int m);
};

} // namespace ibl
} // namespace filament

#endif /* IBL_CUBEMAPSH_H */
