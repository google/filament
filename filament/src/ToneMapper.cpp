/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <cstddef>
#include <filament/ToneMapper.h>

#include "ColorSpaceUtils.h"

#include <math/half.h>
#include <math/scalar.h>
#include <math/mat3.h>
#include <math/vec3.h>

#include <cmath>
#include <limits>
#include <new>

#if defined(__ARM_NEON)
#include <arm_neon.h>
#include "details/ColorGradingNeon.h"
#endif

namespace filament {

using namespace math;

namespace aces {

static float rgb_2_saturation(float3 const rgb) noexcept {
    // Input:  ACES
    // Output: OCES
    constexpr float TINY = 1e-5f;
    float const mi = min(rgb);
    float const ma = max(rgb);
    return (max(ma, TINY) - max(mi, TINY)) / max(ma, 1e-2f);
}

static float rgb_2_yc(float3 const rgb) noexcept {
    constexpr float ycRadiusWeight = 1.75f;

    // Converts RGB to a luminance proxy, here called YC
    // YC is ~ Y + K * Chroma
    // Constant YC is a cone-shaped surface in RGB space, with the tip on the
    // neutral axis, towards white.
    // YC is normalized: RGB 1 1 1 maps to YC = 1
    //
    // ycRadiusWeight defaults to 1.75, although it can be overridden in function
    // call to rgb_2_yc
    // ycRadiusWeight = 1 -> YC for pure cyan, magenta, yellow == YC for neutral
    // of same value
    // ycRadiusWeight = 2 -> YC for pure red, green, blue  == YC for  neutral of
    // same value.

    float const r = rgb.r;
    float const g = rgb.g;
    float const b = rgb.b;

    float const chroma = std::sqrt(b * (b - g) + g * (g - r) + r * (r - b));

    return (b + g + r + ycRadiusWeight * chroma) / 3.0f;
}

static float sigmoid_shaper(float const x) noexcept {
    // Sigmoid function in the range 0 to 1 spanning -2 to +2.
    float const t = max(1.0f - std::abs(x / 2.0f), 0.0f);
    float const y = 1.0f + sign(x) * (1.0f - t * t);
    return y / 2.0f;
}

static float glow_fwd(float const ycIn, float const glowGainIn, float const glowMid) noexcept {
    float glowGainOut;

    if (ycIn <= 2.0f / 3.0f * glowMid) {
        glowGainOut = glowGainIn;
    } else if ( ycIn >= 2.0f * glowMid) {
        glowGainOut = 0.0f;
    } else {
        glowGainOut = glowGainIn * (glowMid / ycIn - 1.0f / 2.0f);
    }

    return glowGainOut;
}

static float rgb_2_hue(float3 const rgb) noexcept {
    // Returns a geometric hue angle in degrees (0-360) based on RGB values.
    // For neutral colors, hue is undefined and the function will return a quiet NaN value.
    float hue = 0.0f;
    // RGB triplets where RGB are equal have an undefined hue
    if (!(rgb.x == rgb.y && rgb.y == rgb.z)) {
        hue = f::RAD_TO_DEG * std::atan2(
                std::sqrt(3.0f) * (rgb.y - rgb.z),
                2.0f * rgb.x - rgb.y - rgb.z);
    }
    return (hue < 0.0f) ? hue + 360.0f : hue;
}

static float center_hue(float const hue, float const centerH) noexcept {
    float hueCentered = hue - centerH;
    if (hueCentered < -180.0f) {
        hueCentered = hueCentered + 360.0f;
    } else if (hueCentered > 180.0f) {
        hueCentered = hueCentered - 360.0f;
    }
    return hueCentered;
}

static float3 darkSurround_to_dimSurround(float3 linearCV) noexcept {
    constexpr float DIM_SURROUND_GAMMA = 0.9811f;

    float3 XYZ = AP1_to_XYZ * linearCV;
    float3 xyY = XYZ_to_xyY(XYZ);

    xyY.z = clamp(xyY.z, 0.0f, float(std::numeric_limits<half>::max()));
    xyY.z = std::pow(xyY.z, DIM_SURROUND_GAMMA);

    XYZ = xyY_to_XYZ(xyY);
    return XYZ_to_AP1 * XYZ;
}

static float3 ACES(float3 color, float brightness) noexcept {
    // Some bits were removed to adapt to our desired output

    // "Glow" module constants
    constexpr float RRT_GLOW_GAIN = 0.05f;
    constexpr float RRT_GLOW_MID = 0.08f;

    // Red modifier constants
    constexpr float RRT_RED_SCALE = 0.82f;
    constexpr float RRT_RED_PIVOT = 0.03f;
    constexpr float RRT_RED_HUE   = 0.0f;
    constexpr float RRT_RED_WIDTH = 135.0f;

    // Desaturation constants
    constexpr float RRT_SAT_FACTOR = 0.96f;
    constexpr float ODT_SAT_FACTOR = 0.93f;

    float3 ap0 = Rec2020_to_AP0 * color;

    // Glow module
    float const saturation = rgb_2_saturation(ap0);
    float const ycIn = rgb_2_yc(ap0);
    float const s = sigmoid_shaper((saturation - 0.4f) / 0.2f);
    float const addedGlow = 1.0f + glow_fwd(ycIn, RRT_GLOW_GAIN * s, RRT_GLOW_MID);
    ap0 *= addedGlow;

    // Red modifier
    float const hue = rgb_2_hue(ap0);
    float const centeredHue = center_hue(hue, RRT_RED_HUE);
    float hueWeight = smoothstep(0.0f, 1.0f, 1.0f - std::abs(2.0f * centeredHue / RRT_RED_WIDTH));
    hueWeight *= hueWeight;

    ap0.r += hueWeight * saturation * (RRT_RED_PIVOT - ap0.r) * (1.0f - RRT_RED_SCALE);

    // ACES to RGB rendering space
    float3 ap1 = clamp(AP0_to_AP1 * ap0, 0.0f, (float) std::numeric_limits<half>::max());

    // Global desaturation
    ap1 = mix(float3(dot(ap1, LUMINANCE_AP1)), ap1, RRT_SAT_FACTOR);

    // NOTE: This is specific to Filament and added only to match ACES to our legacy tone mapper
    //       which was a fit of ACES in Rec.709 but with a brightness boost.
    ap1 *= brightness;

    // Fitting of RRT + ODT (RGB monitor 100 nits dim) from:
    // https://github.com/colour-science/colour-unity/blob/master/Assets/Colour/Notebooks/CIECAM02_Unity.ipynb
    constexpr float a = 2.785085f;
    constexpr float b = 0.107772f;
    constexpr float c = 2.936045f;
    constexpr float d = 0.887122f;
    constexpr float e = 0.806889f;
    float3 const rgbPost = (ap1 * (a * ap1 + b)) / (ap1 * (c * ap1 + d) + e);

    // Apply gamma adjustment to compensate for dim surround
    float3 linearCV = darkSurround_to_dimSurround(rgbPost);

    // Apply desaturation to compensate for luminance difference
    linearCV = mix(float3(dot(linearCV, LUMINANCE_AP1)), linearCV, ODT_SAT_FACTOR);

    return AP1_to_Rec2020 * linearCV;
}

} // namespace aces

//------------------------------------------------------------------------------
// Tone mappers
//------------------------------------------------------------------------------

#define DEFAULT_CONSTRUCTORS(A) \
        A::A() noexcept = default; \
        A::~A() noexcept = default;

DEFAULT_CONSTRUCTORS(ToneMapper)

#if defined(__ARM_NEON)
void ToneMapper::operator()(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb) const noexcept {
    alignas(16) float r[4];
    alignas(16) float g[4];
    alignas(16) float b[4];
    vst1q_f32(r, vr);
    vst1q_f32(g, vg);
    vst1q_f32(b, vb);

    float3 const c0 = (*this)(float3{r[0], g[0], b[0]});
    float3 const c1 = (*this)(float3{r[1], g[1], b[1]});
    float3 const c2 = (*this)(float3{r[2], g[2], b[2]});
    float3 const c3 = (*this)(float3{r[3], g[3], b[3]});

    vr = float32x4_t{c0.r, c1.r, c2.r, c3.r};
    vg = float32x4_t{c0.g, c1.g, c2.g, c3.g};
    vb = float32x4_t{c0.b, c1.b, c2.b, c3.b};
}
#endif

//------------------------------------------------------------------------------
// Linear tone mapper
//------------------------------------------------------------------------------

DEFAULT_CONSTRUCTORS(LinearToneMapper)

float3 LinearToneMapper::operator()(float3 const c) const noexcept {
    return saturate(c);
}

#if defined(__ARM_NEON)
void LinearToneMapper::operator()(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb) const noexcept {
    float32x4_t const zero = vdupq_n_f32(0.0f);
    float32x4_t const one = vdupq_n_f32(1.0f);
    vr = vmaxq_f32(vminq_f32(vr, one), zero);
    vg = vmaxq_f32(vminq_f32(vg, one), zero);
    vb = vmaxq_f32(vminq_f32(vb, one), zero);
}
#endif

//------------------------------------------------------------------------------
// ACES tone mappers
//------------------------------------------------------------------------------

#if defined(__ARM_NEON)
static void v_toneMappingACES(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb, float const brightness) noexcept {
    // Rec2020_to_AP0
    float32x4_t ap0_r = vaddq_f32(vaddq_f32(vmulq_n_f32(vr, Rec2020_to_AP0[0][0]), vmulq_n_f32(vg, Rec2020_to_AP0[1][0])), vmulq_n_f32(vb, Rec2020_to_AP0[2][0]));
    float32x4_t ap0_g = vaddq_f32(vaddq_f32(vmulq_n_f32(vr, Rec2020_to_AP0[0][1]), vmulq_n_f32(vg, Rec2020_to_AP0[1][1])), vmulq_n_f32(vb, Rec2020_to_AP0[2][1]));
    float32x4_t ap0_b = vaddq_f32(vaddq_f32(vmulq_n_f32(vr, Rec2020_to_AP0[0][2]), vmulq_n_f32(vg, Rec2020_to_AP0[1][2])), vmulq_n_f32(vb, Rec2020_to_AP0[2][2]));

    float32x4_t const mi = vminq_f32(vminq_f32(ap0_r, ap0_g), ap0_b);
    float32x4_t const ma = vmaxq_f32(vmaxq_f32(ap0_r, ap0_g), ap0_b);
    float32x4_t const sat = vdivq_f32(vsubq_f32(vmaxq_f32(ma, vdupq_n_f32(1e-5f)), vmaxq_f32(mi, vdupq_n_f32(1e-5f))), vmaxq_f32(ma, vdupq_n_f32(1e-2f)));

    float32x4_t const chroma_sq = vaddq_f32(vaddq_f32(
            vmulq_f32(ap0_b, vsubq_f32(ap0_b, ap0_g)),
            vmulq_f32(ap0_g, vsubq_f32(ap0_g, ap0_r))),
            vmulq_f32(ap0_r, vsubq_f32(ap0_r, ap0_b)));
    float32x4_t const chroma = vsqrtq_f32(vmaxq_f32(chroma_sq, vdupq_n_f32(0.0f)));
    float32x4_t const ycIn = vmulq_n_f32(vaddq_f32(vaddq_f32(ap0_b, ap0_g), vaddq_f32(ap0_r, vmulq_n_f32(chroma, 1.75f))), 1.0f / 3.0f);

    float32x4_t const x = vmulq_n_f32(vsubq_f32(sat, vdupq_n_f32(0.4f)), 5.0f); // / 0.2f
    float32x4_t const t = vmaxq_f32(vsubq_f32(vdupq_n_f32(1.0f), vabsq_f32(vmulq_n_f32(x, 0.5f))), vdupq_n_f32(0.0f));

    // sign(x)
    float32x4_t const x_sign = vbslq_f32(vcltq_f32(x, vdupq_n_f32(0.0f)), vdupq_n_f32(-1.0f),
            vbslq_f32(vcgtq_f32(x, vdupq_n_f32(0.0f)), vdupq_n_f32(1.0f), vdupq_n_f32(0.0f)));
    float32x4_t const s = vmulq_n_f32(vaddq_f32(vdupq_n_f32(1.0f),
            vmulq_f32(x_sign, vsubq_f32(vdupq_n_f32(1.0f), vmulq_f32(t, t)))), 0.5f);

    float32x4_t addedGlow = vdupq_n_f32(1.0f);
    uint32x4_t const cond1 = vcleq_f32(ycIn, vdupq_n_f32(2.0f / 3.0f * 0.08f));
    uint32x4_t const cond2 = vandq_u32(vmvnq_u32(cond1), vcltq_f32(ycIn, vdupq_n_f32(2.0f * 0.08f)));

    float32x4_t const glow1 = vmulq_n_f32(s, 0.05f);
    float32x4_t const glow2 = vmulq_f32(glow1, vsubq_f32(vdivq_f32(vdupq_n_f32(0.08f), ycIn), vdupq_n_f32(0.5f)));

    addedGlow = vaddq_f32(addedGlow, vbslq_f32(cond1, glow1, vbslq_f32(cond2, glow2, vdupq_n_f32(0.0f))));

    ap0_r = vmulq_f32(ap0_r, addedGlow);
    ap0_g = vmulq_f32(ap0_g, addedGlow);
    ap0_b = vmulq_f32(ap0_b, addedGlow);

    float32x4_t hue = vmulq_n_f32(v_atan2q_f32(vmulq_n_f32(vsubq_f32(ap0_g, ap0_b), 1.73205081f),
                                  vsubq_f32(vsubq_f32(vmulq_n_f32(ap0_r, 2.0f), ap0_g), ap0_b)), 57.29577951308f);

    uint32x4_t const eq_mask = vandq_u32(vceqq_f32(ap0_r, ap0_g), vceqq_f32(ap0_g, ap0_b));
    hue = vbslq_f32(eq_mask, vdupq_n_f32(0.0f), hue);
    hue = vbslq_f32(vcltq_f32(hue, vdupq_n_f32(0.0f)), vaddq_f32(hue, vdupq_n_f32(360.0f)), hue);

    float32x4_t centeredHue = hue; // - 0.0f
    centeredHue = vbslq_f32(vcltq_f32(centeredHue, vdupq_n_f32(-180.0f)), vaddq_f32(centeredHue, vdupq_n_f32(360.0f)), centeredHue);
    centeredHue = vbslq_f32(vcgtq_f32(centeredHue, vdupq_n_f32(180.0f)), vsubq_f32(centeredHue, vdupq_n_f32(360.0f)), centeredHue);

    // smoothstep(0, 1, 1 - abs(2*ch/135))
    float32x4_t const st = vmaxq_f32(vminq_f32(vsubq_f32(vdupq_n_f32(1.0f), vabsq_f32(vmulq_n_f32(centeredHue, 2.0f / 135.0f))), vdupq_n_f32(1.0f)), vdupq_n_f32(0.0f));
    float32x4_t hueWeight = vmulq_f32(vmulq_f32(st, st), vsubq_f32(vdupq_n_f32(3.0f), vmulq_n_f32(st, 2.0f)));
    hueWeight = vmulq_f32(hueWeight, hueWeight);

    ap0_r = vaddq_f32(ap0_r, vmulq_n_f32(vmulq_f32(vmulq_f32(hueWeight, sat), vsubq_f32(vdupq_n_f32(0.03f), ap0_r)), 0.18f)); // 1-0.82

    // AP0_to_AP1
    float32x4_t ap1_r = vaddq_f32(vaddq_f32(vmulq_n_f32(ap0_r, AP0_to_AP1[0][0]), vmulq_n_f32(ap0_g, AP0_to_AP1[1][0])), vmulq_n_f32(ap0_b, AP0_to_AP1[2][0]));
    float32x4_t ap1_g = vaddq_f32(vaddq_f32(vmulq_n_f32(ap0_r, AP0_to_AP1[0][1]), vmulq_n_f32(ap0_g, AP0_to_AP1[1][1])), vmulq_n_f32(ap0_b, AP0_to_AP1[2][1]));
    float32x4_t ap1_b = vaddq_f32(vaddq_f32(vmulq_n_f32(ap0_r, AP0_to_AP1[0][2]), vmulq_n_f32(ap0_g, AP0_to_AP1[1][2])), vmulq_n_f32(ap0_b, AP0_to_AP1[2][2]));

    float32x4_t const max_half = vdupq_n_f32(65504.0f);
    ap1_r = vmaxq_f32(vminq_f32(ap1_r, max_half), vdupq_n_f32(0.0f));
    ap1_g = vmaxq_f32(vminq_f32(ap1_g, max_half), vdupq_n_f32(0.0f));
    ap1_b = vmaxq_f32(vminq_f32(ap1_b, max_half), vdupq_n_f32(0.0f));

    float32x4_t luma = vaddq_f32(vaddq_f32(vmulq_n_f32(ap1_r, LUMINANCE_AP1.r), vmulq_n_f32(ap1_g, LUMINANCE_AP1.g)), vmulq_n_f32(ap1_b, LUMINANCE_AP1.b));
    ap1_r = vaddq_f32(luma, vmulq_n_f32(vsubq_f32(ap1_r, luma), 0.96f));
    ap1_g = vaddq_f32(luma, vmulq_n_f32(vsubq_f32(ap1_g, luma), 0.96f));
    ap1_b = vaddq_f32(luma, vmulq_n_f32(vsubq_f32(ap1_b, luma), 0.96f));

    ap1_r = vmulq_n_f32(ap1_r, brightness);
    ap1_g = vmulq_n_f32(ap1_g, brightness);
    ap1_b = vmulq_n_f32(ap1_b, brightness);

    // a=2.785085, b=0.107772, c=2.936045, d=0.887122, e=0.806889
    float32x4_t const num_r = vmulq_f32(ap1_r, vaddq_f32(vmulq_n_f32(ap1_r, 2.785085f), vdupq_n_f32(0.107772f)));
    float32x4_t const num_g = vmulq_f32(ap1_g, vaddq_f32(vmulq_n_f32(ap1_g, 2.785085f), vdupq_n_f32(0.107772f)));
    float32x4_t const num_b = vmulq_f32(ap1_b, vaddq_f32(vmulq_n_f32(ap1_b, 2.785085f), vdupq_n_f32(0.107772f)));

    float32x4_t const den_r = vaddq_f32(vmulq_f32(ap1_r, vaddq_f32(vmulq_n_f32(ap1_r, 2.936045f), vdupq_n_f32(0.887122f))), vdupq_n_f32(0.806889f));
    float32x4_t const den_g = vaddq_f32(vmulq_f32(ap1_g, vaddq_f32(vmulq_n_f32(ap1_g, 2.936045f), vdupq_n_f32(0.887122f))), vdupq_n_f32(0.806889f));
    float32x4_t const den_b = vaddq_f32(vmulq_f32(ap1_b, vaddq_f32(vmulq_n_f32(ap1_b, 2.936045f), vdupq_n_f32(0.887122f))), vdupq_n_f32(0.806889f));

    float32x4_t const post_r = vdivq_f32(num_r, den_r);
    float32x4_t const post_g = vdivq_f32(num_g, den_g);
    float32x4_t const post_b = vdivq_f32(num_b, den_b);

    // AP1_to_XYZ
    float32x4_t xyz_x = vaddq_f32(vaddq_f32(vmulq_n_f32(post_r, AP1_to_XYZ[0][0]), vmulq_n_f32(post_g, AP1_to_XYZ[1][0])), vmulq_n_f32(post_b, AP1_to_XYZ[2][0]));
    float32x4_t xyz_y = vaddq_f32(vaddq_f32(vmulq_n_f32(post_r, AP1_to_XYZ[0][1]), vmulq_n_f32(post_g, AP1_to_XYZ[1][1])), vmulq_n_f32(post_b, AP1_to_XYZ[2][1]));
    float32x4_t xyz_z = vaddq_f32(vaddq_f32(vmulq_n_f32(post_r, AP1_to_XYZ[0][2]), vmulq_n_f32(post_g, AP1_to_XYZ[1][2])), vmulq_n_f32(post_b, AP1_to_XYZ[2][2]));

    // XYZ_to_xyY
    float32x4_t const inv_sum = vdivq_f32(vdupq_n_f32(1.0f), vmaxq_f32(vaddq_f32(vaddq_f32(xyz_x, xyz_y), xyz_z), vdupq_n_f32(1e-5f)));
    float32x4_t const xyy_x = vmulq_f32(xyz_x, inv_sum);
    float32x4_t const xyy_y = vmulq_f32(xyz_y, inv_sum);
    float32x4_t xyy_z = xyz_y;

    xyy_z = vmaxq_f32(vminq_f32(xyy_z, max_half), vdupq_n_f32(0.0f));
    xyy_z = v_powq_f32(xyy_z, 0.9811f);

    // xyY_to_XYZ
    xyz_y = xyy_z;
    xyz_x = vmulq_f32(vdivq_f32(xyy_z, vmaxq_f32(xyy_y, vdupq_n_f32(1e-5f))), xyy_x);
    xyz_z = vmulq_f32(vdivq_f32(xyy_z, vmaxq_f32(xyy_y, vdupq_n_f32(1e-5f))), vsubq_f32(vsubq_f32(vdupq_n_f32(1.0f), xyy_x), xyy_y));

    // XYZ_to_AP1
    float32x4_t lin_r = vaddq_f32(vaddq_f32(vmulq_n_f32(xyz_x, XYZ_to_AP1[0][0]), vmulq_n_f32(xyz_y, XYZ_to_AP1[1][0])), vmulq_n_f32(xyz_z, XYZ_to_AP1[2][0]));
    float32x4_t lin_g = vaddq_f32(vaddq_f32(vmulq_n_f32(xyz_x, XYZ_to_AP1[0][1]), vmulq_n_f32(xyz_y, XYZ_to_AP1[1][1])), vmulq_n_f32(xyz_z, XYZ_to_AP1[2][1]));
    float32x4_t lin_b = vaddq_f32(vaddq_f32(vmulq_n_f32(xyz_x, XYZ_to_AP1[0][2]), vmulq_n_f32(xyz_y, XYZ_to_AP1[1][2])), vmulq_n_f32(xyz_z, XYZ_to_AP1[2][2]));

    luma = vaddq_f32(vaddq_f32(vmulq_n_f32(lin_r, LUMINANCE_AP1.r), vmulq_n_f32(lin_g, LUMINANCE_AP1.g)), vmulq_n_f32(lin_b, LUMINANCE_AP1.b));
    lin_r = vaddq_f32(luma, vmulq_n_f32(vsubq_f32(lin_r, luma), 0.93f));
    lin_g = vaddq_f32(luma, vmulq_n_f32(vsubq_f32(lin_g, luma), 0.93f));
    lin_b = vaddq_f32(luma, vmulq_n_f32(vsubq_f32(lin_b, luma), 0.93f));

    // AP1_to_Rec2020
    vr = vaddq_f32(vaddq_f32(vmulq_n_f32(lin_r, AP1_to_Rec2020[0][0]), vmulq_n_f32(lin_g, AP1_to_Rec2020[1][0])), vmulq_n_f32(lin_b, AP1_to_Rec2020[2][0]));
    vg = vaddq_f32(vaddq_f32(vmulq_n_f32(lin_r, AP1_to_Rec2020[0][1]), vmulq_n_f32(lin_g, AP1_to_Rec2020[1][1])), vmulq_n_f32(lin_b, AP1_to_Rec2020[2][1]));
    vb = vaddq_f32(vaddq_f32(vmulq_n_f32(lin_r, AP1_to_Rec2020[0][2]), vmulq_n_f32(lin_g, AP1_to_Rec2020[1][2])), vmulq_n_f32(lin_b, AP1_to_Rec2020[2][2]));
}
#endif

DEFAULT_CONSTRUCTORS(ACESToneMapper)

float3 ACESToneMapper::operator()(float3 const c) const noexcept {
    return aces::ACES(c, 1.0f);
}

#if defined(__ARM_NEON)
void ACESToneMapper::operator()(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb) const noexcept {
    v_toneMappingACES(vr, vg, vb, 1.0f);
}
#endif

DEFAULT_CONSTRUCTORS(ACESLegacyToneMapper)

float3 ACESLegacyToneMapper::operator()(float3 const c) const noexcept {
    return aces::ACES(c, 1.0f / 0.6f);
}

#if defined(__ARM_NEON)
void ACESLegacyToneMapper::operator()(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb) const noexcept {
    v_toneMappingACES(vr, vg, vb, 1.66666667f);
}
#endif

DEFAULT_CONSTRUCTORS(FilmicToneMapper)

float3 FilmicToneMapper::operator()(float3 const x) const noexcept {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    constexpr float a = 2.51f;
    constexpr float b = 0.03f;
    constexpr float c = 2.43f;
    constexpr float d = 0.59f;
    constexpr float e = 0.14f;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

#if defined(__ARM_NEON)
void FilmicToneMapper::operator()(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb) const noexcept {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    constexpr float a = 2.51f;
    constexpr float b = 0.03f;
    constexpr float c = 2.43f;
    constexpr float d = 0.59f;
    constexpr float e = 0.14f;

    float32x4_t const b_vec = vdupq_n_f32(b);
    float32x4_t const d_vec = vdupq_n_f32(d);
    float32x4_t const e_vec = vdupq_n_f32(e);

    float32x4_t const num_r = vmulq_f32(vr, vmlaq_n_f32(b_vec, vr, a));
    float32x4_t const num_g = vmulq_f32(vg, vmlaq_n_f32(b_vec, vg, a));
    float32x4_t const num_b = vmulq_f32(vb, vmlaq_n_f32(b_vec, vb, a));

    float32x4_t const den_r = vmlaq_f32(e_vec, vr, vmlaq_n_f32(d_vec, vr, c));
    float32x4_t const den_g = vmlaq_f32(e_vec, vg, vmlaq_n_f32(d_vec, vg, c));
    float32x4_t const den_b = vmlaq_f32(e_vec, vb, vmlaq_n_f32(d_vec, vb, c));

    vr = vdivq_f32(num_r, den_r);
    vg = vdivq_f32(num_g, den_g);
    vb = vdivq_f32(num_b, den_b);
}
#endif

//------------------------------------------------------------------------------
// PBR Neutral tone mapper
//------------------------------------------------------------------------------

DEFAULT_CONSTRUCTORS(PBRNeutralToneMapper)

float3 PBRNeutralToneMapper::operator()(float3 color) const noexcept {
    // PBR Tone Mapping, https://modelviewer.dev/examples/tone-mapping.html
    constexpr float startCompression = 0.8f - 0.04f;
    constexpr float desaturation = 0.15f;

    float const x = min(color.r, min(color.g, color.b));
    float const offset = x < 0.08f ? x - 6.25f * x * x : 0.04f;
    color -= offset;

    float const peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression) return color;

    constexpr float d = 1.0f - startCompression;
    float const newPeak = 1.0f - d * d / (peak + d - startCompression);
    color *= newPeak / peak;

    float const g = 1.0f - 1.0f / (desaturation * (peak - newPeak) + 1.0f);
    return mix(color, float3(newPeak), g);
}

#if defined(__ARM_NEON)
void PBRNeutralToneMapper::operator()(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb) const noexcept {
    constexpr float startCompression = 0.8f - 0.04f;
    constexpr float desaturation = 0.15f;

    float32x4_t const x = vminq_f32(vr, vminq_f32(vg, vb));
    float32x4_t const quad = vsubq_f32(x, vmulq_n_f32(vmulq_f32(x, x), 6.25f));
    uint32x4_t const mask_offset = vcltq_f32(x, vdupq_n_f32(0.08f));
    float32x4_t const offset = vbslq_f32(mask_offset, quad, vdupq_n_f32(0.04f));

    float32x4_t const c_r = vsubq_f32(vr, offset);
    float32x4_t const c_g = vsubq_f32(vg, offset);
    float32x4_t const c_b = vsubq_f32(vb, offset);

    float32x4_t const peak = vmaxq_f32(c_r, vmaxq_f32(c_g, c_b));
    uint32x4_t const cmp_peak = vcgeq_f32(peak, vdupq_n_f32(startCompression));

    constexpr float d = 1.0f - startCompression;
    constexpr float d_sq = d * d;
    float32x4_t const newPeak = vsubq_f32(vdupq_n_f32(1.0f), vdivq_f32(vdupq_n_f32(d_sq), vaddq_f32(peak, vdupq_n_f32(d - startCompression))));
    float32x4_t const ratio = vdivq_f32(newPeak, peak);
    float32x4_t const r_r = vmulq_f32(c_r, ratio);
    float32x4_t const r_g = vmulq_f32(c_g, ratio);
    float32x4_t const r_b = vmulq_f32(c_b, ratio);

    float32x4_t const g = vsubq_f32(vdupq_n_f32(1.0f), vdivq_f32(vdupq_n_f32(1.0f), vmlaq_n_f32(vdupq_n_f32(1.0f), vsubq_f32(peak, newPeak), desaturation)));
    float32x4_t const res_r = vaddq_f32(r_r, vmulq_f32(g, vsubq_f32(newPeak, r_r)));
    float32x4_t const res_g = vaddq_f32(r_g, vmulq_f32(g, vsubq_f32(newPeak, r_g)));
    float32x4_t const res_b = vaddq_f32(r_b, vmulq_f32(g, vsubq_f32(newPeak, r_b)));

    vr = vbslq_f32(cmp_peak, res_r, c_r);
    vg = vbslq_f32(cmp_peak, res_g, c_g);
    vb = vbslq_f32(cmp_peak, res_b, c_b);
}
#endif

//------------------------------------------------------------------------------
// GT7 tone mapper
//------------------------------------------------------------------------------

// The following implementation is based on code provided by Polyphony Digital,
// under the following license:
//
// MIT License
//
// Copyright (c) 2025 Polyphony Digital Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// See "Driving Toward Reality: Physically Based Tone Mapping and Perceptual
// Fidelity in Gran Turismo 7", SIGGRAPH 2025, by Yasutomi, Suzuki, and Uchimura.

constexpr float ReferenceLuminance = 100.0f; // 100 cd/m^2 equals a value of 1 in the framebuffer
constexpr float SdrPaperWhite = 250.0f; // Paper white target of 250 nits

UTILS_UNUSED
static float frameBufferValueToPhysicalValue(float const v) noexcept {
    // Converts a linear framebuffer value to physical luminance (cd/m^2)
    // where 1.0 corresponds to the reference luminance.
    return v * ReferenceLuminance;
}

static float3 frameBufferValueToPhysicalValue(float3 const v) noexcept {
    return v * ReferenceLuminance;
}

static float physicalValueToFrameBufferValue(float const v) noexcept {
    // Converts a physical luminance (cd/m^2) to a linear framebuffer value,
    // where 1.0 corresponds to the reference luminance.
    return v / ReferenceLuminance;
}

static float3 physicalValueToFrameBufferValue(float3 const v) noexcept {
    return v / ReferenceLuminance;
}

namespace {

class GT7Curve {
public:
    float evaluate(const float x) const {
        if (x < 0.0f) return 0.0f;

        const float weightLinear = smoothstep(0.0f, mMidPoint, x);
        const float weightToe = 1.0f - weightLinear;

        // Shoulder mapping for highlights.
        const float shoulder = mKa + mKb * expf(x * mKc);

        if (x < mLinearSection * mPeakIntensity) {
            const float toeMapped = mMidPoint * powf(x / mMidPoint, mToeStrength);
            return weightToe * toeMapped + weightLinear * x;
        }
        return shoulder;
    }

