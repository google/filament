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

#include "src/tint/lang/hlsl/writer/raise/promote_initializers.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {

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
        /// Map of values to the new let versions
        Hashmap<core::ir::Value*, core::ir::Let*, 8> values_in_lets{};

        for (auto* block : ir.blocks.Objects()) {
            values_in_lets.Clear();

            // In the root block, all structs need to be split out, no nested structs
            bool is_root_block = block == ir.root_block;

            Process(block, is_root_block, values_in_lets);
        }
    }

    struct ValueInfo {
        core::ir::Instruction* inst;
        size_t index;
        core::ir::Value* val;
    };

    void Process(core::ir::Block* block,
                 bool is_root_block,
                 Hashmap<core::ir::Value*, core::ir::Let*, 8>& values_in_lets) {
        Vector<ValueInfo, 4> worklist;

        for (auto* inst : *block) {
            if (inst->Is<core::ir::Let>()) {
                continue;
            }
            if (inst->Is<core::ir::Var>()) {
                // In the root block we need to split struct and array vars out to turn them into
                // `static const` variables.
                if (!is_root_block) {
                    continue;
                }
            }

            // Check each operand of the instruction to determine if it's a struct or array.
            auto operands = inst->Operands();
            for (size_t i = 0; i < operands.Length(); ++i) {
                auto* operand = operands[i];
                if (!operand || !operand->Type() ||
                    !operand->Type()->IsAnyOf<core::type::Struct, core::type::Array>()) {
                    continue;
                }
                if (operand->IsAnyOf<core::ir::InstructionResult, core::ir::Constant>()) {
                    worklist.Push({inst, i, operand});
                }
            }
        }

        Vector<core::ir::Construct*, 4> const_worklist;
        for (auto& item : worklist) {
            if (auto* res = As<core::ir::InstructionResult>(item.val)) {
                // If the value isn't already a `let`, put it into a `let`.
                if (!res->Instruction()->Is<core::ir::Let>()) {
                    PutInLet(item.inst, item.index, res, values_in_lets);
                }
            } else if (auto* val = As<core::ir::Constant>(item.val)) {
                auto* let = PutInLet(item.inst, item.index, val, values_in_lets);
                auto ret = HoistModuleScopeLetToConstruct(is_root_block, item.inst, let, val);
                if (ret.has_value()) {
                    const_worklist.Insert(0, *ret);
                }
            }
        }

        // If any element in the constant is `struct` or `array` it needs to be pulled out
        // into it's own `let`. That also means the `constant` value needs to turn into a
        // `Construct`.
        while (!const_worklist.IsEmpty()) {
            auto item = const_worklist.Pop();

            tint::Slice<core::ir::Value* const> args = item->Args();
            for (size_t i = 0; i < args.Length(); ++i) {
                auto ret = ProcessConstant(args[i], item, i);
                if (ret.has_value()) {
                    const_worklist.Insert(0, *ret);
                }
            }
        }
    }

    // Process a constant operand and replace if it's a struct initializer
    std::optional<core::ir::Construct*> ProcessConstant(core::ir::Value* operand,
                                                        core::ir::Construct* parent,
                                                        size_t idx) {
        auto* const_val = operand->As<core::ir::Constant>();
        TINT_ASSERT(const_val);

        if (!const_val->Type()->Is<core::type::Struct>()) {
            return std::nullopt;
        }

        auto* let = b.Let(const_val->Type());

        Vector<core::ir::Value*, 4> new_args = GatherArgs(const_val);

        auto* construct = b.Construct(const_val->Type(), new_args);
        let->SetValue(construct->Result(0));

        // Put the `let` in before the `construct` value that we're based off of
        let->InsertBefore(parent);
        // Put the new `construct` in before the `let`.
        construct->InsertBefore(let);

        // Replace the argument in the originating `construct` with the new `let`.
        parent->SetArg(idx, let->Result(0));

        return {construct};
    }

    // Determine if this is a root block var which contains a struct initializer and, if
    // so, setup the instruction for the needed replacement.
    std::optional<core::ir::Construct*> HoistModuleScopeLetToConstruct(bool is_root_block,
                                                                       core::ir::Instruction* inst,
                                                                       core::ir::Let* let,
                                                                       core::ir::Constant* val) {
        // Only care about root-block variables
        if (!is_root_block || !inst->Is<core::ir::Var>()) {
            return std::nullopt;
        }
        // Only care about struct constants
        if (!val->Type()->Is<core::type::Struct>()) {
            return std::nullopt;
        }

        // This may not actually need to be a `construct` but pull it out now to
        // make further changes, if they're necessary, easier.
        Vector<core::ir::Value*, 4> args = GatherArgs(val);

        // Turn the `constant` into a `construct` call and replace the value of the `let` that
        // was created.
        auto* construct = b.Construct(val->Type(), args);
        let->SetValue(construct->Result(0));
        construct->InsertBefore(let);

        return {construct};
    }

    // Gather the arguments to the constant and create a `ir::Value` array from them which can
    // be used in a `construct`.
    Vector<core::ir::Value*, 4> GatherArgs(core::ir::Constant* val) {
        Vector<core::ir::Value*, 4> args;
        if (auto* const_val = val->Value()->As<core::constant::Composite>()) {
            for (auto v : const_val->elements) {
                args.Push(b.Constant(v));
            }
        } else if (auto* splat_val = val->Value()->As<core::constant::Splat>()) {
            for (uint32_t i = 0; i < splat_val->NumElements(); i++) {
                args.Push(b.Constant(splat_val->el));
            }
        }
        return args;
    }

    core::ir::Let* MakeLet(core::ir::Value* value) {
        auto* let = b.Let(value->Type());
        let->SetValue(value);

        auto name = b.ir.NameOf(value);
        if (name.IsValid()) {
            b.ir.SetName(let->Result(0), name);
            b.ir.ClearName(value);
        }
        return let;
    }

    core::ir::Let* PutInLet(core::ir::Instruction* inst,
                            size_t index,
                            core::ir::Value* value,
                            Hashmap<core::ir::Value*, core::ir::Let*, 8>& values_in_lets) {
        core::ir::Let* let = nullptr;
        if (auto res = values_in_lets.Get(value); res) {
            let = *res;
        } else {
            let = MakeLet(value);
            let->InsertBefore(inst);
            values_in_lets.Add(value, let);
        }

        inst->SetOperand(index, let->Result(0));
        return let;
    }
};

}  // namespace

Result<SuccessType> PromoteInitializers(core::ir::Module& ir) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "hlsl.PromoteInitializers", kPromoteInitializersCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
