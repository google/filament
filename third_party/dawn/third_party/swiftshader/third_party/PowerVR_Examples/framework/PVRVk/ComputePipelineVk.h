/*!
\brief The PVRVk Compute Pipeline class, an interface to a VkPipeline that has been
created for the VK_PIPELINE_BINDING_POINT_COMPUTE
\file PVRVk/ComputePipelineVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/PipelineVk.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/PipelineConfigVk.h"
namespace pvrvk {
/// <summary>Compute pipeline create parameters.</summary>
struct ComputePipelineCreateInfo : public PipelineCreateInfo<ComputePipeline>
{
public:
	PipelineShaderStageCreateInfo computeShader; //!< Compute shader information

	ComputePipelineCreateInfo() : PipelineCreateInfo() {}
};

namespace impl {
/// <summary>Vulkan Computepipeline wrapper<summary>
class ComputePipeline_ : public Pipeline<ComputePipeline, ComputePipelineCreateInfo>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class ComputePipeline_;
	};

	static ComputePipeline constructShared(const DeviceWeakPtr& device, VkPipeline vkPipeline, const ComputePipelineCreateInfo& desc)
	{
		return std::make_shared<ComputePipeline_>(make_shared_enabler{}, device, vkPipeline, desc);
	}

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(ComputePipeline_)
	ComputePipeline_(make_shared_enabler, const DeviceWeakPtr& device, VkPipeline vkPipeline, const ComputePipelineCreateInfo& desc) : Pipeline(device, vkPipeline, desc) {}
	//!\endcond
};
} // namespace impl
} // namespace pvrvk
