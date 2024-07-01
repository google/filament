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

#ifndef TNT_FILAMENT_POSTPROCESSINGDESCRIPTORSET_H
#define TNT_FILAMENT_POSTPROCESSINGDESCRIPTORSET_H

#include "DescriptorSet.h"

#include "DescriptorSetLayout.h"

#include "TypedUniformBuffer.h"

#include <private/filament/UibStructs.h>

#include <backend/DriverApiForward.h>

namespace filament {

class FEngine;

class PostProcessDescriptorSet {
public:
    explicit PostProcessDescriptorSet() noexcept;

    void init(FEngine& engine) noexcept;

    void terminate(backend::DriverApi& driver);

    void setFrameUniforms(backend::DriverApi& driver,
            TypedUniformBuffer<PerViewUib>& uniforms) noexcept;

    void bind(backend::DriverApi& driver) noexcept;

    DescriptorSetLayout const& getLayout() const noexcept {
        return mDescriptorSetLayout;
    }

private:
    DescriptorSetLayout mDescriptorSetLayout;
    DescriptorSet mDescriptorSet;
};

} // namespace filament

#endif //TNT_FILAMENT_POSTPROCESSINGDESCRIPTORSET_H
