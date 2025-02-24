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

#include "dawn/native/d3d11/BindGroupTrackerD3D11.h"

#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Range.h"
#include "dawn/native/Format.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BindGroupD3D11.h"
#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/native/d3d11/SamplerD3D11.h"
#include "dawn/native/d3d11/TextureD3D11.h"

namespace dawn::native::d3d11 {
namespace {

bool CheckAllSlotsAreEmpty(const ScopedSwapStateCommandRecordingContext* commandContext) {
    auto* deviceContext = commandContext->GetD3D11DeviceContext4();

    // Reserve one slot for builtin constants.
    constexpr uint32_t kReservedCBVSlots = 1;

    // Check constant buffer slots
    for (UINT slot = 0;
         slot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - kReservedCBVSlots; ++slot) {
        ID3D11Buffer* buffer = nullptr;
        deviceContext->VSGetConstantBuffers1(slot, 1, &buffer, nullptr, nullptr);
        DAWN_ASSERT(buffer == nullptr);
        deviceContext->PSGetConstantBuffers1(slot, 1, &buffer, nullptr, nullptr);
        DAWN_ASSERT(buffer == nullptr);
        deviceContext->CSGetConstantBuffers1(slot, 1, &buffer, nullptr, nullptr);
        DAWN_ASSERT(buffer == nullptr);
    }

    // Check resource slots
    for (UINT slot = 0; slot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++slot) {
        ID3D11ShaderResourceView* srv = nullptr;
        deviceContext->VSGetShaderResources(slot, 1, &srv);
        DAWN_ASSERT(srv == nullptr);
        deviceContext->PSGetShaderResources(slot, 1, &srv);
        DAWN_ASSERT(srv == nullptr);
        deviceContext->CSGetShaderResources(slot, 1, &srv);
        DAWN_ASSERT(srv == nullptr);
    }

    // Check sampler slots
    for (UINT slot = 0; slot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; ++slot) {
        ID3D11SamplerState* sampler = nullptr;
        deviceContext->VSGetSamplers(slot, 1, &sampler);
        DAWN_ASSERT(sampler == nullptr);
        deviceContext->PSGetSamplers(slot, 1, &sampler);
        DAWN_ASSERT(sampler == nullptr);
        deviceContext->CSGetSamplers(slot, 1, &sampler);
        DAWN_ASSERT(sampler == nullptr);
    }

    // Check UAV slots for compute
    for (UINT slot = 0; slot < D3D11_1_UAV_SLOT_COUNT; ++slot) {
        ID3D11UnorderedAccessView* uav = nullptr;
        deviceContext->CSGetUnorderedAccessViews(slot, 1, &uav);
        DAWN_ASSERT(uav == nullptr);
    }
    // Check UAV slots for render
    for (UINT slot = 0; slot < commandContext->GetDevice()->GetUAVSlotCount(); ++slot) {
        ID3D11UnorderedAccessView* uav = nullptr;
        deviceContext->OMGetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, slot, 1,
                                                                 &uav);
        DAWN_ASSERT(uav == nullptr);
    }

    return true;
}

void ResetAllRenderSlots(const ScopedSwapStateCommandRecordingContext* commandContext) {
    auto* deviceContext = commandContext->GetD3D11DeviceContext4();

    // Reserve one slot for builtin constants.
    constexpr uint32_t kReservedCBVSlots = 1;

    ID3D11Buffer* d3d11Buffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {};
    uint32_t num = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - kReservedCBVSlots;
    deviceContext->VSSetConstantBuffers1(0, num, d3d11Buffers, nullptr, nullptr);
    deviceContext->PSSetConstantBuffers1(0, num, d3d11Buffers, nullptr, nullptr);

    ID3D11ShaderResourceView* d3d11SRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
    num = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
    deviceContext->VSSetShaderResources(0, num, d3d11SRVs);
    deviceContext->PSSetShaderResources(0, num, d3d11SRVs);

    ID3D11SamplerState* d3d11Samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = {};
    num = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
    deviceContext->VSSetSamplers(0, num, d3d11Samplers);
    deviceContext->PSSetSamplers(0, num, d3d11Samplers);

    ID3D11UnorderedAccessView* d3d11UAVs[D3D11_1_UAV_SLOT_COUNT] = {};
    num = commandContext->GetDevice()->GetUAVSlotCount();
    deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(
        D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, 0, num, d3d11UAVs, nullptr);
}

}  // namespace

