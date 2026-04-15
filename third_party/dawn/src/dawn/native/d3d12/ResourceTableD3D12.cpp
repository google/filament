// Copyright 2026 The Dawn & Tint Authors
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

#include "dawn/native/d3d12/ResourceTableD3D12.h"

#include <utility>
#include <vector>

#include "dawn/common/Enumerator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Range.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/Queue.h"
#include "dawn/native/ResourceTableDefaultResources.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/StagingDescriptorAllocatorD3D12.h"

namespace dawn::native::d3d12 {

// static
std::vector<D3D12_DESCRIPTOR_RANGE1> ResourceTable::GetCbvUavSrvDescriptorRanges(
    const PipelineLayout& layout) {
    // For SM 6.5-, we need to create one descriptor table with multiple overlapping ranges,
    // each in its own register space, per resource type. This is because HLSL does not allow
    // overlapping register ranges, and we need separate unbounded array types for each resource
    // type: HLSL example:
    //    Texture2D<float4> TextureTable1[] : register(t0, space1);
    //    Texture2D<uint4> TextureTable2[] : register(t0, space2);
    //    ...
    //    float4 color = TextureTable1[MyTextureIndex + 1].Load(int3(0,0,0));
    //    uint4 data = TextureTable2[MyTextureIndex + 2].Load(int3(0,0,0));
    //
    // For SM 6.6+, we don't need any descriptor tables, but only a single shader-visible
    // descriptor heap to hold the resources, and shaders access the heap via built-in
    // ResourceDescriptorHeap. HLSL example:
    //    Texture2D<float4> myTex = ResourceDescriptorHeap[MyTextureIndex + 1];
    //    Texture2D<uint4> myTex = ResourceDescriptorHeap[MyTextureIndex + 2];
    //
    // TODO(crbug.com/480110521): Add support for the SM >= 6.6 path

    std::vector<D3D12_DESCRIPTOR_RANGE1> ranges;

    const uint32_t baseRegisterSpace = layout.GetBaseResourceTableRegisterSpace();
    const uint32_t defaultResourceCount =
        static_cast<uint32_t>(ResourceTableDefaultResources::GetCount());

    // The metadata storage buffer is bound to (kBaseResourceTableRegisterSpace, 0)
    ranges.push_back(D3D12_DESCRIPTOR_RANGE1{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 0,
        .RegisterSpace = baseRegisterSpace,
        .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,
        .OffsetInDescriptorsFromTableStart = 0,  // Only one in this space
    });

    // Create multiple overlapping ranges, one per resource type,
    // bound to (1 + baseRegisterSpace + i, 0)
    for (uint32_t i : Range(defaultResourceCount)) {
        ranges.push_back(D3D12_DESCRIPTOR_RANGE1{
            .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            .NumDescriptors = kMaxResourceTableSize + defaultResourceCount,
            // HLSL doesn't allow overlapping register ranges, so each one is in its own space
            // (group), and starts at register (binding) 0 with no other range bound after it.
            .BaseShaderRegister = 0,
            .RegisterSpace = 1 + baseRegisterSpace + i,
            // Volatile required for bindless
            .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,
            // Force the same offset in the descriptor table to overlaps these ranges
            .OffsetInDescriptorsFromTableStart = 1,
        });
    }

    return ranges;
}

// static
ResultOrError<Ref<ResourceTable>> ResourceTable::Create(Device* device,
                                                        const ResourceTableDescriptor* descriptor) {
    Ref<ResourceTable> table = AcquireRef(new ResourceTable(device, descriptor));
    DAWN_TRY(table->Initialize());
    return table;
}

ResourceTable::~ResourceTable() = default;

MaybeError ResourceTable::Initialize() {
    DAWN_TRY(ResourceTableBase::InitializeBase());

    Device* device = ToBackend(GetDevice());
    ID3D12Device* d3d12Device = device->GetD3D12Device();

    // Allocate the CPU descriptor heap
    const uint32_t descriptorCount = 1u + static_cast<uint32_t>(GetSizeWithDefaultResources());
    DAWN_TRY(AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorCount));

    // Only write the metadata buffer to the heap initially, all the other bindings will be written
    // as needed when they are inserted in the ResourceTable.
    Buffer* metadataBuffer = ToBackend(GetMetadataBuffer());
    ID3D12Resource* resource = metadataBuffer->GetD3D12Resource();
    DAWN_ASSERT(resource != nullptr);

    // Like StorageBuffer, Tint outputs HLSL shaders for readonly storage buffer with
    // ByteAddressBuffer. So we must use D3D12_BUFFER_SRV_FLAG_RAW when making the SRV
    // descriptor. And it has similar requirement for format, element size, etc.
    D3D12_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = metadataBuffer->GetSize() / 4;
    desc.Buffer.StructureByteStride = 0;
    desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

    uint32_t offsetInDescriptorCount = 0;  // Metadata buffer is the first element in the table
    d3d12Device->CreateShaderResourceView(
        resource, &desc,
        mCPUViewAllocation.OffsetFrom(mViewSizeIncrement, offsetInDescriptorCount));

    return {};
}

MaybeError ResourceTable::AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType,
                                          uint32_t descriptorCount) {
    DAWN_ASSERT(!mCPUHeap);

    Device* device = ToBackend(GetDevice());
    ID3D12Device* d3d12Device = device->GetD3D12Device();

    D3D12_DESCRIPTOR_HEAP_DESC heapDescriptor;
    heapDescriptor.Type = heapType;
    heapDescriptor.NumDescriptors = descriptorCount;
    heapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDescriptor.NodeMask = 0;

    ComPtr<ID3D12DescriptorHeap> heap;
    DAWN_TRY(CheckHRESULT(d3d12Device->CreateDescriptorHeap(&heapDescriptor, IID_PPV_ARGS(&heap)),
                          "ID3D12Device::CreateDescriptorHeap"));

    const D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptor = {
        heap->GetCPUDescriptorHandleForHeapStart().ptr};

    mCPUHeap = std::move(heap);
    mCPUViewAllocation = CPUDescriptorHeapAllocation{baseCPUDescriptor, 0};
    mViewSizeIncrement = d3d12Device->GetDescriptorHandleIncrementSize(heapType);

    return {};
}

