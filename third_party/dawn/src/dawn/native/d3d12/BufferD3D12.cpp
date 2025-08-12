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

#include "dawn/native/d3d12/BufferD3D12.h"

#include <algorithm>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/Queue.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/CommandRecordingContext.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/HeapD3D12.h"
#include "dawn/native/d3d12/QueueD3D12.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "dawn/native/d3d12/SharedBufferMemoryD3D12.h"
#include "dawn/native/d3d12/SharedFenceD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::d3d12 {

namespace {
D3D12_RESOURCE_FLAGS D3D12ResourceFlags(wgpu::BufferUsage usage) {
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer)) {
        flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    return flags;
}

D3D12_RESOURCE_STATES D3D12BufferUsage(wgpu::BufferUsage usage) {
    D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

    if (usage & wgpu::BufferUsage::CopySrc) {
        resourceState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
    }
    if (usage & wgpu::BufferUsage::CopyDst) {
        resourceState |= D3D12_RESOURCE_STATE_COPY_DEST;
    }
    if (usage & (wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Uniform)) {
        resourceState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    }
    if (usage & wgpu::BufferUsage::Index) {
        resourceState |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    }
    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer)) {
        resourceState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }
    if (usage & kReadOnlyStorageBuffer) {
        resourceState |= (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                          D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    if (usage & kIndirectBufferForBackendResourceTracking) {
        resourceState |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    }
    if (usage & wgpu::BufferUsage::QueryResolve) {
        resourceState |= D3D12_RESOURCE_STATE_COPY_DEST;
    }

    return resourceState;
}

size_t D3D12BufferSizeAlignment(wgpu::BufferUsage usage) {
    if ((usage & wgpu::BufferUsage::Uniform) != 0) {
        // D3D buffers are always resource size aligned to 64KB. However, D3D12's validation
        // forbids binding a CBV to an unaligned size. To prevent, one can always safely
        // align the buffer size to the CBV data alignment as other buffer usages
        // ignore it (no size check). The validation will still enforce bound checks with
        // the unaligned size returned by GetSize().
        // https://docs.microsoft.com/en-us/windows/win32/direct3d12/uploading-resources#buffer-alignment
        return D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    }
    return 1;
}

ResourceHeapKind GetResourceHeapKind(wgpu::BufferUsage bufferUsage, uint32_t resourceHeapTier) {
    if (bufferUsage == (wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc)) {
        if (resourceHeapTier >= 2) {
            return ResourceHeapKind::Upload_AllBuffersAndTextures;
        } else {
            return ResourceHeapKind::Upload_OnlyBuffers;
        }
    }
    if (bufferUsage == (wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead)) {
        if (resourceHeapTier >= 2) {
            return ResourceHeapKind::Readback_AllBuffersAndTextures;
        } else {
            return ResourceHeapKind::Readback_OnlyBuffers;
        }
    }

    if (bufferUsage & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite)) {
        return ResourceHeapKind::Custom_WriteBack_OnlyBuffers;
    }

    if (resourceHeapTier >= 2) {
        return ResourceHeapKind::Default_AllBuffersAndTextures;
    } else {
        return ResourceHeapKind::Default_OnlyBuffers;
    }
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
    return buffer;
}

// static
ResultOrError<Ref<Buffer>> Buffer::CreateFromSharedBufferMemory(
    SharedBufferMemory* memory,
    const UnpackedPtr<BufferDescriptor>& descriptor) {
    Device* device = ToBackend(memory->GetDevice());
    Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor));
    DAWN_TRY(buffer->InitializeAsExternalBuffer(memory->GetD3DResource(), descriptor));
    buffer->mSharedResourceMemoryContents = memory->GetContents();
    return buffer;
}

