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

#include "dawn/native/vulkan/BackendVk.h"

#include <algorithm>
#include <string>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/VulkanBackend.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

// TODO(crbug.com/dawn/283): Link against the Vulkan Loader and remove this.
#if defined(DAWN_ENABLE_SWIFTSHADER)
#if DAWN_PLATFORM_IS(LINUX) || DAWN_PLATFORM_IS(FUCHSIA)
constexpr char kSwiftshaderLibName[] = "libvk_swiftshader.so";
#elif DAWN_PLATFORM_IS(WINDOWS)
constexpr char kSwiftshaderLibName[] = "vk_swiftshader.dll";
#elif DAWN_PLATFORM_IS(MACOS)
constexpr char kSwiftshaderLibName[] = "libvk_swiftshader.dylib";
#else
#error "Unimplemented Swiftshader Vulkan backend platform"
#endif
#endif

#if DAWN_PLATFORM_IS(LINUX)
#if DAWN_PLATFORM_IS(ANDROID)
constexpr char kVulkanLibName[] = "libvulkan.so";
#else
constexpr char kVulkanLibName[] = "libvulkan.so.1";
#endif
#elif DAWN_PLATFORM_IS(WINDOWS)
constexpr char kVulkanLibName[] = "vulkan-1.dll";
#elif DAWN_PLATFORM_IS(MACOS)
constexpr char kVulkanLibName[] = "libvulkan.dylib";
#elif DAWN_PLATFORM_IS(FUCHSIA)
constexpr char kVulkanLibName[] = "libvulkan.so";
#else
#error "Unimplemented Vulkan backend platform"
#endif

struct SkippedMessage {
    const char* messageId;
    const char* messageContents;
};

