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

#include <webgpu/webgpu.h>

#include <algorithm>
#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/NullBackend.h"
#include "dawn/tests/PartitionAllocSupport.h"
#include "dawn/tests/StringViewMatchers.h"
#include "dawn/tests/ToggleParser.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WireHelper.h"
#include "dawn/webgpu_cpp_print.h"

namespace {

bool gUseWire = false;
// NOLINTNEXTLINE(runtime/string)
std::string gWireTraceDir = "";
std::unique_ptr<ToggleParser> gToggleParser = nullptr;
static ValidationTest* gCurrentTest = nullptr;

}  // namespace

void InitDawnValidationTestEnvironment(int argc, char** argv) {
    dawn::InitializePartitionAllocForTesting();
    dawn::InitializeDanglingPointerDetectorForTesting();

    gToggleParser = std::make_unique<ToggleParser>();

    for (int i = 1; i < argc; ++i) {
        if (strcmp("-w", argv[i]) == 0 || strcmp("--use-wire", argv[i]) == 0) {
            gUseWire = true;
            continue;
        }

        constexpr const char kWireTraceDirArg[] = "--wire-trace-dir=";
        size_t argLen = sizeof(kWireTraceDirArg) - 1;
        if (strncmp(argv[i], kWireTraceDirArg, argLen) == 0) {
            gWireTraceDir = argv[i] + argLen;
            continue;
        }

        if (gToggleParser->ParseEnabledToggles(argv[i])) {
            continue;
        }

        if (gToggleParser->ParseDisabledToggles(argv[i])) {
            continue;
        }

        if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
            dawn::InfoLog()
                << "\n\nUsage: " << argv[0]
                << " [GTEST_FLAGS...] [-w]\n"
                   "    [--enable-toggles=toggles] [--disable-toggles=toggles]\n"
                   "  -w, --use-wire: Run the tests through the wire (defaults to no wire)\n"
                   "  --enable-toggles: Comma-delimited list of Dawn toggles to enable.\n"
                   "    ex.) skip_validation,disable_robustness,turn_off_vsync\n"
                   "  --disable-toggles: Comma-delimited list of Dawn toggles to disable\n";
            continue;
        }

        // Skip over args that look like they're for Googletest.
        constexpr const char kGtestArgPrefix[] = "--gtest_";
        if (strncmp(kGtestArgPrefix, argv[i], sizeof(kGtestArgPrefix) - 1) == 0) {
            continue;
        }

        dawn::WarningLog() << " Unused argument: " << argv[i];
    }
}

