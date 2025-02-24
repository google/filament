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

#ifndef SRC_TINT_LANG_HLSL_WRITER_RAISE_LOCALIZE_STRUCT_ARRAY_ASSIGNMENT_H_
#define SRC_TINT_LANG_HLSL_WRITER_RAISE_LOCALIZE_STRUCT_ARRAY_ASSIGNMENT_H_

#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::hlsl::writer::raise {

/// This transform replaces assignment to dynamically-indexed fixed-size arrays in structs on
/// shader-local variables with code that copies the arrays to a temporary local variable, assigns
/// to the local variable, and copies the array back. This is to work around FXC's compilation
/// failure for these cases (see crbug.com/tint/1206).
///
/// As per Microsoft, this occurs because a structure instance declared in a local variable in FXC
/// maps to non-indexable register space. If that contains an array that's dynamically indexed, FXC
/// will attempt to resolve indexes to constants. This can force unrolling of loops when indexes are
/// loop-dependent.
///
/// Our workaround copies the inner array to a local variable, because local array variables are
/// mapped to the indexable register space by FXC.

/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> LocalizeStructArrayAssignment(core::ir::Module& module);

}  // namespace tint::hlsl::writer::raise

#endif  // SRC_TINT_LANG_HLSL_WRITER_RAISE_LOCALIZE_STRUCT_ARRAY_ASSIGNMENT_H_