MaybeError Buffer::InitializeAsExternalBuffer(ComPtr<ID3D12Resource> d3d12Buffer,
                                              const UnpackedPtr<BufferDescriptor>& descriptor) {
    mAllocatedSize = descriptor->size;
    AllocationInfo info;
    info.mMethod = AllocationMethod::kExternal;
    info.mRequestedSize = mAllocatedSize;
    mResourceAllocation = {info, 0, std::move(d3d12Buffer), nullptr, ResourceHeapKind::InvalidEnum};
    return {};
}

Buffer::Buffer(Device* device, const UnpackedPtr<BufferDescriptor>& descriptor)
    : BufferBase(device, descriptor) {}

MaybeError Buffer::Initialize(bool mappedAtCreation) {
    // Allocate at least 4 bytes so clamped accesses are always in bounds.
    uint64_t size = std::max(GetSize(), uint64_t(4u));
    size_t alignment = D3D12BufferSizeAlignment(GetInternalUsage());
    if (size > std::numeric_limits<uint64_t>::max() - alignment) {
        // Alignment would overflow.
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }
    mAllocatedSize = Align(size, alignment);

    D3D12_RESOURCE_DESC resourceDescriptor;
    resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDescriptor.Alignment = 0;
    resourceDescriptor.Width = mAllocatedSize;
    resourceDescriptor.Height = 1;
    resourceDescriptor.DepthOrArraySize = 1;
    resourceDescriptor.MipLevels = 1;
    resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
    resourceDescriptor.SampleDesc.Count = 1;
    resourceDescriptor.SampleDesc.Quality = 0;
    resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    // Add CopyDst for non-mappable buffer initialization with mappedAtCreation
    // and robust resource initialization.
    resourceDescriptor.Flags = D3D12ResourceFlags(GetInternalUsage() | wgpu::BufferUsage::CopyDst);

    ResourceHeapKind resourceHeapKind =
        GetResourceHeapKind(GetInternalUsage(), ToBackend(GetDevice())->GetResourceHeapTier());
    mLastState = D3D12_RESOURCE_STATE_COMMON;

    switch (resourceHeapKind) {
            // D3D12 requires buffers on the READBACK heap to have the
            // D3D12_RESOURCE_STATE_COPY_DEST state
        case ResourceHeapKind::Readback_AllBuffersAndTextures:
        case ResourceHeapKind::Readback_OnlyBuffers: {
            mLastState |= D3D12_RESOURCE_STATE_COPY_DEST;
            mFixedResourceState = true;
            break;
        }
            // D3D12 requires buffers on the UPLOAD heap to have the
            // D3D12_RESOURCE_STATE_GENERIC_READ state
        case ResourceHeapKind::Upload_AllBuffersAndTextures:
        case ResourceHeapKind::Upload_OnlyBuffers: {
            mLastState |= D3D12_RESOURCE_STATE_GENERIC_READ;
            mFixedResourceState = true;
            break;
        }
        case ResourceHeapKind::Default_AllBuffersAndTextures:
        case ResourceHeapKind::Default_OnlyBuffers:
        case ResourceHeapKind::Custom_WriteBack_OnlyBuffers:
            break;
        default:
            DAWN_UNREACHABLE();
    }

    DAWN_TRY_ASSIGN(mResourceAllocation,
                    ToBackend(GetDevice())
                        ->AllocateMemory(resourceHeapKind, resourceDescriptor, mLastState, 0));

    SetLabelImpl();

    // The buffers with mappedAtCreation == true will be initialized in
    // BufferBase::MapAtCreation().
    if (GetDevice()->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting) &&
        !mappedAtCreation) {
        CommandRecordingContext* commandRecordingContext =
            ToBackend(GetDevice()->GetQueue())->GetPendingCommandContext();
        DAWN_TRY(ClearBuffer(commandRecordingContext, uint8_t(1u)));
    }

    // Initialize the padding bytes to zero.
    if (GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse) && !mappedAtCreation) {
        uint64_t paddingBytes = GetAllocatedSize() - GetSize();
        if (paddingBytes > 0) {
            CommandRecordingContext* commandRecordingContext =
                ToBackend(GetDevice()->GetQueue())->GetPendingCommandContext();

            uint64_t clearSize = paddingBytes;
            uint64_t clearOffset = GetSize();
            DAWN_TRY(ClearBuffer(commandRecordingContext, 0, clearOffset, clearSize));
        }
    }

    return {};
}