ValidationTest::ValidationTest() {
    gCurrentTest = this;

    DawnProcTable procs = dawn::native::GetProcs();

    // Forward to dawn::native instanceRequestAdapter, but save the returned adapter in
    // gCurrentTest->mBackendAdapter.
    procs.instanceRequestAdapter = [](WGPUInstance self, const WGPURequestAdapterOptions* options,
                                      WGPURequestAdapterCallbackInfo callbackInfo) -> WGPUFuture {
        DAWN_ASSERT(gCurrentTest);
        DAWN_ASSERT(callbackInfo.mode == WGPUCallbackMode_AllowSpontaneous);

        return dawn::native::GetProcs().instanceRequestAdapter(
            self, options,
            {nullptr, WGPUCallbackMode_AllowSpontaneous,
             [](WGPURequestAdapterStatus status, WGPUAdapter cAdapter, WGPUStringView message,
                void* userdata, void*) {
                 gCurrentTest->mBackendAdapter = dawn::native::FromAPI(cAdapter);

                 auto* info = static_cast<WGPURequestAdapterCallbackInfo*>(userdata);
                 info->callback(status, cAdapter, message, info->userdata1, info->userdata2);
                 delete info;
             },
             new WGPURequestAdapterCallbackInfo(callbackInfo), nullptr});
    };

    // Forward to dawn::native instanceRequestAdapter, but save the returned backend device in
    // gCurrentTest->mLastCreatedBackendDevice.
    procs.adapterRequestDevice = [](WGPUAdapter self, const WGPUDeviceDescriptor* descriptor,
                                    WGPURequestDeviceCallbackInfo callbackInfo) -> WGPUFuture {
        DAWN_ASSERT(gCurrentTest);
        DAWN_ASSERT(callbackInfo.mode == WGPUCallbackMode_AllowSpontaneous);

        wgpu::DeviceDescriptor deviceDesc = {};
        if (descriptor != nullptr) {
            deviceDesc = *(reinterpret_cast<const wgpu::DeviceDescriptor*>(descriptor));
        }

        // Set the toggles for the device. We start with all test specific toggles, then toggle
        // flags so that toggle flags will always take precedence. Note that disabling toggles also
        // take precedence.
        wgpu::DawnTogglesDescriptor deviceTogglesDesc;
        deviceTogglesDesc.nextInChain = deviceDesc.nextInChain;
        deviceDesc.nextInChain = &deviceTogglesDesc;

        auto enabledToggles = gCurrentTest->GetEnabledToggles();
        auto disabledToggles = gCurrentTest->GetDisabledToggles();
        for (const std::string& toggle : gToggleParser->GetEnabledToggles()) {
            enabledToggles.push_back(toggle.c_str());
        }
        for (const std::string& toggle : gToggleParser->GetDisabledToggles()) {
            disabledToggles.push_back(toggle.c_str());
        }
        deviceTogglesDesc.enabledToggles = enabledToggles.data();
        deviceTogglesDesc.enabledToggleCount = enabledToggles.size();
        deviceTogglesDesc.disabledToggles = disabledToggles.data();
        deviceTogglesDesc.disabledToggleCount = disabledToggles.size();

        return dawn::native::GetProcs().adapterRequestDevice(
            self, reinterpret_cast<WGPUDeviceDescriptor*>(&deviceDesc),
            {nullptr, WGPUCallbackMode_AllowSpontaneous,
             [](WGPURequestDeviceStatus status, WGPUDevice cDevice, WGPUStringView message,
                void* userdata, void*) {
                 gCurrentTest->mLastCreatedBackendDevice = cDevice;

                 auto* info = static_cast<WGPURequestDeviceCallbackInfo*>(userdata);
                 info->callback(status, cDevice, message, info->userdata1, info->userdata2);
                 delete info;
             },
             new WGPURequestDeviceCallbackInfo(callbackInfo), nullptr});
    };

    mWireHelper = dawn::utils::CreateWireHelper(procs, gUseWire, gWireTraceDir.c_str());
}

void ValidationTest::SetUp() {
    // By default create the instance with toggle AllowUnsafeAPIs enabled, which would be inherited
    // to adapter and device toggles and allow us to test unsafe apis (including experimental
    // features). To test device with AllowUnsafeAPIs disabled, require it in device toggles
    // descriptor to override the inheritance. Alternatively, override AllowUnsafeAPIs() if
    // querying for features via the adapter, i.e. prior to device creation.
    wgpu::DawnTogglesDescriptor instanceToggles = {};
    if (AllowUnsafeAPIs()) {
        static const char* allowUnsafeApisToggle = "allow_unsafe_apis";
        instanceToggles.enabledToggleCount = 1;
        instanceToggles.enabledToggles = &allowUnsafeApisToggle;
    }

    wgpu::InstanceDescriptor instanceDesc = {};
    instanceDesc.nextInChain = &instanceToggles;
    static constexpr auto kRequiredFeatures =
        std::array{wgpu::InstanceFeatureName::MultipleDevicesPerAdapter,
                   wgpu::InstanceFeatureName::ShaderSourceSPIRV};
    instanceDesc.requiredFeatureCount = kRequiredFeatures.size();
    instanceDesc.requiredFeatures = kRequiredFeatures.data();

    SetUp(&instanceDesc);
}

ValidationTest::~ValidationTest() {
    // We need to destroy Dawn objects before the wire helper which sets procs to null otherwise the
    // dawn*Release will call a nullptr
    device = nullptr;
    adapter = nullptr;
    instance = nullptr;
    mWireHelper.reset();

    // Check that all devices were destructed.
    // Note that if the test is skipped before SetUp is called, mDawnInstance will not get set and
    // remain nullptr.
    if (mDawnInstance) {
        EXPECT_EQ(mDawnInstance->GetDeviceCountForTesting(), 0u);
    }

    gCurrentTest = nullptr;
}

