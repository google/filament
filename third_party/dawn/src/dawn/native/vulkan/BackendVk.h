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

#ifndef SRC_DAWN_NATIVE_VULKAN_BACKENDVK_H_
#define SRC_DAWN_NATIVE_VULKAN_BACKENDVK_H_

#include <mutex>
#include <string>
#include <vector>

#include "dawn/native/BackendConnection.h"

#include "absl/container/flat_hash_map.h"
#include "dawn/common/DynamicLib.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/ityp_array.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/VulkanFunctions.h"
#include "dawn/native/vulkan/VulkanInfo.h"

namespace dawn::native::vulkan {

enum class ICD {
    None,
    SwiftShader,
};
constexpr uint32_t kICDCount = 2u;

class Device;

// VulkanInstance holds the reference to the Vulkan library, the VkInstance, VkPhysicalDevices
// on that instance, Vulkan functions loaded from the library, and global information
// gathered from the instance. VkPhysicalDevices bound to the VkInstance are bound to the GPU
// and GPU driver, keeping them active. It is RefCounted so that (eventually) when all adapters
// on an instance are no longer in use, the instance is deleted. This can be particuarly useful
// when we create multiple instances to selectively discover ICDs (like only
// SwiftShader/iGPU/dGPU/eGPU), and only one physical device on one instance remains in use. We
// can delete the VkInstances that are not in use to avoid holding the discrete GPU active.
class VulkanInstance : public RefCounted {
  public:
    static ResultOrError<Ref<VulkanInstance>> Create(const InstanceBase* instance, ICD icd);
    ~VulkanInstance() override;

    const VulkanFunctions& GetFunctions() const;
    VkInstance GetVkInstance() const;
    const VulkanGlobalInfo& GetGlobalInfo() const;
    const std::vector<VkPhysicalDevice>& GetVkPhysicalDevices() const;

    // TODO(dawn:831): This set of functions guards may need to be adjusted when Dawn is updated
    // to support multithreading.
    void StartListeningForDeviceMessages(Device* device);
    void StopListeningForDeviceMessages(Device* device);
    bool HandleDeviceMessage(std::string deviceDebugPrefix, std::string message);

  private:
    VulkanInstance();

    MaybeError Initialize(const InstanceBase* instance, ICD icd);
    ResultOrError<VulkanGlobalKnobs> CreateVkInstance(const InstanceBase* instance);

    MaybeError RegisterDebugUtils();

    DynamicLib mVulkanLib;
    VulkanGlobalInfo mGlobalInfo = {};
    VkInstance mInstance = VK_NULL_HANDLE;
    VulkanFunctions mFunctions;

    VkDebugUtilsMessengerEXT mDebugUtilsMessenger = VK_NULL_HANDLE;

    std::vector<VkPhysicalDevice> mVkPhysicalDevices;

    // Devices keep the VulkanInstance alive, so as long as devices remove themselves from this
    // map on destruction the pointers it contains should remain valid.
    absl::flat_hash_map<std::string, Device*> mMessageListenerDevices;
    std::mutex mMessageListenerDevicesMutex;
};

class Backend : public BackendConnection {
  public:
    explicit Backend(InstanceBase* instance);
    ~Backend() override;

    MaybeError Initialize();

    std::vector<Ref<PhysicalDeviceBase>> DiscoverPhysicalDevices(
        const UnpackedPtr<RequestAdapterOptions>& options) override;

  private:
    ityp::bitset<ICD, kICDCount> mVulkanInstancesCreated = {};
    ityp::array<ICD, Ref<VulkanInstance>, kICDCount> mVulkanInstances = {};
    ityp::array<ICD, std::vector<Ref<PhysicalDevice>>, kICDCount> mPhysicalDevices = {};
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_BACKENDVK_H_
