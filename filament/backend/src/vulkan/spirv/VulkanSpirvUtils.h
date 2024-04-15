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

#ifndef TNT_FILAMENT_BACKEND_VULKANSPIRVUTILS_H
#define TNT_FILAMENT_BACKEND_VULKANSPIRVUTILS_H

#include <backend/Program.h>

#include <utils/FixedCapacityVector.h>

#include <tuple>
#include <vector>

namespace filament::backend {

using SpecConstantValue = Program::SpecializationConstant::Type;

// For certain drivers, using spec constant can lead to compile errors [1] or undesirable behaviors
// [2]. In those instances, we simply change the spirv and set them to constants.
//
// (Implemenation note: we cannot write to the blob because spirv-validator does not properly handle
//  the Nop (no-op) instruction, and swiftshader validates the shader before compilation. So we need
//  to skip those instructions instead).
//
// [1]: QC driver cannot use spec constant to size fields
//      (https://github.com/google/filament/issues/6444).
// [2]: An internal driver does not DCE a block guarded by a spec-const boolean set to false
//      (b/310603393).
void workaroundSpecConstant(Program::ShaderBlob const& blob,
        utils::FixedCapacityVector<Program::SpecializationConstant> const& specConstants,
        std::vector<uint32_t>& output);

// bindings for UBO, samplers, input attachment
std::tuple<uint32_t, uint32_t, uint32_t> getProgramBindings(Program::ShaderBlob const& blob);

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANSPIRVUTILS_H