BindGroupTracker::BindGroupTracker(const ScopedSwapStateCommandRecordingContext* commandContext)
    : mCommandContext(commandContext) {
    mLastAppliedPipelineLayout = mCommandContext->GetDevice()->GetEmptyPipelineLayout();
}

BindGroupTracker::~BindGroupTracker() {
    DAWN_ASSERT(CheckAllSlotsAreEmpty(mCommandContext));
}

template <wgpu::ShaderStage kVisibleStage>
MaybeError BindGroupTracker::ApplyBindGroup(BindGroupIndex index) {
    constexpr wgpu::ShaderStage kVisibleFragment = wgpu::ShaderStage::Fragment & kVisibleStage;
    constexpr wgpu::ShaderStage kVisibleVertex = wgpu::ShaderStage::Vertex & kVisibleStage;
    constexpr wgpu::ShaderStage kVisibleCompute = wgpu::ShaderStage::Compute & kVisibleStage;

    auto* deviceContext = mCommandContext->GetD3D11DeviceContext4();
    BindGroupBase* group = mBindGroups[index];
    const ityp::vector<BindingIndex, uint64_t>& dynamicOffsets = mDynamicOffsets[index];
    const auto& indices = ToBackend(mPipelineLayout)->GetBindingIndexInfo()[index];

    for (BindingIndex bindingIndex : Range(group->GetLayout()->GetBindingCount())) {
        const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);
        const uint32_t bindingSlot = indices[bindingIndex];
        const auto bindingVisibility = bindingInfo.visibility & kVisibleStage;

        DAWN_TRY(MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) -> MaybeError {
                BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
                auto offset = binding.offset;
                if (layout.hasDynamicOffset) {
                    // Dynamic buffers are packed at the front of BindingIndices.
                    offset += dynamicOffsets[bindingIndex];
                }

                switch (layout.type) {
                    case wgpu::BufferBindingType::Uniform: {
                        ID3D11Buffer* d3d11Buffer;
                        DAWN_TRY_ASSIGN(d3d11Buffer, ToGPUUsableBuffer(binding.buffer)
                                                         ->GetD3D11ConstantBuffer(mCommandContext));
                        // https://learn.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-vssetconstantbuffers1
                        // Offset and size are measured in shader constants, which are 16 bytes
                        // (4*32-bit components). And the offsets and counts must be multiples
                        // of 16.
                        // WebGPU's minUniformBufferOffsetAlignment is 256.
                        DAWN_ASSERT(IsAligned(offset, 256));
                        uint32_t firstConstant = static_cast<uint32_t>(offset / 16);
                        uint32_t size = static_cast<uint32_t>(Align(binding.size, 16) / 16);
                        uint32_t numConstants = Align(size, 16);
                        DAWN_ASSERT(offset + numConstants * 16 <=
                                    binding.buffer->GetAllocatedSize());

                        if (bindingVisibility & kVisibleVertex) {
                            deviceContext->VSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                 &firstConstant, &numConstants);
                        }
                        if (bindingVisibility & kVisibleFragment) {
                            deviceContext->PSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                 &firstConstant, &numConstants);
                        }
                        if (bindingVisibility & kVisibleCompute) {
                            deviceContext->CSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                 &firstConstant, &numConstants);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::Storage:
                    case kInternalStorageBufferBinding: {
                        // Skip fragment on purpose because render passes requires a single
                        // OMSetRenderTargetsAndUnorderedAccessViews call to set all UAVs.
                        // Delegate to RenderPassBindGroupTracker::Apply.
                        if (bindingVisibility & wgpu::ShaderStage::Compute) {
                            ComPtr<ID3D11UnorderedAccessView> d3d11UAV;
                            DAWN_TRY_ASSIGN(d3d11UAV,
                                            ToGPUUsableBuffer(binding.buffer)
                                                ->UseAsUAV(mCommandContext, offset, binding.size));
                            deviceContext->CSSetUnorderedAccessViews(
                                bindingSlot, 1, d3d11UAV.GetAddressOf(), nullptr);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::ReadOnlyStorage: {
                        ComPtr<ID3D11ShaderResourceView> d3d11SRV;
                        DAWN_TRY_ASSIGN(d3d11SRV,
                                        ToGPUUsableBuffer(binding.buffer)
                                            ->UseAsSRV(mCommandContext, offset, binding.size));
                        if (bindingVisibility & kVisibleVertex) {
                            deviceContext->VSSetShaderResources(bindingSlot, 1,
                                                                d3d11SRV.GetAddressOf());
                        }
                        if (bindingVisibility & kVisibleFragment) {
                            deviceContext->PSSetShaderResources(bindingSlot, 1,
                                                                d3d11SRV.GetAddressOf());
                        }
                        if (bindingVisibility & kVisibleCompute) {
                            deviceContext->CSSetShaderResources(bindingSlot, 1,
                                                                d3d11SRV.GetAddressOf());
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::BindingNotUsed:
                    case wgpu::BufferBindingType::Undefined:
                        DAWN_UNREACHABLE();
                }
                return {};
            },
            [&](const StaticSamplerBindingInfo&) -> MaybeError {
                // Static samplers are implemented in the frontend on
                // D3D11.
                DAWN_UNREACHABLE();
                return {};
            },
            [&](const SamplerBindingInfo&) -> MaybeError {
                Sampler* sampler = ToBackend(group->GetBindingAsSampler(bindingIndex));
                ID3D11SamplerState* d3d11SamplerState = sampler->GetD3D11SamplerState();
                if (bindingVisibility & kVisibleVertex) {
                    deviceContext->VSSetSamplers(bindingSlot, 1, &d3d11SamplerState);
                }
                if (bindingVisibility & kVisibleFragment) {
                    deviceContext->PSSetSamplers(bindingSlot, 1, &d3d11SamplerState);
                }
                if (bindingVisibility & kVisibleCompute) {
                    deviceContext->CSSetSamplers(bindingSlot, 1, &d3d11SamplerState);
                }
                return {};
            },
            [&](const TextureBindingInfo&) -> MaybeError {
                TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                ComPtr<ID3D11ShaderResourceView> srv;
                // For sampling from stencil, we have to use an internal mirror 'R8Uint' texture.
                if (view->GetAspects() == Aspect::Stencil) {
                    DAWN_TRY_ASSIGN(
                        srv, ToBackend(view->GetTexture())->GetStencilSRV(mCommandContext, view));
                } else {
                    DAWN_TRY_ASSIGN(srv, view->GetOrCreateD3D11ShaderResourceView());
                }
                if (bindingVisibility & kVisibleVertex) {
                    deviceContext->VSSetShaderResources(bindingSlot, 1, srv.GetAddressOf());
                }
                if (bindingVisibility & kVisibleFragment) {
                    deviceContext->PSSetShaderResources(bindingSlot, 1, srv.GetAddressOf());
                }
                if (bindingVisibility & kVisibleCompute) {
                    deviceContext->CSSetShaderResources(bindingSlot, 1, srv.GetAddressOf());
                }
                return {};
            },
            [&](const StorageTextureBindingInfo& layout) -> MaybeError {
                TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                switch (layout.access) {
                    case wgpu::StorageTextureAccess::WriteOnly:
                    case wgpu::StorageTextureAccess::ReadWrite: {
                        // Skip fragment on purpose because render passes requires a single
                        // OMSetRenderTargetsAndUnorderedAccessViews call to set all UAVs.
                        // Delegate to RenderPassBindGroupTracker::Apply.
                        if (bindingVisibility & kVisibleCompute) {
                            ID3D11UnorderedAccessView* d3d11UAV = nullptr;
                            DAWN_TRY_ASSIGN(d3d11UAV, view->GetOrCreateD3D11UnorderedAccessView());
                            deviceContext->CSSetUnorderedAccessViews(bindingSlot, 1, &d3d11UAV,
                                                                     nullptr);
                        }
                        break;
                    }
                    case wgpu::StorageTextureAccess::ReadOnly: {
                        ID3D11ShaderResourceView* d3d11SRV = nullptr;
                        DAWN_TRY_ASSIGN(d3d11SRV, view->GetOrCreateD3D11ShaderResourceView());
                        if (bindingVisibility & kVisibleVertex) {
                            deviceContext->VSSetShaderResources(bindingSlot, 1, &d3d11SRV);
                        }
                        if (bindingVisibility & kVisibleFragment) {
                            deviceContext->PSSetShaderResources(bindingSlot, 1, &d3d11SRV);
                        }
                        if (bindingVisibility & kVisibleCompute) {
                            deviceContext->CSSetShaderResources(bindingSlot, 1, &d3d11SRV);
                        }
                        break;
                    }
                    default:
                        DAWN_UNREACHABLE();
                }
                return {};
            },
            [](const InputAttachmentBindingInfo&) -> MaybeError {
                DAWN_UNREACHABLE();
                return {};
            }));
    }
    return {};
}

