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

#include "src/tint/lang/core/ir/transform/direct_variable_access.h"

#include <string>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/utils/containers/reverse.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {
namespace {

/// An access root, originating from a module-scope var.
/// These roots are not passed by parameter, but instead the callee references the module-scope var
/// directly.
struct RootModuleScopeVar {
    /// The module-scope var
    Var* var = nullptr;

    /// @return a hash value for this object
    tint::HashCode HashCode() const { return Hash(var); }

    /// Inequality operator
    bool operator!=(const RootModuleScopeVar& other) const { return var != other.var; }
};

/// An access root, originating from a pointer parameter or function-scope var.
/// These roots are passed by pointer parameter.
struct RootPtrParameter {
    /// The parameter pointer type
    const type::Pointer* type = nullptr;

    /// @return a hash value for this object
    tint::HashCode HashCode() const { return Hash(type); }

    /// Inequality operator
    bool operator!=(const RootPtrParameter& other) const { return type != other.type; }
};

/// An access root. Either a RootModuleScopeVar or RootPtrParameter.
using AccessRoot = std::variant<RootModuleScopeVar, RootPtrParameter>;

/// MemberAccess is an access operator to a struct member.
struct MemberAccess {
    /// The member being accessed
    const type::StructMember* member;

    /// @return a hash member for this object
    tint::HashCode HashCode() const { return Hash(member); }

    /// Inequality operator
    bool operator!=(const MemberAccess& other) const { return member != other.member; }
};

/// IndexAccess is an access operator to an array element or matrix column.
/// The ordered list of indices is passed by parameter.
struct IndexAccess {
    /// @return a hash value for this object
    tint::HashCode HashCode() const { return 42; }

    /// Inequality operator
    bool operator!=(const IndexAccess&) const { return false; }
};

/// An access operation. Either a MemberAccess or IndexAccess.
using AccessOp = std::variant<MemberAccess, IndexAccess>;

/// A AccessShape describes the static "path" from a root variable to an element within the
/// variable.
///
/// Functions that have parameters which need transforming will be forked into one or more
/// 'variants'. Each variant has different AccessShapes for the parameters - the transform will only
/// emit one variant when the shapes of the parameter accesses match.
///
/// Array accessors index expressions are held externally to the AccessShape, so
/// AccessShape will be considered equal even if the array or matrix index values differ.
///
/// For example, consider the following:
///
/// ```
/// struct A {
///     x : array<i32, 8>,
///     y : u32,
/// };
/// struct B {
///     x : i32,
///     y : array<A, 4>
/// };
/// var<workgroup> C : B;
/// ```
///
/// The following AccessShape would describe the following:
///
/// +====================================+===============+=================================+
/// | AccessShape                        | Type          |  Expression                     |
/// +====================================+===============+=================================+
/// | [ Var 'C', MemberAccess 'x' ]      | i32           |  C.x                            |
/// +------------------------------------+---------------+---------------------------------+
/// | [ Var 'C', MemberAccess 'y' ]      | array<A, 4>   |  C.y                            |
/// +------------------------------------+---------------+---------------------------------+
/// | [ Var 'C', MemberAccess 'y',       | A             |  C.y[indices[0]]                |
/// |   IndexAccess ]                    |               |                                 |
/// +------------------------------------+---------------+---------------------------------+
/// | [ Var 'C', MemberAccess 'y',       | array<i32, 8> |  C.y[indices[0]].x              |
/// |   IndexAccess, MemberAccess 'x' ]  |               |                                 |
/// +------------------------------------+---------------+---------------------------------+
/// | [ Var 'C', MemberAccess 'y',       | i32           |  C.y[indices[0]].x[indices[1]]  |
/// |   IndexAccess, MemberAccess 'x',   |               |                                 |
/// |   IndexAccess ]                    |               |                                 |
/// +------------------------------------+---------------+---------------------------------+
/// | [ Var 'C', MemberAccess 'y',       | u32           |  C.y[indices[0]].y              |
/// |   IndexAccess, MemberAccess 'y' ]  |               |                                 |
/// +------------------------------------+---------------+---------------------------------+
///
/// Where: `indices` is the AccessChain::indices.
struct AccessShape {
    /// The access root.
    AccessRoot root;
    /// The access operations.
    Vector<AccessOp, 8> ops;

    /// @returns the number of IndexAccess operations in #ops.
    uint32_t NumIndexAccesses() const {
        uint32_t count = 0;
        for (auto& op : ops) {
            if (std::holds_alternative<IndexAccess>(op)) {
                count++;
            }
        }
        return count;
    }

