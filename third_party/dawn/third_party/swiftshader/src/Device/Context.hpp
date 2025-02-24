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

#ifndef vk_Context_hpp
#define vk_Context_hpp

#include "Config.hpp"
#include "Memset.hpp"
#include "Stream.hpp"
#include "System/Types.hpp"
#include "Vulkan/VkDescriptorSet.hpp"
#include "Vulkan/VkFormat.hpp"

#include <vector>

namespace vk {

class Buffer;
class Device;
class ImageView;
class PipelineLayout;
class RenderPass;

struct InputsDynamicStateFlags
{
	bool dynamicVertexInputBindingStride : 1;
        bool dynamicVertexInput : 1;
};

// Note: The split between Inputs and VertexInputInterfaceState is mostly superficial.  The state
// (be it dynamic or static) in Inputs should have been mostly a part of VertexInputInterfaceState.
// Changing that requires some surgery.
struct VertexInputInterfaceDynamicStateFlags
{
	bool dynamicPrimitiveRestartEnable : 1;
	bool dynamicPrimitiveTopology : 1;
};

struct PreRasterizationDynamicStateFlags
{
	bool dynamicLineWidth : 1;
	bool dynamicDepthBias : 1;
	bool dynamicDepthBiasEnable : 1;
	bool dynamicCullMode : 1;
	bool dynamicFrontFace : 1;
	bool dynamicViewport : 1;
	bool dynamicScissor : 1;
	bool dynamicViewportWithCount : 1;
	bool dynamicScissorWithCount : 1;
	bool dynamicRasterizerDiscardEnable : 1;
};

struct FragmentDynamicStateFlags
{
	bool dynamicDepthTestEnable : 1;
	bool dynamicDepthWriteEnable : 1;
	bool dynamicDepthBoundsTestEnable : 1;
	bool dynamicDepthBounds : 1;
	bool dynamicDepthCompareOp : 1;
	bool dynamicStencilTestEnable : 1;
	bool dynamicStencilOp : 1;
	bool dynamicStencilCompareMask : 1;
	bool dynamicStencilWriteMask : 1;
	bool dynamicStencilReference : 1;
};

struct FragmentOutputInterfaceDynamicStateFlags
{
	bool dynamicBlendConstants : 1;
};

struct DynamicStateFlags
{
    // Note: InputsDynamicStateFlags is kept local to Inputs
	VertexInputInterfaceDynamicStateFlags vertexInputInterface;
	PreRasterizationDynamicStateFlags preRasterization;
	FragmentDynamicStateFlags fragment;
	FragmentOutputInterfaceDynamicStateFlags fragmentOutputInterface;
};

struct VertexInputBinding
{
	Buffer *buffer = nullptr;
	VkDeviceSize offset = 0;
	VkDeviceSize size = 0;
};

struct IndexBuffer
{
	inline VkIndexType getIndexType() const { return indexType; }
	void setIndexBufferBinding(const VertexInputBinding &indexBufferBinding, VkIndexType type);
	void getIndexBuffers(VkPrimitiveTopology topology, uint32_t count, uint32_t first, bool indexed, bool hasPrimitiveRestartEnable, std::vector<std::pair<uint32_t, void *>> *indexBuffers) const;

private:
	uint32_t bytesPerIndex() const;

	VertexInputBinding binding;
	VkIndexType indexType;
};

struct Attachments
{
	ImageView *colorBuffer[sw::MAX_COLOR_BUFFERS] = {};
	ImageView *depthBuffer = nullptr;
	ImageView *stencilBuffer = nullptr;

	// VK_KHR_dynamic_rendering_local_read allows color locations to be mapped to the render
	// pass attachments, but blend and other state is not affected by this map.  The image views
	// placed in colorBuffer are indexed by "location" (i.e the decoration in the shader), and
	// the following maps facilitate the association between the attachment-specific state and
	// the location-indexed color buffers.
	uint32_t indexToLocation[sw::MAX_COLOR_BUFFERS] = {};
	uint32_t locationToIndex[sw::MAX_COLOR_BUFFERS] = {};

