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

#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

class Driver;

// Device provides a wrapper around a VkDevice with a number of helper functions
// for common test operations.
class Device
{
public:
	Device();
	~Device();

	// CreateComputeDevice enumerates the physical devices, looking for a device
	// that supports compute.
	// If a compatible physical device is found, then a device is created and
	// assigned to out.
	// If a compatible physical device is not found, VK_SUCCESS will still be
	// returned (as there was no Vulkan error), but calling Device::IsValid()
	// on this device will return false.
	static VkResult CreateComputeDevice(
	    const Driver *driver, VkInstance instance, std::unique_ptr<Device> &out);

	// IsValid returns true if the Device is initialized and can be used.
	bool IsValid() const;

	// CreateBuffer creates a new buffer with the
	// VK_BUFFER_USAGE_STORAGE_BUFFER_BIT usage, and
	// VK_SHARING_MODE_EXCLUSIVE sharing mode.
	VkResult CreateStorageBuffer(VkDeviceMemory memory, VkDeviceSize size,
	                             VkDeviceSize offset, VkBuffer *out) const;

	// DestroyBuffer destroys a VkBuffer.
	void DestroyBuffer(VkBuffer buffer) const;

	// CreateShaderModule creates a new shader module with the given SPIR-V
	// code.
	VkResult CreateShaderModule(const std::vector<uint32_t> &spirv,
	                            VkShaderModule *out) const;

	// DestroyShaderModule destroys a VkShaderModule.
	void DestroyShaderModule(VkShaderModule shaderModule) const;

	// CreateDescriptorSetLayout creates a new descriptor set layout with the
	// given bindings.
	VkResult CreateDescriptorSetLayout(
	    const std::vector<VkDescriptorSetLayoutBinding> &bindings,
	    VkDescriptorSetLayout *out) const;

	// DestroyDescriptorSetLayout destroys a VkDescriptorSetLayout.
	void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) const;

	// CreatePipelineLayout creates a new single set descriptor set layout.
	VkResult CreatePipelineLayout(VkDescriptorSetLayout layout,
	                              VkPipelineLayout *out) const;

	// DestroyPipelineLayout destroys a VkPipelineLayout.
	void DestroyPipelineLayout(VkPipelineLayout pipelineLayout) const;

	// CreateComputePipeline creates a new compute pipeline with the entry point
	// "main".
	VkResult CreateComputePipeline(VkShaderModule module,
	                               VkPipelineLayout pipelineLayout,
	                               VkPipeline *out) const;

	// DestroyPipeline destroys a graphics or compute pipeline.
	void DestroyPipeline(VkPipeline pipeline) const;

	// CreateStorageBufferDescriptorPool creates a new descriptor pool that can
	// hold descriptorCount storage buffers.
	VkResult CreateStorageBufferDescriptorPool(uint32_t descriptorCount,
	                                           VkDescriptorPool *out) const;

	// DestroyDescriptorPool destroys the VkDescriptorPool.
	void DestroyDescriptorPool(VkDescriptorPool descriptorPool) const;

	// AllocateDescriptorSet allocates a single descriptor set with the given
	// layout from pool.
	VkResult AllocateDescriptorSet(VkDescriptorPool pool,
	                               VkDescriptorSetLayout layout,
	                               VkDescriptorSet *out) const;

	// UpdateStorageBufferDescriptorSets updates the storage buffers in
	// descriptorSet with the given list of VkDescriptorBufferInfos.
	void UpdateStorageBufferDescriptorSets(VkDescriptorSet descriptorSet,
	                                       const std::vector<VkDescriptorBufferInfo> &bufferInfos) const;

	// AllocateMemory allocates size bytes from a memory heap that has all the
	// given flag bits set.
	// If memory could not be allocated from any heap then
	// VK_ERROR_OUT_OF_DEVICE_MEMORY is returned.
	VkResult AllocateMemory(size_t size, VkMemoryPropertyFlags flags, VkDeviceMemory *out) const;

	// FreeMemory frees the VkDeviceMemory.
	void FreeMemory(VkDeviceMemory memory) const;

	// MapMemory wraps vkMapMemory, supplying the first VkDevice parameter.
	VkResult MapMemory(VkDeviceMemory memory, VkDeviceSize offset,
	                   VkDeviceSize size, VkMemoryMapFlags flags, void **ppData) const;

	// UnmapMemory wraps vkUnmapMemory, supplying the first VkDevice parameter.
	void UnmapMemory(VkDeviceMemory memory) const;

	// CreateCommandPool creates a new command pool.
	VkResult CreateCommandPool(VkCommandPool *out) const;

	// DestroyCommandPool destroys a VkCommandPool.
	void DestroyCommandPool(VkCommandPool commandPool) const;

	// AllocateCommandBuffer creates a new command buffer with a primary level.
	VkResult AllocateCommandBuffer(VkCommandPool pool, VkCommandBuffer *out) const;

	// FreeCommandBuffer frees the VkCommandBuffer.
	void FreeCommandBuffer(VkCommandPool pool, VkCommandBuffer buffer);

	// BeginCommandBuffer begins writing to commandBuffer.
	VkResult BeginCommandBuffer(VkCommandBufferUsageFlags usage, VkCommandBuffer commandBuffer) const;

	// QueueSubmitAndWait submits the given command buffer and waits for it to
	// complete.
	VkResult QueueSubmitAndWait(VkCommandBuffer commandBuffer) const;

	static VkResult GetPhysicalDevices(
	    const Driver *driver, VkInstance instance,
	    std::vector<VkPhysicalDevice> &out);

	static int GetComputeQueueFamilyIndex(
	    const Driver *driver, VkPhysicalDevice device);

private:
	Device(const Driver *driver, VkDevice device, VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

	static std::vector<VkQueueFamilyProperties>
	GetPhysicalDeviceQueueFamilyProperties(
	    const Driver *driver, VkPhysicalDevice device);

	const Driver *driver;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	uint32_t queueFamilyIndex;
};