    /// @return a hash value for this object
    tint::HashCode HashCode() const { return Hash(root, ops); }

    /// Inequality operator
    bool operator!=(const AccessShape& other) const {
        return root != other.root || ops != other.ops;
    }
};

/// AccessChain describes a chain of access expressions originating from a variable.
struct AccessChain {
    /// The shape of the access chain
    AccessShape shape;
    /// The originating pointer
    Value* root_ptr = nullptr;
    /// The array of dynamic indices
    Vector<Value*, 8> indices;
};

/// A variant signature describes the access shape of all the function's pointer parameters.
/// This is a map of pointer parameter index to access shape.
using VariantSignature = Hashmap<size_t, AccessShape, 4>;

/// FnInfo describes a function that has pointer parameters which need replacing.
/// This function will be replaced by zero, one or many variants. Each variant will have a unique
/// access shape for the function's the pointer arguments.
struct FnInfo {
    /// A map of variant signature to the variant's unique IR function.
    Hashmap<VariantSignature, Function*, 4> variants_by_sig;
    /// The order to emit the variants in the final module.
    Vector<Function*, 4> ordered_variants;
};

/// FnVariant describes a unique variant of a function that has pointer parameters that need
/// replacing.
struct FnVariant {
    /// The signature of the variant.
    VariantSignature signature;
    /// The IR function for this variant.
    Function* fn = nullptr;
    /// The function information of the original function that this variant is based off.
    FnInfo* info = nullptr;
};

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// The transform options
    const DirectVariableAccessOptions& options;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The symbol table.
    SymbolTable& sym{ir.symbols};

    /// The functions that have pointer parameters that need transforming.
    /// These functions will be replaced with zero, one or many forked variants.
    Hashmap<ir::Function*, FnInfo*, 8> need_forking{};

    /// Queue of variants that need building
    Vector<FnVariant, 8> variants_to_build{};

    /// Allocator for FnInfo
    BlockAllocator<FnInfo> fn_info_allocator{};

    /// Process the module.
    void Process() {
        // Make a copy of all the functions in the IR module.
        // Use transform to convert from ConstPropagatingPtr<Function> to Function*
        auto input_fns = Transform<8>(ir.functions.Slice(), [](auto& fn) { return fn.Get(); });

        // Populate #need_forking
        GatherFnsThatNeedForking();

        // Transform the functions that make calls to #need_forking, which aren't in #need_forking
        // themselves.
        BuildRootFns();

        // Build variants of the functions in #need_forking.
        BuildFnVariants();

        // Rebuild ir.functions.
        EmitFunctions(input_fns);
    }

    /// Populates #need_forking with all the functions that have pointer parameters which need
    /// transforming. These functions will be replaced with variants based on the access shapes.
    void GatherFnsThatNeedForking() {
        for (auto& fn : ir.functions) {
            if (!fn->Alive()) {
                continue;
            }
            for (auto* param : fn->Params()) {
                if (!NeedsTransforming(param)) {
                    continue;
                }
                need_forking.Add(fn, fn_info_allocator.Create());
                break;
            }
        }
    }

    /// Adjusts the calls of all the functions that make calls to #need_forking, which aren't in
    /// #need_forking themselves. This populates #variants_to_build with the called functions.
    void BuildRootFns() {
        for (auto& fn : ir.functions) {
            if (!need_forking.Contains(fn)) {
                TransformCalls(fn);
            }
        }
    }