const ScopedSwapStateCommandRecordingContext* BindGroupTracker::GetCommandContext() const {
    return mCommandContext.get();
}

ComputePassBindGroupTracker::ComputePassBindGroupTracker(
    const ScopedSwapStateCommandRecordingContext* commandContext)
    : BindGroupTracker(commandContext) {}

ComputePassBindGroupTracker::~ComputePassBindGroupTracker() {
    for (BindGroupIndex index :
         IterateBitSet(mLastAppliedPipelineLayout->GetBindGroupLayoutsMask())) {
        UnapplyComputeBindings(index);
    }
}

void ComputePassBindGroupTracker::UnapplyComputeBindings(BindGroupIndex index) {
    auto* deviceContext = GetCommandContext()->GetD3D11DeviceContext4();
    const BindGroupLayoutInternalBase* groupLayout =
        mLastAppliedPipelineLayout->GetBindGroupLayout(index);
    const auto& indices = ToBackend(mLastAppliedPipelineLayout)->GetBindingIndexInfo()[index];

    for (BindingIndex bindingIndex : Range(groupLayout->GetBindingCount())) {
        const BindingInfo& bindingInfo = groupLayout->GetBindingInfo(bindingIndex);
        const uint32_t bindingSlot = indices[bindingIndex];
        if (!(bindingInfo.visibility & wgpu::ShaderStage::Compute)) {
            continue;
        }

        MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                switch (layout.type) {
                    case wgpu::BufferBindingType::Uniform: {
                        ID3D11Buffer* nullBuffer = nullptr;
                        deviceContext->CSSetConstantBuffers1(bindingSlot, 1, &nullBuffer, nullptr,
                                                             nullptr);
                        break;
                    }
                    case wgpu::BufferBindingType::Storage:
                    case kInternalStorageBufferBinding: {
                        ID3D11UnorderedAccessView* nullUAV = nullptr;
                        deviceContext->CSSetUnorderedAccessViews(bindingSlot, 1, &nullUAV, nullptr);
                        break;
                    }
                    case wgpu::BufferBindingType::ReadOnlyStorage: {
                        ID3D11ShaderResourceView* nullSRV = nullptr;
                        deviceContext->CSSetShaderResources(bindingSlot, 1, &nullSRV);
                        break;
                    }
                    case wgpu::BufferBindingType::BindingNotUsed:
                    case wgpu::BufferBindingType::Undefined:
                        DAWN_UNREACHABLE();
                }
            },
            [&](const StaticSamplerBindingInfo&) {
                // Static samplers are implemented in the frontend on
                // D3D11.
                DAWN_UNREACHABLE();
            },
            [&](const SamplerBindingInfo&) {
                ID3D11SamplerState* nullSampler = nullptr;
                deviceContext->CSSetSamplers(bindingSlot, 1, &nullSampler);
            },
            [&](const TextureBindingInfo&) {
                ID3D11ShaderResourceView* nullSRV = nullptr;
                deviceContext->CSSetShaderResources(bindingSlot, 1, &nullSRV);
            },
            [&](const StorageTextureBindingInfo& layout) {
                switch (layout.access) {
                    case wgpu::StorageTextureAccess::WriteOnly:
                    case wgpu::StorageTextureAccess::ReadWrite: {
                        ID3D11UnorderedAccessView* nullUAV = nullptr;
                        deviceContext->CSSetUnorderedAccessViews(bindingSlot, 1, &nullUAV, nullptr);
                        break;
                    }
                    case wgpu::StorageTextureAccess::ReadOnly: {
                        ID3D11ShaderResourceView* nullSRV = nullptr;
                        deviceContext->CSSetShaderResources(bindingSlot, 1, &nullSRV);
                        break;
                    }
                    default:
                        DAWN_UNREACHABLE();
                }
            },
            [](const InputAttachmentBindingInfo&) { DAWN_UNREACHABLE(); });
    }
}

