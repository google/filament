// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/robustness.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/ast/transform/hoist_to_decl_before.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/builtin_fn.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/load.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::Robustness);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::Robustness::Config);

namespace tint::ast::transform {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform
struct Robustness::State {
    /// Constructor
    /// @param p the source program
    /// @param c the transform config
    State(const Program& p, Config&& c) : src(p), cfg(std::move(c)) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        if (HasAction(Action::kPredicate)) {
            AddPredicateParameters();
        }

        // Walk all the AST nodes in the module, starting with the leaf nodes.
        // The most deeply nested expressions will come first.
        for (auto* node : ctx.src->ASTNodes().Objects()) {
            Switch(
                node,  //
                [&](const IndexAccessorExpression* e) {
                    // obj[idx]
                    // Array, matrix and vector indexing may require robustness transformation.
                    auto* expr = sem.Get(e)->Unwrap()->As<sem::IndexAccessorExpression>();
                    if (IsIgnoredResourceBinding(expr->Object()->RootIdentifier())) {
                        return;
                    }
                    if (cfg.disable_runtime_sized_array_index_clamping &&
                        IsIndexAccessingRuntimeSizedArray(expr)) {
                        // Ensure the index is always u32 as using a negative index is an undefined
                        // behavior in SPIRV.
                        auto* idx = CastToU32(expr->Index());
                        ctx.Replace(expr->Declaration()->index, idx);
                        return;
                    }
                    switch (ActionFor(expr)) {
                        case Action::kPredicate:
                            PredicateIndexAccessor(expr);
                            break;
                        case Action::kClamp:
                            ClampIndexAccessor(expr);
                            break;
                        case Action::kIgnore:
                            break;
                    }
                },
                [&](const IdentifierExpression* e) {
                    // Identifiers may resolve to pointer lets, which may be predicated.
                    // Inspect.
                    if (auto* user = sem.Get<sem::VariableUser>(e)) {
                        auto* v = user->Variable();
                        if (v->Type()->Is<core::type::Pointer>()) {
                            // Propagate predicate from pointer
                            if (auto pred = predicates.Get(v->Declaration()->initializer)) {
                                predicates.Add(e, *pred);
                            }
                        }
                    }
                },
                [&](const AccessorExpression* e) {
                    // obj.member
                    // Propagate the predication from the object to this expression.
                    if (auto pred = predicates.Get(e->object)) {
                        predicates.Add(e, *pred);
                    }
                },
                [&](const UnaryOpExpression* e) {
                    // Includes address-of, or indirection
                    // Propagate the predication from the inner expression to this expression.
                    if (auto pred = predicates.Get(e->expr)) {
                        predicates.Add(e, *pred);
                    }
                },
                [&](const AssignmentStatement* s) {
                    if (auto pred = predicates.Get(s->lhs)) {
                        // Assignment target is predicated
                        // Replace statement with condition on the predicate
                        ctx.Replace(s, b.If(*pred, b.Block(ctx.Clone(s))));
                    }
                },
                [&](const CompoundAssignmentStatement* s) {
                    if (auto pred = predicates.Get(s->lhs)) {
                        // Assignment expression is predicated
                        // Replace statement with condition on the predicate
                        ctx.Replace(s, b.If(*pred, b.Block(ctx.Clone(s))));
                    }
                },
                [&](const IncrementDecrementStatement* s) {
                    if (auto pred = predicates.Get(s->lhs)) {
                        // Assignment expression is predicated
                        // Replace statement with condition on the predicate
                        ctx.Replace(s, b.If(*pred, b.Block(ctx.Clone(s))));
                    }
                },
                [&](const CallExpression* e) {
                    if (auto* call = sem.Get<sem::Call>(e)) {
                        Switch(
                            call->Target(),  //
                            [&](const sem::BuiltinFn* builtin) {
                                // Calls to builtins may require robustness transformation.
                                // Inspect.
                                if (builtin->IsTexture()) {
                                    switch (cfg.texture_action) {
                                        case Action::kPredicate:
                                            PredicateTextureBuiltin(call, builtin);
                                            break;
                                        case Action::kClamp:
                                            ClampTextureBuiltin(call, builtin);
                                            break;
                                        case Action::kIgnore:
                                            break;
                                    }
                                } else {
                                    MaybePredicateNonTextureBuiltin(call, builtin);
                                }
                            },
                            [&](const sem::Function* fn) {
                                // Calls to user function may require passing additional predicate
                                // arguments.
                                InsertPredicateArguments(call, fn);
                            });
                    }
                });

            // Check whether the node is an expression that:
            // * Has a predicate
            // * Is of a non-pointer or non-reference type
            // If the above is true, then we need to predicate evaluation of this expression by
            // replacing `expr` with `predicated_expr` and injecting the following above the
            // expression's statement:
            //
            //   var predicated_expr : expr_ty;
            //   if (predicate) {
            //     predicated_expr = expr;
            //   }
            //
            if (auto* expr = node->As<Expression>()) {
                if (auto pred = predicates.Get(expr)) {
                    // Expression is predicated
                    auto* sem_expr = sem.GetVal(expr);
                    if (!sem_expr->Type()->Is<core::type::MemoryView>()) {
                        auto pred_load = b.Symbols().New("predicated_expr");
                        auto ty = CreateASTTypeFor(ctx, sem_expr->Type());
                        hoist.InsertBefore(sem_expr->Stmt(), b.Decl(b.Var(pred_load, ty)));
                        hoist.InsertBefore(
                            sem_expr->Stmt(),
                            b.If(*pred, b.Block(b.Assign(pred_load, ctx.Clone(expr)))));
                        ctx.Replace(expr, b.Expr(pred_load));

                        // The predication has been consumed for this expression.
                        // Don't predicate expressions that use this expression.
                        predicates.Remove(expr);
                    }
                }
            }
        }