MaybeError Buffer::InitializeHostMapped(const BufferHostMappedPointer* hostMappedDesc) {
    Device* device = ToBackend(GetDevice());

    ComPtr<ID3D12Device3> d3d12Device3;
    DAWN_TRY(CheckHRESULT(device->GetD3D12Device()->QueryInterface(IID_PPV_ARGS(&d3d12Device3)),
                          "QueryInterface ID3D12Device3"));

    ComPtr<ID3D12Heap> d3d12Heap;
    DAWN_TRY(CheckOutOfMemoryHRESULT(d3d12Device3->OpenExistingHeapFromAddress(
                                         hostMappedDesc->pointer, IID_PPV_ARGS(&d3d12Heap)),
                                     "ID3D12Device3::OpenExistingHeapFromAddress"));

    uint64_t heapSize = d3d12Heap->GetDesc().SizeInBytes;

    D3D12_RESOURCE_DESC resourceDescriptor;
    resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDescriptor.Alignment = 0;
    resourceDescriptor.Width = GetSize();
    resourceDescriptor.Height = 1;
    resourceDescriptor.DepthOrArraySize = 1;
    resourceDescriptor.MipLevels = 1;
    resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
    resourceDescriptor.SampleDesc.Count = 1;
    resourceDescriptor.SampleDesc.Quality = 0;
    resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDescriptor.Flags =
        D3D12ResourceFlags(GetInternalUsage()) | D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;

    D3D12_RESOURCE_ALLOCATION_INFO resourceInfo =
        device->GetD3D12Device()->GetResourceAllocationInfo(0, 1, &resourceDescriptor);
    DAWN_INVALID_IF(resourceInfo.SizeInBytes > heapSize,
                    "Resource required %u bytes, but heap is %u bytes.", resourceInfo.SizeInBytes,
                    heapSize);
    DAWN_INVALID_IF(!IsPtrAligned(hostMappedDesc->pointer, resourceInfo.Alignment),
                    "Host-mapped pointer (%p) did not satisfy required alignment (%u).",
                    hostMappedDesc->pointer, resourceInfo.Alignment);

    mAllocatedSize = resourceInfo.SizeInBytes;
    mLastState = D3D12_RESOURCE_STATE_COMMON;

    auto heap = std::make_unique<Heap>(
        std::move(d3d12Heap),
        device->GetDeviceInfo().isUMA ? MemorySegment::Local : MemorySegment::NonLocal, GetSize());

    // Consider the imported heap as already resident. Lock it because it is externally allocated.
    device->GetResidencyManager()->TrackResidentAllocation(heap.get());
    DAWN_TRY(device->GetResidencyManager()->LockAllocation(heap.get()));

    ComPtr<ID3D12Resource> placedResource;
    DAWN_TRY(CheckOutOfMemoryHRESULT(
        device->GetD3D12Device()->CreatePlacedResource(heap->GetD3D12Heap(), 0, &resourceDescriptor,
                                                       D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                       IID_PPV_ARGS(&placedResource)),
        "ID3D12Device::CreatePlacedResource"));

    mResourceAllocation = {AllocationInfo{0, AllocationMethod::kExternal, mAllocatedSize}, 0,
                           std::move(placedResource),
                           /* heap is external, and not tracked for residency */ nullptr,
                           ResourceHeapKind::InvalidEnum};
    mHostMappedHeap = std::move(heap);
    mHostMappedDisposeCallback = hostMappedDesc->disposeCallback;
    mHostMappedDisposeUserdata = hostMappedDesc->userdata;

    SetLabelImpl();

    // Assume the data is initialized since an external pointer was provided.
    SetInitialized(true);
    return {};
}

