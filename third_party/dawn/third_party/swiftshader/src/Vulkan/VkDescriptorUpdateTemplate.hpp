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

#ifndef VK_DESCRIPTOR_UPDATE_TEMPLATE_HPP_
#define VK_DESCRIPTOR_UPDATE_TEMPLATE_HPP_

#include "VkObject.hpp"

namespace vk {

class DescriptorSetLayout;
class Device;

class DescriptorUpdateTemplate : public Object<DescriptorUpdateTemplate, VkDescriptorUpdateTemplate>
{
public:
	DescriptorUpdateTemplate(const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo, void *mem);

	static size_t ComputeRequiredAllocationSize(const VkDescriptorUpdateTemplateCreateInfo *info);

	void updateDescriptorSet(Device *device, VkDescriptorSet descriptorSet, const void *pData);

private:
	uint32_t descriptorUpdateEntryCount = 0;
	VkDescriptorUpdateTemplateEntry *descriptorUpdateEntries = nullptr;
	DescriptorSetLayout *descriptorSetLayout = nullptr;
};

static inline DescriptorUpdateTemplate *Cast(VkDescriptorUpdateTemplate object)
{
	return DescriptorUpdateTemplate::Cast(object);
}

}  // namespace vk

#endif  // VK_DESCRIPTOR_UPDATE_TEMPLATE_HPP_
