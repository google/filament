/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "MaterialVariants.h"

#include <private/filament/Variant.h>

#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

#include <utils/Slice.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

namespace filamat {

namespace {

using filament::UserVariantFilterBit;
using filament::UserVariantFilterMask;

bool containsVariant(std::vector<Variant> const& variants, filament::Variant const variant,
        filament::backend::ShaderStage const stage) {
    return std::any_of(variants.begin(), variants.end(), [variant, stage](Variant const& item) {
        return item.variant == variant && item.stage == stage;
    });
}

bool containsVariant(utils::Slice<const filament::Variant> const variants,
        filament::Variant const variant) {
    return std::find(variants.begin(), variants.end(), variant) != variants.end();
}

TEST(Variant, PunctualShadowReceiversDoNotAliasSsr) {
    using V = filament::Variant;
    constexpr std::array<V::type_t, 2> SAMPLERS = { 0, V::S2D };
    constexpr std::array<V::type_t, 2> FOG_STATES = { 0, V::FOG };
    constexpr std::array<V::type_t, 2> SKINNING_STATES = { 0, V::SKN };
    constexpr std::array<V::type_t, 2> STEREO_STATES = { 0, V::STE };
    constexpr std::array<V::type_t, 2> DIRECTIONAL_STATES = { 0, V::DIR };
    constexpr UserVariantFilterMask SSR_FILTER = uint32_t(UserVariantFilterBit::SSR);

    for (V::type_t const sampler : SAMPLERS) {
        for (V::type_t const fog : FOG_STATES) {
            for (V::type_t const skinning : SKINNING_STATES) {
                for (V::type_t const stereo : STEREO_STATES) {
                    for (V::type_t const directional : DIRECTIONAL_STATES) {
                        V const requested(V::DYN | V::SRE | sampler | fog | skinning | stereo |
                                directional);
                        V const filtered = V::filterVariant(requested, true);
                        V const expected(requested.key & ~V::DYN);

                        EXPECT_EQ(filtered, expected);
                        EXPECT_TRUE(V::isValidStandardVariant(filtered));
                        EXPECT_TRUE(V::isValidSurfaceVariant(filtered));
                        EXPECT_FALSE(V::isSSRVariant(filtered));
                        EXPECT_FALSE(V::isValidDepthVariant(filtered));
                        EXPECT_TRUE(V::isShadowReceiverVariant(filtered));
                        EXPECT_EQ(V::isShadowSampler2DVariant(filtered), sampler == V::S2D);
                        EXPECT_EQ(V::filterUserVariant(filtered, SSR_FILTER), filtered);
                    }
                }
            }
        }
    }
}

TEST(Variant, SpecialSsrVariantIsDistinctAndFilterable) {
    using V = filament::Variant;
    constexpr UserVariantFilterMask SSR_FILTER = uint32_t(UserVariantFilterBit::SSR);
    constexpr UserVariantFilterMask SHADOW_RECEIVER_FILTER =
            uint32_t(UserVariantFilterBit::SHADOW_RECEIVER);
    constexpr UserVariantFilterMask VSM_FILTER = uint32_t(UserVariantFilterBit::VSM);

    // Keep the SSR sentinel independent of lighting bits that can migrate to spec constants.
    EXPECT_EQ(V::SPECIAL_SSR_VARIANT, V::type_t(V::MNT | V::PCK | V::DEP));
    EXPECT_EQ(V::SPECIAL_SSR_VARIANT & (V::DIR | V::DYN | V::SRE), 0u);

    V const ssr(V::SPECIAL_SSR_VARIANT);
    EXPECT_TRUE(V::isSSRVariant(ssr));
    EXPECT_TRUE(V::isValidSurfaceVariant(ssr));
    EXPECT_TRUE(V::isValid(ssr));
    EXPECT_FALSE(V::isValidStandardVariant(ssr));
    EXPECT_FALSE(V::isValidDepthVariant(ssr));
    EXPECT_FALSE(V::isShadowReceiverVariant(ssr));
    EXPECT_FALSE(V::isShadowSampler2DVariant(ssr));
    EXPECT_FALSE(V::isDepthMomentsVariant(ssr));
    EXPECT_FALSE(V::isPickingVariant(ssr));
    EXPECT_FALSE(V::isFogVariant(ssr));
    EXPECT_EQ(V::filterVariant(ssr, true), ssr);
    EXPECT_EQ(V::filterVariant(ssr, false), ssr);
    EXPECT_EQ(V::filterVariantFragment(ssr), V(V::SPECIAL_SSR_VARIANT));
    EXPECT_EQ(V::filterUserVariant(ssr, SHADOW_RECEIVER_FILTER), ssr);
    EXPECT_EQ(V::filterUserVariant(ssr, VSM_FILTER), ssr);

    size_t ssrVariantCount = 0;
    for (size_t key = 0; key < filament::VARIANT_COUNT; ++key) {
        V const variant(static_cast<V::type_t>(key));
        if (!V::isSSRVariant(variant)) {
            continue;
        }
        ++ssrVariantCount;
        EXPECT_FALSE(V::isValidStandardVariant(variant));
        EXPECT_FALSE(V::isValidDepthVariant(variant));
        EXPECT_TRUE(V::isValidSurfaceVariant(variant));
    }
    EXPECT_EQ(ssrVariantCount, 1u);
}

TEST(Variant, FogFilteringPreservesPickingAndFiltersSurfaceFog) {
    using V = filament::Variant;
    constexpr UserVariantFilterMask FOG_FILTER = uint32_t(UserVariantFilterBit::FOG);

    V const surface(V::SRE | V::FOG);
    EXPECT_EQ(V::filterVariantFog(surface, true), surface);
    EXPECT_EQ(V::filterVariantFog(surface, false), V(V::SRE));
    EXPECT_EQ(V::filterUserVariant(surface, FOG_FILTER), V(V::SRE));

    V const ssr(V::SPECIAL_SSR_VARIANT);
    EXPECT_EQ(V::filterVariantFog(ssr, true), ssr);
    EXPECT_EQ(V::filterVariantFog(ssr, false), ssr);
    EXPECT_EQ(V::filterUserVariant(ssr, FOG_FILTER), ssr);

    V const picking(V::PCK | V::DEP);
    EXPECT_EQ(V::filterVariantFog(picking, true), picking);
    EXPECT_EQ(V::filterVariantFog(picking, false), picking);
    EXPECT_EQ(V::filterUserVariant(picking, FOG_FILTER), picking);
}

TEST(Variant, SurfaceShaderEnumerationKeepsShadowVariantsWhenSsrIsFiltered) {
    using V = filament::Variant;
    constexpr UserVariantFilterMask SSR_FILTER = uint32_t(UserVariantFilterBit::SSR);
    constexpr UserVariantFilterMask VSM_FILTER = uint32_t(UserVariantFilterBit::VSM);
    constexpr UserVariantFilterMask SHADOW_RECEIVER_FILTER =
            uint32_t(UserVariantFilterBit::SHADOW_RECEIVER);

    std::vector<Variant> const all = determineSurfaceVariants(0, true, false);
    EXPECT_TRUE(containsVariant(all, V(V::SRE), filament::backend::ShaderStage::VERTEX));
    EXPECT_TRUE(containsVariant(all, V(V::SRE), filament::backend::ShaderStage::FRAGMENT));
    EXPECT_TRUE(containsVariant(
            all, V(V::SRE | V::FOG), filament::backend::ShaderStage::FRAGMENT));
    EXPECT_TRUE(containsVariant(
            all, V(V::S2D | V::SRE), filament::backend::ShaderStage::FRAGMENT));
    EXPECT_TRUE(containsVariant(
            all, V(V::S2D | V::FOG | V::SRE), filament::backend::ShaderStage::FRAGMENT));
    EXPECT_TRUE(containsVariant(
            all, V(V::SPECIAL_SSR_VARIANT), filament::backend::ShaderStage::FRAGMENT));

    std::vector<Variant> const withoutSsr =
            determineSurfaceVariants(SSR_FILTER, true, false);
    EXPECT_TRUE(containsVariant(
            withoutSsr, V(V::S2D | V::SRE), filament::backend::ShaderStage::FRAGMENT));
    EXPECT_TRUE(containsVariant(
            withoutSsr, V(V::S2D | V::FOG | V::SRE), filament::backend::ShaderStage::FRAGMENT));
    EXPECT_FALSE(containsVariant(
            withoutSsr, V(V::SPECIAL_SSR_VARIANT), filament::backend::ShaderStage::FRAGMENT));

    std::vector<Variant> const withoutVsm =
            determineSurfaceVariants(VSM_FILTER, true, false);
    EXPECT_TRUE(containsVariant(
            withoutVsm, V(V::SRE), filament::backend::ShaderStage::FRAGMENT));
    EXPECT_FALSE(containsVariant(
            withoutVsm, V(V::S2D | V::SRE), filament::backend::ShaderStage::FRAGMENT));
    EXPECT_TRUE(containsVariant(
            withoutVsm, V(V::SPECIAL_SSR_VARIANT), filament::backend::ShaderStage::FRAGMENT));

    std::vector<Variant> const withoutShadowReceiver =
            determineSurfaceVariants(SHADOW_RECEIVER_FILTER, true, false);
    EXPECT_FALSE(containsVariant(
            withoutShadowReceiver, V(V::SRE), filament::backend::ShaderStage::FRAGMENT));
    EXPECT_FALSE(containsVariant(
            withoutShadowReceiver, V(V::S2D | V::SRE), filament::backend::ShaderStage::FRAGMENT));
    EXPECT_TRUE(containsVariant(
            withoutShadowReceiver, V(V::SPECIAL_SSR_VARIANT),
            filament::backend::ShaderStage::FRAGMENT));

    std::vector<Variant> const shadowMultiplier =
            determineSurfaceVariants(SSR_FILTER, false, true);
    EXPECT_TRUE(containsVariant(
            shadowMultiplier, V(V::S2D | V::SRE), filament::backend::ShaderStage::FRAGMENT));
}

TEST(Variant, RuntimeProgramEnumerationContainsShadowAndSsrVariants) {
    using V = filament::Variant;
    utils::Slice<const V> const lit = filament::VariantUtils::getLitVariants();

    EXPECT_TRUE(containsVariant(lit, V(V::SRE)));
    EXPECT_TRUE(containsVariant(lit, V(V::SRE | V::SKN)));
    EXPECT_TRUE(containsVariant(lit, V(V::S2D | V::SRE)));
    EXPECT_TRUE(containsVariant(lit, V(V::S2D | V::FOG | V::SRE)));
    EXPECT_TRUE(containsVariant(lit, V(V::SPECIAL_SSR_VARIANT)));
}

} // anonymous namespace

} // namespace filamat
