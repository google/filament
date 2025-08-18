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

#include "dawn/native/Device.h"

#include <webgpu/webgpu.h>

#include <algorithm>
#include <array>
#include <mutex>
#include <string>
#include <utility>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_format.h"
#include "dawn/common/Log.h"
#include "dawn/common/Ref.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/common/Version_autogen.h"
#include "dawn/native/AsyncTask.h"
#include "dawn/native/AttachmentState.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/BlitBufferToDepthStencil.h"
#include "dawn/native/BlobCache.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CacheRequest.h"
#include "dawn/native/CacheResult.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CompilationMessages.h"
#include "dawn/native/CreatePipelineAsyncEvent.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/Error.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/ErrorInjector.h"
#include "dawn/native/ErrorScope.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/Instance.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/PipelineCache.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderBundleEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/ShaderModuleParseRequest.h"
#include "dawn/native/SharedBufferMemory.h"
#include "dawn/native/SharedFence.h"
#include "dawn/native/SharedTextureMemory.h"
#include "dawn/native/Surface.h"
#include "dawn/native/SwapChain.h"
#include "dawn/native/Texture.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/native/WaitListEvent.h"
#include "dawn/native/utils/WGPUHelpers.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

// DeviceBase sub-structures

struct DeviceBase::Caches {
    ContentLessObjectCache<AttachmentState> attachmentStates;
    ContentLessObjectCache<BindGroupLayoutInternalBase> bindGroupLayouts;
    ContentLessObjectCache<ComputePipelineBase> computePipelines;
    ContentLessObjectCache<PipelineLayoutBase> pipelineLayouts;
    ContentLessObjectCache<RenderPipelineBase> renderPipelines;
    ContentLessObjectCache<SamplerBase> samplers;
    ContentLessObjectCache<ShaderModuleBase> shaderModules;
};

// Tries to find an object in the cache, creating and inserting into the cache if not found.
template <typename RefCountedT, typename CreateFn>
auto GetOrCreate(ContentLessObjectCache<RefCountedT>& cache,
                 RefCountedT* blueprint,
                 CreateFn createFn) {
    using ReturnType = decltype(createFn());

    // If we find the blueprint in the cache we can just return it.
    Ref<RefCountedT> result = cache.Find(blueprint);
    if (result != nullptr) {
        return ReturnType(result);
    }

    using UnwrappedReturnType = typename detail::UnwrapResultOrError<ReturnType>::type;
    static_assert(std::is_same_v<UnwrappedReturnType, Ref<RefCountedT>>,
                  "CreateFn should return an unwrapped type that is the same as Ref<RefCountedT>.");

    // Create the result and try inserting it. Note that inserts can race because the critical
    // sections here is disjoint, hence the checks to verify whether this thread inserted.
    if constexpr (!detail::IsResultOrError<ReturnType>::value) {
        result = createFn();
    } else {
        auto resultOrError = createFn();
        if (resultOrError.IsError()) [[unlikely]] {
            return ReturnType(std::move(resultOrError.AcquireError()));
        }
        result = resultOrError.AcquireSuccess();
    }
    DAWN_ASSERT(result.Get() != nullptr);

    bool inserted = false;
    std::tie(result, inserted) = cache.Insert(result.Get());
    return ReturnType(result);
}

namespace {

static constexpr WGPUUncapturedErrorCallbackInfo kEmptyUncapturedErrorCallbackInfo = {
    nullptr, nullptr, nullptr, nullptr};
static constexpr WGPULoggingCallbackInfo kEmptyLoggingCallbackInfo = {nullptr, nullptr, nullptr,
                                                                      nullptr};

void TrimErrorScopeStacks(
    absl::flat_hash_map<ThreadUniqueId, std::unique_ptr<ErrorScopeStack>>& errorScopeStacks) {
    for (auto it = errorScopeStacks.begin(); it != errorScopeStacks.end();) {
        if (!IsThreadAlive(it->first)) {
            errorScopeStacks.erase(it++);
        } else {
            it++;
        }
    }
}

}  // anonymous namespace

DeviceBase::DeviceLostEvent::DeviceLostEvent(const WGPUDeviceLostCallbackInfo& callbackInfo)
    : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode),
                   TrackedEvent::NonProgressing{}),
      mCallback(callbackInfo.callback),
      mUserdata1(callbackInfo.userdata1),
      mUserdata2(callbackInfo.userdata2) {}

DeviceBase::DeviceLostEvent::~DeviceLostEvent() {
    EnsureComplete(EventCompletionType::Shutdown);
}

// static
Ref<DeviceBase::DeviceLostEvent> DeviceBase::DeviceLostEvent::Create(
    const DeviceDescriptor* descriptor) {
    DAWN_ASSERT(descriptor != nullptr);

#if defined(DAWN_ENABLE_ASSERTS)
    static constexpr WGPUDeviceLostCallbackInfo kDefaultDeviceLostCallbackInfo = {
        nullptr, WGPUCallbackMode_AllowSpontaneous,
        [](WGPUDevice const*, WGPUDeviceLostReason, WGPUStringView, void*, void*) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No Dawn device lost callback was set. This is probably not "
                                      "intended. If you really want to ignore device lost and "
                                      "suppress this message, set the callback explicitly.";
            }
        },
        nullptr, nullptr};
#else
    static constexpr WGPUDeviceLostCallbackInfo kDefaultDeviceLostCallbackInfo = {
        nullptr, WGPUCallbackMode_AllowProcessEvents, nullptr, nullptr, nullptr};
#endif  // DAWN_ENABLE_ASSERTS

    WGPUDeviceLostCallbackInfo deviceLostCallbackInfo = kDefaultDeviceLostCallbackInfo;
    if (descriptor->deviceLostCallbackInfo.callback != nullptr) {
        deviceLostCallbackInfo = descriptor->deviceLostCallbackInfo;
    }
    return AcquireRef(new DeviceBase::DeviceLostEvent(deviceLostCallbackInfo));
}

void DeviceBase::DeviceLostEvent::SetLost(EventManager* eventManager,
                                          wgpu::DeviceLostReason reason,
                                          std::string_view message) {
    mReason = reason;
    mMessage = message;
    eventManager->SetFutureReady(this);
    if (mDevice) {
        // If the device was already set, then the device must be associated with this event. Since
        // the event should only be set and triggered once, unset the event in the device now.
        mDevice->mLostFuture = GetFuture();
        mDevice->mLostEvent = nullptr;
    }
}

void DeviceBase::DeviceLostEvent::Complete(EventCompletionType completionType) {
    if (completionType == EventCompletionType::Shutdown) {
        mReason = wgpu::DeviceLostReason::CallbackCancelled;
        mMessage = "A valid external Instance reference no longer exists.";
    }

    // Some users may use the device lost callback to deallocate resources allocated for the
    // uncaptured error and logging callbacks, so reset these callbacks before calling the
    // device lost callback.
    if (mDevice != nullptr) {
        mDevice->mUncapturedErrorCallbackInfo = kEmptyUncapturedErrorCallbackInfo;
        {
            std::lock_guard<std::shared_mutex> lock(mDevice->mLoggingMutex);
            mDevice->mLoggingCallbackInfo = kEmptyLoggingCallbackInfo;
        }
    }

    auto device = ToAPI(mDevice.Get());
    void* userdata1 = mUserdata1.ExtractAsDangling();
    void* userdata2 = mUserdata2.ExtractAsDangling();

    if (mReason == wgpu::DeviceLostReason::CallbackCancelled ||
        mReason == wgpu::DeviceLostReason::FailedCreation) {
        device = nullptr;
    }
    if (mCallback) {
        mCallback(&device, ToAPI(mReason), ToOutputStringView(mMessage), userdata1, userdata2);
    }

    // Break the ref cycle between DeviceBase and DeviceLostEvent.
    mDevice = nullptr;
}

ResultOrError<Ref<PipelineLayoutBase>> ValidateLayoutAndGetComputePipelineDescriptorWithDefaults(
    DeviceBase* device,
    const ComputePipelineDescriptor& descriptor,
    ComputePipelineDescriptor* outDescriptor) {
    Ref<PipelineLayoutBase> layoutRef;
    *outDescriptor = descriptor;

    if (outDescriptor->layout == nullptr) {
        DAWN_TRY_ASSIGN(layoutRef,
                        PipelineLayoutBase::CreateDefault(device,
                                                          {{
                                                              SingleShaderStage::Compute,
                                                              outDescriptor->compute.module,
                                                              outDescriptor->compute.entryPoint,
                                                              outDescriptor->compute.constantCount,
                                                              outDescriptor->compute.constants,
                                                          }},
                                                          /*allowInternalBinding=*/false));
        outDescriptor->layout = layoutRef.Get();
    }

    return layoutRef;
}

ResultOrError<Ref<PipelineLayoutBase>> ValidateLayoutAndGetRenderPipelineDescriptorWithDefaults(
    DeviceBase* device,
    const RenderPipelineDescriptor& descriptor,
    RenderPipelineDescriptor* outDescriptor,
    bool allowInternalBinding) {
    Ref<PipelineLayoutBase> layoutRef;
    *outDescriptor = descriptor;

    if (descriptor.layout == nullptr) {
        // Ref will keep the pipeline layout alive until the end of the function where
        // the pipeline will take another reference.
        DAWN_TRY_ASSIGN(layoutRef,
                        PipelineLayoutBase::CreateDefault(
                            device, GetRenderStagesAndSetPlaceholderShader(device, &descriptor),
                            allowInternalBinding));
        outDescriptor->layout = layoutRef.Get();
    }

    return layoutRef;
}

// DeviceBase

DeviceBase::DeviceBase(AdapterBase* adapter,
                       const UnpackedPtr<DeviceDescriptor>& descriptor,
                       const TogglesState& deviceToggles,
                       Ref<DeviceLostEvent>&& lostEvent)
    : mLostEvent(std::move(lostEvent)),
      mAdapter(adapter),
      mToggles(deviceToggles),
      mNextPipelineCompatibilityToken(1) {
    DAWN_ASSERT(descriptor);

    DAWN_ASSERT(mLostEvent);
    mLostEvent->mDevice = this;

#if defined(DAWN_ENABLE_ASSERTS)
    static constexpr WGPUUncapturedErrorCallbackInfo kDefaultUncapturedErrorCallbackInfo = {
        nullptr,
        [](WGPUDevice const*, WGPUErrorType, WGPUStringView, void*, void*) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No Dawn device uncaptured error callback was set. This is "
                                      "probably not intended. If you really want to ignore errors "
                                      "and suppress this message, set the callback explicitly.";
            }
        },
        nullptr, nullptr};
#else
    static constexpr WGPUUncapturedErrorCallbackInfo kDefaultUncapturedErrorCallbackInfo =
        kEmptyUncapturedErrorCallbackInfo;
#endif  // DAWN_ENABLE_ASSERTS
    mUncapturedErrorCallbackInfo = kDefaultUncapturedErrorCallbackInfo;
    if (descriptor->uncapturedErrorCallbackInfo.callback != nullptr) {
        mUncapturedErrorCallbackInfo = descriptor->uncapturedErrorCallbackInfo;
    }

    AdapterInfo adapterInfo;
    adapter->APIGetInfo(&adapterInfo);

    ApplyFeatures(descriptor, adapter->GetFeatureLevel());

    auto effectiveFeatureLevel = HasFeature(Feature::CoreFeaturesAndLimits)
                                     ? wgpu::FeatureLevel::Core
                                     : wgpu::FeatureLevel::Compatibility;

    DawnCacheDeviceDescriptor cacheDesc = {};
    const auto* cacheDescIn = descriptor.Get<DawnCacheDeviceDescriptor>();
    if (cacheDescIn != nullptr) {
        cacheDesc = *cacheDescIn;
    }

    if (cacheDesc.loadDataFunction == nullptr && cacheDesc.storeDataFunction == nullptr &&
        cacheDesc.functionUserdata == nullptr && GetPlatform()->GetCachingInterface() != nullptr) {
        // Populate cache functions and userdata from legacy cachingInterface.
        cacheDesc.loadDataFunction = [](const void* key, size_t keySize, void* value,
                                        size_t valueSize, void* userdata) {
            auto* cachingInterface = static_cast<dawn::platform::CachingInterface*>(userdata);
            return cachingInterface->LoadData(key, keySize, value, valueSize);
        };
        cacheDesc.storeDataFunction = [](const void* key, size_t keySize, const void* value,
                                         size_t valueSize, void* userdata) {
            auto* cachingInterface = static_cast<dawn::platform::CachingInterface*>(userdata);
            return cachingInterface->StoreData(key, keySize, value, valueSize);
        };
        cacheDesc.functionUserdata = GetPlatform()->GetCachingInterface();
    }

    // Disable caching if the DisableBlobCache toggle is enabled.
    if (IsToggleEnabled(Toggle::DisableBlobCache)) {
        cacheDesc.loadDataFunction = nullptr;
        cacheDesc.storeDataFunction = nullptr;
        cacheDesc.functionUserdata = nullptr;
    }

    mBlobCache =
        std::make_unique<BlobCache>(cacheDesc, IsToggleEnabled(Toggle::BlobCacheHashValidation));

    if (descriptor->requiredLimits != nullptr) {
        UnpackLimitsIn(descriptor->requiredLimits, &mLimits);
        mLimits = ReifyDefaultLimits(mLimits, effectiveFeatureLevel);
    } else {
        GetDefaultLimits(&mLimits, effectiveFeatureLevel);
    }

    // Get texelCopyBufferRowAlignmentLimits from physical device
    mLimits.texelCopyBufferRowAlignmentLimits =
        GetPhysicalDevice()->GetLimits().texelCopyBufferRowAlignmentLimits;

    // Get hostMappedPointerLimits from physical device
    mLimits.hostMappedPointerLimits = GetPhysicalDevice()->GetLimits().hostMappedPointerLimits;

    // Handle maxXXXPerStage/maxXXXInStage.
    EnforceLimitSpecInvariants(&mLimits, effectiveFeatureLevel);

    if (mLimits.compat.maxStorageBuffersInFragmentStage < 1) {
        // If there is no storage buffer in fragment stage, UseBlitForB2T is not possible.
        mToggles.ForceSet(Toggle::UseBlitForB2T, false);
    }

    mFormatTable = BuildFormatTable(this);

    if (!descriptor->label.IsUndefined()) {
        mLabel = std::string(descriptor->label);
    }

    mIsImmediateErrorHandlingEnabled = IsToggleEnabled(Toggle::EnableImmediateErrorHandling);

    // Generate entry point name from isolation key if provided.
    if (!cacheDesc.isolationKey.IsUndefined()) {
        std::string_view isolationKey = cacheDesc.isolationKey;

        std::stringstream ss;
        ss << "dawn_entry_point_";
        // Combine with hexadecimal representation of bytes in isolation key.
        for (const char& byte : isolationKey) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(byte);
        }

        mIsolatedEntryPointName = ss.str();
    } else {
        mIsolatedEntryPointName = "dawn_entry_point";
    }

    // Record the cache key from the adapter info. Note that currently, if a new extension
    // descriptor is added (and probably handled here), the cache key recording needs to be
    // updated.
    StreamIn(&mDeviceCacheKey, kDawnVersion, adapterInfo, mEnabledFeatures.featuresBitSet, mToggles,
             cacheDesc);
}

