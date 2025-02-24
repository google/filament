// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#include "Device.hpp"
#include "Driver.hpp"

Device::Device()
    : driver(nullptr)
    , device(nullptr)
    , physicalDevice(nullptr)
    , queueFamilyIndex(0)
{}

Device::Device(
    const Driver *driver, VkDevice device, VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex)
    : driver(driver)
    , device(device)
    , physicalDevice(physicalDevice)
    , queueFamilyIndex(queueFamilyIndex)
{}

Device::~Device()
{
	if(device != nullptr)
	{
		driver->vkDeviceWaitIdle(device);
		driver->vkDestroyDevice(device, nullptr);
	}
}

bool Device::IsValid() const
{
	return device != nullptr;
}

VkResult Device::CreateComputeDevice(
    const Driver *driver, VkInstance instance, std::unique_ptr<Device> &out)
{
	VkResult result;

	// Gather all physical devices
	std::vector<VkPhysicalDevice> physicalDevices;
	result = GetPhysicalDevices(driver, instance, physicalDevices);
	if(result != VK_SUCCESS)
	{
		return result;
	}

	// Inspect each physical device's queue families for compute support.
	for(auto physicalDevice : physicalDevices)
	{
		int queueFamilyIndex = GetComputeQueueFamilyIndex(driver, physicalDevice);
		if(queueFamilyIndex < 0)
		{
			continue;
		}

		const float queuePrioritory = 1.0f;
		const VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,  // sType
			nullptr,                                     // pNext
			0,                                           // flags
			(uint32_t)queueFamilyIndex,                  // queueFamilyIndex
			1,                                           // queueCount
			&queuePrioritory,                            // pQueuePriorities
		};

		const VkDeviceCreateInfo deviceCreateInfo = {
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,  // sType
			nullptr,                               // pNext
			0,                                     // flags
			1,                                     // queueCreateInfoCount
			&deviceQueueCreateInfo,                // pQueueCreateInfos
			0,                                     // enabledLayerCount
			nullptr,                               // ppEnabledLayerNames
			0,                                     // enabledExtensionCount
			nullptr,                               // ppEnabledExtensionNames
			nullptr,                               // pEnabledFeatures
		};

		VkDevice device;
		result = driver->vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
		if(result != VK_SUCCESS)
		{
			return result;
		}

		out.reset(new Device(driver, device, physicalDevice, static_cast<uint32_t>(queueFamilyIndex)));
		return VK_SUCCESS;
	}

	return VK_SUCCESS;
}

