// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_ANALYSIS_SUBGROUP_MATRIX_H_
#define SRC_TINT_LANG_CORE_IR_ANALYSIS_SUBGROUP_MATRIX_H_

#include "src/tint/api/common/subgroup_matrix.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::core::ir::analysis {

/// Gathers information about the subgroup matrix configurations used in the module.
///
/// This returns two fundamental types of information on the subgroup matrix uses.
///
/// 1. The Matrix multiply configurations
///   * This provides the `M`, `N`, `K`, input and output types for the
///     `subgroupMatrixMultiply` and `subgroupMatrtixMultiplyAccumulate` calls in the
///     module.  (The configs are de-duplicated so each combination is only returned once.)
///
/// 2. The used matrix configurations
///   * This provides the `C`, `R`, type and if it's left/right (input) or result usage for
///     every subgroup matrix seen in the file.  (The configs are de-duplicated so each
///     combination is only returned once.)
///
/// @param module the module to transform
/// @returns the subgroup matrix information
SubgroupMatrixInfo GatherSubgroupMatrixInfo(core::ir::Module& module);

}  // namespace tint::core::ir::analysis

#endif  // SRC_TINT_LANG_CORE_IR_ANALYSIS_SUBGROUP_MATRIX_H_
