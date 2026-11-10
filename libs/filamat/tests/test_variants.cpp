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

TEST(Variant, DepthVariantsAreIndependentOfLighting) {
    using V = filament::Variant;

    for (std::size_t key = 0; key < filament::VARIANT_COUNT; key++) {
        V const variant(static_cast<V::type_t>(key));
        if (V::isValidDepthVariant(variant)) {
            EXPECT_EQ(V::filterVariant(variant, false), variant);
            EXPECT_EQ(V::filterVariant(variant, true), variant);
        }
    }
}

TEST(Variant, UnlitDepthMomentsCanBeExplicitlyFiltered) {
    using V = filament::Variant;
    using filament::backend::ShaderStage;

    V const depthMoments(V::DEP | V::MNT);
    std::vector<filamat::Variant> const unlit =
            filamat::determineSurfaceVariants(0, false, false);

    EXPECT_TRUE(containsVariant(unlit, depthMoments, ShaderStage::VERTEX));
    EXPECT_TRUE(containsVariant(unlit, depthMoments, ShaderStage::FRAGMENT));

    constexpr UserVariantFilterMask VSM_FILTER =
            UserVariantFilterMask(UserVariantFilterBit::VSM);
    std::vector<filamat::Variant> const withoutVsm =
            filamat::determineSurfaceVariants(VSM_FILTER, false, false);

    EXPECT_FALSE(containsVariant(withoutVsm, depthMoments, ShaderStage::VERTEX));
    EXPECT_FALSE(containsVariant(withoutVsm, depthMoments, ShaderStage::FRAGMENT));
}

