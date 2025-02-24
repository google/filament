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

#include "VkPipeline.hpp"

#include "VkDestroy.hpp"
#include "VkDevice.hpp"
#include "VkPipelineCache.hpp"
#include "VkPipelineLayout.hpp"
#include "VkRenderPass.hpp"
#include "VkShaderModule.hpp"
#include "VkStringify.hpp"
#include "Pipeline/ComputeProgram.hpp"
#include "Pipeline/SpirvShader.hpp"

#include "marl/trace.h"

#include "spirv-tools/optimizer.hpp"

#include <iostream>

namespace {

// optimizeSpirv() applies and freezes specializations into constants, and runs spirv-opt.
sw::SpirvBinary optimizeSpirv(const vk::PipelineCache::SpirvBinaryKey &key)
{
	const sw::SpirvBinary &code = key.getBinary();
	const VkSpecializationInfo *specializationInfo = key.getSpecializationInfo();
	bool optimize = key.getOptimization();

	spvtools::Optimizer opt{ vk::SPIRV_VERSION };

	opt.SetMessageConsumer([](spv_message_level_t level, const char *source, const spv_position_t &position, const char *message) {
		switch(level)
		{
		case SPV_MSG_FATAL: sw::warn("SPIR-V FATAL: %d:%d %s\n", int(position.line), int(position.column), message);
		case SPV_MSG_INTERNAL_ERROR: sw::warn("SPIR-V INTERNAL_ERROR: %d:%d %s\n", int(position.line), int(position.column), message);
		case SPV_MSG_ERROR: sw::warn("SPIR-V ERROR: %d:%d %s\n", int(position.line), int(position.column), message);
		case SPV_MSG_WARNING: sw::warn("SPIR-V WARNING: %d:%d %s\n", int(position.line), int(position.column), message);
		case SPV_MSG_INFO: sw::trace("SPIR-V INFO: %d:%d %s\n", int(position.line), int(position.column), message);
		case SPV_MSG_DEBUG: sw::trace("SPIR-V DEBUG: %d:%d %s\n", int(position.line), int(position.column), message);
		default: sw::trace("SPIR-V MESSAGE: %d:%d %s\n", int(position.line), int(position.column), message);
		}
	});

	// If the pipeline uses specialization, apply the specializations before freezing
	if(specializationInfo)
	{
		std::unordered_map<uint32_t, std::vector<uint32_t>> specializations;
		const uint8_t *specializationData = static_cast<const uint8_t *>(specializationInfo->pData);

		for(uint32_t i = 0; i < specializationInfo->mapEntryCount; i++)
		{
			const VkSpecializationMapEntry &entry = specializationInfo->pMapEntries[i];
			const uint8_t *value_ptr = specializationData + entry.offset;
			std::vector<uint32_t> value(reinterpret_cast<const uint32_t *>(value_ptr),
			                            reinterpret_cast<const uint32_t *>(value_ptr + entry.size));
			specializations.emplace(entry.constantID, std::move(value));
		}

		opt.RegisterPass(spvtools::CreateSetSpecConstantDefaultValuePass(specializations));
	}

	if(optimize)
	{
		// Remove DontInline flags so the optimizer force-inlines all functions,
		// as we currently don't support OpFunctionCall (b/141246700).
		opt.RegisterPass(spvtools::CreateRemoveDontInlinePass());

		// Full optimization list taken from spirv-opt.
		opt.RegisterPerformancePasses();
	}

	spvtools::OptimizerOptions optimizerOptions = {};
#if defined(NDEBUG)
	optimizerOptions.set_run_validator(false);
#else
	optimizerOptions.set_run_validator(true);
	spvtools::ValidatorOptions validatorOptions = {};
	validatorOptions.SetScalarBlockLayout(true);            // VK_EXT_scalar_block_layout
	validatorOptions.SetUniformBufferStandardLayout(true);  // VK_KHR_uniform_buffer_standard_layout
	validatorOptions.SetAllowLocalSizeId(true);             // VK_KHR_maintenance4
	optimizerOptions.set_validator_options(validatorOptions);
#endif

	sw::SpirvBinary optimized;
	opt.Run(code.data(), code.size(), &optimized, optimizerOptions);
	ASSERT(optimized.size() > 0);

	if(false)
	{
		spvtools::SpirvTools core(vk::SPIRV_VERSION);
		std::string preOpt;
		core.Disassemble(code, &preOpt, SPV_BINARY_TO_TEXT_OPTION_NONE);
		std::string postOpt;
		core.Disassemble(optimized, &postOpt, SPV_BINARY_TO_TEXT_OPTION_NONE);
		std::cout << "PRE-OPT: " << preOpt << std::endl
		          << "POST-OPT: " << postOpt << std::endl;
	}

	return optimized;
}

std::shared_ptr<sw::ComputeProgram> createProgram(vk::Device *device, std::shared_ptr<sw::SpirvShader> shader, const vk::PipelineLayout *layout)
{
	MARL_SCOPED_EVENT("createProgram");

	vk::DescriptorSet::Bindings descriptorSets;  // TODO(b/129523279): Delay code generation until dispatch time.
	// TODO(b/119409619): use allocator.
	auto program = std::make_shared<sw::ComputeProgram>(device, shader, layout, descriptorSets);
	program->generate();
	program->finalize("ComputeProgram");

	return program;
}

class PipelineCreationFeedback
{
public:
	PipelineCreationFeedback(const VkGraphicsPipelineCreateInfo *pCreateInfo)
	    : pipelineCreationFeedback(GetPipelineCreationFeedback(pCreateInfo->pNext))
	{
		pipelineCreationBegins();
	}

