// Copyright 2022 The Dawn & Tint Authors
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

#include "dawn/tests/unittests/native/mocks/BufferMock.h"

#include <memory>

#include "dawn/native/ChainUtils.h"

namespace dawn::native {

using ::testing::Return;

BufferMock::BufferMock(DeviceMock* device,
                       const UnpackedPtr<BufferDescriptor>& descriptor,
                       std::optional<uint64_t> allocatedSize)
    : BufferBase(device, descriptor) {
    mAllocatedSize = allocatedSize.value_or(GetSize());
    DAWN_ASSERT(mAllocatedSize >= GetSize());
    mBackingData = std::unique_ptr<uint8_t[]>(new uint8_t[mAllocatedSize]);

    ON_CALL(*this, DestroyImpl).WillByDefault([this] { this->BufferBase::DestroyImpl(); });
    ON_CALL(*this, GetMappedPointerImpl).WillByDefault(Return(mBackingData.get()));
    ON_CALL(*this, IsCPUWritableAtCreation).WillByDefault([this] {
        return (GetInternalUsage() & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite)) !=
               0;
    });
}

BufferMock::BufferMock(DeviceMock* device,
                       const BufferDescriptor* descriptor,
                       std::optional<uint64_t> allocatedSize)
    : BufferMock(device, Unpack(descriptor), allocatedSize) {}

BufferMock::~BufferMock() = default;

}  // namespace dawn::native