void ValidationTest::TearDown() {
    FlushWire();
    ASSERT_FALSE(mExpectError);

    // Note that if the test is skipped before SetUp is called, mDawnInstance will not get set and
    // remain nullptr.
    if (mDawnInstance) {
        EXPECT_EQ(mLastWarningCount, mDawnInstance->GetDeprecationWarningCountForTesting());
    }

    // The device will be destroyed soon after, so we want to set the expectation.
    ExpectDeviceDestruction();
}

void ValidationTest::StartExpectDeviceError(testing::Matcher<std::string> errorMatcher) {
    mExpectError = true;
    mError = false;
    mErrorMatcher = errorMatcher;
}

void ValidationTest::StartExpectDeviceError() {
    StartExpectDeviceError(testing::_);
}

bool ValidationTest::EndExpectDeviceError() {
    mExpectError = false;
    mErrorMatcher = testing::_;
    return mError;
}
std::string ValidationTest::GetLastDeviceErrorMessage() const {
    return mDeviceErrorMessage;
}

void ValidationTest::ExpectDeviceDestruction() {
    mExpectDestruction = true;
}

bool ValidationTest::UsesWire() const {
    return gUseWire;
}

void ValidationTest::FlushWire() {
    EXPECT_TRUE(mWireHelper->FlushClient());
    EXPECT_TRUE(mWireHelper->FlushServer());
}

void ValidationTest::WaitForAllOperations() {
    do {
        FlushWire();
        if (UsesWire()) {
            instance.ProcessEvents();
        }
    } while (dawn::native::InstanceProcessEvents(mDawnInstance->Get()) || !mWireHelper->IsIdle());
}

const dawn::native::ToggleInfo* ValidationTest::GetToggleInfo(const char* name) const {
    return mDawnInstance->GetToggleInfo(name);
}

bool ValidationTest::HasToggleEnabled(const char* toggle) const {
    auto toggles = dawn::native::GetTogglesUsed(backendDevice);
    return std::find_if(toggles.begin(), toggles.end(), [toggle](const char* name) {
               return strcmp(toggle, name) == 0;
           }) != toggles.end();
}

const dawn::utils::ComboLimits& ValidationTest::GetSupportedLimits() const {
    return deviceLimits;
}

bool ValidationTest::AllowUnsafeAPIs() {
    return true;
}

std::vector<wgpu::FeatureName> ValidationTest::GetRequiredFeatures() {
    return {};
}

void ValidationTest::GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                                       dawn::utils::ComboLimits& required) {}

std::vector<const char*> ValidationTest::GetEnabledToggles() {
    return {};
}

std::vector<const char*> ValidationTest::GetDisabledToggles() {
    return {};
}

dawn::utils::WireHelper* ValidationTest::GetWireHelper() const {
    return mWireHelper.get();
}

uint64_t ValidationTest::GetInstanceDeprecationCountForTesting() {
    return mDawnInstance->GetDeprecationWarningCountForTesting();
}

uint32_t ValidationTest::GetDeviceCreationDeprecationWarningExpectation(
    const wgpu::DeviceDescriptor& descriptor) {
    uint32_t expectedDeprecatedCount = 0;

    std::unordered_set<wgpu::FeatureName> requiredFeatureSet;
    for (uint32_t i = 0; i < descriptor.requiredFeatureCount; ++i) {
        requiredFeatureSet.insert(descriptor.requiredFeatures[i]);
    }

    return expectedDeprecatedCount;
}

wgpu::Device ValidationTest::RequestDeviceSync(const wgpu::DeviceDescriptor& deviceDesc) {
    DAWN_ASSERT(adapter);

    wgpu::Device apiDevice;
    EXPECT_DEPRECATION_WARNINGS(
        adapter.RequestDevice(&deviceDesc, wgpu::CallbackMode::AllowSpontaneous,
                              [&apiDevice](wgpu::RequestDeviceStatus status, wgpu::Device result,
                                           wgpu::StringView message) {
                                  if (status != wgpu::RequestDeviceStatus::Success) {
                                      ADD_FAILURE() << "Unable to create device: " << message;
                                      DAWN_ASSERT(false);
                                  }
                                  apiDevice = std::move(result);
                              }),
        GetDeviceCreationDeprecationWarningExpectation(deviceDesc));

    DAWN_ASSERT(apiDevice);
    return apiDevice;
}

