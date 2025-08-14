// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/EncodingContext.h"

#include "dawn/common/Assert.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/IndirectDrawValidationEncoder.h"
#include "dawn/native/RenderBundleEncoder.h"

namespace dawn::native {

EncodingContext::EncodingContext(DeviceBase* device, const ApiObjectBase* initialEncoder)
    : mDevice(device),
      mTopLevelEncoder(initialEncoder),
      mCurrentEncoder(initialEncoder),
      mStatus(Status::Open) {
    DAWN_ASSERT(!initialEncoder->IsError());
}

EncodingContext::EncodingContext(DeviceBase* device, ErrorMonad::ErrorTag tag)
    : mDevice(device),
      mTopLevelEncoder(nullptr),
      mCurrentEncoder(nullptr),
      mStatus(Status::ErrorAtCreation) {}

EncodingContext::~EncodingContext() {
    Destroy();
}

void EncodingContext::Destroy() {
    mDebugGroupLabels.clear();

    if (!mWereIndirectDrawMetadataAcquired) {
        mIndirectDrawMetadata.clear();
    }
    if (!mWereCommandsAcquired) {
        CommandIterator commands = AcquireCommands();
        FreeCommands(&commands);
    }

    CloseWithStatus(Status::Destroyed);
}

CommandIterator EncodingContext::AcquireCommands() {
    DAWN_ASSERT(!mWereCommandsAcquired);
    mWereCommandsAcquired = true;

    CommitCommands(std::move(mPendingCommands));

    CommandIterator commands;
    commands.AcquireCommandBlocks(std::move(mAllocators));
    return commands;
}

void EncodingContext::HandleError(std::unique_ptr<ErrorData> error) {
    // Append in reverse so that the most recently set debug group is printed first, like a
    // call stack.
    for (auto iter = mDebugGroupLabels.rbegin(); iter != mDebugGroupLabels.rend(); ++iter) {
        error->AppendDebugGroup(*iter);
    }

    if (mDevice->IsImmediateErrorHandlingEnabled() || mStatus == Status::Finished) {
        // EncodingContext is unprotected from multiple threads by default, but this code will
        // modify Device's internal states so we need to lock the device now.
        auto deviceGuard = mDevice->GetGuard();
        mDevice->HandleError(std::move(error));
    } else {
        // TODO(crbug.com/42240579): ASSERT that encoding only generates validation errors.

        // If the encoding context is not finished, errors are deferred until
        // Finish() is called.
        if (mError == nullptr) {
            mError = std::move(error);
        }
    }

    CloseWithStatus(Status::ErrorInRecording);
}

void EncodingContext::WillBeginRenderPass() {
    DAWN_ASSERT(mCurrentEncoder == mTopLevelEncoder);
    if (mDevice->IsValidationEnabled() || mDevice->MayRequireDuplicationOfIndirectParameters()) {
        // When validation is enabled or indirect parameters require duplication, we are going
        // to want to capture all commands encoded between and including BeginRenderPassCmd and
        // EndRenderPassCmd, and defer their sequencing util after we have a chance to insert
        // any necessary validation or duplication commands. To support this we commit any
        // current commands now, so that the impending BeginRenderPassCmd starts in a fresh
        // CommandAllocator.
        CommitCommands(std::move(mPendingCommands));
    }
}

void EncodingContext::EnterPass(const ApiObjectBase* passEncoder) {
    // Assert we're at the top level.
    DAWN_ASSERT(mCurrentEncoder == mTopLevelEncoder);
    DAWN_ASSERT(passEncoder != nullptr);

    mCurrentEncoder = passEncoder;
}

MaybeError EncodingContext::ExitRenderPass(const ApiObjectBase* passEncoder,
                                           RenderPassResourceUsageTracker usageTracker,
                                           CommandEncoder* commandEncoder,
                                           IndirectDrawMetadata indirectDrawMetadata) {
    DAWN_ASSERT(mCurrentEncoder != mTopLevelEncoder);
    DAWN_ASSERT(mCurrentEncoder == passEncoder);

    mCurrentEncoder = mTopLevelEncoder;

    if (mDevice->IsValidationEnabled() || mDevice->MayRequireDuplicationOfIndirectParameters()) {
        // With validation enabled, commands were committed just before BeginRenderPassCmd was
        // encoded by our RenderPassEncoder (see WillBeginRenderPass above). This means
        // mPendingCommands contains only the commands from BeginRenderPassCmd to
        // EndRenderPassCmd, inclusive. Now we swap out this allocator with a fresh one to give
        // the validation encoder a chance to insert its commands first.
        // Note: If encoding validation commands fails, no commands should be in mPendingCommands,
        //       so swap back the renderCommands to ensure that they are not leaked.
        CommandAllocator renderCommands = std::move(mPendingCommands);

        // The below function might create new resources. Device must already be locked via
        // renderpassEncoder's APIEnd().
        // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
        // Command Submit time, so the locking would be removed from here at that point.
        {
            DAWN_ASSERT(mDevice->IsLockedByCurrentThreadIfNeeded());

            DAWN_TRY_WITH_CLEANUP(
                EncodeIndirectDrawValidationCommands(mDevice, commandEncoder, &usageTracker,
                                                     &indirectDrawMetadata),
                { mPendingCommands = std::move(renderCommands); });
        }

        CommitCommands(std::move(mPendingCommands));
        CommitCommands(std::move(renderCommands));
    }

    mIndirectDrawMetadata.emplace_back(std::move(indirectDrawMetadata));

    mRenderPassUsages.push_back(usageTracker.AcquireResourceUsage());
    return {};
}

void EncodingContext::ExitComputePass(const ApiObjectBase* passEncoder,
                                      ComputePassResourceUsage usages) {
    DAWN_ASSERT(mCurrentEncoder != mTopLevelEncoder);
    DAWN_ASSERT(mCurrentEncoder == passEncoder);

    mCurrentEncoder = mTopLevelEncoder;
    mComputePassUsages.push_back(std::move(usages));
}

void EncodingContext::EnsurePassExited(const ApiObjectBase* passEncoder) {
    if (mCurrentEncoder != mTopLevelEncoder && mCurrentEncoder == passEncoder) {
        // The current pass encoder is being deleted. Implicitly end the pass with an error.
        mCurrentEncoder = mTopLevelEncoder;
        HandleError(DAWN_VALIDATION_ERROR("Command buffer recording ended before %s was ended.",
                                          passEncoder));
    }
}

const RenderPassUsages& EncodingContext::GetRenderPassUsages() const {
    DAWN_ASSERT(!mWereRenderPassUsagesAcquired);
    return mRenderPassUsages;
}

RenderPassUsages EncodingContext::AcquireRenderPassUsages() {
    DAWN_ASSERT(!mWereRenderPassUsagesAcquired);
    mWereRenderPassUsagesAcquired = true;
    return std::move(mRenderPassUsages);
}

const ComputePassUsages& EncodingContext::GetComputePassUsages() const {
    DAWN_ASSERT(!mWereComputePassUsagesAcquired);
    return mComputePassUsages;
}

ComputePassUsages EncodingContext::AcquireComputePassUsages() {
    DAWN_ASSERT(!mWereComputePassUsagesAcquired);
    mWereComputePassUsagesAcquired = true;
    return std::move(mComputePassUsages);
}

std::vector<IndirectDrawMetadata> EncodingContext::AcquireIndirectDrawMetadata() {
    DAWN_ASSERT(!mWereIndirectDrawMetadataAcquired);
    mWereIndirectDrawMetadataAcquired = true;
    return std::move(mIndirectDrawMetadata);
}

void EncodingContext::PushDebugGroupLabel(std::string_view groupLabel) {
    mDebugGroupLabels.emplace_back(groupLabel);
}

void EncodingContext::PopDebugGroupLabel() {
    mDebugGroupLabels.pop_back();
}

MaybeError EncodingContext::Finish() {
    CommitCommands(std::move(mPendingCommands));

    switch (mStatus) {
        case Status::ErrorAtCreation:
        case Status::Destroyed:
            return {};

        case Status::Finished:
            return DAWN_VALIDATION_ERROR("Command encoding already finished.");

        case Status::ErrorInRecording:
        case Status::Open:
            break;
    }
    const ApiObjectBase* currentEncoder = mCurrentEncoder;
    const ApiObjectBase* topLevelEncoder = mTopLevelEncoder;

    // Even if finish validation fails, it is now invalid to call any encoding commands.
    CloseWithStatus(Status::Finished);

    if (mError != nullptr) {
        return std::move(mError);
    }

    DAWN_INVALID_IF(currentEncoder != topLevelEncoder,
                    "Command buffer recording ended before %s was ended.", currentEncoder);
    return {};
}

void EncodingContext::CommitCommands(CommandAllocator allocator) {
    if (!allocator.IsEmpty()) {
        mAllocators.push_back(std::move(allocator));
    }
}

void EncodingContext::CloseWithStatus(Status status) {
    switch (status) {
        case Status::Open:
        case Status::ErrorAtCreation:
            // We cannot close with Open, and ErrorInRecording is set at construction, never after.
            DAWN_UNREACHABLE();

        // Status::ErrorInRecording causes encoding errors to be silently ignored (as we already
        // have one for this encoder), but both Status::Finished and Status::Destroyed make all
        // further errors surfaced on the device. For that reason we promote a
        // Status::ErrorInRecording to Finished/Destroyed.
        case Status::Destroyed:
        case Status::Finished:
            if (mStatus == Status::ErrorInRecording || mStatus == Status::Open) {
                mStatus = status;
            }
            break;

        case Status::ErrorInRecording:
            if (mStatus == Status::Open) {
                mStatus = status;
            }
            break;
    }

    mTopLevelEncoder = nullptr;
    mCurrentEncoder = nullptr;
}

}  // namespace dawn::native
