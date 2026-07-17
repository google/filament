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

#include <gtest/gtest.h>

#include <private/filament/Variant.h>

#include <filament/MaterialEnums.h>

using namespace filament;

// The variant a scene lit only by punctual lights requests when the view uses
// sampler2D shadows (VSM/DPCF/PCSS).
static constexpr Variant PUNCTUAL_ONLY_S2D_VARIANT{ Variant::S2D | Variant::SRE };

TEST(VariantTest, SsrKeyLivesInReservedSpace) {
    constexpr Variant ssr{ Variant::SPECIAL_SSR_VARIANT };
    EXPECT_TRUE(Variant::isSSRVariant(ssr));
    EXPECT_TRUE(Variant::isReserved(ssr));
    // reserved depth space: the color pass never sets DEP, the depth pass never sets SRE
    EXPECT_NE(0, Variant::SPECIAL_SSR_VARIANT & Variant::DEP);
    EXPECT_NE(0, Variant::SPECIAL_SSR_VARIANT & Variant::SRE);

    // the SSR fragment shader keeps its key; skinning only affects its vertex shader
    EXPECT_EQ(ssr, Variant::filterVariantFragment(ssr));
    EXPECT_EQ(Variant(Variant::SKN),
            Variant::filterVariantVertex(Variant(Variant::SPECIAL_SSR_VARIANT | Variant::SKN)));
}

TEST(VariantTest, PunctualOnlySoftShadowVariantIsNotTheSsrKey) {
    EXPECT_FALSE(Variant::isSSRVariant(PUNCTUAL_ONLY_S2D_VARIANT));
    EXPECT_TRUE(Variant::isValidStandardVariant(PUNCTUAL_ONLY_S2D_VARIANT));
    EXPECT_EQ(PUNCTUAL_ONLY_S2D_VARIANT,
            Variant::filterVariant(PUNCTUAL_ONLY_S2D_VARIANT, true));
    EXPECT_TRUE(Variant::isFragmentVariant(PUNCTUAL_ONLY_S2D_VARIANT));
}

TEST(VariantTest, UserFiltersKeepSsrAndShadowVariantsApart) {
    constexpr Variant ssr{ Variant::SPECIAL_SSR_VARIANT };

    // only the ssr filter removes the SSR variant
    EXPECT_EQ(Variant(0), Variant::filterUserVariant(ssr,
            UserVariantFilterMask(UserVariantFilterBit::SSR)));
    EXPECT_EQ(ssr, Variant::filterUserVariant(ssr,
            UserVariantFilterMask(UserVariantFilterBit::VSM)));

    // the ssr filter must not touch the punctual-only sampler2D shadow variant
    EXPECT_EQ(PUNCTUAL_ONLY_S2D_VARIANT,
            Variant::filterUserVariant(PUNCTUAL_ONLY_S2D_VARIANT,
                    UserVariantFilterMask(UserVariantFilterBit::SSR)));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
