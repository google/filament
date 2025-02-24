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

#include "VkDevice.hpp"

#include "VkConfig.hpp"
#include "VkDescriptorSetLayout.hpp"
#include "VkFence.hpp"
#include "VkQueue.hpp"
#include "VkSemaphore.hpp"
#include "VkStringify.hpp"
#include "VkTimelineSemaphore.hpp"
#include "Debug/Context.hpp"
#include "Debug/Server.hpp"
#include "Device/Blitter.hpp"
#include "System/Debug.hpp"

#include <chrono>
#include <climits>
#include <new>  // Must #include this to use "placement new"

namespace {

using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;

time_point now()
{
	return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now());
}

const time_point getEndTimePoint(uint64_t timeout, bool &infiniteTimeout)
{
	const time_point start = now();
	const uint64_t max_timeout = (LLONG_MAX - start.time_since_epoch().count());
	infiniteTimeout = (timeout > max_timeout);
	return start + std::chrono::nanoseconds(std::min(max_timeout, timeout));
}

}  // anonymous namespace

namespace vk {

void Device::SamplingRoutineCache::updateSnapshot()
{
	marl::lock lock(mutex);

	if(snapshotNeedsUpdate)
	{
		snapshot.clear();

		for(auto it : cache)
		{
			snapshot[it.key()] = it.data();
		}

		snapshotNeedsUpdate = false;
	}
}

Device::SamplerIndexer::~SamplerIndexer()
{
	ASSERT(map.empty());
}

uint32_t Device::SamplerIndexer::index(const SamplerState &samplerState)
{
	marl::lock lock(mutex);

	auto it = map.find(samplerState);

	if(it != map.end())
	{
		it->second.count++;
		return it->second.id;
	}

	nextID++;

	map.emplace(samplerState, Identifier{ nextID, 1 });

	return nextID;
}

void Device::SamplerIndexer::remove(const SamplerState &samplerState)
{
	marl::lock lock(mutex);

	auto it = map.find(samplerState);
	ASSERT(it != map.end());

	auto count = --it->second.count;
	if(count == 0)
	{
		map.erase(it);
	}
}

const SamplerState *Device::SamplerIndexer::find(uint32_t id)
{
	marl::lock lock(mutex);

	auto it = std::find_if(std::begin(map), std::end(map),
	                       [&id](auto &&p) { return p.second.id == id; });

	return (it != std::end(map)) ? &(it->first) : nullptr;
}

Device::Device(const VkDeviceCreateInfo *pCreateInfo, void *mem, PhysicalDevice *physicalDevice, const VkPhysicalDeviceFeatures *enabledFeatures, const std::shared_ptr<marl::Scheduler> &scheduler)
    : physicalDevice(physicalDevice)
    , queues(reinterpret_cast<Queue *>(mem))
    , enabledExtensionCount(pCreateInfo->enabledExtensionCount)
    , enabledFeatures(enabledFeatures ? *enabledFeatures : VkPhysicalDeviceFeatures{})  // "Setting pEnabledFeatures to NULL and not including a VkPhysicalDeviceFeatures2 in the pNext member of VkDeviceCreateInfo is equivalent to setting all members of the structure to VK_FALSE."
    , scheduler(scheduler)
{
	for(uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
	{
		const VkDeviceQueueCreateInfo &queueCreateInfo = pCreateInfo->pQueueCreateInfos[i];
		queueCount += queueCreateInfo.queueCount;
	}

	uint32_t queueID = 0;
	for(uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
	{
		const VkDeviceQueueCreateInfo &queueCreateInfo = pCreateInfo->pQueueCreateInfos[i];

		for(uint32_t j = 0; j < queueCreateInfo.queueCount; j++, queueID++)
		{
			new(&queues[queueID]) Queue(this, scheduler.get());
		}
	}

	extensions = reinterpret_cast<ExtensionName *>(static_cast<uint8_t *>(mem) + (sizeof(Queue) * queueCount));
	for(uint32_t i = 0; i < enabledExtensionCount; i++)
	{
		strncpy(extensions[i], pCreateInfo->ppEnabledExtensionNames[i], VK_MAX_EXTENSION_NAME_SIZE);
	}

	if(pCreateInfo->enabledLayerCount)
	{
		// "The ppEnabledLayerNames and enabledLayerCount members of VkDeviceCreateInfo are deprecated and their values must be ignored by implementations."
		UNSUPPORTED("enabledLayerCount");
	}

	// TODO(b/119409619): use an allocator here so we can control all memory allocations
	blitter.reset(new sw::Blitter());
	samplingRoutineCache.reset(new SamplingRoutineCache());
	samplerIndexer.reset(new SamplerIndexer());

#ifdef SWIFTSHADER_DEVICE_MEMORY_REPORT
	const auto *deviceMemoryReportCreateInfo = GetExtendedStruct<VkDeviceDeviceMemoryReportCreateInfoEXT>(pCreateInfo->pNext, VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT);
	if(deviceMemoryReportCreateInfo && deviceMemoryReportCreateInfo->pfnUserCallback != nullptr)
	{
		deviceMemoryReportCallbacks.emplace_back(deviceMemoryReportCreateInfo->pfnUserCallback, deviceMemoryReportCreateInfo->pUserData);
	}
#endif  // SWIFTSHADER_DEVICE_MEMORY_REPORT
}

void Device::destroy(const VkAllocationCallbacks *pAllocator)
{
	for(uint32_t i = 0; i < queueCount; i++)
	{
		queues[i].~Queue();
	}

	vk::freeHostMemory(queues, pAllocator);
}

size_t Device::ComputeRequiredAllocationSize(const VkDeviceCreateInfo *pCreateInfo)
{
	uint32_t queueCount = 0;
	for(uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
	{
		queueCount += pCreateInfo->pQueueCreateInfos[i].queueCount;
	}

	return (sizeof(Queue) * queueCount) + (pCreateInfo->enabledExtensionCount * sizeof(ExtensionName));
}

bool Device::hasExtension(const char *extensionName) const
{
	for(uint32_t i = 0; i < enabledExtensionCount; i++)
	{
		if(strncmp(extensions[i], extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0)
		{
			return true;
		}
	}
	return false;
}

VkQueue Device::getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex) const
{
	ASSERT(queueFamilyIndex == 0);

	return queues[queueIndex];
}

VkResult Device::waitForFences(uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll, uint64_t timeout)
{
	bool infiniteTimeout = false;
	const time_point end_ns = getEndTimePoint(timeout, infiniteTimeout);

	if(waitAll != VK_FALSE)  // All fences must be signaled
	{
		for(uint32_t i = 0; i < fenceCount; i++)
		{
			if(timeout == 0)
			{
				if(Cast(pFences[i])->getStatus() != VK_SUCCESS)  // At least one fence is not signaled
				{
					return VK_TIMEOUT;
				}
			}
			else if(infiniteTimeout)
			{
				if(Cast(pFences[i])->wait() != VK_SUCCESS)  // At least one fence is not signaled
				{
					return VK_TIMEOUT;
				}
			}
			else
			{
				if(Cast(pFences[i])->wait(end_ns) != VK_SUCCESS)  // At least one fence is not signaled
				{
					return VK_TIMEOUT;
				}
			}
		}

		return VK_SUCCESS;
	}
	else  // At least one fence must be signaled
	{
		marl::containers::vector<marl::Event, 8> events;
		for(uint32_t i = 0; i < fenceCount; i++)
		{
			events.push_back(Cast(pFences[i])->getCountedEvent()->event());
		}

		auto any = marl::Event::any(events.begin(), events.end());

		if(timeout == 0)
		{
			return any.isSignalled() ? VK_SUCCESS : VK_TIMEOUT;
		}
		else if(infiniteTimeout)
		{
			any.wait();
			return VK_SUCCESS;
		}
		else
		{
			return any.wait_until(end_ns) ? VK_SUCCESS : VK_TIMEOUT;
		}
	}
}

VkResult Device::waitForSemaphores(const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout)
{
	bool infiniteTimeout = false;
	const time_point end_ns = getEndTimePoint(timeout, infiniteTimeout);

	if(pWaitInfo->flags & VK_SEMAPHORE_WAIT_ANY_BIT)
	{
		TimelineSemaphore::WaitForAny any(pWaitInfo);
		if(infiniteTimeout)
		{
			any.wait();
			return VK_SUCCESS;
		}
		return any.wait(end_ns);
	}
	else
	{
		ASSERT(pWaitInfo->flags == 0);
		for(uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++)
		{
			TimelineSemaphore *semaphore = DynamicCast<TimelineSemaphore>(pWaitInfo->pSemaphores[i]);
			uint64_t value = pWaitInfo->pValues[i];
			if(infiniteTimeout)
			{
				semaphore->wait(value);
			}
			else if(semaphore->wait(pWaitInfo->pValues[i], end_ns) != VK_SUCCESS)
			{
				return VK_TIMEOUT;
			}
		}
		return VK_SUCCESS;
	}
}

VkResult Device::waitIdle()
{
	for(uint32_t i = 0; i < queueCount; i++)
	{
		queues[i].waitIdle();
	}

	return VK_SUCCESS;
}

void Device::getDescriptorSetLayoutSupport(const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                           VkDescriptorSetLayoutSupport *pSupport) const
{
	// From Vulkan Spec 13.2.1 Descriptor Set Layout, in description of vkGetDescriptorSetLayoutSupport:
	// "This command does not consider other limits such as maxPerStageDescriptor*, and so a descriptor
	// set layout that is supported according to this command must still satisfy the pipeline layout limits
	// such as maxPerStageDescriptor* in order to be used in a pipeline layout."

	// We have no "strange" limitations to enforce beyond the device limits, so we can safely always claim support.
	pSupport->supported = VK_TRUE;

	if(pCreateInfo->bindingCount > 0)
	{
		bool hasVariableSizedDescriptor = false;

		const VkBaseInStructure *layoutInfo = reinterpret_cast<const VkBaseInStructure *>(pCreateInfo->pNext);
		while(layoutInfo && !hasVariableSizedDescriptor)
		{
			if(layoutInfo->sType == VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO)
			{
				const VkDescriptorSetLayoutBindingFlagsCreateInfo *bindingFlagsCreateInfo =
				    reinterpret_cast<const VkDescriptorSetLayoutBindingFlagsCreateInfo *>(layoutInfo);

				for(uint32_t i = 0; i < bindingFlagsCreateInfo->bindingCount; i++)
				{
					if(bindingFlagsCreateInfo->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)
					{
						hasVariableSizedDescriptor = true;
						break;
					}
				}
			}
			else
			{
				UNSUPPORTED("layoutInfo->sType = %s", vk::Stringify(layoutInfo->sType).c_str());
			}

			layoutInfo = layoutInfo->pNext;
		}

		const auto &highestNumberedBinding = pCreateInfo->pBindings[pCreateInfo->bindingCount - 1];

		VkBaseOutStructure *layoutSupport = reinterpret_cast<VkBaseOutStructure *>(pSupport->pNext);
		while(layoutSupport)
		{
			if(layoutSupport->sType == VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT)
			{
				VkDescriptorSetVariableDescriptorCountLayoutSupport *variableDescriptorCountLayoutSupport =
				    reinterpret_cast<VkDescriptorSetVariableDescriptorCountLayoutSupport *>(layoutSupport);

				// If the VkDescriptorSetLayoutCreateInfo structure does not include a variable-sized descriptor,
				// [...] then maxVariableDescriptorCount is set to zero.
				variableDescriptorCountLayoutSupport->maxVariableDescriptorCount =
				    hasVariableSizedDescriptor ? ((highestNumberedBinding.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) ? vk::MAX_INLINE_UNIFORM_BLOCK_SIZE : vk::MAX_UPDATE_AFTER_BIND_DESCRIPTORS) : 0;
			}
			else
			{
				UNSUPPORTED("layoutSupport->sType = %s", vk::Stringify(layoutSupport->sType).c_str());
			}

			layoutSupport = layoutSupport->pNext;
		}
	}
}

void Device::updateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites,
                                  uint32_t descriptorCopyCount, const VkCopyDescriptorSet *pDescriptorCopies)
{
	for(uint32_t i = 0; i < descriptorWriteCount; i++)
	{
		DescriptorSetLayout::WriteDescriptorSet(this, pDescriptorWrites[i]);
	}

	for(uint32_t i = 0; i < descriptorCopyCount; i++)
	{
		DescriptorSetLayout::CopyDescriptorSet(pDescriptorCopies[i]);
	}
}

void Device::getRequirements(VkMemoryDedicatedRequirements *requirements) const
{
	requirements->prefersDedicatedAllocation = VK_FALSE;
	requirements->requiresDedicatedAllocation = VK_FALSE;
}

Device::SamplingRoutineCache *Device::getSamplingRoutineCache() const
{
	return samplingRoutineCache.get();
}

void Device::updateSamplingRoutineSnapshotCache()
{
	samplingRoutineCache->updateSnapshot();
}

uint32_t Device::indexSampler(const SamplerState &samplerState)
{
	return samplerIndexer->index(samplerState);
}

void Device::removeSampler(const SamplerState &samplerState)
{
	samplerIndexer->remove(samplerState);
}

const SamplerState *Device::findSampler(uint32_t samplerId) const
{
	return samplerIndexer->find(samplerId);
}

VkResult Device::setDebugUtilsObjectName(const VkDebugUtilsObjectNameInfoEXT *pNameInfo)
{
	// Optionally maps user-friendly name to an object
	return VK_SUCCESS;
}

VkResult Device::setDebugUtilsObjectTag(const VkDebugUtilsObjectTagInfoEXT *pTagInfo)
{
	// Optionally attach arbitrary data to an object
	return VK_SUCCESS;
}

void Device::registerImageView(ImageView *imageView)
{
	if(imageView != nullptr)
	{
		marl::lock lock(imageViewSetMutex);
		imageViewSet.insert(imageView);
	}
}

void Device::unregisterImageView(ImageView *imageView)
{
	if(imageView != nullptr)
	{
		marl::lock lock(imageViewSetMutex);
		auto it = imageViewSet.find(imageView);
		if(it != imageViewSet.end())
		{
			imageViewSet.erase(it);
		}
	}
}

void Device::prepareForSampling(ImageView *imageView)
{
	if(imageView != nullptr)
	{
		marl::lock lock(imageViewSetMutex);

		auto it = imageViewSet.find(imageView);
		if(it != imageViewSet.end())
		{
			imageView->prepareForSampling();
		}
	}
}

void Device::contentsChanged(ImageView *imageView, Image::ContentsChangedContext context)
{
	if(imageView != nullptr)
	{
		marl::lock lock(imageViewSetMutex);

		auto it = imageViewSet.find(imageView);
		if(it != imageViewSet.end())
		{
			imageView->contentsChanged(context);
		}
	}
}

VkResult Device::setPrivateData(VkObjectType objectType, uint64_t objectHandle, const PrivateData *privateDataSlot, uint64_t data)
{
	marl::lock lock(privateDataMutex);

	auto &privateDataSlotMap = privateData[privateDataSlot];
	const PrivateDataObject privateDataObject = { objectType, objectHandle };
	privateDataSlotMap[privateDataObject] = data;
	return VK_SUCCESS;
}

void Device::getPrivateData(VkObjectType objectType, uint64_t objectHandle, const PrivateData *privateDataSlot, uint64_t *data)
{
	marl::lock lock(privateDataMutex);

	*data = 0;
	auto it = privateData.find(privateDataSlot);
	if(it != privateData.end())
	{
		auto &privateDataSlotMap = it->second;
		const PrivateDataObject privateDataObject = { objectType, objectHandle };
		auto it2 = privateDataSlotMap.find(privateDataObject);
		if(it2 != privateDataSlotMap.end())
		{
			*data = it2->second;
		}
	}
}

void Device::removePrivateDataSlot(const PrivateData *privateDataSlot)
{
	marl::lock lock(privateDataMutex);

	privateData.erase(privateDataSlot);
}

#ifdef SWIFTSHADER_DEVICE_MEMORY_REPORT
void Device::emitDeviceMemoryReport(VkDeviceMemoryReportEventTypeEXT type, uint64_t memoryObjectId, VkDeviceSize size, VkObjectType objectType, uint64_t objectHandle, uint32_t heapIndex)
{
	if(deviceMemoryReportCallbacks.empty()) return;

	const VkDeviceMemoryReportCallbackDataEXT callbackData = {
		VK_STRUCTURE_TYPE_DEVICE_MEMORY_REPORT_CALLBACK_DATA_EXT,  // sType
		nullptr,                                                   // pNext
		0,                                                         // flags
		type,                                                      // type
		memoryObjectId,                                            // memoryObjectId
		size,                                                      // size
		objectType,                                                // objectType
		objectHandle,                                              // objectHandle
		heapIndex,                                                 // heapIndex
	};
	for(const auto &callback : deviceMemoryReportCallbacks)
	{
		callback.first(&callbackData, callback.second);
	}
}
#endif  // SWIFTSHADER_DEVICE_MEMORY_REPORT

}  // namespace vk
