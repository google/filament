/*
 * Copyright (C) 2025 The Android Open Source Project
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

#pragma once

#include <initializer_list>
#include <string_view>
#include <variant>

#include <stddef.h>
#include <stdint.h>

namespace filament {

// This is intended to be used only to hold the static material data
struct StaticMaterialInfo {
    struct ConstantInfo {
        std::string_view name;
        std::variant<int32_t, float, bool> value;
    };
    std::string_view name;
    uint8_t const* data;
    size_t size;
    // the life-time of objects pointed to by this initializer_list<> is extended to the
    // life-time of the initializer_list
    std::initializer_list<ConstantInfo> constants;
};

#define MATERIAL(p, n) p ## _ ## n ## _DATA, size_t(p ## _ ## n ## _SIZE)

} // namespace filament
