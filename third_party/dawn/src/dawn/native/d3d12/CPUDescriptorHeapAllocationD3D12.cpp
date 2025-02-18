// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/native/d3d12/CPUDescriptorHeapAllocationD3D12.h"
#include "dawn/native/Error.h"

namespace dawn::native::d3d12 {

CPUDescriptorHeapAllocation::CPUDescriptorHeapAllocation(D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor,
                                                         uint32_t heapIndex)
    : mBaseDescriptor(baseDescriptor), mHeapIndex(heapIndex) {}

D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHeapAllocation::GetBaseDescriptor() const {
    DAWN_ASSERT(IsValid());
    return mBaseDescriptor;
}

D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHeapAllocation::OffsetFrom(
    uint32_t sizeIncrementInBytes,
    uint32_t offsetInDescriptorCount) const {
    DAWN_ASSERT(IsValid());
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mBaseDescriptor;
    cpuHandle.ptr += sizeIncrementInBytes * offsetInDescriptorCount;
    return cpuHandle;
}

uint32_t CPUDescriptorHeapAllocation::GetHeapIndex() const {
    DAWN_ASSERT(mHeapIndex >= 0);
    return mHeapIndex;
}

bool CPUDescriptorHeapAllocation::IsValid() const {
    return mBaseDescriptor.ptr != 0;
}

void CPUDescriptorHeapAllocation::Invalidate() {
    mBaseDescriptor = {0};
}

}  // namespace dawn::native::d3d12