	PipelineCreationFeedback(const VkComputePipelineCreateInfo *pCreateInfo)
	    : pipelineCreationFeedback(GetPipelineCreationFeedback(pCreateInfo->pNext))
	{
		pipelineCreationBegins();
	}

	~PipelineCreationFeedback()
	{
		pipelineCreationEnds();
	}

	void stageCreationBegins(uint32_t stage)
	{
		if(pipelineCreationFeedback && (stage < pipelineCreationFeedback->pipelineStageCreationFeedbackCount))
		{
			// Record stage creation begin time
			pipelineCreationFeedback->pPipelineStageCreationFeedbacks[stage].duration = now();
		}
	}

	void cacheHit(uint32_t stage)
	{
		if(pipelineCreationFeedback)
		{
			pipelineCreationFeedback->pPipelineCreationFeedback->flags |=
			    VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT;
			if(stage < pipelineCreationFeedback->pipelineStageCreationFeedbackCount)
			{
				pipelineCreationFeedback->pPipelineStageCreationFeedbacks[stage].flags |=
				    VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT;
			}
		}
	}

	void stageCreationEnds(uint32_t stage)
	{
		if(pipelineCreationFeedback && (stage < pipelineCreationFeedback->pipelineStageCreationFeedbackCount))
		{
			pipelineCreationFeedback->pPipelineStageCreationFeedbacks[stage].flags |=
			    VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT;
			pipelineCreationFeedback->pPipelineStageCreationFeedbacks[stage].duration =
			    now() - pipelineCreationFeedback->pPipelineStageCreationFeedbacks[stage].duration;
		}
	}

	void pipelineCreationError()
	{
		clear();
		pipelineCreationFeedback = nullptr;
	}

private:
	static const VkPipelineCreationFeedbackCreateInfo *GetPipelineCreationFeedback(const void *pNext)
	{
		return vk::GetExtendedStruct<VkPipelineCreationFeedbackCreateInfo>(pNext, VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO);
	}

	void pipelineCreationBegins()
	{
		if(pipelineCreationFeedback)
		{
			clear();

			// Record pipeline creation begin time
			pipelineCreationFeedback->pPipelineCreationFeedback->duration = now();
		}
	}

	void pipelineCreationEnds()
	{
		if(pipelineCreationFeedback)
		{
			pipelineCreationFeedback->pPipelineCreationFeedback->flags |=
			    VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT;
			pipelineCreationFeedback->pPipelineCreationFeedback->duration =
			    now() - pipelineCreationFeedback->pPipelineCreationFeedback->duration;
		}
	}

	void clear()
	{
		if(pipelineCreationFeedback)
		{
			// Clear all flags and durations
			pipelineCreationFeedback->pPipelineCreationFeedback->flags = 0;
			pipelineCreationFeedback->pPipelineCreationFeedback->duration = 0;
			for(uint32_t i = 0; i < pipelineCreationFeedback->pipelineStageCreationFeedbackCount; i++)
			{
				pipelineCreationFeedback->pPipelineStageCreationFeedbacks[i].flags = 0;
				pipelineCreationFeedback->pPipelineStageCreationFeedbacks[i].duration = 0;
			}
		}
	}

