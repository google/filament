// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/opengl/BufferGL.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/UtilsGL.h"

namespace dawn::native::opengl {

// Buffer

// static
ResultOrError<Ref<Buffer>> Buffer::CreateInternalBuffer(Device* device,
                                                        const BufferDescriptor* descriptor,
                                                        bool shouldLazyClear) {
    Ref<Buffer> buffer;
    DAWN_TRY_ASSIGN(buffer, Buffer::Create(device, Unpack(descriptor)));

    if (!shouldLazyClear) {
        buffer->SetInitialized(true);
    }

    if (descriptor->mappedAtCreation) {
        [[maybe_unused]] bool usingStagingBuffer;
        DAWN_TRY_ASSIGN(usingStagingBuffer, buffer->MapAtCreationInternal());
    }

    return std::move(buffer);
}

// static
ResultOrError<Ref<Buffer>> Buffer::Create(Device* device,
                                          const UnpackedPtr<BufferDescriptor>& descriptor) {
    const OpenGLFunctions& gl = device->GetGL();

    GLuint handle = 0;
    DAWN_GL_TRY(gl, GenBuffers(1, &handle));
    Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor, handle));

    DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, handle));

    // The buffers with mappedAtCreation == true will be initialized in
    // BufferBase::MapAtCreation().
    if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting) &&
        !descriptor->mappedAtCreation) {
        std::vector<uint8_t> clearValues(buffer->mAllocatedSize, 1u);
        DAWN_GL_TRY_ALWAYS_CHECK(gl, BufferData(GL_ARRAY_BUFFER, buffer->mAllocatedSize,
                                                clearValues.data(), GL_STATIC_DRAW));
    } else {
        // Buffers start uninitialized if you pass nullptr to glBufferData.
        DAWN_GL_TRY_ALWAYS_CHECK(
            gl, BufferData(GL_ARRAY_BUFFER, buffer->mAllocatedSize, nullptr, GL_STATIC_DRAW));
    }

    buffer->TrackUsage();

    return std::move(buffer);
}

Buffer::Buffer(Device* device, const UnpackedPtr<BufferDescriptor>& descriptor, GLuint handle)
    : BufferBase(device, descriptor), mBuffer(handle) {
    // Allocate at least 4 bytes so clamped accesses are always in bounds.
    // Align with 4 byte to avoid out-of-bounds access issue in compute emulation for 2 byte
    // element.
    mAllocatedSize = Align(std::max(GetSize(), uint64_t(4u)), uint64_t(4u));
}

Buffer::~Buffer() = default;

GLuint Buffer::GetHandle() const {
    return mBuffer;
}

MaybeError Buffer::EnsureDataInitialized(bool* outDidDataInitialization) {
    if (!NeedsInitialization()) {
        if (outDidDataInitialization) {
            *outDidDataInitialization = false;
        }
        return {};
    }

    DAWN_TRY(InitializeToZero());
    if (outDidDataInitialization) {
        *outDidDataInitialization = true;
    }
    return {};
}

MaybeError Buffer::EnsureDataInitializedAsDestination(uint64_t offset,
                                                      uint64_t size,
                                                      bool* outDidDataInitialization) {
    if (!NeedsInitialization()) {
        if (outDidDataInitialization) {
            *outDidDataInitialization = false;
        }
        return {};
    }

    if (IsFullBufferRange(offset, size)) {
        SetInitialized(true);
        if (outDidDataInitialization) {
            *outDidDataInitialization = false;
        }
        return {};
    }

    DAWN_TRY(InitializeToZero());
    if (outDidDataInitialization) {
        *outDidDataInitialization = true;
    }
    return {};
}

MaybeError Buffer::EnsureDataInitializedAsDestination(const CopyTextureToBufferCmd* copy,
                                                      bool* outDidDataInitialization) {
    if (!NeedsInitialization()) {
        if (outDidDataInitialization) {
            *outDidDataInitialization = false;
        }
        return {};
    }

    if (IsFullBufferOverwrittenInTextureToBufferCopy(copy)) {
        SetInitialized(true);
        if (outDidDataInitialization) {
            *outDidDataInitialization = false;
        }
        return {};
    }

    DAWN_TRY(InitializeToZero());
    if (outDidDataInitialization) {
        *outDidDataInitialization = true;
    }
    return {};
}

MaybeError Buffer::InitializeToZero() {
    DAWN_ASSERT(NeedsInitialization());

    const uint64_t size = GetAllocatedSize();
    Device* device = ToBackend(GetDevice());
    const OpenGLFunctions& gl = device->GetGL();

    const std::vector<uint8_t> clearValues(size, 0u);
    DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, mBuffer));
    DAWN_GL_TRY(gl, BufferSubData(GL_ARRAY_BUFFER, 0, size, clearValues.data()));
    device->IncrementLazyClearCountForTesting();

    TrackUsage();
    SetInitialized(true);
    return {};
}

bool Buffer::IsCPUWritableAtCreation() const {
    // TODO(enga): All buffers in GL can be mapped. Investigate if mapping them will cause the
    // driver to migrate it to shared memory.
    return true;
}

MaybeError Buffer::MapAtCreationImpl() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, mBuffer));
    mMappedData = DAWN_GL_TRY_ALWAYS_CHECK(
        gl, MapBufferRange(GL_ARRAY_BUFFER, 0, GetSize(), GL_MAP_WRITE_BIT));
    return {};
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    // It is an error to map an empty range in OpenGL. We always have at least a 4-byte buffer
    // so we extend the range to be 4 bytes.
    if (size == 0) {
        if (offset != 0) {
            offset -= 4;
        }
        size = 4;
    }

    DAWN_TRY(EnsureDataInitialized());

    // This does GPU->CPU synchronization, we could require a high
    // version of OpenGL that would let us map the buffer unsynchronized.
    DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, mBuffer));
    void* mappedData = nullptr;
    if (mode & wgpu::MapMode::Read) {
        mappedData = DAWN_GL_TRY_ALWAYS_CHECK(
            gl, MapBufferRange(GL_ARRAY_BUFFER, offset, size, GL_MAP_READ_BIT));
    } else {
        DAWN_ASSERT(mode & wgpu::MapMode::Write);
        mappedData = DAWN_GL_TRY_ALWAYS_CHECK(
            gl, MapBufferRange(GL_ARRAY_BUFFER, offset, size,
                               GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    }

    // The frontend asks that the pointer returned by GetMappedPointer is from the start of
    // the resource but OpenGL gives us the pointer at offset. Remove the offset.
    mMappedData = static_cast<uint8_t*>(mappedData) - offset;
    return {};
}

void* Buffer::GetMappedPointerImpl() {
    // The mapping offset has already been removed.
    return mMappedData;
}

void Buffer::UnmapImpl() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    DAWN_GL_TRY_IGNORE_ERRORS(gl, BindBuffer(GL_ARRAY_BUFFER, mBuffer));
    DAWN_GL_TRY_IGNORE_ERRORS(gl, UnmapBuffer(GL_ARRAY_BUFFER));
    mMappedData = nullptr;
}

void Buffer::DestroyImpl() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    BufferBase::DestroyImpl();
    DAWN_GL_TRY_IGNORE_ERRORS(gl, DeleteBuffers(1, &mBuffer));
    mBuffer = 0;
}

}  // namespace dawn::native::opengl
