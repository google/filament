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

#include "dawn/native/d3d12/BindGroupD3D12.h"

#include <utility>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Range.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/Queue.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/BufferD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/SamplerHeapCacheD3D12.h"
#include "dawn/native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/TextureD3D12.h"

namespace dawn::native::d3d12 {

// static
ResultOrError<Ref<BindGroup>> BindGroup::Create(Device* device,
                                                const BindGroupDescriptor* descriptor) {
    return ToBackend(descriptor->layout->GetInternalBindGroupLayout())
        ->AllocateBindGroup(device, descriptor);
}

BindGroup::BindGroup(Device* device,
                     const BindGroupDescriptor* descriptor,
                     const CPUDescriptorHeapAllocation& viewAllocation)
    : BindGroupBase(this, device, descriptor), mCPUViewAllocation(viewAllocation) {}

BindGroup::~BindGroup() = default;

MaybeError BindGroup::InitializeImpl() {
    BindGroupLayout* bgl = ToBackend(GetLayout());

    const auto& descriptorHeapOffsets = bgl->GetDescriptorHeapOffsets();
    uint32_t viewSizeIncrement = bgl->GetViewSizeIncrement();

    Device* device = ToBackend(GetDevice());
    ID3D12Device* d3d12Device = device->GetD3D12Device();

    // It's not necessary to create descriptors in the descriptor heap for dynamic resources.
    // This is because they are created as root descriptors which are never heap allocated.
    // Since dynamic buffers are packed in the front, we can skip over these bindings by
    // starting from the dynamic buffer count.
    for (BindingIndex bindingIndex : Range(bgl->GetDynamicBufferCount(), bgl->GetBindingCount())) {
        const BindingInfo& bindingInfo = bgl->GetBindingInfo(bindingIndex);

        // Increment size does not need to be stored and is only used to get a handle
        // local to the allocation with OffsetFrom().
        MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                BufferBinding binding = GetBindingAsBufferBinding(bindingIndex);

                ID3D12Resource* resource = ToBackend(binding.buffer)->GetD3D12Resource();
                if (resource == nullptr) {
                    // The Buffer was destroyed. Skip creating buffer views since there is no
                    // resource. This bind group won't be used as it is an error to submit a
                    // command buffer that references destroyed resources.
                    return;
                }

                switch (layout.type) {
                    case wgpu::BufferBindingType::Uniform: {
                        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
                        desc.SizeInBytes =
                            Align(binding.size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
                        desc.BufferLocation = ToBackend(binding.buffer)->GetVA() + binding.offset;

                        d3d12Device->CreateConstantBufferView(
                            &desc, mCPUViewAllocation.OffsetFrom(
                                       viewSizeIncrement, descriptorHeapOffsets[bindingIndex]));
                        break;
                    }
                    case wgpu::BufferBindingType::Storage:
                    case kInternalStorageBufferBinding: {
                        // Since Tint outputs HLSL shaders with RWByteAddressBuffer,
                        // we must use D3D12_BUFFER_UAV_FLAG_RAW when making the
                        // UNORDERED_ACCESS_VIEW_DESC. Using D3D12_BUFFER_UAV_FLAG_RAW requires
                        // that we use DXGI_FORMAT_R32_TYPELESS as the format of the view.
                        // DXGI_FORMAT_R32_TYPELESS requires that the element size be 4
                        // byte aligned. Since binding.size and binding.offset are in bytes,
                        // we need to divide by 4 to obtain the element size.
                        D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
                        desc.Buffer.NumElements = binding.size / 4;
                        desc.Format = DXGI_FORMAT_R32_TYPELESS;
                        desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                        desc.Buffer.FirstElement = binding.offset / 4;
                        desc.Buffer.StructureByteStride = 0;
                        desc.Buffer.CounterOffsetInBytes = 0;
                        desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

                        d3d12Device->CreateUnorderedAccessView(
                            resource, nullptr, &desc,
                            mCPUViewAllocation.OffsetFrom(viewSizeIncrement,
                                                          descriptorHeapOffsets[bindingIndex]));
                        break;
                    }
                    case wgpu::BufferBindingType::ReadOnlyStorage:
                    case kInternalReadOnlyStorageBufferBinding: {
                        // Like StorageBuffer, Tint outputs HLSL shaders for readonly
                        // storage buffer with ByteAddressBuffer. So we must use
                        // D3D12_BUFFER_SRV_FLAG_RAW when making the SRV descriptor. And it has
                        // similar requirement for format, element size, etc.
                        D3D12_SHADER_RESOURCE_VIEW_DESC desc;
                        desc.Format = DXGI_FORMAT_R32_TYPELESS;
                        desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                        desc.Buffer.FirstElement = binding.offset / 4;
                        desc.Buffer.NumElements = binding.size / 4;
                        desc.Buffer.StructureByteStride = 0;
                        desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
                        d3d12Device->CreateShaderResourceView(
                            resource, &desc,
                            mCPUViewAllocation.OffsetFrom(viewSizeIncrement,
                                                          descriptorHeapOffsets[bindingIndex]));
                        break;
                    }
                    case wgpu::BufferBindingType::BindingNotUsed:
                    case wgpu::BufferBindingType::Undefined:
                        DAWN_UNREACHABLE();
                }
            },
            [&](const TextureBindingInfo&) {
                auto* view = ToBackend(GetBindingAsTextureView(bindingIndex));
                auto& srv = view->GetSRVDescriptor();

                ID3D12Resource* resource = ToBackend(view->GetTexture())->GetD3D12Resource();
                if (resource == nullptr) {
                    // The Texture was destroyed. Skip creating the SRV since there is no
                    // resource. This bind group won't be used as it is an error to submit a
                    // command buffer that references destroyed resources.
                    return;
                }

                d3d12Device->CreateShaderResourceView(
                    resource, &srv,
                    mCPUViewAllocation.OffsetFrom(viewSizeIncrement,
                                                  descriptorHeapOffsets[bindingIndex]));
            },
            [&](const StorageTextureBindingInfo& layout) {
                TextureView* view = ToBackend(GetBindingAsTextureView(bindingIndex));

                ID3D12Resource* resource = ToBackend(view->GetTexture())->GetD3D12Resource();
                if (resource == nullptr) {
                    // The Texture was destroyed. Skip creating the SRV/UAV since there is no
                    // resource. This bind group won't be used as it is an error to submit a
                    // command buffer that references destroyed resources.
                    return;
                }

                switch (layout.access) {
                    case wgpu::StorageTextureAccess::WriteOnly:
                    case wgpu::StorageTextureAccess::ReadWrite: {
                        D3D12_UNORDERED_ACCESS_VIEW_DESC uav = view->GetUAVDescriptor();
                        d3d12Device->CreateUnorderedAccessView(
                            resource, nullptr, &uav,
                            mCPUViewAllocation.OffsetFrom(viewSizeIncrement,
                                                          descriptorHeapOffsets[bindingIndex]));
                        break;
                    }
                    case wgpu::StorageTextureAccess::ReadOnly: {
                        D3D12_SHADER_RESOURCE_VIEW_DESC srv = view->GetSRVDescriptor();
                        d3d12Device->CreateShaderResourceView(
                            resource, &srv,
                            mCPUViewAllocation.OffsetFrom(viewSizeIncrement,
                                                          descriptorHeapOffsets[bindingIndex]));
                        break;
                    }
                    case wgpu::StorageTextureAccess::BindingNotUsed:
                    case wgpu::StorageTextureAccess::Undefined:
                        DAWN_UNREACHABLE();
                }
            },
            [](const StaticSamplerBindingInfo&) {
                // Static samplers are already initialized in the pipeline layout.
            },
            // No-op as samplers will be later initialized by CreateSamplers().
            [](const SamplerBindingInfo&) {},
            [](const InputAttachmentBindingInfo&) { DAWN_UNREACHABLE(); });
    }

