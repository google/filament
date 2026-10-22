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

#include "src/dawn/wire/client/ShaderModule.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "absl/strings/str_format.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/wire/client/Client.h"
#include "src/dawn/wire/client/Device.h"
#include "src/utils/compiler.h"

namespace dawn::wire::client {

// static
ShaderModule* ShaderModule::Create(Device* device, const ShaderModuleDescriptor* descriptor) {
    DAWN_ASSERT(descriptor != nullptr);

    Client* client = device->GetClient();
    Ref<ShaderModule> shaderModule = client->Make<ShaderModule>(device->GetInstance());

    ShaderModuleDescriptor desc = *descriptor;
    desc.nextInChain = nullptr;

    // In Dawn wire client, we replace ShaderSourceSPIRV extensions with DawnShaderSourceSPIRV
    // extensions. This requires us to potentially make a deep copy of the extensions to re-create
    // the chain, replacing any ShaderSourceSPIRVs along the way. When/if we run into duplicate or
    // invalid extensions, instead of trying to forward the descriptor to the server, we just ask
    // the server to create an error shader module instead.
    std::optional<DawnShaderSourceSPIRV> dawnShaderSourceSPIRV;
    std::optional<ShaderSourceWGSL> shaderSourceWGSL;
    std::optional<DawnShaderModuleSPIRVOptionsDescriptor> dawnShaderModuleSPIRVOptionsDescriptor;
    std::optional<ShaderModuleCompilationOptions> shaderModuleCompilationOptions;

    std::vector<ChainedStruct*> chainOrder;
    std::string errorMessage;
    for (const ChainedStruct* chain = descriptor->nextInChain; chain != nullptr;
         chain = chain->nextInChain) {
        switch (chain->sType) {
            case wgpu::SType::ShaderSourceSPIRV: {
                if (dawnShaderSourceSPIRV.has_value()) {
                    errorMessage = "Duplicate chained struct of type ShaderSourceSPIRV.";
                    break;
                }
                const auto* spirv = reinterpret_cast<const wgpu::ShaderSourceSPIRV*>(chain);
                DawnShaderSourceSPIRV dawnSpirv;
                // SAFETY: The application must ensure that `code` points at `codeSize` uint32_ts.
                DAWN_UNSAFE_BUFFERS(dawnSpirv.code = {spirv->code, spirv->codeSize});
                dawnShaderSourceSPIRV = dawnSpirv;
                chainOrder.push_back(&*dawnShaderSourceSPIRV);
                break;
            }
            case wgpu::SType::DawnShaderSourceSPIRV:
                if (dawnShaderSourceSPIRV.has_value()) {
                    errorMessage = "Duplicate chained struct of type DawnShaderSourceSPIRV.";
                    break;
                }
                dawnShaderSourceSPIRV = *reinterpret_cast<const DawnShaderSourceSPIRV*>(chain);
                chainOrder.push_back(&*dawnShaderSourceSPIRV);
                break;
            case wgpu::SType::ShaderSourceWGSL:
                if (shaderSourceWGSL.has_value()) {
                    errorMessage = "Duplicate chained struct of type ShaderSourceWGSL.";
                    break;
                }
                shaderSourceWGSL = *reinterpret_cast<const ShaderSourceWGSL*>(chain);
                chainOrder.push_back(&*shaderSourceWGSL);
                break;
            case wgpu::SType::DawnShaderModuleSPIRVOptionsDescriptor:
                if (dawnShaderModuleSPIRVOptionsDescriptor.has_value()) {
                    errorMessage =
                        "Duplicate chained struct of type DawnShaderModuleSPIRVOptionsDescriptor.";
                    break;
                }
                dawnShaderModuleSPIRVOptionsDescriptor =
                    *reinterpret_cast<const DawnShaderModuleSPIRVOptionsDescriptor*>(chain);
                chainOrder.push_back(&*dawnShaderModuleSPIRVOptionsDescriptor);
                break;
            case wgpu::SType::ShaderModuleCompilationOptions:
                if (shaderModuleCompilationOptions.has_value()) {
                    errorMessage =
                        "Duplicate chained struct of type ShaderModuleCompilationOptions.";
                    break;
                }
                shaderModuleCompilationOptions =
                    *reinterpret_cast<const ShaderModuleCompilationOptions*>(chain);
                chainOrder.push_back(&*shaderModuleCompilationOptions);
                break;
            default:
                errorMessage =
                    absl::StrFormat("Unsupported or invalid chained struct with SType (%u).",
                                    static_cast<uint32_t>(chain->sType));
                break;
        }
        if (!errorMessage.empty()) {
            break;
        }
    }

    if (!errorMessage.empty()) {
        DeviceCreateErrorShaderModuleCmd cmd;
        cmd.self = ToAPI(device);
        cmd.descriptor = ToAPI(&desc);
        cmd.errorMessage = ToOutputStringView(errorMessage);
        cmd.result = shaderModule->GetWireHandle(client);
        client->SerializeCommand(cmd);
        return ReturnToAPI2(std::move(shaderModule));
    }

    const ChainedStruct** last = &desc.nextInChain;
    for (ChainedStruct* current : chainOrder) {
        current->nextInChain = nullptr;
        *last = current;
        last = &current->nextInChain;
    }

    DeviceCreateShaderModuleCmd cmd;
    cmd.self = ToAPI(device);
    cmd.descriptor = ToAPI(&desc);
    cmd.result = shaderModule->GetWireHandle(client);
    client->SerializeCommand(cmd);
    return ReturnToAPI2(std::move(shaderModule));
}

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
        mShader->mUtf16s.reserve(info->messageCount);
        for (size_t i = 0; i < info->messageCount; i++) {
            DAWN_UNSAFE_TODO(DAWN_ASSERT(info->messages[i].length != WGPU_STRLEN));
            mShader->mMessageStrings.push_back(
                ToString(DAWN_UNSAFE_TODO(info->messages[i]).message));
            mShader->mMessages.push_back(DAWN_UNSAFE_TODO(info->messages[i]));
            mShader->mMessages[i].message = ToOutputStringView(mShader->mMessageStrings[i]);
            mShader->mMessages[i].nextInChain = nullptr;

            // Iterate the message chain for extensions that we want to handle.
            WGPUChainedStruct** tail = &mShader->mMessages[i].nextInChain;
            WGPUChainedStruct* chain = DAWN_UNSAFE_TODO(info->messages[i]).nextInChain;
            // Guard against duplicates, to avoid a reallocation on the destination vector.
            // Duplicate structs of the same type are not valid in the first place, so don't
            // do much to try to recover or error out.
            bool seenDawnCompilationMessageUtf16 = false;
            while (chain != nullptr) {
                switch (chain->sType) {
                    case WGPUSType_DawnCompilationMessageUtf16: {
                        if (!seenDawnCompilationMessageUtf16) {
                            seenDawnCompilationMessageUtf16 = true;
                            mShader->mUtf16s.push_back(
                                *reinterpret_cast<const WGPUDawnCompilationMessageUtf16*>(chain));
                            *tail = &(mShader->mUtf16s.back().chain);
                            tail = &((*tail)->next);
                        }
                        break;
                    }
                    default:
                        break;
                }

                chain = chain->next;
            }

            // Ensure that the tail is pointing to nothing else.
            *tail = nullptr;
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

Future ShaderModule::APIGetCompilationInfo(const WGPUCompilationInfoCallbackInfo& callbackInfo) {
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(AcquireRef(new CompilationInfoEvent(callbackInfo, this)));
    if (!tracked) {
        return {futureIDInternal};
    }

    // If we already have a cached compilation info object, we can set it ready now.
    if (mCompilationInfo) {
        auto wireStatus = GetEventManager().SetFutureReady<CompilationInfoEvent>(futureIDInternal);
        DAWN_CHECK(wireStatus == WireResult::Success);
        return {futureIDInternal};
    }

    ShaderModuleGetCompilationInfoCmd cmd;
    cmd.shaderModuleId = GetWireHandle(GetClient()).id;
    cmd.instanceId = GetInstance()->GetWireHandle(GetClient()).id;
    cmd.future = {futureIDInternal};

    GetClient()->SerializeCommand(cmd);
    return {futureIDInternal};
}

WireResult Client::DoShaderModuleGetCompilationInfoCallback(ObjectId instanceId,
                                                            WGPUFuture future,
                                                            WGPUCompilationInfoRequestStatus status,
                                                            const WGPUCompilationInfo* info) {
    return SetFutureReady<ShaderModule::CompilationInfoEvent>(instanceId, future.id, status, info);
}

}  // namespace dawn::wire::client
