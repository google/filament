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

#include "dawn/wire/client/ShaderModule.h"

#include <memory>
#include <utility>

#include "dawn/common/StringViewUtils.h"
#include "dawn/wire/client/Client.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {

class ShaderModule::CompilationInfoEvent final : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::CompilationInfo;

    CompilationInfoEvent(const WGPUCompilationInfoCallbackInfo& callbackInfo,
                         Ref<ShaderModule> shader)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2),
          mShader(std::move(shader)) {
        DAWN_ASSERT(mShader != nullptr);
    }

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureId,
                         WGPUCompilationInfoRequestStatus status,
                         const WGPUCompilationInfo* info) {
        if (mShader->mCompilationInfo) {
            // If we already cached the compilation info on the shader, we don't need to do it
            // again. This can happen if we were to call GetCompilationInfo multiple times before
            // the wire flushes.
            return ReadyHook(futureId);
        }

        mStatus = status;

        // Deep copy the WGPUCompilationInfo
        mShader->mMessageStrings.reserve(info->messageCount);
        mShader->mMessages.reserve(info->messageCount);
        for (size_t i = 0; i < info->messageCount; i++) {
            DAWN_ASSERT(info->messages[i].length != WGPU_STRLEN);
            mShader->mMessageStrings.push_back(ToString(info->messages[i].message));
            mShader->mMessages.push_back(info->messages[i]);
            mShader->mMessages[i].message = ToOutputStringView(mShader->mMessageStrings[i]);
        }
        mShader->mCompilationInfo = {nullptr, mShader->mMessages.size(), mShader->mMessages.data()};

        return WireResult::Success;
    }

    WireResult ReadyHook(FutureID futureId) {
        // We call this ReadyHook when we already have a cached compilation on the shader (usually
        // from a previous GetCompilationInfo call).
        DAWN_ASSERT(mShader->mCompilationInfo);
        mStatus = WGPUCompilationInfoRequestStatus_Success;
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        WGPUCompilationInfo* compilationInfo = nullptr;
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUCompilationInfoRequestStatus_CallbackCancelled;
        } else {
            compilationInfo = &(*mShader->mCompilationInfo);
        }

        void* userdata1 = mUserdata1.ExtractAsDangling();
        void* userdata2 = mUserdata2.ExtractAsDangling();
        if (mCallback) {
            mCallback(mStatus, compilationInfo, userdata1, userdata2);
        }
    }

    WGPUCompilationInfoCallback mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    WGPUCompilationInfoRequestStatus mStatus;

    // Strong reference to the shader so that when we call the callback we can pass the
    // compilation info from `mShader`.
    Ref<ShaderModule> mShader;
};

ObjectType ShaderModule::GetObjectType() const {
    return ObjectType::ShaderModule;
}

WGPUFuture ShaderModule::APIGetCompilationInfo(
    const WGPUCompilationInfoCallbackInfo& callbackInfo) {
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(std::make_unique<CompilationInfoEvent>(callbackInfo, this));
    if (!tracked) {
        return {futureIDInternal};
    }

    // If we already have a cached compilation info object, we can set it ready now.
    if (mCompilationInfo) {
        DAWN_CHECK(GetEventManager().SetFutureReady<CompilationInfoEvent>(futureIDInternal) ==
                   WireResult::Success);
        return {futureIDInternal};
    }

    ShaderModuleGetCompilationInfoCmd cmd;
    cmd.shaderModuleId = GetWireId();
    cmd.eventManagerHandle = GetEventManagerHandle();
    cmd.future = {futureIDInternal};

    GetClient()->SerializeCommand(cmd);
    return {futureIDInternal};
}

WireResult Client::DoShaderModuleGetCompilationInfoCallback(ObjectHandle eventManager,
                                                            WGPUFuture future,
                                                            WGPUCompilationInfoRequestStatus status,
                                                            const WGPUCompilationInfo* info) {
    return SetFutureReady<ShaderModule::CompilationInfoEvent>(eventManager, future.id, status,
                                                              info);
}

}  // namespace dawn::wire::client