DeviceBase::DeviceBase() : mState(State::Alive), mToggles(ToggleStage::Device) {
    GetDefaultLimits(&mLimits, wgpu::FeatureLevel::Core);
    EnforceLimitSpecInvariants(&mLimits, wgpu::FeatureLevel::Core);
    mFormatTable = BuildFormatTable(this);

    DeviceDescriptor desc = {};
    desc.deviceLostCallbackInfo = {nullptr, WGPUCallbackMode_AllowSpontaneous, nullptr, nullptr,
                                   nullptr};
    mLostEvent = DeviceLostEvent::Create(&desc);
    mLostEvent->mDevice = this;
}

DeviceBase::~DeviceBase() {
    // We need to explicitly release the Queue before we complete the destructor so that the
    // Queue does not get destroyed after the Device.
    mQueue = nullptr;
}

MaybeError DeviceBase::Initialize(const UnpackedPtr<DeviceDescriptor>& descriptor,
                                  Ref<QueueBase> defaultQueue) {
    mQueue = std::move(defaultQueue);

    SetWGSLExtensionAllowList();

    mCaches = std::make_unique<DeviceBase::Caches>();
    mDynamicUploader = std::make_unique<DynamicUploader>(this);
    mCallbackTaskManager = AcquireRef(new CallbackTaskManager());
    mInternalPipelineStore = std::make_unique<InternalPipelineStore>(this);

    DAWN_ASSERT(GetPlatform() != nullptr);
    mWorkerTaskPool = GetPlatform()->CreateWorkerTaskPool();
    mAsyncTaskManager = std::make_unique<AsyncTaskManager>(mWorkerTaskPool.get());

    // Starting from now the backend can start doing reentrant calls so the device is marked as
    // alive.
    mState = State::Alive;

    // Fake an error after the creation of a device here for testing.
    if (descriptor.Has<DawnFakeDeviceInitializeErrorForTesting>()) {
        return DAWN_INTERNAL_ERROR("DawnFakeDeviceInitialzeErrorForTesting");
    }

    DAWN_TRY_ASSIGN(mEmptyBindGroupLayout, CreateEmptyBindGroupLayout());
    DAWN_TRY_ASSIGN(mEmptyPipelineLayout, CreateEmptyPipelineLayout());

    // If placeholder fragment shader module is needed, initialize it
    if (IsToggleEnabled(Toggle::UsePlaceholderFragmentInVertexOnlyPipeline)) {
        // The empty fragment shader, used as a work around for vertex-only render pipeline
        constexpr char kEmptyFragmentShader[] = R"(
                @fragment fn fs_empty_main() {}
            )";
        ShaderModuleDescriptor shaderDesc;
        ShaderSourceWGSL wgslDesc;
        wgslDesc.code = kEmptyFragmentShader;
        shaderDesc.nextInChain = &wgslDesc;

        DAWN_TRY_ASSIGN(mInternalPipelineStore->placeholderFragmentShader,
                        CreateShaderModule(&shaderDesc, /* internalExtensions */ {}));
    }

    if (HasFeature(Feature::ImplicitDeviceSynchronization)) {
        mMutex = AcquireRef(new DeviceMutex);
    } else {
        mMutex = nullptr;
    }

    GetInstance()->AddDevice(this);

    return {};
}

void DeviceBase::WillDropLastExternalRef() {
    {
        // This will be invoked by API side, so we need to lock.
        // Note: we cannot hold the lock when flushing the callbacks so have to limit the scope of
        // the lock.
        auto deviceGuard = GetGuard();

        // Set DeviceLostEvent to pass a null device to the callback (which may happen in Destroy()
        // depending on the CallbackMode). This also makes DeviceLostEvent skip unregistering the
        // UncapturedError and Logging callbacks; they'll be unregistered later in this function.
        if (mLostEvent) {
            mLostEvent->mDevice = nullptr;
        }

        // DeviceBase uses RefCountedWithExternalCount to break refcycles.
        //
        // DeviceBase holds multiple Refs to various API objects (pipelines, buffers, etc.) which
        // are used to implement various device-level facilities. These objects are cached on the
        // device, so we want to keep them around instead of making transient allocations. However,
        // many of the objects also hold a Ref<Device> back to their parent device.
        //
        // In order to break this cycle and prevent leaks, when the application drops the last
        // external ref and WillDropLastExternalRef is called, the device clears out any member refs
        // to API objects that hold back-refs to the device - thus breaking any reference cycles.
        //
        // Currently, this is done by calling Destroy on the device to cease all in-flight work and
        // drop references to internal objects. We may want to lift this in the future, but it would
        // make things more complex because there might be pending tasks which hold a ref back to
        // the device - either directly or indirectly. We would need to ensure those tasks don't
        // create new reference cycles, and we would need to continuously try draining the pending
        // tasks to clear out all remaining refs.
        Destroy();
    }

    // Flush last remaining callback tasks.
    if (mCallbackTaskManager) {
        do {
            FlushCallbackTaskQueue();
        } while (!mCallbackTaskManager->IsEmpty());
    }

    auto deviceGuard = GetGuard();
    // Drop the device's reference to the queue. Because the application dropped the last external
    // reference, it's UB if they try to get the queue from APIGetQueue().
    mQueue = nullptr;

    // Reset callbacks since after dropping the last external reference, the application may have
    // freed any device-scope memory needed to run the callback.
    mUncapturedErrorCallbackInfo = kEmptyUncapturedErrorCallbackInfo;
    {
        std::lock_guard<std::shared_mutex> lock(mLoggingMutex);
        mLoggingCallbackInfo = kEmptyLoggingCallbackInfo;
    }

    GetInstance()->RemoveDevice(this);

    // Once last external ref dropped, all callbacks should be forwarded to Instance's callback
    // queue instead.
    mCallbackTaskManager = GetInstance()->GetCallbackTaskManager();
}

void DeviceBase::DestroyObjects() {
    // List of object types in reverse "dependency" order so we can iterate and delete the
    // objects safely. We define dependent here such that if B has a ref to A, then B depends on
    // A. We therefore try to destroy B before destroying A. Note that this only considers the
    // immediate frontend dependencies, while backend objects could add complications and extra
    // dependencies.
    //
    // Note that AttachmentState is not an ApiObject so it cannot be eagerly destroyed. However,
    // since AttachmentStates are cached by the device, objects that hold references to
    // AttachmentStates should make sure to un-ref them in their Destroy operation so that we
    // can destroy the frontend cache.

    // clang-format off
    static constexpr std::array<ObjectType, 21> kObjectTypeDependencyOrder = {
        ObjectType::ComputePassEncoder,
        ObjectType::RenderPassEncoder,
        ObjectType::RenderBundleEncoder,
        ObjectType::RenderBundle,
        ObjectType::CommandEncoder,
        ObjectType::CommandBuffer,
        ObjectType::RenderPipeline,
        ObjectType::ComputePipeline,
        ObjectType::PipelineLayout,
        ObjectType::BindGroup,
        ObjectType::BindGroupLayout,
        ObjectType::BindGroupLayoutInternal,
        ObjectType::ShaderModule,
        ObjectType::SharedBufferMemory,
        ObjectType::SharedTextureMemory,
        ObjectType::SharedFence,
        ObjectType::ExternalTexture,
        ObjectType::Texture,  // Note that Textures own the TextureViews.
        ObjectType::QuerySet,
        ObjectType::Sampler,
        ObjectType::Buffer,
    };
    // clang-format on

    for (ObjectType type : kObjectTypeDependencyOrder) {
        mObjectLists[type].Destroy();
    }
}

void DeviceBase::Destroy() {
    // Skip if we are already destroyed.
    if (mState == State::Destroyed) {
        return;
    }

    // This function may be called re-entrantly inside APITick(). Tick triggers callbacks
    // inside which the application may destroy the device. Thus, we should be careful not
    // to delete objects that are needed inside Tick after callbacks have been called.
    //  - mCallbackTaskManager is not deleted since we flush the callback queue at the end
    // of Tick(). Note: that flush should always be empty since all callbacks are drained
    // inside Destroy() so there should be no outstanding tasks holding objects alive.
    //  - Similarly, mAsyncTaskManager is not deleted since we use it to return a status
    // from Tick() whether or not there is any more pending work.

    // Skip handling device facilities if they haven't even been created (or failed doing so)
    if (mState != State::BeingCreated) {
        // The device is being destroyed so it will be lost, call the application callback.
        HandleDeviceLost(wgpu::DeviceLostReason::Destroyed, "Device was destroyed.");

        // Call all the callbacks immediately as the device is about to shut down.
        // TODO(crbug.com/dawn/826): Cancel the tasks that are in flight if possible.
        mAsyncTaskManager->WaitAllPendingTasks();
        mCallbackTaskManager->HandleShutDown();

        // Finish destroying all objects owned by the device. Note that this must be done before
        // DestroyImpl() as it may relinquish resources that will be freed by backends in the
        // DestroyImpl() call.
        DestroyObjects();
    }

    // Disconnect the device, depending on which state we are currently in.
    switch (mState) {
        case State::BeingCreated:
            // The GPU timeline was never started so we don't have to wait.
            break;

        case State::Alive:
            // Alive is the only state which can have GPU work happening. Wait for all of it to
            // complete before proceeding with destruction.
            // Ignore errors so that we can continue with destruction
            IgnoreErrors(mQueue->WaitForIdleForDestruction());
            break;

        case State::BeingDisconnected:
            // Getting disconnected is a transient state happening in a single API call so there
            // is always an external reference keeping the Device alive, which means the
            // destructor cannot run while BeingDisconnected.
            DAWN_UNREACHABLE();
            break;

        case State::Disconnected:
            break;

        case State::Destroyed:
            // If we are already destroyed we should've skipped this work entirely.
            DAWN_UNREACHABLE();
            break;
    }

    if (mState != State::BeingCreated) {
        // The GPU timeline is finished.
        mQueue->AssumeCommandsComplete();
        DAWN_ASSERT(mQueue->GetCompletedCommandSerial() == mQueue->GetLastSubmittedCommandSerial());
        mQueue->Tick(mQueue->GetCompletedCommandSerial());

        // Call TickImpl once last time to clean up resources
        // Ignore errors so that we can continue with destruction
        IgnoreErrors(TickImpl());
    }

    // At this point GPU operations are always finished, so we are in the disconnected state.
    // Note that currently this state change is required because some of the backend
    // implementations of DestroyImpl checks that we are disconnected before doing work.
    mState = State::Disconnected;

    mDynamicUploader = nullptr;
    mEmptyBindGroupLayout = nullptr;
    mEmptyPipelineLayout = nullptr;
    mInternalPipelineStore = nullptr;
    mExternalTexturePlaceholderView = nullptr;
    mTemporaryUniformBuffer = nullptr;

    // Note: mQueue is not released here since the application may still get it after calling
    // Destroy() via APIGetQueue.
    if (mQueue != nullptr) {
        mQueue->AssumeCommandsComplete();
        mQueue->Destroy();
    }

    // Now that the GPU timeline is empty, destroy the backend device.
    DestroyImpl();

    mCaches = nullptr;
    mState = State::Destroyed;
}

void DeviceBase::APIDestroy() {
    Destroy();
}

void DeviceBase::HandleDeviceLost(wgpu::DeviceLostReason reason, std::string_view message) {
    if (mLostEvent != nullptr) {
        mLostEvent->SetLost(GetInstance()->GetEventManager(), reason, message);
    }
}