TEST(Variant, PunctualShadowReceiversDoNotAliasSsr) {
    using V = filament::Variant;
    constexpr std::array<V::type_t, 2> SAMPLERS = { 0, V::S2D };
    constexpr std::array<V::type_t, 2> SKINNING_STATES = { 0, V::SKN };
    constexpr std::array<V::type_t, 2> STEREO_STATES = { 0, V::STE };
    constexpr UserVariantFilterMask SSR_FILTER = uint32_t(UserVariantFilterBit::SSR);

    for (V::type_t const sampler : SAMPLERS) {
        for (V::type_t const skinning : SKINNING_STATES) {
            for (V::type_t const stereo : STEREO_STATES) {
                V const requested(V::SRE | sampler | skinning | stereo);
                V const filtered = V::filterVariant(requested, true);

                EXPECT_EQ(filtered, requested)
                        << "filterVariant must not alter lit shadow-receiver variants";
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

TEST(Variant, SpecialSsrVariantIsDistinctAndFilterable) {
    using V = filament::Variant;
    constexpr UserVariantFilterMask SSR_FILTER = uint32_t(UserVariantFilterBit::SSR);
    constexpr UserVariantFilterMask SHADOW_RECEIVER_FILTER =
            uint32_t(UserVariantFilterBit::SHADOW_RECEIVER);
    constexpr UserVariantFilterMask VSM_FILTER = uint32_t(UserVariantFilterBit::VSM);
    constexpr UserVariantFilterMask STE_FILTER = uint32_t(UserVariantFilterBit::STE);
    constexpr UserVariantFilterMask MOTION_VECTORS_FILTER =
            uint32_t(UserVariantFilterBit::MOTION_VECTORS);

    // The SSR sentinel is a reserved depth-variant encoding; it must stay clear of the
    // shadow-receiver bit so it cannot be confused with a real shadow-receiving variant.
    EXPECT_EQ(V::SPECIAL_SSR_VARIANT, V::type_t(V::MNT | V::PCK | V::DEP | V::STE));

    V const ssr(V::SPECIAL_SSR_VARIANT);
    EXPECT_TRUE(V::isSSRVariant(ssr));
    EXPECT_TRUE(V::isValidSurfaceVariant(ssr));
    EXPECT_TRUE(V::isValid(ssr));
    EXPECT_FALSE(V::isValidStandardVariant(ssr));
    EXPECT_FALSE(V::isValidDepthVariant(ssr));
    EXPECT_FALSE(V::isMotionVectorsVariant(ssr));
    EXPECT_FALSE(V::isShadowReceiverVariant(ssr));
    EXPECT_FALSE(V::isShadowSampler2DVariant(ssr));
    EXPECT_FALSE(V::isDepthMomentsVariant(ssr));
    EXPECT_FALSE(V::isPickingVariant(ssr));
    EXPECT_FALSE(V::isStereoVariant(ssr));
    EXPECT_FALSE(ssr.hasStereo());
    EXPECT_EQ(V::filterVariant(ssr, true), ssr);
    EXPECT_EQ(V::filterVariant(ssr, false), ssr);
    EXPECT_EQ(V::filterVariantFragment(ssr), V(V::SPECIAL_SSR_VARIANT));
    EXPECT_EQ(V::filterUserVariant(ssr, SHADOW_RECEIVER_FILTER), ssr);
    EXPECT_EQ(V::filterUserVariant(ssr, VSM_FILTER), ssr);
    EXPECT_EQ(V::filterUserVariant(ssr, STE_FILTER), ssr);
    EXPECT_EQ(V::filterUserVariant(ssr, MOTION_VECTORS_FILTER), ssr);
    EXPECT_EQ(V::filterUserVariant(ssr, SSR_FILTER), V(V::NO_VARIANT));

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
    EXPECT_EQ(ssrVariantCount, 2u);
}

TEST(Variant, SsrPreservesSkinningAndSharesFragmentShader) {
    using V = filament::Variant;
    using filament::backend::ShaderStage;
    V const ssr(V::SPECIAL_SSR_VARIANT);
    V const skinnedSsr(V::SPECIAL_SSR_VARIANT | V::SKN);

    for (size_t key = 0; key < filament::VARIANT_COUNT; ++key) {
        V const variant(static_cast<V::type_t>(key));
        EXPECT_EQ(V::isSSRVariant(variant), variant == ssr || variant == skinnedSsr);
    }
    for (V const variant : { ssr, skinnedSsr }) {
        EXPECT_TRUE(V::isValidSurfaceVariant(variant));
        EXPECT_FALSE(V::isValidDepthVariant(variant));
        EXPECT_FALSE(V::isPickingVariant(variant));
        EXPECT_FALSE(V::isDepthMomentsVariant(variant));
        EXPECT_FALSE(variant.hasStereo());
        EXPECT_FALSE(V::isStereoVariant(variant));
        EXPECT_FALSE(V::isMotionVectorsVariant(variant));
        EXPECT_EQ(V::filterVariantVertex(variant), V(variant.key & V::SKN));
        EXPECT_EQ(V::filterVariantFragment(variant), ssr);
        for (bool const lit : { false, true }) {
            EXPECT_EQ(V::filterVariant(variant, lit), variant);
        }
        for (auto const filter : { UserVariantFilterBit::VSM,
                     UserVariantFilterBit::SHADOW_RECEIVER, UserVariantFilterBit::STE,
                     UserVariantFilterBit::MOTION_VECTORS }) {
            EXPECT_EQ(V::filterUserVariant(variant, uint32_t(filter)), variant);
        }
        EXPECT_EQ(V::filterUserVariant(variant, uint32_t(UserVariantFilterBit::SSR)),
                V(variant.key & V::SKN));
        EXPECT_EQ(V::filterUserVariant(variant, uint32_t(UserVariantFilterBit::SKINNING)), ssr);
        EXPECT_TRUE(containsVariant(filament::VariantUtils::getLitVariants(), variant));
        EXPECT_TRUE(containsVariant(filament::VariantUtils::getUnlitVariants(), variant));
        EXPECT_FALSE(containsVariant(filament::VariantUtils::getDepthVariants(), variant));
    }
    for (bool const lit : { false, true }) {
        auto const shaders = determineSurfaceVariants(0, lit, false);
        EXPECT_TRUE(containsVariant(shaders, V(V::SKN), ShaderStage::VERTEX));
        EXPECT_TRUE(containsVariant(shaders, ssr, ShaderStage::FRAGMENT));
        EXPECT_FALSE(containsVariant(shaders, skinnedSsr, ShaderStage::FRAGMENT));
        auto const withoutSsr = determineSurfaceVariants(
                uint32_t(UserVariantFilterBit::SSR), lit, false);
        EXPECT_FALSE(containsVariant(withoutSsr, ssr, ShaderStage::FRAGMENT));
        auto const withoutSkinning = determineSurfaceVariants(
                uint32_t(UserVariantFilterBit::SKINNING), lit, false);
        EXPECT_FALSE(containsVariant(withoutSkinning, V(V::SKN), ShaderStage::VERTEX));
        EXPECT_TRUE(containsVariant(withoutSkinning, ssr, ShaderStage::FRAGMENT));
    }
}

TEST(Variant, PickingBitIsReservedInStandardVariants) {
    using V = filament::Variant;

    // PCK is only meaningful in depth variants.
    auto const isStandardWithPicking = [](V const variant) {
        return !(variant.key & V::DEP) && (variant.key & V::PCK);
    };
    for (size_t key = 0; key < filament::VARIANT_COUNT; ++key) {
        V const variant(static_cast<V::type_t>(key));
        if (isStandardWithPicking(variant)) {
            EXPECT_FALSE(V::isValidStandardVariant(variant)) << +variant.key;
            EXPECT_FALSE(V::isPickingVariant(variant)) << +variant.key;
            EXPECT_TRUE(V::isReserved(variant)) << +variant.key;
        }
    }

    for (bool const isLit : { false, true }) {
        for (Variant const& item : determineSurfaceVariants(0, isLit, false)) {
            EXPECT_FALSE(isStandardWithPicking(item.variant)) << +item.variant.key;
        }
    }

    V const picking(V::PCK | V::DEP);
    EXPECT_TRUE(V::isPickingVariant(picking));
    EXPECT_TRUE(V::isValidDepthVariant(picking));
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
            all, V(V::S2D | V::SRE), filament::backend::ShaderStage::FRAGMENT));
    EXPECT_TRUE(containsVariant(
            all, V(V::SPECIAL_SSR_VARIANT), filament::backend::ShaderStage::FRAGMENT));

    std::vector<Variant> const withoutSsr =
            determineSurfaceVariants(SSR_FILTER, true, false);
    EXPECT_TRUE(containsVariant(
            withoutSsr, V(V::S2D | V::SRE), filament::backend::ShaderStage::FRAGMENT));
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
    EXPECT_TRUE(containsVariant(lit, V(V::SPECIAL_SSR_VARIANT)));
}

TEST(Variant, MotionVectorsVariantsAreValidDepthVariants) {
    using V = filament::Variant;
    V const mot(V::SPECIAL_MOT_VARIANT);
    V const skinnedMot(V::SPECIAL_MOT_VARIANT | V::SKN);

    // In depth variants, MOT is encoded as MNT | PCK with STE = 0
    EXPECT_EQ(V::MOT, V::type_t(V::MNT | V::PCK));
    EXPECT_EQ(V::SPECIAL_MOT_VARIANT, V::type_t(V::MNT | V::PCK | V::DEP));

    for (V const variant : { mot, skinnedMot }) {
        EXPECT_TRUE(V::isMotionVectorsVariant(variant));
        EXPECT_TRUE(V::isValidDepthVariant(variant));
        EXPECT_TRUE(V::isValid(variant));
        EXPECT_FALSE(V::isReserved(variant));
        EXPECT_FALSE(V::isValidStandardVariant(variant));
        EXPECT_FALSE(V::isValidSurfaceVariant(variant));
        EXPECT_FALSE(V::isSSRVariant(variant));
        EXPECT_FALSE(V::isDepthMomentsVariant(variant));
        EXPECT_FALSE(V::isPickingVariant(variant));
        EXPECT_FALSE(V::isStereoVariant(variant));
        EXPECT_FALSE(V::isShadowReceiverVariant(variant));
        EXPECT_FALSE(V::isShadowSampler2DVariant(variant));
        EXPECT_EQ(V::filterVariant(variant, true), variant);
        EXPECT_EQ(V::filterVariant(variant, false), variant);
        EXPECT_TRUE(V::isVertexVariant(variant));
        EXPECT_EQ(V::filterVariantVertex(variant), variant);
        // all motion vectors variants share the same fragment shader
        EXPECT_EQ(V::filterVariantFragment(variant), mot);
    }
    EXPECT_TRUE(V::isFragmentVariant(mot));
    EXPECT_FALSE(V::isFragmentVariant(skinnedMot));

    // neither the other depth variants nor standard variants are motion vectors variants
    EXPECT_FALSE(V::isMotionVectorsVariant(V(V::DEP)));
    EXPECT_FALSE(V::isMotionVectorsVariant(V(V::DEP | V::SKN)));
    EXPECT_FALSE(V::isMotionVectorsVariant(V(V::DEP | V::MNT)));
    EXPECT_FALSE(V::isMotionVectorsVariant(V(V::DEP | V::PCK)));
    EXPECT_FALSE(V::isMotionVectorsVariant(V(V::MOT)));
    EXPECT_FALSE(V::isMotionVectorsVariant(V(V::MOT | V::SKN)));
    EXPECT_FALSE(V::isMotionVectorsVariant(V(V::MOT | V::SRE)));
}

TEST(Variant, MotionVectorsVariantsAreMonoOnlyAndExclusive) {
    using V = filament::Variant;
    for (V::type_t const skinning : { V::type_t(0), V::SKN }) {
        V const stereo(V::DEP | V::STE | V::MOT | skinning);
        EXPECT_FALSE(V::isMotionVectorsVariant(stereo));
        EXPECT_FALSE(V::isValidDepthVariant(stereo));
        // (MNT | PCK | DEP | STE), with or without SKN, is an SSR variant.
        EXPECT_TRUE(V::isSSRVariant(stereo));
        EXPECT_FALSE(V::isReserved(stereo));

        // MOT can't be combined with SRE either
        V const variant(V::DEP | V::MOT | skinning | V::SRE);
        EXPECT_FALSE(V::isMotionVectorsVariant(variant));
        EXPECT_FALSE(V::isValidDepthVariant(variant));
        EXPECT_TRUE(V::isReserved(variant));
    }
}

TEST(Variant, VariantCountsIncludeMotionVectors) {
    using V = filament::Variant;
    std::size_t validCount = 0;
    std::size_t depthCount = 0;
    std::size_t motionVectorsCount = 0;
    for (std::size_t key = 0; key < filament::VARIANT_COUNT; ++key) {
        V const variant(static_cast<V::type_t>(key));
        if (V::isValid(variant)) {
            ++validCount;
        }
        if (V::isValidDepthVariant(variant)) {
            ++depthCount;
        }
        if (V::isMotionVectorsVariant(variant)) {
            ++motionVectorsCount;
        }
    }
    // 12 standard + 2 SSR + 12 depth + 2 motion vectors
    EXPECT_EQ(validCount, 28u);
    EXPECT_EQ(depthCount, 14u);
    EXPECT_EQ(motionVectorsCount, 2u);

    utils::Slice<const V> const depth = filament::VariantUtils::getDepthVariants();
    EXPECT_EQ(depth.size(), 14u);
    EXPECT_TRUE(containsVariant(depth, V(V::DEP | V::MOT)));
    EXPECT_TRUE(containsVariant(depth, V(V::DEP | V::SKN | V::MOT)));

    // the color pass enumerations, used by Material::compile(), don't contain them
    EXPECT_FALSE(containsVariant(filament::VariantUtils::getLitVariants(), V(V::DEP | V::MOT)));
    EXPECT_FALSE(containsVariant(filament::VariantUtils::getUnlitVariants(), V(V::DEP | V::MOT)));
}

TEST(Variant, MotionVectorsUserVariantFilter) {
    using V = filament::Variant;
    constexpr UserVariantFilterMask MOTION_VECTORS_FILTER =
            uint32_t(UserVariantFilterBit::MOTION_VECTORS);
    constexpr UserVariantFilterMask VSM_FILTER =
            uint32_t(UserVariantFilterBit::VSM);
    constexpr UserVariantFilterMask SKINNING_FILTER = uint32_t(UserVariantFilterBit::SKINNING);

    EXPECT_EQ(uint32_t(UserVariantFilterBit::MOTION_VECTORS), 0x100u);
    EXPECT_EQ(uint32_t(UserVariantFilterBit::ALL), 0x1FFu);

    V const mot(V::DEP | V::MOT);
    V const skinnedMot(V::DEP | V::SKN | V::MOT);

    // the MOTION_VECTORS filter strips MOT from the motion vectors variants only
    EXPECT_EQ(V::filterUserVariant(mot, MOTION_VECTORS_FILTER), V(V::DEP));
    EXPECT_EQ(V::filterUserVariant(skinnedMot, MOTION_VECTORS_FILTER), V(V::DEP | V::SKN));
    EXPECT_EQ(V::filterUserVariant(V(V::DEP | V::MNT), MOTION_VECTORS_FILTER), V(V::DEP | V::MNT));
    EXPECT_EQ(V::filterUserVariant(V(V::DEP | V::PCK), MOTION_VECTORS_FILTER), V(V::DEP | V::PCK));
    EXPECT_EQ(V::filterUserVariant(V(V::S2D | V::SRE), MOTION_VECTORS_FILTER),
            V(V::S2D | V::SRE));

    // the VSM filter leaves the motion vectors variants alone despite sharing MNT
    EXPECT_EQ(V::filterUserVariant(mot, VSM_FILTER), mot);
    EXPECT_EQ(V::filterUserVariant(skinnedMot, VSM_FILTER), skinnedMot);
    EXPECT_EQ(V::filterUserVariant(V(V::DEP | V::MNT), VSM_FILTER), V(V::DEP));
    EXPECT_EQ(V::filterUserVariant(V(V::S2D | V::SRE), VSM_FILTER), V(V::SRE));

    // the SKINNING filter only removes the skinning bit
    EXPECT_EQ(V::filterUserVariant(skinnedMot, SKINNING_FILTER), mot);
    EXPECT_EQ(V::filterUserVariant(mot, SKINNING_FILTER), mot);

    // no other filter changes them
    for (uint32_t bit = 1; bit <= uint32_t(UserVariantFilterBit::ALL); bit <<= 1) {
        if (bit == uint32_t(UserVariantFilterBit::MOTION_VECTORS) ||
                bit == uint32_t(UserVariantFilterBit::SKINNING)) {
            continue;
        }
        EXPECT_EQ(V::filterUserVariant(mot, UserVariantFilterMask(bit)), mot) << "bit " << bit;
        EXPECT_EQ(V::filterUserVariant(skinnedMot, UserVariantFilterMask(bit)), skinnedMot)
                << "bit " << bit;
    }
}

TEST(Variant, SurfaceShaderEnumerationContainsMotionVectorsVariants) {
    using V = filament::Variant;
    using filament::backend::ShaderStage;
    constexpr UserVariantFilterMask MOTION_VECTORS_FILTER =
            uint32_t(UserVariantFilterBit::MOTION_VECTORS);
    constexpr UserVariantFilterMask VSM_FILTER =
            uint32_t(UserVariantFilterBit::VSM);
    constexpr UserVariantFilterMask SKINNING_FILTER = uint32_t(UserVariantFilterBit::SKINNING);

    V const mot(V::DEP | V::MOT);
    V const skinnedMot(V::DEP | V::SKN | V::MOT);
    V const stereoSkinnedMot(V::DEP | V::STE | V::SKN | V::MOT);

    for (bool const isLit : { true, false }) {
        std::vector<Variant> const all = determineSurfaceVariants(0, isLit, false);
        EXPECT_TRUE(containsVariant(all, mot, ShaderStage::VERTEX));
        EXPECT_TRUE(containsVariant(all, mot, ShaderStage::FRAGMENT));
        EXPECT_TRUE(containsVariant(all, skinnedMot, ShaderStage::VERTEX));
        // the skinned variant uses the fragment shader of the non-skinned one
        EXPECT_FALSE(containsVariant(all, skinnedMot, ShaderStage::FRAGMENT));
        EXPECT_FALSE(containsVariant(all, stereoSkinnedMot, ShaderStage::VERTEX));
        EXPECT_FALSE(containsVariant(all, stereoSkinnedMot, ShaderStage::FRAGMENT));

        std::vector<Variant> const withoutVsm =
                determineSurfaceVariants(VSM_FILTER, isLit, false);
        EXPECT_TRUE(containsVariant(withoutVsm, mot, ShaderStage::VERTEX));
        EXPECT_TRUE(containsVariant(withoutVsm, mot, ShaderStage::FRAGMENT));
        EXPECT_TRUE(containsVariant(withoutVsm, skinnedMot, ShaderStage::VERTEX));

        std::vector<Variant> const withoutMotionVectors =
                determineSurfaceVariants(MOTION_VECTORS_FILTER, isLit, false);
        EXPECT_FALSE(containsVariant(withoutMotionVectors, mot, ShaderStage::VERTEX));
        EXPECT_FALSE(containsVariant(withoutMotionVectors, mot, ShaderStage::FRAGMENT));
        EXPECT_FALSE(containsVariant(withoutMotionVectors, skinnedMot, ShaderStage::VERTEX));
        EXPECT_TRUE(containsVariant(withoutMotionVectors, V(V::DEP), ShaderStage::VERTEX));
        EXPECT_TRUE(containsVariant(withoutMotionVectors, V(V::DEP), ShaderStage::FRAGMENT));
        // nothing else is filtered out
        EXPECT_EQ(all.size() - withoutMotionVectors.size(), 3u);

        std::vector<Variant> const withoutSkinning =
                determineSurfaceVariants(SKINNING_FILTER, isLit, false);
        EXPECT_TRUE(containsVariant(withoutSkinning, mot, ShaderStage::VERTEX));
        EXPECT_TRUE(containsVariant(withoutSkinning, mot, ShaderStage::FRAGMENT));
        EXPECT_FALSE(containsVariant(withoutSkinning, skinnedMot, ShaderStage::VERTEX));
    }
}

} // anonymous namespace

} // namespace filamat
