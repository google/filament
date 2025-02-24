/*!
\brief Function implementations of the Vulkan Device class.
\file PVRVk/DeviceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRVk/DeviceVk.h"
#include "PVRVk/InstanceVk.h"
#include "PVRVk/BufferVk.h"
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/FramebufferVk.h"
#include "PVRVk/DeviceMemoryVk.h"
#include "PVRVk/QueueVk.h"
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRVk/SamplerVk.h"
#include "PVRVk/ShaderModuleVk.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/RenderPassVk.h"
#include "PVRVk/EventVk.h"
#include "PVRVk/FenceVk.h"
#include "PVRVk/SemaphoreVk.h"
#include "PVRVk/GraphicsPipelineVk.h"
#include "PVRVk/ComputePipelineVk.h"
#include "PVRVk/PopulateCreateInfoVk.h"
#include "PVRVk/SwapchainVk.h"
#include "PVRVk/PipelineCacheVk.h"
#include "PVRVk/QueryPoolVk.h"

namespace pvrvk {

static inline VkDeviceQueueCreateInfo createQueueCreateInfo()
{
	static const float priority[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

	VkDeviceQueueCreateInfo createInfo = {};
	createInfo.queueCount = 1;
	createInfo.sType = static_cast<VkStructureType>(StructureType::e_DEVICE_QUEUE_CREATE_INFO);
	createInfo.pNext = NULL;
	createInfo.queueFamilyIndex = static_cast<uint32_t>(-1);
	createInfo.pQueuePriorities = priority;
	createInfo.flags = static_cast<VkDeviceQueueCreateFlags>(DeviceQueueCreateFlags::e_NONE);

	return createInfo;
}
} // namespace pvrvk
// Object creation
namespace pvrvk {
namespace impl {
std::vector<const char*> filterExtensions(const std::vector<VkExtensionProperties>& vec, const char* const* filters, uint32_t numfilters)
{
	std::vector<const char*> retval;
	for (uint32_t i = 0; i < vec.size(); ++i)
	{
		for (uint32_t j = 0; j < numfilters; ++j)
		{
			if (!strcmp(vec[i].extensionName, filters[j]))
			{
				retval.emplace_back(filters[j]);
				break;
			}
		}
	}
	return retval;
}

std::vector<const char*> filterLayers(const std::vector<VkLayerProperties>& vec, const char* const* filters, uint32_t numfilters)
{
	std::vector<const char*> retval;
	for (uint32_t i = 0; i < vec.size(); ++i)
	{
		for (uint32_t j = 0; j < numfilters; ++j)
		{
			if (!strcmp(vec[i].layerName, filters[j])) { retval.emplace_back(filters[j]); }
		}
	}
	return retval;
}

pvrvk::GraphicsPipeline Device_::createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo, const PipelineCache& pipelineCache)
{
	GraphicsPipelinePopulate pipelineFactory;
	VkPipeline vkPipeline;

	if (!pipelineFactory.init(createInfo)) { return GraphicsPipeline(); }
	vkThrowIfFailed(getVkBindings().vkCreateGraphicsPipelines(
						getVkHandle(), pipelineCache ? pipelineCache->getVkHandle() : VK_NULL_HANDLE, 1, &pipelineFactory.getVkCreateInfo(), nullptr, &vkPipeline),
		"Create GraphicsPipeline Failed.");

	Device device = shared_from_this();
	return GraphicsPipeline_::constructShared(device, vkPipeline, createInfo);
}

void Device_::createGraphicsPipelines(const GraphicsPipelineCreateInfo* createInfos, uint32_t numCreateInfos, const PipelineCache& pipelineCache, GraphicsPipeline* outPipelines)
{
	ArrayOrVector<GraphicsPipelinePopulate, 4> pipelineFactories(numCreateInfos);
	ArrayOrVector<VkGraphicsPipelineCreateInfo, 4> vkCreateInfos(numCreateInfos);
	ArrayOrVector<VkPipeline, 4> vkPipelines(numCreateInfos);

	for (uint32_t i = 0; i < numCreateInfos; ++i)
	{
		pipelineFactories[i].init(createInfos[i]);
		vkCreateInfos[i] = pipelineFactories[i].getVkCreateInfo();
	}
	vkThrowIfFailed(getVkBindings().vkCreateGraphicsPipelines(
						getVkHandle(), pipelineCache ? pipelineCache->getVkHandle() : VK_NULL_HANDLE, numCreateInfos, vkCreateInfos.get(), nullptr, vkPipelines.get()),
		"Create GraphicsPipeline Failed");

	Device device = shared_from_this();
	// create the pipeline wrapper
	for (uint32_t i = 0; i < numCreateInfos; ++i) { outPipelines[i] = GraphicsPipeline_::constructShared(device, vkPipelines[i], createInfos[i]); }
}

ComputePipeline Device_::createComputePipeline(const ComputePipelineCreateInfo& createInfo, const PipelineCache& pipelineCache)
{
	ComputePipelinePopulate pipelineFactory;
	VkPipeline vkPipeline;

	pipelineFactory.init(createInfo);
	vkThrowIfFailed(
		getVkBindings().vkCreateComputePipelines(getVkHandle(), pipelineCache ? pipelineCache->getVkHandle() : VK_NULL_HANDLE, 1, &pipelineFactory.createInfo, nullptr, &vkPipeline),
		"Create ComputePipeline Failed.");

	Device device = shared_from_this();
	return ComputePipeline_::constructShared(device, vkPipeline, createInfo);
}

void Device_::createComputePipelines(const ComputePipelineCreateInfo* createInfos, uint32_t numCreateInfos, const PipelineCache& pipelineCache, ComputePipeline* outPipelines)
{
	pvrvk::ArrayOrVector<ComputePipelinePopulate, 2> pipelineFactories(numCreateInfos);
	pvrvk::ArrayOrVector<VkComputePipelineCreateInfo, 2> vkCreateInfos(numCreateInfos);
	pvrvk::ArrayOrVector<VkPipeline, 2> vkPipelines(numCreateInfos);

	for (uint32_t i = 0; i < numCreateInfos; ++i)
	{
		pipelineFactories[i].init(createInfos[i]);
		vkCreateInfos[i] = pipelineFactories[i].createInfo;
	}
	vkThrowIfFailed(getVkBindings().vkCreateComputePipelines(getVkHandle(), pipelineCache ? pipelineCache->getVkHandle() : VK_NULL_HANDLE, static_cast<uint32_t>(numCreateInfos),
						vkCreateInfos.get(), nullptr, vkPipelines.get()),
		"Create ComputePipelines Failed");

	Device device = shared_from_this();
	// create the pipeline wrapper
	for (uint32_t i = 0; i < numCreateInfos; ++i) { outPipelines[i] = ComputePipeline_::constructShared(device, vkPipelines[i], createInfos[i]); }
}

pvrvk::Image Device_::createImage(const ImageCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return Image_::constructShared(device, createInfo);
}

void Device_::updateDescriptorSets(const WriteDescriptorSet* writeDescSets, uint32_t numWriteDescSets, const CopyDescriptorSet* copyDescSets, uint32_t numCopyDescSets)
{
	// WRITE DESCRIPTORSET
	pvrvk::ArrayOrVector<VkWriteDescriptorSet, 4> vkWriteDescSets(numWriteDescSets);
	// Count number of image, buffer and texel buffer view needed
	uint32_t numImageInfos = 0, numBufferInfos = 0, numTexelBufferView = 0;

	for (uint32_t i = 0; i < numWriteDescSets; ++i)
	{
		if ((writeDescSets[i].getDescriptorType() >= DescriptorType::e_SAMPLER && writeDescSets[i].getDescriptorType() <= DescriptorType::e_STORAGE_IMAGE) ||
			writeDescSets[i].getDescriptorType() == DescriptorType::e_INPUT_ATTACHMENT)
		{
#ifdef DEBUG
			if (writeDescSets[i].getDescriptorType() == DescriptorType::e_SAMPLER)
			{
				// Validate the Sampler bindings
				std::for_each(writeDescSets[i]._infos.begin(), writeDescSets[i]._infos.end(), [](const WriteDescriptorSet::DescriptorInfos& infos) {
					if (infos.isValid()) { assert(infos.imageInfo.sampler && "Sampler Must be valid"); }
				});
			}
			else if (writeDescSets[i].getDescriptorType() == DescriptorType::e_COMBINED_IMAGE_SAMPLER)
			{
				// Validate the ImageView and Sampler bindings
				std::for_each(writeDescSets[i]._infos.begin(), writeDescSets[i]._infos.end(), [](const WriteDescriptorSet::DescriptorInfos& infos) {
					if (infos.isValid())
					{
						assert(infos.imageInfo.imageView && "ImageView Must be valid");
						assert(infos.imageInfo.sampler && "Sampler Must be valid");
					}
				});
			}
			else
			{
				// Validate the ImageView bindings
				std::for_each(writeDescSets[i]._infos.begin(), writeDescSets[i]._infos.end(), [](const WriteDescriptorSet::DescriptorInfos& infos) {
					if (infos.isValid()) { assert(infos.imageInfo.imageView && "ImageView Must be valid"); }
				});
			}
#endif
			numImageInfos += writeDescSets[i].getNumDescriptors();
		}
		else if (writeDescSets[i].getDescriptorType() >= DescriptorType::e_UNIFORM_BUFFER && writeDescSets[i].getDescriptorType() <= DescriptorType::e_STORAGE_BUFFER_DYNAMIC)
		{
#ifdef DEBUG
			// Validate the Buffer bindings
			std::for_each(writeDescSets[i]._infos.begin(), writeDescSets[i]._infos.end(), [](const WriteDescriptorSet::DescriptorInfos& infos) {
				if (infos.isValid()) { assert(infos.bufferInfo.buffer && "Buffer Must be valid"); }
			});
#endif
			numBufferInfos += writeDescSets[i].getNumDescriptors();
		}
		else if (writeDescSets[i].getDescriptorType() == DescriptorType::e_UNIFORM_TEXEL_BUFFER || writeDescSets[i].getDescriptorType() <= DescriptorType::e_STORAGE_TEXEL_BUFFER) // Texel buffer
		{
#ifdef DEBUG
			// Validate the BufferView bindings
			std::for_each(writeDescSets[i]._infos.begin(), writeDescSets[i]._infos.end(), [](const WriteDescriptorSet::DescriptorInfos& infos) {
				if (infos.isValid()) { assert(infos.texelBuffer->getBuffer() && "Buffer Must be valid"); }
			});
#endif
			numTexelBufferView += writeDescSets[i].getNumDescriptors();
		}
		else
		{
			assert(false && "Unsupported Descriptor type");
		}
	}

	// now allocate
	pvrvk::ArrayOrVector<VkDescriptorBufferInfo, 4> bufferInfoVk(numBufferInfos);
	pvrvk::ArrayOrVector<VkDescriptorImageInfo, 4> imageInfoVk(numImageInfos);
	pvrvk::ArrayOrVector<VkBufferView, 4> texelBufferVk(numTexelBufferView);
	uint32_t vkImageInfoOffset = 0;
	uint32_t vkBufferInfoOffset = 0;

	// now process
	for (uint32_t i = 0; i < numWriteDescSets; ++i)
	{
		VkWriteDescriptorSet& vkWriteDescSet = vkWriteDescSets[i];
		const WriteDescriptorSet& writeDescSet = writeDescSets[i];
		memset(&vkWriteDescSet, 0, sizeof(vkWriteDescSet));
		vkWriteDescSet.descriptorType = static_cast<VkDescriptorType>(writeDescSet.getDescriptorType());
		vkWriteDescSet.sType = static_cast<VkStructureType>(StructureType::e_WRITE_DESCRIPTOR_SET);
		vkWriteDescSet.dstArrayElement = writeDescSet.getDestArrayElement();
		vkWriteDescSet.dstBinding = writeDescSet.getDestBinding();
		vkWriteDescSet.dstSet = writeDescSet.getDescriptorSet()->getVkHandle();
		writeDescSet.updateKeepAliveIntoDestinationDescriptorSet();

		// Do the buffer info
		if (writeDescSet._infoType == WriteDescriptorSet::InfoType::BufferInfo)
		{
			vkWriteDescSet.pBufferInfo = bufferInfoVk.get() + vkBufferInfoOffset;
			std::transform(writeDescSet._infos.begin(), writeDescSet._infos.end(), bufferInfoVk.get() + vkBufferInfoOffset,
				[&](const WriteDescriptorSet::DescriptorInfos& writeDescSet) -> VkDescriptorBufferInfo {
					return VkDescriptorBufferInfo{ writeDescSet.bufferInfo.buffer->getVkHandle(), writeDescSet.bufferInfo.offset, writeDescSet.bufferInfo.range };
				});
			vkWriteDescSet.descriptorCount = writeDescSet._infos.size();
			vkBufferInfoOffset += writeDescSet._infos.size();
		}
		else if (writeDescSet._infoType == WriteDescriptorSet::InfoType::ImageInfo)
		{
			vkWriteDescSet.pImageInfo = imageInfoVk.get() + vkImageInfoOffset;
			std::transform(writeDescSet._infos.begin(), writeDescSet._infos.end(), imageInfoVk.get() + vkImageInfoOffset,
				[&](const WriteDescriptorSet::DescriptorInfos& writeDescSet) -> VkDescriptorImageInfo {
					return VkDescriptorImageInfo{ (writeDescSet.imageInfo.sampler ? writeDescSet.imageInfo.sampler->getVkHandle() : VK_NULL_HANDLE),
						(writeDescSet.imageInfo.imageView ? writeDescSet.imageInfo.imageView->getVkHandle() : VK_NULL_HANDLE),
						static_cast<VkImageLayout>(writeDescSet.imageInfo.imageLayout) };
				});
			vkWriteDescSet.descriptorCount = writeDescSet._infos.size();
			vkImageInfoOffset += writeDescSet._infos.size();
		}
	}

	// COPY DESCRIPTOR SET
	pvrvk::ArrayOrVector<VkCopyDescriptorSet, 4> vkCopyDescriptorSets(numCopyDescSets);
	std::transform(copyDescSets, copyDescSets + numCopyDescSets, vkCopyDescriptorSets.get(), [&](const CopyDescriptorSet& copyDescSet) {
		return VkCopyDescriptorSet{ static_cast<VkStructureType>(StructureType::e_COPY_DESCRIPTOR_SET), nullptr, copyDescSet.srcSet->getVkHandle(), copyDescSet.srcBinding,
			copyDescSet.srcArrayElement, copyDescSet.dstSet->getVkHandle(), copyDescSet.dstBinding, copyDescSet.dstArrayElement, copyDescSet.descriptorCount };
	});

	getVkBindings().vkUpdateDescriptorSets(
		getVkHandle(), static_cast<uint32_t>(numWriteDescSets), vkWriteDescSets.get(), static_cast<uint32_t>(numCopyDescSets), vkCopyDescriptorSets.get());
}

ImageView Device_::createImageView(const ImageViewCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return ImageView_::constructShared(device, createInfo);
}

Framebuffer Device_::createFramebuffer(const FramebufferCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return Framebuffer_::constructShared(device, createInfo);
}

Fence Device_::createFence(const FenceCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return Fence_::constructShared(device, createInfo);
}

Event Device_::createEvent(const EventCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return Event_::constructShared(device, createInfo);
}

Semaphore Device_::createSemaphore(const SemaphoreCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return Semaphore_::constructShared(device, createInfo);
}

Buffer Device_::createBuffer(const BufferCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return Buffer_::constructShared(device, createInfo);
}

DeviceMemory Device_::allocateMemory(const MemoryAllocationInfo& allocationInfo)
{
	assert(allocationInfo.getMemoryTypeIndex() != uint32_t(-1) && allocationInfo.getAllocationSize() > 0u && "Invalid MemoryAllocationInfo");
	const MemoryPropertyFlags memFlags = getPhysicalDevice()->getMemoryProperties().getMemoryTypes()[allocationInfo.getMemoryTypeIndex()].getPropertyFlags();
	Device device = shared_from_this();
	return DeviceMemory_::constructShared(device, allocationInfo, memFlags);
}

ShaderModule Device_::createShaderModule(const ShaderModuleCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return ShaderModule_::constructShared(device, createInfo);
}

Sampler Device_::createSampler(const SamplerCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return Sampler_::constructShared(device, createInfo);
}

RenderPass Device_::createRenderPass(const RenderPassCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return RenderPass_::constructShared(device, createInfo);
}

BufferView Device_::createBufferView(const BufferViewCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return BufferView_::constructShared(device, createInfo);
}

DescriptorPool Device_::createDescriptorPool(const DescriptorPoolCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return DescriptorPool_::constructShared(device, createInfo);
}

CommandPool Device_::createCommandPool(const CommandPoolCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return CommandPool_::constructShared(device, createInfo);
}

PipelineLayout Device_::createPipelineLayout(const PipelineLayoutCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return PipelineLayout_::constructShared(device, createInfo);
}

bool Device_::waitForFences(uint32_t numFences, const Fence* const fences, const bool waitAll, const uint64_t timeout)
{
	pvrvk::ArrayOrVector<VkFence, 4> vkFences(numFences);

	for (uint32_t i = 0; i < numFences; i++) { vkFences[i] = fences[i]->getVkHandle(); }

	Result res;
	vkThrowIfError(res = static_cast<pvrvk::Result>(getVkBindings().vkWaitForFences(getVkHandle(), numFences, vkFences.get(), waitAll, timeout)), "WaitForFences failed");
	if (res == Result::e_SUCCESS) { return true; }
	assert(res == Result::e_TIMEOUT && "WaitForFences returned neither success nor timeout, yet did not throw!");
	return false;
}

void Device_::resetFences(uint32_t numFences, const Fence* const fences)
{
	pvrvk::ArrayOrVector<VkFence, 4> vkFences(numFences);

	for (uint32_t i = 0; i < numFences; i++) { vkFences[i] = fences[i]->getVkHandle(); }

	vkThrowIfFailed(getVkBindings().vkResetFences(getVkHandle(), numFences, vkFences.get()), "Reset fences failed");
}

pvrvk::DescriptorSetLayout Device_::createDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return DescriptorSetLayout_::constructShared(device, createInfo);
}

PipelineCache Device_::createPipelineCache(const PipelineCacheCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return PipelineCache_::constructShared(device, createInfo);
}

void Device_::mergePipelineCache(const PipelineCache* srcPipeCaches, uint32_t numSrcPipeCaches, PipelineCache destPipeCache)
{
	pvrvk::ArrayOrVector<VkPipelineCache, 4> vkSrcPipeCaches(numSrcPipeCaches);
	std::transform(srcPipeCaches, srcPipeCaches + numSrcPipeCaches, vkSrcPipeCaches.get(), [&](const PipelineCache& pipelineCache) { return pipelineCache->getVkHandle(); });

	vkThrowIfFailed(getVkBindings().vkMergePipelineCaches(getVkHandle(), destPipeCache->getVkHandle(), numSrcPipeCaches, vkSrcPipeCaches.get()), "Failed to merge Pipeline Caches");
}

Swapchain Device_::createSwapchain(const SwapchainCreateInfo& createInfo, const Surface& surface)
{
	Device device = shared_from_this();
	return Swapchain_::constructShared(device, surface, createInfo);
}

QueryPool Device_::createQueryPool(const QueryPoolCreateInfo& createInfo)
{
	Device device = shared_from_this();
	return QueryPool_::constructShared(device, createInfo);
}

void Device_::waitIdle() { getVkBindings().vkDeviceWaitIdle(getVkHandle()); }

// CAUTION - We will be abusing queueFamilyProperties[...].numQueues as a counter for queues remaining.
struct QueueFamilyCreateInfo
{
	uint32_t queueFamilyId;
	uint32_t queueId;
	bool supportPresentation;
	explicit QueueFamilyCreateInfo(uint32_t queueFamilyId = -1, uint32_t queueId = -1, bool supportPresentation = 0)
		: queueFamilyId(queueFamilyId), queueId(queueId), supportPresentation(supportPresentation)
	{}
};

void Device_::retrieveQueues()
{
	// Retrieve device queues
	const std::vector<QueueFamilyProperties>& queueFamProps = getPhysicalDevice()->getQueueFamilyProperties();

	uint32_t queueFamilyIndex;
	uint32_t queueIndex;
	float queuePriority;
	for (uint32_t i = 0; i < _createInfo.getNumDeviceQueueCreateInfos(); ++i)
	{
		const DeviceQueueCreateInfo& queueCreateInfo = _createInfo.getDeviceQueueCreateInfo(i);
		queueFamilyIndex = queueCreateInfo.getQueueFamilyIndex();

		_queueFamilies.emplace_back(QueueFamily());
		_queueFamilies.back().queueFamily = queueFamilyIndex;
		_queueFamilies.back().queues.resize(queueCreateInfo.getNumQueues());
		VkQueue vkQueue;
		Device device = shared_from_this();
		for (queueIndex = 0; queueIndex < queueCreateInfo.getNumQueues(); ++queueIndex)
		{
			queuePriority = queueCreateInfo.getQueuePriority(queueIndex);
			getVkBindings().vkGetDeviceQueue(getVkHandle(), queueFamilyIndex, queueIndex, &vkQueue);
			_queueFamilies.back().queues[queueIndex] = Queue_::constructShared(device, vkQueue, queueFamProps[queueFamilyIndex].getQueueFlags(), queueFamilyIndex, queuePriority);
		}
	}
}

Device_::Device_(make_shared_enabler, PhysicalDevice& physicalDevice, const DeviceCreateInfo& createInfo) : PVRVkPhysicalDeviceObjectBase(physicalDevice)
{
	_createInfo = createInfo;

	assert(getPhysicalDevice()->getQueueFamilyProperties().size() >= static_cast<size_t>(1) && "A Vulkan device must support at least 1 queue family.");

	ArrayOrVector<VkDeviceQueueCreateInfo, 2> queueCreateInfos(_createInfo.getNumDeviceQueueCreateInfos());
	for (uint32_t i = 0; i < _createInfo.getNumDeviceQueueCreateInfos(); ++i)
	{
		const DeviceQueueCreateInfo& queueCreateInfo = _createInfo.getDeviceQueueCreateInfo(i);

		VkDeviceQueueCreateInfo queueCreateInfoVk = {};
		queueCreateInfoVk.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfoVk.queueFamilyIndex = queueCreateInfo.getQueueFamilyIndex();
		queueCreateInfoVk.queueCount = queueCreateInfo.getNumQueues();
		queueCreateInfoVk.pQueuePriorities = queueCreateInfo.getQueuePriorities().data();
		queueCreateInfos[i] = queueCreateInfoVk;
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_DEVICE_CREATE_INFO);
	deviceCreateInfo.flags = static_cast<VkDeviceCreateFlags>(_createInfo.getFlags());
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(_createInfo.getNumDeviceQueueCreateInfos());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.get();

	deviceCreateInfo.pEnabledFeatures = &_createInfo.getEnabledFeatures()->get();

	// Extensions
	std::vector<const char*> enabledExtensions;
	if (_createInfo.getExtensionList().getNumExtensions())
	{
		for (uint32_t i = 0; i < _createInfo.getExtensionList().getNumExtensions(); ++i)
		{ enabledExtensions.emplace_back(_createInfo.getExtensionList().getExtension(i).getName().c_str()); }

		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}

	vkThrowIfFailed(getPhysicalDevice()->getInstance()->getVkBindings().vkCreateDevice(getPhysicalDevice()->getVkHandle(), &deviceCreateInfo, nullptr, &_vkHandle),
		"Vulkan Device Creation failed");

	initVkDeviceBindings(getVkHandle(), &_vkBindings, getPhysicalDevice()->getInstance()->getVkBindings().vkGetDeviceProcAddr);

	// setup the extension table which can be used to cheaply determine support for extensions
	_extensionTable.setEnabledExtensions(enabledExtensions);

	if (getEnabledExtensionTable().extTransformFeedbackEnabled)
	{
		// use VK_KHR_get_physical_device_properties2 if the extension is supported
		if (getPhysicalDevice()->getInstance()->getEnabledExtensionTable().khrGetPhysicalDeviceProperties2Enabled)
		{
			{
				VkPhysicalDeviceTransformFeedbackFeaturesEXT physicalDeviceTransformFeedbackFeaturesEXT = {};

				VkPhysicalDeviceFeatures2KHR deviceFeatures = {};
				deviceFeatures.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_FEATURES_2_KHR);
				deviceFeatures.pNext = &physicalDeviceTransformFeedbackFeaturesEXT;
				getPhysicalDevice()->getInstance()->getVkBindings().vkGetPhysicalDeviceFeatures2KHR(getPhysicalDevice()->getVkHandle(), &deviceFeatures);

				// Bypass the VkBaseInStructure part of the returned properties
				uint32_t offset = sizeof(VkBaseInStructure);
				memcpy(&_transformFeedbackFeatures, &physicalDeviceTransformFeedbackFeaturesEXT + offset, sizeof(physicalDeviceTransformFeedbackFeaturesEXT) - offset);
			}

			{
				VkPhysicalDeviceTransformFeedbackPropertiesEXT physicalDeviceTransformFeedbackPropertiesEXT = {};

				VkPhysicalDeviceProperties2KHR deviceProperties = {};
				deviceProperties.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_PROPERTIES_2_KHR);
				deviceProperties.pNext = &physicalDeviceTransformFeedbackPropertiesEXT;
				getPhysicalDevice()->getInstance()->getVkBindings().vkGetPhysicalDeviceProperties2(getPhysicalDevice()->getVkHandle(), &deviceProperties);

				// Bypass the VkBaseInStructure part of the returned properties
				uint32_t offset = sizeof(VkBaseInStructure);
				memcpy(&_transformFeedbackProperties, &physicalDeviceTransformFeedbackPropertiesEXT + offset, sizeof(physicalDeviceTransformFeedbackPropertiesEXT) - offset);
			}
		}
	}
}
} // namespace impl
} // namespace pvrvk

//!\endcond