// Array of Validation error/warning messages that will be ignored, should include bugID
constexpr SkippedMessage kSkippedMessages[] = {
    // These errors are generated when simultaneously using a read-only depth/stencil attachment as
    // a texture binding. This is valid Vulkan.
    //
    // When storeOp=NONE is not present, Dawn uses storeOp=STORE, but Vulkan validation layer
    // considers the image read-only and produces a hazard. Dawn can't rely on storeOp=NONE and
    // so this is not expected to be worked around.
    // See http://crbug.com/dawn/1225 for more details.
    //
    {"SYNC-HAZARD-READ-AFTER-WRITE",
     "Access info (usage: SYNC_FRAGMENT_SHADER_SHADER_SAMPLED_READ, prior_usage: "
     "SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE"},
    // Depth used as storage
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "depth aspect during store with storeOp VK_ATTACHMENT_STORE_OP_STORE. Access info (usage: "
     "SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_STORAGE_READ, read_barriers: VkPipelineStageFlags2KHR(0)"},
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "depth aspect during store with storeOp VK_ATTACHMENT_STORE_OP_STORE. Access info (usage: "
     "SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_STORAGE_READ, read_barriers: VkPipelineStageFlags2(0)"},
    // Depth used in sampling
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "depth aspect during store with storeOp VK_ATTACHMENT_STORE_OP_STORE. Access info (usage: "
     "SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_SAMPLED_READ, read_barriers: VkPipelineStageFlags2KHR(0)"},
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "depth aspect during store with storeOp VK_ATTACHMENT_STORE_OP_STORE. Access info (usage: "
     "SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_SAMPLED_READ, read_barriers: VkPipelineStageFlags2(0)"},
    // Stencil used as storage
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "stencil aspect during store with stencilStoreOp VK_ATTACHMENT_STORE_OP_STORE. Access info "
     "(usage: SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_STORAGE_READ, read_barriers: VkPipelineStageFlags2KHR(0)"},
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "stencil aspect during store with stencilStoreOp VK_ATTACHMENT_STORE_OP_STORE. Access info "
     "(usage: SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_STORAGE_READ, read_barriers: VkPipelineStageFlags2(0)"},
    // Stencil used in sampling (note no tests actually hit this)
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "stencil aspect during store with stencilStoreOp VK_ATTACHMENT_STORE_OP_STORE. Access info "
     "(usage: SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_SAMPLED_READ, read_barriers: VkPipelineStageFlags2KHR(0)"},
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "stencil aspect during store with stencilStoreOp VK_ATTACHMENT_STORE_OP_STORE. Access info "
     "(usage: SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_SAMPLED_READ, read_barriers: VkPipelineStageFlags2(0)"},
    // http://crbug.com/360147114
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "Submitted access info (submitted_usage: SYNC_CLEAR_TRANSFER_WRITE, command: vkCmdFillBuffer"},

    // https://issues.chromium.org/issues/41479545
    {"SYNC-HAZARD-WRITE-AFTER-WRITE",
     "Access info (usage: SYNC_ACCESS_INDEX_NONE, prior_usage: SYNC_CLEAR_TRANSFER_WRITE, "
     "write_barriers: "
     "SYNC_VERTEX_SHADER_SHADER_BINDING_TABLE_READ|SYNC_VERTEX_SHADER_SHADER_SAMPLED_READ|SYNC_"
     "VERTEX_SHADER_SHADER_STORAGE_READ|SYNC_FRAGMENT_SHADER_SHADER_BINDING_TABLE_READ|SYNC_"
     "FRAGMENT_SHADER_SHADER_SAMPLED_READ|SYNC_FRAGMENT_SHADER_SHADER_STORAGE_READ|SYNC_COMPUTE_"
     "SHADER_SHADER_BINDING_TABLE_READ|SYNC_COMPUTE_SHADER_SHADER_SAMPLED_READ|SYNC_COMPUTE_SHADER_"
     "SHADER_STORAGE_READ, command: vkCmdFillBuffer"},

    // http://anglebug.com/7513
    {"VUID-VkGraphicsPipelineCreateInfo-pStages-06896",
     "contains fragment shader state, but stages"},

    // A warning that's generated on valid usage of the WebGPU API where a fragment output doesn't
    // have a corresponding attachment
    {"UNASSIGNED-CoreValidation-Shader-OutputNotConsumed",
     "fragment shader writes to output location 0 with no matching attachment"},

    // There are various VVL (and other) errors in dawn::native::vulkan::ExternalImage*. Suppress
    // them for now as everything *should* be fixed by using SharedTextureMemory in the future.
    // http://crbug.com/1499919
    {"VUID-VkMemoryAllocateInfo-allocationSize-01742",
     "vkAllocateMemory(): pAllocateInfo->allocationSize allocationSize (4096) "
     "does not match pAllocateInfo->pNext<VkImportMemoryFdInfoKHR>"},
    {"VUID-VkMemoryAllocateInfo-allocationSize-01742",
     "vkAllocateMemory(): pAllocateInfo->allocationSize allocationSize (512) "
     "does not match pAllocateInfo->pNext<VkImportMemoryFdInfoKHR>"},
    {"VUID-VkMemoryAllocateInfo-allocationSize-01742",
     "vkAllocateMemory(): pAllocateInfo->memoryTypeIndex memoryTypeIndex (7) "
     "does not match pAllocateInfo->pNext<VkImportMemoryFdInfoKHR>"},
    {"VUID-VkMemoryDedicatedAllocateInfo-image-01878",
     "vkAllocateMemory(): pAllocateInfo->pNext<VkMemoryDedicatedAllocateInfo>"},
    // crbug.com/324282958
    {"NVIDIA", "vkBindImageMemory: memoryTypeIndex"},
};

