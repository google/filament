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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_GLDESCRIPTORSETLAYOUT_H
#define TNT_FILAMENT_BACKEND_OPENGL_GLDESCRIPTORSETLAYOUT_H

#include "DriverBase.h"

#include <backend/DriverEnums.h>

#include <algorithm>
#include <utility>

#include <stdint.h>

namespace filament::backend {

struct GLDescriptorSetLayout : public HwDescriptorSetLayout, public DescriptorSetLayout {
    using HwDescriptorSetLayout::HwDescriptorSetLayout;
    explicit GLDescriptorSetLayout(DescriptorSetLayout&& layout) noexcept
            : DescriptorSetLayout(std::move(layout)) {

        std::sort(bindings.begin(), bindings.end(),
                [](auto&& lhs, auto&& rhs){
            return lhs.binding < rhs.binding;
        });

        auto p = std::max_element(bindings.cbegin(), bindings.cend(),
                [](auto const& lhs, auto const& rhs) {
            return lhs.binding < rhs.binding;
        });
        maxDescriptorBinding = p->binding;
    }
    uint8_t maxDescriptorBinding = 0;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_OPENGL_GLDESCRIPTORSETLAYOUT_H
