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

#ifndef VK_DEVICE_HPP_
#define VK_DEVICE_HPP_

#include "VkImageView.hpp"
#include "VkSampler.hpp"
#include "Device/Blitter.hpp"
#include "Pipeline/Constants.hpp"
#include "Reactor/Routine.hpp"
#include "System/LRUCache.hpp"

#include "marl/mutex.h"
#include "marl/tsa.h"

#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace marl {
class Scheduler;
}

namespace vk {

class PhysicalDevice;
class PrivateData;
class Queue;

class Device
{
public:
	static constexpr VkSystemAllocationScope GetAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_DEVICE; }

	Device(const VkDeviceCreateInfo *pCreateInfo, void *mem, PhysicalDevice *physicalDevice, const VkPhysicalDeviceFeatures *enabledFeatures, const std::shared_ptr<marl::Scheduler> &scheduler);
	void destroy(const VkAllocationCallbacks *pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkDeviceCreateInfo *pCreateInfo);

	bool hasExtension(const char *extensionName) const;
	VkQueue getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex) const;
	VkResult waitForFences(uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll, uint64_t timeout);
	VkResult waitForSemaphores(const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout);
	VkResult waitIdle();
	void getDescriptorSetLayoutSupport(const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
	                                   VkDescriptorSetLayoutSupport *pSupport) const;
	PhysicalDevice *getPhysicalDevice() const { return physicalDevice; }
	void updateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites,
	                          uint32_t descriptorCopyCount, const VkCopyDescriptorSet *pDescriptorCopies);
	void getRequirements(VkMemoryDedicatedRequirements *requirements) const;
	const VkPhysicalDeviceFeatures &getEnabledFeatures() const { return enabledFeatures; }
	sw::Blitter *getBlitter() const { return blitter.get(); }

	void registerImageView(ImageView *imageView);
	void unregisterImageView(ImageView *imageView);
	void prepareForSampling(ImageView *imageView);
	void contentsChanged(ImageView *imageView, Image::ContentsChangedContext context);

	VkResult setPrivateData(VkObjectType objectType, uint64_t objectHandle, const PrivateData *privateDataSlot, uint64_t data);
	void getPrivateData(VkObjectType objectType, uint64_t objectHandle, const PrivateData *privateDataSlot, uint64_t *data);
	void removePrivateDataSlot(const PrivateData *privateDataSlot);

	class SamplingRoutineCache
	{
	public:
		SamplingRoutineCache()
		    : cache(1024)
		{}
		~SamplingRoutineCache() {}

		struct Key
		{
			uint32_t instruction;
			uint32_t sampler;
			uint32_t imageView;

			inline bool operator==(const Key &rhs) const;

			struct Hash
			{
				inline std::size_t operator()(const Key &key) const noexcept;
			};
		};

		// getOrCreate() queries the cache for a Routine with the given key.
		// If one is found, it is returned, otherwise createRoutine(key) is
		// called, the returned Routine is added to the cache, and it is
		// returned.
		// Function must be a function of the signature:
		//     std::shared_ptr<rr::Routine>(const Key &)
		template<typename Function>
		std::shared_ptr<rr::Routine> getOrCreate(const Key &key, Function &&createRoutine)
		{
			auto it = snapshot.find(key);
			if(it != snapshot.end()) { return it->second; }

			marl::lock lock(mutex);
			if(auto existingRoutine = cache.lookup(key))
			{
				return existingRoutine;
			}

			std::shared_ptr<rr::Routine> newRoutine = createRoutine(key);
			cache.add(key, newRoutine);
			snapshotNeedsUpdate = true;

			return newRoutine;
		}

		void updateSnapshot();

	private:
		bool snapshotNeedsUpdate = false;
		std::unordered_map<Key, std::shared_ptr<rr::Routine>, Key::Hash> snapshot;

		marl::mutex mutex;
		sw::LRUCache<Key, std::shared_ptr<rr::Routine>, Key::Hash> cache GUARDED_BY(mutex);
	};

	SamplingRoutineCache *getSamplingRoutineCache() const;
	void updateSamplingRoutineSnapshotCache();

	class SamplerIndexer
	{
	public:
		~SamplerIndexer();

		uint32_t index(const SamplerState &samplerState);
		void remove(const SamplerState &samplerState);
		const SamplerState *find(uint32_t id);

	private:
		struct Identifier
		{
			uint32_t id;
			uint32_t count;  // Number of samplers sharing this state identifier.
		};

		marl::mutex mutex;
		std::map<SamplerState, Identifier> map GUARDED_BY(mutex);

		uint32_t nextID = 0;
	};

	uint32_t indexSampler(const SamplerState &samplerState);
	void removeSampler(const SamplerState &samplerState);
	const SamplerState *findSampler(uint32_t samplerId) const;

	VkResult setDebugUtilsObjectName(const VkDebugUtilsObjectNameInfoEXT *pNameInfo);
	VkResult setDebugUtilsObjectTag(const VkDebugUtilsObjectTagInfoEXT *pTagInfo);