MaybeError ComputePassBindGroupTracker::Apply() {
    BeforeApply();

    BindGroupMask inheritedGroups =
        mPipelineLayout->InheritedGroupsMask(mLastAppliedPipelineLayout);
    BindGroupMask previousGroups = mLastAppliedPipelineLayout->GetBindGroupLayoutsMask();

    // To avoid UAV / SRV conflicts with bindings in previously bind groups, we unset the bind
    // groups that aren't reused by the current pipeline.
    // We also need to unset the inherited bind groups which are dirty as the group may have
    // both SRV and UAV, and the same resource may change its binding from UAV to SRV next
    // dispatch in the same group.
    //
    // Note: WebGPU API guarantees that resources are not used both as UAV and SRV in the same
    // render pass. So we don't need to do this inside render passes.
    BindGroupMask groupsToUnset = previousGroups & (~inheritedGroups | mDirtyBindGroups);
    for (BindGroupIndex index : IterateBitSet(groupsToUnset)) {
        UnapplyComputeBindings(index);
    }

    for (BindGroupIndex index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
        DAWN_TRY(ApplyBindGroup<wgpu::ShaderStage::Compute>(index));
    }

    AfterApply();

    return {};
}

RenderPassBindGroupTracker::RenderPassBindGroupTracker(
    const ScopedSwapStateCommandRecordingContext* commandContext,
    std::vector<ComPtr<ID3D11UnorderedAccessView>> pixelLocalStorageUAVs)
    : BindGroupTracker(commandContext), mPixelLocalStorageUAVs(std::move(pixelLocalStorageUAVs)) {}

