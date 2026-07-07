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

#include "DynamicSpecConstKey.h"

#include <utils/Slice.h>

#include <array>
#include <cstdint>

namespace filament {

namespace {

constexpr auto get_keys() noexcept {
    std::array<DynamicSpecConstKey, DYNAMIC_SPEC_CONST_KEY_COUNT> keys;
    for (size_t i = 0; i < DYNAMIC_SPEC_CONST_KEY_COUNT; ++i) {
        keys[i] = DynamicSpecConstKey{uint16_t(i)};
    }
    return keys;
}

static auto const gDynamicSpecConstKeys{ get_keys() };

} // anonymous namespace

utils::Slice<const DynamicSpecConstKey> DynamicSpecConstKey::getAllPossibleKeys() noexcept {
    return { gDynamicSpecConstKeys.data(), gDynamicSpecConstKeys.size() };
}

} // namespace filament
