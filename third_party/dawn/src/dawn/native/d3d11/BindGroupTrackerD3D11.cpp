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

#include "src/dawn/native/d3d11/BindGroupTrackerD3D11.h"

#include <algorithm>
#include <bit>
#include <tuple>
#include <utility>
#include <vector>

#include "src/dawn/common/MatchVariant.h"
#include "src/dawn/common/Range.h"
#include "src/dawn/native/Format.h"
#include "src/dawn/native/d3d/D3DError.h"
#include "src/dawn/native/d3d11/BindGroupD3D11.h"
#include "src/dawn/native/d3d11/BufferD3D11.h"
#include "src/dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "src/dawn/native/d3d11/DeviceD3D11.h"
#include "src/dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "src/dawn/native/d3d11/SamplerD3D11.h"
#include "src/dawn/native/d3d11/TextureD3D11.h"
#include "src/utils/assert.h"
#include "src/utils/numeric.h"

namespace dawn::native::d3d11 {
namespace {

constexpr auto kVertex = SingleShaderStage::Vertex;
constexpr auto kFragment = SingleShaderStage::Fragment;
constexpr auto kCompute = SingleShaderStage::Compute;

bool CheckAllSlotsAreEmpty(const ScopedSwapStateCommandRecordingContext* commandContext) {
    auto* deviceContext = commandContext->GetD3D11DeviceContext3();

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

BufferBinding ExtractBufferBindingInfo(BindGroupBase* group,
                                       BindingIndex bindingIndex,
                                       const BufferBindingInfo& layout,
                                       const ityp::span<BindingIndex, uint32_t>& dynamicOffsets) {
    BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
    if (layout.hasDynamicOffset) {
        binding.offset += dynamicOffsets[bindingIndex];
    }

    return binding;
}

template <SingleShaderStage Stage, typename... Args>
void SetConstantBuffersImpl(ID3D11DeviceContext3* deviceContext, Args&&... args) {
    if constexpr (Stage == kVertex) {
        deviceContext->VSSetConstantBuffers1(std::forward<Args>(args)...);
    } else if constexpr (Stage == kFragment) {
        deviceContext->PSSetConstantBuffers1(std::forward<Args>(args)...);
    } else if constexpr (Stage == kCompute) {
        deviceContext->CSSetConstantBuffers1(std::forward<Args>(args)...);
    }
}

template <SingleShaderStage Stage, typename... Args>
void SetShaderResourcesImpl(ID3D11DeviceContext3* deviceContext, Args&&... args) {
    if constexpr (Stage == kVertex) {
        deviceContext->VSSetShaderResources(std::forward<Args>(args)...);
    } else if constexpr (Stage == kFragment) {
        deviceContext->PSSetShaderResources(std::forward<Args>(args)...);
    } else if constexpr (Stage == kCompute) {
        deviceContext->CSSetShaderResources(std::forward<Args>(args)...);
    }
}

template <SingleShaderStage Stage, typename... Args>
void SetSamplersImpl(ID3D11DeviceContext3* deviceContext, Args&&... args) {
    if constexpr (Stage == kVertex) {
        deviceContext->VSSetSamplers(std::forward<Args>(args)...);
    } else if constexpr (Stage == kFragment) {
        deviceContext->PSSetSamplers(std::forward<Args>(args)...);
    } else if constexpr (Stage == kCompute) {
        deviceContext->CSSetSamplers(std::forward<Args>(args)...);
    }
}

template <SingleShaderStage Stage, typename... Args>
void SetUnorderedAccessViewsImpl(ID3D11DeviceContext3* deviceContext, Args&&... args) {
    if constexpr (Stage == kFragment) {
        deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr,
            std::forward<Args>(args)..., nullptr);
    } else if constexpr (Stage == kCompute) {
        deviceContext->CSSetUnorderedAccessViews(std::forward<Args>(args)..., nullptr);
    } else {
        DAWN_UNREACHABLE();
    }
}

}  // namespace

template <typename Key, uint32_t InitialCapacity>
template <typename Fn>
void BindGroupTracker::BindSlotCache<Key, InitialCapacity>::Bind(uint32_t idx,
                                                                 Key key,
                                                                 Fn&& bindFunc) {
    if (MaxBoundSlots() <= idx) {
        mBoundKeys.resize(idx + 1, Key{});
    }

    if (mBoundKeys[idx] == key) {
        // redundant binding, return
        return;
    }

    bindFunc();

    mBoundKeys[idx] = key;
}

BindGroupTracker::BindGroupTracker(const ScopedSwapStateCommandRecordingContext* commandContext)
    : mCommandContext(commandContext) {}

PipelineLayoutBase* BindGroupTracker::LastAppliedPipelineLayout() const {
    return mLastAppliedPipeline ? mLastAppliedPipeline->GetLayout()
                                : mCommandContext->GetDevice()->GetEmptyPipelineLayout();
}

BindGroupTracker::~BindGroupTracker() {
    UnbindConstantBuffers<kVertex>();
    UnbindConstantBuffers<kFragment>();
    UnbindConstantBuffers<kCompute>();

    UnbindShaderResources<kVertex>();
    UnbindShaderResources<kFragment>();
    UnbindShaderResources<kCompute>();

    UnbindSamplers<kVertex>();
    UnbindSamplers<kFragment>();
    UnbindSamplers<kCompute>();

    UnbindUnorderedAccessViews<kFragment>();
    UnbindUnorderedAccessViews<kCompute>();

    DAWN_ASSERT(CheckAllSlotsAreEmpty(mCommandContext));
}

template <SingleShaderStage Stage>
void BindGroupTracker::SetConstantBuffer(uint32_t idx,
                                         ID3D11Buffer* d3d11Buffer,
                                         uint32_t firstConstant,
                                         uint32_t numConstants) {
    CBufferBindingKey key{std::bit_cast<uintptr_t>(d3d11Buffer), firstConstant, numConstants};
    mConstantBufferSlots[Stage].Bind(idx, key, [&] {
        SetConstantBuffersImpl<Stage>(mCommandContext->GetD3D11DeviceContext3(), idx, 1u,
                                      &d3d11Buffer, &firstConstant, &numConstants);
    });
}

template <SingleShaderStage Stage>
void BindGroupTracker::UnbindConstantBuffers() {
    const auto slots = mConstantBufferSlots[Stage].MaxBoundSlots();
    if (slots == 0) {
        return;
    }

    static constexpr ID3D11Buffer* kNullBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] =
        {};

