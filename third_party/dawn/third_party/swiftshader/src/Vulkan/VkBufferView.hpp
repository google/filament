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

#ifndef VK_BUFFER_VIEW_HPP_
#define VK_BUFFER_VIEW_HPP_

#include "VkFormat.hpp"
#include "VkImageView.hpp"
#include "VkObject.hpp"

namespace vk {

class Buffer;

class BufferView : public Object<BufferView, VkBufferView>
{
public:
	BufferView(const VkBufferViewCreateInfo *pCreateInfo, void *mem);

	static size_t ComputeRequiredAllocationSize(const VkBufferViewCreateInfo *pCreateInfo)
	{
		return 0;
	}

	void *getPointer() const;
	uint32_t getElementCount() const { return static_cast<uint32_t>(range / Format(format).bytes()); }
	uint32_t getRangeInBytes() const { return static_cast<uint32_t>(range); }
	VkFormat getFormat() const { return format; }

	const Identifier id;

private:
	Buffer *buffer;
	VkFormat format;
	VkDeviceSize offset;
	VkDeviceSize range;
};

static inline BufferView *Cast(VkBufferView object)
{
	return BufferView::Cast(object);
}

}  // namespace vk

#endif  // VK_BUFFER_VIEW_HPP_