        ctx.Clone();
        return resolver::Resolve(b);
    }

  private:
    /// The source program
    const Program& src;
    /// The transform's config
    Config cfg;
    /// The target program builder
    ProgramBuilder b{};
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};
    /// Helper for hoisting declarations
    HoistToDeclBefore hoist{ctx};
    /// Alias to the source program's semantic info
    const sem::Info& sem = ctx.src->Sem();
    /// Map of expression to predicate condition
    Hashmap<const Expression*, Symbol, 32> predicates{};

    /// @return the `u32` typed expression that represents the maximum indexable value for the index
    /// accessor @p expr, or nullptr if there is no robustness limit for this expression.
    const Expression* DynamicLimitFor(const sem::IndexAccessorExpression* expr) {
        auto* obj_type = expr->Object()->Type();
        return Switch(
            obj_type->UnwrapPtrOrRef(),  //
            [&](const core::type::Vector* vec) -> const Expression* {
                if (expr->Index()->ConstantValue() || expr->Index()->Is<sem::Swizzle>()) {
                    // Index and size is constant.
                    // Validation will have rejected any OOB accesses.
                    return nullptr;
                }
                return b.Expr(u32(vec->Width() - 1u));
            },
            [&](const core::type::Matrix* mat) -> const Expression* {
                if (expr->Index()->ConstantValue()) {
                    // Index and size is constant.
                    // Validation will have rejected any OOB accesses.
                    return nullptr;
                }
                return b.Expr(u32(mat->Columns() - 1u));
            },
            [&](const core::type::Array* arr) -> const Expression* {
                if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                    // Size is unknown until runtime.
                    // Must clamp, even if the index is constant.

                    auto* arr_ptr = b.AddressOf(ctx.Clone(expr->Object()->Declaration()));
                    return b.Sub(b.Call(wgsl::BuiltinFn::kArrayLength, arr_ptr), 1_u);
                }
                if (auto count = arr->ConstantCount()) {
                    if (expr->Index()->ConstantValue()) {
                        // Index and size is constant.
                        // Validation will have rejected any OOB accesses.
                        return nullptr;
                    }
                    return b.Expr(u32(count.value() - 1u));
                }
                // Note: Don't be tempted to use the array override variable as an expression here,
                // the name might be shadowed!
                b.Diagnostics().AddError(Source{}) << core::type::Array::kErrExpectedConstantCount;
                return nullptr;
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Transform the program to insert additional predicate parameters to all user functions that
    /// have a pointer parameter type in an address space that has predicate action.
    void AddPredicateParameters() {
        for (auto* fn : src.AST().Functions()) {
            for (auto* param : fn->params) {
                auto* sem_param = sem.Get(param);
                if (auto* ptr = sem_param->Type()->As<core::type::Pointer>()) {
                    if (ActionFor(ptr->AddressSpace()) == Action::kPredicate) {
                        auto name = b.Symbols().New(param->name->symbol.Name() + "_predicate");
                        ctx.InsertAfter(fn->params, param, b.Param(name, b.ty.bool_()));

                        // Associate the pointer parameter expressions with the predicate.
                        for (auto* user : sem_param->Users()) {
                            predicates.Add(user->Declaration(), name);
                        }
                    }
                }
            }
        }
    }

    /// Transforms call expressions to user functions, inserting additional predicate arguments
    /// after all pointer parameters with a type in an address space that has predicate action.
    void InsertPredicateArguments(const sem::Call* call, const sem::Function* fn) {
        auto* expr = call->Declaration();
        for (size_t i = 0; i < fn->Parameters().Length(); i++) {
            auto* param = fn->Parameters()[i];
            if (auto* ptr = param->Type()->As<core::type::Pointer>()) {
                if (ActionFor(ptr->AddressSpace()) == Action::kPredicate) {
                    auto* arg = expr->args[i];
                    if (auto predicate = predicates.Get(arg)) {
                        ctx.InsertAfter(expr->args, arg, b.Expr(*predicate));
                    } else {
                        ctx.InsertAfter(expr->args, arg, b.Expr(true));
                    }
                }
            }
        }
    }

    /// Applies predication to the index on an array, vector or matrix.
    /// @param expr the index accessor expression.
    void PredicateIndexAccessor(const sem::IndexAccessorExpression* expr) {
        auto* obj = expr->Object()->Declaration();
        auto* idx = expr->Index()->Declaration();
        auto* max = DynamicLimitFor(expr);
        if (!max) {
            // robustness is not required
            // Just propagate predicate from object
            if (auto pred = predicates.Get(obj)) {
                predicates.Add(expr->Declaration(), *pred);
            }
            return;
        }

        auto* stmt = expr->Stmt();
        auto& obj_pred = predicates.GetOrAddZero(obj);

        auto idx_let = b.Symbols().New("index");
        auto pred = b.Symbols().New("predicate");

        hoist.InsertBefore(stmt, b.Decl(b.Let(idx_let, ctx.Clone(idx))));
        ctx.Replace(idx, b.Expr(idx_let));

        auto* cond = b.LessThanEqual(b.Call<u32>(b.Expr(idx_let)), max);
        if (obj_pred.IsValid()) {
            cond = b.And(b.Expr(obj_pred), cond);
        }
        hoist.InsertBefore(stmt, b.Decl(b.Let(pred, cond)));

        predicates.Add(expr->Declaration(), pred);
    }

    /// Applies bounds clamping to the index on an array, vector or matrix.
    /// @param expr the index accessor expression.
    void ClampIndexAccessor(const sem::IndexAccessorExpression* expr) {
        auto* max = DynamicLimitFor(expr);
        if (!max) {
            return;  // robustness is not required
        }

        auto* expr_sem = expr->Unwrap()->As<sem::IndexAccessorExpression>();
        auto idx = CastToU32(expr_sem->Index());
        auto* clamped_idx = b.Call(wgsl::BuiltinFn::kMin, idx, max);
        ctx.Replace(expr->Declaration()->index, clamped_idx);
    }

    /// Applies predication to the non-texture builtin call, if required.
    void MaybePredicateNonTextureBuiltin(const sem::Call* call, const sem::BuiltinFn* builtin) {
        // Gather the predications for the builtin arguments
        const Expression* predicate = nullptr;
        for (auto* arg : call->Declaration()->args) {
            if (auto pred = predicates.Get(arg)) {
                predicate = And(predicate, b.Expr(*pred));
            }
        }

        if (predicate) {
            if (builtin->Fn() == wgsl::BuiltinFn::kWorkgroupUniformLoad) {
                // https://www.w3.org/TR/WGSL/#workgroupUniformLoad-builtin:
                //  "Executes a control barrier synchronization function that affects memory and
                //   atomic operations in the workgroup address space."
                // Because the call acts like a control barrier, we need to make sure that we still
                // trigger a workgroup barrier if the predicate fails.
                PredicateCall(call, predicate,
                              b.Block(b.CallStmt(b.Call(wgsl::BuiltinFn::kWorkgroupBarrier))));
            } else {
                PredicateCall(call, predicate);
            }
        }
    }

    /// Applies predication to texture builtins, based on whether the coordinates, array index and
    /// level arguments are all in bounds.
    void PredicateTextureBuiltin(const sem::Call* call, const sem::BuiltinFn* builtin) {
        if (!TextureBuiltinNeedsRobustness(builtin->Fn())) {
            return;
        }

        auto* expr = call->Declaration();
        auto* stmt = call->Stmt();

        // Indices of the mandatory texture and coords parameters, and the optional
        // array and level parameters.
        auto& signature = builtin->Signature();
        auto texture_arg_idx = signature.IndexOf(core::ParameterUsage::kTexture);
        auto coords_arg_idx = signature.IndexOf(core::ParameterUsage::kCoords);
        auto array_arg_idx = signature.IndexOf(core::ParameterUsage::kArrayIndex);
        auto level_arg_idx = signature.IndexOf(core::ParameterUsage::kLevel);

        auto* texture_arg = expr->args[static_cast<size_t>(texture_arg_idx)];

        // Build the builtin predicate from the arguments
        const Expression* predicate = nullptr;

        Symbol level_idx, num_levels;
        if (level_arg_idx >= 0) {
            auto* param = builtin->Parameters()[static_cast<size_t>(level_arg_idx)];
            if (param->Type()->IsIntegerScalar()) {
                // let level_idx = u32(level-arg);
                level_idx = b.Symbols().New("level_idx");
                auto* arg = expr->args[static_cast<size_t>(level_arg_idx)];
                hoist.InsertBefore(stmt,
                                   b.Decl(b.Let(level_idx, CastToUnsigned(ctx.Clone(arg), 1u))));

                // let num_levels = textureNumLevels(texture-arg);
                num_levels = b.Symbols().New("num_levels");
                hoist.InsertBefore(
                    stmt, b.Decl(b.Let(num_levels, b.Call(wgsl::BuiltinFn::kTextureNumLevels,
                                                          ctx.Clone(texture_arg)))));

                // predicate: level_idx < num_levels
                predicate = And(predicate, b.LessThan(level_idx, num_levels));

                // Replace the level argument with `level_idx`
                ctx.Replace(arg, b.Expr(level_idx));
            }
        }

        Symbol coords;
        if (coords_arg_idx >= 0) {
            auto* param = builtin->Parameters()[static_cast<size_t>(coords_arg_idx)];
            if (param->Type()->IsIntegerScalarOrVector()) {
                // let coords = u32(coords-arg)
                coords = b.Symbols().New("coords");
                auto* arg = expr->args[static_cast<size_t>(coords_arg_idx)];
                hoist.InsertBefore(stmt,
                                   b.Decl(b.Let(coords, CastToUnsigned(b.Expr(ctx.Clone(arg)),
                                                                       WidthOf(param->Type())))));

                // predicate: all(coords < textureDimensions(texture))
                auto* dimensions =
                    level_idx.IsValid()
                        ? b.Call(wgsl::BuiltinFn::kTextureDimensions, ctx.Clone(texture_arg),
                                 b.Call(wgsl::BuiltinFn::kMin, b.Expr(level_idx),
                                        b.Sub(num_levels, 1_a)))
                        : b.Call(wgsl::BuiltinFn::kTextureDimensions, ctx.Clone(texture_arg));
                predicate =
                    And(predicate, b.Call(wgsl::BuiltinFn::kAll, b.LessThan(coords, dimensions)));

                // Replace the level argument with `coord`
                ctx.Replace(arg, b.Expr(coords));
            }
        }

        if (array_arg_idx >= 0) {
            // let array_idx = u32(array-arg)
            auto* arg = expr->args[static_cast<size_t>(array_arg_idx)];
            auto* num_layers = b.Call(wgsl::BuiltinFn::kTextureNumLayers, ctx.Clone(texture_arg));
            auto array_idx = b.Symbols().New("array_idx");
            hoist.InsertBefore(stmt, b.Decl(b.Let(array_idx, CastToUnsigned(ctx.Clone(arg), 1u))));

            // predicate: array_idx < textureNumLayers(texture)
            predicate = And(predicate, b.LessThan(array_idx, num_layers));

            // Replace the array index argument with `array_idx`
            ctx.Replace(arg, b.Expr(array_idx));
        }

        if (predicate) {
            PredicateCall(call, predicate);
        }
    }

    /// Applies bounds clamping to the coordinates, array index and level arguments of the texture
    /// builtin.
    void ClampTextureBuiltin(const sem::Call* call, const sem::BuiltinFn* builtin) {
        if (!TextureBuiltinNeedsRobustness(builtin->Fn())) {
            return;
        }

        auto* expr = call->Declaration();
        auto* stmt = call->Stmt();

        // Indices of the mandatory texture and coords parameters, and the optional
        // array and level parameters.
        auto& signature = builtin->Signature();
        auto texture_arg_idx = signature.IndexOf(core::ParameterUsage::kTexture);
        auto coords_arg_idx = signature.IndexOf(core::ParameterUsage::kCoords);
        auto array_arg_idx = signature.IndexOf(core::ParameterUsage::kArrayIndex);
        auto level_arg_idx = signature.IndexOf(core::ParameterUsage::kLevel);

        auto* texture_arg = expr->args[static_cast<size_t>(texture_arg_idx)];

        // If the level is provided, then we need to clamp this. As the level is used by
        // textureDimensions() and the texture[Load|Store]() calls, we need to clamp both usages.
        Symbol level_idx;
        if (level_arg_idx >= 0) {
            const auto* param = builtin->Parameters()[static_cast<size_t>(level_arg_idx)];
            if (param->Type()->IsIntegerScalar()) {
                const auto* arg = expr->args[static_cast<size_t>(level_arg_idx)];
                level_idx = b.Symbols().New("level_idx");
                const auto* num_levels =
                    b.Call(wgsl::BuiltinFn::kTextureNumLevels, ctx.Clone(texture_arg));
                const auto* max = b.Sub(num_levels, 1_a);
                hoist.InsertBefore(
                    stmt, b.Decl(b.Let(level_idx, b.Call(wgsl::BuiltinFn::kMin,
                                                         b.Call<u32>(ctx.Clone(arg)), max))));
                ctx.Replace(arg, b.Expr(level_idx));
            }
        }

        // Clamp the coordinates argument
        if (coords_arg_idx >= 0) {
            const auto* param = builtin->Parameters()[static_cast<size_t>(coords_arg_idx)];
            if (param->Type()->IsIntegerScalarOrVector()) {
                auto* arg = expr->args[static_cast<size_t>(coords_arg_idx)];
                const auto width = WidthOf(param->Type());
                const auto* dimensions =
                    level_idx.IsValid()
                        ? b.Call(wgsl::BuiltinFn::kTextureDimensions, ctx.Clone(texture_arg),
                                 level_idx)
                        : b.Call(wgsl::BuiltinFn::kTextureDimensions, ctx.Clone(texture_arg));

                // dimensions is u32 or vecN<u32>
                const auto* unsigned_max = b.Sub(dimensions, ScalarOrVec(b.Expr(1_a), width));
                if (param->Type()->IsSignedIntegerScalarOrVector()) {
                    const auto* zero = ScalarOrVec(b.Expr(0_a), width);
                    const auto* signed_max = CastToSigned(unsigned_max, width);
                    ctx.Replace(arg,
                                b.Call(wgsl::BuiltinFn::kClamp, ctx.Clone(arg), zero, signed_max));
                } else {
                    ctx.Replace(arg, b.Call(wgsl::BuiltinFn::kMin, ctx.Clone(arg), unsigned_max));
                }
            }
        }

        // Clamp the array_index argument, if provided
        if (array_arg_idx >= 0) {
            auto* param = builtin->Parameters()[static_cast<size_t>(array_arg_idx)];
            auto* arg = expr->args[static_cast<size_t>(array_arg_idx)];
            auto* num_layers = b.Call(wgsl::BuiltinFn::kTextureNumLayers, ctx.Clone(texture_arg));

            const auto* unsigned_max = b.Sub(num_layers, 1_a);
            if (param->Type()->IsSignedIntegerScalar()) {
                const auto* signed_max = CastToSigned(unsigned_max, 1u);
                ctx.Replace(arg, b.Call(wgsl::BuiltinFn::kClamp, ctx.Clone(arg), 0_a, signed_max));
            } else {
                ctx.Replace(arg, b.Call(wgsl::BuiltinFn::kMin, ctx.Clone(arg), unsigned_max));
            }
        }
    }

    /// @param type builtin type
    /// @returns true if the given builtin is a texture function that requires predication or
    /// clamping of arguments.
    bool TextureBuiltinNeedsRobustness(wgsl::BuiltinFn type) {
        return type == wgsl::BuiltinFn::kTextureLoad || type == wgsl::BuiltinFn::kTextureDimensions;
    }

    /// @returns a bitwise and of the two expressions, or the other expression if one is null.
    const Expression* And(const Expression* lhs, const Expression* rhs) {
        if (lhs && rhs) {
            return b.And(lhs, rhs);
        }
        if (lhs) {
            return lhs;
        }
        return rhs;
    }

    /// Transforms a call statement or expression so that the expression is predicated by @p
    /// predicate.
    /// @param else_stmt - the statement to execute for the predication failure
    void PredicateCall(const sem::Call* call,
                       const Expression* predicate,
                       const BlockStatement* else_stmt = nullptr) {
        auto* expr = call->Declaration();
        auto* stmt = call->Stmt();
        auto* call_stmt = stmt->Declaration()->As<CallStatement>();
        if (call_stmt && call_stmt->expr == expr) {
            // Wrap the statement in an if-statement with the predicate condition.
            hoist.Replace(stmt, b.If(predicate, b.Block(ctx.Clone(stmt->Declaration())),
                                     ProgramBuilder::ElseStmt(else_stmt)));
        } else {
            // Emit the following before the expression's statement:
            //   var predicated_value : return-type;
            //   if (predicate) {
            //     predicated_value = call(...);
            //   }
            auto value = b.Symbols().New("predicated_value");
            hoist.InsertBefore(stmt, b.Decl(b.Var(value, CreateASTTypeFor(ctx, call->Type()))));
            hoist.InsertBefore(stmt, b.If(predicate, b.Block(b.Assign(value, ctx.Clone(expr))),
                                          ProgramBuilder::ElseStmt(else_stmt)));

            // Replace the call expression with `predicated_value`
            ctx.Replace(expr, b.Expr(value));
        }
    }

    /// @returns true if @p action is enabled for any address space
    bool HasAction(Action action) const {
        return action == cfg.function_action ||       //
               action == cfg.texture_action ||        //
               action == cfg.private_action ||        //
               action == cfg.push_constant_action ||  //
               action == cfg.storage_action ||        //
               action == cfg.uniform_action ||        //
               action == cfg.workgroup_action;
    }

    /// @returns the robustness action to perform for an OOB access with the expression @p expr
    Action ActionFor(const sem::ValueExpression* expr) {
        return Switch(
            expr->Type(),  //
            [&](const core::type::Reference* t) { return ActionFor(t->AddressSpace()); },
            [&](Default) { return cfg.value_action; });
    }

    /// @returns the robustness action to perform for an OOB access in the address space @p
    /// address_space
    Action ActionFor(core::AddressSpace address_space) {
        switch (address_space) {
            case core::AddressSpace::kFunction:
                return cfg.function_action;
            case core::AddressSpace::kHandle:
                return cfg.texture_action;
            case core::AddressSpace::kPrivate:
                return cfg.private_action;
            case core::AddressSpace::kPushConstant:
                return cfg.push_constant_action;
            case core::AddressSpace::kStorage:
                return cfg.storage_action;
            case core::AddressSpace::kUniform:
                return cfg.uniform_action;
            case core::AddressSpace::kWorkgroup:
                return cfg.workgroup_action;
            default:
                break;
        }
        TINT_UNREACHABLE() << "unhandled address space" << address_space;
    }

    /// @returns the vector width of @p ty, or 1 if @p ty is not a vector
    static uint32_t WidthOf(const core::type::Type* ty) {
        if (auto* vec = ty->As<core::type::Vector>()) {
            return vec->Width();
        }
        return 1u;
    }

    /// @returns a scalar or vector type with the element type @p scalar and width @p width
    Type ScalarOrVecTy(Type scalar, uint32_t width) const {
        if (width > 1) {
            return b.ty.vec(scalar, width);
        }
        return scalar;
    }

    /// @returns a vector constructed with the scalar expression @p scalar if @p width > 1,
    /// otherwise returns @p scalar.
    const Expression* ScalarOrVec(const Expression* scalar, uint32_t width) {
        if (width > 1) {
            return b.Call(b.ty.vec<Infer>(width), scalar);
        }
        return scalar;
    }

    /// @returns @p val cast to a `vecN<i32>`, where `N` is @p width, or cast to i32 if @p width
    /// is 1.
    const CallExpression* CastToSigned(const Expression* val, uint32_t width) {
        return b.Call(ScalarOrVecTy(b.ty.i32(), width), val);
    }

    /// @returns @p val cast to a `vecN<u32>`, where `N` is @p width, or cast to u32 if @p width
    /// is 1.
    const CallExpression* CastToUnsigned(const Expression* val, uint32_t width) {
        return b.Call(ScalarOrVecTy(b.ty.u32(), width), val);
    }

    /// @returns true if the variable represents a resource binding that should be ignored in the
    /// robustness check.
    /// TODO(tint:1890): make this function work with unrestricted pointer paramters. Note that this
    /// depends on transform::DirectVariableAccess to have been run first.
    bool IsIgnoredResourceBinding(const sem::Variable* variable) const {
        auto* globalVariable = tint::As<sem::GlobalVariable>(variable);
        if (globalVariable == nullptr) {
            return false;
        }
        auto binding_point = globalVariable->Attributes().binding_point;
        if (!binding_point.has_value()) {
            return false;
        }
        return cfg.bindings_ignored.find(*binding_point) != cfg.bindings_ignored.cend();
    }

    /// @returns true if expr is an IndexAccessorExpression whose object is a runtime-sized array.
    bool IsIndexAccessingRuntimeSizedArray(const sem::IndexAccessorExpression* expr) {
        auto* array_type = expr->Object()->Type()->UnwrapPtrOrRef()->As<core::type::Array>();
        return array_type != nullptr && array_type->Count()->Is<core::type::RuntimeArrayCount>();
    }

    /// @returns a clone of expr->Declaration() if it is an unsigned integer scalar, or
    /// expr->Declaration() cast to u32.
    const ast::Expression* CastToU32(const sem::ValueExpression* expr) {
        auto* idx = ctx.Clone(expr->Declaration());
        if (expr->Type()->IsUnsignedIntegerScalar()) {
            return idx;
        }
        return b.Call<u32>(idx);  // u32(idx)
    }
};

Robustness::Config::Config() = default;
Robustness::Config::Config(const Config&) = default;
Robustness::Config::~Config() = default;
Robustness::Config& Robustness::Config::operator=(const Config&) = default;

Robustness::Robustness() = default;
Robustness::~Robustness() = default;

Transform::ApplyResult Robustness::Apply(const Program& src,
                                         const DataMap& inputs,
                                         DataMap&) const {
    Config cfg;
    if (auto* cfg_data = inputs.Get<Config>()) {
        cfg = *cfg_data;
    }

    return State{src, std::move(cfg)}.Run();
}

}  // namespace tint::ast::transform
