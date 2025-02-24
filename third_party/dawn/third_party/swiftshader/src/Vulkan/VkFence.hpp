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

#ifndef VK_FENCE_HPP_
#define VK_FENCE_HPP_

#include "VkObject.hpp"
#include "System/Synchronization.hpp"

namespace vk {

class Fence : public Object<Fence, VkFence>
{
public:
	Fence(const VkFenceCreateInfo *pCreateInfo, void *mem)
	    : counted_event(std::make_shared<sw::CountedEvent>((pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) != 0))
	{}

	static size_t ComputeRequiredAllocationSize(const VkFenceCreateInfo *pCreateInfo)
	{
		return 0;
	}

	void reset()
	{
		counted_event->reset();
	}

	void complete()
	{
		counted_event->add();
		counted_event->done();
	}

	VkResult getStatus()
	{
		return counted_event->signalled() ? VK_SUCCESS : VK_NOT_READY;
	}

	VkResult wait()
	{
		counted_event->wait();
		return VK_SUCCESS;
	}

	template<class CLOCK, class DURATION>
	VkResult wait(const std::chrono::time_point<CLOCK, DURATION> &timeout)
	{
		return counted_event->wait(timeout) ? VK_SUCCESS : VK_TIMEOUT;
	}

	const std::shared_ptr<sw::CountedEvent> &getCountedEvent() const { return counted_event; };

private:
	Fence(const Fence &) = delete;

	const std::shared_ptr<sw::CountedEvent> counted_event;
};

static inline Fence *Cast(VkFence object)
{
	return Fence::Cast(object);
}

}  // namespace vk

#endif  // VK_FENCE_HPP_
