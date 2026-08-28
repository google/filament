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

#include "Culler.h"

#include <filament/Box.h>

#include <math/fast.h>

#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif

using namespace filament::math;

// use 8 if Culler::result_type is 8-bits, on ARMv8 it allows the compiler to write eight
// results in one go.
#define FILAMENT_CULLER_VECTORIZE_HINT 4

namespace filament {

static_assert(Culler::MODULO % FILAMENT_CULLER_VECTORIZE_HINT == 0,
        "MODULO m=must be a multiple of FILAMENT_CULLER_VECTORIZE_HINT");





void Culler::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float const* UTILS_RESTRICT cx,
        float const* UTILS_RESTRICT cy,
        float const* UTILS_RESTRICT cz,
        float const* UTILS_RESTRICT ex,
        float const* UTILS_RESTRICT ey,
        float const* UTILS_RESTRICT ez,
        size_t count, size_t const bit) noexcept {

    float4 const * UTILS_RESTRICT const planes = frustum.mPlanes;
    count = round(count);

#if defined(__ARM_NEON) && defined(__aarch64__)
    float32x4_t const p0 = vld1q_f32((const float*)&planes[0]);
    float32x4_t const p1 = vld1q_f32((const float*)&planes[1]);
    float32x4_t const p2 = vld1q_f32((const float*)&planes[2]);
    float32x4_t const p3 = vld1q_f32((const float*)&planes[3]);
    float32x4_t const p4 = vld1q_f32((const float*)&planes[4]);
    float32x4_t const p5 = vld1q_f32((const float*)&planes[5]);

    float32x4_t const ap0 = vnegq_f32(vabsq_f32(p0));
    float32x4_t const ap1 = vnegq_f32(vabsq_f32(p1));
    float32x4_t const ap2 = vnegq_f32(vabsq_f32(p2));
    float32x4_t const ap3 = vnegq_f32(vabsq_f32(p3));
    float32x4_t const ap4 = vnegq_f32(vabsq_f32(p4));
    float32x4_t const ap5 = vnegq_f32(vabsq_f32(p5));

    float32x4_t const pw0 = vdupq_laneq_f32(p0, 3);
    float32x4_t const pw1 = vdupq_laneq_f32(p1, 3);
    float32x4_t const pw2 = vdupq_laneq_f32(p2, 3);
    float32x4_t const pw3 = vdupq_laneq_f32(p3, 3);
    float32x4_t const pw4 = vdupq_laneq_f32(p4, 3);
    float32x4_t const pw5 = vdupq_laneq_f32(p5, 3);

    uint8x8_t const bitMask = vdup_n_u8(uint8_t(1u << bit));
    float32x4_t const zero = vdupq_n_f32(0.0f);

    size_t i = 0;
    for (; i + 8 <= count; i += 8) {
        float32x4_t const cx0 = vld1q_f32(cx + i);
        float32x4_t const cx1 = vld1q_f32(cx + i + 4);
        float32x4_t const cy0 = vld1q_f32(cy + i);
        float32x4_t const cy1 = vld1q_f32(cy + i + 4);
        float32x4_t const cz0 = vld1q_f32(cz + i);
        float32x4_t const cz1 = vld1q_f32(cz + i + 4);

        float32x4_t const ex0 = vld1q_f32(ex + i);
        float32x4_t const ex1 = vld1q_f32(ex + i + 4);
        float32x4_t const ey0 = vld1q_f32(ey + i);
        float32x4_t const ey1 = vld1q_f32(ey + i + 4);
        float32x4_t const ez0 = vld1q_f32(ez + i);
        float32x4_t const ez1 = vld1q_f32(ez + i + 4);

        #define CULLER_BOX_SOA_PLANE_DOT(pw, p, ap, dotA, dotB) \
            dotA = vfmaq_laneq_f32(pw, cx0, p, 0); \
            dotB = vfmaq_laneq_f32(pw, cx1, p, 0); \
            dotA = vfmaq_laneq_f32(dotA, ex0, ap, 0); \
            dotB = vfmaq_laneq_f32(dotB, ex1, ap, 0); \
            dotA = vfmaq_laneq_f32(dotA, cy0, p, 1); \
            dotB = vfmaq_laneq_f32(dotB, cy1, p, 1); \
            dotA = vfmaq_laneq_f32(dotA, ey0, ap, 1); \
            dotB = vfmaq_laneq_f32(dotB, ey1, ap, 1); \
            dotA = vfmaq_laneq_f32(dotA, cz0, p, 2); \
            dotB = vfmaq_laneq_f32(dotB, cz1, p, 2); \
            dotA = vfmaq_laneq_f32(dotA, ez0, ap, 2); \
            dotB = vfmaq_laneq_f32(dotB, ez1, ap, 2);

        float32x4_t maxA, maxB;
        CULLER_BOX_SOA_PLANE_DOT(pw0, p0, ap0, maxA, maxB)

        float32x4_t dA, dB;
        CULLER_BOX_SOA_PLANE_DOT(pw1, p1, ap1, dA, dB)
        maxA = vmaxq_f32(maxA, dA); maxB = vmaxq_f32(maxB, dB);

        CULLER_BOX_SOA_PLANE_DOT(pw2, p2, ap2, dA, dB)
        maxA = vmaxq_f32(maxA, dA); maxB = vmaxq_f32(maxB, dB);

        CULLER_BOX_SOA_PLANE_DOT(pw3, p3, ap3, dA, dB)
        maxA = vmaxq_f32(maxA, dA); maxB = vmaxq_f32(maxB, dB);

        CULLER_BOX_SOA_PLANE_DOT(pw4, p4, ap4, dA, dB)
        maxA = vmaxq_f32(maxA, dA); maxB = vmaxq_f32(maxB, dB);

        CULLER_BOX_SOA_PLANE_DOT(pw5, p5, ap5, dA, dB)
        maxA = vmaxq_f32(maxA, dA); maxB = vmaxq_f32(maxB, dB);

        #undef CULLER_BOX_SOA_PLANE_DOT

        uint32x4_t const vis0 = vcltq_f32(maxA, zero);
        uint32x4_t const vis1 = vcltq_f32(maxB, zero);

        uint16x8_t const vis16 = vcombine_u16(vmovn_u32(vis0), vmovn_u32(vis1));
        uint8x8_t const vis8 = vmovn_u16(vis16);

        uint8x8_t const orig = vld1_u8(&results[i]);
        uint8x8_t const updated = vorr_u8(vbic_u8(orig, bitMask), vand_u8(vis8, bitMask));

        vst1_u8(&results[i], updated);
    }

    for (; i < count; i++) {
        int visible = ~0;
        for (size_t j = 0; j < 6; j++) {
            const float dot =
                    planes[j].x * cx[i] - std::abs(planes[j].x) * ex[i] +
                    planes[j].y * cy[i] - std::abs(planes[j].y) * ey[i] +
                    planes[j].z * cz[i] - std::abs(planes[j].z) * ez[i] +
                    planes[j].w;

            visible &= fast::signbit(dot) << bit;
        }

        auto r = results[i];
        r &= ~result_type(1u << bit);
        r |= result_type(visible);
        results[i] = r;
    }
#else
#if defined(__clang__)
    #pragma clang loop vectorize_width(FILAMENT_CULLER_VECTORIZE_HINT)
#endif
    for (size_t i = 0; i < count; i++) {
        int visible = ~0;

#if defined(__clang__)
        #pragma clang loop unroll(full)
#endif
        for (size_t j = 0; j < 6; j++) {
            const float dot =
                    planes[j].x * cx[i] - std::abs(planes[j].x) * ex[i] +
                    planes[j].y * cy[i] - std::abs(planes[j].y) * ey[i] +
                    planes[j].z * cz[i] - std::abs(planes[j].z) * ez[i] +
                    planes[j].w;

            visible &= fast::signbit(dot) << bit;
        }

        auto r = results[i];
        r &= ~result_type(1u << bit);
        r |= result_type(visible);
        results[i] = r;
    }
#endif
}

