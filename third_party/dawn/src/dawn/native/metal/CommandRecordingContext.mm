// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/native/metal/CommandRecordingContext.h"

#include "dawn/common/Assert.h"
#include "dawn/native/Device.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/Forward.h"
#include "dawn/native/metal/QueueMTL.h"

namespace dawn::native::metal {

CommandRecordingContext::CommandRecordingContext(const Queue* queue) : mQueue(queue) {}

CommandRecordingContext::~CommandRecordingContext() {
    // Commands must be acquired.
    DAWN_ASSERT(mCommands == nullptr);
}

id<MTLCommandBuffer> CommandRecordingContext::GetCommands() {
    return mCommands.Get();
}

void CommandRecordingContext::SetNeedsSubmit() {
    mNeedsSubmit = true;
}
bool CommandRecordingContext::NeedsSubmit() const {
    return mNeedsSubmit;
}

void CommandRecordingContext::MarkUsed() {
    mUsed = true;
}
bool CommandRecordingContext::WasUsed() const {
    return mUsed;
}

MaybeError CommandRecordingContext::PrepareNextCommandBuffer(id<MTLCommandQueue> queue) {
    @autoreleasepool {
        DAWN_ASSERT(mCommands == nil);
        DAWN_ASSERT(!mNeedsSubmit);
        DAWN_ASSERT(!mUsed);

        mCommands = [queue commandBuffer];
        if (mCommands == nil) {
            return DAWN_INTERNAL_ERROR("Failed to allocate an MTLCommandBuffer");
        }

        return {};
    }
}

NSPRef<id<MTLCommandBuffer>> CommandRecordingContext::AcquireCommands() {
    // A blit encoder can be left open from WriteBuffer, make sure we close it.
    if (mCommands != nullptr) {
        EndBlit();
    }

    DAWN_ASSERT(!mInEncoder);
    mNeedsSubmit = false;
    mUsed = false;
    return std::move(mCommands);
}

id<MTLBlitCommandEncoder> CommandRecordingContext::BeginBlit(MTLBlitPassDescriptor* descriptor) {
    @autoreleasepool {
        DAWN_ASSERT(descriptor);
        DAWN_ASSERT(mCommands != nullptr);
        DAWN_ASSERT(mBlit == nullptr);
        DAWN_ASSERT(!mInEncoder);

        mInEncoder = true;
        mBlit = [*mCommands blitCommandEncoderWithDescriptor:descriptor];
        return mBlit.Get();
    }
}

id<MTLBlitCommandEncoder> CommandRecordingContext::EnsureBlit() {
    DAWN_ASSERT(mCommands != nullptr);

    if (mBlit == nullptr) {
        @autoreleasepool {
            DAWN_ASSERT(!mInEncoder);
            mInEncoder = true;
            mBlit = [*mCommands blitCommandEncoder];
        }
    }
    return mBlit.Get();
}

void CommandRecordingContext::EndBlit() {
    DAWN_ASSERT(mCommands != nullptr);

    if (mBlit != nullptr) {
        [*mBlit endEncoding];
        mBlit = nullptr;
        mInEncoder = false;
    }
}

MaybeError CommandRecordingContext::EncodeSharedEventWorkaround() {
    DAWN_ASSERT(mQueue->GetDevice()->IsToggleEnabled(
        Toggle::MetalSerializeTimestampGenerationAndResolution));

    if (!mSerializeWorkaround.sharedEvent) {
        id<MTLDevice> mtlDevice = ToBackend(mQueue->GetDevice())->GetMTLDevice();
        mSerializeWorkaround.sharedEvent.Acquire([mtlDevice newSharedEvent]);
        if (mSerializeWorkaround.sharedEvent == nil) {
            return DAWN_INTERNAL_ERROR(
                "Failed to create internal MTLSharedEvent for splitting command buffers.");
        }
    }

    EndBlit();
    id<MTLSharedEvent> sharedEvent =
        static_cast<id<MTLSharedEvent>>(*mSerializeWorkaround.sharedEvent);
    uint64_t signalValue = ++mSerializeWorkaround.signaledValue;
    [*mCommands encodeSignalEvent:sharedEvent value:signalValue];
    [*mCommands encodeWaitForEvent:sharedEvent value:signalValue];
    return {};
}

id<MTLComputeCommandEncoder> CommandRecordingContext::BeginCompute() {
    @autoreleasepool {
        DAWN_ASSERT(mCommands != nullptr);
        DAWN_ASSERT(mCompute == nullptr);
        DAWN_ASSERT(!mInEncoder);

        mInEncoder = true;
        mCompute = [*mCommands computeCommandEncoder];
        return mCompute.Get();
    }
}

id<MTLComputeCommandEncoder> CommandRecordingContext::BeginCompute(
    MTLComputePassDescriptor* descriptor) {
    @autoreleasepool {
        DAWN_ASSERT(descriptor);
        DAWN_ASSERT(mCommands != nullptr);
        DAWN_ASSERT(mCompute == nullptr);
        DAWN_ASSERT(!mInEncoder);

        mInEncoder = true;
        mCompute = [*mCommands computeCommandEncoderWithDescriptor:descriptor];
        return mCompute.Get();
    }
}

void CommandRecordingContext::EndCompute() {
    DAWN_ASSERT(mCommands != nullptr);
    DAWN_ASSERT(mCompute != nullptr);

    [*mCompute endEncoding];
    mCompute = nullptr;
    mInEncoder = false;
}

id<MTLRenderCommandEncoder> CommandRecordingContext::BeginRender(
    MTLRenderPassDescriptor* descriptor) {
    @autoreleasepool {
        DAWN_ASSERT(mCommands != nullptr);
        DAWN_ASSERT(mRender == nullptr);
        DAWN_ASSERT(!mInEncoder);

        mInEncoder = true;
        mRender = [*mCommands renderCommandEncoderWithDescriptor:descriptor];
        return mRender.Get();
    }
}

void CommandRecordingContext::EndRender() {
    DAWN_ASSERT(mCommands != nullptr);
    DAWN_ASSERT(mRender != nullptr);

    [*mRender endEncoding];
    mRender = nullptr;
    mInEncoder = false;
}

void CommandRecordingContext::WaitForSharedEvent(id<MTLSharedEvent> sharedEvent,
                                                 uint64_t signaledValue) {
    // Skip the wait if it's for the same shared event as the queue i.e. a self wait. These can
    // happen if the client passes us the same shared event that we gave it back to us. If these
    // events are waited on, they seem to cause waitUntilScheduled to block for previous command
    // buffers to complete due to dependencies between consecutive command buffers.
    if (sharedEvent != mQueue->GetMTLSharedEvent()) {
        // There may be an open blit encoder from a copy command or writeBuffer.
        // Wait events are only allowed if there is no encoder open.
        EndBlit();
        [*mCommands encodeWaitForEvent:sharedEvent value:signaledValue];
    } else {
        DAWN_ASSERT(ExecutionSerial(signaledValue) <= mQueue->GetLastSubmittedCommandSerial());
    }
}

}  // namespace dawn::native::metal
