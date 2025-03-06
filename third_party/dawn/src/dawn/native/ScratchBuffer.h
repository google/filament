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

#ifndef SRC_DAWN_NATIVE_SCRATCHBUFFER_H_
#define SRC_DAWN_NATIVE_SCRATCHBUFFER_H_

#include <cstdint>

#include "dawn/common/Ref.h"
#include "dawn/native/Buffer.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

class DeviceBase;

// A ScratchBuffer is a lazily allocated and lazily grown GPU buffer for intermittent use by
// commands in the GPU queue. Note that scratch buffers are not zero-initialized, so users must
// be careful not to exposed uninitialized bytes to client shaders.
class ScratchBuffer {
  public:
    // Note that this object does not retain a reference to `device`, so `device` MUST outlive
    // this object.
    ScratchBuffer(DeviceBase* device, wgpu::BufferUsage usage);
    ~ScratchBuffer();

    // Resets this ScratchBuffer, guaranteeing that the next EnsureCapacity call allocates a
    // fresh buffer.
    void Reset();

    // Ensures that this ScratchBuffer is backed by a buffer on `device` with at least
    // `capacity` bytes of storage.
    MaybeError EnsureCapacity(uint64_t capacity);

    BufferBase* GetBuffer() const;

  private:
    const raw_ptr<DeviceBase> mDevice;
    const wgpu::BufferUsage mUsage;
    Ref<BufferBase> mBuffer;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SCRATCHBUFFER_H_
