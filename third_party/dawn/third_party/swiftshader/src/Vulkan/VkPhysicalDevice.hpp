// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VK_PHYSICAL_DEVICE_HPP_
#define VK_PHYSICAL_DEVICE_HPP_

#include "VkFormat.hpp"
#include "VkObject.hpp"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#	include <vulkan/vk_android_native_buffer.h>
#endif

namespace vk {

class PhysicalDevice
{
public:
	static constexpr VkSystemAllocationScope GetAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE; }

	PhysicalDevice(const void *, void *mem);
	void destroy(const VkAllocationCallbacks *pAllocator) {}

	static size_t ComputeRequiredAllocationSize(const void *) { return 0; }

	const VkPhysicalDeviceFeatures &getFeatures() const;
	void getFeatures2(VkPhysicalDeviceFeatures2 *features) const;
	bool hasFeatures(const VkPhysicalDeviceFeatures &requestedFeatures) const;

	bool hasExtendedFeatures(const VkPhysicalDeviceLineRasterizationFeaturesEXT *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceProvokingVertexFeaturesEXT *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceVulkan11Features *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceVulkan12Features *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceVulkan13Features *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceDepthClipEnableFeaturesEXT *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *features) const;
	bool hasExtendedFeatures(const VkPhysicalDevicePrivateDataFeatures *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceTextureCompressionASTCHDRFeatures *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceShaderTerminateInvocationFeatures *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceSubgroupSizeControlFeatures *requested) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceInlineUniformBlockFeatures *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceShaderIntegerDotProductFeatures *features) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *requested) const;
	bool hasExtendedFeatures(const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *requested) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceDescriptorIndexingFeatures *requested) const;
	bool hasExtendedFeatures(const VkPhysicalDevicePipelineRobustnessFeaturesEXT *requested) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceProtectedMemoryFeatures *requested) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceBufferDeviceAddressFeatures *requested) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *requested) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR *requested) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT *requested) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceHostImageCopyFeaturesEXT *requested) const;
	bool hasExtendedFeatures(const VkPhysicalDeviceIndexTypeUint8FeaturesEXT *requested) const;

	const VkPhysicalDeviceProperties &getProperties() const;
	void getProperties(VkPhysicalDeviceIDProperties *properties) const;
	void getProperties(VkPhysicalDeviceMaintenance3Properties *properties) const;
	void getProperties(VkPhysicalDeviceMaintenance4Properties *properties) const;
	void getProperties(VkPhysicalDeviceMultiviewProperties *properties) const;
	void getProperties(VkPhysicalDevicePointClippingProperties *properties) const;
	void getProperties(VkPhysicalDeviceProtectedMemoryProperties *properties) const;
	void getProperties(VkPhysicalDeviceSubgroupProperties *properties) const;
	void getProperties(const VkExternalMemoryHandleTypeFlagBits *handleType, VkExternalImageFormatProperties *properties) const;
	void getProperties(const VkExternalMemoryHandleTypeFlagBits *handleType, VkExternalBufferProperties *properties) const;
	void getProperties(VkSamplerYcbcrConversionImageFormatProperties *properties) const;
#ifdef __ANDROID__
	void getProperties(VkPhysicalDevicePresentationPropertiesANDROID *properties) const;
	void getProperties(const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo, VkAndroidHardwareBufferUsageANDROID *properties) const;
#endif
	void getProperties(const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo, VkExternalBufferProperties *pExternalBufferProperties) const;
	void getProperties(const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo, VkExternalFenceProperties *pExternalFenceProperties) const;
	void getProperties(const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo, VkExternalSemaphoreProperties *pExternalSemaphoreProperties) const;
	void getProperties(VkPhysicalDeviceExternalMemoryHostPropertiesEXT *properties) const;
	void getProperties(VkPhysicalDeviceDriverProperties *properties) const;
	void getProperties(VkPhysicalDeviceLineRasterizationPropertiesEXT *properties) const;
	void getProperties(VkPhysicalDeviceProvokingVertexPropertiesEXT *properties) const;
	void getProperties(VkPhysicalDeviceFloatControlsProperties *) const;
	void getProperties(VkPhysicalDeviceSamplerFilterMinmaxProperties *properties) const;
	void getProperties(VkPhysicalDeviceTimelineSemaphoreProperties *properties) const;
	void getProperties(VkPhysicalDeviceDescriptorIndexingProperties *properties) const;
	void getProperties(VkPhysicalDeviceDepthStencilResolveProperties *properties) const;
	void getProperties(VkPhysicalDeviceCustomBorderColorPropertiesEXT *properties) const;
	void getProperties(VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *properties) const;
	void getProperties(VkPhysicalDeviceSubgroupSizeControlProperties *properties) const;
	void getProperties(VkPhysicalDeviceInlineUniformBlockProperties *properties) const;
	void getProperties(VkPhysicalDeviceTexelBufferAlignmentProperties *properties) const;
	void getProperties(VkPhysicalDeviceShaderIntegerDotProductProperties *properties) const;
	void getProperties(VkPhysicalDevicePipelineRobustnessPropertiesEXT *properties) const;
	void getProperties(VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT *properties) const;
	void getProperties(VkPhysicalDeviceHostImageCopyPropertiesEXT *properties) const;
	void getProperties(VkPhysicalDeviceVulkan11Properties *properties) const;
	void getProperties(VkPhysicalDeviceVulkan12Properties *properties) const;
	void getProperties(VkPhysicalDeviceVulkan13Properties *properties) const;

	static bool isFormatSupported(vk::Format format, VkImageType type, VkImageTiling tiling,
	                              VkImageUsageFlags usage, VkImageUsageFlags stencilUsage, VkImageCreateFlags flags);
	static void GetFormatProperties(Format format, VkFormatProperties *pFormatProperties);
	static void GetFormatProperties(Format format, VkFormatProperties3 *pFormatProperties);
	void getImageFormatProperties(Format format, VkImageType type, VkImageTiling tiling,
	                              VkImageUsageFlags usage, VkImageCreateFlags flags,
	                              VkImageFormatProperties *pImageFormatProperties) const;

	uint32_t getQueueFamilyPropertyCount() const;
	void getQueueFamilyProperties(uint32_t pQueueFamilyPropertyCount,
	                              VkQueueFamilyProperties *pQueueFamilyProperties) const;
	void getQueueFamilyProperties(uint32_t pQueueFamilyPropertyCount,
	                              VkQueueFamilyProperties2 *pQueueFamilyProperties) const;
	void getQueueFamilyGlobalPriorityProperties(VkQueueFamilyGlobalPriorityPropertiesKHR *pQueueFamilyGlobalPriorityProperties) const;
	bool validateQueueGlobalPriority(VkQueueGlobalPriorityKHR queueGlobalPriority) const;
	VkQueueGlobalPriorityKHR getDefaultQueueGlobalPriority() const;
	static const VkPhysicalDeviceMemoryProperties &GetMemoryProperties();

	static const VkPhysicalDeviceLimits &getLimits();

private:
	static VkSampleCountFlags getSampleCounts();
	VkQueueFamilyProperties getQueueFamilyProperties() const;

	template<typename T>
	T getSupportedFeatures(const T *requested) const;
};

using DispatchablePhysicalDevice = DispatchableObject<PhysicalDevice, VkPhysicalDevice>;

static inline PhysicalDevice *Cast(VkPhysicalDevice object)
{
	return DispatchablePhysicalDevice::Cast(object);
}

}  // namespace vk

#endif  // VK_PHYSICAL_DEVICE_HPP_