    SetConstantBuffersImpl<Stage>(mCommandContext->GetD3D11DeviceContext3(), 0u, slots,
                                  kNullBuffers, nullptr, nullptr);
}

template <SingleShaderStage Stage>
void BindGroupTracker::SetShaderResource(uint32_t idx, ID3D11ShaderResourceView* srv) {
    mSRVSlots[Stage].Bind(idx, std::bit_cast<uintptr_t>(srv), [&] {
        SetShaderResourcesImpl<Stage>(mCommandContext->GetD3D11DeviceContext3(), idx, 1u, &srv);
    });
}

template <SingleShaderStage Stage>
void BindGroupTracker::UnbindShaderResources() {
    const auto slots = mSRVSlots[Stage].MaxBoundSlots();
    if (slots == 0) {
        return;
    }

    static constexpr ID3D11ShaderResourceView*
        kNullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};

    SetShaderResourcesImpl<Stage>(mCommandContext->GetD3D11DeviceContext3(), 0u, slots, kNullSRVs);
}

template <SingleShaderStage Stage>
void BindGroupTracker::SetSampler(uint32_t idx, ID3D11SamplerState* sampler) {
    mSamplerSlots[Stage].Bind(idx, std::bit_cast<uintptr_t>(sampler), [&] {
        SetSamplersImpl<Stage>(mCommandContext->GetD3D11DeviceContext3(), idx, 1u, &sampler);
    });
}