RenderPassBindGroupTracker::~RenderPassBindGroupTracker() {
    ResetAllRenderSlots(GetCommandContext());
}

MaybeError RenderPassBindGroupTracker::Apply() {
    BeforeApply();

    // As D3d11 requires to bind all UAVs slots at the same time for pixel shaders, we record
    // all UAV slot assignments in the bind groups, and then bind them all together.
    // TODO(crbug.com/366291600): Clean up D3D11 logic, replace following getters with
    // GetUAVCount() and GetUAVStartSlot(). Clean up related validations.
    const BindGroupMask uavBindGroups = ToBackend(mPipelineLayout)->GetUAVBindGroupLayoutsMask();
    const uint32_t uavSlotCount = ToBackend(mPipelineLayout)->GetTotalUAVBindingCount();
    const uint32_t plsSlotCount = ToBackend(mPipelineLayout)->GetPLSSlotCount();
    const uint32_t unusedUavCount = ToBackend(mPipelineLayout)->GetUnusedUAVBindingCount();

    DAWN_ASSERT(uavSlotCount >= unusedUavCount + plsSlotCount);
    const uint32_t usedUavCount = uavSlotCount - unusedUavCount - plsSlotCount;

    const uint32_t uavStartSlot = unusedUavCount;
    std::vector<ComPtr<ID3D11UnorderedAccessView>> uavsInBindGroup(usedUavCount);

    for (BindGroupIndex index : IterateBitSet(uavBindGroups)) {
        BindGroupBase* group = mBindGroups[index];
        const ityp::vector<BindingIndex, uint64_t>& dynamicOffsets = mDynamicOffsets[index];
        const auto& indices = ToBackend(mPipelineLayout)->GetBindingIndexInfo()[index];

        // D3D11 uav slot allocated in reverse order.
        for (BindingIndex bindingIndex : Range(group->GetLayout()->GetBindingCount())) {
            const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);
            uint32_t pos = indices[bindingIndex] - uavStartSlot;
            DAWN_TRY(MatchVariant(
                bindingInfo.bindingLayout,
                [&](const BufferBindingInfo& layout) -> MaybeError {
                    BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
                    auto offset = binding.offset;
                    if (layout.hasDynamicOffset) {
                        // Dynamic buffers are packed at the front of BindingIndices.
                        offset += dynamicOffsets[bindingIndex];
                    }

                    switch (layout.type) {
                        case wgpu::BufferBindingType::Storage:
                        case kInternalStorageBufferBinding: {
                            ComPtr<ID3D11UnorderedAccessView> d3d11UAV;
                            DAWN_TRY_ASSIGN(d3d11UAV, ToGPUUsableBuffer(binding.buffer)
                                                          ->UseAsUAV(GetCommandContext(), offset,
                                                                     binding.size));
                            uavsInBindGroup[pos] = std::move(d3d11UAV);
                            break;
                        }
                        case wgpu::BufferBindingType::Uniform:
                        case wgpu::BufferBindingType::ReadOnlyStorage: {
                            break;
                        }
                        case wgpu::BufferBindingType::BindingNotUsed:
                        case wgpu::BufferBindingType::Undefined:
                            DAWN_UNREACHABLE();
                    }
                    return {};
                },
                [&](const StorageTextureBindingInfo& layout) -> MaybeError {
                    switch (layout.access) {
                        case wgpu::StorageTextureAccess::WriteOnly:
                        case wgpu::StorageTextureAccess::ReadWrite: {
                            ComPtr<ID3D11UnorderedAccessView> d3d11UAV;
                            TextureView* view =
                                ToBackend(group->GetBindingAsTextureView(bindingIndex));
                            DAWN_TRY_ASSIGN(d3d11UAV, view->GetOrCreateD3D11UnorderedAccessView());
                            uavsInBindGroup[pos] = std::move(d3d11UAV);
                            break;
                        }
                        case wgpu::StorageTextureAccess::ReadOnly:
                            break;
                        default:
                            DAWN_UNREACHABLE();
                            break;
                    }
                    return {};
                },
                [](const TextureBindingInfo&) -> MaybeError { return {}; },
                [](const SamplerBindingInfo&) -> MaybeError { return {}; },
                [](const StaticSamplerBindingInfo&) -> MaybeError {
                    // Static samplers are implemented in the frontend on
                    // D3D11.
                    DAWN_UNREACHABLE();
                    return {};
                },
                [](const InputAttachmentBindingInfo&) -> MaybeError {
                    DAWN_UNREACHABLE();
                    return {};
                }));
        }
    }

    const uint32_t plsCount = mPixelLocalStorageUAVs.size();
    const uint32_t plsAndUavCount = plsCount + usedUavCount;
    std::vector<ID3D11UnorderedAccessView*> plsAndUavs;
    plsAndUavs.reserve(plsAndUavCount);

    for (auto& uav : uavsInBindGroup) {
        plsAndUavs.push_back(uav.Get());
    }

    for (auto& uav : mPixelLocalStorageUAVs) {
        plsAndUavs.push_back(uav.Get());
    }

    if (!plsAndUavs.empty()) {
        GetCommandContext()->GetD3D11DeviceContext4()->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, uavStartSlot,
            plsAndUavCount, plsAndUavs.data(), nullptr);
    }

    for (BindGroupIndex index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
        DAWN_TRY(ApplyBindGroup<wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>(index));
    }

    AfterApply();

    return {};
}

}  // namespace dawn::native::d3d11
