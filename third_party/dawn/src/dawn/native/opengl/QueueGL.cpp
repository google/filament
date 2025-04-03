// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/opengl/QueueGL.h"

#include "dawn/native/BlitBufferToDepthStencil.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/opengl/BufferGL.h"
#include "dawn/native/opengl/CommandBufferGL.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/EGLFunctions.h"
#include "dawn/native/opengl/PhysicalDeviceGL.h"
#include "dawn/native/opengl/SharedFenceEGL.h"
#include "dawn/native/opengl/TextureGL.h"
#include "dawn/native/opengl/UtilsGL.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::opengl {

namespace {
EGLenum EGLSyncTypeFromSharedFenceType(wgpu::SharedFenceType sharedFenceType) {
    switch (sharedFenceType) {
        case wgpu::SharedFenceType::SyncFD:
            return EGL_SYNC_NATIVE_FENCE_ANDROID;
        case wgpu::SharedFenceType::EGLSync:
            return EGL_SYNC_FENCE;
        default:
            DAWN_UNREACHABLE();
    }
}
}  // namespace

ResultOrError<Ref<Queue>> Queue::Create(Device* device, const QueueDescriptor* descriptor) {
    return AcquireRef(new Queue(device, descriptor));
}

Queue::Queue(Device* device, const QueueDescriptor* descriptor) : QueueBase(device, descriptor) {
    const auto& egl = device->GetEGL(false);

    if (egl.HasExt(EGLExt::NativeFenceSync)) {
        mEGLSyncType = EGL_SYNC_NATIVE_FENCE_ANDROID;
    } else if (egl.HasExt(EGLExt::FenceSync)) {
        mEGLSyncType = EGL_SYNC_FENCE;
    } else if (egl.HasExt(EGLExt::ReusableSync)) {
        mEGLSyncType = EGL_SYNC_REUSABLE_KHR;
    } else {
        DAWN_UNREACHABLE();
    }
}

MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
    TRACE_EVENT_BEGIN0(GetDevice()->GetPlatform(), Recording, "CommandBufferGL::Execute");
    for (uint32_t i = 0; i < commandCount; ++i) {
        DAWN_TRY(ToBackend(commands[i])->Execute());
    }
    TRACE_EVENT_END0(GetDevice()->GetPlatform(), Recording, "CommandBufferGL::Execute");
    return {};
}

MaybeError Queue::WriteBufferImpl(BufferBase* buffer,
                                  uint64_t bufferOffset,
                                  const void* data,
                                  size_t size) {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    DAWN_TRY(ToBackend(buffer)->EnsureDataInitializedAsDestination(bufferOffset, size));

    DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, ToBackend(buffer)->GetHandle()));
    DAWN_GL_TRY(gl, BufferSubData(GL_ARRAY_BUFFER, bufferOffset, size, data));
    buffer->MarkUsedInPendingCommands();
    return {};
}

