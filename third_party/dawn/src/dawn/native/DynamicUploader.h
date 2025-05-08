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

#ifndef SRC_DAWN_NATIVE_DYNAMICUPLOADER_H_
#define SRC_DAWN_NATIVE_DYNAMICUPLOADER_H_

#include <memory>
#include <vector>

#include "dawn/common/Ref.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/RingBufferAllocator.h"
#include "partition_alloc/pointers/raw_ptr.h"

// DynamicUploader is the front-end implementation used to manage multiple ring buffers for upload
// usage.
namespace dawn::native {

class BufferBase;

struct UploadReservation {
    void* mappedPointer = nullptr;
    uint64_t offsetInBuffer = 0;
    Ref<BufferBase> buffer;
};

class DynamicUploader {
  public:
    explicit DynamicUploader(DeviceBase* device);
    ~DynamicUploader() = default;

    // Transiently makes a reservation for an upload area for the functor passed in argument.
    template <typename F>
    MaybeError WithUploadReservation(uint64_t size, uint64_t offsetAlignment, F&& f) {
        UploadReservation reservation;
        DAWN_TRY_ASSIGN(reservation, Reserve(size, offsetAlignment));
        return f(reservation);
    }

    void Deallocate(ExecutionSerial lastCompletedSerial, bool freeAll = false);

    bool ShouldFlush() const;

  private:
    static constexpr uint64_t kRingBufferSize = 4 * 1024 * 1024;
    uint64_t GetTotalAllocatedSize() const;

    struct RingBuffer {
        Ref<BufferBase> mStagingBuffer;
        RingBufferAllocator mAllocator;
    };

    ResultOrError<UploadReservation> Reserve(uint64_t size, uint64_t offsetAlignment);

    std::vector<std::unique_ptr<RingBuffer>> mRingBuffers;
    raw_ptr<DeviceBase> mDevice;
};
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_DYNAMICUPLOADER_H_
