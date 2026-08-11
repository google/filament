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
#ifndef TNT_FILAMENT_DYNAMICSPECCONSTKEY_H
#define TNT_FILAMENT_DYNAMICSPECCONSTKEY_H

#include <private/filament/EngineEnums.h>
#include <private/filament/Variant.h>

#include <filament/MaterialEnums.h>

#include <array>

namespace filament {
static constexpr size_t DYNAMIC_SPEC_CONST_KEY_BITS =
        CONFIG_NEXT_DYNAMIC_SPEC_CONSTANT - CONFIG_MAX_RESERVED_SPEC_CONSTANTS;
static constexpr size_t DYNAMIC_SPEC_CONST_KEY_COUNT = 1 << DYNAMIC_SPEC_CONST_KEY_BITS;

struct DynamicSpecConstKey {
    using type_t = uint16_t;

    DynamicSpecConstKey() noexcept = default;
    DynamicSpecConstKey(DynamicSpecConstKey const& rhs) noexcept = default;
    DynamicSpecConstKey& operator=(DynamicSpecConstKey const& rhs) noexcept = default;
    constexpr explicit DynamicSpecConstKey(type_t key) noexcept : key(key) { }

    type_t key = 0u;

    static constexpr type_t DYNAMIC_LIGHTING = 0x1;
    static constexpr type_t EXTRA_DIRECTIONAL_LIGHTS = 0x2;


    constexpr bool operator==(DynamicSpecConstKey rhs) const noexcept {
        return key == rhs.key;
    }

    constexpr bool operator!=(DynamicSpecConstKey rhs) const noexcept {
        return key != rhs.key;
    }

    constexpr DynamicSpecConstKey operator & (type_t rhs) const noexcept {
        return DynamicSpecConstKey(key & rhs);
    }

    constexpr DynamicSpecConstKey operator | (type_t rhs) const noexcept {
        return DynamicSpecConstKey(key | rhs);
    }

    constexpr DynamicSpecConstKey operator ~ () const noexcept {
        return DynamicSpecConstKey(~key);
    }

    constexpr bool hasDynamicLighting() const noexcept {
        return key & DYNAMIC_LIGHTING;
    }

    constexpr void setDynamicLighting(bool v) noexcept {
        key = (key & ~DYNAMIC_LIGHTING) | (v ? DYNAMIC_LIGHTING : type_t(0));
    }

    constexpr bool hasExtraDirectionalLights() const noexcept {
        return key & EXTRA_DIRECTIONAL_LIGHTS;
    }

    constexpr void setExtraDirectionalLights(bool v) noexcept {
        key = (key & ~EXTRA_DIRECTIONAL_LIGHTS) | (v ? EXTRA_DIRECTIONAL_LIGHTS : type_t(0));
    }

    static constexpr DynamicSpecConstKey filterUserVariant(
            DynamicSpecConstKey key, UserVariantFilterMask filterMask) noexcept {
        if (filterMask & uint32_t(UserVariantFilterBit::DYNAMIC_LIGHTING)) {
            key.setDynamicLighting(false);
        }
        if (filterMask & uint32_t(UserVariantFilterBit::DIRECTIONAL_LIGHTING)) {
            key.setExtraDirectionalLights(false);
        }
        return key;
    }

    static constexpr bool canSupportDynamicLighting(Variant const variant,
        MaterialDomain const materialDomain, bool const isLit) noexcept {
        return materialDomain == MaterialDomain::SURFACE && isLit &&
               !Variant::isValidDepthVariant(variant) && !Variant::isSSRVariant(variant);
    }

    static constexpr bool canSupportExtraDirectionalLights(Variant const variant,
        MaterialDomain const materialDomain, bool const isLit) noexcept {
        // the directional-lighting bit is only meaningful on standard variants
        return materialDomain == MaterialDomain::SURFACE && isLit &&
               !Variant::isValidDepthVariant(variant) && !Variant::isSSRVariant(variant) &&
               variant.hasDirectionalLighting();
    }

    static constexpr bool isValidProgramSpecKey(Variant const variant, DynamicSpecConstKey const specKey,
            MaterialDomain const materialDomain, bool const isLit) noexcept {
        return (!specKey.hasDynamicLighting() ||
                       canSupportDynamicLighting(variant, materialDomain, isLit)) &&
               (!specKey.hasExtraDirectionalLights() ||
                       canSupportExtraDirectionalLights(variant, materialDomain, isLit));
    }

    static constexpr DynamicSpecConstKey filterProgramSpecKey(Variant const variant,
            DynamicSpecConstKey specKey, MaterialDomain const materialDomain, bool const isLit) noexcept {
        if (!canSupportDynamicLighting(variant, materialDomain, isLit)) {
            specKey.setDynamicLighting(false);
        }
        if (!canSupportExtraDirectionalLights(variant, materialDomain, isLit)) {
            specKey.setExtraDirectionalLights(false);
        }
        return specKey;
    }

    struct ValidKeys;

    [[nodiscard]] static constexpr ValidKeys getValidKeys(Variant const variant,
            MaterialDomain const materialDomain, bool const isLit) noexcept;
};

struct DynamicSpecConstKey::ValidKeys {
    std::array<DynamicSpecConstKey, DYNAMIC_SPEC_CONST_KEY_COUNT> keys{};
    uint8_t size = 0;

    constexpr DynamicSpecConstKey const* begin() const noexcept { return keys.data(); }
    constexpr DynamicSpecConstKey const* end() const noexcept { return keys.data() + size; }
};

inline constexpr DynamicSpecConstKey::ValidKeys DynamicSpecConstKey::getValidKeys(
        Variant const variant, MaterialDomain const materialDomain, bool const isLit) noexcept {
    ValidKeys result{};
    for (size_t i = 0; i < DYNAMIC_SPEC_CONST_KEY_COUNT; ++i) {
        DynamicSpecConstKey const specKey{ static_cast<type_t>(i) };
        if (isValidProgramSpecKey(variant, specKey, materialDomain, isLit)) {
            result.keys[result.size++] = specKey;
        }
    }
    return result;
}

} // namespace filament

#endif // TNT_FILAMENT_DYNAMICSPECCONSTKEY_H
