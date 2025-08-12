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

#include "src/tint/lang/spirv/writer/raise/keep_binding_array_as_pointer.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/binding_array.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::writer::raise {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        // Find the access instructions that need replacing.
        for (auto* inst : ir.Instructions()) {
            auto* load = inst->As<core::ir::Load>();
            if (load == nullptr) {
                continue;
            }

            auto* ba_type = load->Result()->Type()->As<core::type::BindingArray>();
            if (ba_type == nullptr) {
                continue;
            }
            TINT_ASSERT(ba_type->IsHandle());

            auto* ba_ptr = load->From();
            auto* element_ptr_ty = ty.ptr<handle>(ba_type->ElemType());

            load->Result()->ForEachUseUnsorted([&](core::ir::Usage use) {
                Switch(
                    use.instruction,
                    [&](core::ir::Access* access) {
                        b.InsertBefore(access, [&]() {
                            Vector<core::ir::Value*, 1> indices_copy = access->Indices();
                            auto* element_ptr = b.Access(element_ptr_ty, ba_ptr, indices_copy);
                            b.LoadWithResult(access->DetachResult(), element_ptr);
                            access->Destroy();
                        });
                    },
                    TINT_ICE_ON_NO_MATCH);
            });
            load->Destroy();
        }
    }
};

}  // namespace

Result<SuccessType> KeepBindingArrayAsPointer(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.KeepBindingArrayAsPointer");
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