Buffer::~Buffer() = default;

ID3D12Resource* Buffer::GetD3D12Resource() const {
    return mResourceAllocation.GetD3D12Resource();
}

// When true is returned, a D3D12_RESOURCE_BARRIER has been created and must be used in a
// ResourceBarrier call. Failing to do so will cause the tracked state to become invalid and can
// cause subsequent errors.
bool Buffer::TrackUsageAndGetResourceBarrier(CommandRecordingContext* commandContext,
                                             D3D12_RESOURCE_BARRIER* barrier,
                                             wgpu::BufferUsage newUsage) {
    if (mResourceAllocation.GetInfo().mMethod == AllocationMethod::kExternal) {
        commandContext->AddToSharedBufferList(this);
    }

    // Track the underlying heap to ensure residency.
    // There may be no heap if the allocation is an external one.
    Heap* heap = ToBackend(mResourceAllocation.GetResourceHeap());
    if (heap) {
        commandContext->TrackHeapUsage(heap, GetDevice()->GetQueue()->GetPendingCommandSerial());
    }

    MarkUsedInPendingCommands();

    // Resources in upload and readback heaps must be kept in the COPY_SOURCE/DEST state
    if (mFixedResourceState) {
        DAWN_ASSERT((mLastState == D3D12_RESOURCE_STATE_COPY_DEST &&
                     newUsage == wgpu::BufferUsage::CopyDst) ||
                    (mLastState == D3D12_RESOURCE_STATE_GENERIC_READ &&
                     newUsage == wgpu::BufferUsage::CopySrc));
        return false;
    }

    D3D12_RESOURCE_STATES newState = D3D12BufferUsage(newUsage);
    D3D12_RESOURCE_STATES lastState = mLastState;

    // If the transition is from-UAV-to-UAV, then a UAV barrier is needed.
    // If one of the usages isn't UAV, then other barriers are used.
    bool needsUAVBarrier = lastState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS &&
                           newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    if (needsUAVBarrier) {
        barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier->UAV.pResource = GetD3D12Resource();

        mLastState = newState;
        return true;
    }

    // We can skip transitions to already current usages.
    if (IsSubset(newState, mLastState)) {
        return false;
    }

    mLastState = newState;

    // The COMMON state represents a state where no write operations can be pending, which makes
    // it possible to transition to and from some states without synchronization (i.e. without an
    // explicit ResourceBarrier call). A buffer can be implicitly promoted to 1) a single write
    // state, or 2) multiple read states. A buffer that is accessed within a command list will
    // always implicitly decay to the COMMON state after the call to ExecuteCommandLists
    // completes - this is because all buffer writes are guaranteed to be completed before the
    // next ExecuteCommandLists call executes.
    // https://docs.microsoft.com/en-us/windows/desktop/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12#implicit-state-transitions

    // To track implicit decays, we must record the pending serial on which a transition will
    // occur. When that buffer is used again, the previously recorded serial must be compared to
    // the last completed serial to determine if the buffer has implicity decayed to the common
    // state.
    const ExecutionSerial pendingCommandSerial =
        ToBackend(GetDevice())->GetQueue()->GetPendingCommandSerial();
    if (pendingCommandSerial > mLastUsedSerial) {
        lastState = D3D12_RESOURCE_STATE_COMMON;
        mLastUsedSerial = pendingCommandSerial;
    }

    // All possible buffer states used by Dawn are eligible for implicit promotion from COMMON.
    // These are: COPY_SOURCE, VERTEX_AND_COPY_BUFFER, INDEX_BUFFER, COPY_DEST,
    // UNORDERED_ACCESS, and INDIRECT_ARGUMENT. Note that for implicit promotion, the
    // destination state cannot be 1) more than one write state, or 2) both a read and write
    // state. This goes unchecked here because it should not be allowed through render/compute
    // pass validation.
    if (lastState == D3D12_RESOURCE_STATE_COMMON) {
        return false;
    }

    barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier->Transition.pResource = GetD3D12Resource();
    barrier->Transition.StateBefore = lastState;
    barrier->Transition.StateAfter = newState;
    barrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    return true;
}

