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

#include "shaders/ShaderGenerator.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/Variant.h>

#include <backend/DriverEnums.h>

#include <filament/MaterialEnums.h>

#include <utils/compiler.h>
#include <utils/Panic.h>
#include <utils/Log.h>

#include <algorithm>
#include <vector>

#include <stddef.h>
#include <stdint.h>

namespace filamat {

std::vector<Variant> determineSurfaceVariants(
        filament::UserVariantFilterMask userVariantFilter, bool isLit, bool shadowMultiplier) {
    std::vector<Variant> variants;
    for (size_t k = 0; k < filament::VARIANT_COUNT; k++) {
        filament::Variant const variant(k);
        if (filament::Variant::isReserved(variant)) {
            continue;
        }

        filament::Variant filteredVariant =
                filament::Variant::filterUserVariant(variant, userVariantFilter);

        // Remove variants for unlit materials
        filteredVariant = filament::Variant::filterVariant(
                filteredVariant, isLit || shadowMultiplier);

        auto const vertexVariant = filament::Variant::filterVariantVertex(filteredVariant);
        if (vertexVariant == variant) {
            variants.emplace_back(variant, filament::backend::ShaderStage::VERTEX);
        }

        auto const fragmentVariant = filament::Variant::filterVariantFragment(filteredVariant);
        if (fragmentVariant == variant) {
            variants.emplace_back(variant, filament::backend::ShaderStage::FRAGMENT);
        }

        // Here we make sure that the combination of vertex and fragment variants have compatible
        // PER_VIEW descriptor-set layouts. This could actually be a static/compile-time check
        // because it is entirely decided in DescriptorSets.cpp. Unfortunately it's not possible
        // to write this entirely as a constexpr.

        if (UTILS_UNLIKELY(vertexVariant != fragmentVariant)) {
            // fragment and vertex variants are different, we need to check the layouts are
            // compatible.
            using filament::ReflectionMode;
            using filament::RefractionMode;
            using filament::backend::ShaderStage;

            // And we need to do that for all configurations of the "PER_VIEW" descriptor set
            // layouts (there are eight).
            // See ShaderGenerator::getPerViewDescriptorSetLayoutWithVariant.
            for (auto reflection: {
                    ReflectionMode::SCREEN_SPACE,
                    ReflectionMode::DEFAULT }) {
                for (auto refraction: {
                        RefractionMode::SCREEN_SPACE,
                        RefractionMode::CUBEMAP,
                        RefractionMode::NONE }) {
                    auto const vdsl = ShaderGenerator::getPerViewDescriptorSetLayoutWithVariant(
                            vertexVariant, userVariantFilter, isLit, reflection, refraction);
                    auto const fdsl = ShaderGenerator::getPerViewDescriptorSetLayoutWithVariant(
                            fragmentVariant, userVariantFilter, isLit, reflection, refraction);
                    // Check that all bindings present in the vertex shader DescriptorSetLayout
                    // are also present in the fragment shader DescriptorSetLayout.
                    for (auto const& r: vdsl.bindings) {
                        if (!hasShaderType(r.stageFlags, ShaderStage::VERTEX)) {
                            // ignore descriptors that are of the fragment stage only
                            continue;
                        }
                        auto const pos = std::find_if(fdsl.bindings.begin(), fdsl.bindings.end(),
                                [r](auto const& l) {
                                    return l.count == r.count && l.type == r.type &&
                                           l.binding == r.binding && l.flags == r.flags &&
                                           l.stageFlags == r.stageFlags;
                                });

                        // A mismatch is fatal. The material is ill-formed. This typically
                        // mean a bug / inconsistency in DescriptorsSets.cpp
                        FILAMENT_CHECK_POSTCONDITION(pos != fdsl.bindings.end())
                                << "Variant " << +k << " has mismatched descriptorset layouts";
                    }
                }
            }
        }

    }
    return variants;
}

std::vector<Variant> determinePostProcessVariants() {
    std::vector<Variant> variants;
    // TODO: add a way to filter out post-process variants (e.g., the transparent variant if only
    // opaque is needed)
    for (filament::Variant::type_t k = 0; k < filament::POST_PROCESS_VARIANT_COUNT; k++) {
        filament::Variant const variant(k);
        variants.emplace_back(variant, filament::backend::ShaderStage::VERTEX);
        variants.emplace_back(variant, filament::backend::ShaderStage::FRAGMENT);
    }
    return variants;
}

std::vector<Variant> determineComputeVariants() {
    // TODO: should we have variants for compute shaders?
    std::vector<Variant> variants;
    filament::Variant variant(0);
    variants.emplace_back(variant, filament::backend::ShaderStage::COMPUTE);
    return variants;
}

} // namespace filamat
