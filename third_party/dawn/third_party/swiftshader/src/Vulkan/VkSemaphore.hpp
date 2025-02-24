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

#ifndef VK_SEMAPHORE_HPP_
#define VK_SEMAPHORE_HPP_

#include "VkConfig.hpp"
#include "VkObject.hpp"

#include "marl/event.h"
#include "marl/mutex.h"
#include "marl/tsa.h"

#include "System/Synchronization.hpp"

#if VK_USE_PLATFORM_FUCHSIA
#	include <zircon/types.h>
#endif

namespace vk {

class BinarySemaphore;
class TimelineSemaphore;

class Semaphore
{
public:
	Semaphore(VkSemaphoreType type);

	virtual ~Semaphore() = default;

	static inline Semaphore *Cast(VkSemaphore semaphore)
	{
		return static_cast<Semaphore *>(static_cast<void *>(semaphore));
	}

	virtual void destroy(const VkAllocationCallbacks *pAllocator)
	{
	}

	VkSemaphoreType getSemaphoreType() const;
	// static size_t ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo *pCreateInfo);

protected:
	VkSemaphoreType type;
	marl::mutex mutex;
};

class BinarySemaphore : public Semaphore, public Object<BinarySemaphore, VkSemaphore>
{
public:
	BinarySemaphore(const VkSemaphoreCreateInfo *pCreateInfo, void *mem, const VkAllocationCallbacks *pAllocator);
	void destroy(const VkAllocationCallbacks *pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo *pCreateInfo);

	void wait();

	void wait(const VkPipelineStageFlags &flag)
	{
		// NOTE: not sure what else to do here?
		wait();
	}

	void signal();

#if SWIFTSHADER_EXTERNAL_SEMAPHORE_OPAQUE_FD
	VkResult importFd(int fd, bool temporaryImport);
	VkResult exportFd(int *pFd);
#endif

#if VK_USE_PLATFORM_FUCHSIA
	VkResult importHandle(zx_handle_t handle, bool temporaryImport);
	VkResult exportHandle(zx_handle_t *pHandle);
#endif

	class External;

private:
	// Small technical note on how semaphores are imported/exported with Vulkan:
	//
	// - A Vulkan Semaphore objects has a "payload", corresponding to a
	//   simple atomic boolean flag.
	//
	// - A Vulkan Semaphore object can be "exported": this creates a
	//   platform-specific handle / descriptor (which can be passed to other
	//   processes), and is linked in some way to the original semaphore's
	//   payload.
	//
	// - Similarly, said handle / descriptor can be "imported" into a Vulkan
	//   Semaphore object. By default, that semaphore loses its payload, and
	//   instead uses the one referenced / shared through the descriptor.
	//
	//   Hence if semaphore A exports its payload through a descriptor that
	//   is later imported into semaphore B, then both A and B will use/share
	//   the same payload (i.e. signal flag), making cross-process
	//   synchronization possible.
	//
	// - There are also "temporary imports", where the target semaphore's
	//   payload is not lost, but is simply hidden/stashed. But the next wait()
	//   operation on the same semaphore should remove the temporary import,
	//   and restore the previous payload.
	//
	// - There are many handle / descriptor types, which are listed through
	//   the VkExternalSemaphoreHandleTypeFlagBits. A given Vulkan
	//   implementation might support onle one or several at the same time
	//   (e.g. on Linux or Android, it could support both OPAQUE_FD_BIT and
	//   SYNC_FD_BIT, while on Windows, it would be OPAQUE_WIN32_BIT +
	//   OPAQUE_WIN32_KMT_BIT + D3D12_FENCE_BIT).
	//
	// - To be able to export a semaphore, VkCreateSemaphore() must be called
	//   with a VkSemaphoreCreateInfo that lists the types of all possible
	//   platform-specific handles the semaphore could be exported to
	//   (e.g. on Linux, it is possible to specify that a semaphore might be
	//   exported as an opaque FD, or as a Linux Sync FD).
	//
	//   However, which exact type is however only determined later by the
	//   export operation itself (e.g. vkGetSemaphoreFdKHR() could be called to export
	//   either a VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT or a
	//   VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT).
	//
	//   Once a semaphore has been exported as one type, it is not possible
	//   to export the same payload with a different type (though the spec
	//   doesn't seem to be explicit about this, it's simply impossible in
	//   general).
	//
	// This leads to the following design:
	//
	//   - |internal| is a simple marl::Event that represents the semaphore's
	//     payload when it is not exported, or imported non-temporarily.
	//
	//   - |external| points to an external semaphore payload. It is created
	//     on demand if the semaphore is exported or imported non-temporarily.
	//     Note that once |external| is created, |internal| is ignored.
	//
	//   - |tempExternal| points to a linked-list of temporary external
	//     semaphore payloads. The list head corresponds to the most recent
	//     temporary import.
	//

