/* Copyright (c) 2024-2025 LunarG, Inc.
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
#include <string>

// The goal is to keep instrumentation a standalone executable for testing, but it will need runtime information interfaced with it.
// We declare all types that either the running instance of GPU-AV or the standalone executable testing will need.

namespace gpuav {
namespace spirv {

// Each descriptor set can be tought as a linear, single buffer of descriptors (ignoring binding and arrays for the moment)
//
// Example:
//    layout(binding = 0) buffer a[4];
//    layout(binding = 2) buffer b;
//    layout(binding = 3) buffer c[2];
//
// We can think of this as being in a buffer as
//    [ a0, a1, a2, a3, b0, c0, c1]
//
// In order to do this, we need some sort of LUT, per BINDING, to know where in this LAYOUT of descriptors the binding starts.
// This means given the index into any binding, we can locate the exact descriptor in the entire descriptor set.
//
// This information used to be in a BDA buffer that the GPU would do a look-up and produced slow SPIR-V to compile/execute.
// Now that we do instrumentation at Pipeline creation time, we can just view the DescriptorSetLayout and inject this offset into
// the instrumentation.
//
// ** With Variable Descriptor Count, the buffer will only get smaller from the end.
//    We will still validate as being "uninitialized" in that case.
struct BindingLayout {
    uint32_t start;
    uint32_t count;
};

// When running the DebugPrintf pass, if we detect an instrumented shader has a printf call (for debugging) we can hold them until
// we need them after GPU execution. (Note, this is needed because we don't store the instrumented SPIR-V and have no way to get the
// OpString back afterwards)
struct InternalOnlyDebugPrintf {
    uint32_t unique_shader_id;
    uint32_t op_string_id;
    std::string op_string_text;
};

}  // namespace spirv
}  // namespace gpuav