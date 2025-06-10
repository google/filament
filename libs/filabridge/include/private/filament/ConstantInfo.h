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

#ifndef TNT_FILAMENT_CONSTANTINFO_H
#define TNT_FILAMENT_CONSTANTINFO_H

#include <backend/DriverEnums.h>

#include <utils/CString.h>

namespace filament {

struct MaterialConstant {
    using ConstantType = backend::ConstantType;

    utils::CString name;
    ConstantType type;

    MaterialConstant() = default;
    MaterialConstant(utils::CString name, ConstantType type) : name(std::move(name)), type(type)  {}
};

struct MaterialMutableConstant {
    utils::CString name;
    // The Filament engine stores mutable spec constants as a bitfield, meaning that must override
    // all of the default values built into in the shader souces. Therefore, unlike the immutable
    // constants above, we have to store the default values in the material metadata.
    bool defaultValue;

    MaterialMutableConstant() = default;
    MaterialMutableConstant(utils::CString name, bool defaultValue)
            : name(std::move(name)), defaultValue(defaultValue) {}
};

}

#endif  // TNT_FILAMENT_CONSTANTINFO_H
