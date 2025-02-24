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

#ifndef VK_TIMELINE_SEMAPHORE_HPP_
#define VK_TIMELINE_SEMAPHORE_HPP_

#include "VkConfig.hpp"
#include "VkObject.hpp"
#include "VkSemaphore.hpp"

#include "marl/conditionvariable.h"
#include "marl/mutex.h"

#include "System/Synchronization.hpp"

#include <chrono>

namespace vk {

// Timeline Semaphores track a 64-bit payload instead of a binary payload.
//
// A timeline does not have a "signaled" and "unsignalled" state. Threads instead wait
// for the payload to become a certain value. When a thread signals the timeline, it provides
// a new payload that is greater than the current payload.
//
// There is no way to reset a timeline or to decrease the payload's value. A user must instead
// create a new timeline with a new initial payload if they desire this behavior.
class TimelineSemaphore : public Semaphore, public Object<TimelineSemaphore, VkSemaphore>
{
public:
	// WaitForAny represents a single vkWaitSemaphores() call with the
	// VK_SEMAPHORE_WAIT_ANY_BIT set.
	class WaitForAny
	{
	public:
		// Creates a WaitForAny object and populates it with the contents of a VkSemaphoreWaitInfo.
		WaitForAny(const VkSemaphoreWaitInfo *pWaitInfo);
		~WaitForAny();

		void wait();
		template<class CLOCK, class DURATION>
		VkResult wait(std::chrono::time_point<CLOCK, DURATION> end_ns);
		void signal();

	private:
		marl::mutex mutex;
		marl::ConditionVariable cv;
		// TODO(b/181683382) -- Add Thread Safety Analysis instrumentation when it can properly
		// analyze lambdas.
		bool is_signaled = false;
		marl::containers::vector<TimelineSemaphore *, 16> semaphores;
	};

	TimelineSemaphore(const VkSemaphoreCreateInfo *pCreateInfo, void *mem, const VkAllocationCallbacks *pAllocator);
	TimelineSemaphore();

	static size_t ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo *pCreateInfo);

	// Block until this semaphore is signaled with the specified value;
	void wait(uint64_t value);

	// Wait until a certain amount of time has passed or until the specified value is signaled.
	template<class CLOCK, class DURATION>
	VkResult wait(uint64_t value, std::chrono::time_point<CLOCK, DURATION> end_ns);

	// Set the payload to the specified value and signal all waiting threads.
	void signal(uint64_t value);

	// Retrieve the current payload. This should not be used to make thread execution decisions as
	// there's no guarantee that the value returned here matches the actual payload's value.
	uint64_t getCounterValue();

	// Clean up any allocated resources
	void destroy(const VkAllocationCallbacks *pAllocator);

private:
	enum class AddWaitResult
	{
		kWaitAdded = 0,
		kWaitUpdated,
		kValueAlreadySignaled
	};

	AddWaitResult addWait(WaitForAny *waitObject, uint64_t waitValue);
	void removeWait(WaitForAny *waitObject);

	// Guards access to all the resources that may be accessed by other threads.
	// No clang Thread Safety Analysis is used on variables guarded by mutex
	// as there is an issue with TSA. Despite instrumenting everything properly,
	// compilation will fail when a lambda function uses a guarded resource.
	marl::mutex mutex;

	// Entry point to the marl threading library that handles blocking and unblocking.
	marl::ConditionVariable cv;

	// TODO(b/181683382) -- Add Thread Safety Analysis instrumentation when it can properly
	// analyze lambdas.
	// The 64-bit payload.
	uint64_t counter;

	// All the WaitForAny objects waiting on this semaphore to reach specific values.
	std::map<WaitForAny *, uint64_t> any_waits;
};

template<typename Clock, typename Duration>
VkResult TimelineSemaphore::wait(uint64_t value,
                                 const std::chrono::time_point<Clock, Duration> timeout)
{
	marl::lock lock(mutex);
	if(!cv.wait_until(lock, timeout, [&]() { return counter >= value; }))
	{
		return VK_TIMEOUT;
	}
	return VK_SUCCESS;
}

template<typename Clock, typename Duration>
VkResult TimelineSemaphore::WaitForAny::wait(const std::chrono::time_point<Clock, Duration> timeout)
{
	marl::lock lock(mutex);
	if(!cv.wait_until(lock, timeout, [&]() { return is_signaled; }))
	{
		return VK_TIMEOUT;
	}
	return VK_SUCCESS;
}

}  // namespace vk

#endif  // VK_TIMELINE_SEMAPHORE_HPP_
