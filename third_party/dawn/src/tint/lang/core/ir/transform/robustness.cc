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
        Vector<ir::CoreBuiltinCall*, 64> buffer_view_calls;

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
                        accesses.Push(access);
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
                    // Check if this is a buffer view builtin that needs to be clamped.
                    if (call->Func() == core::BuiltinFn::kBufferView ||
                        call->Func() == core::BuiltinFn::kBufferArrayView) {
                        if (ShouldClamp(call->Args()[0])) {
                            buffer_view_calls.Push(call);
                        }
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

        // Clamp subgroup matrix loads and stores based on their offset and stride.
        for (auto* call : subgroup_matrix_calls) {
            b.InsertBefore(call, [&] {  //
                ClampSubgroupMatrixCall(call);
            });
        }

        // Clamp offset and size for buffer[Array]View calls.
        for (auto* call : buffer_view_calls) {
            b.InsertBefore(call, [&] {  //
                ClampBufferViewArgs(call);
            });
        }
    }

    /// Check if clamping should be applied to a particular value.
    /// @param value the value to check. The value's type must be type::Pointer.
    /// @returns true if pointer accesses in @p param should be clamped
    bool ShouldClamp(Value* value) {
        auto* ptr = value->Type()->As<type::Pointer>();
        TINT_IR_ASSERT(ir, ptr);
        switch (ptr->AddressSpace()) {
            case AddressSpace::kFunction:
            case AddressSpace::kPrivate:
            case AddressSpace::kWorkgroup:
                return true;
            case AddressSpace::kImmediate:
                return config.clamp_immediate_data;
            case AddressSpace::kStorage:
                return config.clamp_storage && !IsRootVarIgnored(value);
            case AddressSpace::kUniform:
                return config.clamp_uniform && !IsRootVarIgnored(value);
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
            TINT_IR_ASSERT(ir, const_idx->Value()->ValueAs<uint32_t>() <=
                                   const_limit->Value()->ValueAs<uint32_t>());
            clamped_idx = b.Constant(u32(const_idx->Value()->ValueAs<uint32_t>()));
        } else if (IndexMayOutOfBound(idx, limit)) {
            // Clamp it to the dynamic limit.
            clamped_idx = b.Min(CastToU32(idx), limit)->Result();
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

        TINT_IR_ASSERT(ir, const_limit->Value()->Type()->Is<type::U32>());
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
        for (size_t i = 0; i < indices.size(); i++) {
            auto* idx = indices[i];
            auto* const_idx = idx->As<ir::Constant>();

            // Determine the limit of the type being indexed into.
            auto maxAllowedIndex = tint::Switch(
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
                    TINT_IR_ASSERT(ir, arr->Count()->Is<type::RuntimeArrayCount>());

                    // Skip clamping runtime-sized array indices if requested.
                    if (config.disable_runtime_sized_array_index_clamping) {
                        return nullptr;
                    }

                    auto* object = access->Object();
                    if (i > 0) {
                        // Generate a pointer to the runtime-sized array if it isn't the base of
                        // this access instruction.
                        auto* base_ptr = object->Type()->As<type::Pointer>();
                        TINT_IR_ASSERT(ir, base_ptr != nullptr);
                        TINT_IR_ASSERT(ir, i == 1);
                        auto* arr_ptr = ty.ptr(base_ptr->AddressSpace(), arr, base_ptr->Access());
                        object = b.Access(arr_ptr, object, indices[0])->Result();
                    }

                    // Use the `arrayLength` builtin to get the limit of a runtime-sized array.
                    // Subtract 1 to get the max allowed index. (Array size is always at least 1.)
                    auto* length = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, object);
                    return b.Subtract(length, b.Constant(1_u))->Result();
                });

            // If there's a dynamic limit that needs enforced, clamp the index operand.
            if (maxAllowedIndex) {
                ClampOperand(access, ir::Access::kIndicesOperandOffset + i, maxAllowedIndex);
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
            auto* limit = b.Subtract(num_levels, 1_u);
            clamped_level = b.Min(CastToU32(args[idx]), limit)->Result();
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
            auto* limit = b.Subtract(dims, one);
            call->SetOperand(CoreBuiltinCall::kArgsOperandOffset + idx,
                             b.Min(CastToU32(args[idx]), limit)->Result());
        };

        // Helper for clamping the array index.
        auto clamp_array_index = [&](uint32_t idx) {
            auto* num_layers = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, args[0]);
            auto* limit = b.Subtract(num_layers, 1_u);
            call->SetOperand(CoreBuiltinCall::kArgsOperandOffset + idx,
                             b.Min(CastToU32(args[idx]), limit)->Result());
        };

        // Helper for clamping the sample index.
        auto clamp_sample_index = [&](uint32_t idx) {
            auto* num_samples = b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, args[0]);
            auto* limit = b.Subtract(num_samples, 1_u);
            call->SetOperand(CoreBuiltinCall::kArgsOperandOffset + idx,
                             b.Min(CastToU32(args[idx]), limit)->Result());
        };

        // Select which arguments to clamp based on the function overload.
        switch (call->Func()) {
            case core::BuiltinFn::kTextureDimensions: {
                if (args.size() > 1) {
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
                if (texture->IsAnyOf<type::MultisampledTexture, type::DepthMultisampledTexture>()) {
                    clamp_sample_index(next_arg++);
                }
                clamp_coords(1u);  // Must run after clamp_level
                break;
            }
            default:
                break;
        }
    }

    /// Clamp the indices and coordinates of a texture builtin call instruction to ensure they are
    /// within the limits of the texture that they are accessing.
    /// @param call the texture builtin call instruction
    void ClampSubgroupMatrixCall(ir::CoreBuiltinCall* call) {
        const auto& args = call->Args();

        TINT_IR_ASSERT(ir, (call->Func() == BuiltinFn::kSubgroupMatrixLoad &&
                            call->ExplicitTemplateParams().Length() == 2) ||
                               (call->Func() == BuiltinFn::kSubgroupMatrixStore &&
                                call->ExplicitTemplateParams().Length() == 1));

        // Extract the arguments from the call.
        auto* arr = args[0];
        auto* offset = args[1];
        bool col_major = true;
        Value* stride = nullptr;
        uint32_t stride_index = 0;
        const type::SubgroupMatrix* matrix_ty = nullptr;
        if (call->Func() == BuiltinFn::kSubgroupMatrixLoad) {
            TINT_IR_ASSERT(
                ir, std::holds_alternative<core::Majorness>(call->ExplicitTemplateParams()[1]));
            col_major = std::get<core::Majorness>(call->ExplicitTemplateParams()[1]) ==
                        core::Majorness::kColMajor;
            stride = args[2];
            stride_index = 2;
            matrix_ty = call->Result()->Type()->As<type::SubgroupMatrix>();
        } else if (call->Func() == BuiltinFn::kSubgroupMatrixStore) {
            matrix_ty = args[2]->Type()->As<type::SubgroupMatrix>();
            TINT_IR_ASSERT(
                ir, std::holds_alternative<core::Majorness>(call->ExplicitTemplateParams()[0]));
            col_major = std::get<core::Majorness>(call->ExplicitTemplateParams()[0]) ==
                        core::Majorness::kColMajor;
            stride = args[3];
            stride_index = 3;
        } else {
            TINT_IR_UNREACHABLE(ir);
        }

        auto* arr_ty = arr->Type()->UnwrapPtr()->As<core::type::Array>();
        const uint32_t arr_stride = arr_ty->ImplicitStride();

        // Determine the minimum valid stride, and the value that we will multiply the stride by to
        // determine the number of elements in memory that will be accessed.
        uint32_t min_stride = 0;
        uint32_t major_dim = 0;
        if (col_major) {
            min_stride = matrix_ty->Rows();
            major_dim = matrix_ty->Columns();
        } else {
            min_stride = matrix_ty->Columns();
            major_dim = matrix_ty->Rows();
        }
        // Offset and stride are counted in array stride.
        // Note: max comes from situations like 8x8 u8 accessed from an array of vec4u.
        min_stride = std::max(min_stride * matrix_ty->Type()->Size() / arr_stride, 1u);

        // Increase the stride so that it is at least `min_stride` if necessary.
        if (auto* const_stride = stride->As<Constant>()) {
            if (const_stride->Value()->ValueAs<uint32_t>() < min_stride) {
                stride = b.Constant(u32(min_stride));
            }
        } else {
            stride = b.InsertBitcastIfNeeded(ty.u32(), stride);
            stride = b.Max(stride, u32(min_stride))->Result();
        }
        call->SetArg(stride_index, stride);

        // If we are not predicating, then clamping the stride is all we need to do.
        if (!config.clamp_subgroup_matrix) {
            return;
        }

        // Get the length of the array.
        TINT_IR_ASSERT(ir, arr_ty);
        Value* array_length = nullptr;
        if (arr_ty->ConstantCount()) {
            array_length = b.Constant(u32(arr_ty->ConstantCount().value()));
        } else {
            TINT_IR_ASSERT(ir, arr_ty->Count()->Is<type::RuntimeArrayCount>());
            array_length = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, arr)->Result(0);
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

        // Binding size is guaranteed to hold enough for `min_stride` matrix. So check if the
        // array length is sufficient for the given parameters and, if not, use 0 offset and
        // minimum stride.
        b.InsertBefore(call, [&] {
            // The beginning of the last row/column is at `offset + (major_dim-1)*stride`.
            // We then add another `min_stride` elements to get to the end of the accessed
            // memory.
            offset = b.InsertBitcastIfNeeded(ty.u32(), offset);
            stride = b.InsertBitcastIfNeeded(ty.u32(), stride);
            auto* last_slice = b.Add(offset, b.Multiply(stride, u32(major_dim - 1)))->Result();
            auto* end = b.Add(last_slice, u32(min_stride));
            auto* in_bounds = b.LessThanEqual(end, array_length);
            offset = b.Call(ty.u32(), BuiltinFn::kSelect, 0_u, offset, in_bounds)->Result();
            stride =
                b.Call(ty.u32(), BuiltinFn::kSelect, u32(min_stride), stride, in_bounds)->Result();
            call->SetArg(1, offset);
            call->SetArg(stride_index, stride);
        });
    }

    uint32_t MaxSubgroupMatrixSizeUse(const CoreBuiltinCall* arrayView) {
        TINT_IR_ASSERT(ir, arrayView->Func() == BuiltinFn::kBufferArrayView);

        uint32_t size = 0;
        Vector<Usage, 4> worklist;
        for (auto& u : arrayView->Result()->UsagesUnsorted()) {
            worklist.Push(u);
        }

        while (!worklist.IsEmpty()) {
            auto use = worklist.Pop();

            // Since we're starting at bufferArrayView call there aren't too many possible uses we
            // have to consider.
            tint::Switch(
                use.instruction,
                [&](const Let* let) {
                    for (auto& u : let->Result()->UsagesUnsorted()) {
                        worklist.Push(u);
                    }
                },
                [&](const UserCall* call) {
                    auto* target = call->Target();
                    auto* param = target->Params()[use.operand_index - call->ArgsOperandOffset()];
                    for (auto& u : param->UsagesUnsorted()) {
                        worklist.Push(u);
                    }
                },
                [&](const CoreBuiltinCall* call) {
                    const type::SubgroupMatrix* mat_ty = nullptr;
                    if (call->Func() == BuiltinFn::kSubgroupMatrixLoad) {
                        mat_ty = call->Result()->Type()->As<type::SubgroupMatrix>();
                    }
                    if (call->Func() == BuiltinFn::kSubgroupMatrixStore) {
                        mat_ty = call->Args()[2]->Type()->As<type::SubgroupMatrix>();
                    }
                    if (mat_ty) {
                        uint32_t mat_size =
                            mat_ty->Rows() * mat_ty->Columns() * mat_ty->Type()->Size();
                        size = std::max(size, mat_size);
                    }
                },
                [&](const Access* access) {
                    for (auto& u : access->Result()->UsagesUnsorted()) {
                        worklist.Push(u);
                    }
                },
                [&](Default) {});
        }

        return size;
    }

    void ClampBufferViewArgs(ir::CoreBuiltinCall* call) {
        // bufferView %ptr, %offset, [%length]
        // bufferArrayView %ptr, %offset, %size, [%length]

        // Determine the minimum size need for the return type.
        // If the type does not have a fixed footprint (i.e. contains a runtime-sized array) then we
        // want to ensure at least one element of it is included.
        auto* store_ty = call->Result()->Type()->UnwrapPtrOrRef();
        uint32_t ty_required_size = 0;
        uint32_t ty_stride = 0;
        uint32_t ty_offset = 0;
        if (store_ty->HasFixedFootprint()) {
            ty_required_size = store_ty->Size();
        } else {
            if (auto* str_ty = store_ty->As<type::Struct>()) {
                auto last = str_ty->Members().Back();
                auto last_ty = last->Type();
                TINT_IR_ASSERT(ir, last_ty->Is<type::Array>());
                ty_offset = last->Offset();
                ty_stride = last_ty->As<type::Array>()->ImplicitStride();
                ty_required_size = ty_offset + ty_stride;
            } else {
                TINT_IR_ASSERT(ir, store_ty->Is<type::Array>());
                ty_stride = store_ty->As<type::Array>()->ImplicitStride();
                ty_required_size = ty_stride;
            }
        }

        // The bound buffer is guaranteed to be large enough for any subgroup matrix access, but the
        // size operand on bufferArrayView might be smaller than necessary. Search forwards for any
        // subgroup matrix memory access and ensure the minimum size is large enough to accommodate
        // the maximum needed size.
        if (call->Func() == core::BuiltinFn::kBufferArrayView) {
            uint32_t max_subgroup_matrix_size = MaxSubgroupMatrixSizeUse(call);
            ty_required_size = std::max(ty_required_size, max_subgroup_matrix_size + ty_offset);
        }

        b.InsertBefore(call, [&] {
            uint32_t required_size = 0;
            auto* offset = call->Args()[1];
            auto* size = call->Func() == BuiltinFn::kBufferArrayView ? call->Args()[2] : nullptr;
            // If the length arg exists, use it. Otherwise, insert a bufferLength call.
            Value* length = nullptr;
            if (call->Func() == BuiltinFn::kBufferView && call->Args().size() > 2) {
                length = call->Args()[2];
            } else if (call->Func() == BuiltinFn::kBufferArrayView && call->Args().size() > 3) {
                length = call->Args()[3];
            } else {
                length = b.Call(ty.u32(), BuiltinFn::kBufferLength, call->Args()[0])->Result();
            }

            // Handle constant arguments.
            bool const_offset = false;
            bool const_size = false;
            if (auto* offset_cnst = offset->As<Constant>()) {
                uint32_t offset_val = offset_cnst->Value()->ValueAs<uint32_t>();
                required_size += offset_val;
                const_offset = true;
                if (!offset->Type()->Is<core::type::U32>()) {
                    offset = b.Constant(u32(offset_val));
                }
            }
            if (size) {
                if (auto* size_cnst = size->As<Constant>()) {
                    auto size_val = size_cnst->Value()->ValueAs<uint32_t>();
                    required_size += size_val;
                    const_size = true;
                    if (!size->Type()->Is<core::type::U32>()) {
                        size = b.Constant(u32(size_val));
                    }
                }
            } else {
                required_size += ty_required_size;
            }

            Value* total_required_size =
                required_size == 0 ? nullptr : b.Constant(u32(required_size));
            if (!const_offset) {
                offset = b.InsertBitcastIfNeeded(ty.u32(), offset);
                if (total_required_size) {
                    total_required_size =
                        b.Call(ty.u32(), BuiltinFn::kAddSat, total_required_size, offset)->Result();
                } else {
                    TINT_IR_ASSERT(ir, size && !const_size);
                    total_required_size = offset;
                }
            }
            if (size && !const_size) {
                // Use the larger of the size arg or the type required size.
                // PropagateBufferSizes performed a round down on the argument which may have
                // resulted in a 0 length array.
                size = b.InsertBitcastIfNeeded(ty.u32(), size);
                size = b.Call(ty.u32(), BuiltinFn::kMax, size, b.Constant(u32(ty_required_size)))
                           ->Result();
                if (total_required_size) {
                    total_required_size =
                        b.Call(ty.u32(), BuiltinFn::kAddSat, total_required_size, size)->Result();
                } else {
                    // offset must have been 0.
                    TINT_IR_ASSERT(ir, offset && const_offset);
                    total_required_size = size;
                }
            }

            // Now check if length < total_required_size
            // If true, this is an invalid memory view and we will try to patch it safely.
            // If false, use the args as is.
            //
            // In the bad case we have the liberty to generate anything safe.
            // So just use offset = 0 and size = required_size.
            // This will generate at least one element in the runtime array.
            // This should be always safe as minimum binding sizes ought to be based on these values
            // via inspection.
            // TODO(github.com/gpuweb/issues/5410): If this resolution changes we may need to
            // introduce predication instead of clamping.
            if (length->Is<Constant>() && total_required_size->Is<Constant>()) {
                bool cmp = length->As<Constant>()->Value()->ValueAs<uint32_t>() <
                           total_required_size->As<Constant>()->Value()->ValueAs<uint32_t>();
                call->SetArg(1, (cmp ? b.Constant(0_u) : offset));
                if (size) {
                    call->SetArg(2, (cmp ? b.Constant(u32(ty_required_size)) : size));
                }
                return;
            }

            auto* len_less_than = b.LessThan(length, total_required_size);
            auto* offset_select = b.Call(ty.u32(), BuiltinFn::kSelect, offset, 0_u, len_less_than);
            call->SetArg(1, offset_select->Result());
            Instruction* size_select = nullptr;
            if (size) {
                size_select = b.Call(ty.u32(), BuiltinFn::kSelect, size,
                                     b.Constant(u32(ty_required_size)), len_less_than);
                call->SetArg(2, size_select->Result());
            }
        });
    }

    // Returns the root Var for `value` by walking up the chain of instructions,
    // or nullptr if none is found.
    Var* RootVarFor(Value* value) {
        Var* result = nullptr;
        while (value) {
            TINT_IR_ASSERT(ir, value->Alive());
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
                        [&](CoreBuiltinCall* call) {
                            Value* call_value = nullptr;
                            if (call->Func() == BuiltinFn::kBufferView ||
                                call->Func() == BuiltinFn::kBufferArrayView) {
                                call_value = call->Args()[0];
                            }
                            return call_value;
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
    AssertValid(ir, "before core.Robustness");

    State{config, ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
