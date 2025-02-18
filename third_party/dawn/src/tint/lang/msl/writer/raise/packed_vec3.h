// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_MSL_WRITER_RAISE_PACKED_VEC3_H_
#define SRC_TINT_LANG_MSL_WRITER_RAISE_PACKED_VEC3_H_

#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::msl::writer::raise {

/// This transform is necessary in order to emit vec3 types with the correct size (so that scalars
/// can follow them in structures), and also to ensure that padding bytes are preserved when writing
/// to a vec3, an array of vec3 elements, or a matrix with vec3 column type.
///
/// The transform will:
/// * Replace `vec3<T>` types with an internal `__packed_vec3` type when they are used in
///   host-shareable address spaces.
/// * Wrap generated `__packed_vec3` types in a structure when they are used in arrays, so that we
///   ensure that the array has the correct element stride.
/// * Rewrite matrix types that have three rows into arrays of column vectors.
/// * Convert values that contain packed types to their original unpacked equivalents after loading
///   them from memory, and the vice versa before storing values to memory.
///
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> PackedVec3(core::ir::Module& module);

}  // namespace tint::msl::writer::raise

#endif  // SRC_TINT_LANG_MSL_WRITER_RAISE_PACKED_VEC3_H_