	VkFormat colorFormat(int location) const;
	VkFormat depthFormat() const;
	VkFormat depthStencilFormat() const;
};

struct DynamicState;
struct Inputs
{
	void initialize(const VkPipelineVertexInputStateCreateInfo *vertexInputState, const VkPipelineDynamicStateCreateInfo *dynamicStateCreateInfo);

	void updateDescriptorSets(const DescriptorSet::Array &dso,
	                          const DescriptorSet::Bindings &ds,
	                          const DescriptorSet::DynamicOffsets &ddo);
	inline const DescriptorSet::Array &getDescriptorSetObjects() const { return descriptorSetObjects; }
	inline const DescriptorSet::Bindings &getDescriptorSets() const { return descriptorSets; }
	inline const DescriptorSet::DynamicOffsets &getDescriptorDynamicOffsets() const { return descriptorDynamicOffsets; }
	inline const sw::Stream &getStream(uint32_t i) const { return stream[i]; }

	void bindVertexInputs(int firstInstance);
	void setVertexInputBinding(const VertexInputBinding vertexInputBindings[], const DynamicState &dynamicState);
	void advanceInstanceAttributes();
	VkDeviceSize getVertexStride(uint32_t i) const;
	VkDeviceSize getInstanceStride(uint32_t i) const;

private:
	InputsDynamicStateFlags dynamicStateFlags = {};
	VertexInputBinding vertexInputBindings[MAX_VERTEX_INPUT_BINDINGS] = {};
	DescriptorSet::Array descriptorSetObjects = {};
	DescriptorSet::Bindings descriptorSets = {};
	DescriptorSet::DynamicOffsets descriptorDynamicOffsets = {};
	sw::Stream stream[sw::MAX_INTERFACE_COMPONENTS / 4];
};

struct MultisampleState
{
	bool sampleShadingEnable = false;
	bool alphaToCoverage = false;

	int sampleCount = 0;
	unsigned int multiSampleMask = 0;
	float minSampleShading = 0.0f;

	void set(const VkPipelineMultisampleStateCreateInfo *multisampleState);
};

struct BlendState : sw::Memset<BlendState>
{
	BlendState()
	    : Memset(this, 0)
	{}

	BlendState(bool alphaBlendEnable,
	           VkBlendFactor sourceBlendFactor,
	           VkBlendFactor destBlendFactor,
	           VkBlendOp blendOperation,
	           VkBlendFactor sourceBlendFactorAlpha,
	           VkBlendFactor destBlendFactorAlpha,
	           VkBlendOp blendOperationAlpha)
	    : Memset(this, 0)
	    , alphaBlendEnable(alphaBlendEnable)
	    , sourceBlendFactor(sourceBlendFactor)
	    , destBlendFactor(destBlendFactor)
	    , blendOperation(blendOperation)
	    , sourceBlendFactorAlpha(sourceBlendFactorAlpha)
	    , destBlendFactorAlpha(destBlendFactorAlpha)
	    , blendOperationAlpha(blendOperationAlpha)
	{}

	bool alphaBlendEnable;
	VkBlendFactor sourceBlendFactor;
	VkBlendFactor destBlendFactor;
	VkBlendOp blendOperation;
	VkBlendFactor sourceBlendFactorAlpha;
	VkBlendFactor destBlendFactorAlpha;
	VkBlendOp blendOperationAlpha;
};

struct DynamicVertexInputBindingState
{
	VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkDeviceSize stride = 0;
	unsigned int divisor = 0;
};

struct DynamicVertexInputAttributeState
{
	VkFormat format = VK_FORMAT_UNDEFINED;
	unsigned int offset = 0;
	unsigned int binding = 0;
};

struct DynamicState
{
	VkViewport viewport = {};
	VkRect2D scissor = {};
	sw::float4 blendConstants = {};
	float depthBiasConstantFactor = 0.0f;
	float depthBiasClamp = 0.0f;
	float depthBiasSlopeFactor = 0.0f;
	float minDepthBounds = 0.0f;
	float maxDepthBounds = 0.0f;
	float lineWidth = 0.0f;

	VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
	VkBool32 depthBoundsTestEnable = VK_FALSE;
	VkCompareOp depthCompareOp = VK_COMPARE_OP_NEVER;
	VkBool32 depthTestEnable = VK_FALSE;
	VkBool32 depthWriteEnable = VK_FALSE;
	VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	uint32_t scissorCount = 0;
	VkRect2D scissors[vk::MAX_VIEWPORTS] = {};
	VkStencilFaceFlags faceMask = (VkStencilFaceFlags)0;
	VkStencilOpState frontStencil = {};
	VkStencilOpState backStencil = {};
	VkBool32 stencilTestEnable = VK_FALSE;
	uint32_t viewportCount = 0;
	VkViewport viewports[vk::MAX_VIEWPORTS] = {};
	VkBool32 rasterizerDiscardEnable = VK_FALSE;
	VkBool32 depthBiasEnable = VK_FALSE;
	VkBool32 primitiveRestartEnable = VK_FALSE;
	DynamicVertexInputBindingState vertexInputBindings[MAX_VERTEX_INPUT_BINDINGS];
	DynamicVertexInputAttributeState vertexInputAttributes[sw::MAX_INTERFACE_COMPONENTS / 4];
};

struct VertexInputInterfaceState
{
	void initialize(const VkPipelineVertexInputStateCreateInfo *vertexInputState,
	                const VkPipelineInputAssemblyStateCreateInfo *inputAssemblyState,
	                const DynamicStateFlags &allDynamicStateFlags);

	void applyState(const DynamicState &dynamicState);

	inline VkPrimitiveTopology getTopology() const { return topology; }
	inline bool hasPrimitiveRestartEnable() const { return primitiveRestartEnable; }

	inline bool hasDynamicTopology() const { return dynamicStateFlags.dynamicPrimitiveTopology; }
	inline bool hasDynamicPrimitiveRestartEnable() const { return dynamicStateFlags.dynamicPrimitiveRestartEnable; }

	bool isDrawPoint(bool polygonModeAware, VkPolygonMode polygonMode) const;
	bool isDrawLine(bool polygonModeAware, VkPolygonMode polygonMode) const;
	bool isDrawTriangle(bool polygonModeAware, VkPolygonMode polygonMode) const;

private:
	VertexInputInterfaceDynamicStateFlags dynamicStateFlags = {};

	bool primitiveRestartEnable = false;

	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
};

struct PreRasterizationState
{
	void initialize(const vk::Device *device,
	                const PipelineLayout *layout,
	                const VkPipelineViewportStateCreateInfo *viewportState,
	                const VkPipelineRasterizationStateCreateInfo *rasterizationState,
	                const vk::RenderPass *renderPass, uint32_t subpassIndex,
	                const VkPipelineRenderingCreateInfo *rendering,
	                const DynamicStateFlags &allDynamicStateFlags);

	inline const PipelineLayout *getPipelineLayout() const { return pipelineLayout; }
	inline void overridePipelineLayout(const PipelineLayout *linkedLayout) { pipelineLayout = linkedLayout; }

	void applyState(const DynamicState &dynamicState);

	inline VkCullModeFlags getCullMode() const { return cullMode; }
	inline VkFrontFace getFrontFace() const { return frontFace; }
	inline VkPolygonMode getPolygonMode() const { return polygonMode; }
	inline VkProvokingVertexModeEXT getProvokingVertexMode() const { return provokingVertexMode; }
	inline VkLineRasterizationModeEXT getLineRasterizationMode() const { return lineRasterizationMode; }

	inline bool hasRasterizerDiscard() const { return rasterizerDiscard; }