	uint64_t now()
	{
		return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	}

	const VkPipelineCreationFeedbackCreateInfo *pipelineCreationFeedback = nullptr;
};

bool getRobustBufferAccess(VkPipelineRobustnessBufferBehaviorEXT behavior, bool inheritRobustBufferAccess)
{
	// Based on behavior:
	// - <not provided>:
	//   * For pipelines, use device's robustBufferAccess
	//   * For shaders, use pipeline's robustBufferAccess
	//     Note that pipeline's robustBufferAccess is already set to device's if not overriden.
	// - Default: Use device's robustBufferAccess
	// - Disabled / Enabled: Override to disabled or enabled
	//
	// This function is passed "DEFAULT" when override is not provided, and
	// inheritRobustBufferAccess is appropriately set to the device or pipeline's
	// robustBufferAccess
	switch(behavior)
	{
	case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT:
		return inheritRobustBufferAccess;
	case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT:
		return false;
	case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT:
		return true;
	default:
		UNSUPPORTED("Unsupported robustness behavior");
		return true;
	}
}

bool getRobustBufferAccess(const VkPipelineRobustnessCreateInfoEXT *overrideRobustness, bool deviceRobustBufferAccess, bool inheritRobustBufferAccess)
{
	VkPipelineRobustnessBufferBehaviorEXT storageBehavior = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT;
	VkPipelineRobustnessBufferBehaviorEXT uniformBehavior = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT;
	VkPipelineRobustnessBufferBehaviorEXT vertexBehavior = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT;

	if(overrideRobustness)
	{
		storageBehavior = overrideRobustness->storageBuffers;
		uniformBehavior = overrideRobustness->uniformBuffers;
		vertexBehavior = overrideRobustness->vertexInputs;
		inheritRobustBufferAccess = deviceRobustBufferAccess;
	}

	bool storageRobustBufferAccess = getRobustBufferAccess(storageBehavior, inheritRobustBufferAccess);
	bool uniformRobustBufferAccess = getRobustBufferAccess(uniformBehavior, inheritRobustBufferAccess);
	bool vertexRobustBufferAccess = getRobustBufferAccess(vertexBehavior, inheritRobustBufferAccess);

	// Note: in the initial implementation, enabling robust access for any buffer enables it for
	// all.  TODO(b/185122256) split robustBufferAccess in the pipeline and shaders into three
	// categories and provide robustness for storage, uniform and vertex buffers accordingly.
	return storageRobustBufferAccess || uniformRobustBufferAccess || vertexRobustBufferAccess;
}

bool getPipelineRobustBufferAccess(const void *pNext, vk::Device *device)
{
	const VkPipelineRobustnessCreateInfoEXT *overrideRobustness = vk::GetExtendedStruct<VkPipelineRobustnessCreateInfoEXT>(pNext, VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO_EXT);
	const bool deviceRobustBufferAccess = device->getEnabledFeatures().robustBufferAccess;

	// For pipelines, there's no robustBufferAccess to inherit from.  Default and no-override
	// both lead to using the device's robustBufferAccess.
	return getRobustBufferAccess(overrideRobustness, deviceRobustBufferAccess, deviceRobustBufferAccess);
}

bool getPipelineStageRobustBufferAccess(const void *pNext, vk::Device *device, bool pipelineRobustBufferAccess)
{
	const VkPipelineRobustnessCreateInfoEXT *overrideRobustness = vk::GetExtendedStruct<VkPipelineRobustnessCreateInfoEXT>(pNext, VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO_EXT);
	const bool deviceRobustBufferAccess = device->getEnabledFeatures().robustBufferAccess;

	return getRobustBufferAccess(overrideRobustness, deviceRobustBufferAccess, pipelineRobustBufferAccess);
}

}  // anonymous namespace

