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

#include <utils/Slice.h>

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

    static utils::Slice<const DynamicSpecConstKey> getAllPossibleKeys() noexcept;

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
};

} // namespace filament

#endif // TNT_FILAMENT_DYNAMICSPECCONSTKEY_H