void Culler::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float const* UTILS_RESTRICT cx,
        float const* UTILS_RESTRICT cy,
        float const* UTILS_RESTRICT cz,
        float const* UTILS_RESTRICT r,
        size_t count) noexcept {

    float4 const * const UTILS_RESTRICT planes = frustum.mPlanes;
    count = round(count);

#if defined(__ARM_NEON) && defined(__aarch64__)
    float32x4_t const p0 = vld1q_f32((const float*)&planes[0]);
    float32x4_t const p1 = vld1q_f32((const float*)&planes[1]);
    float32x4_t const p2 = vld1q_f32((const float*)&planes[2]);
    float32x4_t const p3 = vld1q_f32((const float*)&planes[3]);
    float32x4_t const p4 = vld1q_f32((const float*)&planes[4]);
    float32x4_t const p5 = vld1q_f32((const float*)&planes[5]);

    float32x4_t const pw0 = vdupq_laneq_f32(p0, 3);
    float32x4_t const pw1 = vdupq_laneq_f32(p1, 3);
    float32x4_t const pw2 = vdupq_laneq_f32(p2, 3);
    float32x4_t const pw3 = vdupq_laneq_f32(p3, 3);
    float32x4_t const pw4 = vdupq_laneq_f32(p4, 3);
    float32x4_t const pw5 = vdupq_laneq_f32(p5, 3);

    float32x4_t const zero = vdupq_n_f32(0.0f);

    size_t i = 0;
    for (; i + 16 <= count; i += 16) {
        float32x4_t const cx0 = vld1q_f32(cx + i);
        float32x4_t const cx1 = vld1q_f32(cx + i + 4);
        float32x4_t const cx2 = vld1q_f32(cx + i + 8);
        float32x4_t const cx3 = vld1q_f32(cx + i + 12);

        float32x4_t const cy0 = vld1q_f32(cy + i);
        float32x4_t const cy1 = vld1q_f32(cy + i + 4);
        float32x4_t const cy2 = vld1q_f32(cy + i + 8);
        float32x4_t const cy3 = vld1q_f32(cy + i + 12);

        float32x4_t const cz0 = vld1q_f32(cz + i);
        float32x4_t const cz1 = vld1q_f32(cz + i + 4);
        float32x4_t const cz2 = vld1q_f32(cz + i + 8);
        float32x4_t const cz3 = vld1q_f32(cz + i + 12);

        float32x4_t const cr0 = vld1q_f32(r + i);
        float32x4_t const cr1 = vld1q_f32(r + i + 4);
        float32x4_t const cr2 = vld1q_f32(r + i + 8);
        float32x4_t const cr3 = vld1q_f32(r + i + 12);

        #define CULLER_SPHERE_SOA_PLANE_DOT4(pw, p, dA, dB, dC, dD) \
            dA = vsubq_f32(pw, cr0); \
            dB = vsubq_f32(pw, cr1); \
            dC = vsubq_f32(pw, cr2); \
            dD = vsubq_f32(pw, cr3); \
            dA = vfmaq_laneq_f32(dA, cx0, p, 0); \
            dB = vfmaq_laneq_f32(dB, cx1, p, 0); \
            dC = vfmaq_laneq_f32(dC, cx2, p, 0); \
            dD = vfmaq_laneq_f32(dD, cx3, p, 0); \
            dA = vfmaq_laneq_f32(dA, cy0, p, 1); \
            dB = vfmaq_laneq_f32(dB, cy1, p, 1); \
            dC = vfmaq_laneq_f32(dC, cy2, p, 1); \
            dD = vfmaq_laneq_f32(dD, cy3, p, 1); \
            dA = vfmaq_laneq_f32(dA, cz0, p, 2); \
            dB = vfmaq_laneq_f32(dB, cz1, p, 2); \
            dC = vfmaq_laneq_f32(dC, cz2, p, 2); \
            dD = vfmaq_laneq_f32(dD, cz3, p, 2);

        float32x4_t maxA, maxB, maxC, maxD;
        CULLER_SPHERE_SOA_PLANE_DOT4(pw0, p0, maxA, maxB, maxC, maxD)

        float32x4_t dA, dB, dC, dD;
        CULLER_SPHERE_SOA_PLANE_DOT4(pw1, p1, dA, dB, dC, dD)
        maxA = vmaxq_f32(maxA, dA); maxB = vmaxq_f32(maxB, dB);
        maxC = vmaxq_f32(maxC, dC); maxD = vmaxq_f32(maxD, dD);

        CULLER_SPHERE_SOA_PLANE_DOT4(pw2, p2, dA, dB, dC, dD)
        maxA = vmaxq_f32(maxA, dA); maxB = vmaxq_f32(maxB, dB);
        maxC = vmaxq_f32(maxC, dC); maxD = vmaxq_f32(maxD, dD);

        CULLER_SPHERE_SOA_PLANE_DOT4(pw3, p3, dA, dB, dC, dD)
        maxA = vmaxq_f32(maxA, dA); maxB = vmaxq_f32(maxB, dB);
        maxC = vmaxq_f32(maxC, dC); maxD = vmaxq_f32(maxD, dD);

        CULLER_SPHERE_SOA_PLANE_DOT4(pw4, p4, dA, dB, dC, dD)
        maxA = vmaxq_f32(maxA, dA); maxB = vmaxq_f32(maxB, dB);
        maxC = vmaxq_f32(maxC, dC); maxD = vmaxq_f32(maxD, dD);

        CULLER_SPHERE_SOA_PLANE_DOT4(pw5, p5, dA, dB, dC, dD)
        maxA = vmaxq_f32(maxA, dA); maxB = vmaxq_f32(maxB, dB);
        maxC = vmaxq_f32(maxC, dC); maxD = vmaxq_f32(maxD, dD);

        #undef CULLER_SPHERE_SOA_PLANE_DOT4

        uint32x4_t const vis0 = vcltq_f32(maxA, zero);
        uint32x4_t const vis1 = vcltq_f32(maxB, zero);
        uint32x4_t const vis2 = vcltq_f32(maxC, zero);
        uint32x4_t const vis3 = vcltq_f32(maxD, zero);

        uint16x8_t const vis16_0 = vcombine_u16(vmovn_u32(vis0), vmovn_u32(vis1));
        uint16x8_t const vis16_1 = vcombine_u16(vmovn_u32(vis2), vmovn_u32(vis3));

        uint8x16_t const vis8 = vcombine_u8(vshr_n_u8(vmovn_u16(vis16_0), 7), vshr_n_u8(vmovn_u16(vis16_1), 7));

        vst1q_u8(&results[i], vis8);
    }

    for (; i < count; i++) {
        int visible = ~0;
        for (size_t j = 0; j < 6; j++) {
            const float dot = planes[j].x * cx[i] +
                              planes[j].y * cy[i] +
                              planes[j].z * cz[i] +
                              planes[j].w - r[i];
            visible &= fast::signbit(dot);
        }
        results[i] = result_type(visible);
    }
#else
#if defined(__clang__)
    #pragma clang loop vectorize_width(FILAMENT_CULLER_VECTORIZE_HINT)
#endif
    for (size_t i = 0; i < count; i++) {
        int visible = ~0;

#if defined(__clang__)
        #pragma clang loop unroll(full)
#endif
        for (size_t j = 0; j < 6; j++) {
            const float dot = planes[j].x * cx[i] +
                              planes[j].y * cy[i] +
                              planes[j].z * cz[i] +
                              planes[j].w - r[i];
            visible &= fast::signbit(dot);
        }
        results[i] = result_type(visible);
    }
#endif
}