int Device::GetComputeQueueFamilyIndex(
    const Driver *driver, VkPhysicalDevice device)
{
	auto properties = GetPhysicalDeviceQueueFamilyProperties(driver, device);
	for(uint32_t i = 0; i < properties.size(); i++)
	{
		if((properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

std::vector<VkQueueFamilyProperties>
Device::GetPhysicalDeviceQueueFamilyProperties(
    const Driver *driver, VkPhysicalDevice device)
{
	std::vector<VkQueueFamilyProperties> out;
	uint32_t count = 0;
	driver->vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	out.resize(count);
	driver->vkGetPhysicalDeviceQueueFamilyProperties(device, &count, out.data());
	return out;
}

VkResult Device::GetPhysicalDevices(
    const Driver *driver, VkInstance instance,
    std::vector<VkPhysicalDevice> &out)
{
	uint32_t count = 0;
	VkResult result = driver->vkEnumeratePhysicalDevices(instance, &count, 0);
	if(result != VK_SUCCESS)
	{
		return result;
	}
	out.resize(count);
	return driver->vkEnumeratePhysicalDevices(instance, &count, out.data());
}

VkResult Device::CreateStorageBuffer(
    VkDeviceMemory memory, VkDeviceSize size,
    VkDeviceSize offset, VkBuffer *out) const
{
	const VkBufferCreateInfo info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,  // sType
		nullptr,                               // pNext
		0,                                     // flags
		size,                                  // size
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,    // usage
		VK_SHARING_MODE_EXCLUSIVE,             // sharingMode
		0,                                     // queueFamilyIndexCount
		nullptr,                               // pQueueFamilyIndices
	};

	VkBuffer buffer;
	VkResult result = driver->vkCreateBuffer(device, &info, 0, &buffer);
	if(result != VK_SUCCESS)
	{
		return result;
	}

	result = driver->vkBindBufferMemory(device, buffer, memory, offset);
	if(result != VK_SUCCESS)
	{
		return result;
	}

	*out = buffer;
	return VK_SUCCESS;
}

void Device::DestroyBuffer(VkBuffer buffer) const
{
	driver->vkDestroyBuffer(device, buffer, nullptr);
}

VkResult Device::CreateShaderModule(
    const std::vector<uint32_t> &spirv, VkShaderModule *out) const
{
	VkShaderModuleCreateInfo info = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,  // sType
		nullptr,                                      // pNext
		0,                                            // flags
		spirv.size() * 4,                             // codeSize
		spirv.data(),                                 // pCode
	};
	return driver->vkCreateShaderModule(device, &info, 0, out);
}

void Device::DestroyShaderModule(VkShaderModule shaderModule) const
{
	driver->vkDestroyShaderModule(device, shaderModule, nullptr);
}

VkResult Device::CreateDescriptorSetLayout(
    const std::vector<VkDescriptorSetLayoutBinding> &bindings,
    VkDescriptorSetLayout *out) const
{
	VkDescriptorSetLayoutCreateInfo info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,  // sType
		nullptr,                                              // pNext
		0,                                                    // flags
		(uint32_t)bindings.size(),                            // bindingCount
		bindings.data(),                                      // pBindings
	};

	return driver->vkCreateDescriptorSetLayout(device, &info, 0, out);
}

void Device::DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) const
{
	driver->vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

VkResult Device::CreatePipelineLayout(
    VkDescriptorSetLayout layout, VkPipelineLayout *out) const
{
	VkPipelineLayoutCreateInfo info = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // sType
		nullptr,                                        // pNext
		0,                                              // flags
		1,                                              // setLayoutCount
		&layout,                                        // pSetLayouts
		0,                                              // pushConstantRangeCount
		nullptr,                                        // pPushConstantRanges
	};

	return driver->vkCreatePipelineLayout(device, &info, 0, out);
}

void Device::DestroyPipelineLayout(VkPipelineLayout pipelineLayout) const
{
	driver->vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

VkResult Device::CreateComputePipeline(
    VkShaderModule module, VkPipelineLayout pipelineLayout,
    VkPipeline *out) const
{
	VkComputePipelineCreateInfo info = {
		VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,  // sType
		nullptr,                                         // pNext
		0,                                               // flags
		{
		    // stage
		    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // sType
		    nullptr,                                              // pNext
		    0,                                                    // flags
		    VK_SHADER_STAGE_COMPUTE_BIT,                          // stage
		    module,                                               // module
		    "main",                                               // pName
		    nullptr,                                              // pSpecializationInfo
		},
		pipelineLayout,  // layout
		0,               // basePipelineHandle
		0,               // basePipelineIndex
	};

	return driver->vkCreateComputePipelines(device, 0, 1, &info, 0, out);
}

void Device::DestroyPipeline(VkPipeline pipeline) const
{
	driver->vkDestroyPipeline(device, pipeline, nullptr);
}

VkResult Device::CreateStorageBufferDescriptorPool(uint32_t descriptorCount,
                                                   VkDescriptorPool *out) const
{
	VkDescriptorPoolSize size = {
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  // type
		descriptorCount,                    // descriptorCount
	};

	VkDescriptorPoolCreateInfo info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,  // sType
		nullptr,                                        // pNext
		0,                                              // flags
		1,                                              // maxSets
		1,                                              // poolSizeCount
		&size,                                          // pPoolSizes
	};

	return driver->vkCreateDescriptorPool(device, &info, 0, out);
}

void Device::DestroyDescriptorPool(VkDescriptorPool descriptorPool) const
{
	driver->vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

VkResult Device::AllocateDescriptorSet(
    VkDescriptorPool pool, VkDescriptorSetLayout layout,
    VkDescriptorSet *out) const
{
	VkDescriptorSetAllocateInfo info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,  // sType
		nullptr,                                         // pNext
		pool,                                            // descriptorPool
		1,                                               // descriptorSetCount
		&layout,                                         // pSetLayouts
	};

	return driver->vkAllocateDescriptorSets(device, &info, out);
}

