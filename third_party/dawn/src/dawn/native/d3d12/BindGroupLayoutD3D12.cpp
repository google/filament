// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"

#include <utility>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/SamplerD3D12.h"
#include "dawn/native/d3d12/SamplerHeapCacheD3D12.h"
#include "dawn/native/d3d12/StagingDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"

namespace dawn::native::d3d12 {
namespace {
D3D12_DESCRIPTOR_RANGE_TYPE WGPUBindingInfoToDescriptorRangeType(const BindingInfo& bindingInfo) {
    return MatchVariant(
        bindingInfo.bindingLayout,
        [](const BufferBindingInfo& layout) -> D3D12_DESCRIPTOR_RANGE_TYPE {
            switch (layout.type) {
                case wgpu::BufferBindingType::Uniform:
                    return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                case wgpu::BufferBindingType::Storage:
                case kInternalStorageBufferBinding:
                    return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                case wgpu::BufferBindingType::ReadOnlyStorage:
                    return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                case wgpu::BufferBindingType::BindingNotUsed:
                case wgpu::BufferBindingType::Undefined:
                    DAWN_UNREACHABLE();
            }
        },
        [](const StaticSamplerBindingInfo&) -> D3D12_DESCRIPTOR_RANGE_TYPE {
            return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        },
        [](const SamplerBindingInfo&) -> D3D12_DESCRIPTOR_RANGE_TYPE {
            return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        },
        [](const TextureBindingInfo&) -> D3D12_DESCRIPTOR_RANGE_TYPE {
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        },
        [](const StorageTextureBindingInfo& layout) -> D3D12_DESCRIPTOR_RANGE_TYPE {
            switch (layout.access) {
                case wgpu::StorageTextureAccess::WriteOnly:
                case wgpu::StorageTextureAccess::ReadWrite:
                    return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                case wgpu::StorageTextureAccess::ReadOnly:
                    return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                case wgpu::StorageTextureAccess::BindingNotUsed:
                case wgpu::StorageTextureAccess::Undefined:
                    DAWN_UNREACHABLE();
            }
        },
        [](const InputAttachmentBindingInfo&) -> D3D12_DESCRIPTOR_RANGE_TYPE {
            DAWN_UNREACHABLE();
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        });
}
}  // anonymous namespace

// static
Ref<BindGroupLayout> BindGroupLayout::Create(Device* device,
                                             const BindGroupLayoutDescriptor* descriptor) {
    return AcquireRef(new BindGroupLayout(device, descriptor));
}

BindGroupLayout::BindGroupLayout(Device* device, const BindGroupLayoutDescriptor* descriptor)
    : BindGroupLayoutInternalBase(device, descriptor),
      mDescriptorHeapOffsets(GetBindingCount()),
      mShaderRegisters(GetBindingCount()),
      mCbvUavSrvDescriptorCount(0),
      mSamplerDescriptorCount(0),
      mBindGroupAllocator(MakeFrontendBindGroupAllocator<BindGroup>(4096)) {
    for (BindingIndex bindingIndex{0}; bindingIndex < GetBindingCount(); ++bindingIndex) {
        const BindingInfo& bindingInfo = GetBindingInfo(bindingIndex);

        D3D12_DESCRIPTOR_RANGE_TYPE descriptorRangeType =
            WGPUBindingInfoToDescriptorRangeType(bindingInfo);
        mShaderRegisters[bindingIndex] = uint32_t(bindingInfo.binding);

        // Static samplers aren't stored in the descriptor heap. Handle them separately.
        if (std::holds_alternative<StaticSamplerBindingInfo>(bindingInfo.bindingLayout)) {
            const StaticSamplerBindingInfo& staticSamplerBindingInfo =
                std::get<StaticSamplerBindingInfo>(bindingInfo.bindingLayout);

            Sampler* sampler = ToBackend(staticSamplerBindingInfo.sampler.Get());

            const D3D12_SAMPLER_DESC desc = sampler->GetSamplerDescriptor();
            D3D12_STATIC_SAMPLER_DESC staticSamplerDesc = {};
            staticSamplerDesc.ShaderRegister = GetShaderRegister(bindingIndex);
            staticSamplerDesc.RegisterSpace = kRegisterSpacePlaceholder;
            staticSamplerDesc.ShaderVisibility = ShaderVisibilityType(bindingInfo.visibility);
            staticSamplerDesc.AddressU = desc.AddressU;
            staticSamplerDesc.AddressV = desc.AddressV;
            staticSamplerDesc.AddressW = desc.AddressW;
            staticSamplerDesc.Filter = desc.Filter;
            staticSamplerDesc.MinLOD = desc.MinLOD;
            staticSamplerDesc.MaxLOD = desc.MaxLOD;
            staticSamplerDesc.MipLODBias = desc.MipLODBias;
            staticSamplerDesc.MaxAnisotropy = desc.MaxAnisotropy;
            staticSamplerDesc.ComparisonFunc = desc.ComparisonFunc;

            mStaticSamplers.push_back(staticSamplerDesc);

            continue;
        }

        // For dynamic resources, Dawn uses root descriptor in D3D12 backend. So there is no
        // need to allocate the descriptor from descriptor heap or create descriptor ranges.
        if (bindingIndex < GetDynamicBufferCount()) {
            continue;
        }
        DAWN_ASSERT(!std::holds_alternative<BufferBindingInfo>(bindingInfo.bindingLayout) ||
                    !std::get<BufferBindingInfo>(bindingInfo.bindingLayout).hasDynamicOffset);

        mDescriptorHeapOffsets[bindingIndex] =
            descriptorRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER
                ? mSamplerDescriptorCount++
                : mCbvUavSrvDescriptorCount++;

        D3D12_DESCRIPTOR_RANGE1 range;
        range.RangeType = descriptorRangeType;
        range.NumDescriptors = 1;
        range.BaseShaderRegister = GetShaderRegister(bindingIndex);
        range.RegisterSpace = kRegisterSpacePlaceholder;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        // In Dawn we always use the descriptors as static ones, which means the descriptors in a
        // descriptor heap pointed to by a root descriptor table have been initialized by the time
        // the descriptor table is set on a command list (during recording), and the descriptors
        // cannot be changed until the command list has finished executing for the last time, so we
        // don't need to set DESCRIPTORS_VOLATILE for any binding types.
        range.Flags = MatchVariant(
            bindingInfo.bindingLayout,
            [](const StaticSamplerBindingInfo&) -> D3D12_DESCRIPTOR_RANGE_FLAGS {
                // Static samplers should already be handled. This should never be reached.
                DAWN_UNREACHABLE();
                return D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
            },
            [](const SamplerBindingInfo&) -> D3D12_DESCRIPTOR_RANGE_FLAGS {
                // Sampler descriptor ranges don't support DATA_* flags at all since samplers do not
                // point to data.
                return D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
            },
            [&](const BufferBindingInfo&) -> D3D12_DESCRIPTOR_RANGE_FLAGS {
                // In Dawn it's allowed to do state transitions on the buffers or textures after
                // binding them on the current command list, which indicates a change to its data
                // (or possibly resource metadata), so we cannot bind them as DATA_STATIC. We cannot
                // bind them as DATA_STATIC_WHILE_SET_AT_EXECUTE either because it is required to be
                // rebound to the command list before the next (this) Draw/Dispatch call, while
                // currently we may not rebind these resources if the current bind group is not
                // changed.
                if (GetDevice()->IsRobustnessEnabled()) {
                    return D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS |
                           D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
                }
                return D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
            },
            [](const TextureBindingInfo&) -> D3D12_DESCRIPTOR_RANGE_FLAGS {
                return D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
            },
            [](const StorageTextureBindingInfo&) -> D3D12_DESCRIPTOR_RANGE_FLAGS {
                return D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
            },
            [](const InputAttachmentBindingInfo&) -> D3D12_DESCRIPTOR_RANGE_FLAGS {
                DAWN_UNREACHABLE();
                return D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
            });

        std::vector<D3D12_DESCRIPTOR_RANGE1>& descriptorRanges =
            descriptorRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER ? mSamplerDescriptorRanges
                                                                       : mCbvUavSrvDescriptorRanges;

        // Try to join this range with the previous one, if the current range is a continuation
        // of the previous. This is possible because the binding infos in the base type are
        // sorted.
        if (descriptorRanges.size() >= 2) {
            D3D12_DESCRIPTOR_RANGE1& previous = descriptorRanges.back();
            if (previous.RangeType == range.RangeType &&
                previous.BaseShaderRegister + previous.NumDescriptors == range.BaseShaderRegister) {
                previous.NumDescriptors += range.NumDescriptors;
                continue;
            }
        }

        descriptorRanges.push_back(range);
    }
}

ResultOrError<Ref<BindGroup>> BindGroupLayout::AllocateBindGroup(
    Device* device,
    const BindGroupDescriptor* descriptor) {
    uint32_t viewSizeIncrement = 0;
    CPUDescriptorHeapAllocation viewAllocation;
    if (GetCbvUavSrvDescriptorCount() > 0) {
        auto* viewAllocator =
            device->GetViewStagingDescriptorAllocator(GetCbvUavSrvDescriptorCount());
        DAWN_ASSERT(viewAllocator != nullptr);
        DAWN_TRY((*viewAllocator).Use([&](auto viewAllocator) -> MaybeError {
            DAWN_TRY_ASSIGN(viewAllocation, viewAllocator->AllocateCPUDescriptors());
            viewSizeIncrement = viewAllocator->GetSizeIncrement();
            return {};
        }));
    }

    Ref<BindGroup> bindGroup = AcquireRef<BindGroup>(
        mBindGroupAllocator->Allocate(device, descriptor, viewSizeIncrement, viewAllocation));

    if (GetSamplerDescriptorCount() > 0) {
        Ref<SamplerHeapCacheEntry> samplerHeapCacheEntry;
        DAWN_TRY_ASSIGN(samplerHeapCacheEntry,
                        device->GetSamplerHeapCache()->GetOrCreate(bindGroup.Get()));
        bindGroup->SetSamplerAllocationEntry(std::move(samplerHeapCacheEntry));
    }

    return bindGroup;
}

void BindGroupLayout::DeallocateBindGroup(BindGroup* bindGroup,
                                          CPUDescriptorHeapAllocation* viewAllocation) {
    if (viewAllocation->IsValid()) {
        Device* device = ToBackend(bindGroup->GetDevice());
        auto* viewAllocator =
            device->GetViewStagingDescriptorAllocator(GetCbvUavSrvDescriptorCount());
        (*viewAllocator)->Deallocate(viewAllocation);
    }

    mBindGroupAllocator->Deallocate(bindGroup);
}

ityp::span<BindingIndex, const uint32_t> BindGroupLayout::GetDescriptorHeapOffsets() const {
    return {mDescriptorHeapOffsets.data(), mDescriptorHeapOffsets.size()};
}

uint32_t BindGroupLayout::GetShaderRegister(BindingIndex bindingIndex) const {
    return mShaderRegisters[bindingIndex];
}

uint32_t BindGroupLayout::GetCbvUavSrvDescriptorCount() const {
    return mCbvUavSrvDescriptorCount;
}

uint32_t BindGroupLayout::GetSamplerDescriptorCount() const {
    return mSamplerDescriptorCount;
}

const std::vector<D3D12_DESCRIPTOR_RANGE1>& BindGroupLayout::GetCbvUavSrvDescriptorRanges() const {
    return mCbvUavSrvDescriptorRanges;
}

const std::vector<D3D12_DESCRIPTOR_RANGE1>& BindGroupLayout::GetSamplerDescriptorRanges() const {
    return mSamplerDescriptorRanges;
}

const std::vector<D3D12_STATIC_SAMPLER_DESC>& BindGroupLayout::GetStaticSamplers() const {
    return mStaticSamplers;
}

}  // namespace dawn::native::d3d12
