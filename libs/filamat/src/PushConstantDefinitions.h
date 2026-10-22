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

#ifndef TNT_FILAMAT_PUSH_CONSTANT_DEFINTITIONS_H
#define TNT_FILAMAT_PUSH_CONSTANT_DEFINTITIONS_H

#include <private/filament/EngineEnums.h>
#include <private/filament/PushConstantInfo.h>

#include <utils/FixedCapacityVector.h>

#include <tuple>

namespace filamat {

constexpr char PUSH_CONSTANT_STRUCT_VAR_NAME[] = "pushConstants";

utils::FixedCapacityVector<filament::MaterialPushConstant> const PUSH_CONSTANTS = {
    {
        "morphingBufferOffset",
        filament::backend::ConstantType::INT,
        filament::backend::ShaderStage::VERTEX,
    },
};

// Make sure that the indices defined in filabridge match the actual array indices defined here.
static_assert(static_cast<uint8_t>(filament::PushConstantIds::MORPHING_BUFFER_OFFSET) == 0u);

}// namespace filamat

#endif