void DeviceBase::HandleError(std::unique_ptr<ErrorData> error,
                             InternalErrorType additionalAllowedErrors,
                             wgpu::DeviceLostReason lostReason) {
    auto deviceGuard(GetGuard());
    AppendDebugLayerMessages(error.get());

    InternalErrorType type = error->GetType();
    if (type != InternalErrorType::Validation) {
        // D3D device can provide additional device removed reason. We would
        // like to query and log the device removed reason if the error is
        // not validation error.
        AppendDeviceLostMessage(error.get());
    }

    InternalErrorType allowedErrors =
        InternalErrorType::Validation | InternalErrorType::DeviceLost | additionalAllowedErrors;

    if (type == InternalErrorType::DeviceLost) {
        mState = State::Disconnected;

        // If the ErrorInjector is enabled, then the device loss might be fake and the device
        // still be executing commands. Force a wait for idle in this case, with State being
        // Disconnected so we can detect this case in WaitForIdleForDestruction.
        if (ErrorInjectorEnabled()) {
            IgnoreErrors(mQueue->WaitForIdleForDestruction());
        }

        // A real device lost happened. Set the state to disconnected as the device cannot be
        // used. Also tags all commands as completed since the device stopped running.
        mQueue->AssumeCommandsComplete();
    } else if (!(allowedErrors & type)) {
        // If we receive an error which we did not explicitly allow, assume the backend can't
        // recover and proceed with device destruction. We first wait for all previous commands to
        // be completed so that backend objects can be freed immediately, before handling the loss.
        error->AppendContext("handling unexpected error type %s when allowed errors are %s.", type,
                             allowedErrors);

        // Move away from the Alive state so that the application cannot use this device
        // anymore.
        // TODO(crbug.com/dawn/831): Do we need atomics for this to become visible to other
        // threads in a multithreaded scenario?
        mState = State::BeingDisconnected;

        // Ignore errors so that we can continue with destruction
        // Assume all commands are complete after WaitForIdleForDestruction (because they were)
        IgnoreErrors(mQueue->WaitForIdleForDestruction());
        IgnoreErrors(TickImpl());
        mQueue->AssumeCommandsComplete();
        mState = State::Disconnected;

        // Now everything is as if the device was lost.
        type = InternalErrorType::DeviceLost;
    }

    const std::string messageStr = error->GetFormattedMessage();
    if (type == InternalErrorType::DeviceLost) {
        // The device was lost, schedule the application callback's execution.
        // Note: we don't invoke the callbacks directly here because it could cause re-entrances ->
        // possible deadlock.
        HandleDeviceLost(lostReason, messageStr);
        mQueue->HandleDeviceLoss();

        // TODO(crbug.com/dawn/826): Cancel the tasks that are in flight if possible.
        mAsyncTaskManager->WaitAllPendingTasks();
        mCallbackTaskManager->HandleDeviceLoss();

        // Still forward device loss errors to the error scopes so they all reject.
        GetErrorScopeStack()->HandleError(ToWGPUErrorType(type), messageStr);
    } else {
        // Pass the error to the error scope stack and call the uncaptured error callback
        // if it isn't handled. DeviceLost is not handled here because it should be
        // handled by the lost callback.
        bool captured = GetErrorScopeStack()->HandleError(ToWGPUErrorType(type), messageStr);
        if (!captured) {
            // Only call the uncaptured error callback if the device is alive. After the
            // device is lost, the uncaptured error callback should cease firing.
            if (mUncapturedErrorCallbackInfo.callback != nullptr && mState == State::Alive) {
                auto device = ToAPI(this);
                mUncapturedErrorCallbackInfo.callback(
                    &device, ToAPI(ToWGPUErrorType(type)), ToOutputStringView(messageStr),
                    mUncapturedErrorCallbackInfo.userdata1, mUncapturedErrorCallbackInfo.userdata2);
            }
        }
    }
}

void DeviceBase::ConsumeError(std::unique_ptr<ErrorData> error,
                              InternalErrorType additionalAllowedErrors) {
    DAWN_ASSERT(error != nullptr);
    HandleError(std::move(error), additionalAllowedErrors);
}

void DeviceBase::APISetLoggingCallback(const WGPULoggingCallbackInfo& callbackInfo) {
    if (mState != State::Alive) {
        return;
    }
    std::lock_guard<std::shared_mutex> lock(mLoggingMutex);
    mLoggingCallbackInfo = callbackInfo;
}

ErrorScopeStack* DeviceBase::GetErrorScopeStack() {
    ThreadUniqueId threadId = GetThreadUniqueId();
    if (!mErrorScopeStacks.contains(threadId)) {
        // Each time a new thread creates an error stack on a device, we attempt to clean up
        // terminated thread stacks before adding the new one.
        TrimErrorScopeStacks(mErrorScopeStacks);
        mErrorScopeStacks[threadId] = std::make_unique<ErrorScopeStack>();
    }
    DAWN_ASSERT(mErrorScopeStacks[threadId] != nullptr);
    return mErrorScopeStacks[threadId].get();
}

void DeviceBase::APIPushErrorScope(wgpu::ErrorFilter filter) {
    if (ConsumedError(ValidateErrorFilter(filter))) {
        return;
    }

    GetErrorScopeStack()->Push(filter);
}

Future DeviceBase::APIPopErrorScope(const WGPUPopErrorScopeCallbackInfo& callbackInfo) {
    struct PopErrorScopeEvent final : public EventManager::TrackedEvent {
        WGPUPopErrorScopeCallback mCallback;
        raw_ptr<void> mUserdata1;
        raw_ptr<void> mUserdata2;
        std::optional<ErrorScope> mScope;

        PopErrorScopeEvent(const WGPUPopErrorScopeCallbackInfo& callbackInfo,
                           std::optional<ErrorScope>&& scope)
            : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode),
                           TrackedEvent::Completed{}),
              mCallback(callbackInfo.callback),
              mUserdata1(callbackInfo.userdata1),
              mUserdata2(callbackInfo.userdata2),
              mScope(std::move(scope)) {}

        ~PopErrorScopeEvent() override { EnsureComplete(EventCompletionType::Shutdown); }

        void Complete(EventCompletionType completionType) override {
            WGPUPopErrorScopeStatus status = completionType == EventCompletionType::Ready
                                                 ? WGPUPopErrorScopeStatus_Success
                                                 : WGPUPopErrorScopeStatus_CallbackCancelled;
            WGPUErrorType type;
            WGPUStringView message = kEmptyOutputStringView;
            if (mScope) {
                type = static_cast<WGPUErrorType>(mScope->GetErrorType());
                message = mScope->GetErrorMessage();
            } else {
                status = WGPUPopErrorScopeStatus_Error;
                type = WGPUErrorType_NoError;
                message = ToOutputStringView("No error scopes to pop");
            }

            mCallback(status, type, message, mUserdata1.ExtractAsDangling(),
                      mUserdata2.ExtractAsDangling());
        }
    };

    std::optional<ErrorScope> scope;
    {
        // TODO(crbug.com/dawn/831) Manually acquire device lock instead of relying on code-gen for
        // re-entrancy.
        auto deviceGuard = GetGuard();

        if (IsLost()) {
            scope = ErrorScope(wgpu::ErrorType::NoError, "");
        } else if (!GetErrorScopeStack()->Empty()) {
            scope = GetErrorScopeStack()->Pop();
        }
    }

    FutureID futureID = GetInstance()->GetEventManager()->TrackEvent(
        AcquireRef(new PopErrorScopeEvent(callbackInfo, std::move(scope))));
    return {futureID};
}

BlobCache* DeviceBase::GetBlobCache() const {
    return mBlobCache.get();
}

Blob DeviceBase::LoadCachedBlob(const CacheKey& key) {
    auto loadResult = GetBlobCache()->Load(key);
    if (loadResult.IsSuccess()) {
        return loadResult.AcquireSuccess();
    }
    // Treat hash validation error as cache miss.
    loadResult.AcquireError();
    DAWN_HISTOGRAM_BOOLEAN(GetPlatform(), "BlobCacheHashValidationFailed", true);
    return Blob();
}

void DeviceBase::StoreCachedBlob(const CacheKey& key, const Blob& blob) {
    if (!blob.Empty()) {
        GetBlobCache()->Store(key, blob);
    }
}

MaybeError DeviceBase::ValidateObject(const ApiObjectBase* object) const {
    DAWN_ASSERT(object != nullptr);
    DAWN_INVALID_IF(object->GetDevice() != this,
                    "%s is associated with %s, and cannot be used with %s.", object,
                    object->GetDevice(), this);

    // TODO(dawn:563): Preserve labels for error objects.
    DAWN_INVALID_IF(object->IsError(), "%s is invalid.", object);

    return {};
}

MaybeError DeviceBase::IsNotErrorObject(const ApiObjectBase* object) const {
    DAWN_ASSERT(!IsValidationEnabled());
    DAWN_ASSERT(object != nullptr);
    DAWN_INVALID_IF(object->IsError(), "%s is invalid.", object);

    return {};
}

MaybeError DeviceBase::ValidateIsAlive() const {
    DAWN_INVALID_IF(mState != State::Alive, "%s is lost.", this);
    return {};
}

void DeviceBase::APIForceLoss(wgpu::DeviceLostReason reason, StringView messageIn) {
    std::string_view message = utils::NormalizeMessageString(messageIn);
    if (mState != State::Alive) {
        return;
    }
    // Note that since we are passing None as the allowedErrors, an additional message will be
    // appended noting that the error was unexpected. Since this call is for testing only it is not
    // too important, but useful for users to understand where the extra message is coming from.
    HandleError(DAWN_INTERNAL_ERROR(std::string(message)), InternalErrorType::None, reason);
}

DeviceBase::State DeviceBase::GetState() const {
    return mState;
}

bool DeviceBase::IsLost() const {
    DAWN_ASSERT(mState != State::BeingCreated);
    return mState != State::Alive;
}

ApiObjectList* DeviceBase::GetObjectTrackingList(ObjectType type) {
    return &mObjectLists[type];
}

const ApiObjectList* DeviceBase::GetObjectTrackingList(ObjectType type) const {
    return &mObjectLists[type];
}

InstanceBase* DeviceBase::GetInstance() const {
    return mAdapter->GetInstance();
}

AdapterBase* DeviceBase::GetAdapter() const {
    return mAdapter.Get();
}

PhysicalDeviceBase* DeviceBase::GetPhysicalDevice() const {
    return mAdapter->GetPhysicalDevice();
}

dawn::platform::Platform* DeviceBase::GetPlatform() const {
    return GetAdapter()->GetInstance()->GetPlatform();
}

InternalPipelineStore* DeviceBase::GetInternalPipelineStore() {
    return mInternalPipelineStore.get();
}

bool DeviceBase::HasPendingTasks() {
    return mAsyncTaskManager->HasPendingTasks() || !mCallbackTaskManager->IsEmpty();
}

bool DeviceBase::IsDeviceIdle() {
    if (HasPendingTasks()) {
        return false;
    }
    return !mQueue->HasScheduledCommands();
}

ResultOrError<const Format*> DeviceBase::GetInternalFormat(wgpu::TextureFormat format) const {
    FormatIndex index = ComputeFormatIndex(format);
    DAWN_INVALID_IF(index >= mFormatTable.size(), "Unknown texture format %s.", format);

    const Format* internalFormat = &mFormatTable[index];
    DAWN_INVALID_IF(!internalFormat->IsSupported(), "Unsupported texture format %s, reason: %s.",
                    format, internalFormat->unsupportedReason);

    return internalFormat;
}

const Format& DeviceBase::GetValidInternalFormat(wgpu::TextureFormat format) const {
    FormatIndex index = ComputeFormatIndex(format);
    DAWN_ASSERT(index < mFormatTable.size());
    DAWN_ASSERT(mFormatTable[index].IsSupported());
    return mFormatTable[index];
}

const Format& DeviceBase::GetValidInternalFormat(FormatIndex index) const {
    DAWN_ASSERT(index < mFormatTable.size());
    DAWN_ASSERT(mFormatTable[index].IsSupported());
    return mFormatTable[index];
}

std::vector<const Format*> DeviceBase::GetCompatibleViewFormats(const Format& format) const {
    wgpu::TextureFormat viewFormat =
        format.format == format.baseFormat ? format.baseViewFormat : format.baseFormat;
    if (viewFormat == wgpu::TextureFormat::Undefined) {
        return {};
    }
    const Format& f = mFormatTable[ComputeFormatIndex(viewFormat)];
    if (!f.IsSupported()) {
        return {};
    }
    return {&f};
}

ResultOrError<Ref<BindGroupLayoutBase>> DeviceBase::GetOrCreateBindGroupLayout(
    const BindGroupLayoutDescriptor* descriptor,
    PipelineCompatibilityToken pipelineCompatibilityToken) {
    BindGroupLayoutInternalBase blueprint(this, descriptor, ApiObjectBase::kUntrackedByDevice);

    const size_t blueprintHash = blueprint.ComputeContentHash();
    blueprint.SetContentHash(blueprintHash);

    Ref<BindGroupLayoutInternalBase> internal;
    DAWN_TRY_ASSIGN(internal, GetOrCreate(mCaches->bindGroupLayouts, &blueprint,
                                          [&]() -> ResultOrError<Ref<BindGroupLayoutInternalBase>> {
                                              Ref<BindGroupLayoutInternalBase> result;
                                              DAWN_TRY_ASSIGN(
                                                  result, CreateBindGroupLayoutImpl(descriptor));
                                              result->SetContentHash(blueprintHash);
                                              return result;
                                          }));
    return AcquireRef(
        new BindGroupLayoutBase(this, descriptor->label, internal, pipelineCompatibilityToken));
}

// Private function used at initialization
ResultOrError<Ref<BindGroupLayoutBase>> DeviceBase::CreateEmptyBindGroupLayout() {
    BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 0;
    desc.entries = nullptr;

    return GetOrCreateBindGroupLayout(&desc);
}

