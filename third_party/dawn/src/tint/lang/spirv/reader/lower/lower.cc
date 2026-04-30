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

#include "src/tint/lang/spirv/reader/lower/lower.h"

#include "src/tint/lang/core/ir/transform/dead_code_elimination.h"
#include "src/tint/lang/core/ir/transform/remove_terminator_args.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/reader/lower/atomics.h"
#include "src/tint/lang/spirv/reader/lower/builtins.h"
#include "src/tint/lang/spirv/reader/lower/decompose_strided_array.h"
#include "src/tint/lang/spirv/reader/lower/decompose_strided_matrix.h"
#include "src/tint/lang/spirv/reader/lower/shader_io.h"
#include "src/tint/lang/spirv/reader/lower/texture.h"
#include "src/tint/lang/spirv/reader/lower/transpose_row_major.h"
#include "src/tint/lang/spirv/reader/lower/vector_element_pointer.h"

namespace tint::spirv::reader {

Result<SuccessType> Lower(core::ir::Module& mod) {
    TINT_CHECK_RESULT(core::ir::transform::DeadCodeElimination(mod));
    TINT_CHECK_RESULT(lower::VectorElementPointer(mod));
    TINT_CHECK_RESULT(lower::ShaderIO(mod));
    TINT_CHECK_RESULT(lower::Builtins(mod));

    // TransposeRowMajor must come before DecomposeStridedMatrix as we need to convert the matrices
    // first.
    TINT_CHECK_RESULT(lower::TransposeRowMajor(mod));
    // DecomposeStridedMatrix must come before DecomposeStridedArray, as it introduces strided
    // arrays that need to be replaced.
    TINT_CHECK_RESULT(lower::DecomposeStridedMatrix(mod));
    TINT_CHECK_RESULT(lower::DecomposeStridedArray(mod));
    TINT_CHECK_RESULT(lower::Atomics(mod));
    TINT_CHECK_RESULT(lower::Texture(mod));

    // Remove the terminator args at this point. There are no logical short-circuiting operators in
    // SPIR-V that we will lose track of, all the terminators are for hoisted values. We don't do
    // this on WGSL raise because `RemoveTerminatorArgs` loses the ability to determine logical `&&`
    // and `||` operators based on the terminators. Moving the logical detection to a separate
    // transform is also complicated because of the way it currently detects multi element `&&` and
    // `||` statements.
    TINT_CHECK_RESULT(core::ir::transform::RemoveTerminatorArgs(mod));

    core::ir::AssertValid(mod,
                          core::ir::Capabilities{
                              core::ir::Capability::kAllowMultipleEntryPoints,
                              core::ir::Capability::kAllowOverrides,
                          },
                          "after spirv.Lower");

    return Success;
}

}  // namespace tint::spirv::reader
