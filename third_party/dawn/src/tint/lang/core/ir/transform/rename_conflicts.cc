
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

#include <variant>

#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/transform/rename_conflicts.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/scalar.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/scope_stack.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform, for a single function.
struct State {
    /// Constructor
    /// @param i the IR module
    explicit State(core::ir::Module& i) : ir(i) {}

    /// Processes the module, renaming all declarations that would prevent an identifier resolving
    /// to the correct declaration.
    void Process() {
        scopes.Push(Scope{});
        TINT_DEFER(scopes.Clear());

        RegisterModuleScopeDecls();

        // Process the types
        for (auto* ty : ir.Types()) {
            EnsureResolvable(ty);
        }

        // Process the module-scope variable declarations
        for (auto* inst : *ir.root_block) {
            Process(inst);
        }

        // Process the functions
        for (core::ir::Function* fn : ir.functions) {
            scopes.Push(Scope{});
            TINT_DEFER(scopes.Pop());
            for (auto* param : fn->Params()) {
                EnsureResolvable(param->Type());
                if (auto symbol = ir.NameOf(param); symbol.IsValid()) {
                    Declare(param, symbol);
                }
            }
            Process(fn->Block());
        }
    }

  private:
    /// Map of identifier to declaration.
    /// The declarations may be one of an core::ir::Value or core::type::Struct.
    using Scope = Hashmap<std::string_view, CastableBase*, 8>;

    /// The IR module.
    core::ir::Module& ir;

    /// Stack of scopes
    Vector<Scope, 8> scopes{};

    /// Registers all the module-scope declarations in the root-scope.
    /// Duplicate declarations with the same name will renamed.
    void RegisterModuleScopeDecls() {
        // Declare all the user types
        for (auto* ty : ir.Types()) {
            if (auto* str = ty->As<core::type::Struct>()) {
                if (!IsBuiltinStruct(str)) {
                    Declare(const_cast<core::type::Struct*>(str), str->Name());
                }
            }
        }

        // Declare all the module-scope vars
        for (auto* inst : *ir.root_block) {
            for (auto* result : inst->Results()) {
                if (auto symbol = ir.NameOf(result)) {
                    Declare(result, symbol);
                }
            }
        }

        // Declare all the functions
        for (core::ir::Function* fn : ir.functions) {
            if (auto symbol = ir.NameOf(fn); symbol.IsValid()) {
                Declare(fn, symbol);
            }
        }
    }

    /// Processes the instructions of the block
    void Process(core::ir::Block* block) {
        for (auto* inst : *block) {
            Process(inst);
        }
    }

    /// Processes an instruction, ensuring that all identifier references resolve to the correct
    /// declaration. This may involve renaming of declarations in the outer scopes.
    void Process(core::ir::Instruction* inst) {
        // Check resolving of operands
        for (auto* operand : inst->Operands()) {
            if (operand) {
                // If the operand is a constant, then ensure that type name can be resolved.
                if (auto* c = operand->As<core::ir::Constant>()) {
                    EnsureResolvable(c->Type());
                }
            }
        }

        Switch(
            inst,  //
            [&](core::ir::Loop* loop) {
                // Initializer's scope encompasses the body and continuing
                scopes.Push(Scope{});
                TINT_DEFER(scopes.Pop());
                Process(loop->Initializer());
                {
                    // Body's scope encompasses the continuing
                    scopes.Push(Scope{});
                    TINT_DEFER(scopes.Pop());
                    Process(loop->Body());
                    {
                        scopes.Push(Scope{});
                        TINT_DEFER(scopes.Pop());
                        Process(loop->Continuing());
                    }
                }
            },
            [&](core::ir::ControlInstruction* ctrl) {
                // Traverse into the control instruction's blocks
                ctrl->ForeachBlock([&](core::ir::Block* block) {
                    scopes.Push(Scope{});
                    TINT_DEFER(scopes.Pop());
                    Process(block);
                });
            },
            [&](core::ir::Var*) {
                // Ensure the var's type is resolvable
                EnsureResolvable(inst->Result(0)->Type());
            },
            [&](core::ir::Let*) {
                // Ensure the let's type is resolvable
                EnsureResolvable(inst->Result(0)->Type());
            },
            [&](core::ir::Construct*) {
                // Ensure the type of a type constructor is resolvable
                EnsureResolvable(inst->Result(0)->Type());
            },
            [&](core::ir::CoreBuiltinCall* call) {
                // Ensure builtin of a builtin call is resolvable
                auto name = tint::ToString(call->Func());
                EnsureResolvesToBuiltin(name);
            });

        // Register new operands and check their types can resolve
        for (auto* result : inst->Results()) {
            if (auto symbol = ir.NameOf(result); symbol.IsValid()) {
                Declare(result, symbol);
            }
        }
    }

