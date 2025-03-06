// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_RINGBUFFERALLOCATOR_H_
#define SRC_DAWN_NATIVE_RINGBUFFERALLOCATOR_H_

#include <limits>
#include <memory>

#include "dawn/common/SerialQueue.h"
#include "dawn/native/IntegerTypes.h"

// RingBufferAllocator is the front-end implementation used to manage a ring buffer in GPU memory.
namespace dawn::native {

class RingBufferAllocator {
  public:
    RingBufferAllocator();
    explicit RingBufferAllocator(uint64_t maxSize);
    RingBufferAllocator(const RingBufferAllocator&);
    ~RingBufferAllocator();

    RingBufferAllocator& operator=(const RingBufferAllocator&);

    uint64_t Allocate(uint64_t allocationSize,
                      ExecutionSerial serial,
                      uint64_t offsetAlignment = 1);
    void Deallocate(ExecutionSerial lastCompletedSerial);

    uint64_t GetSize() const;
    bool Empty() const;
    uint64_t GetUsedSize() const;

    static constexpr uint64_t kInvalidOffset = std::numeric_limits<uint64_t>::max();

  private:
    struct Request {
        uint64_t endOffset;
        uint64_t size;
    };

    SerialQueue<ExecutionSerial, Request> mInflightRequests;  // Queue of the recorded sub-alloc
                                                              // requests (e.g. frame of resources).

    uint64_t mUsedEndOffset = 0;    // Tail of used sub-alloc requests (in bytes).
    uint64_t mUsedStartOffset = 0;  // Head of used sub-alloc requests (in bytes).
    uint64_t mMaxBlockSize = 0;     // Max size of the ring buffer (in bytes).
    uint64_t mUsedSize = 0;         // Size of the sub-alloc requests (in bytes) of the ring buffer.
};
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RINGBUFFERALLOCATOR_H_
