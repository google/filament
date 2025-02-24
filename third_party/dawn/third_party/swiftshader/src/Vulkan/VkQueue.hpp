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

#ifndef VK_QUEUE_HPP_
#define VK_QUEUE_HPP_

#include "VkObject.hpp"
#include "Device/Renderer.hpp"
#include "System/Synchronization.hpp"

#include <thread>

namespace marl {
class Scheduler;
}

namespace sw {

class Context;
class Renderer;

}  // namespace sw

namespace vk {

class Device;
class Fence;
struct SubmitInfo;

class Queue
{
	VK_LOADER_DATA loaderData = { ICD_LOADER_MAGIC };

public:
	Queue(Device *device, marl::Scheduler *scheduler);
	~Queue();

	operator VkQueue()
	{
		return reinterpret_cast<VkQueue>(this);
	}

	VkResult submit(uint32_t submitCount, SubmitInfo *pSubmits, Fence *fence);
	VkResult waitIdle();
#ifndef __ANDROID__
	VkResult present(const VkPresentInfoKHR *presentInfo);
#endif

	void beginDebugUtilsLabel(const VkDebugUtilsLabelEXT *pLabelInfo);
	void endDebugUtilsLabel();
	void insertDebugUtilsLabel(const VkDebugUtilsLabelEXT *pLabelInfo);

private:
	struct Task
	{
		uint32_t submitCount = 0;
		SubmitInfo *pSubmits = nullptr;
		std::shared_ptr<sw::CountedEvent> events;

		enum Type
		{
			KILL_THREAD,
			SUBMIT_QUEUE
		};
		Type type = SUBMIT_QUEUE;
	};

	void taskLoop(marl::Scheduler *scheduler);
	void garbageCollect();
	void submitQueue(const Task &task);

	Device *device;
	std::unique_ptr<sw::Renderer> renderer;
	sw::Chan<Task> pending;
	sw::Chan<SubmitInfo *> toDelete;
	std::thread queueThread;
};

static inline Queue *Cast(VkQueue object)
{
	return reinterpret_cast<Queue *>(object);
}

}  // namespace vk

#endif  // VK_QUEUE_HPP_
