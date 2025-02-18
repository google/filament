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

#include "dawn/tests/unittests/native/mocks/QueueMock.h"

#include "dawn/tests/unittests/native/mocks/DeviceMock.h"

using testing::WithArgs;

namespace dawn::native {

QueueMock::QueueMock(DeviceMock* device, const QueueDescriptor* descriptor)
    : QueueBase(device, descriptor) {
    ON_CALL(*this, DestroyImpl).WillByDefault([this] { this->QueueBase::DestroyImpl(); });
    ON_CALL(*this, SubmitImpl)
        .WillByDefault([this](uint32_t, CommandBufferBase* const*) -> MaybeError {
            this->QueueBase::IncrementLastSubmittedCommandSerial();
            return {};
        });
    ON_CALL(*this, CheckAndUpdateCompletedSerials)
        .WillByDefault([this]() -> ResultOrError<ExecutionSerial> {
            return this->QueueBase::GetLastSubmittedCommandSerial();
        });
    ON_CALL(*this, WriteBufferImpl)
        .WillByDefault(WithArgs<0, 1, 2, 3>([this](BufferBase* buffer, uint64_t bufferOffset,
                                                   const void* data, size_t size) -> MaybeError {
            return this->QueueBase::WriteBufferImpl(buffer, bufferOffset, data, size);
        }));
}

QueueMock::~QueueMock() = default;

}  // namespace dawn::native