void Buffer::TrackUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                        wgpu::BufferUsage newUsage) {
    D3D12_RESOURCE_BARRIER barrier;

    if (TrackUsageAndGetResourceBarrier(commandContext, &barrier, newUsage)) {
        commandContext->GetCommandList()->ResourceBarrier(1, &barrier);
    }
}

D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetVA() const {
    return mResourceAllocation.GetGPUPointer();
}

bool Buffer::IsCPUWritableAtCreation() const {
    // We use a staging buffer for the buffers with mappedAtCreation == true and created on the
    // READBACK heap because for the buffers on the READBACK heap, the data written on the CPU
    // side won't be uploaded to GPU. When we enable zero-initialization, the CPU side memory
    // of the buffer is all written to 0 but not the GPU side memory, so on the next mapping
    // operation the zeroes get overwritten by whatever was in the GPU memory when the buffer
    // was created. With a staging buffer, the data on the CPU side will first upload to the
    // staging buffer, and copied from the staging buffer to the GPU memory of the current
    // buffer in the unmap() call.
    // TODO(enga): Handle CPU-visible memory on UMA
    return (GetInternalUsage() & wgpu::BufferUsage::MapWrite) != 0;
}

MaybeError Buffer::MapInternal(bool isWrite, size_t offset, size_t size, const char* contextInfo) {
    DAWN_TRY(SynchronizeBufferBeforeMapping());

    // The mapped buffer can be accessed at any time, so it must be locked to ensure it is never
    // evicted. This buffer should already have been made resident when it was created.
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "BufferD3D12::MapInternal");

    if (mResourceAllocation.GetInfo().mMethod != AllocationMethod::kExternal) {
        Heap* heap = ToBackend(mResourceAllocation.GetResourceHeap());
        DAWN_TRY(ToBackend(GetDevice())->GetResidencyManager()->LockAllocation(heap));
    }

    D3D12_RANGE range = {offset, offset + size};
    // mMappedData is the pointer to the start of the resource, irrespective of offset.
    // MSDN says (note the weird use of "never"):
    //
    //   When ppData is not NULL, the pointer returned is never offset by any values in
    //   pReadRange.
    //
    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12resource-map
    DAWN_TRY(CheckHRESULT(GetD3D12Resource()->Map(0, &range, &mMappedData), contextInfo));

    if (isWrite) {
        mWrittenMappedRange = range;
    }

    return {};
}

MaybeError Buffer::MapAtCreationImpl() {
    // We will use a staging buffer for MapRead buffers instead so we just clear the staging
    // buffer and initialize the original buffer by copying the staging buffer to the original
    // buffer one the first time Unmap() is called.
    DAWN_ASSERT((GetInternalUsage() & wgpu::BufferUsage::MapWrite) != 0);

    // The buffers with mappedAtCreation == true will be initialized in
    // BufferBase::MapAtCreation().
    DAWN_TRY(MapInternal(true, 0, size_t(GetAllocatedSize()), "D3D12 map at creation"));

    return {};
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    // GetPendingCommandContext() call might create a new commandList. Dawn will handle
    // it in Tick() by execute the commandList and signal a fence for it even it is empty.
    // Skip the unnecessary GetPendingCommandContext() call saves an extra fence.
    if (NeedsInitialization()) {
        CommandRecordingContext* commandContext =
            ToBackend(GetDevice()->GetQueue())->GetPendingCommandContext();
        DAWN_TRY(EnsureDataInitialized(commandContext));
    }

    return MapInternal(mode & wgpu::MapMode::Write, offset, size, "D3D12 map async");
}