/*
 * returns whether a box intersects with the frustum
 */
bool Culler::intersects(Frustum const& frustum, Box const& box) noexcept {
    // The main intersection routine assumes multiples of MODULO items
    float cx[MODULO] = { box.center.x };
    float cy[MODULO] = { box.center.y };
    float cz[MODULO] = { box.center.z };
    float ex[MODULO] = { box.halfExtent.x };
    float ey[MODULO] = { box.halfExtent.y };
    float ez[MODULO] = { box.halfExtent.z };
    result_type results[MODULO];
    intersects(results, frustum, cx, cy, cz, ex, ey, ez, MODULO, 0);
    return bool(results[0] & 1);
}

/*
 * returns whether a sphere intersects with the frustum
 */
bool Culler::intersects(Frustum const& frustum, float4 const& sphere) noexcept {
    // The main intersection routine assumes multiples of MODULO items
    float cx[MODULO] = { sphere.x };
    float cy[MODULO] = { sphere.y };
    float cz[MODULO] = { sphere.z };
    float r[MODULO]  = { sphere.w };
    result_type results[MODULO];
    intersects(results, frustum, cx, cy, cz, r, MODULO);
    return bool(results[0] & 1);
}

// For testing...

void Culler::Test::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float const* UTILS_RESTRICT cx,
        float const* UTILS_RESTRICT cy,
        float const* UTILS_RESTRICT cz,
        float const* UTILS_RESTRICT ex,
        float const* UTILS_RESTRICT ey,
        float const* UTILS_RESTRICT ez,
        size_t const count) noexcept {
    Culler::intersects(results, frustum, cx, cy, cz, ex, ey, ez, count, 0);
}

void Culler::Test::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float const* UTILS_RESTRICT cx,
        float const* UTILS_RESTRICT cy,
        float const* UTILS_RESTRICT cz,
        float const* UTILS_RESTRICT r,
        size_t const count) noexcept {
    Culler::intersects(results, frustum, cx, cy, cz, r, count);
}

} // namespace filament
