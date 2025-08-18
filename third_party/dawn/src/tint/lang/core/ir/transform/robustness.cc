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

#include "src/tint/lang/core/ir/transform/robustness.h"

#include <algorithm>
#include <optional>

#include "src/tint/lang/core/ir/analysis/integer_range_analysis.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The robustness config.
    const RobustnessConfig& config;

    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// For integer range analysis when needed
    std::optional<ir::analysis::IntegerRangeAnalysis> integer_range_analysis = {};

    /// Process the module.
    void Process() {
        // Find the access instructions that may need to be clamped.
        Vector<ir::Access*, 64> accesses;
        Vector<ir::LoadVectorElement*, 64> vector_loads;
        Vector<ir::StoreVectorElement*, 64> vector_stores;
        Vector<ir::CoreBuiltinCall*, 64> subgroup_matrix_calls;
        Vector<ir::CoreBuiltinCall*, 64> texture_calls;

        if (config.use_integer_range_analysis) {
            integer_range_analysis.emplace(&ir);
        }

        for (auto* inst : ir.Instructions()) {
            tint::Switch(
                inst,  //
                [&](ir::Access* access) {
                    // Check if accesses into this object should be clamped.
                    if (access->Object()->Type()->Is<type::Pointer>()) {
                        if (ShouldClamp(access->Object())) {
                            accesses.Push(access);
                        }
                    } else {
                        if (config.clamp_value) {
                            accesses.Push(access);
                        }
                    }
                },
                [&](ir::LoadVectorElement* lve) {
                    // Check if loads from this value should be clamped.
                    if (ShouldClamp(lve->From())) {
                        vector_loads.Push(lve);
                    }
                },
                [&](ir::StoreVectorElement* sve) {
                    // Check if stores to this value should be clamped.
                    if (ShouldClamp(sve->To())) {
                        vector_stores.Push(sve);
                    }
                },
                [&](ir::CoreBuiltinCall* call) {
                    // Check if this is a texture builtin that needs to be clamped.
                    if (config.clamp_texture) {
                        if (call->Func() == core::BuiltinFn::kTextureDimensions ||
                            call->Func() == core::BuiltinFn::kTextureLoad) {
                            texture_calls.Push(call);
                        }
                    }
                    // Check if this is a subgroup matrix builtin that needs to be clamped.
                    if (call->Func() == core::BuiltinFn::kSubgroupMatrixLoad ||
                        call->Func() == core::BuiltinFn::kSubgroupMatrixStore) {
                        subgroup_matrix_calls.Push(call);
                    }
                });
        }

        // Clamp access indices.
        for (auto* access : accesses) {
            b.InsertBefore(access, [&] {  //
                ClampAccessIndices(access);
            });
        }

        // Clamp load-vector-element instructions.
        for (auto* lve : vector_loads) {
            auto* vec = lve->From()->Type()->UnwrapPtr()->As<type::Vector>();
            b.InsertBefore(lve, [&] {  //
                ClampOperand(lve, LoadVectorElement::kIndexOperandOffset,
                             b.Constant(u32(vec->Width() - 1u)));
            });
        }

        // Clamp store-vector-element instructions.
        for (auto* sve : vector_stores) {
            auto* vec = sve->To()->Type()->UnwrapPtr()->As<type::Vector>();
            b.InsertBefore(sve, [&] {  //
                ClampOperand(sve, StoreVectorElement::kIndexOperandOffset,
                             b.Constant(u32(vec->Width() - 1u)));
            });
        }

        // Clamp indices and coordinates for texture builtins calls.
        for (auto* call : texture_calls) {
            b.InsertBefore(call, [&] {  //
                ClampTextureCallArgs(call);
            });
        }

        // Predicate subgroup matrix loads and stores based on their offset and stride.
        for (auto* call : subgroup_matrix_calls) {
            b.InsertBefore(call, [&] {  //
                PredicateSubgroupMatrixCall(call);
            });
        }
    }

    /// Check if clamping should be applied to a particular value.
    /// @param value the value to check. The value's type must be type::Pointer.
    /// @returns true if pointer accesses in @p param should be clamped
    bool ShouldClamp(Value* value) {
        auto* ptr = value->Type()->As<type::Pointer>();
        TINT_ASSERT(ptr);
        switch (ptr->AddressSpace()) {
            case AddressSpace::kFunction:
                return config.clamp_function;
            case AddressSpace::kPrivate:
                return config.clamp_private;
            case AddressSpace::kImmediate:
                return config.clamp_immediate_data;
            case AddressSpace::kStorage:
                return config.clamp_storage && !IsRootVarIgnored(value);
            case AddressSpace::kUniform:
                return config.clamp_uniform && !IsRootVarIgnored(value);
            case AddressSpace::kWorkgroup:
                return config.clamp_workgroup;
            case AddressSpace::kUndefined:
            case AddressSpace::kPixelLocal:
            case AddressSpace::kHandle:
            case AddressSpace::kIn:
            case AddressSpace::kOut:
                return false;
        }
        return false;
    }

    /// Convert a value to a u32 if needed.
    /// @param value the value to convert
    /// @returns the converted value, or @p value if it is already a u32
    ir::Value* CastToU32(ir::Value* value) {
        if (value->Type()->IsUnsignedIntegerScalarOrVector()) {
            return value;
        }

        const type::Type* type = ty.u32();
        if (auto* vec = value->Type()->As<type::Vector>()) {
            type = ty.vec(type, vec->Width());
        }
        return b.Convert(type, value)->Result();
    }

    /// Clamp operand @p op_idx of @p inst to ensure it is within @p limit.
    /// @param inst the instruction
    /// @param op_idx the index of the operand that should be clamped
    /// @param limit the limit to clamp to
    void ClampOperand(ir::Instruction* inst, size_t op_idx, ir::Value* limit) {
        auto* idx = inst->Operands()[op_idx];
        auto* const_idx = idx->As<ir::Constant>();
        auto* const_limit = limit->As<ir::Constant>();

        ir::Value* clamped_idx = nullptr;
        if (const_idx && const_limit) {
            // Generate a new constant index that is clamped to the limit.
            clamped_idx = b.Constant(u32(std::min(const_idx->Value()->ValueAs<uint32_t>(),
                                                  const_limit->Value()->ValueAs<uint32_t>())));
        } else if (IndexMayOutOfBound(idx, limit)) {
            // Clamp it to the dynamic limit.
            clamped_idx = b.Call(ty.u32(), core::BuiltinFn::kMin, CastToU32(idx), limit)->Result();
        }

        if (clamped_idx != nullptr) {
            // Replace the index operand with the clamped version.
            inst->SetOperand(op_idx, clamped_idx);
        }
    }

    /// Check if operand @p idx may be less than 0 or greater than @p limit with integer range
    /// analysis algorithm.
    /// @param idx the index to check
    /// @param limit the upper limit @idx to compare with.
    /// @returns true when @idx may be out of bound, false otherwise
    bool IndexMayOutOfBound(ir::Value* idx, ir::Value* limit) {
        // Return true when integer range analysis is disabled.
        if (!integer_range_analysis.has_value()) {
            return true;
        }

        // Return true when `limit` is not a constant value.
        auto* const_limit = limit->As<ir::Constant>();
        if (!const_limit) {
            return true;
        }

        // Return true when we cannot get a valid range for `idx`.
        const auto& integer_range = integer_range_analysis->GetInfo(idx);
        if (!integer_range.IsValid()) {
            return true;
        }

        TINT_ASSERT(const_limit->Value()->Type()->Is<type::U32>());
        uint32_t const_limit_value = const_limit->Value()->ValueAs<uint32_t>();

        using SignedIntegerRange = ir::analysis::IntegerRangeInfo::SignedIntegerRange;
        using UnsignedIntegerRange = ir::analysis::IntegerRangeInfo::UnsignedIntegerRange;

        // Return true when `idx` may be negative or the upper bound of `idx` is greater than
        // `limit`.
        if (std::holds_alternative<UnsignedIntegerRange>(integer_range.range)) {
            UnsignedIntegerRange range = std::get<UnsignedIntegerRange>(integer_range.range);
            return range.max_bound > static_cast<uint64_t>(const_limit_value);
        } else {
            SignedIntegerRange range = std::get<SignedIntegerRange>(integer_range.range);
            if (range.min_bound < 0) {
                return true;
            }
            return range.max_bound > static_cast<int64_t>(const_limit_value);
        }
    }

    /// Clamp the indices of an access instruction to ensure they are within the limits of the types
    /// that they are indexing into.
    /// @param access the access instruction
    void ClampAccessIndices(ir::Access* access) {
        auto* type = access->Object()->Type()->UnwrapPtr();
        auto indices = access->Indices();
        for (size_t i = 0; i < indices.Length(); i++) {
            auto* idx = indices[i];
            auto* const_idx = idx->As<ir::Constant>();

            // Determine the limit of the type being indexed into.
            auto limit = tint::Switch(
                type,  //
                [&](const type::Vector* vec) -> ir::Value* {
                    return b.Constant(u32(vec->Width() - 1u));
                },
                [&](const type::Matrix* mat) -> ir::Value* {
                    return b.Constant(u32(mat->Columns() - 1u));
                },
                [&](const type::Array* arr) -> ir::Value* {
                    if (arr->ConstantCount()) {
                        return b.Constant(u32(arr->ConstantCount().value() - 1u));
                    }
                    TINT_ASSERT(arr->Count()->Is<type::RuntimeArrayCount>());

                    // Skip clamping runtime-sized array indices if requested.
                    if (config.disable_runtime_sized_array_index_clamping) {
                        return nullptr;
                    }

                    auto* object = access->Object();
                    if (i > 0) {
                        // Generate a pointer to the runtime-sized array if it isn't the base of
                        // this access instruction.
                        auto* base_ptr = object->Type()->As<type::Pointer>();
                        TINT_ASSERT(base_ptr != nullptr);
                        TINT_ASSERT(i == 1);
                        auto* arr_ptr = ty.ptr(base_ptr->AddressSpace(), arr, base_ptr->Access());
                        object = b.Access(arr_ptr, object, indices[0])->Result();
                    }

                    // Use the `arrayLength` builtin to get the limit of a runtime-sized array.
                    auto* length = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, object);
                    return b.Subtract(ty.u32(), length, b.Constant(1_u))->Result();
                });

            // If there's a dynamic limit that needs enforced, clamp the index operand.
            if (limit) {
                ClampOperand(access, ir::Access::kIndicesOperandOffset + i, limit);
            }

            // Get the type that this index produces.
            type = const_idx ? type->Element(const_idx->Value()->ValueAs<u32>())
                             : type->Elements().type;
        }
    }

    /// Clamp the indices and coordinates of a texture builtin call instruction to ensure they are
    /// within the limits of the texture that they are accessing.
    /// @param call the texture builtin call instruction
    void ClampTextureCallArgs(ir::CoreBuiltinCall* call) {
        const auto& args = call->Args();
        auto* texture = args[0]->Type()->As<type::Texture>();

        // Helper for clamping the level argument.
        // Keep hold of the clamped value to use for clamping the coordinates.
        Value* clamped_level = nullptr;
        auto clamp_level = [&](uint32_t idx) {
            auto* num_levels = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, args[0]);
            auto* limit = b.Subtract(ty.u32(), num_levels, 1_u);
            clamped_level =
                b.Call(ty.u32(), core::BuiltinFn::kMin, CastToU32(args[idx]), limit)->Result();
            call->SetOperand(CoreBuiltinCall::kArgsOperandOffset + idx, clamped_level);
        };

        // Helper for clamping the coordinates.
        auto clamp_coords = [&](uint32_t idx) {
            auto* arg_ty = args[idx]->Type();
            const type::Type* type = ty.MatchWidth(ty.u32(), arg_ty);
            auto* one = b.MatchWidth(1_u, arg_ty);
            auto* dims = clamped_level ? b.Call(type, core::BuiltinFn::kTextureDimensions, args[0],
                                                clamped_level)
                                       : b.Call(type, core::BuiltinFn::kTextureDimensions, args[0]);
            auto* limit = b.Subtract(type, dims, one);
            call->SetOperand(
                CoreBuiltinCall::kArgsOperandOffset + idx,
                b.Call(type, core::BuiltinFn::kMin, CastToU32(args[idx]), limit)->Result());
        };

        // Helper for clamping the array index.
        auto clamp_array_index = [&](uint32_t idx) {
            auto* num_layers = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, args[0]);
            auto* limit = b.Subtract(ty.u32(), num_layers, 1_u);
            call->SetOperand(
                CoreBuiltinCall::kArgsOperandOffset + idx,
                b.Call(ty.u32(), core::BuiltinFn::kMin, CastToU32(args[idx]), limit)->Result());
        };

        // Select which arguments to clamp based on the function overload.
        switch (call->Func()) {
            case core::BuiltinFn::kTextureDimensions: {
                if (args.Length() > 1) {
                    clamp_level(1u);
                }
                break;
            }
            case core::BuiltinFn::kTextureLoad: {
                uint32_t next_arg = 2u;
                if (type::IsTextureArray(texture->Dim())) {
                    clamp_array_index(next_arg++);
                }
                if (texture->IsAnyOf<type::SampledTexture, type::DepthTexture>()) {
                    clamp_level(next_arg++);
                }
                clamp_coords(1u);  // Must run after clamp_level
                break;
            }
            case core::BuiltinFn::kTextureStore: {
                clamp_coords(1u);
                if (type::IsTextureArray(texture->Dim())) {
                    clamp_array_index(2u);
                }
                break;
            }
            default:
                break;
        }
    }

    /// Clamp the indices and coordinates of a texture builtin call instruction to ensure they are
    /// within the limits of the texture that they are accessing.
    /// @param call the texture builtin call instruction
    void PredicateSubgroupMatrixCall(ir::CoreBuiltinCall* call) {
        const auto& args = call->Args();

        // Extract the arguments from the call.
        auto* arr = args[0];
        auto* offset = args[1];
        Value* col_major = nullptr;
        Value* stride = nullptr;
        uint32_t stride_index = 0;
        const type::SubgroupMatrix* matrix_ty = nullptr;
        if (call->Func() == BuiltinFn::kSubgroupMatrixLoad) {
            col_major = args[2];
            stride = args[3];
            stride_index = 3;
            matrix_ty = call->Result()->Type()->As<type::SubgroupMatrix>();
        } else if (call->Func() == BuiltinFn::kSubgroupMatrixStore) {
            matrix_ty = args[2]->Type()->As<type::SubgroupMatrix>();
            col_major = args[3];
            stride = args[4];
            stride_index = 4;
        } else {
            TINT_UNREACHABLE();
        }

        // Determine the minimum valid stride, and the value that we will multiply the stride by to
        // determine the number of elements in memory that will be accessed.
        uint32_t min_stride = 0;
        uint32_t major_dim = 0;
        if (col_major->As<Constant>()->Value()->ValueAs<bool>()) {
            min_stride = matrix_ty->Rows();
            major_dim = matrix_ty->Columns();
        } else {
            min_stride = matrix_ty->Columns();
            major_dim = matrix_ty->Rows();
        }

        // Increase the stride so that it is at least `min_stride` if necessary.
        if (auto* const_stride = stride->As<Constant>()) {
            if (const_stride->Value()->ValueAs<uint32_t>() < min_stride) {
                stride = b.Constant(u32(min_stride));
            }
        } else {
            stride = b.Call(ty.u32(), core::BuiltinFn::kMax, stride, u32(min_stride))->Result();
        }
        call->SetArg(stride_index, stride);

        // If we are not predicating, then clamping the stride is all we need to do.
        if (!config.predicate_subgroup_matrix) {
            return;
        }

        // Some matrix components types are packed together into a single array element.
        // Take that into account here by scaling the array length to number of components.
        uint32_t components_per_element = 0;
        if (matrix_ty->Type()->IsAnyOf<type::I8, type::U8>()) {
            components_per_element = 4;
        } else {
            TINT_ASSERT((matrix_ty->Type()->IsAnyOf<type::F16, type::F32, type::I32, type::U32>()));
            components_per_element = 1;
        }

        // Get the length of the array (in terms of matrix elements).
        auto* arr_ty = arr->Type()->UnwrapPtr()->As<core::type::Array>();
        TINT_ASSERT(arr_ty);
        Value* array_length = nullptr;
        if (arr_ty->ConstantCount()) {
            array_length =
                b.Constant(u32(arr_ty->ConstantCount().value() * components_per_element));
        } else {
            TINT_ASSERT(arr_ty->Count()->Is<type::RuntimeArrayCount>());
            array_length = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, arr)->Result(0);
            if (components_per_element > 1) {
                array_length = b.Multiply<u32>(array_length, u32(components_per_element))->Result();
            }
        }

        // If the array length, offset, and stride are all constants, then we can determine if the
        // call is in bounds now and skip any predication if so.
        if (array_length->Is<Constant>() && stride->Is<Constant>() && offset->Is<Constant>()) {
            uint32_t const_length = array_length->As<Constant>()->Value()->ValueAs<uint32_t>();
            uint32_t const_stride = stride->As<Constant>()->Value()->ValueAs<uint32_t>();
            uint32_t const_offset = offset->As<Constant>()->Value()->ValueAs<uint32_t>();
            uint32_t const_end = const_offset + (const_stride * (major_dim - 1)) + min_stride;
            if (const_end <= const_length) {
                return;
            }
        }

        // Predicate the builtin call depending on whether it is in bounds.
        auto insertion_point = call->next;
        call->Remove();
        b.InsertBefore(insertion_point, [&] {
            // The beginning of the last row/column is at `offset + (major_dim-1)*stride`.
            // We then add another `min_stride` elements to get to the end of the accessed memory.
            auto* last_slice = b.Add<u32>(offset, b.Multiply<u32>(stride, u32(major_dim - 1)));
            auto* end = b.Add<u32>(last_slice, u32(min_stride));
            auto* in_bounds = b.LessThanEqual<bool>(end, array_length);
            if (call->Func() == BuiltinFn::kSubgroupMatrixLoad) {
                // Declare a variable to hold the result of the load, or a zero-initialized matrix.
                auto* result = b.Var(ty.ptr<function>(matrix_ty));
                auto* load_result = b.InstructionResult(matrix_ty);
                call->Result()->ReplaceAllUsesWith(load_result);

                auto* if_ = b.If(in_bounds);
                b.Append(if_->True(), [&] {  //
                    if_->True()->Append(call);
                    b.Store(result, call->Result());
                    b.ExitIf(if_);
                });
                b.LoadWithResult(load_result, result);
            } else if (call->Func() == BuiltinFn::kSubgroupMatrixStore) {
                auto* if_ = b.If(in_bounds);
                b.Append(if_->True(), [&] {  //
                    if_->True()->Append(call);
                    b.ExitIf(if_);
                });
            } else {
                TINT_UNREACHABLE();
            }
        });
    }

    // Returns the root Var for `value` by walking up the chain of instructions,
    // or nullptr if none is found.
    Var* RootVarFor(Value* value) {
        Var* result = nullptr;
        while (value) {
            TINT_ASSERT(value->Alive());
            value = tint::Switch(
                value,  //
                [&](InstructionResult* res) {
                    // value was emitted by an instruction
                    auto* inst = res->Instruction();
                    return tint::Switch(
                        inst,
                        [&](Access* access) {  //
                            return access->Object();
                        },
                        [&](Let* let) {  //
                            return let->Value();
                        },
                        [&](Var* var) {
                            result = var;
                            return nullptr;  // Done
                        },
                        TINT_ICE_ON_NO_MATCH);
                },
                [&](FunctionParam*) {
                    // Cannot follow function params to vars
                    return nullptr;
                },  //
                TINT_ICE_ON_NO_MATCH);
        }
        return result;
    }

    // Returns true if the binding for `value`'s root variable is in config.bindings_ignored.
    bool IsRootVarIgnored(Value* value) {
        if (auto* var = RootVarFor(value)) {
            if (auto bp = var->BindingPoint()) {
                if (config.bindings_ignored.find(*bp) != config.bindings_ignored.end()) {
                    return true;  // Ignore this variable
                }
            }
        }
        return false;
    }
};

}  // namespace

Result<SuccessType> Robustness(Module& ir, const RobustnessConfig& config) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.Robustness", kRobustnessCapabilities);
    if (result != Success) {
        return result;
    }

    State{config, ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
