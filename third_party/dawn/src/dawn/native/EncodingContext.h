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

#ifndef SRC_DAWN_NATIVE_ENCODINGCONTEXT_H_
#define SRC_DAWN_NATIVE_ENCODINGCONTEXT_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/CommandAllocator.h"
#include "dawn/native/Error.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/IndirectDrawMetadata.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PassResourceUsageTracker.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

class CommandEncoder;
class DeviceBase;
class ApiObjectBase;

// Base class for allocating/iterating commands.
// It performs error tracking as well as encoding state for render/compute passes.
class EncodingContext {
  public:
    EncodingContext(DeviceBase* device, const ApiObjectBase* initialEncoder);
    EncodingContext(DeviceBase* device, ErrorMonad::ErrorTag tag);
    ~EncodingContext();

    // Marks the encoding context as destroyed so that any future encodes will fail, and all
    // encoded commands are released.
    void Destroy();

    CommandIterator AcquireCommands();

    // Functions to handle encoder errors
    void HandleError(std::unique_ptr<ErrorData> error);

    inline bool ConsumedError(MaybeError maybeError) {
        if (DAWN_UNLIKELY(maybeError.IsError())) {
            HandleError(maybeError.AcquireError());
            return true;
        }
        return false;
    }

    template <typename... Args>
    inline bool ConsumedError(MaybeError maybeError, const char* formatStr, const Args&... args) {
        if (DAWN_UNLIKELY(maybeError.IsError())) {
            std::unique_ptr<ErrorData> error = maybeError.AcquireError();
            if (error->GetType() == InternalErrorType::Validation) {
                std::string out;
                absl::UntypedFormatSpec format(formatStr);
                if (absl::FormatUntyped(&out, format, {absl::FormatArg(args)...})) {
                    error->AppendContext(std::move(out));
                } else {
                    error->AppendContext(
                        absl::StrFormat("[Failed to format error message: \"%s\"].", formatStr));
                }
            }
            HandleError(std::move(error));
            return true;
        }
        return false;
    }

    inline MaybeError ValidateCanEncodeOn(const ApiObjectBase* encoder) {
        if (DAWN_UNLIKELY(encoder != mCurrentEncoder)) {
            switch (mStatus) {
                case Status::ErrorAtCreation:
                    return DAWN_VALIDATION_ERROR("Recording in an error %s.", encoder);
                case Status::ErrorInRecording:
                    return DAWN_VALIDATION_ERROR("Recording in an already invalidated %s.",
                                                 encoder);
                case Status::Destroyed:
                    return DAWN_VALIDATION_ERROR("Recording in a destroyed %s.", encoder);

                case Status::Finished:
                    // The encoder has been finished, select the correct error message.
                    DAWN_INVALID_IF(encoder->GetType() == ObjectType::CommandEncoder ||
                                        encoder->GetType() == ObjectType::RenderBundleEncoder,
                                    "%s is already finished.", encoder);
                    return DAWN_VALIDATION_ERROR("Parent encoder of %s is already finished.",
                                                 encoder);

                case Status::Open:
                    // This could happen when an error child encoder is created on an otherwise
                    // valid EncodingContext that then doesn't get notified of the current encoder
                    // change.
                    DAWN_INVALID_IF(encoder->IsError(), "Recording in an error %s.", encoder);

                    // This happens when the CommandEncoder is used while a pass is open.
                    DAWN_INVALID_IF(encoder == mTopLevelEncoder,
                                    "Recording in %s which is locked while %s is open.", encoder,
                                    mCurrentEncoder);

                    // The remaining case is when an encoder is ended but we still try to encode
                    // commands in it.
                    return DAWN_VALIDATION_ERROR(
                        "Commands cannot be recorded in %s which has already been ended.", encoder);
            }
        }
        return {};
    }