void Buffer::UnmapImpl() {
    GetD3D12Resource()->Unmap(0, &mWrittenMappedRange);
    mMappedData = nullptr;
    mWrittenMappedRange = {0, 0};

    // When buffers are mapped, they are locked to keep them in resident memory. We must unlock
    // them when they are unmapped.
    if (mResourceAllocation.GetInfo().mMethod != AllocationMethod::kExternal) {
        Heap* heap = ToBackend(mResourceAllocation.GetResourceHeap());
        ToBackend(GetDevice())->GetResidencyManager()->UnlockAllocation(heap);
    }
}

void* Buffer::GetMappedPointerImpl() {
    // The frontend asks that the pointer returned is from the start of the resource
    // irrespective of the offset passed in MapAsyncImpl, which is what mMappedData is.
    return mMappedData;
}

void Buffer::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the buffer is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the buffer.
    // - It may be called when the last ref to the buffer is dropped and the buffer
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the buffer since there are no other live refs.
    if (mMappedData != nullptr) {
        // If the buffer is currently mapped, unmap without flushing the writes to the GPU
        // since the buffer cannot be used anymore. UnmapImpl checks mWrittenRange to know
        // which parts to flush, so we set it to an empty range to prevent flushes.
        mWrittenMappedRange = {0, 0};
    }
    BufferBase::DestroyImpl();

    ToBackend(GetDevice())->DeallocateMemory(mResourceAllocation);

    if (mHostMappedDisposeCallback) {
        struct DisposeTask : TrackTaskCallback {
            DisposeTask(std::unique_ptr<Heap> heap, wgpu::Callback callback, void* userdata)
                : TrackTaskCallback(nullptr),
                  heap(std::move(heap)),
                  callback(callback),
                  userdata(userdata) {}
            ~DisposeTask() override = default;

            void FinishImpl() override {
                heap = nullptr;
                callback(userdata);
            }
            void HandleDeviceLossImpl() override {
                heap = nullptr;
                callback(userdata);
            }
            void HandleShutDownImpl() override {
                heap = nullptr;
                callback(userdata);
            }

            std::unique_ptr<Heap> heap;
            wgpu::Callback callback;
            raw_ptr<void, DisableDanglingPtrDetection> userdata;
        };
        std::unique_ptr<DisposeTask> request = std::make_unique<DisposeTask>(
            std::move(mHostMappedHeap), mHostMappedDisposeCallback, mHostMappedDisposeUserdata);
        mHostMappedDisposeCallback = nullptr;
        mHostMappedHeap = nullptr;

        GetDevice()->GetQueue()->TrackPendingTask(std::move(request));
    }
}

bool Buffer::CheckIsResidentForTesting() const {
    Heap* heap = ToBackend(mResourceAllocation.GetResourceHeap());
    return heap->IsInList() || heap->IsResidencyLocked();
}

bool Buffer::CheckAllocationMethodForTesting(AllocationMethod allocationMethod) const {
    return mResourceAllocation.GetInfo().mMethod == allocationMethod;
}

MaybeError Buffer::EnsureDataInitialized(CommandRecordingContext* commandContext) {
    if (!NeedsInitialization()) {
        return {};
    }

    DAWN_TRY(InitializeToZero(commandContext));
    return {};
}

ResultOrError<bool> Buffer::EnsureDataInitializedAsDestination(
    CommandRecordingContext* commandContext,
    uint64_t offset,
    uint64_t size) {
    if (!NeedsInitialization()) {
        return {false};
    }

    if (IsFullBufferRange(offset, size)) {
        SetInitialized(true);
        return {false};
    }

    DAWN_TRY(InitializeToZero(commandContext));
    return {true};
}

MaybeError Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                      const CopyTextureToBufferCmd* copy) {
    if (!NeedsInitialization()) {
        return {};
    }

    if (IsFullBufferOverwrittenInTextureToBufferCopy(copy)) {
        SetInitialized(true);
    } else {
        DAWN_TRY(InitializeToZero(commandContext));
    }

    return {};
}

