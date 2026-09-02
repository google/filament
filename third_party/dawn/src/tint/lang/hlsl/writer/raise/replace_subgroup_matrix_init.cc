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

#include "src/tint/lang/hlsl/writer/raise/replace_subgroup_matrix_init.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/hlsl/ir/builtin_call.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// Process the module.
    void Process() {
        // Replace variable initializeres and construct instructions.
        for (auto* inst : ir.Instructions()) {
            if (auto* var = inst->As<core::ir::Var>()) {
                ProcessVar(var);
            } else if (auto* construct = inst->As<core::ir::Construct>()) {
                ProcessConstruct(construct);
            }
        }

        // Replace unreachable in functions that have a subgroup matrix in their return type.
        for (auto* func : ir.functions) {
            if (!ContainsSubgroupMatrix(func->ReturnType())) {
                continue;
            }
            core::ir::Traverse(func->Block(), [&](core::ir::Unreachable* unreachable) {
                b.InsertBefore(unreachable, [&] {  //
                    b.Return(func, MakeZeroInitializer(func->ReturnType()));
                });
                unreachable->Destroy();
            });
        }
    }

    void ProcessVar(core::ir::Var* var) {
        // A subgroup matrix var without an initializer needs to have an explicit zero-initializer.
        auto* type = var->Result()->Type()->UnwrapPtr();
        if (var->Initializer() == nullptr && ContainsSubgroupMatrix(type)) {
            b.InsertBefore(var, [&] {  //
                var->SetInitializer(MakeZeroInitializer(type));
            });
        }
    }

    void ProcessConstruct(core::ir::Construct* construct) {
        if (!ContainsSubgroupMatrix(construct->Result()->Type())) {
            return;
        }

        if (construct->Operands().IsEmpty()) {
            // Any zero-value construct that contains a subgroup matrix should be replaced.
            b.InsertBefore(construct, [&] {
                auto* new_construct = MakeZeroInitializer(construct->Result()->Type());
                construct->Result()->ReplaceAllUsesWith(new_construct);
                construct->Destroy();
            });
        } else if (auto* sm = construct->Result()->Type()->As<core::type::SubgroupMatrix>()) {
            // A non-nested subgroup matrix construct should be replaced.
            // Any non-zero construct must be a splat of a single value.
            TINT_IR_ASSERT(ir, construct->Args().size() == 1u);
            b.InsertBefore(construct, [&] {
                auto* splat = Splat(sm, construct->Args()[0]);
                construct->Result()->ReplaceAllUsesWith(splat->Result());
                construct->Destroy();
            });
        }
    }

    /// @returns true if @p type contains a subgroup matrix type
    bool ContainsSubgroupMatrix(const core::type::Type* type) {
        return tint::Switch(
            type,  //
            [&](const core::type::SubgroupMatrix*) { return true; },
            [&](const core::type::Array* arr) { return ContainsSubgroupMatrix(arr->ElemType()); },
            [&](const core::type::Struct* s) {
                for (auto* member : s->Members()) {
                    if (ContainsSubgroupMatrix(member->Type())) {
                        return true;
                    }
                }
                return false;
            },
            [](Default) { return false; });
    }

    /// Make a zero initializer for @p type, using the Splat() builtin function.
    core::ir::Value* MakeZeroInitializer(const core::type::Type* type) {
        return tint::Switch(
            type,
            [&](const core::type::SubgroupMatrix* sm) {
                core::ir::Value* zero = nullptr;
                if (sm->Type()->Is<core::type::I8>()) {
                    zero = b.Zero<i32>();
                } else if (sm->Type()->Is<core::type::U8>()) {
                    zero = b.Zero<u32>();
                } else {
                    zero = b.Zero(sm->Type());
                }
                return Splat(sm, zero)->Result();
            },
            [&](const core::type::Array* arr) {  //
                TINT_IR_ASSERT(ir, arr->ConstantCount().has_value());
                auto* el = MakeZeroInitializer(arr->ElemType());
                Vector<core::ir::Value*, 8> args;
                for (uint32_t i = 0; i < arr->ConstantCount().value(); i++) {
                    args.Push(el);
                }
                return b.Construct(arr, std::move(args))->Result();
            },
            [&](const core::type::Struct* s) {
                Vector<core::ir::Value*, 8> args;
                for (auto* member : s->Members()) {
                    if (ContainsSubgroupMatrix(member->Type())) {
                        args.Push(MakeZeroInitializer(member->Type()));
                    } else {
                        args.Push(b.Zero(member->Type()));
                    }
                }
                return b.Construct(s, std::move(args))->Result();
            },
            TINT_ICE_ON_NO_MATCH);
    }

    core::ir::Call* Splat(const core::type::SubgroupMatrix* sm, core::ir::Value* value) {
        return b.CallExplicit<hlsl::ir::BuiltinCall>(
            sm, hlsl::BuiltinFn::kSplat, Vector<core::ir::TemplateParameter, 1>{sm}, value);
    }
};

}  // namespace

Result<SuccessType> ReplaceSubgroupMatrixInit(core::ir::Module& ir) {
    AssertValid(ir, "before hlsl.ReplaceSubgroupMatrixInit");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
