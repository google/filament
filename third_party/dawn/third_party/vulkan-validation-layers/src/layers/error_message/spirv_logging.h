/* Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <vector>
#include <sstream>
#include <cstdint>

namespace spirv {
class Instruction;

// When producing error messages for SPIR-V related items and the user generated the shader with debug information, we can use these
// helpers to print out information from their High Level source instead of some cryptic SPIR-V jargon
//
// Due to GPU-AV not being able to create spirv::Instructions fast enough, we do most work here with a raw uint32_t byte stream
void GetShaderSourceInfo(std::ostringstream &ss, const std::vector<uint32_t> &instructions,
                         const spirv::Instruction &last_line_insn);

// These are used where we can't use normal spirv::Instructions.
// The main spot is post-processisng error message in GPU-AV, the time it takes to interchange back from a vector<uint32_t> to a
// vector<Instructions> is too high to do mid-frame. Most things just need these simple helpers
const char* GetOpString(const std::vector<uint32_t>& instructions, uint32_t string_id);
uint32_t GetConstantValue(const std::vector<uint32_t>& instructions, uint32_t constant_id);
void GetExecutionModelNames(const std::vector<uint32_t>& instructions, std::ostringstream& ss);
uint32_t GetDebugLineOffset(const std::vector<uint32_t>& instructions, uint32_t instruction_position);
}  // namespace spirv
