/*!
\brief The PVRVk Device class. One of the bysiest classes in Vulkan, together with the Command Buffer.
\file PVRVk/DeviceVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/PhysicalDeviceVk.h"
#include "PVRVk/ExtensionsVk.h"
#include "PVRVk/LayersVk.h"
#include "PVRVk/PVRVkObjectBaseVk.h"
#include "PVRVk/DebugUtilsVk.h"

namespace pvrvk {
struct SamplerCreateInfo;
namespace impl {
//\cond NO_DOXYGEN
inline void reportDestroyedAfterDevice() { assert(false && "Attempted to destroy object after its corresponding device"); }
//\endcond

/// <summary>GpuDevice implementation that supports Vulkan</summary>
class Device_ : public PVRVkPhysicalDeviceObjectBase<VkDevice, ObjectType::e_DEVICE>, public std::enable_shared_from_this<Device_>
{
private:
	friend class PhysicalDevice_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Device_;
	};

	static Device constructShared(PhysicalDevice& physicalDevice, const DeviceCreateInfo& createInfo)
	{
		return std::make_shared<Device_>(make_shared_enabler{}, physicalDevice, createInfo);
	}

	struct QueueFamily
	{
		uint32_t queueFamily;
		std::vector<Queue> queues;
	};

	DeviceExtensionTable _extensionTable;
	std::vector<QueueFamily> _queueFamilies;
	DeviceCreateInfo _createInfo;
	VkDeviceBindings _vkBindings;
	PhysicalDeviceTransformFeedbackProperties _transformFeedbackProperties;
	PhysicalDeviceTransformFeedbackFeatures _transformFeedbackFeatures;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Device_)
	Device_(make_shared_enabler, PhysicalDevice& physicalDevice, const DeviceCreateInfo& createInfo);

	~Device_()
	{
		_queueFamilies.clear();
		if (getVkHandle() != VK_NULL_HANDLE)
		{
			_vkBindings.vkDestroyDevice(getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
	}
	//!\endcond

	/// <summary>Retrieve and initialise the list of queues</summary>
	void retrieveQueues();

	/// <summary>Wait on the host for the completion of outstanding queue operations
	/// for all queues on this device This is equivalent to calling waitIdle for all
	/// queues owned by this device.</summary>
	void waitIdle();

	/// <summary>createComputePipeline</summary>
	/// <param name="createInfo">create info</param>
	/// <returns>Return a valid compute pipeline on success</returns>
	/// <param name="pipelineCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that
	/// cache is enabled for the duration of the command.</param>
	/// <returns>Return a valid pipeline on success</returns>.
	ComputePipeline createComputePipeline(const ComputePipelineCreateInfo& createInfo, const PipelineCache& pipelineCache = PipelineCache());

	/// <summary>create array of compute pipelines</summary>
	/// <param name="createInfo">Compute pipeline create Infos</param>
	/// <param name="numCreateInfos">Number of compute pipleine to create</param>
	/// <param name="outPipelines">Out pipelines</param>
	/// <param name="pipelineCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that
	/// cache is enabled for the duration of the command.</param>
	void createComputePipelines(const ComputePipelineCreateInfo* createInfo, uint32_t numCreateInfos, const PipelineCache& pipelineCache, ComputePipeline* outPipelines);

	/// <summary>create graphicsPipeline</summary>
	/// <param name="createInfo">Pipeline create info</param>
	/// <param name="pipelineCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that
	/// cache is enabled for the duration of the command.</param>
	/// <returns>Return a valid pipeline on success</returns>.
	GraphicsPipeline createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo, const PipelineCache& pipelineCache = PipelineCache());

	/// <summary>create array of graphics pipelines</summary>
	/// <param name="createInfos">Pipeline create infos</param>
	/// <param name="numCreateInfos">Number of pipeline to create</param>
	/// <param name="outPipelines">Out pipeline</param>
	/// <param name="pipelineCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that
	/// cache is enabled for the duration of the command.</param>
	void createGraphicsPipelines(const GraphicsPipelineCreateInfo* createInfos, uint32_t numCreateInfos, const PipelineCache& pipelineCache, GraphicsPipeline* outPipelines);

	/// <summary>Create sampler object</summary>
	/// <param name="createInfo">Sampler Create info</param>
	/// <returns>Return a valid sampler object on success</returns>.
	Sampler createSampler(const SamplerCreateInfo& createInfo);

	/// <summary>create an image using this device.</summary>
	/// <param name="createInfo">The image creation descriptor</param>
	/// <returns>The created Image object on success</returns>
	Image createImage(const ImageCreateInfo& createInfo);

	/// <summary>Create image view object</summary>
	/// <param name="createInfo">The image view creation descriptor</param>
	/// <returns>The created ImageView object on success</returns>
	ImageView createImageView(const ImageViewCreateInfo& createInfo);

	/// <summary>Create buffer view</summary>
	/// <param name="createInfo">The buffer view creation descriptor</param>
	/// <returns>The created BufferView object on success</returns>
	BufferView createBufferView(const BufferViewCreateInfo& createInfo);

	/// <summary>Create a new buffer object and (optionally) allocate and bind memory for it</summary>
	/// <param name="createInfo">The buffer creation descriptor</param>
	/// <returns>Return a valid object if success</returns>.
	Buffer createBuffer(const BufferCreateInfo& createInfo);

	/// <summary>Create device memory block</summary>
	/// <param name="allocationInfo">memory allocation info</param>
	/// <returns>Return a valid object if success</returns>.
	DeviceMemory allocateMemory(const MemoryAllocationInfo& allocationInfo);

	/// <summary>Create Shader Object</summary>
	/// <param name="createInfo">Shader module createInfo</param>
	/// <returns> Return a valid shader if success</returns>
	ShaderModule createShaderModule(const ShaderModuleCreateInfo& createInfo);

	/// <summary>createFramebuffer Create Framebuffer object</summary>
	/// <param name="createInfo">Framebuffer createInfo</param>
	/// <returns>return a valid object if success</returns>.
	Framebuffer createFramebuffer(const FramebufferCreateInfo& createInfo);

	/// <summary>create renderpass</summary>
	/// <param name="createInfo">RenderPass createInfo</param>
	/// <returns>return a valid object if success</returns>.
	RenderPass createRenderPass(const RenderPassCreateInfo& createInfo);

	/// <summary>Create DescriptorPool</summary>
	/// <param name="createInfo">DescriptorPool createInfo</param>
	/// <returns>return a valid object if success</returns>.
	DescriptorPool createDescriptorPool(const DescriptorPoolCreateInfo& createInfo);

	/// <summary>create Descriptor set layout</summary>
	/// <param name="createInfo">Descriptor layout createInfo</param>
	/// <returns>Return a valid object if success</returns>.
	DescriptorSetLayout createDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& createInfo);

	/// <summary>Create PipelineCache object</summary>
	/// <param name="createInfo">Pipeline cache creation info descriptor.</param>
	/// <returns>Return a valid Pipeline cache object.</returns>
	PipelineCache createPipelineCache(const PipelineCacheCreateInfo& createInfo = PipelineCacheCreateInfo());

	/// <summary>Merge PipelineCache objects</summary>
	/// <param name="srcPipeCaches">Pipeline caches, which will be merged into destPipeCache</param>
	/// <param name="numSrcPipeCaches">Number of source pipeline caches to be merged</param>
	/// <param name="destPipeCache">Pipeline cache to merge results into. The previous contents of destPipeCache are included after the merge</param>
	/// <returns>Return result</returns>.
	void mergePipelineCache(const PipelineCache* srcPipeCaches, uint32_t numSrcPipeCaches, PipelineCache destPipeCache);

	/// <summary>Create pipeline layout</summary>
	/// <param name="createInfo">Pipeline layout create info</param>
	/// <returns>Return a valid object if success</returns>.
	PipelineLayout createPipelineLayout(const PipelineLayoutCreateInfo& createInfo);

	/// <summary>Wait this device for an array of fences</summary>
	/// <param name="numFences">Number of fence to wait</param>
	/// <param name="fences">Fences to wait for</param>
	/// <param name="waitAll">Wait for all fence if flags set to true</param>
	/// <param name="timeout">Wait timeout</param>
	/// <returns>Return true if fence waits successfull, false if timed out.</returns>.
	bool waitForFences(uint32_t numFences, const Fence* fences, const bool waitAll, const uint64_t timeout);

	/// <summary>Reset an array of fences</summary>
	/// <param name="numFences">Number of fence to reset</param>
	/// <param name="fences">Fence to reset</param>
	void resetFences(uint32_t numFences, const Fence* fences);

	/// <summary>Create commandpool</summary>
	/// <param name="createInfo">Command Pool creation info structure</param>
	/// <returns>Return a valid object if success</returns>.
	CommandPool createCommandPool(const CommandPoolCreateInfo& createInfo);

	/// <summary>Create Fence</summary>
	/// <param name="createInfo">Fence create info</param>
	/// <returns>Return a valid object if success</returns>.
	Fence createFence(const FenceCreateInfo& createInfo = FenceCreateInfo());

	/// <summary>Create Event</summary>
	/// <param name="createInfo">Event create info</param>
	/// <returns>Return a valid object if success</returns>.
	Event createEvent(const EventCreateInfo& createInfo = EventCreateInfo());

	/// <summary>Create semaphore</summary>
	/// <param name="createInfo">Semaphore create info</param>
	/// <returns>Return a valid object if success</returns>.
	Semaphore createSemaphore(const SemaphoreCreateInfo& createInfo = SemaphoreCreateInfo());

	/// <summary>Create QueryPool</summary>
	/// <param name="createInfo">QueryPool create info</param>
	/// <returns>return a valid object if success</returns>.
	QueryPool createQueryPool(const QueryPoolCreateInfo& createInfo);

	/// <summary>Create Swapchain</summary>
	/// <param name="createInfo">Swapchain createInfo</param>
	/// <param name="surface">Swapchain's surface</param>
	/// <returns>Return a valid object if success</returns>.
	Swapchain createSwapchain(const SwapchainCreateInfo& createInfo, const Surface& surface);

	/// <summary>Get Queue</summary>
	/// <param name="queueFamily">Queue Family id</param>
	/// <param name="queueId">Queue Id</param>
	/// <returns>Return the queue</returns>
	Queue getQueue(uint32_t queueFamily, uint32_t queueId)
	{
		for (uint32_t i = 0; i < _queueFamilies.size(); ++i)
		{
			if (_queueFamilies[i].queueFamily == queueFamily) { return _queueFamilies[i].queues[queueId]; }
		}
		throw ErrorValidationFailedEXT("Request for queue from family id that did not exist.");
	}

	/// <summary>Get a list of enabled extensions which includes names and spec versions</summary>
	/// <returns>VulkanExtensionList&</returns>
	const VulkanExtensionList& getEnabledExtensionList() { return _createInfo.getExtensionList(); }

	/// <summary>Return a table which contains boolean members set to true/false corresponding to whether specific extensions have been enabled</summary>
	/// <returns>A table of extensions</returns>
	const DeviceExtensionTable& getEnabledExtensionTable() const { return _extensionTable; }

	/// <summary>Update Descriptorsets</summary>
	/// <param name="writeDescSets">Write descriptor sets</param>
	/// <param name="numWriteDescSets">Number of write Descriptor sets</param>
	/// <param name="copyDescSets">Copy operation happens after the Write operation.</param>
	/// <param name="numCopyDescSets">Number of copy descriptor sets</param>
	void updateDescriptorSets(const WriteDescriptorSet* writeDescSets, uint32_t numWriteDescSets, const CopyDescriptorSet* copyDescSets, uint32_t numCopyDescSets);

	/// <summary>Gets the device dispatch table</summary>
	/// <returns>The device dispatch table</returns>
	inline const VkDeviceBindings& getVkBindings() const { return _vkBindings; }

	/// <summary>Gets the Transform feedback properties</summary>
	/// <returns>The physical device transform feedback properties</returns>
	inline const PhysicalDeviceTransformFeedbackProperties& getTransformFeedbackProperties() const { return _transformFeedbackProperties; }

	/// <summary>Gets the Transform feedback properties</summary>
	/// <returns>The physical device transform feedback properties</returns>
	inline PhysicalDeviceTransformFeedbackProperties getTransformFeedbackProperties() { return _transformFeedbackProperties; }

	/// <summary>Gets the Transform feedback features</summary>
	/// <returns>The physical device transform feedback features</returns>
	inline const PhysicalDeviceTransformFeedbackFeatures& getTransformFeedbackFeatures() const { return _transformFeedbackFeatures; }

	/// <summary>Gets the Transform feedback features</summary>
	/// <returns>The physical device transform feedback features</returns>
	inline PhysicalDeviceTransformFeedbackFeatures getTransformFeedbackFeatures() { return _transformFeedbackFeatures; }
};
} // namespace impl
} // namespace pvrvk
