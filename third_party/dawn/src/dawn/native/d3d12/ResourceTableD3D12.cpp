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

#include "src/dawn/native/d3d12/ResourceTableD3D12.h"

#include <utility>
#include <vector>

#include "src/dawn/common/Enumerator.h"
#include "src/dawn/common/MatchVariant.h"
#include "src/dawn/native/DynamicUploader.h"
#include "src/dawn/native/Queue.h"
#include "src/dawn/native/ResourceTableDefaultResources.h"
#include "src/dawn/native/d3d/D3DError.h"
#include "src/dawn/native/d3d12/DeviceD3D12.h"
#include "src/dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "src/dawn/native/d3d12/SamplerD3D12.h"
#include "src/dawn/native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "src/dawn/native/d3d12/StagingDescriptorAllocatorD3D12.h"
#include "src/dawn/native/d3d12/TextureD3D12.h"
#include "src/utils/compiler.h"

namespace dawn::native::d3d12 {

// SamplerIndexPool manages a limited pool of sampler indices (e.g. 2048 on D3D12). It also
// implements deduplication by returning the same sampler index for an input Sampler pointer,
// internally tracking ref counts to return the index when all refs to the same Sampler are
// released. Note that this assumes that the same Sampler pointer is returned for the same
// SamplerDescriptor.
class ResourceTable::SamplerIndexPool {
  public:
    using SamplerIndex = TypedInteger<struct SamplerIndexT, uint16_t>;

    explicit SamplerIndexPool(uint32_t numSamplers) {
        // Init unused sampler indices with 0..numSamplers-1
        mUnused.resize(numSamplers);
        for (uint16_t i = 0; i < mUnused.size(); ++i) {
            mUnused[i] = SamplerIndex{i};
        }
    }

    // Acquire a sampler index for the input sampler. Returns the index, and 'true' if this is a
    // newly acquired sampler index, 'false' if it has already been acquired before for the same
    // Sampler*.
    ResultOrError<std::pair<SamplerIndex, bool>> Acquire(raw_ptr<const SamplerBase> sampler) {
        auto [iter, inserted] = mSamplerToEntry.try_emplace(reinterpret_cast<uintptr_t>(&*sampler));
        SamplerEntry& entry = iter->second;
        if (inserted) {
            // Grab a new one
            if (mUnused.empty()) {
                // TODO(crbug.com/512089609): Make this an Internal error as this is unrecoverable.
                return DAWN_VALIDATION_ERROR("No more unique sampler indices available");
            }
            entry.index = mUnused.back();
            mUnused.pop_back();
            DAWN_ASSERT(entry.refCount == 0);
        }
        ++entry.refCount;
        return std::make_pair(entry.index, inserted);
    }

    // Release the sampler index for the input sampler. If it is the last one, it the sampler index
    // is returned to the pool.
    void Release(raw_ptr<const SamplerBase> sampler) {
        // If this is the last ref to this sampler, return the index to the pool
        auto iter = mSamplerToEntry.find(reinterpret_cast<uintptr_t>(&*sampler));
        DAWN_ASSERT(iter != mSamplerToEntry.end());
        auto& entry = iter->second;
        DAWN_ASSERT(entry.refCount > 0);
        if (--entry.refCount == 0) {
            mUnused.push_back(entry.index);
            mSamplerToEntry.erase(iter);
        }
    }

  private:
    // Unused (available) sampler indices, initialized with range [0, numSamplers-1]
    std::vector<SamplerIndex> mUnused;

