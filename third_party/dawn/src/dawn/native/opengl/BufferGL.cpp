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
    Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor));

    // The buffers with mappedAtCreation == true will be initialized in
    // BufferBase::MapAtCreation().
    bool clear = device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting) &&
                 !descriptor->mappedAtCreation;

    DAWN_TRY(device->EnqueueGL([buffer, clear](const OpenGLFunctions& gl) -> MaybeError {
        DAWN_GL_TRY(gl, GenBuffers(1, &buffer->mBuffer));
        DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, buffer->mBuffer));

        if (clear) {
            std::vector<uint8_t> clearValues(buffer->mAllocatedSize, 1u);
            DAWN_GL_TRY_ALWAYS_CHECK(gl, BufferData(GL_ARRAY_BUFFER, buffer->mAllocatedSize,
                                                    clearValues.data(), GL_STATIC_DRAW));
        } else {
            // Buffers start uninitialized if you pass nullptr to glBufferData.
            DAWN_GL_TRY_ALWAYS_CHECK(
                gl, BufferData(GL_ARRAY_BUFFER, buffer->mAllocatedSize, nullptr, GL_STATIC_DRAW));
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
    uint64_t alignment = 4u;
    // Round uniform buffer sizes up to a multiple of 16 bytes since Tint will polyfill them as
    // array<vec4u, ...>.
    if (GetUsage() & wgpu::BufferUsage::Uniform) {
        alignment = 16u;
    }
    mAllocatedSize = Align(std::max(GetSize(), uint64_t(4u)), alignment);
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
        const std::vector<uint8_t> clearValues(size, 0u);
        DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, self->mBuffer));
        DAWN_GL_TRY(gl, BufferSubData(GL_ARRAY_BUFFER, 0, size, clearValues.data()));
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
    auto device = ToBackend(GetDevice());
    if (device->IsToggleEnabled(Toggle::GLDefer)) {
        mCPUStaging.resize(GetAllocatedSize());
        mMappedData = mCPUStaging.data();
        return {};
    }
    return device->ExecuteGL(
        ExecutionQueueBase::SubmitMode::Normal, [this](const OpenGLFunctions& gl) -> MaybeError {
            DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, mBuffer));
            mMappedData = DAWN_GL_TRY_ALWAYS_CHECK(
                gl, MapBufferRange(GL_ARRAY_BUFFER, 0, GetAllocatedSize(), GL_MAP_WRITE_BIT));
            return {};
        });
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    // This allows the specific code sequence of MapAsync(Write) / WaitAny() to work in deferral
    // at the cost of a host-side copy. This should be removed in favour of eager buffer mapping,
    // once that's implemented.
    if ((mode & wgpu::MapMode::Write) && GetDevice()->IsToggleEnabled(Toggle::ShadowCopyMapWrite)) {
        mCPUStaging.resize(GetSize());
        mMappedData = mCPUStaging.data();
        return {};
    }

    // It is an error to map an empty range in OpenGL. We always have at least a 4-byte buffer
    // so we extend the range to be 4 bytes.
    if (size == 0) {
        if (offset != 0) {
            offset -= 4;
        }
        size = 4;
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
            self->mMappedData = static_cast<uint8_t*>(mappedData) - offset;
            return {};
        });
}

MaybeError Buffer::FinalizeMapImpl(BufferState newState) {
    return {};
}

void* Buffer::GetMappedPointerImpl() {
    // The mapping offset has already been removed.
    return mMappedData;
}

void Buffer::UnmapImpl(BufferState oldState, BufferState newState) {
    auto deviceGuard = GetDevice()->GetGuard();

    auto device = ToBackend(GetDevice());

    if (newState == BufferState::Destroyed) {
        return;
    }
    IgnoreErrors(
        device->EnqueueGL([self = Ref<Buffer>(this)](const OpenGLFunctions& gl) -> MaybeError {
            DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, self->mBuffer));
            if (self->mCPUStaging.size() > 0) {
                auto mappedData = DAWN_GL_TRY_ALWAYS_CHECK(
                    gl, MapBufferRange(GL_ARRAY_BUFFER, 0, self->GetSize(), GL_MAP_WRITE_BIT));
                memcpy(mappedData, self->mCPUStaging.data(), self->GetSize());
                self->mCPUStaging.resize(0);
            }
            DAWN_GL_TRY(gl, UnmapBuffer(GL_ARRAY_BUFFER));
            return {};
        }));
}

void Buffer::DestroyImpl(DestroyReason reason) {
    BufferBase::DestroyImpl(reason);
    mMappedData = nullptr;

    IgnoreErrors(ToBackend(GetDevice())
                     ->EnqueueDestroyGL(this, &Buffer::GetHandle, reason,
                                        [](const OpenGLFunctions& gl, GLuint handle) -> MaybeError {
                                            DAWN_GL_TRY_IGNORE_ERRORS(gl,
                                                                      DeleteBuffers(1, &handle));
                                            return {};
                                        }));
}

}  // namespace dawn::native::opengl
