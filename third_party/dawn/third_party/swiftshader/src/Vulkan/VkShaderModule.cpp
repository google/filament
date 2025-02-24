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

#include "VkShaderModule.hpp"

#include "spirv-tools/libspirv.hpp"

#include <cstring>

namespace vk {

ShaderModule::ShaderModule(const VkShaderModuleCreateInfo *pCreateInfo, void *mem)
    : binary(pCreateInfo->pCode, pCreateInfo->codeSize / sizeof(uint32_t))
{
#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
	spvtools::SpirvTools spirvTools(SPIRV_VERSION);
	spirvTools.SetMessageConsumer([](spv_message_level_t level, const char *source, const spv_position_t &position, const char *message) {
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

	spvtools::ValidatorOptions validatorOptions = {};
	validatorOptions.SetScalarBlockLayout(true);            // VK_EXT_scalar_block_layout
	validatorOptions.SetUniformBufferStandardLayout(true);  // VK_KHR_uniform_buffer_standard_layout
	validatorOptions.SetAllowLocalSizeId(true);             // VK_KHR_maintenance4

	ASSERT(spirvTools.Validate(binary.data(), binary.size(), validatorOptions));  // The SPIR-V code passed to vkCreateShaderModule must be valid (b/158228522)
#endif
}

size_t ShaderModule::ComputeRequiredAllocationSize(const VkShaderModuleCreateInfo *pCreateInfo)
{
	return 0;
}

}  // namespace vk