MaybeError Buffer::SynchronizeBufferBeforeMapping() {
    // Buffers imported with the SharedBufferMemory feature can include fences that must finish
    // before Dawn can use the buffer for mapping operations on the CPU timeline. We acquire and
    // wait for them here.
    if (SharedResourceMemoryContents* contents = GetSharedResourceMemoryContents()) {
        SharedBufferMemoryBase::PendingFenceList fences;
        contents->AcquirePendingFences(&fences);
        for (const auto& fence : fences) {
            HANDLE fenceEvent = 0;
            ComPtr<ID3D12Fence> d3dFence = ToBackend(fence.object)->GetD3DFence();
            if (d3dFence->GetCompletedValue() < fence.signaledValue) {
                d3dFence->SetEventOnCompletion(fence.signaledValue, fenceEvent);
                WaitForSingleObject(fenceEvent, INFINITE);
            }
        }
    }

    return {};
}

MaybeError Buffer::SynchronizeBufferBeforeUseOnGPU() {
    // Buffers imported with the SharedBufferMemory feature can include fences that must finish
    // before Dawn can use the buffer on the GPU timeline. We acquire and wait for them here.
    if (SharedResourceMemoryContents* contents = GetSharedResourceMemoryContents()) {
        Device* device = ToBackend(GetDevice());
        Queue* queue = ToBackend(device->GetQueue());

        SharedBufferMemoryBase::PendingFenceList fences;
        contents->AcquirePendingFences(&fences);

        ID3D12CommandQueue* commandQueue = queue->GetCommandQueue();
        for (const auto& fence : fences) {
            DAWN_TRY(CheckHRESULT(commandQueue->Wait(ToBackend(fence.object)->GetD3DFence(),
                                                     fence.signaledValue),
                                  "D3D12 fence wait"););
            // Keep D3D12 fence alive until commands complete.
            device->ReferenceUntilUnused(ToBackend(fence.object)->GetD3DFence());
        }

        mLastUsageSerial = queue->GetPendingCommandSerial();
    }

    return {};
}

void Buffer::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mResourceAllocation.GetD3D12Resource(), "Dawn_Buffer",
                 GetLabel());
}

MaybeError Buffer::InitializeToZero(CommandRecordingContext* commandContext) {
    DAWN_ASSERT(NeedsInitialization());

    // TODO(crbug.com/dawn/484): skip initializing the buffer when it is created on a heap
    // that has already been zero initialized.
    DAWN_TRY(ClearBuffer(commandContext, uint8_t(0u)));
    SetInitialized(true);
    GetDevice()->IncrementLazyClearCountForTesting();

    return {};
}

MaybeError Buffer::ClearBuffer(CommandRecordingContext* commandContext,
                               uint8_t clearValue,
                               uint64_t offset,
                               uint64_t size) {
    Device* device = ToBackend(GetDevice());
    size = size > 0 ? size : GetAllocatedSize();

    if (GetInternalUsage() & wgpu::BufferUsage::MapWrite) {
        DAWN_TRY(MapInternal(true, static_cast<size_t>(offset), static_cast<size_t>(size),
                             "D3D12 map at clear buffer"));
        memset(mMappedData, clearValue, size);
        UnmapImpl();
    } else if (clearValue == 0u) {
        DAWN_TRY(device->ClearBufferToZero(commandContext, this, offset, size));
    } else {
        // TODO(crbug.com/dawn/852): use ClearUnorderedAccessView*() when the buffer usage
        // includes STORAGE.
        DAWN_TRY(device->GetDynamicUploader()->WithUploadReservation(
            size, kCopyBufferToBufferOffsetAlignment,
            [&](UploadReservation reservation) -> MaybeError {
                memset(reservation.mappedPointer, clearValue, size);
                device->CopyFromStagingToBufferHelper(commandContext, reservation.buffer.Get(),
                                                      reservation.offsetInBuffer, this, offset,
                                                      size);
                return {};
            }));
    }

    return {};
}
}  // namespace dawn::native::d3d12
