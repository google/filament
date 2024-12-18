/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include <private/filament/Variant.h>

#include <filament/MaterialEnums.h>

#include <utils/Slice.h>

#include <array>

#include <stddef.h>
#include <stdint.h>

namespace filament {

Variant Variant::filterUserVariant(
        Variant variant, UserVariantFilterMask filterMask) noexcept {
    // these are easy to filter by just removing the corresponding bit
    if (filterMask & (uint32_t)UserVariantFilterBit::DIRECTIONAL_LIGHTING) {
        variant.key &= ~DIR;
    }
    if (filterMask & (uint32_t)UserVariantFilterBit::DYNAMIC_LIGHTING) {
        variant.key &= ~DYN;
    }
    if (filterMask & (uint32_t)UserVariantFilterBit::SKINNING) {
        variant.key &= ~SKN;
    }
    if (filterMask & (uint32_t)UserVariantFilterBit::STE) {
        variant.key &= ~(filterMask & STE);
    }
    if (!isValidDepthVariant(variant)) {
        // we can't remove FOG from depth variants, this would, in fact, remove picking
        if (filterMask & (uint32_t)UserVariantFilterBit::FOG) {
            variant.key &= ~FOG;
        }
    } else {
        // depth variants can have their VSM bit filtered
        if (filterMask & (uint32_t)UserVariantFilterBit::VSM) {
            variant.key &= ~VSM;
        }
    }
    if (!isSSRVariant(variant)) {
        // SSR variant needs to be handled separately
        if (filterMask & (uint32_t)UserVariantFilterBit::SHADOW_RECEIVER) {
            variant.key &= ~SRE;
        }
        if (filterMask & (uint32_t)UserVariantFilterBit::VSM) {
            variant.key &= ~VSM;
        }
    } else {
        // see if we need to filter out the SSR variants
        if (filterMask & (uint32_t)UserVariantFilterBit::SSR) {
            variant.key &= ~SPECIAL_SSR;
        }
    }
    return variant;
}

namespace details {

namespace {

// Compile-time variant count for lit and unlit
constexpr inline size_t variant_count(bool lit) noexcept {
    size_t count = 0;
    for (size_t i = 0; i < VARIANT_COUNT; i++) {
        Variant variant(i);
        if (!Variant::isValid(variant)) {
            continue;
        }
        variant = Variant::filterVariant(variant, lit);
        if (i == variant.key) {
            count++;
        }
    }
    return count;
}

constexpr inline size_t depth_variant_count() noexcept {
    size_t count = 0;
    for (size_t i = 0; i < VARIANT_COUNT; i++) {
        Variant const variant(i);
        if (Variant::isValidDepthVariant(variant)) {
            count++;
        }
    }
    return count;
}

// Compile-time variant list for lit and unlit
template<bool LIT>
constexpr auto get_variants() noexcept {
    std::array<Variant, variant_count(LIT)> variants;
    size_t count = 0;
    for (size_t i = 0; i < VARIANT_COUNT; i++) {
        Variant variant(i);
        if (Variant::isReserved(variant)) {
            continue;
        }
        variant = Variant::filterVariant(variant, LIT);
        if (i == variant.key) {
            variants[count++] = variant;
        }
    }
    return variants;
}

constexpr auto get_depth_variants() noexcept {
    std::array<Variant, depth_variant_count()> variants;
    size_t count = 0;
    for (size_t i = 0; i < VARIANT_COUNT; i++) {
        Variant const variant(i);
        if (Variant::isValidDepthVariant(variant)) {
            variants[count++] = variant;
        }
    }
    return variants;
}

// Below are compile time sanity-check tests
constexpr inline bool reserved_is_not_valid() noexcept {
    for (size_t i = 0; i < VARIANT_COUNT; i++) {
        const Variant variant(i);
        bool const is_valid = Variant::isValid(variant);
        bool const is_reserved = Variant::isReserved(variant);
        if (is_valid == is_reserved) {
            return false;
        }
    }
    return true;
}

constexpr inline size_t reserved_variant_count() noexcept {
    size_t count = 0;
    for (size_t i = 0; i < VARIANT_COUNT; i++) {
        const Variant variant(i);
        if (Variant::isReserved(variant)) {
            count++;
        }
    }
    return count;
}

constexpr inline size_t valid_variant_count() noexcept {
    size_t count = 0;
    for (size_t i = 0; i < VARIANT_COUNT; i++) {
        const Variant variant(i);
        if (Variant::isValid(variant)) {
            count++;
        }
    }
    return count;
}

constexpr inline size_t vertex_variant_count() noexcept {
    size_t count = 0;
    for (size_t i = 0; i < VARIANT_COUNT; i++) {
        const Variant variant(i);
        if (Variant::isValid(variant)) {
            if (Variant::isVertexVariant(variant)) {
                count++;
            }
        }
    }
    return count;
}

constexpr inline size_t fragment_variant_count() noexcept {
    size_t count = 0;
    for (size_t i = 0; i < VARIANT_COUNT; i++) {
        const Variant variant(i);
        if (Variant::isValid(variant)) {
            if (Variant::isFragmentVariant(variant)) {
                count++;
            }
        }
    }
    return count;
}

} // anonymous namespace


static auto const gLitVariants{ details::get_variants<true>() };
static auto const gUnlitVariants{ details::get_variants<false>() };
static auto const gDepthVariants{ details::get_depth_variants() };

static_assert(reserved_is_not_valid());
static_assert(reserved_variant_count() == 160);
static_assert(valid_variant_count() == 96);
static_assert(vertex_variant_count() == 32 - (4 + 0) + 8 - 0);        // 36
static_assert(fragment_variant_count() == 33 - (2 + 2 + 8) + 4 - 1);    // 24

} // namespace details


namespace VariantUtils {

utils::Slice<Variant> getLitVariants() noexcept {
    return { details::gLitVariants.data(), details::gLitVariants.size() };
}

utils::Slice<Variant> getUnlitVariants() noexcept {
    return { details::gUnlitVariants.data(), details::gUnlitVariants.size() };
}

utils::Slice<Variant> getDepthVariants() noexcept {
    return { details::gDepthVariants.data(), details::gDepthVariants.size() };
}

}; // VariantUtils

} // namespace filament
