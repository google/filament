// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_BUFFERMOCK_H_
#define SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_BUFFERMOCK_H_

#include <memory>

#include "gmock/gmock.h"

#include "dawn/native/Buffer.h"
#include "dawn/tests/unittests/native/mocks/DeviceMock.h"

namespace dawn::native {

class BufferMock : public BufferBase {
  public:
    BufferMock(DeviceMock* device,
               const UnpackedPtr<BufferDescriptor>& descriptor,
               std::optional<uint64_t> allocatedSize = std::nullopt);
    BufferMock(DeviceMock* device,
               const BufferDescriptor* descriptor,
               std::optional<uint64_t> allocatedSize = std::nullopt);
    ~BufferMock() override;

    MOCK_METHOD(void, DestroyImpl, (), (override));

    MOCK_METHOD(MaybeError, MapAtCreationImpl, (), (override));
    MOCK_METHOD(MaybeError,
                MapAsyncImpl,
                (wgpu::MapMode mode, size_t offset, size_t size),
                (override));
    MOCK_METHOD(void, UnmapImpl, (), (override));
    MOCK_METHOD(void*, GetMappedPointer, (), (override));

    MOCK_METHOD(bool, IsCPUWritableAtCreation, (), (const, override));

  private:
    std::unique_ptr<uint8_t[]> mBackingData;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_BUFFERMOCK_H_
