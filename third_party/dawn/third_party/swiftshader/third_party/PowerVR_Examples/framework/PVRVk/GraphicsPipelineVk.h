/*!
\brief The PVRVk GraphicsPipeline. This is an interface for a Vulkan VkPipeline
that was built for the VK_BINDING_POINT_GRAPHICS, separating it from the corresponding Compute pipeline
\file PVRVk/GraphicsPipelineVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/PipelineVk.h"
#include "PVRVk/PipelineConfigVk.h"
#include "PVRVk/DeviceVk.h"
namespace pvrvk {
/// <summary>This represents all the information needed to create a GraphicsPipeline. All items must have proper
/// values for a pipeline to be successfully created, but all those for which it is possible (except, for example,
/// Shaders and Vertex Formats) will have defaults same as their default values OpenGL ES graphics API.
///
/// NOTES: The folloowing are required
///  - at least one viewport & scissor
///  - renderpass
///  - pipeline layout</summary>
struct GraphicsPipelineCreateInfo : public PipelineCreateInfo<GraphicsPipeline>
{
public:
	PipelineDepthStencilStateCreateInfo depthStencil; //!< Depth and stencil buffer creation info
	PipelineColorBlendStateCreateInfo colorBlend; //!< Color blending and attachments info
	PipelineViewportStateCreateInfo viewport; //!< Viewport creation info
	PipelineRasterizationStateCreateInfo rasterizer; //!< Rasterizer configuration creation info
	PipelineVertexInputStateCreateInfo vertexInput; //!< Vertex Input creation info
	PipelineInputAssemblerStateCreateInfo inputAssembler; //!< Input Assembler creation info
	PipelineShaderStageCreateInfo vertexShader; //!< Vertex shader information
	PipelineShaderStageCreateInfo fragmentShader; //!< Fragment shader information
	PipelineShaderStageCreateInfo geometryShader; //!< Geometry shader information
	TesselationStageCreateInfo tesselationStates; //!< Tesselation Control and evaluation shader information
	PipelineMultisampleStateCreateInfo multiSample; //!< Multisampling information
	DynamicStatesCreateInfo dynamicStates; //!< Dynamic state Information
	RenderPass renderPass; //!< The RenderPass
	uint32_t subpass; //!< The subpass index

	GraphicsPipelineCreateInfo() : PipelineCreateInfo(), subpass(0) {}
};
namespace impl {

/// <summary>A Graphics Pipeline is a PVRVk adapter to a Vulkan Pipeline to a pipeline created for
/// VK_PIPELINE_BINDING_POINT_COMPUTE, and as such only supports the part of Vulkan that is
/// supported for Graphics pipelines.</summary>
class GraphicsPipeline_ : public Pipeline<GraphicsPipeline, GraphicsPipelineCreateInfo>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class GraphicsPipeline_;
	};

	static GraphicsPipeline constructShared(const DeviceWeakPtr& device, VkPipeline vkPipeline, const GraphicsPipelineCreateInfo& desc)
	{
		return std::make_shared<GraphicsPipeline_>(make_shared_enabler{}, device, vkPipeline, desc);
	}

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(GraphicsPipeline_)
	GraphicsPipeline_(make_shared_enabler, const DeviceWeakPtr& device, VkPipeline vkPipeline, const GraphicsPipelineCreateInfo& desc) : Pipeline(device, vkPipeline, desc) {}
	//!\endcond
};
} // namespace impl
} // namespace pvrvk