    // Loop through the dynamic storage buffers and build a flat map from the index of the
    // dynamic storage buffer to its binding size. The index |dynamicStorageBufferIndex|
    // means that it is the i'th buffer that is both dynamic and storage, in increasing order
    // of BindingNumber.
    mDynamicStorageBufferLengths.resize(bgl->GetBindingCountInfo().dynamicStorageBufferCount);
    uint32_t dynamicStorageBufferIndex = 0;
    for (BindingIndex bindingIndex(0); bindingIndex < bgl->GetDynamicBufferCount();
         ++bindingIndex) {
        if (bgl->IsStorageBufferBinding(bindingIndex)) {
            mDynamicStorageBufferLengths[dynamicStorageBufferIndex++] =
                GetBindingAsBufferBinding(bindingIndex).size;
        }
    }

    return {};
}

void BindGroup::DestroyImpl() {
    BindGroupBase::DestroyImpl();
    ToBackend(GetLayout())->DeallocateDescriptor(&mCPUViewAllocation);
    DAWN_ASSERT(!mCPUViewAllocation.IsValid());
}

void BindGroup::DeleteThis() {
    // This function must first run the destructor and then deallocate memory. Take a reference to
    // the BindGroupLayout+SlabAllocator before running the destructor so this function can access
    // it afterwards and it's not destroyed prematurely.
    Ref<BindGroupLayout> layout = ToBackend(GetLayout());
    BindGroupBase::DeleteThis();
    layout->DeallocateBindGroup(this);
}

