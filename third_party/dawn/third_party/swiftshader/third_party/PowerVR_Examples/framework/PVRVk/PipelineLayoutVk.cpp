/*!
\brief Function definitions for the Pipeline Layout class
\file PVRVk/SamplerVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/DeviceVk.h"
#include <vector>
namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
PipelineLayout_::PipelineLayout_(make_shared_enabler, const DeviceWeakPtr& device, const PipelineLayoutCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils()
{
	VkPipelineLayoutCreateInfo pipeLayoutInfo = {};
	VkDescriptorSetLayout bindings[4] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
	_createInfo = createInfo;
	uint32_t numLayouts = 0;
	for (uint32_t i = 0; i < createInfo.getNumDescriptorSetLayouts(); ++i)
	{
		auto& ref = createInfo.getDescriptorSetLayout(i);
		if (ref)
		{
			bindings[i] = ref->getVkHandle();
			++numLayouts;
		}
		else
		{
			throw ErrorValidationFailedEXT("PipelineLayout constructor: Push constant range index must be consecutive and have valid data");
		}
	}

	pipeLayoutInfo.sType = static_cast<VkStructureType>(StructureType::e_PIPELINE_LAYOUT_CREATE_INFO);
	pipeLayoutInfo.pSetLayouts = bindings;
	pipeLayoutInfo.setLayoutCount = numLayouts;
	pvrvk::ArrayOrVector<VkPushConstantRange, 2> vkPushConstantRange(createInfo.getNumPushConstantRanges());
	for (uint32_t i = 0; i < createInfo.getNumPushConstantRanges(); ++i)
	{
		if (createInfo.getPushConstantRange(i).getSize() == 0)
		{ throw ErrorValidationFailedEXT("PipelineLayout constructor: Push constant range index must be consecutive and have valid data"); }

		vkPushConstantRange[i].stageFlags = static_cast<VkShaderStageFlags>(createInfo.getPushConstantRange(i).getStageFlags());
		vkPushConstantRange[i].offset = createInfo.getPushConstantRange(i).getOffset();
		vkPushConstantRange[i].size = createInfo.getPushConstantRange(i).getSize();
	}
	pipeLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(createInfo.getNumPushConstantRanges());
	pipeLayoutInfo.pPushConstantRanges = vkPushConstantRange.get();
	vkThrowIfFailed(getDevice()->getVkBindings().vkCreatePipelineLayout(getDevice()->getVkHandle(), &pipeLayoutInfo, NULL, &_vkHandle),
		"PipelineLayout constructor: Failed to create pipeline layout");
}
//!\endcond
} // namespace impl
} // namespace pvrvk
