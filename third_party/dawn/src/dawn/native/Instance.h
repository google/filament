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

#ifndef SRC_DAWN_NATIVE_INSTANCE_H_
#define SRC_DAWN_NATIVE_INSTANCE_H_

#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCountedWithExternalCount.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/BackendConnection.h"
#include "dawn/native/ErrorSink.h"
#include "dawn/native/EventManager.h"
#include "dawn/native/Features.h"
#include "dawn/native/Forward.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "tint/lang/wgsl/features/language_feature.h"

namespace dawn::platform {
class Platform;
}  // namespace dawn::platform

namespace dawn::native {

class CallbackTaskManager;
class DeviceBase;
class Surface;
class X11Functions;

using BackendsBitset = ityp::bitset<wgpu::BackendType, kEnumCount<wgpu::BackendType>>;
using BackendsArray = ityp::
    array<wgpu::BackendType, std::unique_ptr<BackendConnection>, kEnumCount<wgpu::BackendType>>;

wgpu::Status APIGetInstanceCapabilities(InstanceCapabilities* capabilities);
InstanceBase* APICreateInstance(const InstanceDescriptor* descriptor);

// This is called InstanceBase for consistency across the frontend, even if the backends don't
// specialize this class.
class InstanceBase final : public ErrorSink, public RefCountedWithExternalCount<RefCounted> {
  public:
    static ResultOrError<Ref<InstanceBase>> Create(const InstanceDescriptor* descriptor = nullptr);

    Future APIRequestAdapter(const RequestAdapterOptions* options,
                             const WGPURequestAdapterCallbackInfo& callbackInfo);

    // Discovers and returns a vector of adapters.
    // All systems adapters that can be found are returned if no options are passed.
    // Otherwise, returns adapters based on the `options`.
    std::vector<Ref<AdapterBase>> EnumerateAdapters(const RequestAdapterOptions* options = nullptr);

    void EmitLog(WGPULoggingType type, const std::string_view message) const;

    // Consume an error and log its warning at most once. This is useful for
    // physical device creation errors that happen because the backend is not
    // supported or doesn't meet the required capabilities.
    bool ConsumedErrorAndWarnOnce(MaybeError maybeError);

    template <typename T>
    [[nodiscard]] bool ConsumedErrorAndWarnOnce(ResultOrError<T> resultOrError, T* result) {
        if (DAWN_UNLIKELY(resultOrError.IsError())) {
            return ConsumedErrorAndWarnOnce(resultOrError.AcquireError());
        }
        *result = resultOrError.AcquireSuccess();
        return false;
    }

    const TogglesState& GetTogglesState() const;
    const absl::flat_hash_set<tint::wgsl::LanguageFeature>& GetAllowedWGSLLanguageFeatures() const;

    // Used to query the details of a toggle. Return nullptr if toggleName is not a valid name
    // of a toggle supported in Dawn.
    const ToggleInfo* GetToggleInfo(const char* toggleName);
    Toggle ToggleNameToEnum(const char* toggleName);

    // TODO(dawn:2166): Move this method to PhysicalDevice to better detect that the backend
    // validation is actually enabled or not when a physical device is created. Sometimes it is
    // enabled externally via command line or environment variables.
    bool IsBackendValidationEnabled() const;
    void SetBackendValidationLevel(BackendValidationLevel level);
    BackendValidationLevel GetBackendValidationLevel() const;

    bool IsBeginCaptureOnStartupEnabled() const;

    // Testing only API that is NOT thread-safe.
    void SetPlatformForTesting(dawn::platform::Platform* platform);
    dawn::platform::Platform* GetPlatform();

    // Testing only API that is NOT thread-safe.
    uint64_t GetDeprecationWarningCountForTesting();
    void EmitDeprecationWarning(const std::string& warning);

    uint64_t GetDeviceCountForTesting() const;
    void AddDevice(DeviceBase* device);
    void RemoveDevice(DeviceBase* device);

