/* Copyright (c) 2024 LunarG, Inc.
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

#include <stdint.h>

namespace gpuav {
namespace spirv {

struct LinkInfo {
    // SPIR-V module to link in
    const uint32_t* words;
    const uint32_t word_count;

    // Information about the function it has
    uint32_t function_id;

    // used for debugging
    const char* opname;
};

}  // namespace spirv
}  // namespace gpuav