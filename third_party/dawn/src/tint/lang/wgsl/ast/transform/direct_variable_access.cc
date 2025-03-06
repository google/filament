// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/direct_variable_access.h"

#include <algorithm>
#include <string>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/wgsl/ast/transform/hoist_to_decl_before.h"
#include "src/tint/lang/wgsl/ast/traverse_expressions.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/text/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::DirectVariableAccess);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::DirectVariableAccess::Config);

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::ast::transform {

namespace {

/// AccessRoot describes the root of an AccessShape.
struct AccessRoot {
    /// The pointer-unwrapped type of the *transformed* variable.
    /// This may be different for pointers in 'private' and 'function' address space, as the pointer
    /// parameter type is to the *base object* instead of the input pointer type.
    tint::core::type::Type const* type = nullptr;
    /// The originating module-scope variable ('private', 'storage', 'uniform', 'workgroup'),
    /// function-scope variable ('function'), or pointer parameter in the source program.
    tint::sem::Variable const* variable = nullptr;
    /// The address space of the variable or pointer type.
    tint::core::AddressSpace address_space = tint::core::AddressSpace::kUndefined;

    /// @return a hash code for this object
    tint::HashCode HashCode() const { return Hash(type, variable); }
};

/// Inequality operator for AccessRoot
bool operator!=(const AccessRoot& a, const AccessRoot& b) {
    return a.type != b.type || a.variable != b.variable;
}

/// DynamicIndex is used by DirectVariableAccess::State::AccessOp to indicate an array, matrix or
/// vector index.
struct DynamicIndex {
    /// @return a hash code for this object
    tint::HashCode HashCode() const { return 42 /* empty struct: any number will do */; }
};

/// Inequality operator for DynamicIndex
bool operator!=(const DynamicIndex&, const DynamicIndex&) {
    return false;  // empty struct: two DynamicIndex objects are always equal
}

/// AccessOp describes a single access in an access chain.
/// The access is one of:
/// Symbol        - a struct member access.
/// DynamicIndex  - a runtime index on an array, matrix column, or vector element.
using AccessOp = std::variant<tint::Symbol, DynamicIndex>;

/// A vector of AccessOp. Describes the static "path" from a root variable to an element
/// within the variable. Array accessors index expressions are held externally to the
/// AccessShape, so AccessShape will be considered equal even if the array, matrix or vector
/// index values differ.
///
/// For example, consider the following:
///
/// ```
///           struct A {
///               x : array<i32, 8>,
///               y : u32,
///           };
///           struct B {
///               x : i32,
///               y : array<A, 4>
///           };
///           var<workgroup> C : B;
/// ```
///
/// The following AccessShape would describe the following:
///
/// +==============================+===============+=================================+
/// | AccessShape                  | Type          |  Expression                     |
/// +==============================+===============+=================================+
/// | [ Variable 'C', Symbol 'x' ] | i32           |  C.x                            |
/// +------------------------------+---------------+---------------------------------+
/// | [ Variable 'C', Symbol 'y' ] | array<A, 4>   |  C.y                            |
/// +------------------------------+---------------+---------------------------------+
/// | [ Variable 'C', Symbol 'y',  | A             |  C.y[dyn_idx[0]]                |
/// |   DynamicIndex ]             |               |                                 |
/// +------------------------------+---------------+---------------------------------+
/// | [ Variable 'C', Symbol 'y',  | array<i32, 8> |  C.y[dyn_idx[0]].x              |
/// |   DynamicIndex, Symbol 'x' ] |               |                                 |
/// +------------------------------+---------------+---------------------------------+
/// | [ Variable 'C', Symbol 'y',  | i32           |  C.y[dyn_idx[0]].x[dyn_idx[1]]  |
/// |   DynamicIndex, Symbol 'x',  |               |                                 |
/// |   DynamicIndex ]             |               |                                 |
/// +------------------------------+---------------+---------------------------------+
/// | [ Variable 'C', Symbol 'y',  | u32           |  C.y[dyn_idx[0]].y              |
/// |   DynamicIndex, Symbol 'y' ] |               |                                 |
/// +------------------------------+---------------+---------------------------------+
///
/// Where: `dyn_idx` is the AccessChain::dynamic_indices.
struct AccessShape {
    // The originating variable.
    AccessRoot root;
    /// The chain of access ops.
    tint::Vector<AccessOp, 8> ops;

    /// @returns the number of DynamicIndex operations in #ops.
    uint32_t NumDynamicIndices() const {
        uint32_t count = 0;
        for (auto& op : ops) {
            if (std::holds_alternative<DynamicIndex>(op)) {
                count++;
            }
        }
        return count;
    }

    /// @return a hash code for this object
    tint::HashCode HashCode() const { return Hash(root, ops); }
};

/// Equality operator for AccessShape
bool operator==(const AccessShape& a, const AccessShape& b) {
    return !(a.root != b.root) && a.ops == b.ops;
}

/// Inequality operator for AccessShape
bool operator!=(const AccessShape& a, const AccessShape& b) {
    return !(a == b);
}

/// AccessChain describes a chain of access expressions originating from a variable.
struct AccessChain : AccessShape {
    /// The array accessor index expressions. This vector is indexed by the `DynamicIndex`s in
    /// #indices.
    Vector<const sem::ValueExpression*, 8> dynamic_indices;
    /// If true, then this access chain is used as an argument to call a variant.
    bool used_in_call = false;
};

}  // namespace

/// The PIMPL state for the DirectVariableAccess transform
struct DirectVariableAccess::State {
    /// Constructor
    /// @param src the source Program
    /// @param options the transform options
    State(const Program& src, const Options& options)
        : ctx{&b, &src, /* auto_clone_symbols */ true}, opts(options) {}

    /// The main function for the transform.
    /// @returns the ApplyResult
    ApplyResult Run() {
        // If there are no functions with pointer parameters, then this transform can be skipped.
        if (!AnyPointerParameters()) {
            return SkipTransform;
        }

        // Stage 1:
        // Walk all the expressions of the program, starting with the expression leaves.
        // Whenever we find an identifier resolving to a var, pointer parameter or pointer let to
        // another chain, start constructing an access chain. When chains are accessed, these chains
        // are grown and moved up the expression tree. After this stage, we are left with all the
        // expression access chains to variables that we may need to transform.
        for (auto* node : ctx.src->ASTNodes().Objects()) {
            if (auto* expr = sem.GetVal(node)) {
                AppendAccessChain(expr);
            }
        }

        // Stage 2:
        // Walk the functions in dependency order, starting with the entry points.
        // Construct the set of function 'variants' by examining the calls made by each function to
        // their call target. Each variant holds a map of pointer parameter to access chains, and
        // will have the pointer parameters replaced with an array of u32s, used to perform the
        // pointer indexing in the variant.
        // Function call pointer arguments are replaced with an array of these dynamic indices.
        auto decls = sem.Module()->DependencyOrderedDeclarations();
        for (auto* decl : tint::Reverse(decls)) {
            if (auto* fn = sem.Get<sem::Function>(decl)) {
                auto* fn_info = FnInfoFor(fn);
                ProcessFunction(fn, fn_info);
                TransformFunction(fn, fn_info);
            }
        }

        // Stage 3:
        // Filter out access chains that do not need transforming.
        // Ensure that chain dynamic index expressions are evaluated once at the correct place
        ProcessAccessChains();

        // Stage 4:
        // Replace all the access chain expressions in all functions with reconstructed expression
        // using the originating global variable, and any dynamic indices passed in to the function
        // variant.
        TransformAccessChainExpressions();

        // Stage 5:
        // Actually kick the clone.
        CloneState state;
        clone_state = &state;
        ctx.Clone();
        return resolver::Resolve(b);
    }

  private:
    /// Holds symbols of the transformed pointer parameter.
    /// If both symbols are valid, then #base_ptr and #indices are both program-unique symbols
    /// derived from the original parameter name.
    /// If only one symbol is valid, then this is the original parameter symbol.
    struct PtrParamSymbols {
        /// The symbol of the base pointer parameter.
        Symbol base_ptr;
        /// The symbol of the dynamic indicies parameter.
        Symbol indices;
    };

    /// FnVariant describes a unique variant of a function, specialized by the AccessShape of the
    /// pointer arguments - also known as the variant's "signature".
    ///
    /// To help understand what a variant is, consider the following WGSL:
    ///
    /// ```
    /// fn F(a : ptr<storage, u32>, b : u32, c : ptr<storage, u32>) {
    ///    return *a + b + *c;
    /// }
    ///
    /// @group(0) @binding(0) var<storage> S0 : u32;
    /// @group(0) @binding(0) var<storage> S1 : array<u32, 64>;
    ///
    /// fn x() {
    ///    F(&S0, 0, &S0);       // (A)
    ///    F(&S0, 0, &S0);       // (B)
    ///    F(&S1[0], 1, &S0);    // (C)
    ///    F(&S1[5], 2, &S0);    // (D)
    ///    F(&S1[5], 3, &S1[3]); // (E)
    ///    F(&S1[7], 4, &S1[2]); // (F)
    /// }
    /// ```
    ///
    /// Given the calls in x(), function F() will have 3 variants:
    /// (1) F<S0,S0>                   - called by (A) and (B).
    ///                                  Note that only 'uniform', 'storage' and 'workgroup' pointer
    ///                                  parameters are considered for a variant signature, and so
    ///                                  the argument for parameter 'b' is not included in the
    ///                                  signature.
    /// (2) F<S1[dyn_idx],S0>          - called by (C) and (D).
    ///                                  Note that the array index value is external to the
    ///                                  AccessShape, and so is not part of the variant signature.
    /// (3) F<S1[dyn_idx],S1[dyn_idx]> - called by (E) and (F).
    ///
    /// Each variant of the function will be emitted as a separate function by the transform, and
    /// would look something like:
    ///
    /// ```
    /// // variant F<S0,S0> (1)
    /// fn F_S0_S0(b : u32) {
    ///    return S0 + b + S0;
    /// }
    ///
    /// type S1_X = array<u32, 1>;
    ///
    /// // variant F<S1[dyn_idx],S0> (2)
    /// fn F_S1_X_S0(a : S1_X, b : u32) {
    ///    return S1[a[0]] + b + S0;
    /// }
    ///
    /// // variant F<S1[dyn_idx],S1[dyn_idx]> (3)
    /// fn F_S1_X_S1_X(a : S1_X, b : u32, c : S1_X) {
    ///    return S1[a[0]] + b + S1[c[0]];
    /// }
    ///
    /// @group(0) @binding(0) var<storage> S0 : u32;
    /// @group(0) @binding(0) var<storage> S1 : array<u32, 64>;
    ///
    /// fn x() {
    ///    F_S0_S0(0);                        // (A)
    ///    F(&S0, 0, &S0);                    // (B)
    ///    F_S1_X_S0(S1_X(0), 1);             // (C)
    ///    F_S1_X_S0(S1_X(5), 2);             // (D)
    ///    F_S1_X_S1_X(S1_X(5), 3, S1_X(3));  // (E)
    ///    F_S1_X_S1_X(S1_X(7), 4, S1_X(2));  // (F)
    /// }
    /// ```
    struct FnVariant {
        /// The signature of the variant is a map of each of the function's 'uniform', 'storage' and
        /// 'workgroup' pointer parameters to the caller's AccessShape.
        using Signature = Hashmap<const sem::Parameter*, AccessShape, 4>;

        /// The unique name of the variant.
        /// The symbol is in the `ctx.dst` program namespace.
        Symbol name;

        /// A map of direct calls made by this variant to the name of other function variants.
        Hashmap<const sem::Call*, Symbol, 4> calls;

        /// A map of input program parameter to output parameter symbols.
        Hashmap<const sem::Parameter*, PtrParamSymbols, 4> ptr_param_symbols;

        /// The declaration order of the variant, in relation to other variants of the same
        /// function. Used to ensure deterministic ordering of the transform, as map iteration is
        /// not deterministic between compilers.
        size_t order = 0;
    };

    /// FnInfo holds information about a function in the input program.
    struct FnInfo {
        /// A map of variant signature to the variant data.
        Hashmap<FnVariant::Signature, FnVariant, 8> variants;
        /// A map of expressions that have been hoisted to a 'let' declaration in the function.
        Hashmap<const sem::ValueExpression*, Symbol, 8> hoisted_exprs;

        /// @returns the variants of the function in a deterministically ordered vector.
        tint::Vector<std::pair<const FnVariant::Signature*, FnVariant*>, 8> SortedVariants() {
            tint::Vector<std::pair<const FnVariant::Signature*, FnVariant*>, 8> out;
            out.Reserve(variants.Count());
            for (auto& it : variants) {
                out.Push({&it.key.Value(), &it.value});
            }
            out.Sort([&](auto& va, auto& vb) { return va.second->order < vb.second->order; });
            return out;
        }
    };

    /// The program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx;
    /// The transform options
    const Options& opts;
    /// Alias to the semantic info in ctx.src
    const sem::Info& sem = ctx.src->Sem();
    /// Alias to the symbols in ctx.src
    const SymbolTable& sym = ctx.src->Symbols();
    /// Map of semantic function to the function info
    Hashmap<const sem::Function*, FnInfo*, 8> fns;
    /// Map of AccessShape to the name of a type alias for the an array<u32, N> used for the
    /// dynamic indices of an access chain, passed down as the transformed type of a variant's
    /// pointer parameter.
    Hashmap<AccessShape, Symbol, 8> dynamic_index_array_aliases;
    /// Map of semantic expression to AccessChain
    Hashmap<const sem::ValueExpression*, AccessChain*, 32> access_chains;
    /// Allocator for FnInfo
    BlockAllocator<FnInfo> fn_info_allocator;
    /// Allocator for AccessChain
    BlockAllocator<AccessChain> access_chain_allocator;
    /// Helper used for hoisting expressions to lets
    HoistToDeclBefore hoist{ctx};
    /// Map of string to unique symbol (no collisions in output program).
    Hashmap<std::string, Symbol, 8> unique_symbols;

    /// CloneState holds pointers to the current function, variant and variant's parameters.
    struct CloneState {
        /// The current function being cloned
        FnInfo* current_function = nullptr;
        /// The current function variant being built
        FnVariant* current_variant = nullptr;
        /// The signature of the current function variant being built
        const FnVariant::Signature* current_variant_sig = nullptr;
    };

    /// The clone state.
    /// Only valid during the lifetime of the program::CloneContext::Clone().
    CloneState* clone_state = nullptr;

    /// @returns true if any user functions have parameters of a pointer type.
    bool AnyPointerParameters() const {
        for (auto* fn : ctx.src->AST().Functions()) {
            for (auto* param : fn->params) {
                if (sem.Get(param)->Type()->Is<core::type::Pointer>()) {
                    return true;
                }
            }
        }
        return false;
    }

    /// AppendAccessChain creates or extends an existing AccessChain for the given expression,
    /// modifying the #access_chains map.
    void AppendAccessChain(const sem::ValueExpression* expr) {
        // take_chain moves the AccessChain from the expression `from` to the expression `expr`.
        // Returns nullptr if `from` did not hold an access chain.
        auto take_chain = [&](const sem::ValueExpression* from) -> AccessChain* {
            if (auto* chain = AccessChainFor(from)) {
                access_chains.Remove(from);
                access_chains.Add(expr, chain);
                return chain;
            }
            return nullptr;
        };

        Switch(
            expr,
            [&](const sem::VariableUser* user) {
                // Expression resolves to a variable.
                auto* variable = user->Variable();

                auto create_new_chain = [&] {
                    auto* chain = access_chain_allocator.Create();
                    chain->root.variable = variable;
                    chain->root.type = variable->Type();
                    chain->root.address_space = variable->AddressSpace();
                    if (auto* ptr = chain->root.type->As<core::type::Pointer>()) {
                        chain->root.address_space = ptr->AddressSpace();
                    }
                    access_chains.Add(expr, chain);
                };

                Switch(
                    variable->Declaration(),
                    [&](const Var*) {
                        if (variable->AddressSpace() != core::AddressSpace::kHandle) {
                            // Start a new access chain for the non-handle 'var' access
                            create_new_chain();
                        }
                    },
                    [&](const Parameter*) {
                        if (variable->Type()->Is<core::type::Pointer>()) {
                            // Start a new access chain for the pointer parameter access
                            create_new_chain();
                        }
                    },
                    [&](const Let*) {
                        if (variable->Type()->Is<core::type::Pointer>()) {
                            // variable is a pointer-let.
                            auto* init = sem.GetVal(variable->Declaration()->initializer);
                            // Note: We do not use take_chain() here, as we need to preserve the
                            // AccessChain on the let's initializer, as the let needs its
                            // initializer updated, and the let may be used multiple times. Instead
                            // we copy the let's AccessChain into a a new AccessChain.
                            if (auto* init_chain = AccessChainFor(init)) {
                                access_chains.Add(expr, access_chain_allocator.Create(*init_chain));
                            }
                        }
                    });
            },
            [&](const sem::StructMemberAccess* a) {
                // Structure member access.
                // Append the Symbol of the member name to the chain, and move the chain to the
                // member access expression.
                if (auto* chain = take_chain(a->Object())) {
                    chain->ops.Push(a->Member()->Name());
                }
            },
            [&](const sem::IndexAccessorExpression* a) {
                // Array, matrix or vector index.
                // Store the index expression into AccessChain::dynamic_indices, append a
                // DynamicIndex to the chain, and move the chain to the index accessor expression.
                if (auto* chain = take_chain(a->Object())) {
                    chain->ops.Push(DynamicIndex{});
                    chain->dynamic_indices.Push(a->Index());
                }
            },
            [&](const sem::ValueExpression* e) {
                if (auto* unary = e->Declaration()->As<UnaryOpExpression>()) {
                    // Unary op.
                    // If this is a '&' or '*', simply move the chain to the unary op expression.
                    if (unary->op == core::UnaryOp::kAddressOf ||
                        unary->op == core::UnaryOp::kIndirection) {
                        take_chain(sem.GetVal(unary->expr));
                    }
                }
            });
    }

    /// MaybeHoistDynamicIndices examines the AccessChain::dynamic_indices member of @p chain,
    /// hoisting all expressions to their own uniquely named 'let' if none of the following are
    /// true:
    /// 1. The index expression is a constant value.
    /// 2. The index expression's statement is the same as @p usage.
    /// 3. The index expression is an identifier resolving to a 'let', 'const' or parameter, AND
    ///    that identifier resolves to the same variable at @p usage.
    ///
    /// A dynamic index will only be hoisted once. The hoisting applies to all variants of the
    /// function that holds the dynamic index expression.
    void MaybeHoistDynamicIndices(AccessChain* chain, const sem::Statement* usage) {
        for (auto& idx : chain->dynamic_indices) {
            if (idx->ConstantValue()) {
                // Dynamic index is constant.
                continue;  // Hoisting not required.
            }

            if (idx->Stmt() == usage) {
                // The index expression is owned by the statement of usage.
                continue;  // Hoisting not required
            }

            if (auto* idx_variable_user = idx->UnwrapMaterialize()->As<sem::VariableUser>()) {
                auto* idx_variable = idx_variable_user->Variable();
                if (idx_variable->Declaration()->IsAnyOf<Let, Parameter>()) {
                    // Dynamic index is an immutable variable
                    continue;  // Hoisting not required.
                }
            }

            // The dynamic index needs to be hoisted (if it hasn't been already).
            auto fn = FnInfoFor(idx->Stmt()->Function());
            fn->hoisted_exprs.GetOrAdd(idx, [this, idx] {
                // Create a name for the new 'let'
                auto name = b.Symbols().New("ptr_index_save");
                // Insert a new 'let' just above the dynamic index statement.
                hoist.InsertBefore(idx->Stmt(), [this, idx, name] {
                    return b.Decl(b.Let(name, ctx.CloneWithoutTransform(idx->Declaration())));
                });
                return name;
            });
        }
    }

    /// BuildDynamicIndex builds the AST expression node for the dynamic index expression used in an
    /// AccessChain. This is similar to just cloning the expression, but BuildDynamicIndex()
    /// also:
    /// * Collapses constant value index expressions down to the computed value. This acts as an
    ///   constant folding optimization and reduces noise from the transform.
    /// * Casts the resulting expression to a u32 if @p cast_to_u32 is true, and the expression type
    ///   isn't implicitly usable as a u32. This is to help feed the expression into a
    ///   `array<u32, N>` argument passed to a callee variant function.
    const Expression* BuildDynamicIndex(const sem::ValueExpression* idx, bool cast_to_u32) {
        if (auto* val = idx->ConstantValue()) {
            // Expression evaluated to a constant value. Just emit that constant.
            return b.Expr(val->ValueAs<AInt>());
        }

        // Expression is not a constant, clone the expression.
        // Note: If the dynamic index expression was hoisted to a let, then cloning will return an
        // identifier expression to the hoisted let.
        auto* expr = ctx.Clone(idx->Declaration());

        if (cast_to_u32) {
            // The index may be fed to a dynamic index array<u32, N> argument, so the index
            // expression may need casting to u32.
            if (!idx->UnwrapMaterialize()
                     ->Type()
                     ->UnwrapRef()
                     ->IsAnyOf<core::type::U32, core::type::AbstractInt>()) {
                expr = b.Call<u32>(expr);
            }
        }

        return expr;
    }

    /// ProcessFunction scans the direct calls made by the function @p fn, adding new variants to
    /// the callee functions and transforming the call expression to pass dynamic indices instead of
    /// true pointers.
    /// If the function @p fn has pointer parameters that must be transformed to a caller variant,
    /// and the function is not called, then the function is dropped from the output of the
    /// transform, as it cannot be generated.
    /// @note ProcessFunction must be called in dependency order for the program, starting with the
    /// entry points.
    void ProcessFunction(const sem::Function* fn, FnInfo* fn_info) {
        if (fn_info->variants.IsEmpty()) {
            // Function has no variants pre-generated by callers.
            if (MustBeCalled(fn)) {
                // Drop the function, as it wasn't called and cannot be generated.
                ctx.Remove(ctx.src->AST().GlobalDeclarations(), fn->Declaration());
                return;
            }

            // Function was not called. Create a single variant with an empty signature.
            FnVariant variant;
            variant.name = ctx.Clone(fn->Declaration()->name->symbol);
            variant.order = 0;  // Unaltered comes first.
            fn_info->variants.Add(FnVariant::Signature{}, std::move(variant));
        }

        // Process each of the direct calls made by this function.
        for (auto* call : fn->DirectCalls()) {
            ProcessCall(fn_info, call);
        }
    }

    /// ProcessCall creates new variants of the callee function by permuting the call for each of
    /// the variants of @p caller. ProcessCall also registers the clone callback to transform the
    /// call expression to pass dynamic indices instead of true pointers.
    void ProcessCall(FnInfo* caller, const sem::Call* call) {
        auto* target = call->Target()->As<sem::Function>();
        if (!target) {
            // Call target is not a user-declared function.
            return;  // Not interested in this call.
        }

        if (!HasPointerParameter(target)) {
            return;  // Not interested in this call.
        }

        bool call_needs_transforming = false;

        // Build the call target function variant for each variant of the caller.
        for (auto caller_variant_it : caller->SortedVariants()) {
            auto& caller_signature = *caller_variant_it.first;
            auto& caller_variant = *caller_variant_it.second;

            // Build the target variant's signature.
            FnVariant::Signature target_signature;
            for (size_t i = 0; i < call->Arguments().Length(); i++) {
                const auto* arg = call->Arguments()[i];
                const auto* param = target->Parameters()[i];
                const auto* param_ty = param->Type()->As<core::type::Pointer>();
                if (!param_ty) {
                    continue;  // Parameter type is not a pointer.
                }

                // Fetch the access chain for the argument.
                auto* arg_chain = AccessChainFor(arg);
                if (!arg_chain) {
                    continue;  // Argument does not have an access chain
                }

                // Construct the absolute AccessShape by considering the AccessShape of the caller
                // variant's argument. This will propagate back through pointer parameters, to the
                // outermost caller.
                auto absolute = AbsoluteAccessShape(caller_signature, *arg_chain);

                // If the address space of the root variable of the access chain does not require
                // transformation, then there's nothing to do.
                if (!AddressSpaceRequiresTransform(absolute.root.address_space)) {
                    continue;
                }

                // Record that this chain was used in a function call.
                // This preserves the chain during the access chain filtering stage.
                arg_chain->used_in_call = true;

                if (IsPrivateOrFunction(absolute.root.address_space)) {
                    // Pointers in 'private' and 'function' address spaces need to be passed by
                    // pointer argument.
                    absolute.root.variable = param;
                }

                // Add the parameter's absolute AccessShape to the target's signature.
                target_signature.Add(param, std::move(absolute));
            }

            // Construct a new FnVariant if this is the first caller of the target signature
            auto* target_info = FnInfoFor(target);
            auto& target_variant = target_info->variants.GetOrAdd(target_signature, [&] {
                if (target_signature.IsEmpty()) {
                    // Call target does not require any argument changes.
                    FnVariant variant;
                    variant.name = ctx.Clone(target->Declaration()->name->symbol);
                    variant.order = 0;  // Unaltered comes first.
                    return variant;
                }

                // Build an appropriate variant function name.
                // This is derived from the original function name and the pointer parameter
                // chains.
                StringStream ss;
                ss << target->Declaration()->name->symbol.Name();
                for (auto* param : target->Parameters()) {
                    if (auto indices = target_signature.Get(param)) {
                        ss << "_" << AccessShapeName(*indices);
                    }
                }

                // Build the pointer parameter symbols.
                Hashmap<const sem::Parameter*, PtrParamSymbols, 4> ptr_param_symbols;
                for (auto& param_it : target_signature) {
                    auto* param = param_it.key.Value();
                    auto& shape = param_it.value;

                    // Parameter needs replacing with either zero, one or two parameters:
                    // If the parameter is in the 'private' or 'function' address space, then the
                    // originating pointer is always passed down. This always comes first.
                    // If the access chain has dynamic indices, then we create an array<u32, N>
                    // parameter to hold the dynamic indices.
                    bool requires_base_ptr_param = IsPrivateOrFunction(shape.root.address_space);
                    bool requires_indices_param = shape.NumDynamicIndices() > 0;

                    PtrParamSymbols symbols;
                    if (requires_base_ptr_param && requires_indices_param) {
                        auto original_name = param->Declaration()->name->symbol;
                        symbols.base_ptr = UniqueSymbolWithSuffix(original_name, "_base");
                        symbols.indices = UniqueSymbolWithSuffix(original_name, "_indices");
                    } else if (requires_base_ptr_param) {
                        symbols.base_ptr = ctx.Clone(param->Declaration()->name->symbol);
                    } else if (requires_indices_param) {
                        symbols.indices = ctx.Clone(param->Declaration()->name->symbol);
                    }

                    // Remember this base pointer name.
                    ptr_param_symbols.Add(param, symbols);
                }

                // Build the variant.
                FnVariant variant;
                variant.name = b.Symbols().New(ss.str());
                variant.order = target_info->variants.Count() + 1;
                variant.ptr_param_symbols = std::move(ptr_param_symbols);
                return variant;
            });

            // Record the call made by caller variant to the target variant.
            caller_variant.calls.Add(call, target_variant.name);
            if (!target_signature.IsEmpty()) {
                // The call expression will need transforming for at least one caller variant.
                call_needs_transforming = true;
            }
        }

        if (call_needs_transforming) {
            // Register the clone callback to correctly transform the call expression into the
            // appropriate variant calls.
            TransformCall(call);
        }
    }

    /// @returns true if the address space @p address_space requires transforming given the
    /// transform's options.
    bool AddressSpaceRequiresTransform(core::AddressSpace address_space) const {
        switch (address_space) {
            case core::AddressSpace::kUniform:
            case core::AddressSpace::kStorage:
            case core::AddressSpace::kWorkgroup:
                return true;
            case core::AddressSpace::kPrivate:
                return opts.transform_private;
            case core::AddressSpace::kFunction:
                return opts.transform_function;
            default:
                return false;
        }
    }

    /// @returns the AccessChain for the expression @p expr, or nullptr if the expression does
    /// not hold an access chain.
    AccessChain* AccessChainFor(const sem::ValueExpression* expr) const {
        if (auto chain = access_chains.Get(expr)) {
            return *chain;
        }
        return nullptr;
    }

    /// @returns the absolute AccessShape for @p indices, by replacing the originating pointer
    /// parameter with the AccessChain of variant's signature.
    AccessShape AbsoluteAccessShape(const FnVariant::Signature& signature,
                                    const AccessShape& shape) const {
        if (auto* root_param = shape.root.variable->As<sem::Parameter>()) {
            if (auto incoming_chain = signature.Get(root_param)) {
                // Access chain originates from a parameter, which will be transformed into an array
                // of dynamic indices. Concatenate the signature's AccessShape for the parameter
                // to the chain's indices, skipping over the chain's initial parameter index.
                auto absolute = *incoming_chain;
                for (auto& op : shape.ops) {
                    absolute.ops.Push(op);
                }
                return absolute;
            }
        }

        // Chain does not originate from a parameter, so is already absolute.
        return shape;
    }

    /// TransformFunction registers the clone callback to transform the function @p fn into the
    /// (potentially multiple) function's variants. TransformFunction will assign the current
    /// function and variant to #clone_state, which can be used by the other clone callbacks.
    void TransformFunction(const sem::Function* fn, FnInfo* fn_info) {
        // Register a custom handler for the specific function
        ctx.Replace(fn->Declaration(), [this, fn, fn_info] {
            // For the scope of this lambda, assign current_function to fn_info.
            TINT_SCOPED_ASSIGNMENT(clone_state->current_function, fn_info);

            // This callback expects a single function returned. As we're generating potentially
            // many variant functions, keep a record of the last created variant, and explicitly add
            // this to the module if it isn't the last. We'll return the last created variant,
            // taking the place of the original function.
            const Function* pending_variant = nullptr;

            // For each variant of fn...
            for (auto variant_it : fn_info->SortedVariants()) {
                if (pending_variant) {
                    b.AST().AddFunction(pending_variant);
                }

                auto& variant_sig = *variant_it.first;
                auto& variant = *variant_it.second;

                // For the rest of this scope, assign the current variant and variant signature.
                TINT_SCOPED_ASSIGNMENT(clone_state->current_variant_sig, &variant_sig);
                TINT_SCOPED_ASSIGNMENT(clone_state->current_variant, &variant);

                // Build the variant's parameters.
                // Pointer parameters in the 'uniform', 'storage' or 'workgroup' address space are
                // either replaced with an array of dynamic indices, or are dropped (if there are no
                // dynamic indices).
                tint::Vector<const Parameter*, 8> params;
                for (auto* param : fn->Parameters()) {
                    if (auto incoming_shape = variant_sig.Get(param)) {
                        auto& symbols = *variant.ptr_param_symbols.Get(param);
                        if (symbols.base_ptr.IsValid()) {
                            auto base_ptr_ty = b.ty.ptr(
                                incoming_shape->root.address_space,
                                CreateASTTypeFor(ctx, incoming_shape->root.type->UnwrapPtrOrRef()));
                            params.Push(b.Param(symbols.base_ptr, base_ptr_ty));
                        }
                        if (symbols.indices.IsValid()) {
                            // Variant has dynamic indices for this variant, replace it.
                            auto dyn_idx_arr_type = DynamicIndexArrayType(*incoming_shape);
                            params.Push(b.Param(symbols.indices, dyn_idx_arr_type));
                        }
                    } else {
                        // Just a regular parameter. Just clone the original parameter.
                        params.Push(ctx.Clone(param->Declaration()));
                    }
                }

                // Build the variant by cloning the source function. The other clone callbacks will
                // use clone_state->current_variant and clone_state->current_variant_sig to produce
                // the variant.
                auto ret_ty = ctx.Clone(fn->Declaration()->return_type);
                auto body = ctx.Clone(fn->Declaration()->body);
                auto attrs = ctx.Clone(fn->Declaration()->attributes);
                auto ret_attrs = ctx.Clone(fn->Declaration()->return_type_attributes);
                pending_variant =
                    b.create<Function>(b.Ident(variant.name), std::move(params), ret_ty, body,
                                       std::move(attrs), std::move(ret_attrs));
            }

            return pending_variant;
        });
    }

    /// TransformCall registers the clone callback to transform the call expression @p call to call
    /// the correct target variant, and to replace pointers arguments with an array of dynamic
    /// indices.
    void TransformCall(const sem::Call* call) {
        // Register a custom handler for the specific call expression
        ctx.Replace(call->Declaration(), [this, call] {
            auto target_variant = clone_state->current_variant->calls.Get(call);
            if (!target_variant) {
                // The current variant does not need to transform this call.
                return ctx.CloneWithoutTransform(call->Declaration());
            }

            // Build the new call expressions's arguments.
            tint::Vector<const Expression*, 8> new_args;
            for (size_t arg_idx = 0; arg_idx < call->Arguments().Length(); arg_idx++) {
                auto* arg = call->Arguments()[arg_idx];
                auto* param = call->Target()->Parameters()[arg_idx];
                auto* param_ty = param->Type()->As<core::type::Pointer>();
                if (!param_ty) {
                    // Parameter is not a pointer.
                    // Just clone the unaltered argument.
                    new_args.Push(ctx.Clone(arg->Declaration()));
                    continue;  // Parameter is not a pointer
                }

                auto* chain = AccessChainFor(arg);
                if (!chain) {
                    // No access chain means the argument is not a pointer that needs transforming.
                    // Just clone the unaltered argument.
                    new_args.Push(ctx.Clone(arg->Declaration()));
                    continue;
                }

                // Construct the absolute AccessShape by considering the AccessShape of the caller
                // variant's argument. This will propagate back through pointer parameters, to the
                // outermost caller.
                auto full_indices = AbsoluteAccessShape(*clone_state->current_variant_sig, *chain);

                // If the parameter is a pointer in the 'private' or 'function' address space, then
                // we need to pass an additional pointer argument to the base object.
                if (IsPrivateOrFunction(param_ty->AddressSpace())) {
                    auto* root_expr = BuildAccessRootExpr(chain->root, /* deref */ false);
                    if (!chain->root.variable->Is<sem::Parameter>()) {
                        root_expr = b.AddressOf(root_expr);
                    }
                    new_args.Push(root_expr);
                }

                // Get or create the dynamic indices array.
                if (auto dyn_idx_arr_ty = DynamicIndexArrayType(full_indices)) {
                    // Build an array of dynamic indices to pass as the replacement for the pointer.
                    tint::Vector<const Expression*, 8> dyn_idx_args;
                    if (auto* root_param = chain->root.variable->As<sem::Parameter>()) {
                        // Access chain originates from a pointer parameter.
                        if (auto incoming_chain =
                                clone_state->current_variant_sig->Get(root_param)) {
                            auto indices =
                                clone_state->current_variant->ptr_param_symbols.Get(root_param)
                                    ->indices;

                            // This pointer parameter will have been replaced with a array<u32, N>
                            // holding the variant's dynamic indices for the pointer. Unpack these
                            // directly into the array constructor's arguments.
                            auto N = incoming_chain->NumDynamicIndices();
                            for (uint32_t i = 0; i < N; i++) {
                                dyn_idx_args.Push(b.IndexAccessor(indices, u32(i)));
                            }
                        }
                    }
                    // Pass the dynamic indices of the access chain into the array constructor.
                    for (auto& dyn_idx : chain->dynamic_indices) {
                        dyn_idx_args.Push(BuildDynamicIndex(dyn_idx, /* cast_to_u32 */ true));
                    }
                    // Construct the dynamic index array, and push as an argument.
                    new_args.Push(b.Call(dyn_idx_arr_ty, std::move(dyn_idx_args)));
                }
            }

            // Make the call to the target's variant.
            return b.Call(*target_variant, std::move(new_args));
        });
    }

    /// ProcessAccessChains performs the following:
    /// * Removes all AccessChains from expressions that are not either used as a pointer argument
    ///   in a call, or originates from a pointer parameter.
    /// * Hoists the dynamic index expressions of AccessChains to 'let' statements, to prevent
    ///   multiple evaluation of the expressions, and avoid expressions resolving to different
    ///   variables based on lexical scope.
    void ProcessAccessChains() {
        auto chain_exprs = access_chains.Keys();
        chain_exprs.Sort([](const auto& expr_a, const auto& expr_b) {
            return expr_a->Declaration()->node_id.value < expr_b->Declaration()->node_id.value;
        });

        for (auto* expr : chain_exprs) {
            auto* chain = *access_chains.Get(expr);
            if (!chain->used_in_call && !chain->root.variable->Is<sem::Parameter>()) {
                // Chain was not used in a function call, and does not originate from a
                // parameter. This chain does not need transforming. Drop it.
                access_chains.Remove(expr);
                continue;
            }

            // Chain requires transforming.

            // We need to be careful that the chain does not use expressions with side-effects which
            // cannot be repeatedly evaluated. In this situation we can hoist the dynamic index
            // expressions to their own uniquely named lets (if required).
            MaybeHoistDynamicIndices(chain, expr->Stmt());
        }
    }

    /// TransformAccessChainExpressions registers the clone callback to:
    /// * Transform all expressions that have an AccessChain (which aren't arguments to function
    ///   calls, these are handled by TransformCall()), into the equivalent expression using a
    ///   module-scope variable.
    /// * Replace expressions that have been hoisted to a let, with an identifier expression to that
    ///   let.
    void TransformAccessChainExpressions() {
        // Register a custom handler for all non-function call expressions
        ctx.ReplaceAll([this](const Expression* ast_expr) -> const Expression* {
            if (!clone_state->current_variant) {
                // Expression does not belong to a function variant.
                return nullptr;  // Just clone the expression.
            }

            auto* expr = sem.GetVal(ast_expr);
            if (!expr) {
                // No semantic node for the expression.
                return nullptr;  // Just clone the expression.
            }

            // If the expression has been hoisted to a 'let', then replace the expression with an
            // identifier to the hoisted let.
            if (auto hoisted = clone_state->current_function->hoisted_exprs.Get(expr)) {
                return b.Expr(*hoisted);
            }

            auto* chain = AccessChainFor(expr);
            if (!chain) {
                // The expression does not have an AccessChain.
                return nullptr;  // Just clone the expression.
            }

            auto* root_param = chain->root.variable->As<sem::Parameter>();
            if (!root_param) {
                // The expression has an access chain, but does not originate with a pointer
                // parameter. We don't need to change anything here.
                return nullptr;  // Just clone the expression.
            }

            auto incoming_shape = clone_state->current_variant_sig->Get(root_param);
            if (!incoming_shape) {
                // The root parameter of the access chain is not part of the variant's signature.
                return nullptr;  // Just clone the expression.
            }

            // Expression holds an access chain to a pointer parameter that needs transforming.
            // Reconstruct the expression using the variant's incoming shape.

            auto* chain_expr = BuildAccessRootExpr(incoming_shape->root, /* deref */ true);

            // Chain starts with a pointer parameter.
            // Replace this with the variant's incoming shape. This will bring the expression up to
            // the incoming pointer.
            size_t next_dyn_idx_from_indices = 0;
            auto& indices =
                clone_state->current_variant->ptr_param_symbols.Get(root_param)->indices;
            for (auto param_access : incoming_shape->ops) {
                chain_expr = BuildAccessExpr(chain_expr, param_access, [&] {
                    return b.IndexAccessor(indices, AInt(next_dyn_idx_from_indices++));
                });
            }

            // Now build the expression chain within the function.

            // For each access in the chain (excluding the pointer parameter)...
            size_t next_dyn_idx_from_chain = 0;
            for (auto& op : chain->ops) {
                chain_expr = BuildAccessExpr(chain_expr, op, [&] {
                    return BuildDynamicIndex(chain->dynamic_indices[next_dyn_idx_from_chain++],
                                             false);
                });
            }

            // BuildAccessExpr() always returns a non-pointer.
            // If the expression we're replacing is a pointer, take the address.
            if (expr->Type()->Is<core::type::Pointer>()) {
                chain_expr = b.AddressOf(chain_expr);
            }

            return chain_expr;
        });
    }

    /// @returns the FnInfo for the given function, constructing a new FnInfo if @p fn doesn't
    /// already have one.
    FnInfo* FnInfoFor(const sem::Function* fn) {
        return fns.GetOrAdd(fn, [this] { return fn_info_allocator.Create(); });
    }

    /// @returns the type alias used to hold the dynamic indices for @p shape, declaring a new alias
    /// if this is the first call for the given shape.
    Type DynamicIndexArrayType(const AccessShape& shape) {
        auto name = dynamic_index_array_aliases.GetOrAdd(shape, [&] {
            // Count the number of dynamic indices
            uint32_t num_dyn_indices = shape.NumDynamicIndices();
            if (num_dyn_indices == 0) {
                return Symbol{};
            }
            auto symbol = b.Symbols().New(AccessShapeName(shape));
            b.Alias(symbol, b.ty.array(b.ty.u32(), u32(num_dyn_indices)));
            return symbol;
        });
        return name.IsValid() ? b.ty(name) : Type{};
    }

    /// @returns a name describing the given shape
    std::string AccessShapeName(const AccessShape& shape) {
        StringStream ss;

        if (IsPrivateOrFunction(shape.root.address_space)) {
            ss << "F";
        } else {
            ss << shape.root.variable->Declaration()->name->symbol.Name();
        }

        for (auto& op : shape.ops) {
            ss << "_";

            if (std::holds_alternative<DynamicIndex>(op)) {
                /// The op uses a dynamic (runtime-expression) index.
                ss << "X";
                continue;
            }

            auto* member = std::get_if<Symbol>(&op);
            if (DAWN_LIKELY(member)) {
                ss << member->Name();
                continue;
            }

            TINT_ICE() << "unhandled variant for access chain";
        }
        return ss.str();
    }

    /// Builds an expresion to the root of an access, returning the new expression.
    /// @param root the AccessRoot
    /// @param deref if true, the returned expression will always be a reference type.
    const Expression* BuildAccessRootExpr(const AccessRoot& root, bool deref) {
        if (auto* param = root.variable->As<sem::Parameter>()) {
            if (auto symbols = clone_state->current_variant->ptr_param_symbols.Get(param)) {
                if (deref) {
                    return b.Deref(b.Expr(symbols->base_ptr));
                }
                return b.Expr(symbols->base_ptr);
            }
        }

        const Expression* expr = b.Expr(ctx.Clone(root.variable->Declaration()->name->symbol));
        if (deref) {
            if (root.variable->Type()->Is<core::type::Pointer>()) {
                expr = b.Deref(expr);
            }
        }
        return expr;
    }

    /// Builds a single access in an access chain, returning the new expression.
    /// The returned expression will always be of a reference type.
    /// @param expr the input expression
    /// @param access the access to perform on the current expression
    /// @param dynamic_index a function that obtains the next dynamic index
    const Expression* BuildAccessExpr(const Expression* expr,
                                      const AccessOp& access,
                                      std::function<const Expression*()> dynamic_index) {
        if (std::holds_alternative<DynamicIndex>(access)) {
            /// The access uses a dynamic (runtime-expression) index.
            auto* idx = dynamic_index();
            return b.IndexAccessor(expr, idx);
        }

        auto* member = std::get_if<Symbol>(&access);
        if (DAWN_LIKELY(member)) {
            /// The access is a member access.
            return b.MemberAccessor(expr, ctx.Clone(*member));
        }

        TINT_ICE() << "unhandled variant type for access chain";
    }

    /// @returns a new Symbol starting with @p symbol concatenated with @p suffix, and possibly an
    /// underscore and number, if the symbol is already taken.
    Symbol UniqueSymbolWithSuffix(Symbol symbol, const std::string& suffix) {
        auto str = symbol.Name() + suffix;
        return unique_symbols.GetOrAdd(str, [&] { return b.Symbols().New(str); });
    }

    /// @returns true if the function @p fn has at least one pointer parameter.
    static bool HasPointerParameter(const sem::Function* fn) {
        for (auto* param : fn->Parameters()) {
            if (param->Type()->Is<core::type::Pointer>()) {
                return true;
            }
        }
        return false;
    }

    /// @returns true if the function @p fn has at least one pointer parameter in an address space
    /// that must be replaced. If this function is not called, then the function cannot be sensibly
    /// generated, and must be stripped.
    static bool MustBeCalled(const sem::Function* fn) {
        for (auto* param : fn->Parameters()) {
            if (auto* ptr = param->Type()->As<core::type::Pointer>()) {
                switch (ptr->AddressSpace()) {
                    case core::AddressSpace::kUniform:
                    case core::AddressSpace::kStorage:
                    case core::AddressSpace::kWorkgroup:
                        return true;
                    default:
                        return false;
                }
            }
        }
        return false;
    }

    /// @returns true if the given address space is 'private' or 'function'.
    static bool IsPrivateOrFunction(const core::AddressSpace sc) {
        return sc == core::AddressSpace::kPrivate || sc == core::AddressSpace::kFunction;
    }
};

DirectVariableAccess::Config::Config() = default;
DirectVariableAccess::Config::Config(const Options& opt) : options(opt) {}

DirectVariableAccess::Config::~Config() = default;

DirectVariableAccess::DirectVariableAccess() = default;

DirectVariableAccess::~DirectVariableAccess() = default;

Transform::ApplyResult DirectVariableAccess::Apply(const Program& program,
                                                   const DataMap& inputs,
                                                   DataMap&) const {
    Options options;
    if (auto* cfg = inputs.Get<Config>()) {
        options = cfg->options;
    }
    return State(program, options).Run();
}

}  // namespace tint::ast::transform