ResultOrError<Ref<PipelineLayoutBase>> DeviceBase::CreateEmptyPipelineLayout() {
    PipelineLayoutDescriptor desc = {};
    desc.bindGroupLayoutCount = 0;
    desc.bindGroupLayouts = nullptr;

    return GetOrCreatePipelineLayout(Unpack(&desc));
}

BindGroupLayoutBase* DeviceBase::GetEmptyBindGroupLayout() {
    DAWN_ASSERT(mEmptyBindGroupLayout != nullptr);
    return mEmptyBindGroupLayout.Get();
}

PipelineLayoutBase* DeviceBase::GetEmptyPipelineLayout() {
    DAWN_ASSERT(mEmptyPipelineLayout != nullptr);
    return mEmptyPipelineLayout.Get();
}

Ref<ComputePipelineBase> DeviceBase::GetCachedComputePipeline(
    ComputePipelineBase* uninitializedComputePipeline) {
    return mCaches->computePipelines.Find(uninitializedComputePipeline);
}

Ref<RenderPipelineBase> DeviceBase::GetCachedRenderPipeline(
    RenderPipelineBase* uninitializedRenderPipeline) {
    return mCaches->renderPipelines.Find(uninitializedRenderPipeline);
}

Ref<ComputePipelineBase> DeviceBase::AddOrGetCachedComputePipeline(
    Ref<ComputePipelineBase> computePipeline) {
    auto [pipeline, _] = mCaches->computePipelines.Insert(computePipeline.Get());
    return std::move(pipeline);
}

Ref<RenderPipelineBase> DeviceBase::AddOrGetCachedRenderPipeline(
    Ref<RenderPipelineBase> renderPipeline) {
    auto [pipeline, _] = mCaches->renderPipelines.Insert(renderPipeline.Get());
    return std::move(pipeline);
}

ResultOrError<Ref<TextureViewBase>>
DeviceBase::GetOrCreatePlaceholderTextureViewForExternalTexture() {
    if (!mExternalTexturePlaceholderView.Get()) {
        Ref<TextureBase> externalTexturePlaceholder;
        TextureDescriptor textureDesc;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        textureDesc.label = "Dawn_External_Texture_Placeholder_Texture";
        textureDesc.size = {1, 1, 1};
        textureDesc.usage = wgpu::TextureUsage::TextureBinding;

        DAWN_TRY_ASSIGN(externalTexturePlaceholder, CreateTexture(&textureDesc));

        TextureViewDescriptor textureViewDesc;
        textureViewDesc.arrayLayerCount = 1;
        textureViewDesc.aspect = wgpu::TextureAspect::All;
        textureViewDesc.baseArrayLayer = 0;
        textureViewDesc.dimension = wgpu::TextureViewDimension::e2D;
        textureViewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        textureViewDesc.label = "Dawn_External_Texture_Placeholder_Texture_View";
        textureViewDesc.mipLevelCount = 1;

        DAWN_TRY_ASSIGN(mExternalTexturePlaceholderView,
                        CreateTextureView(externalTexturePlaceholder.Get(), &textureViewDesc));
    }

    return mExternalTexturePlaceholderView;
}

ResultOrError<Ref<PipelineLayoutBase>> DeviceBase::GetOrCreatePipelineLayout(
    const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) {
    PipelineLayoutBase blueprint(this, descriptor, ApiObjectBase::kUntrackedByDevice);

    const size_t blueprintHash = blueprint.ComputeContentHash();
    blueprint.SetContentHash(blueprintHash);

    return GetOrCreate(mCaches->pipelineLayouts, &blueprint,
                       [&]() -> ResultOrError<Ref<PipelineLayoutBase>> {
                           Ref<PipelineLayoutBase> result;
                           DAWN_TRY_ASSIGN(result, CreatePipelineLayoutImpl(descriptor));
                           result->SetContentHash(blueprintHash);
                           return result;
                       });
}

ResultOrError<Ref<SamplerBase>> DeviceBase::GetOrCreateSampler(
    const SamplerDescriptor* descriptor) {
    SamplerBase blueprint(this, descriptor, ApiObjectBase::kUntrackedByDevice);

    const size_t blueprintHash = blueprint.ComputeContentHash();
    blueprint.SetContentHash(blueprintHash);

    return GetOrCreate(mCaches->samplers, &blueprint, [&]() -> ResultOrError<Ref<SamplerBase>> {
        Ref<SamplerBase> result;
        DAWN_TRY_ASSIGN(result, CreateSamplerImpl(descriptor));
        result->SetContentHash(blueprintHash);
        return result;
    });
}

Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(AttachmentState* blueprint) {
    return GetOrCreate(mCaches->attachmentStates, blueprint, [&]() -> Ref<AttachmentState> {
        return AcquireRef(new AttachmentState(*blueprint));
    });
}

Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(
    const RenderBundleEncoderDescriptor* descriptor) {
    AttachmentState blueprint(descriptor);
    return GetOrCreateAttachmentState(&blueprint);
}

Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor,
    const PipelineLayoutBase* layout) {
    AttachmentState blueprint(descriptor, layout);
    return GetOrCreateAttachmentState(&blueprint);
}

Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(
    const UnpackedPtr<RenderPassDescriptor>& descriptor) {
    AttachmentState blueprint(descriptor);
    return GetOrCreateAttachmentState(&blueprint);
}

Ref<PipelineCacheBase> DeviceBase::GetOrCreatePipelineCache(const CacheKey& key) {
    return GetOrCreatePipelineCacheImpl(key);
}

// Object creation API methods

BindGroupBase* DeviceBase::APICreateBindGroup(const BindGroupDescriptor* descriptor) {
    Ref<BindGroupBase> result;
    if (ConsumedError(CreateBindGroup(descriptor), &result, "calling %s.CreateBindGroup(%s).", this,
                      descriptor)) {
        return ReturnToAPI(
            BindGroupBase::MakeError(this, descriptor ? descriptor->label : nullptr));
    }
    return ReturnToAPI(std::move(result));
}
BindGroupLayoutBase* DeviceBase::APICreateBindGroupLayout(
    const BindGroupLayoutDescriptor* descriptor) {
    Ref<BindGroupLayoutBase> result;
    if (ConsumedError(CreateBindGroupLayout(descriptor), &result,
                      "calling %s.CreateBindGroupLayout(%s).", this, descriptor)) {
        return ReturnToAPI(
            BindGroupLayoutBase::MakeError(this, descriptor ? descriptor->label : nullptr));
    }
    return ReturnToAPI(std::move(result));
}
BufferBase* DeviceBase::APICreateBuffer(const BufferDescriptor* rawDescriptor) {
    // 1. Validate the descriptor and call CreateBufferImpl.
    bool fakeOOMAtNativeMap = false;
    ResultOrError<Ref<BufferBase>> resultOrError = ([&]() -> ResultOrError<Ref<BufferBase>> {
        DAWN_TRY(ValidateIsAlive());
        UnpackedPtr<BufferDescriptor> descriptor;
        if (IsValidationEnabled()) {
            DAWN_TRY_ASSIGN(descriptor, ValidateBufferDescriptor(this, rawDescriptor));
        } else {
            descriptor = Unpack(rawDescriptor);
        }

        bool hasHostMapped = descriptor.Has<BufferHostMappedPointer>();
        bool fakeOOMAtDevice = false;
        if (auto* ext = descriptor.Get<DawnFakeBufferOOMForTesting>()) {
            fakeOOMAtNativeMap = ext->fakeOOMAtNativeMap;
            fakeOOMAtDevice = ext->fakeOOMAtDevice;
        }

        if (fakeOOMAtDevice) {
            return DAWN_OUT_OF_MEMORY_ERROR("DawnFakeBufferOOMForTesting fakeOOMAtDevice");
        }
        if (hasHostMapped) {
            // Creating a buffer from a host-mapped pointer doesn't require the lock.
            return CreateBufferImpl(descriptor);
        } else {
            auto deviceGuard = GetGuard();
            return CreateBufferImpl(descriptor);
        }
    })();

    // 2. Error handling.
    Ref<BufferBase> buffer;
    std::unique_ptr<ErrorData> deferredError;
    if (resultOrError.IsSuccess()) [[likely]] {
        buffer = resultOrError.AcquireSuccess();
    } else {
        // Make an error buffer, but don't consume the ErrorData yet until we've tried to map,
        // and determined whether to silence it - errors from mapping should always take
        // priority, because that matches the spec's client-server model where mappedAtCreation
        // takes place on the client before even sending the CreateBuffer command to the server.
        deferredError = resultOrError.AcquireError();
        buffer = BufferBase::MakeError(this, rawDescriptor);
    }

    // 3. Mapping at creation. The buffer may be either valid or ErrorBuffer.
    if (rawDescriptor->mappedAtCreation) {
        // MapAtCreation requires the device lock in case it allocates staging memory.
        auto deviceGuard = GetGuard();

        MaybeError mapResult =
            fakeOOMAtNativeMap
                ? DAWN_OUT_OF_MEMORY_ERROR("DawnFakeBufferOOMForTesting fakeOOMAtNativeMap")
                : buffer->MapAtCreation();
        if (mapResult.IsError()) {
            // If we can't map, do "implementation-defined logging" and return null.
            auto error = mapResult.AcquireError();
            EmitLog(wgpu::LoggingType::Error, error->GetFormattedMessage());
            // deferredError is silenced because we drop it here.
            return nullptr;
        }
    }

    // If there was a deferredError saved from earlier, surface it now.
    if (deferredError) {
        deferredError->AppendContext("calling %s.CreateBuffer(%s).", this, rawDescriptor);
        ConsumeError(std::move(deferredError), InternalErrorType::OutOfMemory);
    }
    return ReturnToAPI(std::move(buffer));
}

