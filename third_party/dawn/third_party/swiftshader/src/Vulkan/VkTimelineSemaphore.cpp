// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
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

#include "VkTimelineSemaphore.hpp"
#include "VkSemaphore.hpp"

#include "marl/blockingcall.h"
#include "marl/conditionvariable.h"

#include <vector>

namespace vk {

TimelineSemaphore::TimelineSemaphore(const VkSemaphoreCreateInfo *pCreateInfo, void *mem, const VkAllocationCallbacks *pAllocator)
    : Semaphore(VK_SEMAPHORE_TYPE_TIMELINE)
{
	SemaphoreCreateInfo info(pCreateInfo);
	ASSERT(info.semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE);
	type = info.semaphoreType;
	counter = info.initialPayload;
}

TimelineSemaphore::TimelineSemaphore()
    : Semaphore(VK_SEMAPHORE_TYPE_TIMELINE)
    , counter(0)
{
	type = VK_SEMAPHORE_TYPE_TIMELINE;
}

size_t TimelineSemaphore::ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo *pCreateInfo)
{
	return 0;
}

void TimelineSemaphore::destroy(const VkAllocationCallbacks *pAllocator)
{
}

void TimelineSemaphore::signal(uint64_t value)
{
	marl::lock lock(mutex);
	if(counter < value)
	{
		counter = value;
		cv.notify_all();
		for(auto &[waitObject, waitValue] : any_waits)
		{
			if(counter >= waitValue)
			{
				waitObject->signal();
			}
		}
	}
}

void TimelineSemaphore::wait(uint64_t value)
{
	marl::lock lock(mutex);
	cv.wait(lock, [&]() { return counter >= value; });
}

uint64_t TimelineSemaphore::getCounterValue()
{
	marl::lock lock(mutex);
	return counter;
}

TimelineSemaphore::WaitForAny::WaitForAny(const VkSemaphoreWaitInfo *pWaitInfo)
{
	for(uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++)
	{
		TimelineSemaphore *semaphore = DynamicCast<TimelineSemaphore>(pWaitInfo->pSemaphores[i]);
		uint64_t waitValue = pWaitInfo->pValues[i];
		switch(semaphore->addWait(this, waitValue))
		{
		case AddWaitResult::kWaitAdded:
			semaphores.push_back(semaphore);
			break;
		case AddWaitResult::kValueAlreadySignaled:
			signal();
			break;
		case AddWaitResult::kWaitUpdated:
			// Do nothing.
			break;
		}
	}
}

TimelineSemaphore::WaitForAny::~WaitForAny()
{
	for(TimelineSemaphore *semaphore : semaphores)
	{
		semaphore->removeWait(this);
	}
}

TimelineSemaphore::AddWaitResult
TimelineSemaphore::addWait(WaitForAny *waitObject, uint64_t waitValue)
{
	// Lock the semaphore's mutex, so that its current state can be checked and,
	// if necessary, its list of waits can be updated.
	marl::lock lock(mutex);
	if(counter >= waitValue)
	{
		return AddWaitResult::kValueAlreadySignaled;
	}

	auto it = any_waits.find(waitObject);
	if(it == any_waits.end())
	{
		any_waits[waitObject] = waitValue;
		return AddWaitResult::kWaitAdded;
	}

	// If the same dependency is added more than once, only wait for the
	// lowest expected value provided.
	it->second = std::min(it->second, waitValue);
	return AddWaitResult::kWaitUpdated;
}

void TimelineSemaphore::removeWait(WaitForAny *waitObject)
{
	marl::lock lock(mutex);
	any_waits.erase(waitObject);
}

void TimelineSemaphore::WaitForAny::wait()
{
	marl::lock lock(mutex);
	cv.wait(lock, [&]() { return is_signaled; });
}

void TimelineSemaphore::WaitForAny::signal()
{
	marl::lock lock(mutex);
	if(!is_signaled)
	{
		is_signaled = true;
		cv.notify_all();
	}
}

}  // namespace vk
