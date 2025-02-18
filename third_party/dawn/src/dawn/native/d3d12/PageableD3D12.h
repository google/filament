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

#ifndef SRC_DAWN_NATIVE_D3D12_PAGEABLED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_PAGEABLED3D12_H_

#include "dawn/common/LinkedList.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {
// This class is used to represent ID3D12Pageable allocations, and also serves as a node within
// the ResidencyManager's LRU cache. This node is inserted into the LRU-cache when it is first
// allocated, and any time it is scheduled to be used by the GPU. This node is removed from the
// LRU cache when it is evicted from resident memory due to budget constraints, or when the
// pageable allocation is released.
class Pageable : public LinkNode<Pageable> {
  public:
    Pageable(ComPtr<ID3D12Pageable> d3d12Pageable, MemorySegment memorySegment, uint64_t size);
    ~Pageable();

    ID3D12Pageable* GetD3D12Pageable() const;

    // We set mLastRecordingSerial to denote the serial this pageable was last recorded to be
    // used. We must check this serial against the current serial when recording usages to
    // ensure we do not process residency for this pageable multiple times.
    ExecutionSerial GetLastUsage() const;
    void SetLastUsage(ExecutionSerial serial);

    // The residency manager must know the last serial that any portion of the pageable was
    // submitted to be used so that we can ensure this pageable stays resident in memory at
    // least until that serial has completed.
    ExecutionSerial GetLastSubmission() const;
    void SetLastSubmission(ExecutionSerial serial);

    MemorySegment GetMemorySegment() const;

    uint64_t GetSize() const;

    bool IsInResidencyLRUCache() const;

    // In some scenarios, such as async buffer mapping or descriptor heaps, we must lock
    // residency to ensure the pageable cannot be evicted. Because multiple buffers may be
    // mapped in a single heap, we must track the number of resources currently locked.
    void IncrementResidencyLock();
    void DecrementResidencyLock();
    bool IsResidencyLocked() const;

  protected:
    ComPtr<ID3D12Pageable> mD3d12Pageable;

  private:
    // mLastUsage denotes the last time this pageable was recorded for use.
    ExecutionSerial mLastUsage = ExecutionSerial(0);
    // mLastSubmission denotes the last time this pageable was submitted to the GPU. Note that
    // although this variable often contains the same value as mLastUsage, it can differ in some
    // situations. When some asynchronous APIs (like WriteBuffer) are called, mLastUsage is
    // updated upon the call, but the backend operation is deferred until the next submission
    // to the GPU. This makes mLastSubmission unique from mLastUsage, and allows us to
    // accurately identify when a pageable can be evicted.
    ExecutionSerial mLastSubmission = ExecutionSerial(0);
    MemorySegment mMemorySegment;
    uint32_t mResidencyLockRefCount = 0;
    uint64_t mSize = 0;
};
}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_PAGEABLED3D12_H_
