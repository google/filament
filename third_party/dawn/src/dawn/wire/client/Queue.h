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

#ifndef SRC_DAWN_WIRE_CLIENT_QUEUE_H_
#define SRC_DAWN_WIRE_CLIENT_QUEUE_H_

#include <webgpu/webgpu.h>

#include <atomic>

#include "dawn/wire/WireClient.h"
#include "dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

class Queue final : public ObjectWithEventsBase {
  public:
    using ObjectWithEventsBase::ObjectWithEventsBase;
    ~Queue() override;

    ObjectType GetObjectType() const override;

    uint64_t GetLastSubmitIndex() const;
    uint64_t GetCompletedSubmitIndex() const;

    // Dawn API
    void APISubmit(size_t commandCount, const WGPUCommandBuffer* commands);
    WGPUFuture APIOnSubmittedWorkDone(const WGPUQueueWorkDoneCallbackInfo& callbackInfo);
    void APIWriteBuffer(WGPUBuffer cBuffer, uint64_t bufferOffset, const void* data, size_t size);
    void APIWriteTexture(const WGPUTexelCopyTextureInfo* destination,
                         const void* data,
                         size_t dataSize,
                         const WGPUTexelCopyBufferLayout* dataLayout,
                         const WGPUExtent3D* writeSize);

  private:
    void WriteBufferXL(WGPUBuffer cBuffer, uint64_t bufferOffset, const void* data, size_t size);
    void WriteTextureXL(const WGPUTexelCopyTextureInfo* destination,
                        const void* data,
                        size_t dataSize,
                        const WGPUTexelCopyBufferLayout* dataLayout,
                        const WGPUExtent3D* writeSize);

    uint64_t mLastSubmitIndex = 0;
    std::atomic<uint64_t> mCompletedSubmitIndex = 0;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_QUEUE_H_
