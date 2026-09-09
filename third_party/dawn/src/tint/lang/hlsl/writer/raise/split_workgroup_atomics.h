// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_HLSL_WRITER_RAISE_SPLIT_WORKGROUP_ATOMICS_H_
#define SRC_TINT_LANG_HLSL_WRITER_RAISE_SPLIT_WORKGROUP_ATOMICS_H_

#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::hlsl::writer::raise {

/// SplitWorkgroupAtomics is a transform that splits workgroup variables containing atomic members
/// into two separate variables:
///   1. A data variable with the same layout but atomic<T> replaced by T (preserving the space as
///      padding for layout compatibility).
///   2. A separate atomic<T> variable for each unique atomic leaf, preserving every array dimension
///      along the path to that leaf.
///
/// Non-atomic accesses are redirected to the data variable, and atomic accesses are redirected to
/// the corresponding atomic variable.
///
/// This transform should run after ZeroInitWorkgroupMemory and DirectVariableAccess, and before
/// DecomposeAccess, so that:
///   - Only members of the original variable are zero-initialized.
///   - Pointer parameters have been replaced with direct accesses to the variable.
///   - DecomposeAccess sees a data variable without atomics and decomposes it normally, while the
///     atomic variables are left untouched.
///
/// @param ir the module to transform
/// @returns success or failure
Result<SuccessType> SplitWorkgroupAtomics(core::ir::Module& ir);

}  // namespace tint::hlsl::writer::raise

#endif  // SRC_TINT_LANG_HLSL_WRITER_RAISE_SPLIT_WORKGROUP_ATOMICS_H_
