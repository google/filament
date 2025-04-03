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

#ifndef SRC_TINT_LANG_HLSL_WRITER_RAISE_REPLACE_NON_INDEXABLE_MAT_VEC_STORES_H_
#define SRC_TINT_LANG_HLSL_WRITER_RAISE_REPLACE_NON_INDEXABLE_MAT_VEC_STORES_H_

#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::hlsl::writer::raise {

/// This transform replaces element stores to dynamically-indexed shader-local matrices or vectors
/// with full vector stores. This is done to work around FXC's inability to compile in these cases
/// (see crbug.com/42250349 and crbug.com/42251617).
///
/// This transform is similar to LocalizeStructArrayAssignment, but for matrices and vectors. As
/// with structs of arrays, FXC does not allocate indexable registers to shader-local matrix and
/// vector variables. For dynamically indexed vectors, we replace the indexed element store with
/// a full vector store using a selection mask. For dynamically indexed matrices, we replace the
/// indexed matrix store with a switch over the index, and emit a case for each column that sets the
/// matrix column by constant index.

/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> ReplaceNonIndexableMatVecStores(core::ir::Module& module);

}  // namespace tint::hlsl::writer::raise

#endif  // SRC_TINT_LANG_HLSL_WRITER_RAISE_REPLACE_NON_INDEXABLE_MAT_VEC_STORES_H_
