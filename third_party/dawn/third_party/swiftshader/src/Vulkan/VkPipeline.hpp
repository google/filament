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

#ifndef VK_PIPELINE_HPP_
#define VK_PIPELINE_HPP_

#include "Device/Context.hpp"
#include "Vulkan/VkPipelineCache.hpp"
#include <memory>

namespace sw {

class ComputeProgram;
class SpirvShader;

}  // namespace sw

namespace vk {

class ShaderModule;

class Pipeline
{
public:
	Pipeline(PipelineLayout *layout, Device *device, bool robustBufferAccess);
	virtual ~Pipeline() = default;

	operator VkPipeline()
	{
		return vk::TtoVkT<Pipeline, VkPipeline>(this);
	}

	static inline Pipeline *Cast(VkPipeline object)
	{
		return vk::VkTtoT<Pipeline, VkPipeline>(object);
	}

	void destroy(const VkAllocationCallbacks *pAllocator);

	virtual void destroyPipeline(const VkAllocationCallbacks *pAllocator) = 0;
#ifndef NDEBUG
	virtual VkPipelineBindPoint bindPoint() const = 0;
#endif

	PipelineLayout *getLayout() const
	{
		return layout;
	}

	struct PushConstantStorage
	{
		unsigned char data[vk::MAX_PUSH_CONSTANT_SIZE];
	};

protected:
	PipelineLayout *layout = nullptr;
	Device *const device;

	const bool robustBufferAccess = true;
};

class GraphicsPipeline : public Pipeline, public ObjectBase<GraphicsPipeline, VkPipeline>
{
public:
	GraphicsPipeline(const VkGraphicsPipelineCreateInfo *pCreateInfo,
	                 void *mem,
	                 Device *device);
	virtual ~GraphicsPipeline() = default;

	void destroyPipeline(const VkAllocationCallbacks *pAllocator) override;

#ifndef NDEBUG
	VkPipelineBindPoint bindPoint() const override
	{
		return VK_PIPELINE_BIND_POINT_GRAPHICS;
	}
#endif

	static size_t ComputeRequiredAllocationSize(const VkGraphicsPipelineCreateInfo *pCreateInfo);
	static VkGraphicsPipelineLibraryFlagsEXT GetGraphicsPipelineSubset(const VkGraphicsPipelineCreateInfo *pCreateInfo);

	VkResult compileShaders(const VkAllocationCallbacks *pAllocator, const VkGraphicsPipelineCreateInfo *pCreateInfo, PipelineCache *pipelineCache);

	GraphicsState getCombinedState(const DynamicState &ds) const { return state.combineStates(ds); }
	const GraphicsState &getState() const { return state; }

	void getIndexBuffers(const vk::DynamicState &dynamicState, uint32_t count, uint32_t first, bool indexed, std::vector<std::pair<uint32_t, void *>> *indexBuffers) const;

	IndexBuffer &getIndexBuffer() { return indexBuffer; }
	const IndexBuffer &getIndexBuffer() const { return indexBuffer; }
	Attachments &getAttachments() { return attachments; }
	const Attachments &getAttachments() const { return attachments; }
	Inputs &getInputs() { return inputs; }
	const Inputs &getInputs() const { return inputs; }

	bool preRasterizationContainsImageWrite() const;
	bool fragmentContainsImageWrite() const;

	const std::shared_ptr<sw::SpirvShader> getShader(const VkShaderStageFlagBits &stage) const;

private:
	void setShader(const VkShaderStageFlagBits &stage, const std::shared_ptr<sw::SpirvShader> spirvShader);
	std::shared_ptr<sw::SpirvShader> vertexShader;
	std::shared_ptr<sw::SpirvShader> fragmentShader;

	const GraphicsState state;

	IndexBuffer indexBuffer;
	Attachments attachments;
	Inputs inputs;
};

class ComputePipeline : public Pipeline, public ObjectBase<ComputePipeline, VkPipeline>
{
public:
	ComputePipeline(const VkComputePipelineCreateInfo *pCreateInfo, void *mem, Device *device);
	virtual ~ComputePipeline() = default;

	void destroyPipeline(const VkAllocationCallbacks *pAllocator) override;

#ifndef NDEBUG
	VkPipelineBindPoint bindPoint() const override
	{
		return VK_PIPELINE_BIND_POINT_COMPUTE;
	}
#endif

	static size_t ComputeRequiredAllocationSize(const VkComputePipelineCreateInfo *pCreateInfo);

	VkResult compileShaders(const VkAllocationCallbacks *pAllocator, const VkComputePipelineCreateInfo *pCreateInfo, PipelineCache *pipelineCache);

	void run(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
	         uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
	         const vk::DescriptorSet::Array &descriptorSetObjects,
	         const vk::DescriptorSet::Bindings &descriptorSets,
	         const vk::DescriptorSet::DynamicOffsets &descriptorDynamicOffsets,
	         const vk::Pipeline::PushConstantStorage &pushConstants);

protected:
	std::shared_ptr<sw::SpirvShader> shader;
	std::shared_ptr<sw::ComputeProgram> program;
};

static inline Pipeline *Cast(VkPipeline object)
{
	return Pipeline::Cast(object);
}

}  // namespace vk

#endif  // VK_PIPELINE_HPP_