    template <typename EncodeFunction>
    inline bool TryEncode(const ApiObjectBase* encoder, EncodeFunction&& encodeFunction) {
        if (ConsumedError(ValidateCanEncodeOn(encoder))) {
            return false;
        }
        DAWN_ASSERT(!mWereCommandsAcquired);
        return !ConsumedError(encodeFunction(&mPendingCommands));
    }

    template <typename EncodeFunction, typename... Args>
    inline bool TryEncode(const ApiObjectBase* encoder,
                          EncodeFunction&& encodeFunction,
                          const char* formatStr,
                          const Args&... args) {
        if (ConsumedError(ValidateCanEncodeOn(encoder), formatStr, args...)) {
            return false;
        }
        DAWN_ASSERT(!mWereCommandsAcquired);
        return !ConsumedError(encodeFunction(&mPendingCommands), formatStr, args...);
    }

    // Must be called prior to encoding a BeginRenderPassCmd. Note that it's OK to call this
    // and then not actually call EnterPass+ExitRenderPass, for example if some other pass setup
    // failed validation before the BeginRenderPassCmd could be encoded.
    void WillBeginRenderPass();

    // Functions to set current encoder state
    void EnterPass(const ApiObjectBase* passEncoder);
    MaybeError ExitRenderPass(const ApiObjectBase* passEncoder,
                              RenderPassResourceUsageTracker usageTracker,
                              CommandEncoder* commandEncoder,
                              IndirectDrawMetadata indirectDrawMetadata);
    void ExitComputePass(const ApiObjectBase* passEncoder, ComputePassResourceUsage usages);
    MaybeError Finish();

    // Called when a pass encoder is deleted. Provides an opportunity to clean up if it's the
    // mCurrentEncoder.
    void EnsurePassExited(const ApiObjectBase* passEncoder);

    const RenderPassUsages& GetRenderPassUsages() const;
    const ComputePassUsages& GetComputePassUsages() const;
    RenderPassUsages AcquireRenderPassUsages();
    ComputePassUsages AcquireComputePassUsages();
    std::vector<IndirectDrawMetadata> AcquireIndirectDrawMetadata();

    void PushDebugGroupLabel(std::string_view groupLabel);
    void PopDebugGroupLabel();

  private:
    enum class Status {
        Open,
        Finished,
        ErrorAtCreation,
        ErrorInRecording,
        Destroyed,
    };

    void CommitCommands(CommandAllocator allocator);
    void CloseWithStatus(Status status);

    raw_ptr<DeviceBase> mDevice;

    // There can only be two levels of encoders. Top-level and render/compute pass.
    // The top level encoder is the encoder the EncodingContext is created with.
    // It doubles as a flag for mStatus != Open when it is nullptr, so that a single comparison
    // is necessary in ValidateCanEncodeOn.
    raw_ptr<const ApiObjectBase> mTopLevelEncoder;
    // The current encoder must be the same as the encoder provided to TryEncode,
    // otherwise an error is produced. It may be nullptr if the EncodingContext is an error.
    // The current encoder changes with Enter/ExitPass which should be called by
    // CommandEncoder::Begin/EndPass.
    raw_ptr<const ApiObjectBase> mCurrentEncoder;

    RenderPassUsages mRenderPassUsages;
    bool mWereRenderPassUsagesAcquired = false;
    ComputePassUsages mComputePassUsages;
    bool mWereComputePassUsagesAcquired = false;

    // One for each render pass.
    std::vector<IndirectDrawMetadata> mIndirectDrawMetadata;
    bool mWereIndirectDrawMetadataAcquired = false;

    CommandAllocator mPendingCommands;

    std::vector<CommandAllocator> mAllocators;
    bool mWereCommandsAcquired = false;
    // Contains pointers to strings allocated inside the command allocators.
    std::vector<std::string_view> mDebugGroupLabels;

    Status mStatus;
    std::unique_ptr<ErrorData> mError;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ENCODINGCONTEXT_H_