template <SingleShaderStage Stage>
void BindGroupTracker::UnbindSamplers() {
    const auto slots = mSamplerSlots[Stage].MaxBoundSlots();
    if (slots == 0) {
        return;
    }

    static constexpr ID3D11SamplerState* kNullSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = {};

    SetSamplersImpl<Stage>(mCommandContext->GetD3D11DeviceContext3(), 0u, slots, kNullSamplers);
}

void BindGroupTracker::CSSetUnorderedAccessView(uint32_t idx, ID3D11UnorderedAccessView* uav) {
    mMinUAVSlots[kCompute] = std::min(mMinUAVSlots[kCompute], idx);

    mCSUAVSlots.Bind(idx, std::bit_cast<uintptr_t>(uav), [&] {
        SetUnorderedAccessViewsImpl<kCompute>(mCommandContext->GetD3D11DeviceContext3(), idx, 1u,
                                              &uav);
    });
}

void BindGroupTracker::OMSetUnorderedAccessViews(uint32_t idx,
                                                 uint32_t count,
                                                 ID3D11UnorderedAccessView* const* uavs) {
    mMinUAVSlots[kFragment] = std::min(mMinUAVSlots[kFragment], idx);
    mPSMaxUAVSlot = std::max(mPSMaxUAVSlot, idx + count);

    SetUnorderedAccessViewsImpl<kFragment>(mCommandContext->GetD3D11DeviceContext3(), idx, count,
                                           uavs);
}

template <SingleShaderStage Stage>
void BindGroupTracker::UnbindUnorderedAccessViews() {
    const uint32_t start = mMinUAVSlots[Stage];
    uint32_t end;

    if constexpr (Stage == kFragment) {
        end = mPSMaxUAVSlot;
    } else if constexpr (Stage == kCompute) {
        end = mCSUAVSlots.MaxBoundSlots();
    } else {
        DAWN_UNREACHABLE();
        return;
    }

    if (start >= end) {
        return;
    }

    static constexpr ID3D11UnorderedAccessView* kNullUAVs[D3D11_1_UAV_SLOT_COUNT] = {};

    const auto count = end - start;

    SetUnorderedAccessViewsImpl<Stage>(mCommandContext->GetD3D11DeviceContext3(), start, count,
                                       kNullUAVs);
}

ResultOrError<std::tuple<ID3D11Buffer*, UINT, UINT>> BindGroupTracker::GetConstantBufferBinding(
    BindGroupBase* group,
    BindingIndex bindingIndex,
    const BufferBindingInfo& layout,
    const ityp::span<BindingIndex, uint32_t>& dynamicOffsets) {
    BufferBinding binding = ExtractBufferBindingInfo(group, bindingIndex, layout, dynamicOffsets);

    ID3D11Buffer* d3d11Buffer;
    DAWN_TRY_ASSIGN(d3d11Buffer,
                    ToGPUUsableBuffer(binding.buffer)->GetD3D11ConstantBuffer(mCommandContext));
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-vssetconstantbuffers1
    // Offset and size are measured in shader constants, which are 16 bytes
    // (4*32-bit components). And the offsets and counts must be multiples
    // of 16.
    // WebGPU's minUniformBufferOffsetAlignment is 256.
    DAWN_ASSERT(IsAligned(binding.offset, 256));
    UINT firstConstant = static_cast<UINT>(binding.offset / 16);
    UINT size = static_cast<UINT>(Align(binding.size, 16) / 16);
    UINT numConstants = Align(size, 16);
    DAWN_ASSERT(binding.offset + uint64_t{numConstants} * 16 <= binding.buffer->GetAllocatedSize());

    return std::tuple{d3d11Buffer, firstConstant, numConstants};
}

ID3D11SamplerState* BindGroupTracker::GetSamplerState(BindGroupBase* group,
                                                      BindingIndex bindingIndex) {
    const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);
    DAWN_ASSERT(std::holds_alternative<SamplerBindingInfo>(bindingInfo.bindingLayout));

    Sampler* sampler = ToBackend(group->GetBindingAsSampler(bindingIndex));
    return sampler->GetD3D11SamplerState();
}

