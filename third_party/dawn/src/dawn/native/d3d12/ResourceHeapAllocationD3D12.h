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

#ifndef SRC_DAWN_NATIVE_D3D12_RESOURCEHEAPALLOCATIOND3D12_H_
#define SRC_DAWN_NATIVE_D3D12_RESOURCEHEAPALLOCATIOND3D12_H_

#include "dawn/native/Error.h"
#include "dawn/native/ResourceMemoryAllocation.h"
#include "dawn/native/d3d12/ResourceAllocatorManagerD3D12.h"

namespace dawn::native::d3d12 {

class Heap;

class ResourceHeapAllocation : public ResourceMemoryAllocation {
  public:
    ResourceHeapAllocation() = default;
    ResourceHeapAllocation(const AllocationInfo& info,
                           uint64_t offset,
                           ComPtr<ID3D12Resource> resource,
                           Heap* heap,
                           ResourceHeapKind resourceHeapKind);
    ~ResourceHeapAllocation() override = default;
    ResourceHeapAllocation(const ResourceHeapAllocation&) = default;
    ResourceHeapAllocation& operator=(const ResourceHeapAllocation&) = default;

    void Invalidate() override;

    ID3D12Resource* GetD3D12Resource() const;
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUPointer() const;
    ResourceHeapKind GetResourceHeapKind() const;

  private:
    ComPtr<ID3D12Resource> mResource;
    ResourceHeapKind mResourceHeapKind;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_RESOURCEHEAPALLOCATIOND3D12_H_