namespace dawn::native::vulkan {

namespace {

static constexpr ICD kICDs[] = {
// Other drivers should not be loaded with MSAN because they don't have MSAN instrumentation.
// MSAN will produce false positives since it cannot detect changes to memory that the driver
// has made.
#if !defined(MEMORY_SANITIZER)
    ICD::None,
#endif
#if defined(DAWN_ENABLE_SWIFTSHADER)
    ICD::SwiftShader,
#endif  // defined(DAWN_ENABLE_SWIFTSHADER)
};

// Suppress validation errors that are known. Returns false in that case.
bool ShouldReportDebugMessage(const char* messageId, const char* message) {
    // If a driver gives us a NULL pMessage (which would be a violation of the Vulkan spec)
    // then ignore this message.
    if (message == nullptr) {
        return false;
    }

    // Some Vulkan drivers send "error" messages of "VK_SUCCESS" when zero devices are
    // available; seen in crbug.com/1464122. This is not a real error that we care about.
    // The messageId is ignored because drivers may report
    // __FILE__: __LINE__ info here.
    // https://github.com/Mesa3D/mesa/blob/22.2/src/amd/vulkan/radv_device.c#L1201
    if (strcmp(message, "VK_SUCCESS") == 0) {
        return false;
    }

    // The Vulkan spec does allow pMessageIdName to be NULL, but it may still contain a valid
    // message. Since we can't compare it with our skipped message list allow it through.
    if (messageId == nullptr) {
        return true;
    }

    for (const SkippedMessage& msg : kSkippedMessages) {
        if (strstr(messageId, msg.messageId) != nullptr &&
            strstr(message, msg.messageContents) != nullptr) {
            return false;
        }
    }
    return true;
}

void LogCallbackData(LogSeverity severity,
                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    LogMessage log = LogMessage(severity);

    // pMessageIdName may be NULL, according to the Vulkan spec. Passing NULL into an ostream is
    // undefined behavior, so we'll handle that scenario separately.
    if (pCallbackData->pMessageIdName != nullptr) {
        log << pCallbackData->pMessageIdName;
    } else {
        log << "nullptr";
    }

    log << ": " << pCallbackData->pMessage;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
OnDebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                     VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                     void* pUserData) {
    if (!ShouldReportDebugMessage(pCallbackData->pMessageIdName, pCallbackData->pMessage)) {
        return VK_FALSE;
    }

    if (!(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)) {
        LogCallbackData(LogSeverity::Warning, pCallbackData);
        return VK_FALSE;
    }

    if (pUserData == nullptr) {
        return VK_FALSE;
    }
    VulkanInstance* instance = reinterpret_cast<VulkanInstance*>(pUserData);

    // Look through all the object labels attached to the debug message and try to parse
    // a device debug prefix out of one of them. If a debug prefix is found and matches
    // a registered device, forward the message on to it.
    for (uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
        const VkDebugUtilsObjectNameInfoEXT& object = pCallbackData->pObjects[i];
        std::string deviceDebugPrefix = GetDeviceDebugPrefixFromDebugName(object.pObjectName);
        if (deviceDebugPrefix.empty()) {
            continue;
        }

        if (instance->HandleDeviceMessage(std::move(deviceDebugPrefix), pCallbackData->pMessage)) {
            return VK_FALSE;
        }
    }

    // We get to this line if no device was associated with the message. If the message is a backend
    // validation error then crash as there should have been a debug label on the object. The
    // driver can also produce errors even with backend validation disabled so those errors are
    // just logged.
    LogCallbackData(LogSeverity::Error, pCallbackData);
    DAWN_ASSERT(!(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT));

    return VK_FALSE;
}

// A debug callback specifically for instance creation so that we don't fire an DAWN_ASSERT when
// the instance fails creation in an expected manner (for example the system not having
// Vulkan drivers).
VKAPI_ATTR VkBool32 VKAPI_CALL
OnInstanceCreationDebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                     VkDebugUtilsMessageTypeFlagsEXT /* messageTypes */,
                                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                     void* /* pUserData */) {
    dawn::WarningLog() << pCallbackData->pMessage;
    return VK_FALSE;
}

}  // anonymous namespace

VulkanInstance::VulkanInstance() = default;

VulkanInstance::~VulkanInstance() {
    DAWN_ASSERT(mMessageListenerDevices.empty());

    if (mDebugUtilsMessenger != VK_NULL_HANDLE) {
        mFunctions.DestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessenger, nullptr);
        mDebugUtilsMessenger = VK_NULL_HANDLE;
    }

    // VkPhysicalDevices are destroyed when the VkInstance is destroyed
    if (mInstance != VK_NULL_HANDLE) {
        mFunctions.DestroyInstance(mInstance, nullptr);
        mInstance = VK_NULL_HANDLE;
    }
}