	// Internal template to allocate a new External implementation.
	template<class EXTERNAL>
	External *allocateExternal();

	void deallocateExternal(External *ext);

	// Used internally to import an external payload.
	// |temporaryImport| is true iff the import is temporary.
	// |alloc_func| is callable that allocates a new External instance of the
	// appropriate type.
	// |import_func| is callable that takes a single parameter, which
	// corresponds to the external handle/descriptor, and returns a VkResult
	// values.
	template<typename ALLOC_FUNC, typename IMPORT_FUNC>
	VkResult importPayload(bool temporaryImport,
	                       ALLOC_FUNC alloc_func,
	                       IMPORT_FUNC import_func);

	// Used internally to export a given payload.
	// |alloc_func| is a callable that allocates a new External instance of
	// the appropriate type.
	// |export_func| is a callable that takes a pointer to an External instance,
	// and a pointer to a handle/descriptor, and returns a VkResult.
	template<typename ALLOC_FUNC, typename EXPORT_FUNC>
	VkResult exportPayload(ALLOC_FUNC alloc_func, EXPORT_FUNC export_func);

	const VkAllocationCallbacks *allocator = nullptr;
	VkExternalSemaphoreHandleTypeFlags exportableHandleTypes = (VkExternalSemaphoreHandleTypeFlags)0;
	marl::Event internal;
	External *external GUARDED_BY(mutex) = nullptr;
	External *tempExternal GUARDED_BY(mutex) = nullptr;
};

static inline Semaphore *Cast(VkSemaphore object)
{
	return Semaphore::Cast(object);
}

template<typename T>
static inline T *DynamicCast(VkSemaphore object)
{
	Semaphore *semaphore = vk::Cast(object);
	if(semaphore == nullptr)
	{
		return nullptr;
	}

	static_assert(std::is_same_v<T, BinarySemaphore> || std::is_same_v<T, TimelineSemaphore>);
	if constexpr(std::is_same_v<T, BinarySemaphore>)
	{
		if(semaphore->getSemaphoreType() != VK_SEMAPHORE_TYPE_BINARY)
		{
			return nullptr;
		}
	}
	else
	{
		if(semaphore->getSemaphoreType() != VK_SEMAPHORE_TYPE_TIMELINE)
		{
			return nullptr;
		}
	}
	return static_cast<T *>(semaphore);
}

// This struct helps parse VkSemaphoreCreateInfo. It also looks at the pNext
// structures and stores their data flatly in a single struct. The default
// values of each data member are what the absence of a pNext struct implies
// for those values.
struct SemaphoreCreateInfo
{
	bool exportSemaphore = false;
	VkExternalSemaphoreHandleTypeFlags exportHandleTypes = 0;

	VkSemaphoreType semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
	uint64_t initialPayload = 0;

	SemaphoreCreateInfo(const VkSemaphoreCreateInfo *pCreateInfo);
};

}  // namespace vk

#endif  // VK_SEMAPHORE_HPP_
