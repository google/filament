// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_QUEUEMOCK_H_
#define SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_QUEUEMOCK_H_

#include "gmock/gmock.h"

#include "dawn/native/Queue.h"

namespace dawn::native {

class DeviceMock;

class QueueMock : public QueueBase {
  public:
    QueueMock(DeviceMock* device, const QueueDescriptor* descriptor);
    ~QueueMock() override;

    MOCK_METHOD(MaybeError, SubmitImpl, (uint32_t, CommandBufferBase* const*), (override));
    MOCK_METHOD(MaybeError,
                WriteBufferImpl,
                (BufferBase*, uint64_t, const void*, size_t),
                (override));
    MOCK_METHOD(MaybeError,
                WriteTextureImpl,
                (const TexelCopyTextureInfo&,
                 const void*,
                 size_t,
                 const TexelCopyBufferLayout&,
                 const Extent3D&),
                (override));
    MOCK_METHOD(void, DestroyImpl, (), (override));

    MOCK_METHOD(ResultOrError<ExecutionSerial>, CheckAndUpdateCompletedSerials, (), (override));
    MOCK_METHOD(bool, HasPendingCommands, (), (const, override));
    MOCK_METHOD(MaybeError, SubmitPendingCommandsImpl, (), (override));
    MOCK_METHOD(void, ForceEventualFlushOfCommands, (), (override));
    MOCK_METHOD(MaybeError, WaitForIdleForDestruction, (), (override));
    MOCK_METHOD(ResultOrError<ExecutionSerial>,
                WaitForQueueSerialImpl,
                (ExecutionSerial, Nanoseconds),
                (override));
};

}  // namespace dawn::native

#endif  // SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_QUEUEMOCK_H_