    /// Applies the necessary transformations to all the pointer arguments of calls made to
    /// functions in #need_forking. Also populates #variants_to_build with the variants of the
    /// callee functions.
    /// @param fn the function to transform
    void TransformCalls(Function* fn) {
        // For all the function calls in the function...
        Traverse(fn->Block(), [&](UserCall* call) {
            auto* target = call->Target();
            auto target_info = need_forking.Get(target);
            if (!target_info) {
                // Not a call to a function in #need_forking. Leave alone.
                return;
            }

            // Found a call to a function in #need_forking.
            // This call needs transforming to call the generated variant.

            // New arguments to the call. This includes transformed and untransformed arguments.
            Vector<Value*, 8> new_args;

            // Pointer arguments that are being replaced.
            Vector<Value*, 8> replaced_args;

            // Signature of the callee variant
            VariantSignature signature;

            // For each argument / parameter...
            for (size_t i = 0, n = call->Args().Length(); i < n; i++) {
                auto* arg = call->Args()[i];
                auto* param = target->Params()[i];

                // Argument does not need transformation, push the existing argument to new_args.
                if (!NeedsTransforming(param)) {
                    new_args.Push(arg);
                    continue;
                }

                // This argument needs replacing with:
                // * Nothing: root is a module-scope var and the access chain has no indices.
                // * A single pointer argument to the root variable: The root is a pointer
                //   parameter or a function-scoped variable, and the access chain has no
                //   indices.
                // * A single indices array argument: The root is a module-scope var and the
                //   access chain has indices.
                // * Both a pointer argument and indices array argument: The root is a pointer
                //   parameter or a function-scoped variable and the access chain has indices.
                b.InsertBefore(call, [&] {
                    // Get the access chain for the pointer argument.
                    auto chain = AccessChainFor(arg);
                    // If the root is not a module-scope variable, then pass this root pointer
                    // as an argument.
                    if (std::holds_alternative<RootPtrParameter>(chain.shape.root)) {
                        new_args.Push(chain.root_ptr);
                    }
                    // If the chain access contains indices, then pass these as an array of u32.
                    if (size_t array_len = chain.indices.Length(); array_len > 0) {
                        auto* array = ty.array(ty.u32(), static_cast<uint32_t>(array_len));
                        auto* indices = b.Construct(array, std::move(chain.indices));
                        new_args.Push(indices->Result());
                    }
                    // Record the parameter shape for the variant's signature.
                    signature.Add(i, chain.shape);
                });

                // Record that this pointer argument has been replaced.
                replaced_args.Push(arg);
            }

            // Replace the call's arguments with new_args.
            call->SetArgs(std::move(new_args));

            // Clean up instructions that provided the now unused argument values.
            for (auto* old_arg : replaced_args) {
                DeleteDeadInstructions(old_arg);
            }

            // Look to see if this callee signature already has a variant created.
            auto* new_target = (*target_info)->variants_by_sig.GetOrAdd(signature, [&] {
                // New signature.

                // Clone the original function to seed the new variant.
                auto* variant_fn = CloneContext{ir}.Clone(target);
                (*target_info)->ordered_variants.Push(variant_fn);

                // Copy the original name for the variant
                if (auto fn_name = ir.NameOf(fn)) {
                    ir.SetName(fn, fn_name);
                }

                // Create an entry for the variant, and add it to the queue of variants that need to
                // be built. We don't do this here to avoid unbounded stack usage.
                variants_to_build.Push(FnVariant{/* signature */ signature,
                                                 /* fn */ variant_fn,
                                                 /* info */ *target_info});
                return variant_fn;
            });

            // Re-point the target of the call to the variant.
            call->SetTarget(new_target);
        });
    }

    /// Builds all the variants in #variants_to_build by:
    /// * Replacing the pointer parameters with zero, one or two parameters (root pointer, indices).
    /// * Transforming any calls made by that variant to other functions found in #need_forking.
    /// Note: The transformation of calls can add more variants to #variants_to_build.
    /// BuildFnVariants() will continue to build variants until #variants_to_build is empty.
    void BuildFnVariants() {
        while (!variants_to_build.IsEmpty()) {
            auto variant = variants_to_build.Pop();
            BuildFnVariantParams(variant);
            TransformCalls(variant.fn);
        }
    }

