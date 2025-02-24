// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef VK_DEBUG_UTILS_MESSENGER_HPP_
#define VK_DEBUG_UTILS_MESSENGER_HPP_

#include "VkObject.hpp"

namespace vk {

class DeviceMemory;

class DebugUtilsMessenger : public Object<DebugUtilsMessenger, VkDebugUtilsMessengerEXT>
{
public:
	DebugUtilsMessenger(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, void *mem);
	void destroy(const VkAllocationCallbacks *pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo);

	void submitMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);

private:
	VkDebugUtilsMessageSeverityFlagsEXT messageSeverity = (VkDebugUtilsMessageSeverityFlagsEXT)0;
	VkDebugUtilsMessageTypeFlagsEXT messageType = (VkDebugUtilsMessageTypeFlagsEXT)0;
	PFN_vkDebugUtilsMessengerCallbackEXT pfnUserCallback = nullptr;
	void *pUserData = nullptr;
};

static inline DebugUtilsMessenger *Cast(VkDebugUtilsMessengerEXT object)
{
	return DebugUtilsMessenger::Cast(object);
}

}  // namespace vk

#endif  // VK_DEBUG_UTILS_MESSENGER_HPP_
