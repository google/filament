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

#include <vector>

#include "CubemapIBL.h"

#include "math/mat3.h"
#include <math/scalar.h>
#include <utils/JobSystem.h>

#include "Cubemap.h"
#include "CubemapUtils.h"
#include "ProgressUpdater.h"
#include "utilities.h"

using namespace math;
using namespace image;
using namespace utils;

extern bool g_quiet;

static double pow5(double x) {
    return (x*x)*(x*x)*x;
}

static double3 hemisphereImportanceSampleDggx(double2 u, double a) {
    const double phi = 2 * M_PI * u.x;
    // NOTE: (aa-1) == (a-1)(a+1) produces better fp accuracy
    const double cosTheta2 = (1 - u.y) / (1 + (a + 1) * ((a - 1) * u.y));
    const double cosTheta = std::sqrt(cosTheta2);
    const double sinTheta = std::sqrt(1 - cosTheta2);
    return { sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta };
}

static double3 __UNUSED hemisphereCosSample(double2 u) {
    const double phi = 2 * M_PI * u.x;
    const double cosTheta2 = 1 - u.y;
    const double cosTheta = std::sqrt(cosTheta2);
    const double sinTheta = std::sqrt(1 - cosTheta2);
    return { sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta };
}

static double DistributionGGX(double NoH, double linearRoughness) {
    // NOTE: (aa-1) == (a-1)(a+1) produces better fp accuracy
    double a = linearRoughness;
    double f = (a - 1) * ((a + 1) * (NoH * NoH)) + 1.0;
    return (a * a) / (M_PI * f * f);
}

static double __UNUSED DistributionAshikhmin(double NoH, double linearRoughness) {
    double a = linearRoughness;
    double a2 = a * a;
    double cos2h = NoH * NoH;
    double sin2h = 1 - cos2h;
    double sin4h = sin2h * sin2h;
    return 1 / (M_PI * (1 + 4 * a2)) * (sin4h + 4 * std::exp(-cos2h / (a2 * sin2h)));
}

static double __UNUSED DistributionCharlie(double NoH, double linearRoughness) {
    // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
    double a = linearRoughness;
    double invAlpha = 1 / a;
    double cos2h = NoH * NoH;
    double sin2h = 1 - cos2h;
    return (2 + invAlpha) * std::pow(sin2h, invAlpha * 0.5) / (2 * M_PI);
}

static double Fresnel(double f0, double f90, double LoH) {
    const double Fc = pow5(1 - LoH);
    return f0 * (1 - Fc) + f90 * Fc;
}

