/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMAT_MATERIAL_VARIANTS_H
#define TNT_FILAMAT_MATERIAL_VARIANTS_H

#include <private/filament/Variant.h>

#include <vector>

namespace filamat {

struct Variant {
    using Stage = filament::backend::ShaderType;

    Variant(uint8_t v, Stage s) : variant(v), stage(s) {}

    uint8_t variant;
    Stage stage;
};

std::vector<Variant> determineVariants(uint8_t variantFilter, bool isLit,
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

} // namespace filamat

#endif
