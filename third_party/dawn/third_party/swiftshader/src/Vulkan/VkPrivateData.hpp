// Copyright 2022 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VK_PRIVATE_DATA_HPP_
#define VK_PRIVATE_DATA_HPP_

#include "VkObject.hpp"

namespace vk {

class PrivateData : public Object<PrivateData, VkPrivateDataSlot>
{
public:
	PrivateData(const VkPrivateDataSlotCreateInfo *pCreateInfo, void *mem) {}

	static size_t ComputeRequiredAllocationSize(const VkPrivateDataSlotCreateInfo *pCreateInfo)
	{
		return 0;
	}
};

static inline PrivateData *Cast(VkPrivateDataSlot object)
{
	return PrivateData::Cast(object);
}

}  // namespace vk

#endif  // VK_PRIVATE_DATA_HPP_