	inline float getConstantDepthBias() const { return depthBiasEnable ? constantDepthBias : 0; }
	inline float getSlopeDepthBias() const { return depthBiasEnable ? slopeDepthBias : 0; }
	inline float getDepthBiasClamp() const { return depthBiasEnable ? depthBiasClamp : 0; }

	inline bool hasDepthRangeUnrestricted() const { return depthRangeUnrestricted; }
	inline bool getDepthClampEnable() const { return depthClampEnable; }
	inline bool getDepthClipEnable() const { return depthClipEnable; }
	inline bool getDepthClipNegativeOneToOne() const { return depthClipNegativeOneToOne; }

	inline float getLineWidth() const { return lineWidth; }

	inline const VkRect2D &getScissor() const { return scissor; }
	inline const VkViewport &getViewport() const { return viewport; }

private:
	const PipelineLayout *pipelineLayout = nullptr;

	PreRasterizationDynamicStateFlags dynamicStateFlags = {};

	bool rasterizerDiscard = false;
	bool depthClampEnable = false;
	bool depthClipEnable = false;
	bool depthClipNegativeOneToOne = false;
	bool depthBiasEnable = false;
	bool depthRangeUnrestricted = false;

	VkCullModeFlags cullMode = 0;
	VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
	VkProvokingVertexModeEXT provokingVertexMode = VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT;
	VkLineRasterizationModeEXT lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;

	float depthBiasClamp = 0.0f;
	float constantDepthBias = 0.0f;
	float slopeDepthBias = 0.0f;

	float lineWidth = 0.0f;

	VkRect2D scissor = {};
	VkViewport viewport = {};
};

struct FragmentState
{
	void initialize(const PipelineLayout *layout,
	                const VkPipelineDepthStencilStateCreateInfo *depthStencilState,
	                const vk::RenderPass *renderPass, uint32_t subpassIndex,
	                const VkPipelineRenderingCreateInfo *rendering,
	                const DynamicStateFlags &allDynamicStateFlags);

	inline const PipelineLayout *getPipelineLayout() const { return pipelineLayout; }
	inline void overridePipelineLayout(const PipelineLayout *linkedLayout) { pipelineLayout = linkedLayout; }

	void applyState(const DynamicState &dynamicState);

	inline VkStencilOpState getFrontStencil() const { return frontStencil; }
	inline VkStencilOpState getBackStencil() const { return backStencil; }

	inline float getMinDepthBounds() const { return minDepthBounds; }
	inline float getMaxDepthBounds() const { return maxDepthBounds; }

	inline VkCompareOp getDepthCompareMode() const { return depthCompareMode; }

	bool depthWriteActive(const Attachments &attachments) const;
	bool depthTestActive(const Attachments &attachments) const;
	bool stencilActive(const Attachments &attachments) const;
	bool depthBoundsTestActive(const Attachments &attachments) const;

private:
	void setDepthStencilState(const VkPipelineDepthStencilStateCreateInfo *depthStencilState);

	const PipelineLayout *pipelineLayout = nullptr;

	FragmentDynamicStateFlags dynamicStateFlags = {};

	bool depthTestEnable = false;
	bool depthWriteEnable = false;
	bool depthBoundsTestEnable = false;
	bool stencilEnable = false;

	float minDepthBounds = 0.0f;
	float maxDepthBounds = 0.0f;

	VkCompareOp depthCompareMode = VK_COMPARE_OP_NEVER;

	VkStencilOpState frontStencil = {};
	VkStencilOpState backStencil = {};

	// Note: if a pipeline library is created with the fragment state only, and sample shading
	// is enabled or a render pass is provided, VkPipelineMultisampleStateCreateInfo must be
	// provided.  This must identically match with the one provided for the fragment output
	// interface library.
	//
	// Currently, SwiftShader can always use the copy provided and stored in
	// FragmentOutputInterfaceState.  If a future optimization requires access to this state in
	// a pipeline library without fragment output interface, a copy of MultisampleState can be
	// placed here and initialized under the above condition.
	//
	// Ref: https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap10.html#pipeline-graphics-subsets
};

struct FragmentOutputInterfaceState
{
	void initialize(const VkPipelineColorBlendStateCreateInfo *colorBlendState,
	                const VkPipelineMultisampleStateCreateInfo *multisampleState,
	                const vk::RenderPass *renderPass, uint32_t subpassIndex,
	                const VkPipelineRenderingCreateInfo *rendering,
	                const DynamicStateFlags &allDynamicStateFlags);

