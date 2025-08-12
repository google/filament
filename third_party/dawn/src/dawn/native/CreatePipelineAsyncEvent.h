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

#ifndef SRC_DAWN_NATIVE_CREATEPIPELINEASYNCEVENT_H_
#define SRC_DAWN_NATIVE_CREATEPIPELINEASYNCEVENT_H_

#include <webgpu/webgpu.h>

#include <memory>
#include <string>
#include <type_traits>

#include "dawn/common/Ref.h"
#include "dawn/native/CallbackTaskManager.h"
#include "dawn/native/Error.h"
#include "dawn/native/EventManager.h"
#include "dawn/native/Pipeline.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

class ComputePipelineBase;
class DeviceBase;
class PipelineLayoutBase;
class RenderPipelineBase;
class ShaderModuleBase;

// CreatePipelineAsyncEvent represents the async event managed by event manager,
// and the async task run on a separate task to initialize the pipeline.
template <typename PipelineType, typename CreatePipelineAsyncCallbackInfo>
class CreatePipelineAsyncEvent final : public EventManager::TrackedEvent {
  public:
    using CallbackType = decltype(std::declval<CreatePipelineAsyncCallbackInfo>().callback);

    // Create an event backed by the given wait list event (for async pipeline creation goes through
    // the backend).
    CreatePipelineAsyncEvent(DeviceBase* device,
                             const CreatePipelineAsyncCallbackInfo& callbackInfo,
                             Ref<PipelineType> pipeline,
                             Ref<WaitListEvent> event);
    // Create an event that's ready at creation (for cached results)
    CreatePipelineAsyncEvent(DeviceBase* device,
                             const CreatePipelineAsyncCallbackInfo& callbackInfo,
                             Ref<PipelineType> pipeline);
    // Create an event that's ready at creation (for errors)
    CreatePipelineAsyncEvent(DeviceBase* device,
                             const CreatePipelineAsyncCallbackInfo& callbackInfo,
                             std::unique_ptr<ErrorData> error,
                             StringView label);

    ~CreatePipelineAsyncEvent() override;

    // Entrance call to synchronously initializing the pipeline.
    void InitializeSync();
    // Entrance call to start an AsyncTask initializing the pipeline.
    void InitializeAsync();

    void Complete(EventCompletionType completionType) override;

  private:
    static const char* kDawnHistogramMetricsSuccess;
    static const char* kDawnHistogramMetricsUS;

    void AddOrGetCachedPipeline();

    // Body of pipeline initialization, called synchronously, or wrapped in an AsyncTask run by
    // AsyncTaskManager.
    void InitializeImpl(bool isAsync);

    CallbackType mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;
    // For some errors (e.g. device lost) we still need to resolve and return a pipeline with
    // Pipeline::MakeError. So we need to hold the pipeline and the error separately.
    Ref<PipelineType> mPipeline;
    std::unique_ptr<ErrorData> mError;
    // Used to keep ShaderModuleBase::mTintProgram alive until pipeline initialization is done.
    PipelineBase::ScopedUseShaderPrograms mScopedUseShaderPrograms;
};

using CreateComputePipelineAsyncEvent =
    CreatePipelineAsyncEvent<ComputePipelineBase, WGPUCreateComputePipelineAsyncCallbackInfo>;
using CreateRenderPipelineAsyncEvent =
    CreatePipelineAsyncEvent<RenderPipelineBase, WGPUCreateRenderPipelineAsyncCallbackInfo>;

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CREATEPIPELINEASYNCEVENT_H_