    void initialize(
        const float displayIntensity,
        const float alpha,
        const float grayPoint,
        const float linearSection,
        const float toeStrength
    ) {
        mPeakIntensity = displayIntensity;
        mAlpha = alpha;
        mMidPoint = grayPoint;
        mLinearSection = linearSection;
        mToeStrength = toeStrength;

        // Pre-compute constants for the shoulder region.
        const float k = (mLinearSection - 1.0f) / (mAlpha - 1.0f);
        mKa = mPeakIntensity * mLinearSection + mPeakIntensity * k;
        mKb = -mPeakIntensity * k * expf(mLinearSection / k);
        mKc = -1.0f / (k * mPeakIntensity);
    }

private:
    float mPeakIntensity{};
    float mAlpha{};
    float mMidPoint{};
    float mLinearSection{};
    float mToeStrength{};
    float mKa{};
    float mKb{};
    float mKc{};
};

} // anonymous namespace

struct GT7ToneMapper::State {
    float sdrCorrectionFactor;
    float framebufferLuminanceTarget;
    float framebufferLuminanceTargetUcs;

    float blendRatio;
    float fadeStart;
    float fadeEnd;

    GT7Curve curve;

    // The display target luminance should be ~250 nits for SDR displays,
    // and whatever peak luminance an HDR display supports (700, 1000, etc.).
    void initializeParameters(float const sdrCorrection, float const displayTargetLuminance) {
        sdrCorrectionFactor = sdrCorrection;
        framebufferLuminanceTarget = physicalValueToFrameBufferValue(displayTargetLuminance);

        // TODO: We could expose the curve parameters to users
        curve.initialize(framebufferLuminanceTarget, 0.25f, 0.538f, 0.444f, 1.280f);

        // TODO: Expose these controls to the user
        blendRatio = 0.6f;
        fadeStart = 0.98f;
        fadeEnd = 1.16f;

        const float3 rgb{framebufferLuminanceTarget};
        framebufferLuminanceTargetUcs = Rec2020_to_ICtCp(frameBufferValueToPhysicalValue(rgb)).x;
    }
};

GT7ToneMapper::GT7ToneMapper() noexcept {
    mState = new(std::nothrow) State();

    // Initialize for an SDR target
    mState->initializeParameters(
        1.0f / physicalValueToFrameBufferValue(SdrPaperWhite),
        SdrPaperWhite
    );

    // TODO: To initialize for HDR output, pass 1.0 as the SDR correction factor,
    //       and the desired peak display luminance as the second parameter
}

GT7ToneMapper::~GT7ToneMapper() noexcept {
    delete mState;
}


static float chromaCurve(float const a, float const b, float const x) noexcept {
    return 1.0f - smoothstep(a, b, x);
}

float3 GT7ToneMapper::operator()(float3 color) const noexcept {
    const State& state = *mState;
    const GT7Curve& curve = state.curve;

    float3 const ucs = Rec2020_to_ICtCp(frameBufferValueToPhysicalValue(color));
    float3 const skewedRgb{
        curve.evaluate(color.r),
        curve.evaluate(color.g),
        curve.evaluate(color.b)
    };
    float3 const skewedUcs = Rec2020_to_ICtCp(frameBufferValueToPhysicalValue(skewedRgb));

    float const chromaScale = chromaCurve(
        state.fadeStart, state.fadeEnd, ucs.x / state.framebufferLuminanceTargetUcs);
    float3 const scaledRgb = physicalValueToFrameBufferValue(ICtCp_to_Rec2020(float3{
        skewedUcs.x,
        ucs.y * chromaScale,
        ucs.z * chromaScale
    }));

    const float blendRatio = state.blendRatio;
    const float sdrFactor = state.sdrCorrectionFactor;

    float3 const luminanceTarget{state.framebufferLuminanceTarget};
    return sdrFactor * min(mix(skewedRgb, scaledRgb, blendRatio), luminanceTarget);
}

//------------------------------------------------------------------------------
// AgX tone mapper
//------------------------------------------------------------------------------

AgxToneMapper::AgxToneMapper(AgxLook const look) noexcept : look(look) {}
AgxToneMapper::~AgxToneMapper() noexcept = default;

// These matrices taken from Blender's implementation of AgX, which works with Rec.2020 primaries.
// https://github.com/EaryChow/AgX_LUT_Gen/blob/main/AgXBaseRec2020.py
constexpr mat3f AgXInsetMatrix {
    0.856627153315983, 0.137318972929847, 0.11189821299995,
    0.0951212405381588, 0.761241990602591, 0.0767994186031903,
    0.0482516061458583, 0.101439036467562, 0.811302368396859
};
constexpr mat3f AgXOutsetMatrixInv {
    0.899796955911611, 0.11142098895748, 0.11142098895748,
    0.0871996192028351, 0.875575586156966, 0.0871996192028349,
    0.013003424885555, 0.0130034248855548, 0.801379391839686
};
constexpr mat3f AgXOutsetMatrix { inverse(AgXOutsetMatrixInv) };

// LOG2_MIN      = -10.0
// LOG2_MAX      =  +6.5
// MIDDLE_GRAY   =  0.18
constexpr float AgxMinEv = -12.47393f;      // log2(pow(2, LOG2_MIN) * MIDDLE_GRAY)
constexpr float AgxMaxEv = 4.026069f;       // log2(pow(2, LOG2_MAX) * MIDDLE_GRAY)

// Adapted from https://iolite-engine.com/blog_posts/minimal_agx_implementation
static float3 agxDefaultContrastApprox(float3 x) noexcept {
    float3 const x2 = x * x;
    float3 const x4 = x2 * x2;
    float3 const x6 = x4 * x2;
    return  - 17.86f    * x6 * x
            + 78.01f    * x6
            - 126.7f    * x4 * x
            + 92.06f    * x4
            - 28.72f    * x2 * x
            + 4.361f    * x2
            - 0.1718f   * x
            + 0.002857f;
}

// Adapted from https://iolite-engine.com/blog_posts/minimal_agx_implementation
static float3 agxLook(float3 val, AgxToneMapper::AgxLook look) noexcept {
    if (look == AgxToneMapper::AgxLook::NONE) {
        return val;
    }

    constexpr float3 lw = float3(0.2126f, 0.7152f, 0.0722f);
    float const luma = dot(val, lw);

    // Default
    constexpr float3 offset = float3(0.0f);
    float3 slope = float3(1.0f);
    float3 power = float3(1.0f);
    float sat = 1.0f;

    if (look == AgxToneMapper::AgxLook::GOLDEN) {
        slope = float3(1.0f, 0.9f, 0.5f);
        power = float3(0.8f);
        sat = 1.3;
    }
    if (look == AgxToneMapper::AgxLook::PUNCHY) {
        slope = float3(1.0f);
        power = float3(1.35f, 1.35f, 1.35f);
        sat = 1.4;
    }

    // ASC CDL
    val = pow(val * slope + offset, power);
    return luma + sat * (val - luma);
}

float3 AgxToneMapper::operator()(float3 v) const noexcept {
    // Ensure no negative values
    v = max(float3(0.0f), v);

    v = AgXInsetMatrix * v;

    // Log2 encoding
    v = max(v, 1E-10f); // avoid 0 or negative numbers for log2
    v = log2(v);
    v = (v - AgxMinEv) / (AgxMaxEv - AgxMinEv);

    v = clamp(v, 0.0f, 1.0f);

    // Apply sigmoid
    v = agxDefaultContrastApprox(v);

    // Apply AgX look
    v = agxLook(v, look);

    v = AgXOutsetMatrix * v;

    // Linearize
    v = pow(max(float3(0.0f), v), 2.2f);

    return v;
}

#if defined(__ARM_NEON)
static float32x4_t v_agxDefaultContrastApprox(float32x4_t const x) noexcept {
    float32x4_t const x2 = vmulq_f32(x, x);
    float32x4_t const x4 = vmulq_f32(x2, x2);
    float32x4_t const x6 = vmulq_f32(x4, x2);

    float32x4_t res = vdupq_n_f32(0.002857f);
    res = vmlaq_n_f32(res, x, -0.1718f);
    res = vmlaq_n_f32(res, x2, 4.361f);
    res = vmlaq_f32(res, vmulq_f32(x2, x), vdupq_n_f32(-28.72f));
    res = vmlaq_n_f32(res, x4, 92.06f);
    res = vmlaq_f32(res, vmulq_f32(x4, x), vdupq_n_f32(-126.7f));
    res = vmlaq_n_f32(res, x6, 78.01f);
    res = vmlaq_f32(res, vmulq_f32(x6, x), vdupq_n_f32(-17.86f));
    return res;
}

static void v_agxLook(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb, AgxToneMapper::AgxLook look) noexcept {
    if (look == AgxToneMapper::AgxLook::NONE) {
        return;
    }

    float32x4_t const luma = vaddq_f32(vaddq_f32(vmulq_n_f32(vr, 0.2126f), vmulq_n_f32(vg, 0.7152f)), vmulq_n_f32(vb, 0.0722f));

    float slope_r = 1.0f, slope_g = 1.0f, slope_b = 1.0f;
    float power_r = 1.0f, power_g = 1.0f, power_b = 1.0f;
    float sat = 1.0f;

    if (look == AgxToneMapper::AgxLook::GOLDEN) {
        slope_r = 1.0f; slope_g = 0.9f; slope_b = 0.5f;
        power_r = 0.8f; power_g = 0.8f; power_b = 0.8f;
        sat = 1.3f;
    } else if (look == AgxToneMapper::AgxLook::PUNCHY) {
        slope_r = 1.0f; slope_g = 1.0f; slope_b = 1.0f;
        power_r = 1.35f; power_g = 1.35f; power_b = 1.35f;
        sat = 1.4f;
    }

    vr = v_powq_f32(vmulq_n_f32(vr, slope_r), power_r);
    vg = v_powq_f32(vmulq_n_f32(vg, slope_g), power_g);
    vb = v_powq_f32(vmulq_n_f32(vb, slope_b), power_b);

    vr = vaddq_f32(luma, vmulq_n_f32(vsubq_f32(vr, luma), sat));
    vg = vaddq_f32(luma, vmulq_n_f32(vsubq_f32(vg, luma), sat));
    vb = vaddq_f32(luma, vmulq_n_f32(vsubq_f32(vb, luma), sat));
}

void AgxToneMapper::operator()(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb) const noexcept {
    float32x4_t const zero = vdupq_n_f32(0.0f);
    vr = vmaxq_f32(zero, vr);
    vg = vmaxq_f32(zero, vg);
    vb = vmaxq_f32(zero, vb);

    // AgXInsetMatrix
    float32x4_t in_r = vaddq_f32(vaddq_f32(vmulq_n_f32(vr, AgXInsetMatrix[0][0]), vmulq_n_f32(vg, AgXInsetMatrix[1][0])), vmulq_n_f32(vb, AgXInsetMatrix[2][0]));
    float32x4_t in_g = vaddq_f32(vaddq_f32(vmulq_n_f32(vr, AgXInsetMatrix[0][1]), vmulq_n_f32(vg, AgXInsetMatrix[1][1])), vmulq_n_f32(vb, AgXInsetMatrix[2][1]));
    float32x4_t in_b = vaddq_f32(vaddq_f32(vmulq_n_f32(vr, AgXInsetMatrix[0][2]), vmulq_n_f32(vg, AgXInsetMatrix[1][2])), vmulq_n_f32(vb, AgXInsetMatrix[2][2]));

    float32x4_t const tiny = vdupq_n_f32(1E-10f);
    in_r = v_log2q_f32(vmaxq_f32(in_r, tiny));
    in_g = v_log2q_f32(vmaxq_f32(in_g, tiny));
    in_b = v_log2q_f32(vmaxq_f32(in_b, tiny));

    float const inv_range = 1.0f / (AgxMaxEv - AgxMinEv);
    float32x4_t const min_ev = vdupq_n_f32(AgxMinEv);
    in_r = vmulq_n_f32(vsubq_f32(in_r, min_ev), inv_range);
    in_g = vmulq_n_f32(vsubq_f32(in_g, min_ev), inv_range);
    in_b = vmulq_n_f32(vsubq_f32(in_b, min_ev), inv_range);

    float32x4_t const one = vdupq_n_f32(1.0f);
    in_r = vmaxq_f32(vminq_f32(in_r, one), zero);
    in_g = vmaxq_f32(vminq_f32(in_g, one), zero);
    in_b = vmaxq_f32(vminq_f32(in_b, one), zero);

    in_r = v_agxDefaultContrastApprox(in_r);
    in_g = v_agxDefaultContrastApprox(in_g);
    in_b = v_agxDefaultContrastApprox(in_b);

    v_agxLook(in_r, in_g, in_b, look);

    // AgXOutsetMatrix
    vr = vaddq_f32(vaddq_f32(vmulq_n_f32(in_r, AgXOutsetMatrix[0][0]), vmulq_n_f32(in_g, AgXOutsetMatrix[1][0])), vmulq_n_f32(in_b, AgXOutsetMatrix[2][0]));
    vg = vaddq_f32(vaddq_f32(vmulq_n_f32(in_r, AgXOutsetMatrix[0][1]), vmulq_n_f32(in_g, AgXOutsetMatrix[1][1])), vmulq_n_f32(in_b, AgXOutsetMatrix[2][1]));
    vb = vaddq_f32(vaddq_f32(vmulq_n_f32(in_r, AgXOutsetMatrix[0][2]), vmulq_n_f32(in_g, AgXOutsetMatrix[1][2])), vmulq_n_f32(in_b, AgXOutsetMatrix[2][2]));

    vr = v_powq_f32(vmaxq_f32(zero, vr), 2.2f);
    vg = v_powq_f32(vmaxq_f32(zero, vg), 2.2f);
    vb = v_powq_f32(vmaxq_f32(zero, vb), 2.2f);
}
#endif

//------------------------------------------------------------------------------
// Display range tone mapper
//------------------------------------------------------------------------------

DEFAULT_CONSTRUCTORS(DisplayRangeToneMapper)

float3 DisplayRangeToneMapper::operator()(float3 const c) const noexcept {
    // 16 debug colors + 1 duplicated at the end for easy indexing
    constexpr float3 debugColors[17] = {
            {0.0,     0.0,     0.0},         // black
            {0.0,     0.0,     0.1647},      // darkest blue
            {0.0,     0.0,     0.3647},      // darker blue
            {0.0,     0.0,     0.6647},      // dark blue
            {0.0,     0.0,     0.9647},      // blue
            {0.0,     0.9255,  0.9255},      // cyan
            {0.0,     0.5647,  0.0},         // dark green
            {0.0,     0.7843,  0.0},         // green
            {1.0,     1.0,     0.0},         // yellow
            {0.90588, 0.75294, 0.0},         // yellow-orange
            {1.0,     0.5647,  0.0},         // orange
            {1.0,     0.0,     0.0},         // bright red
            {0.8392,  0.0,     0.0},         // red
            {1.0,     0.0,     1.0},         // magenta
            {0.6,     0.3333,  0.7882},      // purple
            {1.0,     1.0,     1.0},         // white
            {1.0,     1.0,     1.0}          // white
    };

    // The 5th color in the array (cyan) represents middle gray (18%)
    // Every stop above or below middle gray causes a color shift
    // TODO: This should depend on the working color grading color space
    float v = log2(dot(c, LUMINANCE_Rec2020) / 0.18f);
    v = clamp(v + 5.0f, 0.0f, 15.0f);

    size_t const index = size_t(v);
    return mix(debugColors[index], debugColors[index + 1], saturate(v - float(index)));
}

//------------------------------------------------------------------------------
// Generic tone mapper
//------------------------------------------------------------------------------

struct GenericToneMapper::Options {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#endif
    void setParameters(
            float contrast,
            float midGrayIn,
            float midGrayOut,
            float hdrMax
    ) {
        contrast = max(contrast, 1e-5f);
        midGrayIn = clamp(midGrayIn, 1e-5f, 1.0f);
        midGrayOut = clamp(midGrayOut, 1e-5f, 1.0f);
        hdrMax = max(hdrMax, 1.0f);

        this->contrast = contrast;
        this->midGrayIn = midGrayIn;
        this->midGrayOut = midGrayOut;
        this->hdrMax = hdrMax;

        float const a = pow(midGrayIn, contrast);
        float const b = pow(hdrMax, contrast);
        float const c = a - midGrayOut * b;

        inputScale = (a * b * (midGrayOut - 1.0f)) / c;
        outputScale = midGrayOut * (a - b) / c;
    }
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    float contrast;
    float midGrayIn;
    float midGrayOut;
    float hdrMax;

