/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef TNT_FILAMENT_PUSHCONSTANTINFO_H
#define TNT_FILAMENT_PUSHCONSTANTINFO_H

#include <backend/DriverEnums.h>

#include <utils/CString.h>

namespace filament {

struct MaterialPushConstant {
    using ShaderStage = backend::ShaderStage;
    using ConstantType = backend::ConstantType;

    utils::CString name;
    ConstantType type;
    ShaderStage stage;

    MaterialPushConstant() = default;
    MaterialPushConstant(const char* name, ConstantType type, ShaderStage stage)
        : name(name),
          type(type),
          stage(stage) {}
};

}

#endif  // TNT_FILAMENT_PUSHCONSTANTINFO_H