    /// Walks the instructions that built #value to obtain the root variable and the pointer
    /// accesses.
    /// @param value the pointer value to get the access chain for
    /// @return an AccessChain
    AccessChain AccessChainFor(Value* value) {
        AccessChain chain;
        while (value) {
            TINT_ASSERT(value->Alive());
            value = tint::Switch(
                value,  //
                [&](InstructionResult* res) {
                    // value was emitted by an instruction
                    auto* inst = res->Instruction();
                    return tint::Switch(
                        inst,
                        [&](Access* access) {
                            // The AccessOp of this access instruction
                            Vector<AccessOp, 8> ops;
                            // The ordered, non-member accesses performed by this access instruction
                            Vector<Value*, 8> indices;
                            // The pointee-type that each access is being performed on
                            auto* obj_ty = access->Object()->Type()->UnwrapPtr();

                            // For each access operation...
                            for (auto idx : access->Indices()) {
                                if (auto* str = obj_ty->As<type::Struct>()) {
                                    // Struct type accesses must be constant, representing the index
                                    // of the member being accessed.
                                    TINT_ASSERT(idx->Is<Constant>());
                                    auto i = idx->As<Constant>()->Value()->ValueAs<uint32_t>();
                                    auto* member = str->Members()[i];
                                    ops.Push(MemberAccess{member});
                                    obj_ty = member->Type();
                                    continue;
                                }

                                // Array or matrix access.
                                // Convert index to u32 if it isn't already.
                                if (!idx->Type()->Is<type::U32>()) {
                                    idx = b.Convert(ty.u32(), idx)->Result();
                                }

                                ops.Push(IndexAccess{});
                                indices.Push(idx);
                                obj_ty = obj_ty->Elements().type;
                            }

                            // Push the ops and indices in reverse order to the chain. This is done
                            // so we can continue to walk the IR values and push accesses (without
                            // insertion) that bring us closer to the pointer root. These are
                            // reversed again once the root variable is found.
                            for (auto& op : Reverse(ops)) {
                                chain.shape.ops.Push(op);
                            }
                            for (auto& idx : Reverse(indices)) {
                                chain.indices.Push(idx);
                            }

                            TINT_ASSERT(obj_ty == access->Result()->Type()->UnwrapPtr());
                            return access->Object();
                        },
                        [&](Load* load) { return load->From(); },  //
                        [&](Var* var) {
                            // A 'var' is a pointer root.
                            if (var->Block() == ir.root_block) {
                                // Root pointer is a module-scope 'var'
                                chain.shape.root = RootModuleScopeVar{var};
                            } else {
                                // Root pointer is a function-scope 'var'
                                chain.shape.root =
                                    RootPtrParameter{var->Result()->Type()->As<type::Pointer>()};
                            }
                            chain.root_ptr = var->Result();
                            return nullptr;
                        },
                        [&](Let* let) { return let->Value(); },  //
                        TINT_ICE_ON_NO_MATCH);
                },
                [&](FunctionParam* param) {
                    // Root pointer is a parameter of the caller
                    chain.shape.root = RootPtrParameter{param->Type()->As<type::Pointer>()};
                    chain.root_ptr = param;
                    return nullptr;
                },  //
                TINT_ICE_ON_NO_MATCH);
        }

        // Reverse the chain's ops and indices. See above for why.
        chain.shape.ops.Reverse();
        chain.indices.Reverse();

        return chain;
    }

    /// Replaces the pointer parameters that need transforming of the variant function @p variant.
    /// Instructions are inserted at the top of the @p variant function block to reconstruct the
    /// pointer parameters from the access chain using the root pointer and access ops.
    /// @param variant the variant function to transform
    void BuildFnVariantParams(const FnVariant& variant) {
        // Insert new instructions at the top of the function block...
        b.InsertBefore(variant.fn->Block()->Front(), [&] {
            // The replacement parameters for the variant function
            Vector<ir::FunctionParam*, 8> new_params;
            const auto& old_params = variant.fn->Params();
            // For each parameter in the original function...
            for (size_t param_idx = 0; param_idx < old_params.Length(); param_idx++) {
                auto* old_param = old_params[param_idx];

                // Parameter does not need transforming.
                if (!NeedsTransforming(old_param)) {
                    new_params.Push(old_param);
                    continue;
                }

                // Pointer parameter that needs transforming
                // Grab the access shape of the pointer parameter from the signature
                auto shape = variant.signature.Get(param_idx);
                // The pointer value for the root of the chain.
                Value* root_ptr = nullptr;

                // Build the root pointer parameter, if required.
                FunctionParam* root_ptr_param = nullptr;
                if (auto* ptr_param = std::get_if<RootPtrParameter>(&shape->root)) {
                    // Root pointer is passed as a parameter
                    root_ptr_param = b.FunctionParam(ptr_param->type);
                    new_params.Push(root_ptr_param);
                    root_ptr = root_ptr_param;
                } else if (auto* global = std::get_if<RootModuleScopeVar>(&shape->root)) {
                    // Root pointer is a module-scope var
                    root_ptr = global->var->Result();
                } else {
                    TINT_ICE() << "unhandled AccessShape root variant";
                }

                // Build the access indices parameter, if required.
                ir::FunctionParam* indices_param = nullptr;
                if (uint32_t n = shape->NumIndexAccesses(); n > 0) {
                    // Indices are passed as an array of u32
                    indices_param = b.FunctionParam(ty.array(ty.u32(), n));
                    new_params.Push(indices_param);
                }

                // Generate names for the new parameter(s) based on the replaced parameter name.
                if (auto param_name = ir.NameOf(old_param); param_name.IsValid()) {
                    // Propagate old parameter name to the new parameters
                    if (root_ptr_param) {
                        ir.SetName(root_ptr_param, param_name.Name() + "_root");
                    }
                    if (indices_param) {
                        ir.SetName(indices_param, param_name.Name() + "_indices");
                    }
                }

                // Use the newly added parameters to recompute the equivalent of old_param.
                auto* replacement = root_ptr;

                // Emit the access chain if needed.
                if (!shape->ops.IsEmpty()) {
                    // Handle types are passed by value, turn them into a pointer type for the
                    // access chain call.
                    auto* access_type = old_param->Type();
                    if (!access_type->Is<type::Pointer>()) {
                        TINT_ASSERT(access_type->IsHandle());
                        access_type = ty.ptr<handle>(access_type);
                    }

                    // Rebuild the pointer from the root pointer and accesses.
                    uint32_t index_index = 0;
                    auto chain = Transform(shape->ops, [&](const AccessOp& op) -> Value* {
                        if (auto* m = std::get_if<MemberAccess>(&op)) {
                            return b.Constant(u32(m->member->Index()));
                        }
                        auto* access = b.Access(ty.u32(), indices_param, u32(index_index++));
                        return access->Result();
                    });

                    replacement = b.Access(access_type, root_ptr, std::move(chain))->Result();
                }

                // Replaced handles need the final load after the access chain.
                if (!old_param->Type()->Is<type::Pointer>()) {
                    replacement = b.Load(replacement)->Result();
                }

                // Replace the now removed parameter value with the access instruction.
                old_param->ReplaceAllUsesWith(replacement);
                old_param->Destroy();
            }

            // Replace the function's parameters
            variant.fn->SetParams(std::move(new_params));
        });
    }