    const std::vector<std::string>& GetRuntimeSearchPaths() const;

    const Ref<CallbackTaskManager>& GetCallbackTaskManager() const;
    EventManager* GetEventManager();

    // Get backend-independent libraries that need to be loaded dynamically.
    const X11Functions* GetOrLoadX11Functions();

    // TODO(dawn:752) Standardize webgpu.h to decide if we should return bool.
    //   Currently this is a backdoor for Chromium's process event loop.
    bool ProcessEvents();

    // Dawn API
    Surface* APICreateSurface(const SurfaceDescriptor* descriptor);
    void APIProcessEvents();
    [[nodiscard]] wgpu::WaitStatus APIWaitAny(size_t count,
                                              FutureWaitInfo* futures,
                                              uint64_t timeoutNS);
    bool APIHasWGSLLanguageFeature(wgpu::WGSLLanguageFeatureName feature) const;
    wgpu::Status APIGetWGSLLanguageFeatures(SupportedWGSLLanguageFeatures* features) const;

    void DisconnectDawnPlatform();

  private:
    explicit InstanceBase(const TogglesState& instanceToggles);
    ~InstanceBase() override;

    void DeleteThis() override;
    void WillDropLastExternalRef() override;

    InstanceBase(const InstanceBase& other) = delete;
    InstanceBase& operator=(const InstanceBase& other) = delete;

    MaybeError Initialize(const UnpackedPtr<InstanceDescriptor>& descriptor);
    void SetPlatform(dawn::platform::Platform* platform);

    // Lazily creates connections to all backends that have been compiled, may return null even for
    // compiled in backends.
    BackendConnection* GetBackendConnection(wgpu::BackendType backendType);

    // Enumerate physical devices according to options and return them.
    std::vector<Ref<PhysicalDeviceBase>> EnumeratePhysicalDevices(
        const UnpackedPtr<RequestAdapterOptions>& options);

    // Helper function that create adapter on given physical device handling required adapter
    // toggles descriptor.
    Ref<AdapterBase> CreateAdapter(Ref<PhysicalDeviceBase> physicalDevice,
                                   wgpu::FeatureLevel featureLevel,
                                   const DawnTogglesDescriptor* requiredAdapterToggles,
                                   wgpu::PowerPreference powerPreference);

    void GatherWGSLFeatures(const DawnWGSLBlocklist* wgslBlocklist);

    // ErrorSink implementation
    void ConsumeError(std::unique_ptr<ErrorData> error,
                      InternalErrorType additionalAllowedErrors = InternalErrorType::None) override;

    absl::flat_hash_set<std::string> mWarningMessages;

    std::vector<std::string> mRuntimeSearchPaths;

    bool mBeginCaptureOnStartup = false;
    BackendValidationLevel mBackendValidationLevel = BackendValidationLevel::Disabled;

    WGPULoggingCallbackInfo mLoggingCallbackInfo = {};

    std::unique_ptr<dawn::platform::Platform> mDefaultPlatform;
    raw_ptr<dawn::platform::Platform> mPlatform = nullptr;

    BackendsArray mBackends;
    BackendsBitset mBackendsTried;

    TogglesState mToggles;
    TogglesInfo mTogglesInfo;

    absl::flat_hash_set<wgpu::WGSLLanguageFeatureName> mWGSLFeatures;
    absl::flat_hash_set<tint::wgsl::LanguageFeature> mTintLanguageFeatures;

#if defined(DAWN_USE_X11)
    std::unique_ptr<X11Functions> mX11Functions;
#endif  // defined(DAWN_USE_X11)

    Ref<CallbackTaskManager> mCallbackTaskManager;
    EventManager mEventManager;

    struct DeprecationWarnings;
    std::unique_ptr<DeprecationWarnings> mDeprecationWarnings;

    MutexProtected<absl::flat_hash_set<DeviceBase*>> mDevicesList;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_INSTANCE_H_
