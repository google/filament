/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DESCRIPTORSETS_H
#define TNT_FILAMENT_DESCRIPTORSETS_H

#include <backend/DriverEnums.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/Variant.h>

#include <filament/MaterialEnums.h>

#include <utils/CString.h>

namespace filament::descriptor_sets {

backend::DescriptorSetLayout const& getPostProcessLayout() noexcept;
backend::DescriptorSetLayout const& getDepthVariantLayout() noexcept;
backend::DescriptorSetLayout const& getSsrVariantLayout() noexcept;
backend::DescriptorSetLayout const& getPerRenderableLayout() noexcept;

backend::DescriptorSetLayout getPerViewDescriptorSetLayout(
        MaterialDomain domain,
        UserVariantFilterMask variantFilter,
        bool isLit,
        ReflectionMode reflectionMode,
        RefractionMode refractionMode) noexcept;

backend::DescriptorSetLayout getPerViewDescriptorSetLayoutWithVariant(
        Variant variant,
        MaterialDomain domain,
        UserVariantFilterMask variantFilter,
        bool isLit,
        ReflectionMode reflectionMode,
        RefractionMode refractionMode) noexcept;

utils::CString getDescriptorName(
        DescriptorSetBindingPoints set,
        backend::descriptor_binding_t binding) noexcept;

} // namespace filament::descriptor_sets


#endif //TNT_FILAMENT_DESCRIPTORSETS_H
