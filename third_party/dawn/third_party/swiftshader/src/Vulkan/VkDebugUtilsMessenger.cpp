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

#include "VkDebugUtilsMessenger.hpp"

namespace vk {

DebugUtilsMessenger::DebugUtilsMessenger(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, void *mem)
    : messageSeverity(pCreateInfo->messageSeverity)
    , messageType(pCreateInfo->messageType)
    , pfnUserCallback(pCreateInfo->pfnUserCallback)
    , pUserData(pCreateInfo->pUserData)
{
}

void DebugUtilsMessenger::destroy(const VkAllocationCallbacks *pAllocator)
{
}

size_t DebugUtilsMessenger::ComputeRequiredAllocationSize(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo)
{
	return 0;
}

void DebugUtilsMessenger::submitMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData)
{
	(*pfnUserCallback)(messageSeverity, messageTypes, pCallbackData, pUserData);
}

}  // namespace vk