const VulkanFunctions& VulkanInstance::GetFunctions() const {
    return mFunctions;
}

VkInstance VulkanInstance::GetVkInstance() const {
    return mInstance;
}

const VulkanGlobalInfo& VulkanInstance::GetGlobalInfo() const {
    return mGlobalInfo;
}

const std::vector<VkPhysicalDevice>& VulkanInstance::GetVkPhysicalDevices() const {
    return mVkPhysicalDevices;
}

// static
ResultOrError<Ref<VulkanInstance>> VulkanInstance::Create(const InstanceBase* instance, ICD icd) {
    Ref<VulkanInstance> vulkanInstance = AcquireRef(new VulkanInstance());
    DAWN_TRY(vulkanInstance->Initialize(instance, icd));
    return std::move(vulkanInstance);
}

MaybeError VulkanInstance::Initialize(const InstanceBase* instance, ICD icd) {
    // These environment variables need only be set while loading procs and gathering device
    // info.
    ScopedEnvironmentVar vkICDFilenames;
    ScopedEnvironmentVar vkLayerPath;

    const std::vector<std::string>& searchPaths = instance->GetRuntimeSearchPaths();

    auto LoadVulkan = [&](const char* libName) -> MaybeError {
        std::string error;
        if (mVulkanLib.Open(libName, searchPaths, &error)) {
            return {};
        }
        return DAWN_FORMAT_INTERNAL_ERROR("Couldn't load Vulkan: %s", error.c_str());
    };

    switch (icd) {
        case ICD::None: {
            DAWN_TRY(LoadVulkan(kVulkanLibName));
            // Succesfully loaded driver; break.
            break;
        }
        case ICD::SwiftShader: {
#if defined(DAWN_ENABLE_SWIFTSHADER)
            DAWN_TRY(LoadVulkan(kSwiftshaderLibName));
            break;
#endif  // defined(DAWN_ENABLE_SWIFTSHADER)
        // ICD::SwiftShader should not be passed if SwiftShader is not enabled.
            DAWN_UNREACHABLE();
        }
    }

    if (instance->IsBackendValidationEnabled()) {
#if defined(DAWN_ENABLE_VULKAN_VALIDATION_LAYERS)
        auto execDir = GetExecutableDirectory();
        std::string vkDataDir = execDir.value_or("") + DAWN_VK_DATA_DIR;
        if (!vkLayerPath.Set("VK_LAYER_PATH", vkDataDir.c_str())) {
            return DAWN_INTERNAL_ERROR("Couldn't set VK_LAYER_PATH");
        }
#else
        dawn::WarningLog() << "Backend validation enabled but Dawn was not built with "
                              "DAWN_ENABLE_VULKAN_VALIDATION_LAYERS.";
#endif
    }

    DAWN_TRY(mFunctions.LoadGlobalProcs(mVulkanLib));

    DAWN_TRY_ASSIGN(mGlobalInfo, GatherGlobalInfo(mFunctions));
    if (icd != ICD::SwiftShader && mGlobalInfo.apiVersion < VK_MAKE_API_VERSION(0, 1, 1, 0)) {
        // See crbug.com/850881, crbug.com/863086, crbug.com/1465064, crbug.com/346990068
        return DAWN_INTERNAL_ERROR(
            "Vulkan 1.0 driver is unsupported. At least Vulkan 1.1 is required.");
    }

    VulkanGlobalKnobs usedGlobalKnobs = {};
    DAWN_TRY_ASSIGN(usedGlobalKnobs, CreateVkInstance(instance));
    *static_cast<VulkanGlobalKnobs*>(&mGlobalInfo) = usedGlobalKnobs;

    DAWN_TRY(mFunctions.LoadInstanceProcs(mInstance, mGlobalInfo));

    if (usedGlobalKnobs.HasExt(InstanceExt::DebugUtils)) {
        DAWN_TRY(RegisterDebugUtils());
    }

    DAWN_TRY_ASSIGN(mVkPhysicalDevices, GatherPhysicalDevices(mInstance, mFunctions));

    return {};
}

