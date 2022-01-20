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

namespace filament {

Variant Variant::filterUserVariant(
        Variant variant, UserVariantFilterMask filterMask) noexcept {
    // these are easy to filter by just removing the corresponding bit
    if (filterMask & (uint32_t)UserVariantFilterBit::DIRECTIONAL_LIGHTING) {
        variant.key &= ~(filterMask & DIR);
    }
    if (filterMask & (uint32_t)UserVariantFilterBit::DYNAMIC_LIGHTING) {
        variant.key &= ~(filterMask & DYN);
    }
    if (filterMask & (uint32_t)UserVariantFilterBit::SKINNING) {
        variant.key &= ~(filterMask & SKN);
    }
    if (!isValidDepthVariant(variant)) {
        // we can't remove FOG from depth variants, this would, in fact, remove picking
        if (filterMask & (uint32_t)UserVariantFilterBit::FOG) {
            variant.key &= ~(filterMask & FOG);
        }
    }
    if (!isSSRVariant(variant)) {
        // SSR variant needs to be handled separately
        if (filterMask & (uint32_t)UserVariantFilterBit::SHADOW_RECEIVER) {
            variant.key &= ~(filterMask & SRE);
        }
        if (filterMask & (uint32_t)UserVariantFilterBit::VSM) {
            variant.key &= ~(filterMask & VSM);
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

// compile time sanity-check tests

constexpr inline bool reserved_is_not_valid() noexcept {
    for (Variant::type_t i = 0; i < VARIANT_COUNT; i++) {
        const Variant variant(i);
        bool is_valid = Variant::isValid(variant);
        bool is_reserved = Variant::isReserved(variant);
        if (is_valid == is_reserved) {
            return false;
        }
    }
    return true;
}

constexpr inline size_t reserved_variant_count() noexcept {
    size_t count = 0;
    for (Variant::type_t i = 0; i < VARIANT_COUNT; i++) {
        const Variant variant(i);
        if (Variant::isReserved(variant)) {
            count++;
        }
    }
    return count;
}

constexpr inline size_t valid_variant_count() noexcept {
    size_t count = 0;
    for (Variant::type_t i = 0; i < VARIANT_COUNT; i++) {
        const Variant variant(i);
        if (Variant::isValid(variant)) {
            count++;
        }
    }
    return count;
}

constexpr inline size_t vertex_variant_count() noexcept {
    size_t count = 0;
    for (Variant::type_t i = 0; i < VARIANT_COUNT; i++) {
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
    for (Variant::type_t i = 0; i < VARIANT_COUNT; i++) {
        const Variant variant(i);
        if (Variant::isValid(variant)) {
            if (Variant::isFragmentVariant(variant)) {
                count++;
            }
        }
    }
    return count;
}

static_assert(reserved_is_not_valid());
static_assert(reserved_variant_count() == 80);
static_assert(valid_variant_count() == 48);
static_assert(vertex_variant_count() == 16 - (2 + 0) + 4 - 0);        // 18
static_assert(fragment_variant_count() == 33 - (2 + 2 + 8) + 4 - 1);    // 24

} // namespace details

} // namespace filament