	void applyState(const DynamicState &dynamicState);

	inline unsigned int getMultiSampleMask() const { return multisample.multiSampleMask; }
	inline int getSampleCount() const { return multisample.sampleCount; }
	inline bool hasSampleShadingEnabled() const { return multisample.sampleShadingEnable; }
	inline float getMinSampleShading() const { return multisample.minSampleShading; }
	inline bool hasAlphaToCoverage() const { return multisample.alphaToCoverage; }

	inline const sw::float4 &getBlendConstants() const { return blendConstants; }

	// The following take the attachment "location", which may not be the same as the index in
	// the attachment list with VK_KHR_dynamic_rendering_local_read.
	BlendState getBlendState(int location, const Attachments &attachments, bool fragmentContainsKill) const;
	int colorWriteActive(int location, const Attachments &attachments) const;

private:
	void setColorBlendState(const VkPipelineColorBlendStateCreateInfo *colorBlendState);

	VkBlendFactor blendFactor(VkBlendOp blendOperation, VkBlendFactor blendFactor) const;
	VkBlendOp blendOperation(VkBlendOp blendOperation, VkBlendFactor sourceBlendFactor, VkBlendFactor destBlendFactor, vk::Format format) const;

	bool alphaBlendActive(int location, const Attachments &attachments, bool fragmentContainsKill) const;
	bool colorWriteActive(const Attachments &attachments) const;

	int colorWriteMask[sw::MAX_COLOR_BUFFERS] = {};  // RGBA

	FragmentOutputInterfaceDynamicStateFlags dynamicStateFlags = {};

	sw::float4 blendConstants = {};
	BlendState blendState[sw::MAX_COLOR_BUFFERS] = {};

	MultisampleState multisample;
};

struct GraphicsState
{
	GraphicsState(const Device *device, const VkGraphicsPipelineCreateInfo *pCreateInfo, const PipelineLayout *layout);

	GraphicsState combineStates(const DynamicState &dynamicState) const;

	bool hasVertexInputInterfaceState() const
	{
		return (validSubset & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT) != 0;
	}
	bool hasPreRasterizationState() const
	{
		return (validSubset & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) != 0;
	}
	bool hasFragmentState() const
	{
		return (validSubset & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) != 0;
	}
	bool hasFragmentOutputInterfaceState() const
	{
		return (validSubset & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) != 0;
	}

	const VertexInputInterfaceState &getVertexInputInterfaceState() const
	{
		ASSERT(hasVertexInputInterfaceState());
		return vertexInputInterfaceState;
	}
	const PreRasterizationState &getPreRasterizationState() const
	{
		ASSERT(hasPreRasterizationState());
		return preRasterizationState;
	}
	const FragmentState &getFragmentState() const
	{
		ASSERT(hasFragmentState());
		return fragmentState;
	}
	const FragmentOutputInterfaceState &getFragmentOutputInterfaceState() const
	{
		ASSERT(hasFragmentOutputInterfaceState());
		return fragmentOutputInterfaceState;
	}

private:
	// The four subsets of a graphics pipeline as described in the spec.  With
	// VK_EXT_graphics_pipeline_library, a number of these may be valid.
	VertexInputInterfaceState vertexInputInterfaceState;
	PreRasterizationState preRasterizationState;
	FragmentState fragmentState;
	FragmentOutputInterfaceState fragmentOutputInterfaceState;

	VkGraphicsPipelineLibraryFlagsEXT validSubset = 0;
};

}  // namespace vk

#endif  // vk_Context_hpp