MaybeError Queue::WriteTextureImpl(const TexelCopyTextureInfo& destination,
                                   const void* data,
                                   size_t dataSize,
                                   const TexelCopyBufferLayout& dataLayout,
                                   const Extent3D& writeSizePixel) {
    TextureCopy textureCopy;
    textureCopy.texture = destination.texture;
    textureCopy.mipLevel = destination.mipLevel;
    textureCopy.origin = destination.origin;
    textureCopy.aspect = SelectFormatAspects(destination.texture->GetFormat(), destination.aspect);

    DeviceBase* device = GetDevice();
    if (textureCopy.aspect == Aspect::Stencil &&
        (textureCopy.texture->GetFormat().aspects & Aspect::Depth ||
         device->IsToggleEnabled(Toggle::UseBlitForStencilTextureWrite))) {
        // Workaround when write to stencil is unsupported:
        // - when the texture is stencil-only but OES_texture_stencil8 is unavailable.
        // - when the texture is depth-stencil-combined and writing to the stencil aspect.

        // Call WriteTexture to upload data to an intermediate R8Uint texture.
        TextureDescriptor dataTextureDesc = {};
        dataTextureDesc.format = wgpu::TextureFormat::R8Uint;
        dataTextureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        dataTextureDesc.size = writeSizePixel;
        dataTextureDesc.mipLevelCount = 1;
        Ref<TextureBase> dataTexture;
        DAWN_TRY_ASSIGN(dataTexture, device->CreateTexture(&dataTextureDesc));
        {
            TexelCopyTextureInfo destinationDataTexture;
            destinationDataTexture.texture = dataTexture.Get();
            destinationDataTexture.aspect = wgpu::TextureAspect::All;
            // The size of R8Uint texture equals to writeSizePixel and only has 1 mip level.
            // So the x,y,z origins and mipLevel are always 0.
            destinationDataTexture.mipLevel = 0;
            destinationDataTexture.origin = {0, 0, 0};
            DAWN_TRY_CONTEXT(WriteTextureImpl(destinationDataTexture, data, dataSize, dataLayout,
                                              writeSizePixel),
                             "writing to stencil aspect of %s using blit workaround when writing "
                             "to an intermediate r8uint texture.",
                             textureCopy.texture.Get());
        }

        // Blit from R8Uint texture to the stencil texture.
        Ref<CommandEncoderBase> commandEncoder;
        DAWN_TRY_ASSIGN(commandEncoder, device->CreateCommandEncoder());
        DAWN_TRY_CONTEXT(BlitR8ToStencil(device, commandEncoder.Get(), dataTexture.Get(),
                                         textureCopy, writeSizePixel),
                         "writing to stencil aspect of %s using blit workaround.",
                         textureCopy.texture.Get());

        Ref<CommandBufferBase> commandBuffer;
        DAWN_TRY_ASSIGN(commandBuffer, commandEncoder->Finish());
        CommandBufferBase* commands = commandBuffer.Get();
        APISubmit(1, &commands);
        return {};
    }

    SubresourceRange range = GetSubresourcesAffectedByCopy(textureCopy, writeSizePixel);
    if (IsCompleteSubresourceCopiedTo(destination.texture, writeSizePixel, destination.mipLevel,
                                      destination.aspect)) {
        destination.texture->SetIsSubresourceContentInitialized(true, range);
    } else {
        DAWN_TRY(ToBackend(destination.texture)->EnsureSubresourceContentInitialized(range));
    }
    return DoTexSubImage(ToBackend(GetDevice())->GetGL(), textureCopy, data, dataLayout,
                         writeSizePixel);
}

void Queue::OnGLUsed() {
    mHasPendingCommands = true;
}

ResultOrError<bool> Queue::WaitForQueueSerial(ExecutionSerial serial, Nanoseconds timeout) {
    // Search for the first fence >= serial.
    return mFencesInFlight.Use([&](auto fencesInFlight) -> ResultOrError<bool> {
        Ref<WrappedEGLSync> sync;
        for (auto it = fencesInFlight->begin(); it != fencesInFlight->end(); ++it) {
            if (it->second >= serial) {
                sync = it->first;
                break;
            }
        }
        if (sync == nullptr) {
            // Fence sync not found. This serial must have already completed.
            // Return a success status.
            return true;
        }

        // Wait for the fence sync.
        GLenum result;
        DAWN_TRY_ASSIGN(result, sync->ClientWait(EGL_SYNC_FLUSH_COMMANDS_BIT, timeout));

        switch (result) {
            case EGL_TIMEOUT_EXPIRED:
                return false;
            case EGL_CONDITION_SATISFIED:
                return true;
            default:
                DAWN_UNREACHABLE();
        }
    });
}

MaybeError Queue::SubmitFenceSync() {
    return mFencesInFlight.Use([&](auto fencesInFlight) -> MaybeError {
        if (!mHasPendingCommands) {
            return {};
        }
        DisplayEGL* display = ToBackend(GetDevice()->GetPhysicalDevice())->GetDisplay();

        Ref<WrappedEGLSync> sync;
        DAWN_TRY_ASSIGN(sync, WrappedEGLSync::Create(display, mEGLSyncType, nullptr));

        // Signal the sync if it is EGL_SYNC_REUSABLE_KHR. On the other hand,
        // EGL_SYNC_FENCE_KHR has its signal scheduled on creation.
        if (mEGLSyncType == EGL_SYNC_REUSABLE_KHR) {
            DAWN_TRY(sync->Signal(EGL_SIGNALED));
        }

        IncrementLastSubmittedCommandSerial();
        fencesInFlight->emplace_back(sync, GetLastSubmittedCommandSerial());
        mHasPendingCommands = false;
        return {};
    });
}

