/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <private/filament/EngineEnums.h>

namespace filamat {

std::vector<Variant> determineSurfaceVariants(uint8_t variantFilter, bool isLit,
        bool shadowMultiplier) {
    std::vector<Variant> variants;
    uint8_t variantMask = ~variantFilter;
    for (uint8_t k = 0; k < filament::VARIANT_COUNT; k++) {
        if (filament::Variant::isReserved(k)) {
            continue;
        }

        // Remove variants for unlit materials
        uint8_t v = filament::Variant::filterVariant(
                k & variantMask, isLit || shadowMultiplier);

        if (filament::Variant::filterVariantVertex(v) == k) {
            variants.emplace_back(k, filament::backend::ShaderType::VERTEX);
        }

        if (filament::Variant::filterVariantFragment(v) == k) {
            variants.emplace_back(k, filament::backend::ShaderType::FRAGMENT);
        }
    }
    return variants;
}

std::vector<Variant> determinePostProcessVariants() {
    std::vector<Variant> variants;
    // TODO: add a way to filter out post-process variants (e.g., the transparent variant if only
    // opaque is needed)
    for (size_t k = 0; k < filament::POST_PROCESS_VARIANT_COUNT; k++) {
        variants.emplace_back(k, filament::backend::ShaderType::VERTEX);
        variants.emplace_back(k, filament::backend::ShaderType::FRAGMENT);
    }
    return variants;
}

} // namespace filamat