CommandEncoder* DeviceBase::APICreateCommandEncoder(const CommandEncoderDescriptor* descriptor) {
    Ref<CommandEncoder> result;
    if (ConsumedError(CreateCommandEncoder(descriptor), &result,
                      "calling %s.CreateCommandEncoder(%s).", this, descriptor)) {
        result = CommandEncoder::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return ReturnToAPI(std::move(result));
}
ComputePipelineBase* DeviceBase::APICreateComputePipeline(
    const ComputePipelineDescriptor* descriptor) {
    utils::TraceLabel label = utils::GetLabelForTrace(descriptor->label);
    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APICreateComputePipeline", "label",
                 label.label);

    auto resultOrError = CreateComputePipeline(descriptor);
    if (resultOrError.IsSuccess()) {
        return ReturnToAPI(resultOrError.AcquireSuccess());
    }

    Ref<ComputePipelineBase> result;
    if (ConsumedError(std::move(resultOrError), &result, InternalErrorType::Internal,
                      "calling %s.CreateComputePipeline(%s).", this, descriptor)) {
        result = ComputePipelineBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return ReturnToAPI(std::move(result));
}
Future DeviceBase::APICreateComputePipelineAsync(
    const ComputePipelineDescriptor* descriptor,
    const WGPUCreateComputePipelineAsyncCallbackInfo& callbackInfo) {
    utils::TraceLabel label = utils::GetLabelForTrace(descriptor->label);
    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APICreateComputePipelineAsync", "label",
                 label.label);

    EventManager* manager = GetInstance()->GetEventManager();

    auto GetFuture = [&](Ref<EventManager::TrackedEvent>&& event) {
        FutureID futureID = manager->TrackEvent(std::move(event));
        return Future{futureID};
    };

    if (IsLost()) {
        // Device lost error: create an async event that completes when created.
        return GetFuture(AcquireRef(new CreateComputePipelineAsyncEvent(
            this, callbackInfo, DAWN_DEVICE_LOST_ERROR("Device lost"), descriptor->label)));
    }

    auto resultOrError = CreateUninitializedComputePipeline(descriptor);
    if (resultOrError.IsError()) {
        // Validation error: create an async event that completes when created.
        return GetFuture(AcquireRef(new CreateComputePipelineAsyncEvent(
            this, callbackInfo, resultOrError.AcquireError(), descriptor->label)));
    }

    Ref<ComputePipelineBase> uninitializedComputePipeline = resultOrError.AcquireSuccess();
    Ref<ComputePipelineBase> cachedComputePipeline =
        GetCachedComputePipeline(uninitializedComputePipeline.Get());
    if (cachedComputePipeline.Get() != nullptr) {
        // Cached pipeline: create an async event that completes when created.
        return GetFuture(AcquireRef(new CreateComputePipelineAsyncEvent(
            this, callbackInfo, std::move(cachedComputePipeline))));
    }

    // New pipeline: create an event backed by system event that is really async.
    Ref<CreateComputePipelineAsyncEvent> event = AcquireRef(new CreateComputePipelineAsyncEvent(
        this, callbackInfo, std::move(uninitializedComputePipeline),
        AcquireRef(new WaitListEvent())));
    Future future = GetFuture(event);
    InitializeComputePipelineAsyncImpl(std::move(event));
    return future;
}
PipelineLayoutBase* DeviceBase::APICreatePipelineLayout(
    const PipelineLayoutDescriptor* descriptor) {
    Ref<PipelineLayoutBase> result;
    if (ConsumedError(CreatePipelineLayout(descriptor), &result,
                      "calling %s.CreatePipelineLayout(%s).", this, descriptor)) {
        result = PipelineLayoutBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return ReturnToAPI(std::move(result));
}
QuerySetBase* DeviceBase::APICreateQuerySet(const QuerySetDescriptor* descriptor) {
    Ref<QuerySetBase> result;
    if (ConsumedError(CreateQuerySet(descriptor), &result, InternalErrorType::OutOfMemory,
                      "calling %s.CreateQuerySet(%s).", this, descriptor)) {
        result = QuerySetBase::MakeError(this, descriptor);
    }
    return ReturnToAPI(std::move(result));
}
SamplerBase* DeviceBase::APICreateSampler(const SamplerDescriptor* descriptor) {
    Ref<SamplerBase> result;
    if (ConsumedError(CreateSampler(descriptor), &result, "calling %s.CreateSampler(%s).", this,
                      descriptor)) {
        result = SamplerBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return ReturnToAPI(std::move(result));
}
Future DeviceBase::APICreateRenderPipelineAsync(
    const RenderPipelineDescriptor* descriptor,
    const WGPUCreateRenderPipelineAsyncCallbackInfo& callbackInfo) {
    utils::TraceLabel label = utils::GetLabelForTrace(descriptor->label);
    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APICreateRenderPipelineAsync", "label",
                 label.label);

    EventManager* manager = GetInstance()->GetEventManager();

    auto GetFuture = [&](Ref<EventManager::TrackedEvent>&& event) {
        FutureID futureID = manager->TrackEvent(std::move(event));
        return Future{futureID};
    };

    if (IsLost()) {
        // Device lost error: create an async event that completes when created.
        return GetFuture(AcquireRef(new CreateRenderPipelineAsyncEvent(
            this, callbackInfo, DAWN_DEVICE_LOST_ERROR("Device lost"), descriptor->label)));
    }

    auto resultOrError = CreateUninitializedRenderPipeline(descriptor);
    if (resultOrError.IsError()) {
        // Validation error: create an async event that completes when created.
        return GetFuture(AcquireRef(new CreateRenderPipelineAsyncEvent(
            this, callbackInfo, resultOrError.AcquireError(), descriptor->label)));
    }

    Ref<RenderPipelineBase> uninitializedRenderPipeline = resultOrError.AcquireSuccess();
    Ref<RenderPipelineBase> cachedRenderPipeline =
        GetCachedRenderPipeline(uninitializedRenderPipeline.Get());
    if (cachedRenderPipeline.Get() != nullptr) {
        // Cached pipeline: create an async event that completes when created.
        return GetFuture(AcquireRef(new CreateRenderPipelineAsyncEvent(
            this, callbackInfo, std::move(cachedRenderPipeline))));
    }

    // New pipeline: create an event backed by system event that is really async.
    Ref<CreateRenderPipelineAsyncEvent> event = AcquireRef(new CreateRenderPipelineAsyncEvent(
        this, callbackInfo, std::move(uninitializedRenderPipeline),
        AcquireRef(new WaitListEvent())));
    Future future = GetFuture(event);
    InitializeRenderPipelineAsyncImpl(std::move(event));
    return future;
}
RenderBundleEncoder* DeviceBase::APICreateRenderBundleEncoder(
    const RenderBundleEncoderDescriptor* descriptor) {
    Ref<RenderBundleEncoder> result;
    if (ConsumedError(CreateRenderBundleEncoder(descriptor), &result,
                      "calling %s.CreateRenderBundleEncoder(%s).", this, descriptor)) {
        result = RenderBundleEncoder::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return ReturnToAPI(std::move(result));
}
RenderPipelineBase* DeviceBase::APICreateRenderPipeline(
    const RenderPipelineDescriptor* descriptor) {
    utils::TraceLabel label = utils::GetLabelForTrace(descriptor->label);
    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APICreateRenderPipeline", "label",
                 label.label);

    auto resultOrError = CreateRenderPipeline(descriptor);
    if (resultOrError.IsSuccess()) {
        return ReturnToAPI(resultOrError.AcquireSuccess());
    }

    Ref<RenderPipelineBase> result;
    if (ConsumedError(std::move(resultOrError), &result, InternalErrorType::Internal,
                      "calling %s.CreateRenderPipeline(%s).", this, descriptor)) {
        result = RenderPipelineBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return ReturnToAPI(std::move(result));
}
ShaderModuleBase* DeviceBase::APICreateShaderModule(const ShaderModuleDescriptor* descriptor) {
    utils::TraceLabel label = utils::GetLabelForTrace(descriptor->label);
    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APICreateShaderModule", "label", label.label);

    // parseResult is modified by CreateShaderModule via pointer to provide compilation messages in
    // error cases.
    ShaderModuleParseResult parseResult;
    auto creationResult = CreateShaderModule(descriptor, /*internalExtensions=*/{}, &parseResult);

    if (creationResult.IsSuccess()) {
        Ref<ShaderModuleBase> validShaderModule = creationResult.AcquireSuccess();
        DAWN_ASSERT(validShaderModule != nullptr && !validShaderModule->IsError());
        EmitCompilationLog(validShaderModule.Get());
        return ReturnToAPI(std::move(validShaderModule));
    }

    // If shader creation failed, create an error shader module with compilation messages so the
    // application can later retrieve it with GetCompilationInfo.
    Ref<ShaderModuleBase> errorShaderModule = ShaderModuleBase::MakeError(
        this, descriptor ? descriptor->label : nullptr, std::move(parseResult.compilationMessages));
    DAWN_ASSERT(errorShaderModule != nullptr && errorShaderModule->IsError());

    // Acquire the device lock for error handling, and return the error shader module.
    auto deviceGuard = GetGuard();
    // Emit error, including Tint errors and warnings for the error shader module.
    auto consumedError = ConsumedError(creationResult.AcquireError(), InternalErrorType::Internal,
                                       "calling %s.CreateShaderModule(%s).", this, descriptor);
    DAWN_ASSERT(consumedError);
    return ReturnToAPI(std::move(errorShaderModule));
}

ShaderModuleBase* DeviceBase::APICreateErrorShaderModule(const ShaderModuleDescriptor* descriptor,
                                                         StringView errorMessage) {
    ParsedCompilationMessages compilationMessages;
    compilationMessages.AddUnanchoredMessage(errorMessage, wgpu::CompilationMessageType::Error);
    Ref<ShaderModuleBase> result = ShaderModuleBase::MakeError(
        this, descriptor ? descriptor->label : nullptr, std::move(compilationMessages));
    auto log = result->GetCompilationLog();

    std::unique_ptr<ErrorData> errorData = DAWN_VALIDATION_ERROR(
        "Error in calling %s.CreateShaderModule(%s).\n%s", this, descriptor, log);
    ConsumeError(std::move(errorData));

    return ReturnToAPI(std::move(result));
}
TextureBase* DeviceBase::APICreateTexture(const TextureDescriptor* descriptor) {
    Ref<TextureBase> result;
    if (ConsumedError(CreateTexture(descriptor), &result, InternalErrorType::OutOfMemory,
                      "calling %s.CreateTexture(%s).", this, descriptor)) {
        result = TextureBase::MakeError(this, descriptor);
    }
    return ReturnToAPI(std::move(result));
}

// For Dawn Wire

BufferBase* DeviceBase::APICreateErrorBuffer(const BufferDescriptor* desc) {
    if (desc->mappedAtCreation) {
        // This codepath isn't used (at the time of this writing). Just return nullptr
        // (pretend there was a mapping OOM), so we don't have to bother mapping the ErrorBuffer
        // (would have to return nullptr anyway if there was actually an OOM).
        auto error =
            DAWN_OUT_OF_MEMORY_ERROR("mappedAtCreation is not implemented for CreateErrorBuffer");
        error->AppendContext("calling %s.CreateBuffer(%s).", this, desc);
        EmitLog(wgpu::LoggingType::Error, error->GetFormattedMessage());
        return nullptr;
    }

    UnpackedPtr<BufferDescriptor> unpacked;
    if (!ConsumedError(ValidateBufferDescriptor(this, desc), &unpacked,
                       InternalErrorType::OutOfMemory, "calling %s.CreateBuffer(%s).", this,
                       desc)) {
        auto* clientErrorInfo = unpacked.Get<DawnBufferDescriptorErrorInfoFromWireClient>();
        if (clientErrorInfo != nullptr && clientErrorInfo->outOfMemory) {
            HandleError(DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate memory for buffer mapping"),
                        InternalErrorType::OutOfMemory);
        }
    }

    return ReturnToAPI(BufferBase::MakeError(this, desc));
}

ExternalTextureBase* DeviceBase::APICreateErrorExternalTexture() {
    return ReturnToAPI(ExternalTextureBase::MakeError(this));
}

TextureBase* DeviceBase::APICreateErrorTexture(const TextureDescriptor* desc) {
    return ReturnToAPI(TextureBase::MakeError(this, desc));
}

// Other Device API methods

// Returns true if future ticking is needed.
bool DeviceBase::APITick() {
    // TODO(dawn:1987) Add deprecation warning when Instance.ProcessEvents no longer calls this.

    // Tick may trigger callbacks which drop a ref to the device itself. Hold a Ref to ourselves
    // to avoid deleting |this| in the middle of this function call.
    Ref<DeviceBase> self(this);
    bool tickError;
    {
        // Note: we cannot hold the lock when flushing the callbacks so have to limit the scope of
        // the lock here.
        auto deviceGuard = GetGuard();
        tickError = ConsumedError(Tick());
    }

    // We have to check callback tasks in every APITick because it is not related to any global
    // serials.
    FlushCallbackTaskQueue();

    if (tickError) {
        return false;
    }

    auto deviceGuard = GetGuard();
    // We don't throw an error when device is lost. This allows pending callbacks to be
    // executed even after the Device is lost/destroyed.
    if (IsLost()) {
        return HasPendingTasks();
    }

    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APITick::IsDeviceIdle", "isDeviceIdle",
                 IsDeviceIdle());

    return !IsDeviceIdle();
}

MaybeError DeviceBase::Tick() {
    if (IsLost() || !mQueue->HasScheduledCommands()) {
        return {};
    }

    // To avoid overly ticking, we only want to tick when:
    // 1. the last submitted serial has moved beyond the completed serial
    // 2. or the backend still has pending commands to submit.
    DAWN_TRY(mQueue->CheckPassedSerials());
    DAWN_TRY(TickImpl());

    // TODO(crbug.com/dawn/833): decouple TickImpl from updating the serial so that we can
    // tick the dynamic uploader before the backend resource allocators. This would allow
    // reclaiming resources one tick earlier.
    mDynamicUploader->Deallocate(mQueue->GetCompletedCommandSerial());
    mQueue->Tick(mQueue->GetCompletedCommandSerial());

    return {};
}

AdapterBase* DeviceBase::APIGetAdapter() {
    mAdapter->APIAddRef();
    return mAdapter.Get();
}

QueueBase* DeviceBase::APIGetQueue() {
    // Backends gave the primary queue during initialization.
    DAWN_ASSERT(mQueue != nullptr);
    auto queue = mQueue;
    return ReturnToAPI(std::move(queue));
}

ExternalTextureBase* DeviceBase::APICreateExternalTexture(
    const ExternalTextureDescriptor* descriptor) {
    Ref<ExternalTextureBase> result;
    if (ConsumedError(CreateExternalTextureImpl(descriptor), &result,
                      "calling %s.CreateExternalTexture(%s).", this, descriptor)) {
        result = ExternalTextureBase::MakeError(this);
    }

    return ReturnToAPI(std::move(result));
}

SharedBufferMemoryBase* DeviceBase::APIImportSharedBufferMemory(
    const SharedBufferMemoryDescriptor* descriptor) {
    Ref<SharedBufferMemoryBase> result = nullptr;
    if (ConsumedError(
            [&]() -> ResultOrError<Ref<SharedBufferMemoryBase>> {
                DAWN_TRY(ValidateIsAlive());
                return ImportSharedBufferMemoryImpl(descriptor);
            }(),
            &result, "calling %s.ImportSharedBufferMemory(%s).", this, descriptor)) {
        return SharedBufferMemoryBase::MakeError(this, descriptor);
    }
    return result.Detach();
}

ResultOrError<Ref<SharedBufferMemoryBase>> DeviceBase::ImportSharedBufferMemoryImpl(
    const SharedBufferMemoryDescriptor* descriptor) {
    return DAWN_UNIMPLEMENTED_ERROR("Not implemented");
}

SharedTextureMemoryBase* DeviceBase::APIImportSharedTextureMemory(
    const SharedTextureMemoryDescriptor* descriptor) {
    Ref<SharedTextureMemoryBase> result;
    if (ConsumedError(
            [&]() -> ResultOrError<Ref<SharedTextureMemoryBase>> {
                DAWN_TRY(ValidateIsAlive());
                return ImportSharedTextureMemoryImpl(descriptor);
            }(),
            &result, "calling %s.ImportSharedTextureMemory(%s).", this, descriptor)) {
        result = SharedTextureMemoryBase::MakeError(this, descriptor);
    }
    return ReturnToAPI(std::move(result));
}

ResultOrError<Ref<SharedTextureMemoryBase>> DeviceBase::ImportSharedTextureMemoryImpl(
    const SharedTextureMemoryDescriptor* descriptor) {
    return DAWN_UNIMPLEMENTED_ERROR("Not implemented");
}

SharedFenceBase* DeviceBase::APIImportSharedFence(const SharedFenceDescriptor* descriptor) {
    Ref<SharedFenceBase> result;
    if (ConsumedError(
            [&]() -> ResultOrError<Ref<SharedFenceBase>> {
                DAWN_TRY(ValidateIsAlive());
                return ImportSharedFenceImpl(descriptor);
            }(),
            &result, "calling %s.ImportSharedFence(%s).", this, descriptor)) {
        result = SharedFenceBase::MakeError(this, descriptor);
    }
    return ReturnToAPI(std::move(result));
}

ResultOrError<Ref<SharedFenceBase>> DeviceBase::ImportSharedFenceImpl(
    const SharedFenceDescriptor* descriptor) {
    return DAWN_UNIMPLEMENTED_ERROR("Not implemented");
}

void DeviceBase::ApplyFeatures(const UnpackedPtr<DeviceDescriptor>& deviceDescriptor,
                               wgpu::FeatureLevel level) {
    DAWN_ASSERT(deviceDescriptor);
    // Validate all required features with device toggles.
    DAWN_ASSERT(GetPhysicalDevice()->SupportsAllRequiredFeatures(
        {deviceDescriptor->requiredFeatures, deviceDescriptor->requiredFeatureCount}, mToggles));

    for (uint32_t i = 0; i < deviceDescriptor->requiredFeatureCount; ++i) {
        mEnabledFeatures.EnableFeature(deviceDescriptor->requiredFeatures[i]);
    }

    // Handle features that implicitly enable other features.
    if (mEnabledFeatures.IsEnabled(Feature::TextureFormatsTier2)) {
        mEnabledFeatures.EnableFeature(Feature::TextureFormatsTier1);
    }
    if (mEnabledFeatures.IsEnabled(Feature::TextureFormatsTier1)) {
        mEnabledFeatures.EnableFeature(Feature::RG11B10UfloatRenderable);
    }

    if (level == wgpu::FeatureLevel::Core) {
        // Core-defaulting adapters always support the "core-features-and-limits" feature.
        // It is automatically enabled on devices created from such adapters.
        mEnabledFeatures.EnableFeature(wgpu::FeatureName::CoreFeaturesAndLimits);
    }

    // TODO(384921944): Enable Compat's optional features by default in Core Mode.
}

bool DeviceBase::HasFeature(Feature feature) const {
    return mEnabledFeatures.IsEnabled(feature);
}

void DeviceBase::SetWGSLExtensionAllowList() {
    // Set the WGSL extensions and language features allow list based on device's enabled features
    // and other properties.
    if (mEnabledFeatures.IsEnabled(Feature::ShaderF16)) {
        mWGSLAllowedFeatures.extensions.insert(tint::wgsl::Extension::kF16);
    }
    if (mEnabledFeatures.IsEnabled(Feature::Subgroups)) {
        mWGSLAllowedFeatures.extensions.insert(tint::wgsl::Extension::kSubgroups);
    }
    if (IsToggleEnabled(Toggle::AllowUnsafeAPIs)) {
        mWGSLAllowedFeatures.extensions.insert(
            tint::wgsl::Extension::kChromiumDisableUniformityAnalysis);
        mWGSLAllowedFeatures.extensions.insert(tint::wgsl::Extension::kChromiumInternalGraphite);
        mWGSLAllowedFeatures.extensions.insert(
            tint::wgsl::Extension::kChromiumExperimentalImmediate);
    }
    if (mEnabledFeatures.IsEnabled(Feature::DualSourceBlending)) {
        mWGSLAllowedFeatures.extensions.insert(tint::wgsl::Extension::kDualSourceBlending);
    }
    if (mEnabledFeatures.IsEnabled(Feature::PixelLocalStorageNonCoherent) ||
        mEnabledFeatures.IsEnabled(Feature::PixelLocalStorageCoherent)) {
        mWGSLAllowedFeatures.extensions.insert(
            tint::wgsl::Extension::kChromiumExperimentalPixelLocal);
    }
    if (mEnabledFeatures.IsEnabled(Feature::FramebufferFetch)) {
        mWGSLAllowedFeatures.extensions.insert(
            tint::wgsl::Extension::kChromiumExperimentalFramebufferFetch);
    }
    if (mEnabledFeatures.IsEnabled(Feature::ClipDistances)) {
        mWGSLAllowedFeatures.extensions.insert(tint::wgsl::Extension::kClipDistances);
    }
    if (mEnabledFeatures.IsEnabled(Feature::ChromiumExperimentalSubgroupMatrix)) {
        mWGSLAllowedFeatures.extensions.insert(
            tint::wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    }
    if (mEnabledFeatures.IsEnabled(Feature::ChromiumExperimentalPrimitiveId)) {
        mWGSLAllowedFeatures.extensions.insert(
            tint::wgsl::Extension::kChromiumExperimentalPrimitiveId);
    }

    // Language features are enabled instance-wide.
    const auto& allowedFeatures = GetInstance()->GetAllowedWGSLLanguageFeatures();
    mWGSLAllowedFeatures.features = {allowedFeatures.begin(), allowedFeatures.end()};
}

const tint::wgsl::AllowedFeatures& DeviceBase::GetWGSLAllowedFeatures() const {
    return mWGSLAllowedFeatures;
}

bool DeviceBase::IsValidationEnabled() const {
    return !IsToggleEnabled(Toggle::SkipValidation);
}

bool DeviceBase::IsRobustnessEnabled() const {
    return !IsToggleEnabled(Toggle::DisableRobustness);
}

bool DeviceBase::IsCompatibilityMode() const {
    return !HasFeature(Feature::CoreFeaturesAndLimits);
}

bool DeviceBase::IsImmediateErrorHandlingEnabled() const {
    return mIsImmediateErrorHandlingEnabled;
}

size_t DeviceBase::GetLazyClearCountForTesting() {
    return mLazyClearCountForTesting;
}

void DeviceBase::IncrementLazyClearCountForTesting() {
    ++mLazyClearCountForTesting;
}

void DeviceBase::EmitWarningOnce(std::string_view message) {
    if (mWarnings.insert(std::string{message}).second) {
        this->EmitLog(wgpu::LoggingType::Warning, message);
    }
}

void DeviceBase::EmitCompilationLog(const ShaderModuleBase* module) {
    const OwnedCompilationMessages* messages = module->GetCompilationMessages();
    if (!messages->HasWarningsOrErrors()) {
        return;
    }

    // Limit the number of compilation error emitted to avoid spamming the devtools console hard.
    constexpr uint32_t kCompilationLogSpamLimit = 20;
    if (mEmittedCompilationLogCount.load(std::memory_order_acquire) > kCompilationLogSpamLimit) {
        return;
    }

    if (mEmittedCompilationLogCount.fetch_add(1, std::memory_order_acq_rel) ==
        kCompilationLogSpamLimit - 1) {
        // Note: if there are multiple threads emitting logs, this may not actually be the exact
        // last message. This is probably not a huge problem since this message will be emitted
        // somewhere near the end.
        EmitLog(wgpu::LoggingType::Warning,
                "Reached the WGSL compilation log warning limit. To see all the compilation "
                "logs, query them directly on the ShaderModule objects.");
    }

    auto msg = module->GetCompilationLog();
    if (!msg.empty()) {
        EmitLog(wgpu::LoggingType::Warning, msg.c_str());
    }
}

void DeviceBase::EmitLog(std::string_view message) {
    this->EmitLog(wgpu::LoggingType::Info, message);
}

void DeviceBase::EmitLog(wgpu::LoggingType type, std::string_view message) {
    // Acquire a shared lock. This allows multiple threads to emit logs,
    // or even logs to be emitted re-entrantly. It will block if there is a call
    // to SetLoggingCallback. Applications should not call SetLoggingCallback inside
    // the logging callback or they will deadlock.
    std::shared_lock<std::shared_mutex> lock(mLoggingMutex);
    if (mLoggingCallbackInfo.callback) {
        mLoggingCallbackInfo.callback(ToAPI(type), ToOutputStringView(message),
                                      mLoggingCallbackInfo.userdata1,
                                      mLoggingCallbackInfo.userdata2);
    }
}

wgpu::Status DeviceBase::APIGetAHardwareBufferProperties(void* handle,
                                                         AHardwareBufferProperties* properties) {
    if (!HasFeature(Feature::SharedTextureMemoryAHardwareBuffer)) {
        ConsumeError(
            DAWN_VALIDATION_ERROR("Queried APIGetAHardwareBufferProperties() on %s "
                                  "without the %s feature being set.",
                                  this, ToAPI(Feature::SharedTextureMemoryAHardwareBuffer)));
        return wgpu::Status::Error;
    }

    // This method makes a Vulkan API call that will return an error if `handle` is invalid. This
    // is not cause to lose the Dawn device, as it is a client-side error and not a true internal
    // Dawn error.
    if (ConsumedError(GetAHardwareBufferPropertiesImpl(handle, properties),
                      InternalErrorType::Internal)) {
        return wgpu::Status::Error;
    }

    return wgpu::Status::Success;
}

wgpu::Status DeviceBase::APIGetLimits(Limits* limits) const {
    if (GetAdapter()->GetInstance()->ConsumedError(FillLimits(limits, mEnabledFeatures, mLimits))) {
        return wgpu::Status::Error;
    }
    return wgpu::Status::Success;
}

bool DeviceBase::APIHasFeature(wgpu::FeatureName feature) const {
    return mEnabledFeatures.IsEnabled(feature);
}

void DeviceBase::APIGetFeatures(wgpu::SupportedFeatures* features) const {
    this->APIGetFeatures(reinterpret_cast<SupportedFeatures*>(features));
}

void DeviceBase::APIGetFeatures(SupportedFeatures* features) const {
    mEnabledFeatures.ToSupportedFeatures(features);
}

wgpu::Status DeviceBase::APIGetAdapterInfo(AdapterInfo* adapterInfo) const {
    return mAdapter->APIGetInfo(adapterInfo);
}

Future DeviceBase::APIGetLostFuture() const {
    if (mLostEvent) {
        return mLostEvent->GetFuture();
    }
    DAWN_ASSERT(mLostFuture.id != kNullFutureID);
    return mLostFuture;
}

void DeviceBase::APIInjectError(wgpu::ErrorType type, StringView message) {
    if (ConsumedError(ValidateErrorType(type))) {
        return;
    }

    // This method should only be used to make error scope reject. For DeviceLost there is the
    // LoseForTesting function that can be used instead.
    if (type != wgpu::ErrorType::Validation && type != wgpu::ErrorType::OutOfMemory) {
        HandleError(
            DAWN_VALIDATION_ERROR("Invalid injected error, must be Validation or OutOfMemory"));
        return;
    }

    message = utils::NormalizeMessageString(message);
    HandleError(DAWN_MAKE_ERROR(FromWGPUErrorType(type), std::string(message)),
                InternalErrorType::OutOfMemory);
}

void DeviceBase::APIValidateTextureDescriptor(const TextureDescriptor* descriptorOrig) {
    AllowMultiPlanarTextureFormat allowMultiPlanar;
    if (HasFeature(Feature::MultiPlanarFormatExtendedUsages)) {
        allowMultiPlanar = AllowMultiPlanarTextureFormat::Yes;
    } else {
        allowMultiPlanar = AllowMultiPlanarTextureFormat::No;
    }

    TextureDescriptor rawDescriptor = descriptorOrig->WithTrivialFrontendDefaults();

    UnpackedPtr<TextureDescriptor> unpacked;
    if (!ConsumedError(ValidateAndUnpack(&rawDescriptor), &unpacked)) {
        [[maybe_unused]] bool hadError =
            ConsumedError(ValidateTextureDescriptor(this, unpacked, allowMultiPlanar));
    }
}

QueueBase* DeviceBase::GetQueue() const {
    DAWN_ASSERT(mQueue != nullptr);
    return mQueue.Get();
}

// Implementation details of object creation

ResultOrError<Ref<BindGroupBase>> DeviceBase::CreateBindGroup(const BindGroupDescriptor* descriptor,
                                                              UsageValidationMode mode) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateBindGroupDescriptor(this, descriptor, mode),
                         "validating %s against %s", descriptor, descriptor->layout);
    }
    return CreateBindGroupImpl(descriptor);
}

