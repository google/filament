// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/d3d12/ResourceAllocatorManagerD3D12.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "dawn/native/Queue.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/HeapAllocatorD3D12.h"
#include "dawn/native/d3d12/HeapD3D12.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "dawn/native/d3d12/ResourceHeapAllocationD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"

namespace dawn::native::d3d12 {
namespace {
MemorySegment GetMemorySegment(Device* device, ResourceHeapKind resourceHeapKind) {
    if (device->GetDeviceInfo().isUMA) {
        return MemorySegment::Local;
    }

    // Currently we only use Custom_WriteBack_OnlyBuffers on UMA architectures.
    // TODO(386255678): consider ReBAR which is UMA Coherent.
    if (resourceHeapKind == Custom_WriteBack_OnlyBuffers) {
        return MemorySegment::Local;
    }

    D3D12_HEAP_TYPE heapType = GetD3D12HeapType(resourceHeapKind);
    D3D12_HEAP_PROPERTIES heapProperties =
        device->GetD3D12Device()->GetCustomHeapProperties(0, heapType);

    if (heapProperties.MemoryPoolPreference == D3D12_MEMORY_POOL_L1) {
        return MemorySegment::Local;
    }

    return MemorySegment::NonLocal;
}

D3D12_HEAP_FLAGS GetD3D12HeapFlags(ResourceHeapKind resourceHeapKind) {
    switch (resourceHeapKind) {
        case ResourceHeapKind::Default_AllBuffersAndTextures:
        case ResourceHeapKind::Readback_AllBuffersAndTextures:
        case ResourceHeapKind::Upload_AllBuffersAndTextures:
            return D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;
        case ResourceHeapKind::Default_OnlyBuffers:
        case ResourceHeapKind::Readback_OnlyBuffers:
        case ResourceHeapKind::Upload_OnlyBuffers:
        case ResourceHeapKind::Custom_WriteBack_OnlyBuffers:
            return D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
        case ResourceHeapKind::Default_OnlyNonRenderableOrDepthTextures:
            return D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
        case ResourceHeapKind::Default_OnlyRenderableOrDepthTextures:
            return D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
        case EnumCount:
            DAWN_UNREACHABLE();
    }
}

uint64_t GetInitialResourcePlacementAlignment(
    const D3D12_RESOURCE_DESC& requestedResourceDescriptor,
    Device* device) {
    switch (requestedResourceDescriptor.Dimension) {
        case D3D12_RESOURCE_DIMENSION_BUFFER:
            return requestedResourceDescriptor.Alignment;

        // Always try using small resource placement alignment first when Alignment == 0 because if
        // Alignment is set to 0, the runtime will use 4MB for MSAA textures and 64KB for everything
        // else.
        // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_resource_desc
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        case D3D12_RESOURCE_DIMENSION_TEXTURE3D: {
            if (requestedResourceDescriptor.Alignment > 0) {
                return requestedResourceDescriptor.Alignment;
            }

            if (requestedResourceDescriptor.SampleDesc.Count > 1) {
                return device->IsToggleEnabled(Toggle::D3D12Use64KBAlignedMSAATexture)
                           ? D3D12_SMALL_MSAA_RESOURCE_PLACEMENT_ALIGNMENT
                           : D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
            } else if (requestedResourceDescriptor.Flags &
                       (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
                        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) {
                // Render target and depth stencil textures do not support the small alignment.
                return D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            } else {
                return D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
            }
        }
        case D3D12_RESOURCE_DIMENSION_UNKNOWN:
        default:
            DAWN_UNREACHABLE();
    }
}

bool IsClearValueOptimizable(DeviceBase* device, const D3D12_RESOURCE_DESC& resourceDescriptor) {
    if (device->IsToggleEnabled(Toggle::D3D12DontSetClearValueOnDepthTextureCreation)) {
        switch (resourceDescriptor.Format) {
            case DXGI_FORMAT_D16_UNORM:
            case DXGI_FORMAT_D32_FLOAT:
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
                return false;
            default:
                break;
        }
    }

    // Optimized clear color cannot be set on buffers, non-render-target/depth-stencil
    // textures, or typeless resources
    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createcommittedresource
    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createplacedresource
    return !d3d::IsTypeless(resourceDescriptor.Format) &&
           resourceDescriptor.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER &&
           (resourceDescriptor.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
                                        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) != 0;
}

uint32_t GetColumnPitch(uint32_t baseHeight, uint32_t mipLevelCount) {
    // This function returns the number of rows of block for a single layer with all mipmaps.
    //
    // Below is a simple diagram about texture memory layout for one single layer of a mipmap
    // texture. For details about texture memory layout on Intel Gen12 GPU, read page 78 at
    // https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-tgl-vol05-memory_data_formats.pdf.
    //     ----------------------------------------------        ---
    //     |                                            |         |
    //     |                                            |
    //     |                                            |
    //     |                                            |
    //     |                  LOD 0                     |
    //     |                                            |
    //     |                                            |
    //     |                                            |     column pitch (aka QPitch)
    //     |                                            |
    //     |                                            |
    //     ----------------------------------------------
    //     |                    |        |
    //     |                    |  LOD2  |
    //     |     LOD 1          |---------
    //     |                    | LOD3 |
    //     |                    |-------
    //     |                    |   .
    //     ----------------------   .                              |
    //                              .                             ---

    uint32_t level1Height = 0;
    uint32_t level2ToTailHeight = 0;
    if (mipLevelCount >= 2) {
        level1Height = std::max(baseHeight >> 1, 1u);

        for (uint32_t level = 2; level < mipLevelCount; ++level) {
            level2ToTailHeight += std::max(baseHeight >> level, 1u);
        }
    }
    // The height of level 2 to tail (or max) can be greater than the height of level 1. For
    // example, if the single layer's dimension is 16x4 and it has full mipmaps, then there are 5
    // levels: 16x4, 8x2, 4x1, 2x1, 1x1. So level1Height is 2, while level2ToTailHeight is 1+1+1
    // = 3.
    uint32_t columnPitch = baseHeight + std::max(level1Height, level2ToTailHeight);

    // The number of rows of block for a texture must be a multiple of 4.
    return Align(columnPitch, 4);
}

uint32_t ComputeExtraArraySizeForIntelGen12(uint32_t width,
                                            uint32_t height,
                                            uint32_t arrayLayerCount,
                                            uint32_t mipLevelCount,
                                            uint32_t sampleCount,
                                            uint32_t colorFormatBytesPerBlock) {
    // For details about texture memory layout on Intel Gen12 GPU, read
    // https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-tgl-vol05-memory_data_formats.pdf.
    //   - Texture memory layout: from <Surface Memory Organizations> to
    //     <Surface Padding Requirement>.
    //   - Tile-based memory: the entire section of <Address Tiling Function Introduction>.
    constexpr uint32_t kPageSize = 4 * 1024;
    constexpr uint32_t kLinearAlignment = 4 * kPageSize;

    // There are two tile modes: TileYS (64KB per tile) and TileYf (4KB per tile). TileYS is used
    // here because it may have more paddings and therefore requires more extra layers to work
    // around the bug.
    constexpr uint32_t kTileSize = 16 * kPageSize;

    // Tile's width and height vary according to format bit-wise (colorFormatBytesPerBlock)
    uint32_t tileHeight = 0;
    switch (colorFormatBytesPerBlock) {
        case 1:
            tileHeight = 256;
            break;
        case 2:
        case 4:
            tileHeight = 128;
            break;
        case 8:
        case 16:
            tileHeight = 64;
            break;
        default:
            DAWN_UNREACHABLE();
    }
    uint32_t tileWidth = kTileSize / tileHeight;

    uint64_t layerxSamples = arrayLayerCount * sampleCount;

    if (layerxSamples <= 1) {
        return 0;
    }

    uint32_t columnPitch = GetColumnPitch(height, mipLevelCount);

    uint64_t totalWidth = width * colorFormatBytesPerBlock;
    uint64_t totalHeight = columnPitch * layerxSamples;

    // Texture should be aligned on both tile width (512 bytes) and tile height (128 rows) on Intel
    // Gen12 GPU
    uint32_t mainTileCols = Align(totalWidth, tileWidth) / tileWidth;
    uint32_t mainTileRows = Align(totalHeight, tileHeight) / tileHeight;
    uint64_t mainTileCount = mainTileCols * mainTileRows;

    // There is a bug in Intel old drivers to compute the auxiliary memory size (auxSize) of the
    // color texture, which is calculated from the main memory size (mainSize) of the texture. Note
    // that memory allocation for mainSize itself is correct. But during memory allocation for
    // auxSize, it re-caculated mainSize and did it in a wrong way. The incorrect algorithm doesn't
    // respect alignment requirements from tile-based texture memory layout. It just simple aligned
    // to a constant value (16K) for each sample and layer.
    uint64_t expectedMainSize = mainTileCount * kTileSize;
    uint64_t actualMainSize = Align(columnPitch * totalWidth, kLinearAlignment) * layerxSamples;

    // If the incorrect mainSize calculation lead to less-than-expected auxSize, texture corruption
    // is very likely to happen for any color texture access like texture copy, rendering, sampling,
    // etc. So we have to allocate a few more extra layers to offset the less-than-expected auxSize.
    // However, it is fine if the incorrect mainSize calculation doesn't introduce less auxSize. For
    // example, if correct mainSize is 3.8M, it requires 4 pages of auxSize (16K). Any incorrect
    // mainSize between 3.0+ M and 4.0M also requires 16K auxSize according to the calculation:
    // auxSize = Align(mainSize >> 8, kPageSize). And greater auxSize is also fine. But if mainSize
    // is less than 3.0M, its auxSize will be less than 16K and hence texture corruption is caused.
    uint64_t expectedAuxSize = Align(expectedMainSize >> 8, kPageSize);
    uint64_t actualAuxSize = Align(actualMainSize >> 8, kPageSize);
    if (actualAuxSize < expectedAuxSize) {
        uint64_t actualMainSizePerLayer = actualMainSize / arrayLayerCount;
        return (expectedMainSize - actualMainSize + actualMainSizePerLayer - 1) /
               actualMainSizePerLayer;
    }
    return 0;
}

bool ShouldAllocateAsCommittedResource(Device* device, bool forceAllocateAsCommittedResource) {
    return forceAllocateAsCommittedResource ||
           device->IsToggleEnabled(Toggle::DisableResourceSuballocation);
}

D3D12_HEAP_FLAGS GetHeapFlagsForCommittedResource(Device* device,
                                                  const D3D12_RESOURCE_DESC& resourceDescriptor) {
    if (!device->IsToggleEnabled(Toggle::D3D12CreateNotZeroedHeap)) {
        return D3D12_HEAP_FLAG_NONE;
    }

    if (device->IsToggleEnabled(
            Toggle::D3D12DontUseNotZeroedHeapFlagOnTexturesAsCommitedResources) &&
        resourceDescriptor.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER) {
        return D3D12_HEAP_FLAG_NONE;
    }

    return D3D12_HEAP_FLAG_CREATE_NOT_ZEROED;
}

}  // namespace

ResourceAllocatorManager::ResourceAllocatorManager(Device* device) : mDevice(device) {
    D3D12_HEAP_FLAGS createNotZeroedHeapFlag =
        mDevice->IsToggleEnabled(Toggle::D3D12CreateNotZeroedHeap)
            ? D3D12_HEAP_FLAG_CREATE_NOT_ZEROED
            : D3D12_HEAP_FLAG_NONE;

    for (uint32_t i = 0; i < ResourceHeapKind::EnumCount; i++) {
        const ResourceHeapKind resourceHeapKind = static_cast<ResourceHeapKind>(i);
        D3D12_HEAP_FLAGS heapFlags = GetD3D12HeapFlags(resourceHeapKind) | createNotZeroedHeapFlag;
        mHeapAllocators[i] = std::make_unique<HeapAllocator>(
            mDevice, resourceHeapKind, heapFlags, GetMemorySegment(mDevice, resourceHeapKind));
        mPooledHeapAllocators[i] =
            std::make_unique<PooledResourceMemoryAllocator>(mHeapAllocators[i].get());
        mSubAllocatedResourceAllocators[i] = std::make_unique<BuddyMemoryAllocator>(
            kMaxHeapSize, kMinHeapSize, mPooledHeapAllocators[i].get());
    }
}

ResourceAllocatorManager::~ResourceAllocatorManager() {
    // Ensure any remaining objects go through the same shutdown path as normal usage.
    // Placed resources must be released before any heaps they reside in.
    Tick(std::numeric_limits<ExecutionSerial>::max());

    for (uint32_t i = 0; i < ResourceHeapKind::EnumCount; i++) {
        mSubAllocatedResourceAllocators[i] = nullptr;
    }

    DestroyPool();

    DAWN_ASSERT(mAllocationsToDelete.Empty());
    DAWN_ASSERT(mHeapsToDelete.Empty());
}

ResultOrError<ResourceHeapAllocation> ResourceAllocatorManager::AllocateMemory(
    ResourceHeapKind resourceHeapKind,
    const D3D12_RESOURCE_DESC& resourceDescriptor,
    D3D12_RESOURCE_STATES initialUsage,
    uint32_t colorFormatBytesPerBlock,
    bool forceAllocateAsCommittedResource) {
    // In order to suppress a warning in the D3D12 debug layer, we need to specify an
    // optimized clear value. As there are no negative consequences when picking a mismatched
    // clear value, we use zero as the optimized clear value. This also enables fast clears on
    // some architectures.
    D3D12_CLEAR_VALUE zero{};
    D3D12_CLEAR_VALUE* optimizedClearValue = nullptr;
    if (IsClearValueOptimizable(mDevice, resourceDescriptor)) {
        zero.Format = resourceDescriptor.Format;
        optimizedClearValue = &zero;
    }

    // If we are allocating memory for a 2D array texture with a color format on D3D12 backend,
    // we need to allocate extra layers on some Intel Gen12 devices, see crbug.com/dawn/949
    // for details.
    D3D12_RESOURCE_DESC revisedDescriptor = resourceDescriptor;
    if (mDevice->IsToggleEnabled(Toggle::D3D12AllocateExtraMemoryFor2DArrayColorTexture) &&
        resourceDescriptor.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D &&
        resourceDescriptor.DepthOrArraySize > 1 && colorFormatBytesPerBlock > 0) {
        // Multisample textures have one layer at most. Only non-multisample textures need the
        // workaround.
        DAWN_ASSERT(revisedDescriptor.SampleDesc.Count <= 1);
        revisedDescriptor.DepthOrArraySize += ComputeExtraArraySizeForIntelGen12(
            resourceDescriptor.Width, resourceDescriptor.Height,
            resourceDescriptor.DepthOrArraySize, resourceDescriptor.MipLevels,
            resourceDescriptor.SampleDesc.Count, colorFormatBytesPerBlock);
    }

    // TODO(crbug.com/dawn/849): Conditionally disable sub-allocation.
    // For very large resources, there is no benefit to suballocate.
    // For very small resources, it is inefficient to suballocate given the min. heap
    // size could be much larger then the resource allocation.
    // Attempt to satisfy the request using sub-allocation (placed resource in a heap).
    if (!ShouldAllocateAsCommittedResource(mDevice, forceAllocateAsCommittedResource)) {
        ResourceHeapAllocation subAllocation;
        DAWN_TRY_ASSIGN(subAllocation, CreatePlacedResource(resourceHeapKind, revisedDescriptor,
                                                            optimizedClearValue, initialUsage));
        if (subAllocation.GetInfo().mMethod != AllocationMethod::kInvalid) {
            return std::move(subAllocation);
        }
    }

    // If sub-allocation fails, fall-back to direct allocation (committed resource).
    ResourceHeapAllocation directAllocation;
    DAWN_TRY_ASSIGN(directAllocation, CreateCommittedResource(resourceHeapKind, revisedDescriptor,
                                                              optimizedClearValue, initialUsage));
    if (directAllocation.GetInfo().mMethod != AllocationMethod::kInvalid) {
        return std::move(directAllocation);
    }

    // If direct allocation fails, the system is probably out of memory.
    return DAWN_OUT_OF_MEMORY_ERROR("Allocation failed");
}

void ResourceAllocatorManager::Tick(ExecutionSerial completedSerial) {
    for (ResourceHeapAllocation& allocation : mAllocationsToDelete.IterateUpTo(completedSerial)) {
        if (allocation.GetInfo().mMethod == AllocationMethod::kSubAllocated) {
            FreeSubAllocatedMemory(allocation);
        }
    }
    mAllocationsToDelete.ClearUpTo(completedSerial);
    mHeapsToDelete.ClearUpTo(completedSerial);
}

void ResourceAllocatorManager::DeallocateMemory(ResourceHeapAllocation& allocation) {
    if (allocation.GetInfo().mMethod == AllocationMethod::kInvalid) {
        return;
    }

    mAllocationsToDelete.Enqueue(allocation, mDevice->GetQueue()->GetPendingCommandSerial());

    // Directly allocated ResourceHeapAllocations are created with a heap object that must be
    // manually deleted upon deallocation. See ResourceAllocatorManager::CreateCommittedResource
    // for more information. Acquire this heap as a unique_ptr and add it to the queue of heaps
    // to delete. It cannot be deleted immediately because it may be in use by in-flight or
    // pending commands.
    if (allocation.GetInfo().mMethod == AllocationMethod::kDirect) {
        mHeapsToDelete.Enqueue(std::unique_ptr<ResourceHeapBase>(allocation.GetResourceHeap()),
                               mDevice->GetQueue()->GetPendingCommandSerial());
    }

    // Invalidate the allocation immediately in case one accidentally
    // calls DeallocateMemory again using the same allocation.
    allocation.Invalidate();

    DAWN_ASSERT(allocation.GetD3D12Resource() == nullptr);
}

void ResourceAllocatorManager::FreeSubAllocatedMemory(ResourceHeapAllocation& allocation) {
    DAWN_ASSERT(allocation.GetInfo().mMethod == AllocationMethod::kSubAllocated);

    const size_t resourceHeapKindIndex = static_cast<size_t>(allocation.GetResourceHeapKind());
    mSubAllocatedResourceAllocators[resourceHeapKindIndex]->Deallocate(allocation);
}

ResultOrError<ResourceHeapAllocation> ResourceAllocatorManager::CreatePlacedResource(
    ResourceHeapKind resourceHeapKind,
    const D3D12_RESOURCE_DESC& requestedResourceDescriptor,
    const D3D12_CLEAR_VALUE* optimizedClearValue,
    D3D12_RESOURCE_STATES initialUsage) {
    D3D12_RESOURCE_DESC resourceDescriptor = requestedResourceDescriptor;
    resourceDescriptor.Alignment =
        GetInitialResourcePlacementAlignment(requestedResourceDescriptor, mDevice);

    // When you're using CreatePlacedResource, your application must use GetResourceAllocationInfo
    // in order to understand the size and alignment characteristics of texture resources.
    D3D12_RESOURCE_ALLOCATION_INFO resourceInfo =
        mDevice->GetD3D12Device()->GetResourceAllocationInfo(0, 1, &resourceDescriptor);

    // If the requested resource alignment was rejected, set alignment to 0 to do allocation with
    // default alignment in D3D12 (4MB for MSAA textures and 64KB for everything else).
    // If an error occurs, then D3D12_RESOURCE_ALLOCATION_INFO::SizeInBytes equals UINT64_MAX.
    if (resourceInfo.SizeInBytes == std::numeric_limits<uint64_t>::max() ||
        (resourceDescriptor.Alignment != 0 &&
         resourceDescriptor.Alignment != resourceInfo.Alignment)) {
        resourceDescriptor.Alignment = 0;
        resourceInfo =
            mDevice->GetD3D12Device()->GetResourceAllocationInfo(0, 1, &resourceDescriptor);
    }

    // If d3d tells us the resource size is invalid, treat the error as OOM.
    // Otherwise, creating the resource could cause a device loss (too large).
    // This is because NextPowerOfTwo(UINT64_MAX) overflows and proceeds to
    // incorrectly allocate a mismatched size.
    if (resourceInfo.SizeInBytes == 0 ||
        resourceInfo.SizeInBytes == std::numeric_limits<uint64_t>::max()) {
        return DAWN_OUT_OF_MEMORY_ERROR(absl::StrFormat(
            "Resource allocation size (%u) was invalid.", resourceInfo.SizeInBytes));
    }

    BuddyMemoryAllocator* allocator =
        mSubAllocatedResourceAllocators[static_cast<size_t>(resourceHeapKind)].get();

    ResourceMemoryAllocation allocation;
    DAWN_TRY_ASSIGN(allocation,
                    allocator->Allocate(resourceInfo.SizeInBytes, resourceInfo.Alignment));
    if (allocation.GetInfo().mMethod == AllocationMethod::kInvalid) {
        return ResourceHeapAllocation{};  // invalid
    }

    Heap* heap = ToBackend(allocation.GetResourceHeap());

    // Before calling CreatePlacedResource, we must ensure the target heap is resident.
    // CreatePlacedResource will fail if it is not.
    DAWN_TRY(mDevice->GetResidencyManager()->LockAllocation(heap));

    // With placed resources, a single heap can be reused.
    // The resource placed at an offset is only reclaimed
    // upon Tick or after the last command list using the resource has completed
    // on the GPU. This means the same physical memory is not reused
    // within the same command-list and does not require additional synchronization (aliasing
    // barrier).
    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createplacedresource
    ComPtr<ID3D12Resource> placedResource;
    DAWN_TRY(CheckOutOfMemoryHRESULT(
        mDevice->GetD3D12Device()->CreatePlacedResource(
            heap->GetD3D12Heap(), allocation.GetOffset(), &resourceDescriptor, initialUsage,
            optimizedClearValue, IID_PPV_ARGS(&placedResource)),
        "ID3D12Device::CreatePlacedResource"));

    // After CreatePlacedResource has finished, the heap can be unlocked from residency. This
    // will insert it into the residency LRU.
    mDevice->GetResidencyManager()->UnlockAllocation(heap);

    return ResourceHeapAllocation{allocation.GetInfo(), allocation.GetOffset(),
                                  std::move(placedResource), heap, resourceHeapKind};
}

ResultOrError<ResourceHeapAllocation> ResourceAllocatorManager::CreateCommittedResource(
    ResourceHeapKind resourceHeapKind,
    const D3D12_RESOURCE_DESC& resourceDescriptor,
    const D3D12_CLEAR_VALUE* optimizedClearValue,
    D3D12_RESOURCE_STATES initialUsage) {
    D3D12_HEAP_PROPERTIES heapProperties = GetD3D12HeapProperties(resourceHeapKind);

    // If d3d tells us the resource size is invalid, treat the error as OOM.
    // Otherwise, creating the resource could cause a device loss (too large).
    // This is because NextPowerOfTwo(UINT64_MAX) overflows and proceeds to
    // incorrectly allocate a mismatched size.
    D3D12_RESOURCE_ALLOCATION_INFO resourceInfo =
        mDevice->GetD3D12Device()->GetResourceAllocationInfo(0, 1, &resourceDescriptor);

    if (resourceInfo.SizeInBytes == 0 ||
        resourceInfo.SizeInBytes == std::numeric_limits<uint64_t>::max()) {
        return DAWN_OUT_OF_MEMORY_ERROR("Resource allocation size was invalid.");
    }

    if (resourceInfo.SizeInBytes > kMaxHeapSize) {
        return ResourceHeapAllocation{};  // Invalid
    }

    // CreateCommittedResource will implicitly make the created resource resident. We must
    // ensure enough free memory exists before allocating to avoid an out-of-memory error when
    // overcommitted.
    DAWN_TRY(mDevice->GetResidencyManager()->EnsureCanAllocate(
        resourceInfo.SizeInBytes, GetMemorySegment(mDevice, resourceHeapKind)));

    // Note: Heap flags are inferred by the resource descriptor and do not need to be explicitly
    // provided to CreateCommittedResource.
    ComPtr<ID3D12Resource> committedResource;
    D3D12_RESOURCE_DESC appliedResourceDescriptor = resourceDescriptor;
    if (resourceDescriptor.SampleDesc.Count > 1 &&
        mDevice->IsToggleEnabled(Toggle::D3D12Use64KBAlignedMSAATexture)) {
        appliedResourceDescriptor.Alignment = D3D12_SMALL_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
    }

    D3D12_HEAP_FLAGS heapFlags =
        GetHeapFlagsForCommittedResource(mDevice, appliedResourceDescriptor);
    DAWN_TRY(CheckOutOfMemoryHRESULT(
        mDevice->GetD3D12Device()->CreateCommittedResource(
            &heapProperties, heapFlags, &appliedResourceDescriptor, initialUsage,
            optimizedClearValue, IID_PPV_ARGS(&committedResource)),
        "ID3D12Device::CreateCommittedResource"));

    // When using CreateCommittedResource, D3D12 creates an implicit heap that contains the
    // resource allocation. Because Dawn's memory residency management occurs at the resource
    // heap granularity, every directly allocated ResourceHeapAllocation also stores a Heap
    // object. This object is created manually, and must be deleted manually upon deallocation
    // of the committed resource.
    Heap* heap = new Heap(committedResource, GetMemorySegment(mDevice, resourceHeapKind),
                          resourceInfo.SizeInBytes);

    // Calling CreateCommittedResource implicitly calls MakeResident on the resource. We must
    // track this to avoid calling MakeResident a second time.
    mDevice->GetResidencyManager()->TrackResidentAllocation(heap);

    AllocationInfo info;
    info.mMethod = AllocationMethod::kDirect;

    return ResourceHeapAllocation{info,
                                  /*offset*/ 0, std::move(committedResource), heap,
                                  resourceHeapKind};
}

void ResourceAllocatorManager::DestroyPool() {
    for (auto& alloc : mPooledHeapAllocators) {
        alloc->DestroyPool();
    }
}

}  // namespace dawn::native::d3d12