void ResourceTable::FreeCPUHeap() {
    mCPUHeap.Reset();
    mCPUViewAllocation.Invalidate();
    mViewSizeIncrement = 0;
}

// Apply updates to resources or to the metadata buffers that are pending.
MaybeError ResourceTable::ApplyPendingUpdates(CommandRecordingContext* recordingContext) {
    Updates updates = AcquireDirtySlotUpdates();

    if (!updates.metadataUpdates.empty()) {
        DAWN_TRY(UpdateMetadataBuffer(recordingContext, updates.metadataUpdates));
    }
    if (!updates.resourceUpdates.empty()) {
        DAWN_TRY(UpdateResourceBindings(updates.resourceUpdates));
    }

    return {};
}

MaybeError ResourceTable::UpdateMetadataBuffer(CommandRecordingContext* recordingContext,
                                               const std::vector<MetadataUpdate>& updates) {
    Device* device = ToBackend(GetDevice());

    // Allocate enough space for all the data to modify and schedule the copies.
    return device->GetDynamicUploader()->WithUploadReservation(
        sizeof(uint32_t) * updates.size(), kCopyBufferToBufferOffsetAlignment,
        [&](UploadReservation reservation) -> MaybeError {
            uint32_t* stagedData = static_cast<uint32_t*>(reservation.mappedPointer);

            // The metadata buffer will be copied to.
            Buffer* metadataBuffer = ToBackend(GetMetadataBuffer());
            DAWN_ASSERT(metadataBuffer->IsInitialized());
            auto scopedUseMetadataBuffer = metadataBuffer->UseInternal();
            metadataBuffer->TrackUsageAndTransitionNow(recordingContext,
                                                       wgpu::BufferUsage::CopyDst);

            // Record a CopyBufferRegion for each update
            // TODO(crbug.com/473354062): reduce number of calls by copying contiguous regions
            for (auto [i, update] : Enumerate(updates)) {
                stagedData[i] = update.data;  // Copy to staged
                // Copy staged to metadata buffer
                recordingContext->GetCommandList1()->CopyBufferRegion(
                    metadataBuffer->GetD3D12Resource(), update.offset,
                    ToBackend(reservation.buffer)->GetD3D12Resource(),
                    reservation.offsetInBuffer + i * sizeof(uint32_t), sizeof(uint32_t));
            }

            // Transition the buffer back to be used as storage as that's how it will be used for
            // shader-side validation.
            metadataBuffer->TrackUsageAndTransitionNow(recordingContext, kReadOnlyStorageBuffer);

            return {};
        });
}

MaybeError ResourceTable::UpdateResourceBindings(const std::vector<ResourceUpdate>& updates) {
    Device* device = ToBackend(GetDevice());
    ID3D12Device* d3d12Device = device->GetD3D12Device();

    for (const ResourceUpdate& update : updates) {
        // TODO(https://issues.chromium.org/473444515): Support buffer, texel buffers and storage
        // textures.

        MatchVariant(
            update.resource,
            [&](TextureViewBase* textureView) {
                auto* view = ToBackend(textureView);
                ID3D12Resource* resource = ToBackend(view->GetTexture())->GetD3D12Resource();
                if (resource == nullptr) {
                    // Skip resource if it was destroyed
                    return;
                }

                // Add 1 to skip metadata descriptor
                uint32_t offsetInDescriptorCount = 1 + static_cast<uint32_t>(update.slot);

                d3d12Device->CreateShaderResourceView(
                    resource, &view->GetSRVDescriptor(),
                    mCPUViewAllocation.OffsetFrom(mViewSizeIncrement, offsetInDescriptorCount));
            },
            [&](SamplerBase* sampler) {
                // TODO(https://issues.chromium.org/473354063): Support samplers updates.
                // Skip for now to allow most e2e tests to pass when attempting to add default
                // samplers.
            });
    }

    return {};
}

bool ResourceTable::PopulateViews(ShaderVisibleDescriptorAllocator* viewAllocator) {
    if (viewAllocator->IsAllocationStillValid(mGPUViewAllocation)) {
        return true;
    }

    // Attempt to allocate descriptors for the currently bound shader-visible heaps.
    // Return false if allocation fails to indicate that AllocateAndSwitchShaderVisibleHeap should
    // be called.
    Device* device = ToBackend(GetDevice());

    const uint32_t descriptorCount = GetViewDescriptorCount();

    D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptor;
    if (!viewAllocator->AllocateGPUDescriptors(GetViewDescriptorCount(),
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

uint32_t ResourceTable::GetViewDescriptorCount() const {
    // Metadata + all resource descriptors
    return 1u + static_cast<uint32_t>(GetSizeWithDefaultResources());
}

D3D12_GPU_DESCRIPTOR_HANDLE ResourceTable::GetBaseViewDescriptor() const {
    return mGPUViewAllocation.GetBaseDescriptor();
}

void ResourceTable::DestroyImpl(DestroyReason reason) {
    ResourceTableBase::DestroyImpl(reason);
    FreeCPUHeap();
}

void ResourceTable::SetLabelImpl() {
    // TODO(crbug.com/473354062): SetDebugName
}

}  // namespace dawn::native::d3d12
