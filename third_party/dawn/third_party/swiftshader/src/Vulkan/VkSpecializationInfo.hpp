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

#ifndef VK_SPECIALIZATION_INFO_HPP_
#define VK_SPECIALIZATION_INFO_HPP_

#include "VkObject.hpp"

#include <memory>

namespace vk {

struct SpecializationInfo
{
	SpecializationInfo(const VkSpecializationInfo *specializationInfo);
	SpecializationInfo(const SpecializationInfo &copy);

	~SpecializationInfo();

	bool operator<(const SpecializationInfo &specializationInfo) const;

	const VkSpecializationInfo *get() const
	{
		return (info.mapEntryCount > 0) ? &info : nullptr;
	}

private:
	VkSpecializationInfo info = {};
};

}  // namespace vk

#endif  // VK_SPECIALIZATION_INFO_HPP_