template <wgpu::ShaderStage kVisibleStage>
MaybeError BindGroupTracker::ApplyBindGroup(BindGroupIndex index) {
    constexpr wgpu::ShaderStage kVisibleFragment = wgpu::ShaderStage::Fragment & kVisibleStage;
    constexpr wgpu::ShaderStage kVisibleVertex = wgpu::ShaderStage::Vertex & kVisibleStage;
    constexpr wgpu::ShaderStage kVisibleCompute = wgpu::ShaderStage::Compute & kVisibleStage;

    BindGroupBase* group = mBindGroups[index];
    const ityp::span<BindingIndex, uint32_t>& dynamicOffsets = GetDynamicOffsets(index);
    const auto& indices = ToBackend(mPipeline->GetLayout())->GetBindingTableIndexMap()[index];

    for (BindingIndex bindingIndex : Range(group->GetLayout()->GetBindingCount())) {
        const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);
        const auto bindingVisibility = bindingInfo.visibility & kVisibleStage;

        uint32_t bindingSlotVS;
        uint32_t bindingSlotPS;
        uint32_t bindingSlotCS;
        if (bindingVisibility & kVisibleVertex) {
            bindingSlotVS = indices[bindingIndex][kVertex];
        }
        if (bindingVisibility & kVisibleFragment) {
            bindingSlotPS = indices[bindingIndex][kFragment];
        }
        if (bindingVisibility & kVisibleCompute) {
            bindingSlotCS = indices[bindingIndex][kCompute];
        }

        DAWN_TRY(MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) -> MaybeError {
                switch (layout.type) {
                    case wgpu::BufferBindingType::Uniform: {
                        ID3D11Buffer* d3d11Buffer;
                        UINT firstConstant;
                        UINT numConstants;
                        DAWN_TRY_ASSIGN(std::tie(d3d11Buffer, firstConstant, numConstants),
                                        this->GetConstantBufferBinding(group, bindingIndex, layout,
                                                                       dynamicOffsets));

                        if (bindingVisibility & kVisibleVertex) {
                            this->SetConstantBuffer<kVertex>(bindingSlotVS, d3d11Buffer,
                                                             firstConstant, numConstants);
                        }
                        if (bindingVisibility & kVisibleFragment) {
                            this->SetConstantBuffer<kFragment>(bindingSlotPS, d3d11Buffer,
                                                               firstConstant, numConstants);
                        }
                        if (bindingVisibility & kVisibleCompute) {
                            this->SetConstantBuffer<kCompute>(bindingSlotCS, d3d11Buffer,
                                                              firstConstant, numConstants);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::Storage:
                    case kInternalStorageBufferBinding: {
                        // Skip fragment on purpose because render passes requires a single
                        // OMSetRenderTargetsAndUnorderedAccessViews call to set all UAVs.
                        // Delegate to RenderPassBindGroupTracker::Apply.
                        if (bindingVisibility & wgpu::ShaderStage::Compute) {
                            BufferBinding binding = ExtractBufferBindingInfo(
                                group, bindingIndex, layout, dynamicOffsets);
                            DAWN_TRY(ToGPUUsableBuffer(binding.buffer)
                                         ->UseAsUAV(mCommandContext, binding.offset, binding.size,
                                                    [&](ID3D11UnorderedAccessView* uav) {
                                                        this->CSSetUnorderedAccessView(
                                                            bindingSlotCS, uav);
                                                    }));
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::ReadOnlyStorage:
                    case kInternalReadOnlyStorageBufferBinding: {
                        BufferBinding binding =
                            ExtractBufferBindingInfo(group, bindingIndex, layout, dynamicOffsets);
                        DAWN_TRY(
                            ToGPUUsableBuffer(binding.buffer)
                                ->UseAsSRV(
                                    mCommandContext, binding.offset, binding.size,
                                    [&](ID3D11ShaderResourceView* srv) {
                                        if (bindingVisibility & kVisibleVertex) {
                                            this->SetShaderResource<kVertex>(bindingSlotVS, srv);
                                        }
                                        if (bindingVisibility & kVisibleFragment) {
                                            this->SetShaderResource<kFragment>(bindingSlotPS, srv);
                                        }
                                        if (bindingVisibility & kVisibleCompute) {
                                            this->SetShaderResource<kCompute>(bindingSlotCS, srv);
                                        }
                                    }));
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
                ID3D11SamplerState* d3d11SamplerState = GetSamplerState(group, bindingIndex);
                if (bindingVisibility & kVisibleVertex) {
                    this->SetSampler<kVertex>(bindingSlotVS, d3d11SamplerState);
                }
                if (bindingVisibility & kVisibleFragment) {
                    this->SetSampler<kFragment>(bindingSlotPS, d3d11SamplerState);
                }
                if (bindingVisibility & kVisibleCompute) {
                    this->SetSampler<kCompute>(bindingSlotCS, d3d11SamplerState);
                }
                return {};
            },
            [&](const TextureBindingInfo&) -> MaybeError {
                TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                ID3D11ShaderResourceView* srv;
                DAWN_TRY_ASSIGN(srv, view->GetOrCreateD3D11ShaderResourceView(mCommandContext));

                if (bindingVisibility & kVisibleVertex) {
                    this->SetShaderResource<kVertex>(bindingSlotVS, srv);
                }
                if (bindingVisibility & kVisibleFragment) {
                    this->SetShaderResource<kFragment>(bindingSlotPS, srv);
                }
                if (bindingVisibility & kVisibleCompute) {
                    this->SetShaderResource<kCompute>(bindingSlotCS, srv);
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
                            ID3D11UnorderedAccessView* d3d11UAV;
                            DAWN_TRY_ASSIGN(d3d11UAV, view->GetOrCreateD3D11UnorderedAccessView());
                            this->CSSetUnorderedAccessView(bindingSlotCS, d3d11UAV);
                        }
                        break;
                    }
                    case wgpu::StorageTextureAccess::ReadOnly: {
                        ID3D11ShaderResourceView* d3d11SRV;
                        DAWN_TRY_ASSIGN(d3d11SRV,
                                        view->GetOrCreateD3D11ShaderResourceView(mCommandContext));
                        if (bindingVisibility & kVisibleVertex) {
                            this->SetShaderResource<kVertex>(bindingSlotVS, d3d11SRV);
                        }
                        if (bindingVisibility & kVisibleFragment) {
                            this->SetShaderResource<kFragment>(bindingSlotPS, d3d11SRV);
                        }
                        if (bindingVisibility & kVisibleCompute) {
                            this->SetShaderResource<kCompute>(bindingSlotCS, d3d11SRV);
                        }
                        break;
                    }
                    default:
                        DAWN_UNREACHABLE();
                }
                return {};
            },
            [&](const TexelBufferBindingInfo&) -> MaybeError {
                // D3D11 does not support texel buffers.
                // TODO(crbug/382544164): Prototype texel buffer feature
                DAWN_UNREACHABLE();
                return {};
            },
            [](const InputAttachmentBindingInfo&) -> MaybeError { DAWN_UNREACHABLE(); },
            [](const ExternalTextureBindingInfo&) -> MaybeError { DAWN_UNREACHABLE(); }));
    }
    return {};
}

ComputePassBindGroupTracker::ComputePassBindGroupTracker(
    const ScopedSwapStateCommandRecordingContext* commandContext)
    : BindGroupTracker(commandContext) {}

ComputePassBindGroupTracker::~ComputePassBindGroupTracker() = default;

void ComputePassBindGroupTracker::UnapplyComputeBindings(BindGroupIndex index) {
    PipelineLayoutBase* lastAppliedLayout = LastAppliedPipelineLayout();
    const BindGroupLayoutInternalBase* groupLayout = lastAppliedLayout->GetBindGroupLayout(index);
    const auto& indices = ToBackend(lastAppliedLayout)->GetBindingTableIndexMap()[index];

    for (BindingIndex bindingIndex : Range(groupLayout->GetBindingCount())) {
        const BindingInfo& bindingInfo = groupLayout->GetBindingInfo(bindingIndex);
        const uint32_t bindingSlot = indices[bindingIndex][kCompute];
        if (!(bindingInfo.visibility & wgpu::ShaderStage::Compute)) {
            continue;
        }

        MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                switch (layout.type) {
                    case wgpu::BufferBindingType::Uniform: {
                        this->SetConstantBuffer<kCompute>(bindingSlot, nullptr, 0u, 0u);
                        break;
                    }
                    case wgpu::BufferBindingType::Storage:
                    case kInternalStorageBufferBinding: {
                        this->CSSetUnorderedAccessView(bindingSlot, nullptr);
                        break;
                    }
                    case wgpu::BufferBindingType::ReadOnlyStorage:
                    case kInternalReadOnlyStorageBufferBinding: {
                        this->SetShaderResource<kCompute>(bindingSlot, nullptr);
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
            [&](const SamplerBindingInfo&) { this->SetSampler<kCompute>(bindingSlot, nullptr); },
            [&](const TextureBindingInfo&) {
                this->SetShaderResource<kCompute>(bindingSlot, nullptr);
            },
            [&](const StorageTextureBindingInfo& layout) {
                switch (layout.access) {
                    case wgpu::StorageTextureAccess::WriteOnly:
                    case wgpu::StorageTextureAccess::ReadWrite: {
                        this->CSSetUnorderedAccessView(bindingSlot, nullptr);
                        break;
                    }
                    case wgpu::StorageTextureAccess::ReadOnly: {
                        this->SetShaderResource<kCompute>(bindingSlot, nullptr);
                        break;
                    }
                    default:
                        DAWN_UNREACHABLE();
                }
            },
            [&](const TexelBufferBindingInfo&) {
                // D3D11 does not support texel buffers.
                // TODO(crbug/382544164): Prototype texel buffer feature
                DAWN_UNREACHABLE();
            },
            [](const InputAttachmentBindingInfo&) { DAWN_UNREACHABLE(); },
            [](const ExternalTextureBindingInfo&) { DAWN_UNREACHABLE(); });
    }
}

MaybeError ComputePassBindGroupTracker::Apply() {
    BeforeApply();

    PipelineLayoutBase* lastAppliedLayout = LastAppliedPipelineLayout();
    BindGroupMask inheritedGroups = mPipeline->GetLayout()->InheritedGroupsMask(lastAppliedLayout);
    BindGroupMask previousGroups = lastAppliedLayout->GetBindGroupLayoutsMask();

    // To avoid UAV / SRV conflicts with bindings in previously bind groups, we unset the bind
    // groups that aren't reused by the current pipeline.
    // We also need to unset the inherited bind groups which are dirty as the group may have
    // both SRV and UAV, and the same resource may change its binding from UAV to SRV next
    // dispatch in the same group.
    //
    // Note: WebGPU API guarantees that resources are not used both as UAV and SRV in the same
    // render pass. So we don't need to do this inside render passes.
    BindGroupMask groupsToUnset = previousGroups & (~inheritedGroups | mDirtyBindGroups);
    for (BindGroupIndex index : groupsToUnset) {
        UnapplyComputeBindings(index);
    }

    for (BindGroupIndex index : mDirtyBindGroupsObjectChangedOrIsDynamic) {
        DAWN_TRY(ApplyBindGroup<wgpu::ShaderStage::Compute>(index));
    }

    AfterApply();

    return {};
}

RenderPassBindGroupTracker::RenderPassBindGroupTracker(
    const ScopedSwapStateCommandRecordingContext* commandContext,
    std::vector<ComPtr<ID3D11UnorderedAccessView>> pixelLocalStorageUAVs)
    : BindGroupTracker(commandContext), mPixelLocalStorageUAVs(std::move(pixelLocalStorageUAVs)) {}

RenderPassBindGroupTracker::~RenderPassBindGroupTracker() = default;

MaybeError RenderPassBindGroupTracker::Apply() {
    BeforeApply();

    if (!mDirtyBindGroupsObjectChangedOrIsDynamic.any() &&
        LastAppliedPipelineLayout() == mPipeline->GetLayout()) {
        AfterApply();
        return {};
    }

    auto* pipelineLayout = ToBackend(mPipeline->GetLayout());

    // As D3d11 requires to bind all UAVs slots at the same time for pixel shaders, we record
    // all UAV slot assignments in the bind groups, and then bind them all together.
    const BindGroupMask uavBindGroups = pipelineLayout->GetUAVBindGroupLayoutsMask();
    const uint32_t plsSlotCount = pipelineLayout->GetPLSSlotCount();
    const uint32_t uavStartSlot = pipelineLayout->GetUAVStartIndex(kFragment);
    const uint32_t uavCount = pipelineLayout->GetUAVCount(kFragment);

    DAWN_ASSERT(uavCount >= plsSlotCount);
    const uint32_t usedUavCount = uavCount - plsSlotCount;

    absl::InlinedVector<ComPtr<ID3D11UnorderedAccessView>, 1> uavsInBindGroup(usedUavCount);

    for (BindGroupIndex index : uavBindGroups) {
        BindGroupBase* group = mBindGroups[index];
        const ityp::span<BindingIndex, uint32_t>& dynamicOffsets = GetDynamicOffsets(index);
        const auto& indices = pipelineLayout->GetBindingTableIndexMap()[index];

        // D3D11 uav slot allocated in reverse order.
        for (BindingIndex bindingIndex : Range(group->GetLayout()->GetBindingCount())) {
            const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);

            // Skip if this binding isn't visible in the fragment shader.
            if (!(bindingInfo.visibility & wgpu::ShaderStage::Fragment)) {
                continue;
            }

            uint32_t pos = indices[bindingIndex][kFragment] - uavStartSlot;
            DAWN_TRY(MatchVariant(
                bindingInfo.bindingLayout,
                [&](const BufferBindingInfo& layout) -> MaybeError {
                    switch (layout.type) {
                        case wgpu::BufferBindingType::Storage:
                        case kInternalStorageBufferBinding: {
                            BufferBinding binding = ExtractBufferBindingInfo(
                                group, bindingIndex, layout, dynamicOffsets);
                            DAWN_TRY(ToGPUUsableBuffer(binding.buffer)
                                         ->UseAsUAV(GetCommandContext(), binding.offset,
                                                    binding.size,
                                                    [&](ID3D11UnorderedAccessView* uav) {
                                                        uavsInBindGroup[pos] = uav;
                                                    }));
                            break;
                        }
                        case wgpu::BufferBindingType::Uniform:
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                        case kInternalReadOnlyStorageBufferBinding: {
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
                            TextureView* view =
                                ToBackend(group->GetBindingAsTextureView(bindingIndex));
                            ID3D11UnorderedAccessView* d3d11UAV;
                            DAWN_TRY_ASSIGN(d3d11UAV, view->GetOrCreateD3D11UnorderedAccessView());
                            uavsInBindGroup[pos] = d3d11UAV;
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
                [](const TexelBufferBindingInfo&) -> MaybeError {
                    // D3D11 does not support texel buffers.
                    // TODO(crbug/382544164): Prototype texel buffer feature
                    DAWN_UNREACHABLE();
                    return {};
                },
                [](const InputAttachmentBindingInfo&) -> MaybeError { DAWN_UNREACHABLE(); },
                [](const ExternalTextureBindingInfo&) -> MaybeError { DAWN_UNREACHABLE(); }));
        }
    }

    const uint32_t plsCount = static_cast<uint32_t>(mPixelLocalStorageUAVs.size());
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
        this->OMSetUnorderedAccessViews(uavStartSlot, plsAndUavCount, plsAndUavs.data());
    }

    for (BindGroupIndex index : mDirtyBindGroupsObjectChangedOrIsDynamic) {
        DAWN_TRY(ApplyBindGroup<wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>(index));
    }

    AfterApply();

    return {};
}

}  // namespace dawn::native::d3d11
