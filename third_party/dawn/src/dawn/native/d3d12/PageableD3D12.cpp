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

#include "dawn/native/d3d12/PageableD3D12.h"

#include <utility>

namespace dawn::native::d3d12 {
Pageable::Pageable(ComPtr<ID3D12Pageable> d3d12Pageable, MemorySegment memorySegment, uint64_t size)
    : mD3d12Pageable(std::move(d3d12Pageable)), mMemorySegment(memorySegment), mSize(size) {}

// When a pageable is destroyed, it no longer resides in resident memory, so we must evict
// it from the LRU cache. If this heap is not manually removed from the LRU-cache, the
// ResidencyManager will attempt to use it after it has been deallocated.
Pageable::~Pageable() {
    if (IsInResidencyLRUCache()) {
        RemoveFromList();
    }
}

ID3D12Pageable* Pageable::GetD3D12Pageable() const {
    return mD3d12Pageable.Get();
}

ExecutionSerial Pageable::GetLastUsage() const {
    return mLastUsage;
}

void Pageable::SetLastUsage(ExecutionSerial serial) {
    mLastUsage = serial;
}

ExecutionSerial Pageable::GetLastSubmission() const {
    return mLastSubmission;
}

void Pageable::SetLastSubmission(ExecutionSerial serial) {
    mLastSubmission = serial;
}

MemorySegment Pageable::GetMemorySegment() const {
    return mMemorySegment;
}

uint64_t Pageable::GetSize() const {
    return mSize;
}

bool Pageable::IsInResidencyLRUCache() const {
    return IsInList();
}

void Pageable::IncrementResidencyLock() {
    mResidencyLockRefCount++;
}

void Pageable::DecrementResidencyLock() {
    mResidencyLockRefCount--;
}

bool Pageable::IsResidencyLocked() const {
    return mResidencyLockRefCount != 0;
}
}  // namespace dawn::native::d3d12
