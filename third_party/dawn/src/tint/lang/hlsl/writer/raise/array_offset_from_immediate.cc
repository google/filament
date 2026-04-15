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

#include "src/tint/lang/hlsl/writer/raise/array_offset_from_immediate.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/prepare_immediate_data.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/ir/member_builtin_call.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {

using core::ir::Value;
using core::ir::Var;

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// Immediate data layout contains all immediate block info.
    const core::ir::transform::ImmediateDataLayout& immediate_data_layout;

    /// The offset in immediate block for buffer offsets array.
    uint32_t buffer_offsets_offset = 0;

    /// The total number of vec4s used to store buffer offsets provided in the immediate block.
    uint32_t buffer_offsets_array_elements_num = 0;

    /// The map from binding point to the element index which holds the offset into that buffer.
    const std::unordered_map<BindingPoint, uint32_t>& bindpoint_to_offset_index;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        // Validate that buffer_offsets_array_elements_num is large enough
        for (const auto& [binding_point, offset_index] : bindpoint_to_offset_index) {
            uint32_t vec4_index = offset_index / 4;
            if (vec4_index >= buffer_offsets_array_elements_num) {
                TINT_ICE() << "ArrayOffsetFromImmediates: offset_index " << offset_index
                           << " requires vec4 element " << vec4_index
                           << " but buffer_offsets_array_elements_num is "
                           << buffer_offsets_array_elements_num;
            }
        }

        // Look for root vars with a matching binding point in bindpoint_to_offset_index
        for (auto* inst : *ir.root_block) {
            auto* var = inst->As<Var>();
            if (!var) {
                continue;
            }

            if (!var->BindingPoint()) {
                continue;
            }

            auto it = bindpoint_to_offset_index.find(*var->BindingPoint());
            if (it == bindpoint_to_offset_index.end()) {
                continue;
            }

            ProcessVar(var, it->second);
        }
    }

    void ProcessVar(Var* var, uint32_t offset_index) {
        // Because this transform is run after both DirectVariableAccess and DecomposeStorageAccess,
        // all loads and stores to storage buffers with bind points in bindpoint_to_offset_index are
        // now "byte_address_buffer.Load{N}/Store{N}" calls. We iterate over all usages of 'var' and
        // update these loads and stores to add the extra offset.

        // Copy because we destroy elements while iterating
        auto usages_unsorted_copy = var->Result()->UsagesUnsorted();

        for (auto usage : usages_unsorted_copy) {
            tint::Switch(
                usage->instruction,
                [&](hlsl::ir::MemberBuiltinCall* mbc) {
                    // Adds the dynamic offset loaded from the immediate block at offset_index to
                    // the mbc's argument at 'arg_index'.
                    auto add_offset_to_arg = [&](uint32_t arg_index) {
                        b.InsertBefore(mbc, [&] {
                            Value* curr_offset = mbc->Args()[arg_index];
                            Value* dyn_offset = LoadDynamicOffset(offset_index);
                            auto* new_offset = b.Add(curr_offset, dyn_offset);
                            mbc->SetArg(arg_index, new_offset->Result());
                        });
                    };

                    switch (mbc->Func()) {
                        // Handle all member functions that take a byte_address_buffer and an offset
                        case hlsl::BuiltinFn::kInterlockedCompareExchange:
                        case hlsl::BuiltinFn::kInterlockedExchange:
                        case hlsl::BuiltinFn::kInterlockedAdd:
                        case hlsl::BuiltinFn::kInterlockedMax:
                        case hlsl::BuiltinFn::kInterlockedMin:
                        case hlsl::BuiltinFn::kInterlockedAnd:
                        case hlsl::BuiltinFn::kInterlockedOr:
                        case hlsl::BuiltinFn::kInterlockedXor:
                        case hlsl::BuiltinFn::kLoad:
                        case hlsl::BuiltinFn::kLoad2:
                        case hlsl::BuiltinFn::kLoad3:
                        case hlsl::BuiltinFn::kLoad4:
                        case hlsl::BuiltinFn::kLoadF16:
                        case hlsl::BuiltinFn::kLoad2F16:
                        case hlsl::BuiltinFn::kLoad3F16:
                        case hlsl::BuiltinFn::kLoad4F16:
                        case hlsl::BuiltinFn::kLoadU16:
                        case hlsl::BuiltinFn::kLoad2U16:
                        case hlsl::BuiltinFn::kLoad3U16:
                        case hlsl::BuiltinFn::kLoad4U16:
                        case hlsl::BuiltinFn::kStore:
                        case hlsl::BuiltinFn::kStore2:
                        case hlsl::BuiltinFn::kStore3:
                        case hlsl::BuiltinFn::kStore4:
                        case hlsl::BuiltinFn::kStoreF16:
                        case hlsl::BuiltinFn::kStore2F16:
                        case hlsl::BuiltinFn::kStore3F16:
                        case hlsl::BuiltinFn::kStore4F16:
                        case hlsl::BuiltinFn::kStoreU16:
                        case hlsl::BuiltinFn::kStore2U16:
                        case hlsl::BuiltinFn::kStore3U16:
                        case hlsl::BuiltinFn::kStore4U16:
                            add_offset_to_arg(0);
                            break;
                        // Ignore the functions below
                        case hlsl::BuiltinFn::kAsint:
                        case hlsl::BuiltinFn::kAsuint:
                        case hlsl::BuiltinFn::kAsfloat:
                        case hlsl::BuiltinFn::kAsuint16:
                        case hlsl::BuiltinFn::kAsfloat16:
                        case hlsl::BuiltinFn::kDot4AddI8Packed:
                        case hlsl::BuiltinFn::kDot4AddU8Packed:
                        case hlsl::BuiltinFn::kF32Tof16:
                        case hlsl::BuiltinFn::kF16Tof32:
                        case hlsl::BuiltinFn::kMul:
                        case hlsl::BuiltinFn::kPackU8:
                        case hlsl::BuiltinFn::kPackS8:
                        case hlsl::BuiltinFn::kPackClampS8:
                        case hlsl::BuiltinFn::kConvert:
                        case hlsl::BuiltinFn::kSign:
                        case hlsl::BuiltinFn::kTextureStore:
                        case hlsl::BuiltinFn::kUnpackS8S32:
                        case hlsl::BuiltinFn::kUnpackU8U32:
                        case hlsl::BuiltinFn::kWaveGetLaneIndex:
                        case hlsl::BuiltinFn::kWaveGetLaneCount:
                        case hlsl::BuiltinFn::kWaveReadLaneAt:
                        case hlsl::BuiltinFn::kModf:
                        case hlsl::BuiltinFn::kFrexp:
                        case hlsl::BuiltinFn::kGatherCmp:
                        case hlsl::BuiltinFn::kGather:
                        case hlsl::BuiltinFn::kGatherAlpha:
                        case hlsl::BuiltinFn::kGatherBlue:
                        case hlsl::BuiltinFn::kGatherGreen:
                        case hlsl::BuiltinFn::kGatherRed:
                        case hlsl::BuiltinFn::kGetDimensions:
                        case hlsl::BuiltinFn::kSample:
                        case hlsl::BuiltinFn::kSampleBias:
                        case hlsl::BuiltinFn::kSampleCmp:
                        case hlsl::BuiltinFn::kSampleCmpLevelZero:
                        case hlsl::BuiltinFn::kSampleGrad:
                        case hlsl::BuiltinFn::kSampleLevel:
                        case hlsl::BuiltinFn::kNone:
                            break;
                    }
                },
                TINT_ICE_ON_NO_MATCH);
        }
    }

    /// Loads the storage buffer dynamic offset from the immediate block.
    /// @returns the loaded dynamic offset value
    Value* LoadDynamicOffset(uint32_t offset_index) {
        // Load the dynamic offset from the immediate block.
        // The offsets are packed into vec4s to satisfy the 16-byte alignment requirement for
        // array elements in immediate block, so we have to find the vector and element that
        // correspond to the index that we want.
        const uint32_t array_index = offset_index / 4;
        const uint32_t vec_index = offset_index % 4;
        auto* buffer_offsets = b.Access(
            ty.ptr(immediate, ty.array(ty.vec4u(), buffer_offsets_array_elements_num)),
            immediate_data_layout.var, u32(immediate_data_layout.IndexOf(buffer_offsets_offset)));
        auto* vec_ptr =
            b.Access(ty.ptr(immediate, ty.vec4u()), buffer_offsets->Result(), u32(array_index));
        return b.LoadVectorElement(vec_ptr, u32(vec_index))->Result();
    }
};

}  // namespace

Result<SuccessType> ArrayOffsetFromImmediates(
    core::ir::Module& ir,
    const ImmediateDataLayout& immediate_data_layout,
    const uint32_t buffer_offsets_offset,
    const uint32_t buffer_offsets_array_elements_num,
    const std::unordered_map<BindingPoint, uint32_t>& bindpoint_to_offset_index) {
    AssertValid(ir, kArrayOffsetFromImmediateCapabilities, "before core.ArrayOffsetFromImmediates");

    State state{ir, immediate_data_layout, buffer_offsets_offset, buffer_offsets_array_elements_num,
                bindpoint_to_offset_index};
    state.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