ResultOrError<Ref<BindGroupLayoutBase>> DeviceBase::CreateBindGroupLayout(
    const BindGroupLayoutDescriptor* descriptor,
    bool allowInternalBinding) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateBindGroupLayoutDescriptor(this, descriptor, allowInternalBinding),
                         "validating %s", descriptor);
    }
    return GetOrCreateBindGroupLayout(descriptor);
}

ResultOrError<Ref<BufferBase>> DeviceBase::CreateBuffer(const BufferDescriptor* rawDescriptor) {
    DAWN_TRY(ValidateIsAlive());
    UnpackedPtr<BufferDescriptor> descriptor;
    if (IsValidationEnabled()) {
        DAWN_TRY_ASSIGN(descriptor, ValidateBufferDescriptor(this, rawDescriptor));
    } else {
        descriptor = Unpack(rawDescriptor);
    }

    Ref<BufferBase> buffer;
    DAWN_TRY_ASSIGN(buffer, CreateBufferImpl(descriptor));

    if (descriptor->mappedAtCreation) {
        DAWN_TRY(buffer->MapAtCreation());
    }

    return std::move(buffer);
}

ResultOrError<Ref<ComputePipelineBase>> DeviceBase::CreateComputePipeline(
    const ComputePipelineDescriptor* descriptor) {
    // If a pipeline layout is not specified, we cannot use cached pipelines.
    bool useCache = descriptor->layout != nullptr;

    Ref<ComputePipelineBase> uninitializedComputePipeline;
    DAWN_TRY_ASSIGN(uninitializedComputePipeline, CreateUninitializedComputePipeline(descriptor));

    if (useCache) {
        Ref<ComputePipelineBase> cachedComputePipeline =
            GetCachedComputePipeline(uninitializedComputePipeline.Get());
        if (cachedComputePipeline.Get() != nullptr) {
            return cachedComputePipeline;
        }
    }

    MaybeError maybeError;
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetPlatform(), "CreateComputePipelineUS");
        maybeError = uninitializedComputePipeline->Initialize();
    }
    DAWN_HISTOGRAM_BOOLEAN(GetPlatform(), "CreateComputePipelineSuccess", maybeError.IsSuccess());

    DAWN_TRY(std::move(maybeError));
    return useCache ? AddOrGetCachedComputePipeline(std::move(uninitializedComputePipeline))
                    : std::move(uninitializedComputePipeline);
}