ResultOrError<VulkanGlobalKnobs> VulkanInstance::CreateVkInstance(const InstanceBase* instance) {
    VulkanGlobalKnobs usedKnobs = {};
    std::vector<const char*> layerNames;
    InstanceExtSet extensionsToRequest = mGlobalInfo.extensions;

    auto UseLayerIfAvailable = [&](VulkanLayer layer) {
        if (mGlobalInfo.layers[layer]) {
            layerNames.push_back(GetVulkanLayerInfo(layer).name);
            usedKnobs.layers.set(layer, true);
            extensionsToRequest |= mGlobalInfo.layerExtensions[layer];
        }
    };

    // vktrace works by instering a layer, but we hide it behind a macro because the vktrace
    // layer crashes when used without vktrace server started. See this vktrace issue:
    // https://github.com/LunarG/VulkanTools/issues/254
    // Also it is good to put it in first position so that it doesn't see Vulkan calls inserted
    // by other layers.
#if defined(DAWN_USE_VKTRACE)
    UseLayerIfAvailable(VulkanLayer::LunargVkTrace);
#endif
    // RenderDoc installs a layer at the system level for its capture but we don't want to use
    // it unless we are debugging in RenderDoc so we hide it behind a macro.
#if defined(DAWN_USE_RENDERDOC)
    UseLayerIfAvailable(VulkanLayer::RenderDocCapture);
#endif

    if (instance->IsBackendValidationEnabled()) {
        UseLayerIfAvailable(VulkanLayer::Validation);
    }

    // Always use the Fuchsia swapchain layer if available.
    UseLayerIfAvailable(VulkanLayer::FuchsiaImagePipeSwapchain);

    // Available and known instance extensions default to being requested, but some special
    // cases are removed.
    usedKnobs.extensions = extensionsToRequest;

    std::vector<const char*> extensionNames;
    for (InstanceExt ext : extensionsToRequest) {
        const InstanceExtInfo& info = GetInstanceExtInfo(ext);

        if (info.versionPromoted > mGlobalInfo.apiVersion) {
            extensionNames.push_back(info.name);
        }
    }

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = nullptr;
    appInfo.applicationVersion = 0;
    appInfo.pEngineName = "Dawn";
    appInfo.engineVersion = 0;
    appInfo.apiVersion = std::min(mGlobalInfo.apiVersion, VK_API_VERSION_1_3);

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
    createInfo.ppEnabledLayerNames = layerNames.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    VkDebugUtilsMessengerCreateInfoEXT utilsMessengerCreateInfo;
    VkValidationFeaturesEXT validationFeatures;
    PNextChainBuilder createInfoChain(&createInfo);

    // Register the debug callback for instance creation so we receive message for any errors
    // (validation or other).
    if (usedKnobs.HasExt(InstanceExt::DebugUtils)) {
        utilsMessengerCreateInfo.flags = 0;
        utilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        utilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        utilsMessengerCreateInfo.pfnUserCallback = OnInstanceCreationDebugUtilsCallback;
        utilsMessengerCreateInfo.pUserData = nullptr;

        createInfoChain.Add(&utilsMessengerCreateInfo,
                            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
    }

    // Try to turn on synchronization validation if the instance was created with backend
    // validation enabled.
    VkValidationFeatureEnableEXT kEnableSynchronizationValidation =
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
    if (instance->IsBackendValidationEnabled() &&
        usedKnobs.HasExt(InstanceExt::ValidationFeatures)) {
        validationFeatures.enabledValidationFeatureCount = 1;
        validationFeatures.pEnabledValidationFeatures = &kEnableSynchronizationValidation;
        validationFeatures.disabledValidationFeatureCount = 0;
        validationFeatures.pDisabledValidationFeatures = nullptr;

        createInfoChain.Add(&validationFeatures, VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT);
    }

    DAWN_TRY(CheckVkSuccess(mFunctions.CreateInstance(&createInfo, nullptr, &mInstance),
                            "vkCreateInstance"));
    DAWN_INVALID_IF(mInstance == VK_NULL_HANDLE, "Failed to create VkInstance");

    return usedKnobs;
}

MaybeError VulkanInstance::RegisterDebugUtils() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    createInfo.pfnUserCallback = OnDebugUtilsCallback;
    createInfo.pUserData = this;

    return CheckVkSuccess(mFunctions.CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr,
                                                                  &*mDebugUtilsMessenger),
                          "vkCreateDebugUtilsMessengerEXT");
}