    // Mapping of unique sampler to its index and its reference count (number of slots using it).
    // We use a uintptr_t instead of Sampler* to ensure no dereference is ever attempted.
    // Note that if a given sampler pointer is released, then acquired in the same update, it is
    // possible to get back a different sampler index. This should not pose a problem.
    struct SamplerEntry {
        SamplerIndex index;
        uint32_t refCount;
    };
    absl::flat_hash_map<uintptr_t, SamplerEntry> mSamplerToEntry;
};

// static
ResultOrError<ResourceTable::DescriptorRanges> ResourceTable::GetDescriptorRanges(
    const PipelineLayout& layout) {
    // For SM 6.5-, we need to create one descriptor table with multiple overlapping ranges,
    // each in its own register space (group), per resource type. This is because HLSL does not
    // allow overlapping register ranges, and we need separate unbounded array types for each
    // resource type: HLSL example:
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

    DescriptorRanges ranges;

    const uint32_t baseRegisterSpace = layout.GetBaseResourceTableRegisterSpace();

    ityp::span<ResourceTableSlot, const tint::ResourceType> defaultResourceOrder =
        ResourceTableDefaultResources::GetOrder();

    const uint32_t defaultNonSamplersCount =
        uint32_t{ResourceTableDefaultResources::GetNonSamplerCount()};

    // The metadata storage buffer is bound to (kBaseResourceTableRegisterSpace, 0)
    ranges.cbvUavSrvs.push_back(D3D12_DESCRIPTOR_RANGE1{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 0,
        .RegisterSpace = baseRegisterSpace,
        .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,
        // Metadata is at register (binding) 0, the rest of the arrays in this range overlap
        // starting at register (binding) 1.
        .OffsetInDescriptorsFromTableStart = 0,
    });

    // Create multiple overlapping ranges for default resources, one per type, bound to
    // (1 + baseRegisterSpace + i, 0)
    for (auto [i, resourceType] : Enumerate(defaultResourceOrder)) {
        std::vector<D3D12_DESCRIPTOR_RANGE1>* range = nullptr;
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType{};
        UINT numDescriptors = 0;
        UINT offsetFromTableStart = 0;

        if (!tint::IsSampler(resourceType)) {
            range = &ranges.cbvUavSrvs;
            rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            numDescriptors = kMaxResourceTableSize + defaultNonSamplersCount;
            // Force the same offset in the descriptor table to overlaps these ranges,
            // starting at offset 1 because metadata is in register (binding) 0.
            offsetFromTableStart = 1;
        } else {
            range = &ranges.samplers;
            rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            // This size must includes both user and default samplers
            numDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
            // Force the same offset in the descriptor table to overlaps these ranges.
            // Unlike for SRVs, we can overlap them at register (binding) 0 since as there's
            // no metadata entry in this range.
            offsetFromTableStart = 0;
        }

        range->push_back(D3D12_DESCRIPTOR_RANGE1{
            .RangeType = rangeType,
            .NumDescriptors = numDescriptors,
            // HLSL doesn't allow overlapping register ranges, so each one is in its own
            // space (group), and starts at register (binding) 0 with no other range
            // bound after it.
            .BaseShaderRegister = 0,
            // Offset by 1 because metadata is in space baseRegisterSpace
            .RegisterSpace = baseRegisterSpace + 1 + uint32_t{i},
            // Volatile required for bindless
            .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,
            .OffsetInDescriptorsFromTableStart = offsetFromTableStart,
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

    // Allocate the CPU descriptor heaps
    const uint32_t apiSize = uint32_t{GetAPISize()};

    const uint32_t defaultNonSamplersCount =
        uint32_t{ResourceTableDefaultResources::GetNonSamplerCount()};
    const uint32_t numViewDescriptors = 1u + apiSize + defaultNonSamplersCount;
    DAWN_TRY_ASSIGN(mViewHeap, AllocateCPUHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                               numViewDescriptors));

    // Samplers are capped to D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE
    // TODO(crbug.com/499115444): Detect and OOM if user adds more than
    // (D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE - defaultSamplersCount) samplers to the table.
    const uint32_t defaultSamplersCount =
        uint32_t{ResourceTableDefaultResources::GetSamplerCount()};
    const uint32_t numSamplerDescriptors = std::min<uint32_t>(
        apiSize + defaultSamplersCount, D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE);
    DAWN_TRY_ASSIGN(mSamplerHeap, AllocateCPUHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                                                  numSamplerDescriptors));

    // Create sampler index pool with the total number of available sampler descriptors
    mSamplerIndexPool = std::make_unique<SamplerIndexPool>(numSamplerDescriptors);
    // Resize space for all possible slots
    mSlotToSamplerIndex.resize(ResourceTableSlot{GetSizeWithDefaultResources()},
                               kInvalidSamplerIndex);

    // Only write the metadata buffer to the view heap initially, all the other bindings will be
    // written as needed when they are inserted in the ResourceTable.
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
    desc.Buffer.NumElements = checked_cast<UINT>(metadataBuffer->GetSize() / 4);
    desc.Buffer.StructureByteStride = 0;
    desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

    uint32_t offsetInDescriptorCount = 0;  // Metadata buffer is the first element in the table
    d3d12Device->CreateShaderResourceView(
        resource, &desc,
        mViewHeap.cpuAllocation.OffsetFrom(mViewHeap.sizeIncrement, offsetInDescriptorCount));

    return {};
}

// static
ResultOrError<ResourceTable::Heap> ResourceTable::AllocateCPUHeap(
    Device* device,
    D3D12_DESCRIPTOR_HEAP_TYPE heapType,
    uint32_t descriptorCount) {
    ID3D12Device* d3d12Device = device->GetD3D12Device();

    D3D12_DESCRIPTOR_HEAP_DESC heapDescriptor;
    heapDescriptor.Type = heapType;
    heapDescriptor.NumDescriptors = descriptorCount;
    heapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDescriptor.NodeMask = 0;

    ComPtr<ID3D12DescriptorHeap> handle;
    DAWN_TRY(CheckHRESULT(d3d12Device->CreateDescriptorHeap(&heapDescriptor, IID_PPV_ARGS(&handle)),
                          "ID3D12Device::CreateDescriptorHeap"));

    const D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptor = {
        handle->GetCPUDescriptorHandleForHeapStart().ptr};

    return ResourceTable::Heap{
        .handle = std::move(handle),
        .cpuAllocation = CPUDescriptorHeapAllocation{baseCPUDescriptor, 0},
        .gpuAllocation = {},  // Allocated by Populate*
        .sizeIncrement = d3d12Device->GetDescriptorHandleIncrementSize(heapType),
        .numDescriptors = descriptorCount,
    };
}

// static
void ResourceTable::FreeCPUHeap(ResourceTable::Heap& heap) {
    heap.handle.Reset();
    heap.cpuAllocation.Invalidate();
    heap.sizeIncrement = 0;
    heap.numDescriptors = 0;
}

MaybeError ResourceTable::ApplyPendingUpdates(
    CommandRecordingContext* recordingContext,
    const absl::flat_hash_set<TextureBase*>& writableTextures) {
    return ApplyDirtySlotUpdatesWith(
        writableTextures,
        [&](const std::vector<MetadataUpdate>& metadataUpdates,
            const std::vector<ResourceDiff>& resourceDiffs,
            const absl::flat_hash_set<TextureBase*>& texturesToTransition) -> MaybeError {
            // Transition and initialize all required textures
            if (!texturesToTransition.empty()) {
                DAWN_TRY(TransitionResources(recordingContext, texturesToTransition));
            }

            // Update resource bindings before metadata to ensure mSlotToSamplerIndex is up-to-date
            if (!resourceDiffs.empty()) {
                DAWN_TRY(UpdateResourceBindings(resourceDiffs));
            }
            if (!metadataUpdates.empty()) {
                DAWN_TRY(UpdateMetadataBuffer(recordingContext, metadataUpdates));
            }
            return {};
        });
}

MaybeError ResourceTable::TransitionResources(CommandRecordingContext* recordingContext,
                                              const absl::flat_hash_set<TextureBase*>& textures) {
    for (const auto& texture : textures) {
        Texture* textureBackend = ToBackend(texture);
        DAWN_TRY(textureBackend->EnsureSubresourceContentInitialized(
            recordingContext, textureBackend->GetAllSubresources()));
        textureBackend->TrackUsageAndTransitionNow(recordingContext,
                                                   wgpu::TextureUsage::TextureBinding,
                                                   textureBackend->GetAllSubresources());
    }
    return {};
}

MaybeError ResourceTable::UpdateMetadataBuffer(CommandRecordingContext* recordingContext,
                                               const std::vector<MetadataUpdate>& updates) {
    // For metadata updates, we already created an SRV for metadata buffer in Initialize(), so all
    // we need to do is update the buffer.
    Device* device = ToBackend(GetDevice());

    // Allocate enough space for all the data to modify and schedule the copies.
    return device->GetDynamicUploader()->WithUploadReservation(
        sizeof(uint32_t) * updates.size(), kCopyBufferToBufferOffsetAlignment,
        [&](UploadReservation reservation) -> MaybeError {
            Span<uint32_t> stagedData = ReinterpretSpan<uint32_t>(reservation.mappedData);

            // The metadata buffer will be copied to.
            Buffer* metadataBuffer = ToBackend(GetMetadataBuffer());
            DAWN_ASSERT(metadataBuffer->IsResourceInitialized());
            auto scopedUseMetadataBuffer = metadataBuffer->UseInternal();
            metadataBuffer->TrackUsageAndTransitionNow(recordingContext,
                                                       wgpu::BufferUsage::CopyDst);

            // Record a CopyBufferRegion for each update
            // TODO(crbug.com/473354062): reduce number of calls by copying contiguous regions
            for (auto [i, update] : Enumerate(updates)) {
                DAWN_ASSERT((update.data & 0xFFFF0000) == 0);  // Only low 16 bits used for type id
                if (SamplerIndex samplerIndex = mSlotToSamplerIndex[update.slot];
                    samplerIndex != kInvalidSamplerIndex) {
                    // Store sampler index in high 16 bits, type id in low 16 bits
                    stagedData[i] = update.data | (uint32_t{samplerIndex} << 16);
                } else {
                    stagedData[i] = update.data;  // Copy to staged
                }

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

MaybeError ResourceTable::UpdateResourceBindings(const std::vector<ResourceDiff>& diffs) {
    Device* device = ToBackend(GetDevice());
    ID3D12Device* d3d12Device = device->GetD3D12Device();

    for (const ResourceDiff& diff : diffs) {
        // TODO(https://issues.chromium.org/473444515): Support buffer, texel buffers and storage
        // textures.

        if (auto samplerBase = std::get_if<Ref<SamplerBase>>(&diff.removed)) {
            // Release this sampler's index. It'll be returned to the pool if this is the last ref.
            mSamplerIndexPool->Release(samplerBase->Get());
            // Clear the table entry
            mSlotToSamplerIndex[diff.slot] = kInvalidSamplerIndex;
        }

        DAWN_TRY(MatchVariant(
            diff.added,
            [&](std::monostate) -> MaybeError {
                // Nothing to do.
                return {};
            },
            [&](Ref<TextureViewBase> textureView) -> MaybeError {
                auto view = ToBackend(textureView);
                ID3D12Resource* resource = ToBackend(view->GetTexture())->GetD3D12Resource();
                if (resource == nullptr) {
                    // Skip resource if it was destroyed
                    return {};
                }

                // Add 1 to skip metadata descriptor
                uint32_t offsetInDescriptorCount = 1 + static_cast<uint32_t>(diff.slot);

                d3d12Device->CreateShaderResourceView(
                    resource, &view->GetSRVDescriptor(),
                    mViewHeap.cpuAllocation.OffsetFrom(mViewHeap.sizeIncrement,
                                                       offsetInDescriptorCount));
                return {};
            },
            [&](Ref<SamplerBase> samplerBase) -> MaybeError {
                // Acquire a sampler index. If it's the first time we acquire one for this sampler,
                // create descriptor for it in the heap.
                std::pair<SamplerIndex, bool> result;
                DAWN_TRY_ASSIGN(result, mSamplerIndexPool->Acquire(samplerBase.Get()));
                auto [samplerIndex, isNewIndex] = result;
                if (isNewIndex) {
                    const D3D12_SAMPLER_DESC& samplerDesc =
                        ToBackend(samplerBase)->GetSamplerDescriptor();
                    uint32_t offsetInDescriptorCount = uint32_t{samplerIndex};
                    d3d12Device->CreateSampler(
                        &samplerDesc, mSamplerHeap.cpuAllocation.OffsetFrom(
                                          mSamplerHeap.sizeIncrement, offsetInDescriptorCount));
                }

                // Update the table entry
                mSlotToSamplerIndex[diff.slot] = samplerIndex;
                return {};
            }));
    }

    return {};
}

bool ResourceTable::PopulateViews(ShaderVisibleDescriptorAllocator* viewAllocator) {
    if (viewAllocator->IsAllocationStillValid(mViewHeap.gpuAllocation)) {
        return true;
    }

    // Attempt to allocate descriptors for the currently bound shader-visible heaps.
    // Return false if allocation fails to indicate that AllocateAndSwitchShaderVisibleHeap should
    // be called.
    Device* device = ToBackend(GetDevice());

    D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptor;
    if (!viewAllocator->AllocateGPUDescriptors(mViewHeap.numDescriptors,
                                               device->GetQueue()->GetPendingCommandSerial(),
                                               &baseCPUDescriptor, &mViewHeap.gpuAllocation)) {
        return false;
    }

    // CPU bindgroups are sparsely allocated across CPU heaps. Instead of doing
    // simple copies per bindgroup, a single non-simple copy could be issued.
    // TODO(dawn:155): Consider doing this optimization.
    device->GetD3D12Device()->CopyDescriptorsSimple(mViewHeap.numDescriptors, baseCPUDescriptor,
                                                    mViewHeap.cpuAllocation.GetBaseDescriptor(),
                                                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    return true;
}

bool ResourceTable::PopulateSamplers(ShaderVisibleDescriptorAllocator* samplerAllocator) {
    if (samplerAllocator->IsAllocationStillValid(mSamplerHeap.gpuAllocation)) {
        return true;
    }

    // Attempt to allocate descriptors for the currently bound shader-visible heaps.
    // Return false if allocation fails to indicate that AllocateAndSwitchShaderVisibleHeap should
    // be called.
    Device* device = ToBackend(GetDevice());

    D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptor;
    if (!samplerAllocator->AllocateGPUDescriptors(
            mSamplerHeap.numDescriptors, device->GetQueue()->GetPendingCommandSerial(),
            &baseCPUDescriptor, &mSamplerHeap.gpuAllocation)) {
        return false;
    }

    // CPU bindgroups are sparsely allocated across CPU heaps. Instead of doing
    // simple copies per bindgroup, a single non-simple copy could be issued.
    // TODO(dawn:155): Consider doing this optimization.
    device->GetD3D12Device()->CopyDescriptorsSimple(mSamplerHeap.numDescriptors, baseCPUDescriptor,
                                                    mSamplerHeap.cpuAllocation.GetBaseDescriptor(),
                                                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    return true;
}

uint32_t ResourceTable::GetViewDescriptorCount() const {
    return mViewHeap.numDescriptors;
}

uint32_t ResourceTable::GetSamplerDescriptorCount() const {
    return mSamplerHeap.numDescriptors;
}

D3D12_GPU_DESCRIPTOR_HANDLE ResourceTable::GetBaseViewDescriptor() const {
    return mViewHeap.gpuAllocation.GetBaseDescriptor();
}

D3D12_GPU_DESCRIPTOR_HANDLE ResourceTable::GetBaseSamplerDescriptor() const {
    return mSamplerHeap.gpuAllocation.GetBaseDescriptor();
}

void ResourceTable::DestroyImpl(DestroyReason reason) {
    ResourceTableBase::DestroyImpl(reason);
    FreeCPUHeap(mViewHeap);
    FreeCPUHeap(mSamplerHeap);
    mSlotToSamplerIndex.clear();
    mSamplerIndexPool = nullptr;
}

void ResourceTable::SetLabelImpl() {
    // TODO(crbug.com/473354062): SetDebugName
}

}  // namespace dawn::native::d3d12
