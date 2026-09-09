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

#ifndef SRC_DAWN_NATIVE_D3D12_RESOURCETABLED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_RESOURCETABLED3D12_H_

#include <memory>
#include <vector>

#include "src/dawn/native/Error.h"
#include "src/dawn/native/ResourceTable.h"
#include "src/dawn/native/d3d12/CPUDescriptorHeapAllocationD3D12.h"
#include "src/dawn/native/d3d12/GPUDescriptorHeapAllocationD3D12.h"

namespace dawn::native::d3d12 {

class Device;
class CommandRecordingContext;
class ShaderVisibleDescriptorAllocator;
class PipelineLayout;

class ResourceTable final : public ResourceTableBase {
  public:
    // Returns descriptor ranges for metadata buffer and unbounded resource arrays. Used to add
    // root descriptor tables to the root signature.
    struct DescriptorRanges {
        std::vector<D3D12_DESCRIPTOR_RANGE1> cbvUavSrvs;
        std::vector<D3D12_DESCRIPTOR_RANGE1> samplers;
    };
    static ResultOrError<DescriptorRanges> GetDescriptorRanges(const PipelineLayout& layout);

    static ResultOrError<Ref<ResourceTable>> Create(Device* device,
                                                    const ResourceTableDescriptor* descriptor);

    // Apply updates to resources or to the metadata buffers that are pending.
    MaybeError ApplyPendingUpdates(CommandRecordingContext* recordingContext,
                                   const absl::flat_hash_set<TextureBase*>& writableTextures);

    // Returns true if the ResourceTable was successfully populated (now or on a previous call)
    // in the current allocator's heap. If false is returned, caller should
    // AllocateAndSwitchShaderVisibleHeap and populate again.
    bool PopulateViews(ShaderVisibleDescriptorAllocator* viewAllocator);
    bool PopulateSamplers(ShaderVisibleDescriptorAllocator* samplerAllocator);

    // Returns the number of view descriptors currently allocated on the CPU heap, and copied to the
    // GPU heap by PopulateViews.
    uint32_t GetViewDescriptorCount() const;

    // Returns the number of sampler descriptors currently allocated on the CPU heap, and copied to
    // the GPU heap by PopulateSamplers.
    uint32_t GetSamplerDescriptorCount() const;

    // Returns a GPU heap handle handle to the base descriptor of the set of descriptors
    // that were copied there in PopulateViews/Samplers. This should be set as a root descriptor
    // table on the command list to match the definition in the root signature.
    D3D12_GPU_DESCRIPTOR_HANDLE GetBaseViewDescriptor() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetBaseSamplerDescriptor() const;

  protected:
    void DestroyImpl(DestroyReason reason) override;
    void SetLabelImpl() override;

  private:
    ~ResourceTable() override;

    using ResourceTableBase::ResourceTableBase;
    MaybeError Initialize();

    struct Heap {
        ComPtr<ID3D12DescriptorHeap> handle;
        CPUDescriptorHeapAllocation cpuAllocation;
        GPUDescriptorHeapAllocation gpuAllocation;
        uint32_t sizeIncrement = 0;
        uint32_t numDescriptors = 0;
    };

    static ResultOrError<Heap> AllocateCPUHeap(Device* device,
                                               D3D12_DESCRIPTOR_HEAP_TYPE heapType,
                                               uint32_t descriptorCount);
    static void FreeCPUHeap(Heap& heap);

    MaybeError TransitionResources(CommandRecordingContext* recordingContext,
                                   const absl::flat_hash_set<TextureBase*>& textures);
    MaybeError UpdateMetadataBuffer(CommandRecordingContext* recordingContext,
                                    const std::vector<MetadataUpdate>& updates);
    MaybeError UpdateResourceBindings(const std::vector<ResourceDiff>& diffs);

    Heap mViewHeap;
    Heap mSamplerHeap;

    using SamplerIndex = TypedInteger<struct SamplerIndexT, uint16_t>;
    static constexpr auto kInvalidSamplerIndex = SamplerIndex{uint16_t{0xFFFF}};

    class SamplerIndexPool;
    std::unique_ptr<SamplerIndexPool> mSamplerIndexPool;

    // Current mapping of resource table slot to sampler index
    ityp::vector<ResourceTableSlot, SamplerIndex> mSlotToSamplerIndex;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_RESOURCETABLED3D12_H_
