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

#include "configuration.h"

#include <filament/Engine.h>

#include <utils/FixedCapacityVector.h>

#include <unordered_map>

namespace samples {

namespace {

using FilterVector = utils::FixedCapacityVector<char const*>;
std::unordered_map<filament::Engine::Backend, FilterVector> gFilters;

// TODO: This is not necessary once we can support these variants in webgpu.
char const* WEBGPU_VARIANT_FILTERS[] = {
    "skinning",
    "stereo",
};

} // namespace anonymous

FilterVector const& getJitMaterialVariantFilter(filament::Engine::Backend backend) {
    if (auto itr = gFilters.find(backend); itr != gFilters.end()) {
        return itr->second;
    }
    auto build = [&](char const** filters, size_t filterCount) -> FilterVector {
        utils::FixedCapacityVector<char const*> filterStr;
        filterStr.clear();
        filterStr.reserve(filterCount);
        for (size_t i = 0; i < filterCount; ++i) {
            filterStr.push_back(filters[i]);
        }
        return filterStr;
    };

    switch (backend) {
        case filament::Engine::Backend::WEBGPU:
            gFilters[backend] = build(WEBGPU_VARIANT_FILTERS,
                    sizeof(WEBGPU_VARIANT_FILTERS) / sizeof(char const*));
            break;
        default:
            gFilters[backend] = build(nullptr, 0);
    }
    return gFilters[backend];
}

} // namespace samples
