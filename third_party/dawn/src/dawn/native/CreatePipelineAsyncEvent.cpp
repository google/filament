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

#include "dawn/native/CreatePipelineAsyncEvent.h"

#include <webgpu/webgpu.h>

#include <utility>

#include "dawn/common/FutureUtils.h"
#include "dawn/common/Ref.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/AsyncTask.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Device.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/EventManager.h"
#include "dawn/native/Instance.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/SystemEvent.h"
#include "dawn/native/dawn_platform_autogen.h"
#include "dawn/native/utils/WGPUHelpers.h"
#include "dawn/native/wgpu_structs_autogen.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native {

template <>
const char* CreatePipelineAsyncEvent<
    ComputePipelineBase,
    WGPUCreateComputePipelineAsyncCallbackInfo>::kDawnHistogramMetricsSuccess =
    "CreateComputePipelineSuccess";
template <>
const char*
    CreatePipelineAsyncEvent<ComputePipelineBase,
                             WGPUCreateComputePipelineAsyncCallbackInfo>::kDawnHistogramMetricsUS =
        "CreateComputePipelineUS";
template <>
void CreatePipelineAsyncEvent<ComputePipelineBase, WGPUCreateComputePipelineAsyncCallbackInfo>::
    AddOrGetCachedPipeline() {
    DeviceBase* device = mPipeline->GetDevice();
    auto deviceLock(device->GetScopedLock());
    if (device->GetState() == DeviceBase::State::Alive) {
        mPipeline = device->AddOrGetCachedComputePipeline(std::move(mPipeline));
    }
}

template <>
const char* CreatePipelineAsyncEvent<
    RenderPipelineBase,
    WGPUCreateRenderPipelineAsyncCallbackInfo>::kDawnHistogramMetricsSuccess =
    "CreateRenderPipelineSuccess";
template <>
const char*
    CreatePipelineAsyncEvent<RenderPipelineBase,
                             WGPUCreateRenderPipelineAsyncCallbackInfo>::kDawnHistogramMetricsUS =
        "CreateRenderPipelineUS";
template <>
void CreatePipelineAsyncEvent<RenderPipelineBase,
                              WGPUCreateRenderPipelineAsyncCallbackInfo>::AddOrGetCachedPipeline() {
    DeviceBase* device = mPipeline->GetDevice();
    auto deviceLock(device->GetScopedLock());
    if (device->GetState() == DeviceBase::State::Alive) {
        mPipeline = device->AddOrGetCachedRenderPipeline(std::move(mPipeline));
    }
}

template <typename PipelineType, typename CreatePipelineAsyncCallbackInfo>
CreatePipelineAsyncEvent<PipelineType, CreatePipelineAsyncCallbackInfo>::CreatePipelineAsyncEvent(
    DeviceBase* device,
    const CreatePipelineAsyncCallbackInfo& callbackInfo,
    Ref<PipelineType> pipeline,
    Ref<SystemEvent> systemEvent)
    : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode), std::move(systemEvent)),
      mCallback(callbackInfo.callback),
      mUserdata1(callbackInfo.userdata1),
      mUserdata2(callbackInfo.userdata2),
      mPipeline(std::move(pipeline)),
      mScopedUseShaderPrograms(mPipeline->UseShaderPrograms()) {}

template <typename PipelineType, typename CreatePipelineAsyncCallbackInfo>
CreatePipelineAsyncEvent<PipelineType, CreatePipelineAsyncCallbackInfo>::CreatePipelineAsyncEvent(
    DeviceBase* device,
    const CreatePipelineAsyncCallbackInfo& callbackInfo,
    Ref<PipelineType> pipeline)
    : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode), TrackedEvent::Completed{}),
      mCallback(callbackInfo.callback),
      mUserdata1(callbackInfo.userdata1),
      mUserdata2(callbackInfo.userdata2),
      mPipeline(std::move(pipeline)) {}

template <typename PipelineType, typename CreatePipelineAsyncCallbackInfo>
CreatePipelineAsyncEvent<PipelineType, CreatePipelineAsyncCallbackInfo>::CreatePipelineAsyncEvent(
    DeviceBase* device,
    const CreatePipelineAsyncCallbackInfo& callbackInfo,
    std::unique_ptr<ErrorData> error,
    StringView label)
    : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode), TrackedEvent::Completed{}),
      mCallback(callbackInfo.callback),
      mUserdata1(callbackInfo.userdata1),
      mUserdata2(callbackInfo.userdata2),
      mPipeline(PipelineType::MakeError(device, label)),
      mError(std::move(error)) {}

template <typename PipelineType, typename CreatePipelineAsyncCallbackInfo>
CreatePipelineAsyncEvent<PipelineType,
                         CreatePipelineAsyncCallbackInfo>::~CreatePipelineAsyncEvent() {
    EnsureComplete(EventCompletionType::Shutdown);
}

