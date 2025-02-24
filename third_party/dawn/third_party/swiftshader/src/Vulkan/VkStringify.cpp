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

#include "VkStringify.hpp"

#include "System/Debug.hpp"

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NAMESPACE vkhpp
#include <vulkan/vulkan.hpp>

namespace vk {

std::string Stringify(VkStructureType value)
{
#ifndef NDEBUG
	switch(static_cast<int>(value))
	{
	default:
		return vkhpp::to_string(static_cast<vkhpp::StructureType>(value));
	}
#else
	// In Release builds we avoid a dependency on vkhpp::to_string() to reduce binary size.
	return std::to_string(static_cast<int>(value));
#endif
}

std::string Stringify(VkFormat value)
{
#ifndef NDEBUG
	return vkhpp::to_string(static_cast<vkhpp::Format>(value));
#else
	// In Release builds we avoid a dependency on vkhpp::to_string() to reduce binary size.
	return std::to_string(static_cast<int>(value));
#endif
}

}  // namespace vk
