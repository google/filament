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

#include <limits>

#include "dawn/common/Assert.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

void Server::OnQueueWorkDone(QueueWorkDoneUserdata* data, WGPUQueueWorkDoneStatus status) {
    ReturnQueueWorkDoneCallbackCmd cmd;
    cmd.eventManager = data->eventManager;
    cmd.future = data->future;
    cmd.status = status;

    SerializeCommand(cmd);
}

WireResult Server::DoQueueOnSubmittedWorkDone(Known<WGPUQueue> queue,
                                              ObjectHandle eventManager,
                                              WGPUFuture future) {
    auto userdata = MakeUserdata<QueueWorkDoneUserdata>();
    userdata->queue = queue.AsHandle();
    userdata->eventManager = eventManager;
    userdata->future = future;

    mProcs.queueOnSubmittedWorkDone(
        queue->handle, {nullptr, WGPUCallbackMode_AllowProcessEvents,
                        ForwardToServer2<&Server::OnQueueWorkDone>, userdata.release(), nullptr});
    return WireResult::Success;
}

WireResult Server::DoQueueWriteBuffer(Known<WGPUQueue> queue,
                                      Known<WGPUBuffer> buffer,
                                      uint64_t bufferOffset,
                                      const uint8_t* data,
                                      uint64_t size) {
    if (size > std::numeric_limits<size_t>::max()) {
        return WireResult::FatalError;
    }

    mProcs.queueWriteBuffer(queue->handle, buffer->handle, bufferOffset, data,
                            static_cast<size_t>(size));
    return WireResult::Success;
}

WireResult Server::DoQueueWriteTexture(Known<WGPUQueue> queue,
                                       const WGPUTexelCopyTextureInfo* destination,
                                       const uint8_t* data,
                                       uint64_t dataSize,
                                       const WGPUTexelCopyBufferLayout* dataLayout,
                                       const WGPUExtent3D* writeSize) {
    if (dataSize > std::numeric_limits<size_t>::max()) {
        return WireResult::FatalError;
    }

    mProcs.queueWriteTexture(queue->handle, destination, data, static_cast<size_t>(dataSize),
                             dataLayout, writeSize);
    return WireResult::Success;
}

}  // namespace dawn::wire::server
