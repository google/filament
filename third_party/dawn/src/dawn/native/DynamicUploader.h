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

#include "dawn/common/NonMovable.h"
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

class DynamicUploader : NonMovable {
  public:
    explicit DynamicUploader(DeviceBase* device);
    ~DynamicUploader() = default;

    // Transiently makes a reservation for an upload area for the functor passed in argument.
    template <typename F>
    MaybeError WithUploadReservation(uint64_t size, uint64_t offsetAlignment, F&& f) {
        UploadReservation reservation;
        DAWN_TRY_ASSIGN(reservation, Reserve(size, offsetAlignment));
        DAWN_TRY(f(reservation));
        return OnStagingMemoryFreePendingOnSubmit(size);
    }

    // Notifies the dynamic uploader that some freeing of memory is associated with the pending
    // submit. The dynamic uploader may take some action in this case, like forcing an early submit.
    MaybeError OnStagingMemoryFreePendingOnSubmit(uint64_t size);

    void Deallocate(ExecutionSerial lastCompletedSerial, bool freeAll = false);

  private:
    ResultOrError<UploadReservation> Reserve(uint64_t size, uint64_t offsetAlignment);

    struct RingBuffer {
        Ref<BufferBase> mStagingBuffer;
        RingBufferAllocator mAllocator;
    };
    std::vector<std::unique_ptr<RingBuffer>> mRingBuffers;

    // Serial used to track when a serial has been scheduled and the corresponding pending memory
    // will be freed in finite time.
    ExecutionSerial mLastPendingSerialSeen = kBeginningOfGPUTime;
    uint64_t mMemoryPendingSubmit = 0;
    raw_ptr<DeviceBase> mDevice;
};
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_DYNAMICUPLOADER_H_
