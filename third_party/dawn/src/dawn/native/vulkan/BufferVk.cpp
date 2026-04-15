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

#include "dawn/native/vulkan/BufferVk.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/GPUInfo.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/Queue.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/QueueVk.h"
#include "dawn/native/vulkan/ResourceHeapVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

namespace {

VkBufferUsageFlags VulkanBufferUsage(wgpu::BufferUsage usage) {
    VkBufferUsageFlags flags = 0;

    if (usage & (wgpu::BufferUsage::CopySrc | kInternalCopySrcBuffer)) {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (usage & wgpu::BufferUsage::CopyDst) {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if (usage & wgpu::BufferUsage::Index) {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (usage & wgpu::BufferUsage::Vertex) {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (usage & wgpu::BufferUsage::Uniform) {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer | kReadOnlyStorageBuffer)) {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (usage & (wgpu::BufferUsage::TexelBuffer | kReadOnlyTexelBuffer)) {
        // Both bits are set so the VkBufferView can be used with either
        // VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER or VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
        // at bind group creation time, depending on access mode and device capabilities.
        flags |=
            VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (usage & wgpu::BufferUsage::Indirect) {
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if (usage & wgpu::BufferUsage::QueryResolve) {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    return flags;
}

VkPipelineStageFlags VulkanPipelineStage(wgpu::BufferUsage usage, wgpu::ShaderStage shaderStage) {
    VkPipelineStageFlags flags = 0;

    if (usage & kMappableBufferUsages) {
        flags |= VK_PIPELINE_STAGE_HOST_BIT;
    }
    if (usage &
        (wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | kInternalCopySrcBuffer)) {
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    if (usage & (wgpu::BufferUsage::Index | wgpu::BufferUsage::Vertex)) {
        flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    }
    if (usage & kShaderBufferUsages) {
        if (shaderStage & wgpu::ShaderStage::Vertex) {
            flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        }
        if (shaderStage & wgpu::ShaderStage::Fragment) {
            flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        if (shaderStage & wgpu::ShaderStage::Compute) {
            flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        }
    }
    if (usage & kIndirectBufferForBackendResourceTracking) {
        flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    }
    if (usage & wgpu::BufferUsage::QueryResolve) {
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    return flags;
}

VkAccessFlags VulkanAccessFlags(wgpu::BufferUsage usage) {
    VkAccessFlags flags = 0;

    if (usage & wgpu::BufferUsage::MapRead) {
        flags |= VK_ACCESS_HOST_READ_BIT;
    }
    if (usage & wgpu::BufferUsage::MapWrite) {
        flags |= VK_ACCESS_HOST_WRITE_BIT;
    }
    if (usage & (wgpu::BufferUsage::CopySrc | kInternalCopySrcBuffer)) {
        flags |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if (usage & wgpu::BufferUsage::CopyDst) {
        flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    if (usage & wgpu::BufferUsage::Index) {
        flags |= VK_ACCESS_INDEX_READ_BIT;
    }
    if (usage & wgpu::BufferUsage::Vertex) {
        flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }
    if (usage & wgpu::BufferUsage::Uniform) {
        flags |= VK_ACCESS_UNIFORM_READ_BIT;
    }
    if (usage &
        (wgpu::BufferUsage::Storage | kInternalStorageBuffer | wgpu::BufferUsage::TexelBuffer)) {
        flags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    }
    if (usage & (kReadOnlyStorageBuffer | kReadOnlyTexelBuffer)) {
        flags |= VK_ACCESS_SHADER_READ_BIT;
    }
    if (usage & kIndirectBufferForBackendResourceTracking) {
        flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    }
    if (usage & wgpu::BufferUsage::QueryResolve) {
        flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }

    return flags;
}

MemoryKind GetMemoryKindFor(wgpu::BufferUsage bufferUsage) {
    MemoryKind requestKind = MemoryKind::Linear;
    if (bufferUsage & wgpu::BufferUsage::MapRead) {
        requestKind |= MemoryKind::ReadMappable;
    }
    if (bufferUsage & wgpu::BufferUsage::MapWrite) {
        requestKind |= MemoryKind::WriteMappable;
    }

    // `kDeviceLocalBufferUsages` covers all the buffer usages that prefer the memory type
    // `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`.
    constexpr wgpu::BufferUsage kDeviceLocalBufferUsages =
        wgpu::BufferUsage::Index | wgpu::BufferUsage::QueryResolve | wgpu::BufferUsage::Storage |
        wgpu::BufferUsage::Uniform | wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::Vertex |
        kInternalStorageBuffer | kReadOnlyStorageBuffer | kReadOnlyTexelBuffer |
        kIndirectBufferForBackendResourceTracking;
    if (bufferUsage & kDeviceLocalBufferUsages) {
        requestKind |= MemoryKind::DeviceLocal;
    }

    return requestKind;
}

// Returns a Vulkan spec compliant memory range by aligning `offset` down and `size` up to multiples
// of `nonCoherentAtomSize`.
VkMappedMemoryRange GetMappedMemoryRange(const ResourceMemoryAllocation& allocation,
                                         size_t offset,
                                         size_t size,
                                         size_t nonCoherentAtomSize) {
    DAWN_ASSERT(IsAligned(allocation.GetOffset(), nonCoherentAtomSize));

    // `offset` must always be a multiple of nonCoherentAtomSize. `size` must either be a multiple
    // of nonCoherentAtomSize or offset+size must be equal to the size of the allocation.
    size_t fullOffset = allocation.GetOffset() + offset;
    size_t alignedOffset = AlignDown(fullOffset, nonCoherentAtomSize);
    size_t alignedSize = Align(size + (fullOffset - alignedOffset), nonCoherentAtomSize);

    size_t allocationSize = allocation.GetInfo().mRequestedSize;
    if (alignedOffset + alignedSize > allocationSize) {
        alignedSize = allocationSize - alignedOffset;
    }

    return VkMappedMemoryRange{.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                               .pNext = nullptr,
                               .memory = ToBackend(allocation.GetResourceHeap())->GetMemory(),
                               .offset = alignedOffset,
                               .size = alignedSize};
}

}  // namespace

// static
ResultOrError<Ref<Buffer>> Buffer::Create(Device* device,
                                          const UnpackedPtr<BufferDescriptor>& descriptor) {
    Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor));

    if (auto* hostMappedDesc = descriptor.Get<BufferHostMappedPointer>()) {
        DAWN_TRY(buffer->InitializeHostMapped(hostMappedDesc));
    } else {
        DAWN_TRY(buffer->Initialize(descriptor->mappedAtCreation));
    }
    return std::move(buffer);
}

MaybeError Buffer::Initialize(bool mappedAtCreation) {
    // vkCmdFillBuffer requires the size to be a multiple of 4.
    constexpr size_t kAlignment = 4u;

    uint32_t extraBytes = 0u;
    if (GetInternalUsage() & (wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index)) {
        // vkCmdSetIndexBuffer and vkCmdSetVertexBuffer are invalid if the offset
        // is equal to the whole buffer size. Allocate at least one more byte so it
        // is valid to setVertex/IndexBuffer with a zero-sized range at the end
        // of the buffer with (offset=buffer.size, size=0).
        extraBytes = 1u;
    }

    uint64_t size = GetSize();
    if (size > std::numeric_limits<uint64_t>::max() - extraBytes) {
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }

    size += extraBytes;

    // Allocate at least 4 bytes so clamped accesses are always in bounds.
    // Also, Vulkan requires the size to be non-zero.
    size = std::max(size, uint64_t(4u));

    if (size > std::numeric_limits<uint64_t>::max() - kAlignment) {
        // Alignment would overlow.
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }
    mAllocatedSize = Align(size, kAlignment);

    // Round uniform buffer sizes up to a multiple of 16 bytes since Tint will polyfill them as
    // array<vec4u, ...>.
    if (GetUsage() & wgpu::BufferUsage::Uniform) {
        mAllocatedSize = Align(size, 16u);
    }

    // Avoid passing ludicrously large sizes to drivers because it causes issues: drivers add
    // some constants to the size passed and align it, but for values close to the maximum
    // VkDeviceSize this can cause overflows and makes drivers crash or return bad sizes in the
    // VkmemoryRequirements. See https://gitlab.khronos.org/vulkan/vulkan/issues/1904
    // Any size with one of two top bits of VkDeviceSize set is a HUGE allocation and we can
    // safely return an OOM error.
    if (mAllocatedSize & (uint64_t(3) << uint64_t(62))) {
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer size is HUGE and could cause overflows");
    }

    VkBufferCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.size = mAllocatedSize;
    // Add CopyDst for non-mappable buffer initialization with mappedAtCreation
    // and robust resource initialization.
    createInfo.usage = VulkanBufferUsage(GetInternalUsage() | wgpu::BufferUsage::CopyDst);
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = 0;

    Device* device = ToBackend(GetDevice());
    DAWN_TRY(CheckVkOOMThenSuccess(
        device->fn.CreateBuffer(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "vkCreateBuffer"));

    // Gather requirements for the buffer's memory and allocate it.
    VkMemoryRequirements requirements;
    device->fn.GetBufferMemoryRequirements(device->GetVkDevice(), mHandle, &requirements);

    MemoryKind requestKind = GetMemoryKindFor(GetInternalUsage());
    DAWN_TRY_ASSIGN(mMemoryAllocation,
                    device->GetResourceMemoryAllocator()->Allocate(requirements, requestKind));

    // Finally associate it with the buffer.
    DAWN_TRY(CheckVkSuccess(
        device->fn.BindBufferMemory(device->GetVkDevice(), mHandle,
                                    ToBackend(mMemoryAllocation.GetResourceHeap())->GetMemory(),
                                    mMemoryAllocation.GetOffset()),
        "vkBindBufferMemory"));

    // Get if buffer is host visible and coherent. This can be the case even if the buffer was not
    // created with map usages, as on integrated GPUs all memory will typically be host visible.
    const size_t memoryType = ToBackend(mMemoryAllocation.GetResourceHeap())->GetMemoryType();
    const VkMemoryPropertyFlags memoryPropertyFlags =
        device->GetDeviceInfo().memoryTypes[memoryType].propertyFlags;
    mHostVisible = IsSubset(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memoryPropertyFlags);
    mHostCoherent = IsSubset(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryPropertyFlags);

    // The buffers with mappedAtCreation == true will be initialized in BufferBase::MapAtCreation().
    if (!mappedAtCreation) {
        uint32_t paddingClearSize = Align(GetAllocatedSize() - GetSize(), 4);
        uint64_t paddingClearOffset = GetAllocatedSize() - paddingClearSize;

        if (mHostVisible && GetSize() > 0) {
            // For host visible buffers do initialization on CPU to avoid a GPU write that
            // interferes with using the UploadData() fast path.
            if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
                DAWN_TRY(MapMemoryAndPerformOperation(
                    0, mAllocatedSize,
                    [](std::span<uint8_t> mapped) { std::ranges::fill(mapped, 0x01); }));
            }
            if (device->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse) &&
                paddingClearSize > 0) {
                DAWN_TRY(
                    MapMemoryAndPerformOperation(paddingClearOffset, paddingClearSize,
                                                 [&paddingClearSize](std::span<uint8_t> mapped) {
                                                     DAWN_ASSERT(mapped.size() == paddingClearSize);
                                                     std::ranges::fill(mapped, 0x0);
                                                 }));
            }
        } else {
            if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
                auto scopedUseDuringCreation = UseInternal();
                ClearBuffer(ToBackend(device->GetQueue())->GetPendingRecordingContext(),
                            0x01010101);
            }
            if (device->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse) &&
                paddingClearSize > 0) {
                auto scopedUseDuringCreation = UseInternal();
                CommandRecordingContext* recordingContext =
                    ToBackend(device->GetQueue())->GetPendingRecordingContext();
                ClearBuffer(recordingContext, 0, paddingClearOffset, paddingClearSize);
            }
        }
    }

    SetLabelImpl();

    return {};
}

MaybeError Buffer::InitializeHostMapped(const BufferHostMappedPointer* hostMappedDesc) {
    static constexpr auto kHandleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;

    mAllocatedSize = GetSize();

    VkExternalMemoryBufferCreateInfo externalMemoryCreateInfo;
    externalMemoryCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
    externalMemoryCreateInfo.pNext = nullptr;
    externalMemoryCreateInfo.handleTypes = kHandleType;

    VkBufferCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = &externalMemoryCreateInfo;
    createInfo.flags = 0;
    createInfo.size = mAllocatedSize;
    createInfo.usage = VulkanBufferUsage(GetInternalUsage());
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = 0;

    Device* device = ToBackend(GetDevice());
    DAWN_TRY(CheckVkOOMThenSuccess(
        device->fn.CreateBuffer(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "vkCreateBuffer"));

    // Gather requirements for the buffer's memory and allocate it.
    VkMemoryRequirements requirements;
    device->fn.GetBufferMemoryRequirements(device->GetVkDevice(), mHandle, &requirements);

    // Gather memory requirements from the pointer.
    VkMemoryHostPointerPropertiesEXT hostPointerProperties;
    hostPointerProperties.sType = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT;
    hostPointerProperties.pNext = nullptr;
    DAWN_TRY(CheckVkSuccess(
        device->fn.GetMemoryHostPointerPropertiesEXT(
            device->GetVkDevice(), VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT,
            hostMappedDesc->pointer, &hostPointerProperties),
        "vkGetHostPointerPropertiesEXT"));

    // Merge the memory type requirements from buffer and the host pointer.
    // Don't do this on SwiftShader which reports incompatible memory types even though there
    // is no real Device/Host distinction.
    if (!gpu_info::IsGoogleSwiftshader(GetDevice()->GetPhysicalDevice()->GetVendorId(),
                                       GetDevice()->GetPhysicalDevice()->GetDeviceId())) {
        requirements.memoryTypeBits &= hostPointerProperties.memoryTypeBits;
    }

    // We can choose memory type with `requirements.memoryTypeBits` only because host-mapped memory
    // - is CPU-visible
    // - is device-local on UMA
    // - cannot be non-device-local on non-UMA
    MemoryKind requestKind = MemoryKind::Linear;
    int memoryTypeIndex =
        device->GetResourceMemoryAllocator()->FindBestTypeIndex(requirements, requestKind);
    DAWN_INVALID_IF(memoryTypeIndex < 0, "Failed to find suitable memory type.");

    // Make a device memory wrapping the host pointer.
    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.allocationSize = mAllocatedSize;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;

    VkImportMemoryHostPointerInfoEXT importMemoryHostPointerInfo;
    importMemoryHostPointerInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
    importMemoryHostPointerInfo.pNext = nullptr;
    importMemoryHostPointerInfo.handleType = kHandleType;
    importMemoryHostPointerInfo.pHostPointer = hostMappedDesc->pointer;
    allocateInfo.pNext = &importMemoryHostPointerInfo;

    DAWN_TRY(CheckVkSuccess(device->fn.AllocateMemory(device->GetVkDevice(), &allocateInfo, nullptr,
                                                      &*mDedicatedDeviceMemory),
                            "vkAllocateMemory"));

    // Finally associate it with the buffer.
    DAWN_TRY(CheckVkSuccess(
        device->fn.BindBufferMemory(device->GetVkDevice(), mHandle, mDedicatedDeviceMemory, 0),
        "vkBindBufferMemory"));

    mHostMappedDisposeCallback = hostMappedDesc->disposeCallback;
    mHostMappedDisposeUserdata = hostMappedDesc->userdata;

    SetLabelImpl();

    // Assume the data is initialized since an external pointer was provided.
    SetInitialized(true);
    return {};
}

Buffer::~Buffer() = default;

VkBuffer Buffer::GetHandle() const {
    return mHandle;
}

void Buffer::TransitionUsageNow(CommandRecordingContext* recordingContext,
                                wgpu::BufferUsage usage,
                                wgpu::ShaderStage shaderStage) {
    recordingContext->CheckBufferNeedsEagerTransition(this, usage);
    BufferBarrier barrier = TrackUsageAndGetResourceBarrier(usage, shaderStage);
    recordingContext->EmitBufferBarrierIfNecessary(ToBackend(GetDevice()), barrier);
}

BufferBarrier Buffer::TrackUsageAndGetResourceBarrier(wgpu::BufferUsage usage,
                                                      wgpu::ShaderStage shaderStage) {
    if (shaderStage == wgpu::ShaderStage::None) {
        // If the buffer isn't used in any shader stages, ignore shader usages. Eg. ignore a uniform
        // buffer that isn't actually read in any shader.
        usage &= ~kShaderBufferUsages;
    }

    const bool isMapUsage = usage & kMappableBufferUsages;
    if (isMapUsage) {
        DAWN_ASSERT(shaderStage == wgpu::ShaderStage::None);
        DAWN_ASSERT(IsSubset(usage, kMappableBufferUsages));
        // HOST->GPU barriers aren't required. For MapRead, vkQueueSubmit() happens-after all CPU
        // reads are complete. For MapWrite, the writes are made available to the "Host domain" with
        // vkFlushMappedMemory() (or buffers are coherent) and there is an implicit "Host domain" ->
        // "Device domain" barrier in vkQueueSubmit().
    } else {
        // Request non CPU usage, so assume the buffer will be used in pending commands.
        MarkUsedInPendingCommands();
    }

    const bool readOnly = IsSubset(usage, kReadOnlyBufferUsages);
    VkAccessFlags srcAccess = 0;
    VkPipelineStageFlags srcStage = 0;

    if (readOnly) {
        if ((shaderStage & wgpu::ShaderStage::Fragment) &&
            (mReadShaderStages & wgpu::ShaderStage::Vertex)) {
            // There is an implicit vertex->fragment dependency, so if the vertex stage has already
            // waited, there is no need for fragment to wait. Add the fragment usage so we know to
            // wait for it before the next write.
            mReadShaderStages |= wgpu::ShaderStage::Fragment;
        }

        if (IsSubset(usage, mReadUsage) && IsSubset(shaderStage, mReadShaderStages)) {
            // This usage and shader stage has already waited for the last write.
            // No need for another barrier.
            return {};
        }

        if (usage & kReadOnlyShaderBufferUsages) {
            // Preemptively transition to all read-only shader buffer usages if one is used to
            // avoid unnecessary barriers later.
            usage |= GetInternalUsage() & kReadOnlyShaderBufferUsages;
        }

        if (!isMapUsage) {
            mReadUsage |= usage;
            mReadShaderStages |= shaderStage;
        }

        if (mLastWriteUsage == wgpu::BufferUsage::None) {
            // Read dependency with no prior writes. No barrier needed.
            return {};
        }

        // Write -> read barrier.
        srcAccess = VulkanAccessFlags(mLastWriteUsage);
        srcStage = VulkanPipelineStage(mLastWriteUsage, mLastWriteShaderStage);
    } else {
        bool skipBarrier = false;

        if (mLastWriteUsage == wgpu::BufferUsage::None && mReadUsage == wgpu::BufferUsage::None) {
            // The buffer has never been used so we don't need a barrier.
            skipBarrier = true;
        } else if (mReadUsage == wgpu::BufferUsage::None) {
            // No reads since the last write.
            // Write -> write barrier.
            srcAccess = VulkanAccessFlags(mLastWriteUsage);
            srcStage = VulkanPipelineStage(mLastWriteUsage, mLastWriteShaderStage);
        } else {
            // Read -> write barrier.
            srcAccess = VulkanAccessFlags(mReadUsage);
            srcStage = VulkanPipelineStage(mReadUsage, mReadShaderStages);
        }

        if (!isMapUsage) {
            mLastWriteUsage = usage;
            mLastWriteShaderStage = shaderStage;

            mReadUsage = wgpu::BufferUsage::None;
            mReadShaderStages = wgpu::ShaderStage::None;
        }

        if (skipBarrier) {
            return {};
        }
    }

    return BufferBarrier{.srcAccessMask = srcAccess,
                         .dstAccessMask = VulkanAccessFlags(usage),
                         .srcStages = srcStage,
                         .dstStages = VulkanPipelineStage(usage, shaderStage)};
}

bool Buffer::IsCPUWritableAtCreation() const {
    // TODO(enga): Handle CPU-visible memory on UMA
    return mMemoryAllocation.GetMappedPointer() != nullptr;
}

MaybeError Buffer::MapAtCreationImpl() {
    return {};
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    return {};
}

MaybeError Buffer::FinalizeMapImpl(BufferState newState) {
    Device* device = ToBackend(GetDevice());

    // The real mapped pointer is never returned for zero sized buffers. MappedAtCreation buffers
    // are initialized in BufferBase already.
    if (NeedsInitialization() && GetSize() > 0 && newState == BufferState::Mapped) {
        std::memset(GetMappedPointerImpl(), 0, GetAllocatedSize());
        GetDevice()->IncrementLazyClearCountForTesting();
        SetInitialized(true);

        // If the buffer is non-coherent then make sure either the whole buffer will be flushed
        // later or flush it now.
        if (!mHostCoherent &&
            (MapMode() == wgpu::MapMode::Read || MapOffset() != 0 || MapSize() != GetSize())) {
            VkDeviceSize nonCoherentAtomSize =
                device->GetDeviceInfo().properties.limits.nonCoherentAtomSize;

            VkMappedMemoryRange range =
                GetMappedMemoryRange(mMemoryAllocation, 0, GetAllocatedSize(), nonCoherentAtomSize);

            device->fn.FlushMappedMemoryRanges(device->GetVkDevice(), 1, &range);
        }
    }

    if (!mHostCoherent) {
        // Map reads always require invalidation. Map writes only require invalidation if the buffer
        // contents could have been modified by the GPU previously, eg. it's not being mapped on
        // creation and the buffer is GPU writable.
        if (MapMode() == wgpu::MapMode::Read ||
            (newState != BufferState::MappedAtCreation &&
             !IsSubset(GetInternalUsage(), kReadOnlyBufferUsages | wgpu::BufferUsage::MapWrite))) {
            VkDeviceSize nonCoherentAtomSize =
                device->GetDeviceInfo().properties.limits.nonCoherentAtomSize;

            VkMappedMemoryRange range = GetMappedMemoryRange(mMemoryAllocation, MapOffset(),
                                                             MapSize(), nonCoherentAtomSize);

            device->fn.InvalidateMappedMemoryRanges(device->GetVkDevice(), 1, &range);
        }
    }
    return {};
}

void Buffer::UnmapImpl(BufferState oldState, BufferState newState) {
    // We keep CPU-visible memory mapped at all times but need to flush writes to GPU memory here.
    if (!mHostCoherent && IsMappedState(oldState) && MapMode() == wgpu::MapMode::Write) {
        Device* device = ToBackend(GetDevice());
        VkDeviceSize nonCoherentAtomSize =
            device->GetDeviceInfo().properties.limits.nonCoherentAtomSize;

        VkMappedMemoryRange range =
            GetMappedMemoryRange(mMemoryAllocation, MapOffset(), MapSize(), nonCoherentAtomSize);

        device->fn.FlushMappedMemoryRanges(device->GetVkDevice(), 1, &range);
    }
}

void* Buffer::GetMappedPointerImpl() {
    uint8_t* memory = mMemoryAllocation.GetMappedPointer();
    DAWN_ASSERT(memory != nullptr);
    return memory;
}

MaybeError Buffer::UploadData(uint64_t bufferOffset, const void* data, size_t size) {
    if (size == 0) {
        return {};
    }

    Device* device = ToBackend(GetDevice());

    const bool isInUse = GetLastUsageSerial() > device->GetQueue()->GetCompletedCommandSerial();

    // Check if the buffer might have pending writes on the GPU. Even if the write workload has
    // finished, the write may still need a barrier to make the write available. For MapWrite
    // buffers we know the GPU->HOST barrier was eagerly inserted. For other buffers we don't know
    // if the right barrier was inserted and assume it wasn't.
    const bool hasPendingWrites = !(GetInternalUsage() & wgpu::BufferUsage::MapWrite ||
                                    mLastWriteUsage == wgpu::BufferUsage::None);

    if (isInUse || hasPendingWrites || !mHostVisible) {
        // Write to scratch buffer and copy into final destination buffer.
        return BufferBase::UploadData(bufferOffset, data, size);
    }

    // Buffer does not have any pending uses and is CPU writable. We can map the buffer directly
    // and write the contents, skipping the scratch buffer.

    // If the buffer needs initialization request the full buffer is mapped.
    bool needsZeroInitialization = NeedsInitialization() && size < GetSize();
    uint64_t mapSize = needsZeroInitialization ? mAllocatedSize : size;
    uint64_t mapOffset = needsZeroInitialization ? 0 : bufferOffset;

    return MapMemoryAndPerformOperation(mapOffset, mapSize, [&](std::span<uint8_t> mapped) {
        uint64_t dstOffset = 0;
        if (needsZeroInitialization) {
            DAWN_ASSERT(mapped.size() == mAllocatedSize);
            std::ranges::fill(mapped, 0x0);
            GetDevice()->IncrementLazyClearCountForTesting();
            dstOffset = bufferOffset;
        }
        // The buffer is always initialized here, either by explicit zero initialization
        // above or memcpy below.
        SetInitialized(true);

        DAWN_ASSERT(mapped.size() >= dstOffset + size);
        memcpy(mapped.data() + dstOffset, data, size);
    });
}

template <typename F>
MaybeError Buffer::MapMemoryAndPerformOperation(uint64_t requestedOffset,
                                                size_t requestedSize,
                                                F&& op) {
    Device* device = ToBackend(GetDevice());
    const bool isMappable = GetInternalUsage() & kMappableBufferUsages;

    DAWN_ASSERT(mHostVisible);
    DAWN_ASSERT(GetLastUsageSerial() <= device->GetQueue()->GetCompletedCommandSerial());

    VkDeviceMemory deviceMemory = ToBackend(mMemoryAllocation.GetResourceHeap())->GetMemory();
    uint8_t* memory = nullptr;
    uint64_t realOffset = requestedOffset;

    if (isMappable) {
        // Mappable buffers are already persistently mapped.
        memory = mMemoryAllocation.GetMappedPointer();
    } else {
        // TODO(crbug.com/dawn/774): Persistently map frequently updated buffers instead of
        // mapping/unmapping each time.
        VkDeviceSize offset = mMemoryAllocation.GetOffset();
        VkDeviceSize mapSize = mAllocatedSize;
        if (mHostCoherent) {
            // We can map only the part of the buffer we need to upload the data.
            // We avoid this for non-coherent memory as the mapping needs to be aligned to
            // nonCoherentAtomSize.
            offset += requestedOffset;
            mapSize = requestedSize;
            realOffset = 0;
        }

        void* mappedPointer;
        DAWN_TRY(CheckVkSuccess(device->fn.MapMemory(device->GetVkDevice(), deviceMemory, offset,
                                                     mapSize, 0, &mappedPointer),
                                "vkMapMemory"));
        memory = static_cast<uint8_t*>(mappedPointer);
    }

    VkMappedMemoryRange mappedMemoryRange = {};
    mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedMemoryRange.memory = deviceMemory;
    mappedMemoryRange.offset = mMemoryAllocation.GetOffset();
    mappedMemoryRange.size = mAllocatedSize;
    if (!mHostCoherent) {
        // For non-coherent memory we need to explicitly invalidate the memory range to make
        // available GPU writes visible.
        device->fn.InvalidateMappedMemoryRanges(device->GetVkDevice(), 1, &mappedMemoryRange);
    }

    // Pass a span that is exactly the offset/size requested even if a larger range was mapped.
    op(std::span(memory + realOffset, requestedSize));

    if (!mHostCoherent) {
        // For non-coherent memory we need to explicitly flush the memory range to make the host
        // write visible.
        // TODO(crbug.com/dawn/774): Batch the flush calls instead of doing one per writeBuffer.
        device->fn.FlushMappedMemoryRanges(device->GetVkDevice(), 1, &mappedMemoryRange);
    }

    if (!isMappable) {
        device->fn.UnmapMemory(device->GetVkDevice(), deviceMemory);
    }
    return {};
}

void Buffer::DestroyImpl(DestroyReason reason) {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the buffer is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the buffer.
    // - It may be called when the last ref to the buffer is dropped and the buffer
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the buffer since there are no other live refs.
    BufferBase::DestroyImpl(reason);

    ToBackend(GetDevice())->GetResourceMemoryAllocator()->Deallocate(&mMemoryAllocation);

    if (mHandle != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        mHandle = VK_NULL_HANDLE;
    }

    if (mDedicatedDeviceMemory != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mDedicatedDeviceMemory);
        mDedicatedDeviceMemory = VK_NULL_HANDLE;
    }

    if (mHostMappedDisposeCallback) {
        struct DisposeTask : TrackTaskCallback {
            explicit DisposeTask(wgpu::Callback callback, void* userdata)
                : TrackTaskCallback(nullptr), callback(callback), userdata(userdata) {}
            ~DisposeTask() override = default;

            void FinishImpl() override { callback(userdata); }
            void HandleDeviceLossImpl() override { callback(userdata); }
            void HandleShutDownImpl() override { callback(userdata); }

            wgpu::Callback callback;
            raw_ptr<void, DisableDanglingPtrDetection> userdata;
        };
        std::unique_ptr<DisposeTask> request =
            std::make_unique<DisposeTask>(mHostMappedDisposeCallback, mHostMappedDisposeUserdata);
        mHostMappedDisposeCallback = nullptr;

        GetDevice()->GetQueue()->TrackPendingTask(std::move(request));
    }
}

bool Buffer::EnsureDataInitialized(CommandRecordingContext* recordingContext) {
    if (!NeedsInitialization()) {
        return false;
    }

    InitializeToZero(recordingContext);
    return true;
}

bool Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                                uint64_t offset,
                                                uint64_t size) {
    if (!NeedsInitialization()) {
        return false;
    }

    if (IsFullBufferRange(offset, size)) {
        SetInitialized(true);
        return false;
    }

    InitializeToZero(recordingContext);
    return true;
}

bool Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                                const CopyTextureToBufferCmd* copy) {
    if (!NeedsInitialization()) {
        return false;
    }

    if (IsFullBufferOverwrittenInTextureToBufferCopy(copy)) {
        SetInitialized(true);
        return false;
    }

    InitializeToZero(recordingContext);
    return true;
}

// static
void Buffer::TransitionMappableBuffersEagerly(Device* device,
                                              CommandRecordingContext* recordingContext,
                                              const absl::flat_hash_set<Ref<Buffer>>& buffers) {
    DAWN_ASSERT(!buffers.empty());

    size_t originalBufferCount = buffers.size();

    BufferBarrier barrier;
    for (const Ref<Buffer>& buffer : buffers) {
        wgpu::BufferUsage mapUsage = buffer->GetInternalUsage() & kMappableBufferUsages;
        barrier.Merge(buffer->TrackUsageAndGetResourceBarrier(mapUsage, wgpu::ShaderStage::None));
    }
    // TrackUsageAndGetResourceBarrier() should not modify recordingContext for map usages.
    DAWN_ASSERT(buffers.size() == originalBufferCount);

    recordingContext->EmitBufferBarrierIfNecessary(device, barrier);
}

void Buffer::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_Buffer", GetLabel());
}

void Buffer::InitializeToZero(CommandRecordingContext* recordingContext) {
    DAWN_ASSERT(NeedsInitialization());

    ClearBuffer(recordingContext, 0u);
    GetDevice()->IncrementLazyClearCountForTesting();
    SetInitialized(true);
}

void Buffer::ClearBuffer(CommandRecordingContext* recordingContext,
                         uint32_t clearValue,
                         uint64_t offset,
                         uint64_t size) {
    DAWN_ASSERT(recordingContext != nullptr);
    size = size > 0 ? size : GetAllocatedSize();
    DAWN_ASSERT(size > 0);

    TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopyDst);

    Device* device = ToBackend(GetDevice());
    // VK_WHOLE_SIZE doesn't work on old Windows Intel Vulkan drivers, so we don't use it.
    // Note: Allocated size must be a multiple of 4.
    DAWN_ASSERT(size % 4 == 0);
    device->fn.CmdFillBuffer(recordingContext->commandBuffer, mHandle, offset, size, clearValue);
}

bool BufferBarrier::IsEmpty() const {
    return srcStages == 0 || dstStages == 0;
}

void BufferBarrier::Merge(const BufferBarrier& other) {
    srcAccessMask |= other.srcAccessMask;
    dstAccessMask |= other.dstAccessMask;
    srcStages |= other.srcStages;
    dstStages |= other.dstStages;
}

}  // namespace dawn::native::vulkan
