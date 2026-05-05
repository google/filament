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

#include "src/tint/lang/core/ir/transform/propagate_buffer_sizes.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/hashset.h"

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

    void Process() {
        auto ordered_funcs = ir.DependencyOrderedFunctions();
        for (auto* func : ordered_funcs) {
            ProcessFunction(func);
        }
    }

    void ProcessFunction(Function* func) {
        // For each buffer_view call in `func`:
        // 1. If the call is rooted by a constant-sized buffer, propagate the size
        // 2. If the call is rooted by a runtime-sized param, add to `calls`
        // 3. Skip runtime-sized variable roots.
        Hashmap<FunctionParam*, Vector<CoreBuiltinCall*, 4>, 8> calls;
        Traverse(func->Block(), [this, &calls](CoreBuiltinCall* call) {
            if (call->Func() != BuiltinFn::kBufferView &&
                call->Func() != BuiltinFn::kBufferArrayView &&
                call->Func() != BuiltinFn::kBufferLength) {
                return;
            }

            auto* root = RootIdentifier(call->Args()[0]);
            auto* buffer_ty = root->Type()->UnwrapPtr()->As<type::Buffer>();
            if (buffer_ty->Count()->Is<type::RuntimeArrayCount>()) {
                if (auto* param = root->As<FunctionParam>()) {
                    calls.GetOrAddZero(param).Push(call);
                }
                return;
            }

            TINT_IR_ASSERT(ir, buffer_ty->Count()->Is<type::ConstantArrayCount>());
            call->AppendArg(b.Constant(u32(buffer_ty->ConstantCount().value())));
        });

        if (calls.IsEmpty()) {
            return;
        }

        // For each parameter tracked above:
        // 1. Find the roots (either a sized parameter or global var).
        // 2. If there is a single root:
        //    i.  If the count is constant, propagate it
        //    ii. Otherwise, skip it.
        // 3. If there are multiple roots:
        //    i.   Add a parameter to the function
        //    ii.  Propagate that parameter to each associated buffer view call
        //    iii. For each call to `func`, append a new call to bufferLength as the new arg
        //
        // Since function are processed in dependency order, this will build a chain of bufferLength
        // calls. Each bufferLength call will be processed similarly and have the length appended.
        // Later extra calls can be elided.
        auto func_uses = func->UsagesSorted();
        auto num_params = func->Params().Length();
        for (size_t i = 0; i < num_params; i++) {
            auto* param = func->Params()[i];
            if (!calls.Contains(param)) {
                continue;
            }

            Hashset<Value*, 4> roots;
            TraceRoots(param, roots);
            if (roots.Count() == 1) {
                auto* root = roots.Vector()[0];
                auto* buffer_ty = root->Type()->UnwrapPtr()->As<type::Buffer>();
                auto count = buffer_ty->ConstantCount();
                if (count != std::nullopt) {
                    auto param_calls = calls.Get(param);
                    for (auto* buffer_call : *param_calls) {
                        buffer_call->AppendArg(b.Constant(u32(count.value())));
                    }
                }

                continue;
            }

            for (auto use : func_uses) {
                auto* inst = use.instruction;
                if (!inst->Is<UserCall>()) {
                    continue;
                }

                auto* call = inst->As<UserCall>();
                b.InsertBefore(call, [&] {
                    auto len =
                        b.Call(ty.u32(), BuiltinFn::kBufferLength, call->Args()[param->Index()]);
                    call->AppendArg(len->Result());
                });
            }

            auto new_param = b.FunctionParam(ty.u32());
            func->AppendParam(new_param);
            auto param_calls = calls.Get(param);
            for (auto* buffer_call : *param_calls) {
                buffer_call->AppendArg(new_param);
            }
        }
    }

    /// Find the roots of `param`. Each root is either:
    /// * A global variable, or
    /// * A constant-sized buffer function parameter
    /// @param param The function parameter
    /// @param roots The set of roots
    void TraceRoots(FunctionParam* param, Hashset<Value*, 4>& roots) {
        auto func_uses = param->Function()->UsagesSorted();
        for (auto use : func_uses) {
            auto* inst = use.instruction;
            if (!inst->Is<UserCall>()) {
                continue;
            }
            auto* call = inst->As<UserCall>();
            auto* arg = call->Args()[param->Index()];
            auto* root = RootIdentifier(arg);
            if (auto* root_param = root->As<FunctionParam>()) {
                auto* store_ty = root_param->Type()->UnwrapPtr();
                TINT_IR_ASSERT(ir, store_ty->Is<type::Buffer>());
                auto* buffer_ty = store_ty->As<type::Buffer>();
                auto* count = buffer_ty->Count();
                if (count->Is<type::RuntimeArrayCount>()) {
                    TraceRoots(root_param, roots);
                } else {
                    roots.Add(root_param);
                }
            } else {
                roots.Add(root);
            }
        }
    }

    /// Find the root identifier of `value`
    /// @param value The value (a buffer) to trace
    /// @returns The root identifier (either global var or function parameter)
    Value* RootIdentifier(Value* value) {
        Value* result = nullptr;
        while (value) {
            TINT_IR_ASSERT(ir, value->Alive());
            value = tint::Switch(
                value,  //
                [&](InstructionResult* res) {
                    // Don't need to check accesses since buffers can't be used with them.
                    auto* inst = res->Instruction();
                    return tint::Switch(
                        inst,  //
                        [&](Let* let) { return let->Value(); },
                        [&](Var* var) {
                            result = var->Result();
                            return nullptr;
                        },
                        TINT_ICE_ON_NO_MATCH);
                },
                [&](FunctionParam* param) {
                    result = param;
                    return nullptr;
                },
                TINT_ICE_ON_NO_MATCH);
        }
        return result;
    }
};
}  // namespace

Result<SuccessType> PropagateBufferSizes(Module& ir) {
    AssertValid(ir, kPropagateBufferSizesCapabilities, "before core.PropagateBufferSizes");

    State{ir}.Process();

    return Success;
}
}  // namespace tint::core::ir::transform
