// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VK_DESCRIPTOR_SET_HPP_
#define VK_DESCRIPTOR_SET_HPP_

#include "VkObject.hpp"
#include "marl/mutex.h"

#include <array>
#include <cstdint>
#include <memory>

namespace vk {

class DescriptorSetLayout;
class Device;
class PipelineLayout;

struct alignas(16) DescriptorSetHeader
{
	DescriptorSetLayout *layout;
	marl::mutex mutex;
};

class alignas(16) DescriptorSet : public Object<DescriptorSet, VkDescriptorSet>
{
public:
	using Array = std::array<DescriptorSet *, vk::MAX_BOUND_DESCRIPTOR_SETS>;
	using Bindings = std::array<uint8_t *, vk::MAX_BOUND_DESCRIPTOR_SETS>;
	using DynamicOffsets = std::array<uint32_t, vk::MAX_DESCRIPTOR_SET_COMBINED_BUFFERS_DYNAMIC>;

	static void ContentsChanged(const Array &descriptorSets, const PipelineLayout *layout, Device *device);
	static void PrepareForSampling(const Array &descriptorSets, const PipelineLayout *layout, Device *device);

	uint8_t *getDataAddress();  // Returns a pointer to the descriptor payload following the header.

	DescriptorSetHeader header;

private:
	enum NotificationType
	{
		CONTENTS_CHANGED,
		PREPARE_FOR_SAMPLING
	};
	static void ParseDescriptors(const Array &descriptorSets, const PipelineLayout *layout, Device *device, NotificationType notificationType);
};

inline DescriptorSet *Cast(VkDescriptorSet object)
{
	return DescriptorSet::Cast(object);
}

}  // namespace vk

#endif  // VK_DESCRIPTOR_SET_HPP_
