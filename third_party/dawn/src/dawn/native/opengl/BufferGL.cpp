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

#include "src/dawn/native/opengl/BufferGL.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "src/dawn/native/ChainUtils.h"
#include "src/dawn/native/CommandBuffer.h"
#include "src/dawn/native/opengl/DeviceGL.h"
#include "src/dawn/native/opengl/UtilsGL.h"
#include "src/utils/compiler.h"
#include "src/utils/numeric.h"

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
    Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor));

    // The buffers with mappedAtCreation == true will be initialized in
    // BufferBase::MapAtCreation().
    bool clear = device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting) &&
                 !descriptor->mappedAtCreation;

    DAWN_TRY(device->EnqueueGL([buffer, clear](const OpenGLFunctions& gl) -> MaybeError {
        DAWN_GL_TRY(gl, GenBuffers(1, &buffer->mBuffer));
        DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, buffer->mBuffer));

        auto allocatedSize = buffer->mAllocatedSize.value();
        if (clear) {
            std::vector<uint8_t> clearValues(checked_cast<size_t>(allocatedSize), 1u);
            DAWN_GL_TRY_ALWAYS_CHECK(
                gl, BufferData(GL_ARRAY_BUFFER, checked_cast<GLsizeiptr>(allocatedSize),
                               clearValues.data(), GL_STATIC_DRAW));
        } else {
            // Buffers start uninitialized if you pass nullptr to glBufferData.
            DAWN_GL_TRY_ALWAYS_CHECK(
                gl, BufferData(GL_ARRAY_BUFFER, checked_cast<GLsizeiptr>(allocatedSize), nullptr,
                               GL_STATIC_DRAW));
        }
        return {};
    }));

    {
        auto scopedUseBuffer = buffer->UseInternal();
        buffer->TrackUsage();
    }

    return std::move(buffer);
}

Buffer::Buffer(Device* device, const UnpackedPtr<BufferDescriptor>& descriptor)
    : BufferBase(device, descriptor) {
    // Allocate at least 4 bytes so clamped accesses are always in bounds.
    // Align with 4 byte to avoid out-of-bounds access issue in compute emulation for 2 byte
    // element.
    size_t alignment = 4u;
    // Round uniform buffer sizes up to a multiple of 16 bytes since Tint will polyfill them as
    // array<vec4u, ...>.
    if (GetUsage() & wgpu::BufferUsage::Uniform) {
        alignment = 16u;
    }
    mAllocatedSize = Align(std::max(GetSize(), uint64_t{4}), alignment);
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

    Device* device = ToBackend(GetDevice());

    DAWN_TRY(device->EnqueueGL([self = Ref<Buffer>(this), size = GetAllocatedSize()](
                                   const OpenGLFunctions& gl) -> MaybeError {
        const std::vector<uint8_t> clearValues(checked_cast<size_t>(size), 0u);
        DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, self->mBuffer));
        DAWN_GL_TRY(gl, BufferSubData(GL_ARRAY_BUFFER, 0, checked_cast<GLsizeiptr>(size),
                                      clearValues.data()));
        return {};
    }));
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
    mMappedDataOffsetInBuffer = 0u;

    auto device = ToBackend(GetDevice());
    if (device->IsToggleEnabled(Toggle::GLDefer)) {
        mCPUStaging.resize(checked_cast<size_t>(GetAllocatedSize()));
        mMappedData = mCPUStaging;
        return {};
    }

    return device->ExecuteGL(
        ExecutionQueueBase::SubmitMode::Normal, [this](const OpenGLFunctions& gl) -> MaybeError {
            DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, mBuffer));
            void* mappedPointer = DAWN_GL_TRY_ALWAYS_CHECK(
                gl, MapBufferRange(GL_ARRAY_BUFFER, 0, checked_cast<GLsizeiptr>(GetAllocatedSize()),
                                   GL_MAP_WRITE_BIT));
            // SAFETY: A successful call to glMapBufferRange returns a pointer to `length` bytes of
            // data.
            mMappedData = DAWN_UNSAFE_BUFFERS(
                {static_cast<std::byte*>(mappedPointer), checked_cast<size_t>(GetAllocatedSize())});
            return {};
        });
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    // We only map the range requested by the MapAsync call so mMappedData will correspond to an
    // interval in the buffer that can be at an offset from the start.
    mMappedDataOffsetInBuffer = offset;

    // It is an error to map an empty range in OpenGL so skip empty glMapBufferRange calls.
    if (size == 0) {
        mMappedData = {};
        return {};
    }

    auto deviceGuard = GetDevice()->GetGuard();

    DAWN_TRY(EnsureDataInitialized());
    if (GetDevice()->IsToggleEnabled(Toggle::GLDefer)) {
        TrackUsage();
    }

    return ToBackend(GetDevice())
        ->EnqueueGL([self = Ref<Buffer>(this), offset, size,
                     mode](const OpenGLFunctions& gl) -> MaybeError {
            // This does GPU->CPU synchronization, we could require a high
            // version of OpenGL that would let us map the buffer unsynchronized.
            DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, self->mBuffer));
            void* mappedPointer = nullptr;
            if (mode & wgpu::MapMode::Read) {
                mappedPointer = DAWN_GL_TRY_ALWAYS_CHECK(
                    gl, MapBufferRange(GL_ARRAY_BUFFER, offset, size, GL_MAP_READ_BIT));
            } else {
                DAWN_ASSERT(mode & wgpu::MapMode::Write);
                mappedPointer = DAWN_GL_TRY_ALWAYS_CHECK(
                    gl, MapBufferRange(GL_ARRAY_BUFFER, offset, size,
                                       GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
            }

            // SAFETY: A successful call to glMapBufferRange returns a pointer to `length` bytes of
            // data.
            self->mMappedData = DAWN_UNSAFE_BUFFERS({static_cast<std::byte*>(mappedPointer), size});
            return {};
        });
}