namespace vk {
Pipeline::Pipeline(PipelineLayout *layout, Device *device, bool robustBufferAccess)
    : layout(layout)
    , device(device)
    , robustBufferAccess(robustBufferAccess)
{
	if(layout)
	{
		layout->incRefCount();
	}
}

void Pipeline::destroy(const VkAllocationCallbacks *pAllocator)
{
	destroyPipeline(pAllocator);

	if(layout)
	{
		vk::release(static_cast<VkPipelineLayout>(*layout), pAllocator);
	}
}

GraphicsPipeline::GraphicsPipeline(const VkGraphicsPipelineCreateInfo *pCreateInfo, void *mem, Device *device)
    : Pipeline(vk::Cast(pCreateInfo->layout), device, getPipelineRobustBufferAccess(pCreateInfo->pNext, device))
    , state(device, pCreateInfo, layout)
{
	// Either the vertex input interface comes from a pipeline library, or the
	// VkGraphicsPipelineCreateInfo itself.  Same with shaders.
	const auto *libraryCreateInfo = GetExtendedStruct<VkPipelineLibraryCreateInfoKHR>(pCreateInfo->pNext, VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR);
	bool vertexInputInterfaceInLibraries = false;
	bool fragmentOutputInterfaceInLibraries = false;
	if(libraryCreateInfo)
	{
		for(uint32_t i = 0; i < libraryCreateInfo->libraryCount; ++i)
		{
			const auto *library = static_cast<const vk::GraphicsPipeline *>(vk::Cast(libraryCreateInfo->pLibraries[i]));
			if(library->state.hasVertexInputInterfaceState())
			{
				inputs = library->inputs;
				vertexInputInterfaceInLibraries = true;
			}
			if(library->state.hasPreRasterizationState())
			{
				vertexShader = library->vertexShader;
			}
			if(library->state.hasFragmentState())
			{
				fragmentShader = library->fragmentShader;
			}
			if(library->state.hasFragmentOutputInterfaceState())
			{
				memcpy(attachments.indexToLocation, library->attachments.indexToLocation, sizeof(attachments.indexToLocation));
				memcpy(attachments.locationToIndex, library->attachments.locationToIndex, sizeof(attachments.locationToIndex));
				fragmentOutputInterfaceInLibraries = true;
			}
		}
	}
	if(state.hasVertexInputInterfaceState() && !vertexInputInterfaceInLibraries)
	{
		inputs.initialize(pCreateInfo->pVertexInputState, pCreateInfo->pDynamicState);
	}
	if(state.hasFragmentOutputInterfaceState() && !fragmentOutputInterfaceInLibraries)
	{
		const auto *colorMapping = GetExtendedStruct<VkRenderingAttachmentLocationInfoKHR>(pCreateInfo->pNext, VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO_KHR);
		if(colorMapping)
		{
			// Note that with VK_KHR_dynamic_rendering_local_read, if
			// VkRenderingAttachmentLocationInfoKHR is provided, setting an index to
			// VK_ATTACHMENT_UNUSED disables output for that attachment, even if write
			// mask is not explicitly disabled.
			for(uint32_t i = 0; i < sw::MAX_COLOR_BUFFERS; ++i)
			{
				attachments.indexToLocation[i] = VK_ATTACHMENT_UNUSED;
				attachments.locationToIndex[i] = VK_ATTACHMENT_UNUSED;
			}

			for(uint32_t i = 0; i < colorMapping->colorAttachmentCount; ++i)
			{
				const uint32_t location = colorMapping->pColorAttachmentLocations[i];
				if(location != VK_ATTACHMENT_UNUSED)
				{
					attachments.indexToLocation[i] = location;
					attachments.locationToIndex[location] = i;
				}
			}
		}
		else
		{
			for(uint32_t i = 0; i < sw::MAX_COLOR_BUFFERS; ++i)
			{
				attachments.indexToLocation[i] = i;
				attachments.locationToIndex[i] = i;
			}
		}
	}
}

void GraphicsPipeline::destroyPipeline(const VkAllocationCallbacks *pAllocator)
{
	vertexShader.reset();
	fragmentShader.reset();
}

size_t GraphicsPipeline::ComputeRequiredAllocationSize(const VkGraphicsPipelineCreateInfo *pCreateInfo)
{
	return 0;
}

VkGraphicsPipelineLibraryFlagsEXT GraphicsPipeline::GetGraphicsPipelineSubset(const VkGraphicsPipelineCreateInfo *pCreateInfo)
{
	const auto *libraryCreateInfo = vk::GetExtendedStruct<VkPipelineLibraryCreateInfoKHR>(pCreateInfo->pNext, VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR);
	const auto *graphicsLibraryCreateInfo = vk::GetExtendedStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(pCreateInfo->pNext, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT);

	if(graphicsLibraryCreateInfo)
	{
		return graphicsLibraryCreateInfo->flags;
	}

	// > If this structure is omitted, and either VkGraphicsPipelineCreateInfo::flags
	// > includes VK_PIPELINE_CREATE_LIBRARY_BIT_KHR or the
	// > VkGraphicsPipelineCreateInfo::pNext chain includes a VkPipelineLibraryCreateInfoKHR
	// > structure with a libraryCount greater than 0, it is as if flags is 0. Otherwise if
	// > this structure is omitted, it is as if flags includes all possible subsets of the
	// > graphics pipeline (i.e. a complete graphics pipeline).
	//
	// The above basically says that when a pipeline is created:
	// - If not a library and not created from libraries, it's a complete pipeline (i.e.
	//   Vulkan 1.0 pipelines)
	// - If only created from other libraries, no state is taken from
	//   VkGraphicsPipelineCreateInfo.
	//
	// Otherwise the behavior when creating a library from other libraries is that some
	// state is taken from VkGraphicsPipelineCreateInfo and some from the libraries.
	const bool isLibrary = (pCreateInfo->flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) != 0;
	if(isLibrary || (libraryCreateInfo && libraryCreateInfo->libraryCount > 0))
	{
		return 0;
	}

	return VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |
	       VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
	       VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
	       VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;
}

void GraphicsPipeline::getIndexBuffers(const vk::DynamicState &dynamicState, uint32_t count, uint32_t first, bool indexed, std::vector<std::pair<uint32_t, void *>> *indexBuffers) const
{
	const vk::VertexInputInterfaceState &vertexInputInterfaceState = state.getVertexInputInterfaceState();

	const VkPrimitiveTopology topology = vertexInputInterfaceState.hasDynamicTopology() ? dynamicState.primitiveTopology : vertexInputInterfaceState.getTopology();
	const bool hasPrimitiveRestartEnable = vertexInputInterfaceState.hasDynamicPrimitiveRestartEnable() ? dynamicState.primitiveRestartEnable : vertexInputInterfaceState.hasPrimitiveRestartEnable();
	indexBuffer.getIndexBuffers(topology, count, first, indexed, hasPrimitiveRestartEnable, indexBuffers);
}

bool GraphicsPipeline::preRasterizationContainsImageWrite() const
{
	return vertexShader.get() && vertexShader->containsImageWrite();
}

bool GraphicsPipeline::fragmentContainsImageWrite() const
{
	return fragmentShader.get() && fragmentShader->containsImageWrite();
}

void GraphicsPipeline::setShader(const VkShaderStageFlagBits &stage, const std::shared_ptr<sw::SpirvShader> spirvShader)
{
	switch(stage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT:
		ASSERT(vertexShader.get() == nullptr);
		vertexShader = spirvShader;
		break;

	case VK_SHADER_STAGE_FRAGMENT_BIT:
		ASSERT(fragmentShader.get() == nullptr);
		fragmentShader = spirvShader;
		break;

	default:
		UNSUPPORTED("Unsupported stage");
		break;
	}
}

const std::shared_ptr<sw::SpirvShader> GraphicsPipeline::getShader(const VkShaderStageFlagBits &stage) const
{
	switch(stage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT:
		return vertexShader;
	case VK_SHADER_STAGE_FRAGMENT_BIT:
		return fragmentShader;
	default:
		UNSUPPORTED("Unsupported stage");
		return fragmentShader;
	}
}

VkResult GraphicsPipeline::compileShaders(const VkAllocationCallbacks *pAllocator, const VkGraphicsPipelineCreateInfo *pCreateInfo, PipelineCache *pPipelineCache)
{
	PipelineCreationFeedback pipelineCreationFeedback(pCreateInfo);
	VkGraphicsPipelineLibraryFlagsEXT pipelineSubset = GetGraphicsPipelineSubset(pCreateInfo);
	const bool expectVertexShader = (pipelineSubset & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) != 0;
	const bool expectFragmentShader = (pipelineSubset & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) != 0;

	const auto *inputAttachmentMapping = GetExtendedStruct<VkRenderingInputAttachmentIndexInfoKHR>(pCreateInfo->pNext, VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR);

	for(uint32_t stageIndex = 0; stageIndex < pCreateInfo->stageCount; stageIndex++)
	{
		const VkPipelineShaderStageCreateInfo &stageInfo = pCreateInfo->pStages[stageIndex];

		// Ignore stages that don't exist in the pipeline library.
		if((stageInfo.stage == VK_SHADER_STAGE_VERTEX_BIT && !expectVertexShader) ||
		   (stageInfo.stage == VK_SHADER_STAGE_FRAGMENT_BIT && !expectFragmentShader))
		{
			continue;
		}

		pipelineCreationFeedback.stageCreationBegins(stageIndex);

		if((stageInfo.flags &
		    ~(VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT |
		      VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT)) != 0)
		{
			UNSUPPORTED("pStage->flags 0x%08X", int(stageInfo.flags));
		}

		const bool optimize = true;  // TODO(b/251802301): Don't optimize when debugging shaders.

		const ShaderModule *module = vk::Cast(stageInfo.module);

		// VK_EXT_graphics_pipeline_library allows VkShaderModuleCreateInfo to be chained to
		// VkPipelineShaderStageCreateInfo, which is used if stageInfo.module is
		// VK_NULL_HANDLE.
		VkShaderModule tempModule = {};
		if(stageInfo.module == VK_NULL_HANDLE)
		{
			const auto *moduleCreateInfo = vk::GetExtendedStruct<VkShaderModuleCreateInfo>(stageInfo.pNext,
			                                                                               VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
			ASSERT(moduleCreateInfo);
			VkResult createResult = vk::ShaderModule::Create(nullptr, moduleCreateInfo, &tempModule);
			if(createResult != VK_SUCCESS)
			{
				return createResult;
			}

			module = vk::Cast(tempModule);
		}

		const PipelineCache::SpirvBinaryKey key(module->getBinary(), stageInfo.pSpecializationInfo, robustBufferAccess, optimize);

		if((pCreateInfo->flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT) &&
		   (!pPipelineCache || !pPipelineCache->contains(key)))
		{
			pipelineCreationFeedback.pipelineCreationError();
			return VK_PIPELINE_COMPILE_REQUIRED_EXT;
		}

		sw::SpirvBinary spirv;

		if(pPipelineCache)
		{
			auto onCacheMiss = [&] { return optimizeSpirv(key); };
			auto onCacheHit = [&] { pipelineCreationFeedback.cacheHit(stageIndex); };
			spirv = pPipelineCache->getOrOptimizeSpirv(key, onCacheMiss, onCacheHit);
		}
		else
		{
			spirv = optimizeSpirv(key);

			// If the pipeline does not have specialization constants, there's a 1-to-1 mapping between the unoptimized and optimized SPIR-V,
			// so we should use a 1-to-1 mapping of the identifiers to avoid JIT routine recompiles.
			if(!key.getSpecializationInfo())
			{
				spirv.mapOptimizedIdentifier(key.getBinary());
			}
		}

		const bool stageRobustBufferAccess = getPipelineStageRobustBufferAccess(stageInfo.pNext, device, robustBufferAccess);

		// TODO(b/201798871): use allocator.
		auto shader = std::make_shared<sw::SpirvShader>(stageInfo.stage, stageInfo.pName, spirv,
		                                                vk::Cast(pCreateInfo->renderPass), pCreateInfo->subpass, inputAttachmentMapping, stageRobustBufferAccess);

		setShader(stageInfo.stage, shader);

		pipelineCreationFeedback.stageCreationEnds(stageIndex);

		if(tempModule != VK_NULL_HANDLE)
		{
			vk::destroy(tempModule, nullptr);
		}
	}

	return VK_SUCCESS;
}

ComputePipeline::ComputePipeline(const VkComputePipelineCreateInfo *pCreateInfo, void *mem, Device *device)
    : Pipeline(vk::Cast(pCreateInfo->layout), device, getPipelineRobustBufferAccess(pCreateInfo->pNext, device))
{
}

void ComputePipeline::destroyPipeline(const VkAllocationCallbacks *pAllocator)
{
	shader.reset();
	program.reset();
}

size_t ComputePipeline::ComputeRequiredAllocationSize(const VkComputePipelineCreateInfo *pCreateInfo)
{
	return 0;
}

VkResult ComputePipeline::compileShaders(const VkAllocationCallbacks *pAllocator, const VkComputePipelineCreateInfo *pCreateInfo, PipelineCache *pPipelineCache)
{
	PipelineCreationFeedback pipelineCreationFeedback(pCreateInfo);
	pipelineCreationFeedback.stageCreationBegins(0);

	auto &stage = pCreateInfo->stage;
	const ShaderModule *module = vk::Cast(stage.module);

	// VK_EXT_graphics_pipeline_library allows VkShaderModuleCreateInfo to be chained to
	// VkPipelineShaderStageCreateInfo, which is used if stageInfo.module is
	// VK_NULL_HANDLE.
	VkShaderModule tempModule = {};
	if(stage.module == VK_NULL_HANDLE)
	{
		const auto *moduleCreateInfo = vk::GetExtendedStruct<VkShaderModuleCreateInfo>(stage.pNext,
		                                                                               VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
		ASSERT(moduleCreateInfo);
		VkResult createResult = vk::ShaderModule::Create(nullptr, moduleCreateInfo, &tempModule);
		if(createResult != VK_SUCCESS)
		{
			return createResult;
		}

		module = vk::Cast(tempModule);
	}

	ASSERT(shader.get() == nullptr);
	ASSERT(program.get() == nullptr);

	const bool optimize = true;  // TODO(b/251802301): Don't optimize when debugging shaders.

	const PipelineCache::SpirvBinaryKey shaderKey(module->getBinary(), stage.pSpecializationInfo, robustBufferAccess, optimize);

	if((pCreateInfo->flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT) &&
	   (!pPipelineCache || !pPipelineCache->contains(shaderKey)))
	{
		pipelineCreationFeedback.pipelineCreationError();
		return VK_PIPELINE_COMPILE_REQUIRED_EXT;
	}

	sw::SpirvBinary spirv;

	if(pPipelineCache)
	{
		auto onCacheMiss = [&] { return optimizeSpirv(shaderKey); };
		auto onCacheHit = [&] { pipelineCreationFeedback.cacheHit(0); };
		spirv = pPipelineCache->getOrOptimizeSpirv(shaderKey, onCacheMiss, onCacheHit);
	}
	else
	{
		spirv = optimizeSpirv(shaderKey);

		// If the pipeline does not have specialization constants, there's a 1-to-1 mapping between the unoptimized and optimized SPIR-V,
		// so we should use a 1-to-1 mapping of the identifiers to avoid JIT routine recompiles.
		if(!shaderKey.getSpecializationInfo())
		{
			spirv.mapOptimizedIdentifier(shaderKey.getBinary());
		}
	}

	const bool stageRobustBufferAccess = getPipelineStageRobustBufferAccess(stage.pNext, device, robustBufferAccess);

	// TODO(b/201798871): use allocator.
	shader = std::make_shared<sw::SpirvShader>(stage.stage, stage.pName, spirv,
	                                           nullptr, 0, nullptr, stageRobustBufferAccess);

	const PipelineCache::ComputeProgramKey programKey(shader->getIdentifier(), layout->identifier);

	if(pPipelineCache)
	{
		program = pPipelineCache->getOrCreateComputeProgram(programKey, [&] {
			return createProgram(device, shader, layout);
		});
	}
	else
	{
		program = createProgram(device, shader, layout);
	}

	pipelineCreationFeedback.stageCreationEnds(0);

	return VK_SUCCESS;
}

void ComputePipeline::run(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                          uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                          const vk::DescriptorSet::Array &descriptorSetObjects,
                          const vk::DescriptorSet::Bindings &descriptorSets,
                          const vk::DescriptorSet::DynamicOffsets &descriptorDynamicOffsets,
                          const vk::Pipeline::PushConstantStorage &pushConstants)
{
	ASSERT_OR_RETURN(program != nullptr);
	program->run(
	    descriptorSetObjects, descriptorSets, descriptorDynamicOffsets, pushConstants,
	    baseGroupX, baseGroupY, baseGroupZ,
	    groupCountX, groupCountY, groupCountZ);
}

}  // namespace vk