#ifdef SWIFTSHADER_DEVICE_MEMORY_REPORT
	void emitDeviceMemoryReport(VkDeviceMemoryReportEventTypeEXT type, uint64_t memoryObjectId, VkDeviceSize size, VkObjectType objectType, uint64_t objectHandle, uint32_t heapIndex = 0);
#endif  // SWIFTSHADER_DEVICE_MEMORY_REPORT

	const sw::Constants constants;

private:
	PhysicalDevice *const physicalDevice = nullptr;
	Queue *const queues = nullptr;
	uint32_t queueCount = 0;
	std::unique_ptr<sw::Blitter> blitter;
	uint32_t enabledExtensionCount = 0;
	typedef char ExtensionName[VK_MAX_EXTENSION_NAME_SIZE];
	ExtensionName *extensions = nullptr;
	const VkPhysicalDeviceFeatures enabledFeatures = {};

	std::shared_ptr<marl::Scheduler> scheduler;
	std::unique_ptr<SamplingRoutineCache> samplingRoutineCache;
	std::unique_ptr<SamplerIndexer> samplerIndexer;

	marl::mutex imageViewSetMutex;
	std::unordered_set<ImageView *> imageViewSet GUARDED_BY(imageViewSetMutex);

	struct PrivateDataObject
	{
		VkObjectType objectType;
		uint64_t objectHandle;

		bool operator==(const PrivateDataObject &privateDataObject) const
		{
			return (objectType == privateDataObject.objectType) &&
			       (objectHandle == privateDataObject.objectHandle);
		}

		struct Hash
		{
			std::size_t operator()(const PrivateDataObject &privateDataObject) const noexcept
			{
				// Since the object type is linked to the object's handle,
				// simply use the object handle as the hash value.
				return static_cast<size_t>(privateDataObject.objectHandle);
			}
		};
	};
	typedef std::unordered_map<PrivateDataObject, uint64_t, PrivateDataObject::Hash> PrivateDataSlot;

	marl::mutex privateDataMutex;
	std::map<const PrivateData *, PrivateDataSlot> privateData;

#ifdef SWIFTSHADER_DEVICE_MEMORY_REPORT
	std::vector<std::pair<PFN_vkDeviceMemoryReportCallbackEXT, void *>> deviceMemoryReportCallbacks;
#endif  // SWIFTSHADER_DEVICE_MEMORY_REPORT
};

using DispatchableDevice = DispatchableObject<Device, VkDevice>;

static inline Device *Cast(VkDevice object)
{
	return DispatchableDevice::Cast(object);
}

inline bool vk::Device::SamplingRoutineCache::Key::operator==(const Key &rhs) const
{
	return instruction == rhs.instruction && sampler == rhs.sampler && imageView == rhs.imageView;
}

inline std::size_t vk::Device::SamplingRoutineCache::Key::Hash::operator()(const Key &key) const noexcept
{
	// Combine three 32-bit integers into a 64-bit hash.
	// 2642239 is the largest prime which when cubed is smaller than 2^64.
	uint64_t hash = key.instruction;
	hash = (hash * 2642239) ^ key.sampler;
	hash = (hash * 2642239) ^ key.imageView;
	return static_cast<std::size_t>(hash);  // Truncates to 32-bits on 32-bit platforms.
}

}  // namespace vk

#endif  // VK_DEVICE_HPP_