void Device::UpdateStorageBufferDescriptorSets(
    VkDescriptorSet descriptorSet,
    const std::vector<VkDescriptorBufferInfo> &bufferInfos) const
{
	std::vector<VkWriteDescriptorSet> writes;
	writes.reserve(bufferInfos.size());
	for(uint32_t i = 0; i < bufferInfos.size(); i++)
	{
		writes.push_back(VkWriteDescriptorSet{
		    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,  // sType
		    nullptr,                                 // pNext
		    descriptorSet,                           // dstSet
		    i,                                       // dstBinding
		    0,                                       // dstArrayElement
		    1,                                       // descriptorCount
		    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,       // descriptorType
		    nullptr,                                 // pImageInfo
		    &bufferInfos[i],                         // pBufferInfo
		    nullptr,                                 // pTexelBufferView
		});
	}

	driver->vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}

VkResult Device::AllocateMemory(size_t size, VkMemoryPropertyFlags flags, VkDeviceMemory *out) const
{
	VkPhysicalDeviceMemoryProperties properties;
	driver->vkGetPhysicalDeviceMemoryProperties(physicalDevice, &properties);

	for(uint32_t type = 0; type < properties.memoryTypeCount; type++)
	{
		if((flags & properties.memoryTypes[type].propertyFlags) == 0)
		{
			continue;  // Type mismatch
		}

		if(size > properties.memoryHeaps[properties.memoryTypes[type].heapIndex].size)
		{
			continue;  // Too small.
		}

		const VkMemoryAllocateInfo info = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,  // sType
			nullptr,                                 // pNext
			size,                                    // allocationSize
			type,                                    // memoryTypeIndex
		};

		return driver->vkAllocateMemory(device, &info, 0, out);
	}

	return VK_ERROR_OUT_OF_DEVICE_MEMORY;  // TODO: Change to something not made up?
}

void Device::FreeMemory(VkDeviceMemory memory) const
{
	driver->vkFreeMemory(device, memory, nullptr);
}

VkResult Device::MapMemory(VkDeviceMemory memory, VkDeviceSize offset,
                           VkDeviceSize size, VkMemoryMapFlags flags, void **ppData) const
{
	return driver->vkMapMemory(device, memory, offset, size, flags, ppData);
}

void Device::UnmapMemory(VkDeviceMemory memory) const
{
	driver->vkUnmapMemory(device, memory);
}

VkResult Device::CreateCommandPool(VkCommandPool *out) const
{
	VkCommandPoolCreateInfo info = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  // sType
		nullptr,                                     // pNext
		0,                                           // flags
		queueFamilyIndex,                            // queueFamilyIndex
	};
	return driver->vkCreateCommandPool(device, &info, 0, out);
}

void Device::DestroyCommandPool(VkCommandPool commandPool) const
{
	return driver->vkDestroyCommandPool(device, commandPool, nullptr);
}

VkResult Device::AllocateCommandBuffer(
    VkCommandPool pool, VkCommandBuffer *out) const
{
	VkCommandBufferAllocateInfo info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,  // sType
		nullptr,                                         // pNext
		pool,                                            // commandPool
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,                 // level
		1,                                               // commandBufferCount
	};
	return driver->vkAllocateCommandBuffers(device, &info, out);
}

void Device::FreeCommandBuffer(VkCommandPool pool, VkCommandBuffer buffer)
{
	driver->vkFreeCommandBuffers(device, pool, 1, &buffer);
}

VkResult Device::BeginCommandBuffer(
    VkCommandBufferUsageFlags usage, VkCommandBuffer commandBuffer) const
{
	VkCommandBufferBeginInfo info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // sType
		nullptr,                                      // pNext
		usage,                                        // flags
		nullptr,                                      // pInheritanceInfo
	};

	return driver->vkBeginCommandBuffer(commandBuffer, &info);
}

VkResult Device::QueueSubmitAndWait(VkCommandBuffer commandBuffer) const
{
	VkQueue queue;
	driver->vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

	VkSubmitInfo info = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
		nullptr,                        // pNext
		0,                              // waitSemaphoreCount
		nullptr,                        // pWaitSemaphores
		nullptr,                        // pWaitDstStageMask
		1,                              // commandBufferCount
		&commandBuffer,                 // pCommandBuffers
		0,                              // signalSemaphoreCount
		nullptr,                        // pSignalSemaphores
	};

	VkResult result = driver->vkQueueSubmit(queue, 1, &info, VK_NULL_HANDLE);
	if(result != VK_SUCCESS)
	{
		return result;
	}

	return driver->vkQueueWaitIdle(queue);
}
