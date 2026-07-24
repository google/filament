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

#include <backend/DriverEnums.h>

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

namespace {

using filament::UserVariantFilterBit;
using filament::UserVariantFilterMask;

bool containsVariant(std::vector<filamat::Variant> const& variants,
        filament::Variant const variant, filament::backend::ShaderStage const stage) {
    for (auto const& item : variants) {
        if (item.variant == variant && item.stage == stage) {
            return true;
        }
    }
    return false;
}

} // namespace

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