dawn::native::Adapter& ValidationTest::GetBackendAdapter() {
    return mBackendAdapter;
}

void ValidationTest::SetUp(const wgpu::InstanceDescriptor* nativeDesc,
                           const wgpu::InstanceDescriptor* wireDesc) {
    std::string traceName =
        std::string(::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name()) +
        "_" + ::testing::UnitTest::GetInstance()->current_test_info()->name();
    mWireHelper->BeginWireTrace(traceName.c_str());

    // Initialize the instances.
    std::tie(instance, mDawnInstance) = mWireHelper->CreateInstances(nativeDesc, wireDesc);

    // Initialize the adapter.
    wgpu::RequestAdapterOptions options = {};
    options.backendType = wgpu::BackendType::Null;
    options.featureLevel = gCurrentTest->UseCompatibilityMode() ? wgpu::FeatureLevel::Compatibility
                                                                : wgpu::FeatureLevel::Core;
    instance.RequestAdapter(&options, wgpu::CallbackMode::AllowSpontaneous,
                            [this](wgpu::RequestAdapterStatus, wgpu::Adapter result,
                                   wgpu::StringView) -> void { adapter = std::move(result); });

    FlushWire();
    DAWN_ASSERT(adapter);

    // Initialize the device.
    wgpu::DeviceDescriptor deviceDescriptor = {};
    deviceDescriptor.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [this](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
            if (mExpectDestruction) {
                EXPECT_EQ(reason, wgpu::DeviceLostReason::Destroyed);
                return;
            }
            ADD_FAILURE() << "Device lost during test: " << message;
            DAWN_ASSERT(false);
        });
    deviceDescriptor.SetUncapturedErrorCallback(
        [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message,
           ValidationTest* self) {
            DAWN_ASSERT(type != wgpu::ErrorType::NoError);

            ASSERT_TRUE(self->mExpectError) << "Got unexpected device error: " << message;
            ASSERT_FALSE(self->mError) << "Got two errors in expect block, first one is:\n"  //
                                       << self->mDeviceErrorMessage                          //
                                       << "\nsecond one is:\n"                               //
                                       << message;

            self->mDeviceErrorMessage = message;
            if (self->mExpectError) {
                ASSERT_THAT(message, testing::SizedStringMatches(self->mErrorMatcher));
            }
            self->mError = true;
        },
        this);

    // Set the required features for the device.
    auto requiredFeatures = GetRequiredFeatures();
    deviceDescriptor.requiredFeatures = requiredFeatures.data();
    deviceDescriptor.requiredFeatureCount = requiredFeatures.size();

    dawn::utils::ComboLimits supportedLimits;
    dawn::native::GetProcs().adapterGetLimits(
        mBackendAdapter.Get(), reinterpret_cast<WGPULimits*>(supportedLimits.GetLinked()));
    dawn::utils::ComboLimits requiredLimits{};
    GetRequiredLimits(supportedLimits, requiredLimits);
    deviceDescriptor.requiredLimits = requiredLimits.GetLinked();

    device = RequestDeviceSync(deviceDescriptor);
    DAWN_ASSERT(device);
    device.GetLimits(deviceLimits.GetLinked());

    // We only want to set the backendDevice when the device was created via the test setup.
    backendDevice = mLastCreatedBackendDevice;
}

bool ValidationTest::UseCompatibilityMode() const {
    return false;
}

ValidationTest::PlaceholderRenderPass::PlaceholderRenderPass(const wgpu::Device& device)
    : attachmentFormat(wgpu::TextureFormat::RGBA8Unorm), width(400), height(400) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = attachmentFormat;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    attachment = device.CreateTexture(&descriptor);

    wgpu::TextureView view = attachment.CreateView();
    mColorAttachment.view = view;
    mColorAttachment.resolveTarget = nullptr;
    mColorAttachment.clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
    mColorAttachment.loadOp = wgpu::LoadOp::Clear;
    mColorAttachment.storeOp = wgpu::StoreOp::Store;

    colorAttachmentCount = 1;
    colorAttachments = &mColorAttachment;
    depthStencilAttachment = nullptr;
}
