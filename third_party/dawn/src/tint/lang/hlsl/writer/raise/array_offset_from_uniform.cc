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

#include "src/tint/lang/hlsl/writer/raise/array_offset_from_uniform.h"

#include <algorithm>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/hlsl/ir/member_builtin_call.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::hlsl::writer::raise {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The binding point to use for the uniform buffer.
    BindingPoint ubo_binding;

    /// The map from binding point to the element index which holds the offset into that buffer.
    const std::unordered_map<BindingPoint, uint32_t>& bindpoint_to_offset_index;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The uniform buffer variable that holds the dynamic offset of each storage buffer.
    core::ir::Var* buffer_sizes_var = nullptr;

    /// Process the module.
    void Process() {
        // Look for root vars with a matching binding point in bindpoint_to_offset_index
        for (auto* inst : *ir.root_block) {
            auto* var = inst->As<core::ir::Var>();
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

    void ProcessVar(core::ir::Var* var, uint32_t offset_index) {
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
                    // Adds the dynamic offset loaded from the uniform buffer at offset_index to
                    // the mbc's argument at 'arg_index'.
                    auto add_offset_to_arg = [&](uint32_t arg_index) {
                        b.InsertBefore(mbc, [&] {
                            core::ir::Value* curr_offset = mbc->Args()[arg_index];
                            core::ir::Value* dyn_offset = LoadDynamicOffset(offset_index);
                            auto* new_offset = b.Add(curr_offset, dyn_offset);
                            mbc->SetArg(arg_index, new_offset->Result());
                        });
                    };

                    switch (mbc->Func()) {
                        // Handle all member functions that take a byte_address_buffer and an offset
                        case BuiltinFn::kInterlockedCompareExchange:
                        case BuiltinFn::kInterlockedExchange:
                        case BuiltinFn::kInterlockedAdd:
                        case BuiltinFn::kInterlockedMax:
                        case BuiltinFn::kInterlockedMin:
                        case BuiltinFn::kInterlockedAnd:
                        case BuiltinFn::kInterlockedOr:
                        case BuiltinFn::kInterlockedXor:
                        case BuiltinFn::kLoad:
                        case BuiltinFn::kLoad2:
                        case BuiltinFn::kLoad3:
                        case BuiltinFn::kLoad4:
                        case BuiltinFn::kLoadF16:
                        case BuiltinFn::kLoad2F16:
                        case BuiltinFn::kLoad3F16:
                        case BuiltinFn::kLoad4F16:
                        case BuiltinFn::kLoadU16:
                        case BuiltinFn::kLoad2U16:
                        case BuiltinFn::kLoad3U16:
                        case BuiltinFn::kLoad4U16:
                        case BuiltinFn::kStore:
                        case BuiltinFn::kStore2:
                        case BuiltinFn::kStore3:
                        case BuiltinFn::kStore4:
                        case BuiltinFn::kStoreF16:
                        case BuiltinFn::kStore2F16:
                        case BuiltinFn::kStore3F16:
                        case BuiltinFn::kStore4F16:
                        case BuiltinFn::kStoreU16:
                        case BuiltinFn::kStore2U16:
                        case BuiltinFn::kStore3U16:
                        case BuiltinFn::kStore4U16:
                            add_offset_to_arg(0);
                            break;
                        // Ignore the functions below
                        case BuiltinFn::kAsint:
                        case BuiltinFn::kAsuint:
                        case BuiltinFn::kAsfloat:
                        case BuiltinFn::kAsuint16:
                        case BuiltinFn::kAsfloat16:
                        case BuiltinFn::kDot4AddI8Packed:
                        case BuiltinFn::kDot4AddU8Packed:
                        case BuiltinFn::kF32Tof16:
                        case BuiltinFn::kF16Tof32:
                        case BuiltinFn::kMul:
                        case BuiltinFn::kPackU8:
                        case BuiltinFn::kPackS8:
                        case BuiltinFn::kPackClampS8:
                        case BuiltinFn::kConvert:
                        case BuiltinFn::kSign:
                        case BuiltinFn::kTextureStore:
                        case BuiltinFn::kUnpackS8S32:
                        case BuiltinFn::kUnpackU8U32:
                        case BuiltinFn::kWaveGetLaneIndex:
                        case BuiltinFn::kWaveGetLaneCount:
                        case BuiltinFn::kWaveReadLaneAt:
                        case BuiltinFn::kModf:
                        case BuiltinFn::kFrexp:
                        case BuiltinFn::kGatherCmp:
                        case BuiltinFn::kGather:
                        case BuiltinFn::kGatherAlpha:
                        case BuiltinFn::kGatherBlue:
                        case BuiltinFn::kGatherGreen:
                        case BuiltinFn::kGatherRed:
                        case BuiltinFn::kGetDimensions:
                        case BuiltinFn::kSample:
                        case BuiltinFn::kSampleBias:
                        case BuiltinFn::kSampleCmp:
                        case BuiltinFn::kSampleCmpLevelZero:
                        case BuiltinFn::kSampleGrad:
                        case BuiltinFn::kSampleLevel:
                        case BuiltinFn::kNone:
                            break;
                    }
                },
                TINT_ICE_ON_NO_MATCH);
        }
    }

    /// Loads the storage buffer dynamic offset from the uniform buffer.
    /// @returns the loaded dynamic offset value
    core::ir::Value* LoadDynamicOffset(uint32_t offset_index) {
        return b.Load(b.Access<ptr<uniform, u32>>(BufferOffsets(), u32(offset_index)))->Result();
    }

    /// Get (or create, on first call) the uniform buffer that contains the storage buffer offsets.
    /// @returns the uniform buffer pointer
    core::ir::Value* BufferOffsets() {
        if (buffer_sizes_var) {
            return buffer_sizes_var->Result();
        }

        // Find the largest index declared in the map, in order to determine the number of elements
        // needed in the array of buffer sizes.
        uint32_t max_index = 0;
        for (auto& entry : bindpoint_to_offset_index) {
            max_index = std::max(max_index, entry.second);
        }

        uint32_t num_elements = max_index + 1;
        b.Append(ir.root_block, [&] {
            buffer_sizes_var = b.Var("tint_storage_buffer_dynamic_offsets",
                                     ty.ptr<uniform>(ty.array(ty.u32(), num_elements)));
        });
        buffer_sizes_var->SetBindingPoint(ubo_binding.group, ubo_binding.binding);
        return buffer_sizes_var->Result();
    }
};

}  // namespace

Result<SuccessType> ArrayOffsetFromUniform(
    core::ir::Module& ir,
    BindingPoint ubo_binding,
    const std::unordered_map<BindingPoint, uint32_t>& bindpoint_to_offset_index) {
    AssertValid(ir, kArrayOffsetFromUniformCapabilities, "before hlsl.ArrayOffsetFromUniform");

    State state{ir, ubo_binding, bindpoint_to_offset_index};
    state.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
