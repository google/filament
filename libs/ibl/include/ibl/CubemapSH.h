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

#include <math/vec3.h>

#include <memory>
#include <vector>

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
     */
    static std::unique_ptr<filament::math::double3[]> computeSH(const Cubemap& cm, size_t numBands,
            bool irradiance);

    /**
     * Render given spherical harmonics into a cubemap
     */
    static void renderSH(Cubemap& cm,
            const std::unique_ptr<filament::math::double3[]>& sh, size_t numBands);

    /**
     * Compute spherical harmonics of the irradiance of the given cubemap.
     * The SH basis are pre-scaled for easier rendering
     */
    static std::unique_ptr<filament::math::double3[]> computeIrradianceSH3Bands(const Cubemap& cm);

    /**
     * Render pre-scaled irrandiance SH
     */
    static void renderPreScaledSH3Bands(Cubemap& cm,
            const std::unique_ptr<filament::math::double3[]>& sh);

    static size_t getShIndex(ssize_t m, size_t l) {
        return SHindex(m, l);
    }

private:
    static inline size_t SHindex(ssize_t m, size_t l) {
        return l * (l + 1) + m;
    }

    static void computeShBasis(
            double* SHb,
            size_t numBands,
            const filament::math::double3& s);

    static double Kml(ssize_t m, size_t l);

    static double computeTruncatedCosSh(size_t l);

    // debugging only...
    static double Legendre(ssize_t l, ssize_t m, double x);
    static double TSH(int l, int m, const filament::math::double3& d);
    static void printShBase(std::ostream& out, int l, int m);
};

} // namespace ibl
} // namespace filament

#endif /* IBL_CUBEMAPSH_H */