template <typename PipelineType, typename CreatePipelineAsyncCallbackInfo>
void CreatePipelineAsyncEvent<PipelineType, CreatePipelineAsyncCallbackInfo>::InitializeImpl(
    bool isAsync) {
    DeviceBase* device = mPipeline->GetDevice();
    const char* eventLabel = utils::GetLabelForTrace(mPipeline->GetLabel());
    if (isAsync) {
        TRACE_EVENT_FLOW_END1(device->GetPlatform(), General,
                              "CreatePipelineAsyncEvent::InitializeAsync", this, "label",
                              eventLabel);
    }
    TRACE_EVENT1(device->GetPlatform(), General, "CreatePipelineAsyncEvent::InitializeImpl",
                 "label", eventLabel);

    MaybeError maybeError;
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(device->GetPlatform(), kDawnHistogramMetricsUS);
        maybeError = mPipeline->Initialize(std::move(mScopedUseShaderPrograms));
    }
    DAWN_HISTOGRAM_BOOLEAN(device->GetPlatform(), kDawnHistogramMetricsSuccess,
                           maybeError.IsSuccess());
    if (maybeError.IsError()) {
        mError = maybeError.AcquireError();
    }

    // TODO(dawn:2451): API re-entrant callbacks in spontaneous mode that use the device could
    // deadlock itself.
    device->GetInstance()->GetEventManager()->SetFutureReady(this);
}

template <typename PipelineType, typename CreatePipelineAsyncCallbackInfo>
void CreatePipelineAsyncEvent<PipelineType, CreatePipelineAsyncCallbackInfo>::InitializeSync() {
    InitializeImpl(false);
}

template <typename PipelineType, typename CreatePipelineAsyncCallbackInfo>
void CreatePipelineAsyncEvent<PipelineType, CreatePipelineAsyncCallbackInfo>::InitializeAsync() {
    DeviceBase* device = mPipeline->GetDevice();
    const char* eventLabel = utils::GetLabelForTrace(mPipeline->GetLabel());
    TRACE_EVENT_FLOW_BEGIN1(device->GetPlatform(), General,
                            "CreatePipelineAsyncEvent::InitializeAsync", this, "label", eventLabel);

    auto asyncTask = [event = Ref<CreatePipelineAsyncEvent>(this)] { event->InitializeImpl(true); };
    device->GetAsyncTaskManager()->PostTask(std::move(asyncTask));
}

template <typename PipelineType, typename CreatePipelineAsyncCallbackInfo>
void CreatePipelineAsyncEvent<PipelineType, CreatePipelineAsyncCallbackInfo>::Complete(
    EventCompletionType completionType) {
    auto userdata1 = mUserdata1.ExtractAsDangling();
    auto userdata2 = mUserdata2.ExtractAsDangling();

    if (completionType == EventCompletionType::Shutdown) {
        if (mCallback) {
            mCallback(WGPUCreatePipelineAsyncStatus_CallbackCancelled, nullptr,
                      ToOutputStringView("Instance dropped"), userdata1, userdata2);
        }
        return;
    }

    DeviceBase* device = mPipeline->GetDevice();
    // TODO(dawn:2353): Device losts later than this check could potentially lead to racing
    // condition.
    if (device->IsLost()) {
        // Invalid async creation should "succeed" if the device is already lost.
        if (!mPipeline->IsError()) {
            mPipeline = PipelineType::MakeError(device, mPipeline->GetLabel().c_str());
        }
        if (mCallback) {
            mCallback(WGPUCreatePipelineAsyncStatus_Success,
                      ToAPI(ReturnToAPI(std::move(mPipeline))), kEmptyOutputStringView, userdata1,
                      userdata2);
        }
        return;
    }

    if (mError != nullptr) {
        WGPUCreatePipelineAsyncStatus status;
        switch (mError->GetType()) {
            case InternalErrorType::Validation:
                status = WGPUCreatePipelineAsyncStatus_ValidationError;
                break;
            default:
                status = WGPUCreatePipelineAsyncStatus_InternalError;
                break;
        }
        if (mCallback) {
            mCallback(status, nullptr, ToOutputStringView(mError->GetFormattedMessage()), userdata1,
                      userdata2);
        }
        return;
    }

    AddOrGetCachedPipeline();
    if (mCallback) {
        mCallback(WGPUCreatePipelineAsyncStatus_Success, ToAPI(ReturnToAPI(std::move(mPipeline))),
                  kEmptyOutputStringView, userdata1, userdata2);
    }
}

template class CreatePipelineAsyncEvent<ComputePipelineBase,
                                        WGPUCreateComputePipelineAsyncCallbackInfo>;
template class CreatePipelineAsyncEvent<RenderPipelineBase,
                                        WGPUCreateRenderPipelineAsyncCallbackInfo>;

}  // namespace dawn::native
