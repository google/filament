/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_WEBGPU_SPECIALIZATIONCONSTANTPROCESSOR_H
#define TNT_FILAMENT_BACKEND_WEBGPU_SPECIALIZATIONCONSTANTPROCESSOR_H

#include <backend/Program.h>

#include <utils/CString.h>

#include <string_view>

namespace filament::backend::webgpuutils {

// Returns a copy of the WGSL source with Program specialization values written into the default
// initializers of matching @id(n) override declarations. Constants absent from this shader stage
// are ignored. The WebGPU backend uses this instead of native pipeline override constants.
utils::CString specializeShaderSource(std::string_view source,
        Program::SpecializationConstantsInfo const& specializationConstants);

} // namespace filament::backend::webgpuutils

#endif // TNT_FILAMENT_BACKEND_WEBGPU_SPECIALIZATIONCONSTANTPROCESSOR_H
