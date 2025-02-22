// Copyright (c) 2023 Google LLC.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDE_SPIRV_TOOLS_EXTRACT_SOURCE_HPP_
#define INCLUDE_SPIRV_TOOLS_EXTRACT_SOURCE_HPP_

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

// Parse a SPIR-V module, and extracts all HLSL source code from it.
// This function doesn't lift the SPIR-V code, but only relies on debug symbols.
// This means if the compiler didn't include some files, they won't show up.
//
// Returns a map of <filename, source_code> extracted from it.
// - `binary`: a vector containing the whole SPIR-V binary to extract source
// from.
// - `output`: <filename, source_code> mapping, mapping each filename
//            (if defined) to its code.
//
// Returns `true` if the extraction succeeded, `false` otherwise.
// `output` value is undefined if `false` is returned.
bool ExtractSourceFromModule(
    const std::vector<uint32_t>& binary,
    std::unordered_map<std::string, std::string>* output);

#endif  // INCLUDE_SPIRV_TOOLS_EXTRACT_SOURCE_HPP_
