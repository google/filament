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

#include "src/tint/lang/core/ir/transform/remove_uniform_vector_component_loads.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// Process the module.
    void Process() {
        // Find all load_vector_element instructions that need to be replaced.
        Vector<core::ir::LoadVectorElement*, 4> worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* lve = inst->As<core::ir::LoadVectorElement>()) {
                auto* ptr = lve->From()->Type()->As<core::type::Pointer>();
                if (ptr->AddressSpace() == core::AddressSpace::kUniform) {
                    worklist.Push(lve);
                }
            }
        }

        // Replace the instructions that we found.
        for (auto* lve : worklist) {
            b.InsertBefore(lve, [&] {
                // Capture the `load` in a `let` so that it does not get inlined.
                // This is necessary for the GLSL backend, where inlining the load expression will
                // turn it into a reference that defers the load until the component is accessed.
                auto* load = b.Load(lve->From());
                auto* let = b.Let(load);
                auto* access = b.Access(lve->Result()->Type(), let, lve->Index());
                lve->Result()->ReplaceAllUsesWith(access->Result());
            });
            lve->Destroy();
        }
    }
};

}  // namespace

Result<SuccessType> RemoveUniformVectorComponentLoads(core::ir::Module& ir) {
    core::ir::AssertValid(ir,
                          core::ir::Capabilities{
                              core::ir::Capability::kAllowDuplicateBindings,
                              core::ir::Capability::kAllowNonCoreTypes,
                              core::ir::Capability::kAllow8BitIntegers,
                          },
                          "before core.RemoveUniformVectorComponentLoads");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