    /// Ensures that the type @p type can be resolved given its identifier(s)
    void EnsureResolvable(const core::type::Type* type) {
        while (type) {
            type = tint::Switch(
                type,  //
                [&](const core::type::Scalar* s) {
                    EnsureResolvesToBuiltin(s->FriendlyName());
                    return nullptr;
                },
                [&](const core::type::Vector* v) {
                    EnsureResolvesToBuiltin("vec" + tint::ToString(v->Width()));
                    return v->Type();
                },
                [&](const core::type::Matrix* m) {
                    EnsureResolvesToBuiltin("mat" + tint::ToString(m->Columns()) + "x" +
                                            tint::ToString(m->Rows()));
                    return m->Type();
                },
                [&](const core::type::Array* a) -> const core::type::Type* {
                    EnsureResolvesToBuiltin("array");
                    return a->ElemType();
                },
                [&](const core::type::Pointer* p) {
                    EnsureResolvesToBuiltin(tint::ToString(p->Access()));
                    EnsureResolvesToBuiltin(tint::ToString(p->AddressSpace()));
                    return p->StoreType();
                },
                [&](const core::type::Struct* s) {
                    auto name = s->Name().NameView();
                    if (IsBuiltinStruct(s)) {
                        EnsureResolvesToBuiltin(name);
                    }
                    return nullptr;
                });
        }
    }

    /// Ensures that the identifier @p identifier resolves to a builtin symbol.
    void EnsureResolvesToBuiltin(std::string_view identifier) {
        for (auto& scope : tint::Reverse(scopes)) {
            if (auto decl = scope.Get(identifier)) {
                // Operand is shadowed
                scope.Remove(identifier);
                Rename(*decl, identifier);
            }
        }
    }

    /// Registers the declaration @p thing in the current scope with the name @p name
    /// If there is an existing declaration with the given name in any parent scope then @p thing
    /// will be renamed.
    void Declare(CastableBase* thing, Symbol name) {
        // Check if the declaration would shadow another declaration in the current scope or any
        // parent scope, and rename it if so.
        for (auto& scope : tint::Reverse(scopes)) {
            if (auto decl = scope.Get(name.NameView())) {
                if (*decl.value != thing) {
                    name = Rename(thing, name.NameView());
                    break;
                }
            }
        }

        // Add the declaration to the current scope, and make sure that it was either successfully
        // added or has already been added.
        auto add = scopes.Back().Add(name.NameView(), thing);
        TINT_ASSERT(add || add.value == thing);
    }

    /// Rename changes the name of @p thing with the old name of @p old_name
    /// @returns the new name
    Symbol Rename(CastableBase* thing, std::string_view old_name) {
        Symbol new_name = ir.symbols.New(old_name);
        Switch(
            thing,  //
            [&](core::ir::Value* value) { ir.SetName(value, new_name); },
            [&](core::type::Struct* str) { str->SetName(new_name); },  //
            TINT_ICE_ON_NO_MATCH);
        return new_name;
    }

    /// @return true if @p s is a builtin (non-user declared) structure.
    bool IsBuiltinStruct(const core::type::Struct* s) {
        return tint::HasPrefix(s->Name().NameView(), "__");
    }
};

}  // namespace

Result<SuccessType> RenameConflicts(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.RenameConflicts", kRenameConflictsCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