    /// Repopulates #ir.functions with the functions in #need_forking replaced with their generated
    /// variants.
    /// @param input_fns the content of #ir.functions before transformation began.
    void EmitFunctions(VectorRef<Function*> input_fns) {
        ir.functions.Clear();
        for (auto& fn : input_fns) {
            if (auto info = need_forking.Get(fn)) {
                fn->Destroy();
                for (auto variant : (*info)->ordered_variants) {
                    ir.functions.Push(variant);
                }
            } else {
                ir.functions.Push(fn);
            }
        }
    }

    /// @return true if @p param is a parameter that requires transforming, based on the
    /// transform options.
    /// @param param the function parameter
    bool NeedsTransforming(FunctionParam* param) const {
        auto* param_type = param->Type();

        if (auto* ptr = param_type->As<type::Pointer>()) {
            // DVA needs to be updated if handles start to be passed by pointer.
            TINT_ASSERT(ptr->AddressSpace() != core::AddressSpace::kHandle);
            switch (ptr->AddressSpace()) {
                case core::AddressSpace::kStorage:
                case core::AddressSpace::kUniform:
                case core::AddressSpace::kWorkgroup:
                    return true;
                case core::AddressSpace::kFunction:
                    return options.transform_function;
                case core::AddressSpace::kPrivate:
                    return options.transform_private;
                default:
                    break;
            }
        }

        if (param_type->IsHandle()) {
            return options.transform_handle;
        }

        return false;
    }

    /// Walks the instructions that built @p value, deleting those that are no longer used.
    /// @param value the pointer value that was used as a now replaced pointer argument.
    void DeleteDeadInstructions(ir::Value* value) {
        // While value has no uses...
        while (value && !value->IsUsed()) {
            auto* inst_res = value->As<InstructionResult>();
            if (!inst_res) {
                return;  // Only instructions can be removed.
            }
            value = tint::Switch(
                inst_res->Instruction(),  //
                [&](Access* access) {
                    TINT_DEFER(access->Destroy());
                    return access->Object();
                },
                [&](Let* let) {
                    TINT_DEFER(let->Destroy());
                    return let->Value();
                },
                [&](Load* load) {
                    if (options.transform_handle) {
                        TINT_DEFER(load->Destroy());
                        return load->From();
                    }
                    return load->From();
                });
        }
    }
};

}  // namespace

Result<SuccessType> DirectVariableAccess(Module& ir, const DirectVariableAccessOptions& options) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "core.DirectVariableAccess", kDirectVariableAccessCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir, options}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