static double Visibility(double NoV, double NoL, double a) {
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    // Height-correlated GGX
    const double a2 = a * a;
    const double GGXL = NoV * std::sqrt((NoL - NoL * a2) * NoL + a2);
    const double GGXV = NoL * std::sqrt((NoV - NoV * a2) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

static double __UNUSED VisibilityAshikhmin(double NoV, double NoL, double a) {
    return 1 / (4 * (NoL + NoV - NoL * NoV));
}


/*
 *
 * Importance sampling GGX - Trowbridge-Reitz
 * ------------------------------------------
 *
 * Important samples are chosen to integrate Dggx() * cos(theta) over the hemisphere.
 *
 * All calculations are made in tangent space, with n = [0 0 1]
 *
 *             l        h (important sample)
 *             .\      /.
 *             . \    / .
 *             .  \  /  .
 *             .   \/   .
 *         ----+---o----+-------> n [0 0 1]
 *     cos(2*theta)     cos(theta)
 *        = n•l            = n•h
 *
 *  v = n
 *  f0 = f90 = 1
 *  V = 1
 *
 *  h is micro facet's normal
 *
 *  l is the reflection of v (i.e.: n) around h  ==>  n•h = l•h = v•h
 *
 *  h = important_sample_ggx()
 *
 *  n•h = [0 0 1]•h = h.z
 *
 *  l = reflect(-n, h)
 *    = 2 * (n•h) * h - n;
 *
 *  n•l = cos(2 * theta)
 *      = cos(theta)^2 - sin(theta)^2
 *      = (n•h)^2 - (1 - (n•h)^2)
 *      = 2(n•h)^2 - 1
 *
 *
 *  pdf() = D(h) <n•h> |J(h)|
 *
 *               1
 *  |J(h)| = ----------
 *            4 <v•h>
 *
 *
 * Pre-filtered importance sampling
 * --------------------------------
 *
 *  see: "Real-time Shading with Filtered Importance Sampling", Jaroslav Krivanek
 *  see: "GPU-Based Importance Sampling, GPU Gems 3", Mark Colbert
 *
 *
 *                   Ωs
 *     lod = log4(K ----)
 *                   Ωp
 *
 *     log4(K) = 1, works well for box filters
 *     K = 4
 *
 *             1
 *     Ωs = ---------, solid-angle of an important sample
 *           N * pdf
 *
 *              4 PI
 *     Ωp ~ --------------, solid-angle of a sample in the base cubemap
 *           texel_count
 *
 *
 * Evaluating the integral
 * -----------------------
 *
 *                    K     fr(h)
 *            Er() = --- ∑ ------- L(h) <n•l>
 *                    N  h   pdf
 *
 * with:
 *
 *            fr() = D(h)
 *
 *                       N
 *            K = -----------------
 *                    fr(h)
 *                 ∑ ------- <n•l>
 *                 h   pdf
 *
 *
 *  It results that:
 *
 *            K           4 <v•h>
 *    Er() = --- ∑ D(h) ------------ L(h) <n•l>
 *            N  h        D(h) <n•h>
 *
 *
 *              K
 *    Er() = 4 --- ∑ L(h) <n•l>
 *              N  h
 *
 *                  N       4
 *    Er() = ------------- --- ∑ V(v) <n•l>
 *             4 ∑ <n•l>    N
 *
 *
 *  +------------------------------+
 *  |          ∑ <n•l> L(h)        |
 *  |  Er() = --------------       |
 *  |            ∑ <n•l>           |
 *  +------------------------------+
 *
 */

void CubemapIBL::roughnessFilter(Cubemap& dst,
        const std::vector<Cubemap>& levels, double linearRoughness, size_t maxNumSamples)
{
    const float numSamples = maxNumSamples;
    const float inumSamples = 1.0f / numSamples;
    const size_t maxLevel = levels.size()-1;
    const float maxLevelf = maxLevel;
    const Cubemap& base(levels[0]);
    const size_t dim0 = base.getDimensions();
    const float omegaP = float((4 * M_PI) / (6 * dim0 * dim0));

    ProgressUpdater updater(1);
    std::atomic_uint progress = {0};

    if (linearRoughness == 0) {
        if (!g_quiet) {
            updater.start();
        }
        CubemapUtils::process<CubemapUtils::EmptyState>(dst, [&, quiet = g_quiet]
                (CubemapUtils::EmptyState&, size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
                    size_t p = progress.fetch_add(1, std::memory_order_relaxed) + 1;
                    if (!quiet) {
                        updater.update(0, p, dim * 6);
                    }
                    const Cubemap& cm = levels[0];
                    for (size_t x = 0; x < dim; ++x, ++data) {
                        const double2 p(dst.center(x, y));
                        const double3 N(dst.getDirectionFor(f, p.x, p.y));
                        // FIXME: we should pick the proper LOD here and do trilinear filtering
                        Cubemap::writeAt(data, cm.sampleAt(N));
                    }
                });
        if (!g_quiet) {
            updater.stop();
        }
        return;
    }


    // be careful w/ the size of this structure, the smaller the better
    struct CacheEntry {
        double3 L;
        float brdf_NoL;
        float lerp;
        uint8_t l0;
        uint8_t l1;
    };

    std::vector<CacheEntry> cache;
    cache.reserve(maxNumSamples);

    // precompute everything that only depends on the sample #
    double weight = 0;
    // index of the sample to use
    // our goal is to use maxNumSamples for which NoL is > 0
    // to achieve this, we might have to try more samples than
    // maxNumSamples
    for (size_t sampleIndex = 0 ; sampleIndex < maxNumSamples; sampleIndex++) {

        // get Hammersley distribution for the half-sphere
        const double2 u = hammersley(uint32_t(sampleIndex), inumSamples);

        // Importance sampling GGX - Trowbridge-Reitz
        const double3 H = hemisphereImportanceSampleDggx(u, linearRoughness);

#if 0
        // This produces the same result that the code below using the the non-simplified
        // equation. This let's us see that N == V and that L = -reflect(V, H)
        // Keep this for reference.
        const double3 N = {0, 0, 1};
        const double3 V = N;
        const double3 L = 2 * dot(H, V) * H - V;
        const double NoL = dot(N, L);
        const double NoH = dot(N, H);
        const double NoH2 = NoH * NoH;
        const double NoV = dot(N, V);
#else
        const double NoV = 1;
        const double NoH = H.z;
        const double NoH2 = H.z*H.z;
        const double NoL = 2*NoH2 - 1;
        const double3 L(2*NoH*H.x, 2*NoH*H.y, NoL);
#endif

        if (NoL > 0) {
            const double pdf = DistributionGGX(NoH, linearRoughness) / 4;

            // K is a LOD bias that allows a bit of overlapping between samples
            constexpr float K = 4;
            const double omegaS = 1 / (numSamples * pdf);
            const double l = float(log4(omegaS) - log4(omegaP) + log4(K));
            const float mipLevel = clamp(float(l), 0.0f, maxLevelf);

            const float brdf_NoL = float(NoL);

            weight += brdf_NoL;

            uint8_t l0 = uint8_t(mipLevel);
            uint8_t l1 = uint8_t(std::min(maxLevel, size_t(l0 + 1)));
            float lerp = mipLevel - l0;

            cache.push_back({ L, brdf_NoL, lerp, l0, l1 });
        }
    }

    std::for_each(cache.begin(), cache.end(), [weight](CacheEntry& entry){
        entry.brdf_NoL /= weight;
    });

    // we can sample the cubemap in any order, sort by the weight, it could improve fp precision
    std::sort(cache.begin(), cache.end(), [](CacheEntry const& lhs, CacheEntry const& rhs){
        return lhs.brdf_NoL < rhs.brdf_NoL;
    });

    if (!g_quiet) {
        updater.start();
    }

    CubemapUtils::process<CubemapUtils::EmptyState>(dst,
            [ &, quiet=g_quiet ](CubemapUtils::EmptyState&, size_t y, Cubemap::Face f, Cubemap::Texel* data,
                    size_t dim) {

        size_t p = progress.fetch_add(1, std::memory_order_relaxed) + 1;
        if (!quiet) {
            updater.update(0, p, dim * 6);
        }

        mat3 R;
        const size_t numSamples = cache.size();
        for (size_t x = 0; x < dim; ++x, ++data) {
            const double2 p(dst.center(x, y));
            const double3 N(dst.getDirectionFor(f, p.x, p.y));

            // center the cone around the normal (handle case of normal close to up)
            const double3 up = std::abs(N.z) < 0.999 ? double3(0, 0, 1) : double3(1, 0, 0);
            R[0] = normalize(cross(up, N));
            R[1] = cross(N, R[0]);
            R[2] = N;

            float3 Li = 0;
            for (size_t sample = 0; sample < numSamples; sample++) {
                const CacheEntry& e = cache[sample];
                const double3 L(R * e.L);
                const Cubemap& cmBase = levels[e.l0];
                const Cubemap& next = levels[e.l1];
                const float3 c0 = Cubemap::trilinearFilterAt(cmBase, next, e.lerp, L);
                Li += c0 * e.brdf_NoL;
            }
            Cubemap::writeAt(data, Cubemap::Texel(Li));
        }
    });

    if (!g_quiet) {
        updater.stop();
    }
}

/*
 *
 * Importance sampling
 * -------------------
 *
 * Important samples are chosen to integrate cos(theta) over the hemisphere.
 *
 * All calculations are made in tangent space, with n = [0 0 1]
 *
 *                      l (important sample)
 *                     /.
 *                    / .
 *                   /  .
 *                  /   .
 *         --------o----+-------> n (direction)
 *                   cos(theta)
 *                    = n•l
 *
 *
 *  'direction' is given as an input parameter, and serves as tge z direction of the tangent space.
 *
 *  l = important_sample_cos()
 *
 *  n•l = [0 0 1] • l = l.z
 *
 *           n•l
 *  pdf() = -----
 *           PI
 *
 *
 * Pre-filtered importance sampling
 * --------------------------------
 *
 *  see: "Real-time Shading with Filtered Importance Sampling", Jaroslav Krivanek
 *  see: "GPU-Based Importance Sampling, GPU Gems 3", Mark Colbert
 *
 *
 *                   Ωs
 *     lod = log4(K ----)
 *                   Ωp
 *
 *     log4(K) = 1, works well for box filters
 *     K = 4
 *
 *             1
 *     Ωs = ---------, solid-angle of an important sample
 *           N * pdf
 *
 *              4 PI
 *     Ωp ~ --------------, solid-angle of a sample in the base cubemap
 *           texel_count
 *
 *
 * Evaluating the integral
 * -----------------------
 *
 * We are trying to evaluate the following integral:
 * (we pre-multiply by PI to avoid a 1/PI in the shader)
 *
 *                       /
 *             Ed() = PI | L(s) <n•l> ds
 *                       /
 *                       Ω
 *
 * For this, we're using importance sampling:
 *
 *                    PI     L(l)
 *            Ed() = ---- ∑ ------- <n•l>
 *                    N   l   pdf
 *
 *
 *  It results that:
 *
 *            PI           n•l
 *    Ed() = ---- ∑ L(l) ------  <n•l>
 *            N   l        PI
 *
 *
 *  +----------------------+
 *  |          1           |
 *  |  Ed() = ---- ∑ L(l)  |
 *  |          N   l       |
 *  +----------------------+
 *
 */

void CubemapIBL::diffuseIrradiance(Cubemap& dst, const std::vector<Cubemap>& levels, size_t maxNumSamples)
{
    const float numSamples = maxNumSamples;
    const float inumSamples = 1.0f / numSamples;
    const size_t maxLevel = levels.size()-1;
    const float maxLevelf = maxLevel;
    const Cubemap& base(levels[0]);
    const size_t dim0 = base.getDimensions();
    const float omegaP = float((4 * M_PI) / (6 * dim0 * dim0));

    ProgressUpdater updater(1);
    std::atomic_uint progress = {0};


    struct CacheEntry {
        double3 L;
        float lerp;
        uint8_t l0;
        uint8_t l1;
    };

    std::vector<CacheEntry> cache;
    cache.reserve(maxNumSamples);

    // precompute everything that only depends on the sample #
    for (size_t sampleIndex = 0, sample = 0 ; sampleIndex < maxNumSamples; sampleIndex++) {
        // get Hammersley distribution for the half-sphere
        const double2 u = hammersley(uint32_t(sampleIndex), inumSamples);
        const double3 L = hemisphereCosSample(u);
        const double3 N = { 0, 0, 1 };
        const double NoL = dot(N, L);

        if (NoL > 0) {
            double pdf = NoL * M_1_PI;

            constexpr float K = 4;
            const double omegaS = 1.0 / (numSamples * pdf);
            const double l = float(log4(omegaS) - log4(omegaP) + log4(K));
            const float mipLevel = clamp(float(l), 0.0f, maxLevelf);

            uint8_t l0 = uint8_t(mipLevel);
            uint8_t l1 = uint8_t(std::min(maxLevel, size_t(l0 + 1)));
            float lerp = mipLevel - l0;

            cache.push_back({ L, lerp, l0, l1 });
        }
    }

    if (!g_quiet) {
        updater.start();
    }

    CubemapUtils::process<CubemapUtils::EmptyState>(dst,
            [ &, quiet=g_quiet ](CubemapUtils::EmptyState&, size_t y, Cubemap::Face f, Cubemap::Texel* data,
                    size_t dim) {

        size_t p = progress.fetch_add(1, std::memory_order_relaxed) + 1;
        if (!quiet) {
            updater.update(0, p, dim * 6);
        }

        mat3 R;
        const size_t numSamples = cache.size();
        for (size_t x = 0; x < dim; ++x, ++data) {
            const double2 p(dst.center(x, y));
            const double3 N(dst.getDirectionFor(f, p.x, p.y));

            // center the cone around the normal (handle case of normal close to up)
            const double3 up = std::abs(N.z) < 0.999 ? double3(0, 0, 1) : double3(1, 0, 0);
            R[0] = normalize(cross(up, N));
            R[1] = cross(N, R[0]);
            R[2] = N;

            float3 Li = 0;
            for (size_t sample = 0; sample < numSamples; sample++) {
                const CacheEntry& e = cache[sample];
                const double3 L(R * e.L);
                const Cubemap& cmBase = levels[e.l0];
                const Cubemap& next = levels[e.l1];
                const float3 c0 = Cubemap::trilinearFilterAt(cmBase, next, e.lerp, L);
                Li += c0;
            }
            Cubemap::writeAt(data, Cubemap::Texel(Li * inumSamples));
        }
    });

    if (!g_quiet) {
        updater.stop();
    }
}

// Not importance-sampled
static double2 __UNUSED DFV_NoIS(double NoV, double roughness, size_t numSamples) {
    double2 r = 0;
    const double linearRoughness = roughness * roughness;
    const double3 V(std::sqrt(1 - NoV * NoV), 0, NoV);
    for (size_t i=0 ; i<numSamples ; i++) {
        const double2 u = hammersley(uint32_t(i), 1.0f / numSamples);
        const double3 H = hemisphereCosSample(u);
        const double3 L = 2 * dot(V, H)*H - V;
        const double VoH = saturate(dot(V, H));
        const double NoL = saturate(L.z);
        const double NoH = saturate(H.z);
        if (NoL > 0) {
            // Note: remember VoH == LoH  (H is half vector)
            const double v = Visibility(NoV, NoL, linearRoughness) * NoL * (VoH / NoH);
            const double Fc = pow5(1 - VoH);
            const double d = DistributionGGX(NoH, linearRoughness);
            r.x += d * v * (1.0 - Fc);
            r.y += d * v * Fc;
        }
    }
    return r * (M_PI * 4.0 / numSamples);
}

/*
 *
 * Importance sampling GGX - Trowbridge-Reitz
 * ------------------------------------------
 *
 * Important samples are chosen to integrate Dggx() * cos(theta) over the hemisphere.
 *
 * All calculations are made in tangent space, with n = [0 0 1]
 *
 *                      h (important sample)
 *                     /.
 *                    / .
 *                   /  .
 *                  /   .
 *         --------o----+-------> n
 *                   cos(theta)
 *                    = n•h
 *
 *  h is micro facet's normal
 *  l is the reflection of v around h, l = reflect(-v, h)  ==>  v•h = l•h
 *
 *  n•v is given as an input parameter at runtime
 *
 *  Since n = [0 0 1], we also have v.z = n•v
 *
 *  Since we need to compute v•h, we chose v as below. This choice only affects the
 *  computation of v•h (and therefore the fresnel term too), but doesn't affect
 *  n•l, which only relies on l.z (which itself only relies on v.z, i.e.: n•v)
 *
 *      | sqrt(1 - (n•v)^2)     (sin)
 *  v = | 0
 *      | n•v                   (cos)
 *
 *
 *  h = important_sample_ggx()
 *
 *  l = reflect(-v, h) = 2 * v•h * h - v;
 *
 *  n•l = [0 0 1] • l = l.z
 *
 *  n•h = [0 0 1] • l = h.z
 *
 *
 *  pdf() = D(h) <n•h> |J(h)|
 *
 *               1
 *  |J(h)| = ----------
 *            4 <v•h>
 *
 *
 * Evaluating the integral
 * -----------------------
 *
 * We are trying to evaluate the following integral:
 *
 *                    /
 *             Er() = | fr(s) <n•l> ds
 *                    /
 *                    Ω
 *
 * For this, we're using importance sampling:
 *
 *                    1     fr(h)
 *            Er() = --- ∑ ------- <n•l>
 *                    N  h   pdf
 *
 * with:
 *
 *            fr() = D(h) F(h) V(v, l)
 *
 *
 *  It results that:
 *
 *            1                        4 <v•h>
 *    Er() = --- ∑ D(h) F(h) V(v, l) ------------ <n•l>
 *            N  h                     D(h) <n•h>
 *
 *
 *  +-------------------------------------------+
 *  |          4                  <v•h>         |
 *  |  Er() = --- ∑ F(h) V(v, l) ------- <n•l>  |
 *  |          N  h               <n•h>         |
 *  +-------------------------------------------+
 *
 */

static double2 DFV(double NoV, double linearRoughness, size_t numSamples) {
    double2 r = 0;
    const double3 V(std::sqrt(1 - NoV * NoV), 0, NoV);
    for (size_t i=0 ; i<numSamples ; i++) {
        const double2 u = hammersley(uint32_t(i), 1.0f / numSamples);
        const double3 H = hemisphereImportanceSampleDggx(u, linearRoughness);
        const double3 L = 2 * dot(V, H)*H - V;
        const double VoH = saturate(dot(V, H));
        const double NoL = saturate(L.z);
        const double NoH = saturate(H.z);
        if (NoL > 0) {
            /*
             * Fc = (1 - V•H)^5
             * F(h) = f0*(1 - Fc) + f90*Fc
             *
             * f0 and f90 are known at runtime, but thankfully can be factored out, allowing us
             * to split the integral in two terms and store both terms separately in a LUT.
             *
             * At runtime, we can reconstruct Er() exactly as below:
             *
             *            4                      <v•h>
             *   DFV.x = --- ∑ (1 - Fc) V(v, l) ------- <n•l>
             *            N  h                   <n•h>
             *
             *
             *            4                      <v•h>
             *   DFV.y = --- ∑ (    Fc) V(v, l) ------- <n•l>
             *            N  h                   <n•h>
             *
             *
             *   Er() = f0 * DFV.x + f90 * DFV.y
             *
             */
            const double v = Visibility(NoV, NoL, linearRoughness) * NoL * (VoH / NoH);
            const double Fc = pow5(1 - VoH);
            r.x += v * (1.0 - Fc);
            r.y += v * Fc;
        }
    }
    return r * (4.0 / numSamples);
}

static double2 DFV_Multiscatter(double NoV, double linearRoughness, size_t numSamples) {
    double2 r = 0;
    const double3 V(std::sqrt(1 - NoV * NoV), 0, NoV);
    for (size_t i = 0; i < numSamples; i++) {
        const double2 u = hammersley(uint32_t(i), 1.0f / numSamples);
        const double3 H = hemisphereImportanceSampleDggx(u, linearRoughness);
        const double3 L = 2 * dot(V, H) * H - V;
        const double VoH = saturate(dot(V, H));
        const double NoL = saturate(L.z);
        const double NoH = saturate(H.z);
        if (NoL > 0) {
            const double v = Visibility(NoV, NoL, linearRoughness) * NoL * (VoH / NoH);
            const double Fc = pow5(1 - VoH);
            /*
             * Assuming f90 = 1
             *   Fc = (1 - V•H)^5
             *   F(h) = f0*(1 - Fc) + Fc
             *
             * f0 and f90 are known at runtime, but thankfully can be factored out, allowing us
             * to split the integral in two terms and store both terms separately in a LUT.
             *
             * At runtime, we can reconstruct Er() exactly as below:
             *
             *            4                <v•h>
             *   DFV.x = --- ∑ Fc V(v, l) ------- <n•l>
             *            N  h             <n•h>
             *
             *
             *            4                <v•h>
             *   DFV.y = --- ∑    V(v, l) ------- <n•l>
             *            N  h             <n•h>
             *
             *
             *   Er() = (1 - f0) * DFV.x + f0 * DFV.y
             *
             *        = mix(DFV.xxx, DFV.yyy, f0)
             *
             */
            r.x += v * Fc;
            r.y += v;
        }
    }
    return r * (4.0 / numSamples);
}

void CubemapIBL::brdf(Cubemap& dst, double linearRoughness) {
    CubemapUtils::process<CubemapUtils::EmptyState>(dst,
            [ & ](CubemapUtils::EmptyState&, size_t y,
                    Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
                for (size_t x=0 ; x<dim ; ++x, ++data) {
                    const double2 p(dst.center(x, y));
                    const double3 H(dst.getDirectionFor(f, p.x, p.y));
                    const double3 N = { 0, 0, 1 };
                    const double3 V = N;
                    const double3 L = 2 * dot(H, V) * H - V;
                    const double NoL = dot(N, L);
                    const double NoH = dot(N, H);
                    const double NoV = dot(N, V);
                    const double LoH = dot(L, H);
                    float brdf_NoL = 0;
                    if (NoL > 0 && LoH > 0) {
                        const double D = DistributionGGX(NoH, linearRoughness);
                        const double F = Fresnel(0.04, 1.0, LoH);
                        const double V = Visibility(NoV, NoL, linearRoughness);
                        brdf_NoL = float(D * F * V * NoL);
                    }
                    Cubemap::writeAt(data, Cubemap::Texel{ brdf_NoL });
                }
            });
}

static double2 __UNUSED prefilteredDFG_Karis(double NoV, double roughness) {
    // see https://www.unrealengine.com/blog/physically-based-shading-on-mobile
    const double4 c0(-1.0, -0.0275, -0.572,  0.022);
    const double4 c1( 1.0,  0.0425,  1.040, -0.040);
    double4 r = roughness * c0 + c1;
    float a004 = (float) (std::min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y);
    return double2(-1.04, 1.04) * a004 + double2(r.z, r.w);
}

static double2 __UNUSED prefilteredDFG_Cloth_Ashikhmin(double NoV, double roughness) {
    const double4 c0(0.24, 0.93, 0.01, 0.2);
    const double4 c1(2.0, -1.3, 0.4, 0.03);
    double s = 1.0 - NoV;
    double e = s - c0.y;
    double g = c0.x * std::exp2(-(e * e) / (2.0 * c0.z)) + s * c0.w;
    double n = roughness * c1.x + c1.y;
    double r = std::max(1.0 - n * n, c1.z) * g;
    return {r, r * c1.w};
}

static double2 __UNUSED prefilteredDFG_Cloth_Charlie(double NoV, double roughness) {
    const double3 c0(0.95, 1250.0, 0.0095);
    const double4 c1(0.04, 0.2, 0.3, 0.2);
    double a = 1.0 - NoV;
    double b = 1.0 - roughness;
    double n = std::pow(c1.x + a, 64.0);
    double e = b - c0.x;
    double g = std::exp2(-(e * e) * c0.y);
    double f = b + c1.y;
    double c = n * g + c1.z * (a + c1.w) * roughness + f * f * std::pow(a, 8.0);
    double r = std::min(c, 18.0);
    return {r, r * c0.z};
}

void CubemapIBL::DFG(Image& dst, bool multiscatter) {
    auto dfvFunction = multiscatter ? ::DFV_Multiscatter : ::DFV;
    JobSystem& js = CubemapUtils::getJobSystem();
    auto job = jobs::parallel_for<char>(js, nullptr, nullptr, uint32_t(dst.getHeight()),
            [ &dst, dfvFunction ](char* d, size_t c) {
                const size_t width = dst.getWidth();
                const size_t height = dst.getHeight();
                size_t y0 = size_t(d);
                for (size_t y = y0; y < y0 + c; y++) {
                    Cubemap::Texel* UTILS_RESTRICT data =
                            static_cast<Cubemap::Texel*>(dst.getPixelRef(0, y));

                    const double coord = saturate((height - y + 0.5) / height);
                    // map the coordinate in the texture to a linear_roughness,
                    // here we're using ^2, but other mappings are possible.
                    // ==> coord = sqrt(linear_roughness)
                    const double linear_roughness = coord * coord;
                    for (size_t x = 0; x < height; x++, data++) {
                        // const double NoV = double(x) / (width-1);
                        const double NoV = saturate((x + 0.5) / width);
                        float3 r = { dfvFunction(NoV, linear_roughness, 1024), 0 };
                        *data = r;
                    }
                }
            }, jobs::CountSplitter<1, 8>());
    js.runAndWait(job);
    js.reset();
}