    // TEMP
    float inputScale;
    float outputScale;
};

GenericToneMapper::GenericToneMapper(
        float const contrast,
        float const midGrayIn,
        float const midGrayOut,
        float const hdrMax
) noexcept {
    mOptions = new(std::nothrow) Options();
    mOptions->setParameters(contrast, midGrayIn, midGrayOut, hdrMax);
}

GenericToneMapper::~GenericToneMapper() noexcept {
    delete mOptions;
}

GenericToneMapper::GenericToneMapper(GenericToneMapper&& rhs)  noexcept : mOptions(rhs.mOptions) {
    rhs.mOptions = nullptr;
}

GenericToneMapper& GenericToneMapper::operator=(GenericToneMapper&& rhs) noexcept {
    mOptions = rhs.mOptions;
    rhs.mOptions = nullptr;
    return *this;
}

float3 GenericToneMapper::operator()(float3 x) const noexcept {
    x = pow(x, mOptions->contrast);
    return mOptions->outputScale * x / (x + mOptions->inputScale);
}

#if defined(__ARM_NEON)
void GenericToneMapper::operator()(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb) const noexcept {
    float const contrast = mOptions->contrast;
    float32x4_t const outScale = vdupq_n_f32(mOptions->outputScale);
    float32x4_t const inScale = vdupq_n_f32(mOptions->inputScale);

    vr = v_powq_f32(vr, contrast);
    vg = v_powq_f32(vg, contrast);
    vb = v_powq_f32(vb, contrast);

    vr = vdivq_f32(vmulq_f32(outScale, vr), vaddq_f32(vr, inScale));
    vg = vdivq_f32(vmulq_f32(outScale, vg), vaddq_f32(vg, inScale));
    vb = vdivq_f32(vmulq_f32(outScale, vb), vaddq_f32(vb, inScale));
}
#endif

float GenericToneMapper::getContrast() const noexcept { return  mOptions->contrast; }
float GenericToneMapper::getMidGrayIn() const noexcept { return  mOptions->midGrayIn; }
float GenericToneMapper::getMidGrayOut() const noexcept { return  mOptions->midGrayOut; }
float GenericToneMapper::getHdrMax() const noexcept { return  mOptions->hdrMax; }

void GenericToneMapper::setContrast(float const contrast) noexcept {
    mOptions->setParameters(
            contrast,
            mOptions->midGrayIn,
            mOptions->midGrayOut,
            mOptions->hdrMax
    );
}
void GenericToneMapper::setMidGrayIn(float const midGrayIn) noexcept {
    mOptions->setParameters(
            mOptions->contrast,
            midGrayIn,
            mOptions->midGrayOut,
            mOptions->hdrMax
    );
}

void GenericToneMapper::setMidGrayOut(float const midGrayOut) noexcept {
    mOptions->setParameters(
            mOptions->contrast,
            mOptions->midGrayIn,
            midGrayOut,
            mOptions->hdrMax
    );
}

void GenericToneMapper::setHdrMax(float const hdrMax) noexcept {
    mOptions->setParameters(
            mOptions->contrast,
            mOptions->midGrayIn,
            mOptions->midGrayOut,
            hdrMax
    );
}

#undef DEFAULT_CONSTRUCTORS

} // namespace filament
