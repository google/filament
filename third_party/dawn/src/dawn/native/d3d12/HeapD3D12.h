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

#ifndef SRC_DAWN_NATIVE_D3D12_HEAPD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_HEAPD3D12_H_

#include "dawn/native/ResourceHeap.h"
#include "dawn/native/d3d12/PageableD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;

// This class is used to represent ID3D12Heap allocations, as well as an implicit heap
// representing a directly allocated resource. It inherits from Pageable because each Heap must
// be represented in the ResidencyManager.
class Heap : public ResourceHeapBase, public Pageable {
  public:
    Heap(ComPtr<ID3D12Pageable> d3d12Pageable, MemorySegment memorySegment, uint64_t size);

    ID3D12Heap* GetD3D12Heap() const;

  private:
    ComPtr<ID3D12Heap> mD3d12Heap;
};
}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_HEAPD3D12_H_