MaybeError Buffer::FinalizeMapImpl(BufferState newState) {
    return {};
}

Span<std::byte> Buffer::GetMappedRangeImpl(size_t offset, size_t size) {
    DAWN_ASSERT(offset >= mMappedDataOffsetInBuffer);
    return mMappedData.subspan(offset - mMappedDataOffsetInBuffer, size);
}

void Buffer::UnmapImpl(BufferState oldState, BufferState newState) {
    auto deviceGuard = GetDevice()->GetGuard();

    auto device = ToBackend(GetDevice());

    if (newState == BufferState::Destroyed) {
        return;
    }

    // There is nothing to do for empty mappings, and the buffer wasn't even mapped.
    if (mMappedData.empty()) {
        return;
    }

    IgnoreErrors(
        device->EnqueueGL([self = Ref<Buffer>(this)](const OpenGLFunctions& gl) -> MaybeError {
            DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, self->mBuffer));
            if (self->mCPUStaging.size() > 0) {
                void* mappedPointer = DAWN_GL_TRY_ALWAYS_CHECK(
                    gl, MapBufferRange(GL_ARRAY_BUFFER, self->mMappedDataOffsetInBuffer,
                                       self->mCPUStaging.size(), GL_MAP_WRITE_BIT));

                // SAFETY: A successful call to glMapBufferRange returns a pointer to `length` bytes
                // of data.
                Span<std::byte> mappedData = DAWN_UNSAFE_BUFFERS(
                    {static_cast<std::byte*>(mappedPointer), self->mCPUStaging.size()});
                mappedData.CopyFrom(self->mCPUStaging);
                self->mCPUStaging.resize(0);
            }
            DAWN_GL_TRY(gl, UnmapBuffer(GL_ARRAY_BUFFER));
            return {};
        }));
}

void Buffer::DestroyImpl(DestroyReason reason) {
    BufferBase::DestroyImpl(reason);
    mMappedData = {};

    IgnoreErrors(ToBackend(GetDevice())
                     ->EnqueueDestroyGL(this, &Buffer::GetHandle, reason,
                                        [](const OpenGLFunctions& gl, GLuint handle) -> MaybeError {
                                            DAWN_GL_TRY_IGNORE_ERRORS(gl,
                                                                      DeleteBuffers(1, &handle));
                                            return {};
                                        }));
}

}  // namespace dawn::native::opengl
