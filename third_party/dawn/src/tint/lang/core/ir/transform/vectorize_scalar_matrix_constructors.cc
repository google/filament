// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/vectorize_scalar_matrix_constructors.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        // Find and replace matrix constructors that take scalar operands.
        for (auto inst : ir.Instructions()) {
            if (auto* construct = inst->As<Construct>()) {
                if (construct->Result(0)->Type()->As<type::Matrix>()) {
                    if (construct->Operands().Length() > 0 &&
                        construct->Operands()[0]->Type()->Is<type::Scalar>()) {
                        b.InsertBefore(construct, [&] {  //
                            ReplaceConstructor(construct);
                        });
                    }
                }
            }
        }
    }

    /// Replace a matrix construct instruction.
    /// @param construct the instruction to replace
    void ReplaceConstructor(Construct* construct) {
        auto* mat = construct->Result(0)->Type()->As<type::Matrix>();
        auto* col = mat->ColumnType();
        const auto& scalars = construct->Operands();

        // Collect consecutive scalars into column vectors.
        Vector<Value*, 4> columns;
        for (uint32_t c = 0; c < mat->Columns(); c++) {
            Vector<Value*, 4> values;
            for (uint32_t r = 0; r < col->Width(); r++) {
                values.Push(scalars[c * col->Width() + r]);
            }
            columns.Push(b.Construct(col, std::move(values))->Result(0));
        }

        // Construct the matrix from the column vectors and replace the original instruction.
        b.ConstructWithResult(construct->DetachResult(), std::move(columns));
        construct->Destroy();
    }
};

}  // namespace

Result<SuccessType> VectorizeScalarMatrixConstructors(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.VectorizeScalarMatrixConstructors",
                                          kVectorizeScalarMatrixConstructorsCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
