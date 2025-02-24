// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VULKAN_TESTER_HPP_
#define VULKAN_TESTER_HPP_

#include "VulkanHeaders.hpp"

#include <memory>

class VulkanTester
{
public:
	VulkanTester();
	virtual ~VulkanTester();

	// Call once after construction so that virtual functions may be called during init
	void initialize();

	const vk::DynamicLoader &dynamicLoader() const { return *dl; }
	vk::PhysicalDevice &getPhysicalDevice() { return physicalDevice; }
	vk::Device &getDevice() { return device; }
	vk::Queue &getQueue() { return queue; }
	uint32_t getQueueFamilyIndex() const { return queueFamilyIndex; }

private:
	std::unique_ptr<vk::DynamicLoader> loadDriver();

	std::unique_ptr<class ScopedSetIcdFilenames> setIcdFilenames;
	std::unique_ptr<vk::DynamicLoader> dl;
	vk::DebugUtilsMessengerEXT debugReport;

protected:
	const uint32_t queueFamilyIndex = 0;

	vk::Instance instance;  // Owning handle
	vk::PhysicalDevice physicalDevice;
	vk::Device device;  // Owning handle
	vk::Queue queue;
};

#endif  // VULKAN_TESTER_HPP_