ResultOrError<Ref<CommandEncoder>> DeviceBase::CreateCommandEncoder(
    const CommandEncoderDescriptor* descriptor) {
    const CommandEncoderDescriptor defaultDescriptor = {};
    if (descriptor == nullptr) {
        descriptor = &defaultDescriptor;
    }

    DAWN_TRY(ValidateIsAlive());
    UnpackedPtr<CommandEncoderDescriptor> unpacked;
    if (IsValidationEnabled()) {
        DAWN_TRY_ASSIGN(unpacked, ValidateCommandEncoderDescriptor(this, descriptor));
    } else {
        unpacked = Unpack(descriptor);
    }
    return CommandEncoder::Create(this, unpacked);
}

// Overwritten on the backends to return pipeline caches if supported.
Ref<PipelineCacheBase> DeviceBase::GetOrCreatePipelineCacheImpl(const CacheKey& key) {
    DAWN_UNREACHABLE();
}

ResultOrError<Ref<ComputePipelineBase>> DeviceBase::CreateUninitializedComputePipeline(
    const ComputePipelineDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY(ValidateComputePipelineDescriptor(this, descriptor));
    }

    Ref<PipelineLayoutBase> layoutRef;
    ComputePipelineDescriptor appliedDescriptor;
    DAWN_TRY_ASSIGN(layoutRef, ValidateLayoutAndGetComputePipelineDescriptorWithDefaults(
                                   this, *descriptor, &appliedDescriptor));

    return CreateUninitializedComputePipelineImpl(Unpack(&appliedDescriptor));
}

// This base version is creating the pipeline synchronously,
// and is overwritten on the backends that actually support asynchronous pipeline creation.
void DeviceBase::InitializeComputePipelineAsyncImpl(Ref<CreateComputePipelineAsyncEvent> event) {
    event->InitializeSync();
}

// This base version is creating the pipeline synchronously,
// and is overwritten on the backends that actually support asynchronous pipeline creation.
void DeviceBase::InitializeRenderPipelineAsyncImpl(Ref<CreateRenderPipelineAsyncEvent> event) {
    event->InitializeSync();
}

ResultOrError<Ref<PipelineLayoutBase>> DeviceBase::CreatePipelineLayout(
    const PipelineLayoutDescriptor* descriptor,
    PipelineCompatibilityToken pipelineCompatibilityToken) {
    DAWN_TRY(ValidateIsAlive());
    UnpackedPtr<PipelineLayoutDescriptor> unpacked;
    if (IsValidationEnabled()) {
        DAWN_TRY_ASSIGN(unpacked, ValidatePipelineLayoutDescriptor(this, descriptor,
                                                                   pipelineCompatibilityToken));
    } else {
        unpacked = Unpack(descriptor);
    }

    // When we are not creating explicit pipeline layouts, i.e. we are using 'auto', don't use the
    // cache.
    if (pipelineCompatibilityToken != kExplicitPCT) {
        Ref<PipelineLayoutBase> result;
        DAWN_TRY_ASSIGN(result, CreatePipelineLayoutImpl(unpacked));
        result->SetContentHash(result->ComputeContentHash());
        return result;
    }
    return GetOrCreatePipelineLayout(unpacked);
}

ResultOrError<Ref<ExternalTextureBase>> DeviceBase::CreateExternalTextureImpl(
    const ExternalTextureDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateExternalTextureDescriptor(this, descriptor), "validating %s",
                         descriptor);
    }

    return ExternalTextureBase::Create(this, descriptor);
}

ResultOrError<Ref<QuerySetBase>> DeviceBase::CreateQuerySet(const QuerySetDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateQuerySetDescriptor(this, descriptor), "validating %s", descriptor);
    }
    return CreateQuerySetImpl(descriptor);
}

ResultOrError<Ref<RenderBundleEncoder>> DeviceBase::CreateRenderBundleEncoder(
    const RenderBundleEncoderDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateRenderBundleEncoderDescriptor(this, descriptor),
                         "validating render bundle encoder descriptor.");
    }
    return RenderBundleEncoder::Create(this, descriptor);
}

ResultOrError<Ref<RenderPipelineBase>> DeviceBase::CreateRenderPipeline(
    const RenderPipelineDescriptor* descriptor,
    bool allowInternalBinding) {
    // If a pipeline layout is not specified, we cannot use cached pipelines.
    bool useCache = descriptor->layout != nullptr;

    Ref<RenderPipelineBase> uninitializedRenderPipeline;
    DAWN_TRY_ASSIGN(uninitializedRenderPipeline,
                    CreateUninitializedRenderPipeline(descriptor, allowInternalBinding));

    if (useCache) {
        Ref<RenderPipelineBase> cachedRenderPipeline =
            GetCachedRenderPipeline(uninitializedRenderPipeline.Get());
        if (cachedRenderPipeline != nullptr) {
            return cachedRenderPipeline;
        }
    }

    MaybeError maybeError;
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetPlatform(), "CreateRenderPipelineUS");
        maybeError = uninitializedRenderPipeline->Initialize();
    }
    DAWN_HISTOGRAM_BOOLEAN(GetPlatform(), "CreateRenderPipelineSuccess", maybeError.IsSuccess());

    DAWN_TRY(std::move(maybeError));
    return useCache ? AddOrGetCachedRenderPipeline(std::move(uninitializedRenderPipeline))
                    : std::move(uninitializedRenderPipeline);
}

ResultOrError<Ref<RenderPipelineBase>> DeviceBase::CreateUninitializedRenderPipeline(
    const RenderPipelineDescriptor* descriptor,
    bool allowInternalBinding) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY(ValidateRenderPipelineDescriptor(this, descriptor));

        // Validation for kMaxBindGroupsPlusVertexBuffers is skipped because it is not necessary so
        // far.
        static_assert(kMaxBindGroups + kMaxVertexBuffers <= kMaxBindGroupsPlusVertexBuffers);
    }

    // Ref will keep the pipeline layout alive until the end of the function where
    // the pipeline will take another reference.
    Ref<PipelineLayoutBase> layoutRef;
    RenderPipelineDescriptor appliedDescriptor;
    DAWN_TRY_ASSIGN(layoutRef, ValidateLayoutAndGetRenderPipelineDescriptorWithDefaults(
                                   this, *descriptor, &appliedDescriptor, allowInternalBinding));

    return CreateUninitializedRenderPipelineImpl(Unpack(&appliedDescriptor));
}

ResultOrError<Ref<SamplerBase>> DeviceBase::CreateSampler(const SamplerDescriptor* descriptorOrig) {
    DAWN_TRY(ValidateIsAlive());

    SamplerDescriptor descriptor = {};
    if (descriptorOrig) {
        descriptor = descriptorOrig->WithTrivialFrontendDefaults();
    }

    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateSamplerDescriptor(this, &descriptor), "validating %s",
                         &descriptor);
    }

    return GetOrCreateSampler(&descriptor);
}

ResultOrError<Ref<ShaderModuleBase>> DeviceBase::CreateShaderModule(
    const ShaderModuleDescriptor* descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    ShaderModuleParseResult* outputParseResult) {
    DAWN_TRY(ValidateIsAlive());

    // Unpack and validate the descriptor chain before doing further validation or cache
    // lookups.
    UnpackedPtr<ShaderModuleDescriptor> unpacked;
    DAWN_TRY_ASSIGN_CONTEXT(unpacked, ValidateAndUnpack(descriptor), "validating and unpacking %s",
                            descriptor);

    // A WGSL (xor SPIR-V, if enabled) subdescriptor is required, and a Dawn-specific SPIR-V
    // options descriptor is allowed when using SPIR-V.
    wgpu::SType moduleType = wgpu::SType(0u);
    DAWN_TRY_ASSIGN(
        moduleType,
        (unpacked.ValidateBranches<Branch<ShaderSourceWGSL, ShaderModuleCompilationOptions>,
                                   Branch<ShaderSourceSPIRV, DawnShaderModuleSPIRVOptionsDescriptor,
                                          ShaderModuleCompilationOptions>>()));

    // Module type specific validation
    switch (moduleType) {
        case wgpu::SType::ShaderSourceSPIRV: {
            DAWN_INVALID_IF(
                !TINT_BUILD_SPV_READER || IsToggleEnabled(Toggle::DisallowSpirv) ||
                    !GetInstance()->HasFeature(wgpu::InstanceFeatureName::ShaderSourceSPIRV),
                "SPIR-V is disallowed.");
            break;
        }
        case wgpu::SType::ShaderSourceWGSL: {
            DAWN_INVALID_IF(unpacked.Has<ShaderModuleCompilationOptions>() &&
                                !HasFeature(Feature::ShaderModuleCompilationOptions),
                            "Shader module compilation options used without %s enabled.",
                            wgpu::FeatureName::ShaderModuleCompilationOptions);
            break;
        }
        default:
            DAWN_UNREACHABLE();
    }

    // Dump shader source code if required.
    if (IsToggleEnabled(Toggle::DumpShaders)) {
        DumpShaderFromDescriptor(this, unpacked);
    }

    // Check the cache and do actual validation and parsing if cache missed.
    ShaderModuleBase blueprint(this, unpacked, internalExtensions,
                               ApiObjectBase::kUntrackedByDevice);

    const size_t blueprintHash = blueprint.ComputeContentHash();
    blueprint.SetContentHash(blueprintHash);

    // Check in-memory shader module cache first, and if missed check the blob cache, and if missed
    // again call ParseShaderModule.
    return GetOrCreate(
        mCaches->shaderModules, &blueprint, [&]() -> ResultOrError<Ref<ShaderModuleBase>> {
            SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetPlatform(), "CreateShaderModuleUS");

            auto resultOrError = [&]() -> ResultOrError<Ref<ShaderModuleBase>> {
                ShaderModuleParseRequest req = BuildShaderModuleParseRequest(
                    this, blueprint.GetHash(), unpacked, internalExtensions,
                    /* needReflection */ true);

                // Check blob cache first before calling ParseShaderModule. ShaderModuleParseResult
                // returned from blob cache or ParseShaderModule will hold compilation messages and
                // validation errors if any. ShaderModuleParseResult from ParseShaderModule also
                // holds tint program.
                CacheResult<ShaderModuleParseResult> result;
                DAWN_TRY_LOAD_OR_RUN(result, this, std::move(req),
                                     ShaderModuleParseResult::FromBlob, ParseShaderModule,
                                     "ShaderModuleParsing");
                GetBlobCache()->EnsureStored(result);
                ShaderModuleParseResult parseResult = result.Acquire();

                // If ShaderModuleParseResult has validation error, move the compilation messages to
                // *outputParseResult so that we can create an error shader module from it, and then
                // return the validation error.
                if (parseResult.HasError()) {
                    auto error = parseResult.cachedValidationError->ToErrorData();
                    if (outputParseResult) {
                        *outputParseResult = std::move(parseResult);
                    }
                    return error;
                }
                // Otherwise with no error, create a shader module from parse result and return it.
                Ref<ShaderModuleBase> shaderModule;
                DAWN_TRY_ASSIGN(shaderModule,
                                CreateShaderModuleImpl(unpacked, internalExtensions, &parseResult));
                shaderModule->SetContentHash(blueprintHash);
                return shaderModule;
            }();

            DAWN_HISTOGRAM_BOOLEAN(GetPlatform(), "CreateShaderModuleSuccess",
                                   resultOrError.IsSuccess());

            return resultOrError;
        });
}

ResultOrError<Ref<SwapChainBase>> DeviceBase::CreateSwapChain(Surface* surface,
                                                              SwapChainBase* previousSwapChain,
                                                              const SurfaceConfiguration* config) {
    // Nothing to validate here as it is done in Surface::Configure
    return CreateSwapChainImpl(surface, previousSwapChain, config);
}

ResultOrError<Ref<TextureBase>> DeviceBase::CreateTexture(const TextureDescriptor* descriptorOrig) {
    DAWN_TRY(ValidateIsAlive());

    TextureDescriptor rawDescriptor = descriptorOrig->WithTrivialFrontendDefaults();

    UnpackedPtr<TextureDescriptor> descriptor;
    if (IsValidationEnabled()) {
        AllowMultiPlanarTextureFormat allowMultiPlanar;
        if (HasFeature(Feature::MultiPlanarFormatExtendedUsages)) {
            allowMultiPlanar = AllowMultiPlanarTextureFormat::SingleLayerOnly;
        } else {
            allowMultiPlanar = AllowMultiPlanarTextureFormat::No;
        }
        DAWN_TRY_ASSIGN_CONTEXT(descriptor, ValidateAndUnpack(&rawDescriptor), "validating %s.",
                                &rawDescriptor);
        DAWN_TRY_CONTEXT(ValidateTextureDescriptor(this, descriptor, allowMultiPlanar),
                         "validating %s.", descriptor);
    } else {
        descriptor = Unpack(&rawDescriptor);
    }

    return CreateTextureImpl(descriptor);
}