bool BindGroup::PopulateViews(MutexProtected<ShaderVisibleDescriptorAllocator>& viewAllocator) {
    const BindGroupLayout* bgl = ToBackend(GetLayout());

    const uint32_t descriptorCount = bgl->GetCbvUavSrvDescriptorCount();
    if (descriptorCount == 0 || viewAllocator->IsAllocationStillValid(mGPUViewAllocation)) {
        return true;
    }

    // Attempt to allocate descriptors for the currently bound shader-visible heaps.
    // If either failed, return early to re-allocate and switch the heaps.
    Device* device = ToBackend(GetDevice());

    D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptor;
    if (!viewAllocator->AllocateGPUDescriptors(descriptorCount,
                                               device->GetQueue()->GetPendingCommandSerial(),
                                               &baseCPUDescriptor, &mGPUViewAllocation)) {
        return false;
    }

    // CPU bindgroups are sparsely allocated across CPU heaps. Instead of doing
    // simple copies per bindgroup, a single non-simple copy could be issued.
    // TODO(dawn:155): Consider doing this optimization.
    device->GetD3D12Device()->CopyDescriptorsSimple(descriptorCount, baseCPUDescriptor,
                                                    mCPUViewAllocation.GetBaseDescriptor(),
                                                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    return true;
}

D3D12_GPU_DESCRIPTOR_HANDLE BindGroup::GetBaseViewDescriptor() const {
    return mGPUViewAllocation.GetBaseDescriptor();
}

D3D12_GPU_DESCRIPTOR_HANDLE BindGroup::GetBaseSamplerDescriptor() const {
    DAWN_ASSERT(mSamplerAllocationEntry != nullptr);
    return mSamplerAllocationEntry->GetBaseDescriptor();
}

bool BindGroup::PopulateSamplers(
    MutexProtected<ShaderVisibleDescriptorAllocator>& samplerAllocator) {
    if (mSamplerAllocationEntry == nullptr) {
        return true;
    }
    return mSamplerAllocationEntry->Populate(samplerAllocator);
}

void BindGroup::SetSamplerAllocationEntry(Ref<SamplerHeapCacheEntry> entry) {
    mSamplerAllocationEntry = std::move(entry);
}

const BindGroup::DynamicStorageBufferLengths& BindGroup::GetDynamicStorageBufferLengths() const {
    return mDynamicStorageBufferLengths;
}

}  // namespace dawn::native::d3d12
