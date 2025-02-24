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

#ifndef VK_SHADER_MODULE_HPP_
#define VK_SHADER_MODULE_HPP_

#include "VkObject.hpp"
#include "Pipeline/SpirvBinary.hpp"

namespace vk {

class ShaderModule : public Object<ShaderModule, VkShaderModule>
{
public:
	ShaderModule(const VkShaderModuleCreateInfo *pCreateInfo, void *mem);

	static size_t ComputeRequiredAllocationSize(const VkShaderModuleCreateInfo *pCreateInfo);
	const sw::SpirvBinary &getBinary() const { return binary; }

private:
	sw::SpirvBinary binary;
};

static inline ShaderModule *Cast(VkShaderModule object)
{
	return ShaderModule::Cast(object);
}

}  // namespace vk

#endif  // VK_SHADER_MODULE_HPP_