ResultOrError<Ref<TextureViewBase>> DeviceBase::CreateTextureView(
    TextureBase* texture,
    const TextureViewDescriptor* descriptorOrig) {
    DAWN_TRY(ValidateIsAlive());
    DAWN_TRY(ValidateObject(texture));

    TextureViewDescriptor desc;
    DAWN_TRY_ASSIGN(desc, GetTextureViewDescriptorWithDefaults(texture, descriptorOrig));
    std::string generatedLabel;
    if (descriptorOrig == nullptr) {
        generatedLabel = absl::StrFormat("defaulted from %s", texture);
        desc.label = {generatedLabel.data(), generatedLabel.length()};
    }

    UnpackedPtr<TextureViewDescriptor> descriptor;
    if (IsValidationEnabled()) {
        DAWN_TRY_ASSIGN_CONTEXT(descriptor, ValidateAndUnpack(&desc), "validating %s.", &desc);
        DAWN_TRY_CONTEXT(ValidateTextureViewDescriptor(this, texture, descriptor),
                         "validating %s against %s.", descriptor, texture);
    } else {
        descriptor = Unpack(&desc);
    }

    return texture->GetOrCreateViewFromCache(
        descriptor, [&](const TextureViewQuery&) -> ResultOrError<Ref<TextureViewBase>> {
            return CreateTextureViewImpl(texture, descriptor);
        });
}

// Other implementation details

DynamicUploader* DeviceBase::GetDynamicUploader() const {
    return mDynamicUploader.get();
}

// The Toggle device facility

std::vector<const char*> DeviceBase::GetTogglesUsed() const {
    return mToggles.GetEnabledToggleNames();
}

bool DeviceBase::IsToggleEnabled(Toggle toggle) const {
    return mToggles.IsEnabled(toggle);
}

const TogglesState& DeviceBase::GetTogglesState() const {
    return mToggles;
}

const FeaturesSet& DeviceBase::GetEnabledFeatures() const {
    return mEnabledFeatures;
}

void DeviceBase::ForceEnableFeatureForTesting(Feature feature) {
    mEnabledFeatures.EnableFeature(feature);
    mFormatTable = BuildFormatTable(this);
}

void DeviceBase::FlushCallbackTaskQueue() {
    // Callbacks might cause re-entrances. Mutex shouldn't be locked. So we expect there is no
    // locked mutex before entering this method.
    DAWN_ASSERT(mMutex == nullptr || !mMutex->IsLockedByCurrentThread());

    Ref<CallbackTaskManager> callbackTaskManager;

    {
        // This is a data race with the assignment to InstanceBase's callback queue manager in
        // WillDropLastExternalRef(). Need to protect with a lock and keep the old
        // mCallbackTaskManager alive.
        // TODO(crbug.com/dawn/752): In future, all devices should use InstanceBase's callback queue
        // manager from the start. So we won't need to care about data race at that point.
        auto deviceGuard = GetGuard();
        callbackTaskManager = mCallbackTaskManager;
    }

    callbackTaskManager->Flush();
}

const CombinedLimits& DeviceBase::GetLimits() const {
    return mLimits;
}

AsyncTaskManager* DeviceBase::GetAsyncTaskManager() const {
    return mAsyncTaskManager.get();
}

CallbackTaskManager* DeviceBase::GetCallbackTaskManager() const {
    return mCallbackTaskManager.Get();
}

dawn::platform::WorkerTaskPool* DeviceBase::GetWorkerTaskPool() const {
    return mWorkerTaskPool.get();
}

PipelineCompatibilityToken DeviceBase::GetNextPipelineCompatibilityToken() {
    return PipelineCompatibilityToken(mNextPipelineCompatibilityToken++);
}

const CacheKey& DeviceBase::GetCacheKey() const {
    return mDeviceCacheKey;
}

const std::string& DeviceBase::GetLabel() const {
    return mLabel;
}

void DeviceBase::APISetLabel(StringView label) {
    mLabel = utils::NormalizeMessageString(label);
    SetLabelImpl();
}

void DeviceBase::SetLabelImpl() {}

bool DeviceBase::ReduceMemoryUsageImpl() {
    return false;
}

void DeviceBase::PerformIdleTasksImpl() {}

bool DeviceBase::ShouldDuplicateNumWorkgroupsForDispatchIndirect(
    ComputePipelineBase* computePipeline) const {
    return false;
}

bool DeviceBase::MayRequireDuplicationOfIndirectParameters() const {
    return false;
}

bool DeviceBase::ShouldDuplicateParametersForDrawIndirect(
    const RenderPipelineBase* renderPipelineBase) const {
    return false;
}

bool DeviceBase::BackendWillValidateMultiDraw() const {
    return false;
}

bool DeviceBase::ShouldApplyIndexBufferOffsetToFirstIndex() const {
    return false;
}

bool DeviceBase::CanTextureLoadResolveTargetInTheSameRenderpass() const {
    return false;
}

bool DeviceBase::CanResolveSubRect() const {
    return false;
}

bool DeviceBase::CanAddStorageUsageToBufferWithoutSideEffects(wgpu::BufferUsage storageUsage,
                                                              wgpu::BufferUsage originalUsage,
                                                              size_t bufferSize) const {
    return true;
}

uint64_t DeviceBase::GetBufferCopyOffsetAlignmentForDepthStencil() const {
    // For depth-stencil texture, buffer offset must be a multiple of 4, which is required
    // by WebGPU and Vulkan SPEC.
    return 4u;
}

MaybeError DeviceBase::CopyFromStagingToTexture(BufferBase* source,
                                                const TexelCopyBufferLayout& src,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    if (dst.aspect == Aspect::Depth &&
        IsToggleEnabled(Toggle::UseBlitForBufferToDepthTextureCopy)) {
        DAWN_TRY_CONTEXT(BlitStagingBufferToDepth(this, source, src, dst, copySizePixels),
                         "copying from staging buffer to depth aspect of %s using blit workaround.",
                         dst.texture.Get());
    } else if (dst.aspect == Aspect::Stencil &&
               IsToggleEnabled(Toggle::UseBlitForBufferToStencilTextureCopy)) {
        DAWN_TRY_CONTEXT(
            BlitStagingBufferToStencil(this, source, src, dst, copySizePixels),
            "copying from staging buffer to stencil aspect of %s using blit workaround.",
            dst.texture.Get());
    } else {
        DAWN_TRY(CopyFromStagingToTextureImpl(source, src, dst, copySizePixels));
    }

    return {};
}

DeviceGuard DeviceBase::GetGuard() {
    return DeviceGuard(this);
}

DeviceGuard DeviceBase::GetGuardForDelete() {
    // When acquiring the guard for deletion, we do not currently enable Defer. This is not
    // currently enforced by any assertions here because it would require making the Defer class
    // a refcounted object to handle the case when the Device is destroyed while the lock is held,
    // resulting in a dangling pointer to the Defer owned by the Device. As a proxy assertion,
    // ~DeviceBase checks that the Defer object is not set.
    return DeviceGuard(this, mMutex.Get());
}

void DeviceBase::DeferIfLocked(std::function<void()> f) {
    // If we are not using implicit synchronized mode, we don't have a device-wide lock, so we can
    // just run the defer task now.
    if (mMutex == nullptr) {
        f();
        return;
    }

    // If we don't have a Defer, that means we are not locked, so we can just run the defer task
    // now.
    if (!mMutex->mDefer) {
        f();
        return;
    }

    // Otherwise, verify that we are only calling this in the thread that is holding the lock and
    // defer the function.
    DAWN_ASSERT(mMutex->IsLockedByCurrentThread() && mMutex->mDefer);
    mMutex->mDefer->Append(std::move(f));
}

bool DeviceBase::IsLockedByCurrentThreadIfNeeded() const {
    return mMutex == nullptr || mMutex->IsLockedByCurrentThread();
}

void DeviceBase::DumpMemoryStatistics(dawn::native::MemoryDump* dump) const {
    DAWN_ASSERT(IsLockedByCurrentThreadIfNeeded());
    std::string prefix = absl::StrFormat("device_%p", static_cast<const void*>(this));
    GetObjectTrackingList(ObjectType::Texture)->ForEach([&](const ApiObjectBase* texture) {
        static_cast<const TextureBase*>(texture)->DumpMemoryStatistics(dump, prefix.c_str());
    });
    GetObjectTrackingList(ObjectType::Buffer)->ForEach([&](const ApiObjectBase* buffer) {
        static_cast<const BufferBase*>(buffer)->DumpMemoryStatistics(dump, prefix.c_str());
    });
}

MemoryUsageInfo DeviceBase::ComputeEstimatedMemoryUsage() const {
    DAWN_ASSERT(IsLockedByCurrentThreadIfNeeded());
    MemoryUsageInfo info = {};

    GetObjectTrackingList(ObjectType::Texture)->ForEach([&](const ApiObjectBase* ptr) {
        auto texture = static_cast<const TextureBase*>(ptr);
        auto size = texture->ComputeEstimatedByteSize();
        info.totalUsage += size;
        info.texturesUsage += size;
        if (texture->GetSampleCount() > 1) {
            info.msaaTexturesUsage += size;
            info.msaaTexturesCount++;
            info.largestMsaaTextureUsage = std::max(info.largestMsaaTextureUsage, size);
        }
        if (texture->GetFormat().HasDepthOrStencil()) {
            info.depthStencilTexturesUsage += size;
        }
    });
    GetObjectTrackingList(ObjectType::Buffer)->ForEach([&](const ApiObjectBase* buffer) {
        auto size = static_cast<const BufferBase*>(buffer)->GetAllocatedSize();
        info.totalUsage += size;
        info.buffersUsage += size;
    });

    return info;
}

AllocatorMemoryInfo DeviceBase::GetAllocatorMemoryInfo() const {
    return {};
}

bool DeviceBase::ReduceMemoryUsage() {
    DAWN_ASSERT(IsLockedByCurrentThreadIfNeeded());
    if (ConsumedError(GetQueue()->CheckPassedSerials())) {
        return false;
    }
    GetDynamicUploader()->Deallocate(GetQueue()->GetCompletedCommandSerial(), /*freeAll=*/true);
    mInternalPipelineStore->ResetScratchBuffers();
    mTemporaryUniformBuffer = nullptr;

    GetObjectTrackingList(ObjectType::BindGroupLayoutInternal)->ForEach([](ApiObjectBase* object) {
        static_cast<BindGroupLayoutInternalBase*>(object)->ReduceMemoryUsage();
    });

    TrimErrorScopeStacks(mErrorScopeStacks);

    // TODO(crbug.com/398193014): This could return a future to wait on instead of just a bool
    // saying there is work to wait on.
    return ReduceMemoryUsageImpl();
}

void DeviceBase::PerformIdleTasks() {
    DAWN_ASSERT(IsLockedByCurrentThreadIfNeeded());
    PerformIdleTasksImpl();
}

ResultOrError<Ref<BufferBase>> DeviceBase::GetOrCreateTemporaryUniformBuffer(size_t size) {
    if (!mTemporaryUniformBuffer || mTemporaryUniformBuffer->GetSize() != size) {
        BufferDescriptor desc;
        desc.label = "Internal_TemporaryUniform";
        desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        desc.size = size;
        DAWN_TRY_ASSIGN(mTemporaryUniformBuffer, CreateBuffer(&desc));
    }

    return mTemporaryUniformBuffer;
}

bool DeviceBase::HasFlexibleTextureViews() const {
    // TODO(384921944): Once FlexibleTextureViews is enabled by default in Core Mode, we only need
    // to check HasFeature(FlexibleTextureViews).
    return !IsCompatibilityMode() || HasFeature(Feature::FlexibleTextureViews);
}

std::string_view DeviceBase::GetIsolatedEntryPointName() const {
    return mIsolatedEntryPointName;
}

IgnoreLazyClearCountScope::IgnoreLazyClearCountScope(DeviceBase* device)
    : mDevice(device), mLazyClearCountForTesting(device->mLazyClearCountForTesting) {}

IgnoreLazyClearCountScope::~IgnoreLazyClearCountScope() {
    mDevice->mLazyClearCountForTesting = mLazyClearCountForTesting;
}

std::pair<std::string, bool> DeviceBase::GetTraceInfo() {
    static std::atomic<uint32_t> s_count = 0;

    auto [traceFileBase, traceFileBaseSet] = GetEnvironmentVar("DAWN_TRACE_FILE_BASE");
    auto [traceDeviceFilter, traceDeviceFilterSet] = GetEnvironmentVar("DAWN_TRACE_DEVICE_FILTER");

    // Skip if trace file name is not set.
    if (!traceFileBaseSet) {
        return {"", false};
    }

    // If a filter was specified, only trace if the filter is found
    if (traceDeviceFilterSet && GetLabel().find(traceDeviceFilter) == std::string::npos) {
        return {"", false};
    }

    /*
    One reason to put the date is Metal will not overwrite an existing trace.
    Deleting a trace is problematic as a trace is a folder and dawn shouldn't
    be deleting folders. So, adding the date means when you re-run your program
    you'll get a trace that doesn't clash with your last trace. The count is
    there because if you're running something that makes makes more than one
    device, if they are created within the same second they'd have the same
    name and metal would fail to capture.
    */

    uint32_t count = s_count.fetch_add(1, std::memory_order_acq_rel);

    std::time_t now = std::time(0);
    std::tm tm(*std::localtime(&now));
    std::string traceName(absl::StrFormat("%s-%04d-%02d-%02dT%02d-%02d-%02d-c%03d", traceFileBase,
                                          tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                                          tm.tm_min, tm.tm_sec, count));

    return {traceName, true};
}

tint::InternalCompilerErrorCallbackInfo DeviceBase::GetTintInternalCompilerErrorCallback() {
    static auto tintInternalCompilerErrorCallback = [](std::string err, void* userdata) {
        static_cast<DeviceBase*>(userdata)->HandleError(DAWN_INTERNAL_ERROR(err));
    };
    return tint::InternalCompilerErrorCallbackInfo{
        .callback = tintInternalCompilerErrorCallback,
        .userdata = this,
    };
}

}  // namespace dawn::native