void VulkanInstance::StartListeningForDeviceMessages(Device* device) {
    std::lock_guard<std::mutex> lock(mMessageListenerDevicesMutex);
    mMessageListenerDevices.emplace(device->GetDebugPrefix(), device);
}
void VulkanInstance::StopListeningForDeviceMessages(Device* device) {
    std::lock_guard<std::mutex> lock(mMessageListenerDevicesMutex);
    mMessageListenerDevices.erase(device->GetDebugPrefix());
}
bool VulkanInstance::HandleDeviceMessage(std::string deviceDebugPrefix, std::string message) {
    std::lock_guard<std::mutex> lock(mMessageListenerDevicesMutex);
    auto it = mMessageListenerDevices.find(deviceDebugPrefix);
    if (it != mMessageListenerDevices.end()) {
        it->second->OnDebugMessage(std::move(message));
        return true;
    }
    return false;
}

Backend::Backend(InstanceBase* instance) : BackendConnection(instance, wgpu::BackendType::Vulkan) {}

Backend::~Backend() = default;

std::vector<Ref<PhysicalDeviceBase>> Backend::DiscoverPhysicalDevices(
    const UnpackedPtr<RequestAdapterOptions>& options) {
    std::vector<Ref<PhysicalDeviceBase>> physicalDevices;
    InstanceBase* instance = GetInstance();
    for (ICD icd : kICDs) {
#if DAWN_PLATFORM_IS(MACOS)
        // On Mac, we don't expect non-Swiftshader Vulkan to be available.
        if (icd == ICD::None) {
            continue;
        }
#endif  // DAWN_PLATFORM_IS(MACOS)
        if (options->forceFallbackAdapter && icd != ICD::SwiftShader) {
            continue;
        }
        if (mPhysicalDevices[icd].empty()) {
            if (!mVulkanInstancesCreated[icd]) {
                mVulkanInstancesCreated.set(icd);

                [[maybe_unused]] bool hadError =
                    instance->ConsumedErrorAndWarnOnce([&]() -> MaybeError {
                        DAWN_TRY_ASSIGN(mVulkanInstances[icd],
                                        VulkanInstance::Create(instance, icd));
                        return {};
                    }());
            }

            if (mVulkanInstances[icd] == nullptr) {
                // Instance failed to initialize.
                continue;
            }

            const std::vector<VkPhysicalDevice>& vkPhysicalDevices =
                mVulkanInstances[icd]->GetVkPhysicalDevices();
            for (VkPhysicalDevice vkPhysicalDevice : vkPhysicalDevices) {
                Ref<PhysicalDevice> physicalDevice =
                    AcquireRef(new PhysicalDevice(mVulkanInstances[icd].Get(), vkPhysicalDevice));
                if (instance->ConsumedErrorAndWarnOnce(physicalDevice->Initialize())) {
                    continue;
                }
                // This loop can't filter adapters based on SupportsFeatureLevel() since the results
                // are cached for subsequent calls that might have a different feature level.
                mPhysicalDevices[icd].push_back(std::move(physicalDevice));
            }
        }
        for (auto& physicalDevice : mPhysicalDevices[icd]) {
            if (physicalDevice->SupportsFeatureLevel(options->featureLevel, instance)) {
                physicalDevices.push_back(physicalDevice);
            }
        }
    }
    return physicalDevices;
}

BackendConnection* Connect(InstanceBase* instance) {
    return new Backend(instance);
}

}  // namespace dawn::native::vulkan