ResultOrError<Ref<SharedFence>> Queue::GetOrCreateSharedFence(ExecutionSerial lastUsageSerial,
                                                              wgpu::SharedFenceType type) {
    Ref<WrappedEGLSync> sync;

    EGLenum requestedSyncType = EGLSyncTypeFromSharedFenceType(type);
    // We can use the internal syncs if their type is compatible. All internal syncs are valid for
    // SharedFenceType::EGLSync otherwise the more specific type must match
    bool internalSyncTypeIsCompatibleWithSharedFenceType =
        (type == wgpu::SharedFenceType::EGLSync) || (requestedSyncType == mEGLSyncType);

    if (internalSyncTypeIsCompatibleWithSharedFenceType) {
        // Look for an existing sync that can represent this serial.
        sync = mFencesInFlight.Use([&](auto fencesInFlight) -> Ref<WrappedEGLSync> {
            for (auto it = fencesInFlight->begin(); it != fencesInFlight->end(); ++it) {
                if (it->second >= lastUsageSerial) {
                    return it->first;
                }
            }
            return {};
        });
    }

    Device* device = ToBackend(GetDevice());

    if (sync == nullptr) {
        DisplayEGL* display = ToBackend(device->GetPhysicalDevice())->GetDisplay();
        DAWN_TRY_ASSIGN(sync, WrappedEGLSync::Create(display, requestedSyncType, nullptr));
    }
    DAWN_ASSERT(sync != nullptr);

    // If we are sharing this sync externally, make sure to flush all commands.
    // The FD cannot be queried before the flush and clients may hang if they try to wait on the
    // sync.
    const OpenGLFunctions& gl = device->GetGL();
    DAWN_GL_TRY(gl, Flush());

    SystemHandle handle;
    if (type == wgpu::SharedFenceType::SyncFD) {
        EGLint fd;
        DAWN_TRY_ASSIGN(fd, sync->DupFD());

        handle = SystemHandle::Acquire(fd);
    }

    return AcquireRef(new SharedFenceEGL(ToBackend(GetDevice()), "Internal EGLSync", type,
                                         std::move(handle), sync));
}

bool Queue::HasPendingCommands() const {
    return mHasPendingCommands;
}

MaybeError Queue::SubmitPendingCommands() {
    DAWN_TRY(SubmitFenceSync());
    return {};
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    return mFencesInFlight.Use([&](auto fencesInFlight) -> ResultOrError<ExecutionSerial> {
        ExecutionSerial fenceSerial{0};
        while (!fencesInFlight->empty()) {
            auto [sync, tentativeSerial] = fencesInFlight->front();

            // Fence are added in order, so we can stop searching as soon
            // as we see one that's not ready.
            GLenum result;
            DAWN_TRY_ASSIGN(result, sync->ClientWait(EGL_SYNC_FLUSH_COMMANDS_BIT, Nanoseconds(0)));
            if (result == EGL_TIMEOUT_EXPIRED) {
                return fenceSerial;
            }
            // Update fenceSerial since fence is ready.
            fenceSerial = tentativeSerial;

            fencesInFlight->pop_front();

            DAWN_ASSERT(fenceSerial > GetCompletedCommandSerial());
        }
        return fenceSerial;
    });
}

void Queue::ForceEventualFlushOfCommands() {
    mHasPendingCommands = true;
}

MaybeError Queue::WaitForIdleForDestruction() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    DAWN_GL_TRY(gl, Finish());
    DAWN_TRY(CheckPassedSerials());
    DAWN_ASSERT(mFencesInFlight->empty());
    mHasPendingCommands = false;
    return {};
}

}  // namespace dawn::native::opengl
