// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See thread_safety_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/

// NOLINTBEGIN

#include "thread_tracker/thread_safety_validation.h"

namespace threadsafety {
void Instance::PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                           VkInstance* pInstance, const RecordObject& record_obj) {}

void Instance::PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                            VkInstance* pInstance, const RecordObject& record_obj) {
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pInstance);
    }
}

void Instance::PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {
    StartWriteObject(instance, record_obj.location);
    // Host access to instance must be externally synchronized
    // all sname:VkPhysicalDevice objects enumerated from pname:instance must be externally synchronized between host accesses
}

void Instance::PostCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {
    FinishWriteObject(instance, record_obj.location);
    DestroyObject(instance);
    // Host access to instance must be externally synchronized
    // all sname:VkPhysicalDevice objects enumerated from pname:instance must be externally synchronized between host accesses
}

void Instance::PreCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                     VkPhysicalDevice* pPhysicalDevices, const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                      VkPhysicalDevice* pPhysicalDevices, const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
}

void Instance::PreCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName, const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName, const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
}

void Device::PreCallRecordGetDeviceProcAddr(VkDevice device, const char* pName, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceProcAddr(VkDevice device, const char* pName, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Instance::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                         const RecordObject& record_obj) {}

void Instance::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                          const RecordObject& record_obj) {
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pDevice);
    }
}

void Device::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                      const RecordObject& record_obj) {
    StartWriteObject(queue, record_obj.location);
    StartWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void Device::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                       const RecordObject& record_obj) {
    FinishWriteObject(queue, record_obj.location);
    FinishWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void Device::PreCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) {
    StartWriteObject(queue, record_obj.location);
    // Host access to queue must be externally synchronized
}

void Device::PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) {
    FinishWriteObject(queue, record_obj.location);
    // Host access to queue must be externally synchronized
}

void Device::PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pMemory);
    }
}

void Device::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(memory, record_obj.location);
    // Host access to memory must be externally synchronized
}

void Device::PostCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(memory, record_obj.location);
    DestroyObject(memory);
    // Host access to memory must be externally synchronized
}

void Device::PreCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                    VkMemoryMapFlags flags, void** ppData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(memory, record_obj.location);
    // Host access to memory must be externally synchronized
}

void Device::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                     VkMemoryMapFlags flags, void** ppData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(memory, record_obj.location);
    // Host access to memory must be externally synchronized
}

void Device::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(memory, record_obj.location);
    // Host access to memory must be externally synchronized
}

void Device::PostCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(memory, record_obj.location);
    // Host access to memory must be externally synchronized
}

void Device::PreCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                  const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                   const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                       const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                        const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(memory, record_obj.location);
}

void Device::PostCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(memory, record_obj.location);
}

void Device::PreCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(buffer, record_obj.location);
    StartReadObject(memory, record_obj.location);
    // Host access to buffer must be externally synchronized
}

void Device::PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(buffer, record_obj.location);
    FinishReadObject(memory, record_obj.location);
    // Host access to buffer must be externally synchronized
}

void Device::PreCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(image, record_obj.location);
    StartReadObject(memory, record_obj.location);
    // Host access to image must be externally synchronized
}

void Device::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(image, record_obj.location);
    FinishReadObject(memory, record_obj.location);
    // Host access to image must be externally synchronized
}

void Device::PreCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(buffer, record_obj.location);
}

void Device::PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
}

void Device::PreCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(image, record_obj.location);
}

void Device::PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(image, record_obj.location);
}

void Device::PreCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                           VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(image, record_obj.location);
}

void Device::PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(image, record_obj.location);
}

void Device::PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                          const RecordObject& record_obj) {
    StartWriteObject(queue, record_obj.location);
    StartWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void Device::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                           const RecordObject& record_obj) {
    FinishWriteObject(queue, record_obj.location);
    FinishWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void Device::PreCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkFence* pFence, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkFence* pFence, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFence);
    }
}

void Device::PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(fence, record_obj.location);
    // Host access to fence must be externally synchronized
}

void Device::PostCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(fence, record_obj.location);
    DestroyObject(fence);
    // Host access to fence must be externally synchronized
}

void Device::PreCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            StartWriteObject(pFences[index], record_obj.location);
        }
    }
    // Host access to each member of pFences must be externally synchronized
}

void Device::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            FinishWriteObject(pFences[index], record_obj.location);
        }
    }
    // Host access to each member of pFences must be externally synchronized
}

void Device::PreCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(fence, record_obj.location);
}

void Device::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(fence, record_obj.location);
}

void Device::PreCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                        uint64_t timeout, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            StartReadObject(pFences[index], record_obj.location);
        }
    }
}

void Device::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                         uint64_t timeout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            FinishReadObject(pFences[index], record_obj.location);
        }
    }
}

void Device::PreCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSemaphore);
    }
}

void Device::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(semaphore, record_obj.location);
    // Host access to semaphore must be externally synchronized
}

void Device::PostCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(semaphore, record_obj.location);
    DestroyObject(semaphore);
    // Host access to semaphore must be externally synchronized
}

void Device::PreCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkEvent* pEvent, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkEvent* pEvent, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pEvent);
    }
}

void Device::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(event, record_obj.location);
    // Host access to event must be externally synchronized
}

void Device::PostCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(event, record_obj.location);
    DestroyObject(event);
    // Host access to event must be externally synchronized
}

void Device::PreCallRecordGetEventStatus(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(event, record_obj.location);
}

void Device::PostCallRecordGetEventStatus(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(event, record_obj.location);
}

void Device::PreCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(event, record_obj.location);
    // Host access to event must be externally synchronized
}

void Device::PostCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(event, record_obj.location);
    // Host access to event must be externally synchronized
}

void Device::PreCallRecordResetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(event, record_obj.location);
    // Host access to event must be externally synchronized
}

void Device::PostCallRecordResetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(event, record_obj.location);
    // Host access to event must be externally synchronized
}

void Device::PreCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pQueryPool);
    }
}

void Device::PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(queryPool, record_obj.location);
    // Host access to queryPool must be externally synchronized
}

void Device::PostCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(queryPool, record_obj.location);
    DestroyObject(queryPool);
    // Host access to queryPool must be externally synchronized
}

void Device::PreCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                              size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
}

void Device::PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                               size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
}

void Device::PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pBuffer);
    }
}

void Device::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(buffer, record_obj.location);
    // Host access to buffer must be externally synchronized
}

void Device::PostCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(buffer, record_obj.location);
    DestroyObject(buffer);
    // Host access to buffer must be externally synchronized
}

void Device::PreCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pView);
    }
}

void Device::PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(bufferView, record_obj.location);
    // Host access to bufferView must be externally synchronized
}

void Device::PostCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(bufferView, record_obj.location);
    DestroyObject(bufferView);
    // Host access to bufferView must be externally synchronized
}

void Device::PreCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkImage* pImage, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkImage* pImage, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pImage);
    }
}

void Device::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(image, record_obj.location);
    // Host access to image must be externally synchronized
}

void Device::PostCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(image, record_obj.location);
    DestroyObject(image);
    // Host access to image must be externally synchronized
}

void Device::PreCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                    VkSubresourceLayout* pLayout, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(image, record_obj.location);
}

void Device::PostCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                     VkSubresourceLayout* pLayout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(image, record_obj.location);
}

void Device::PreCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pView);
    }
}

void Device::PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(imageView, record_obj.location);
    // Host access to imageView must be externally synchronized
}

void Device::PostCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(imageView, record_obj.location);
    DestroyObject(imageView);
    // Host access to imageView must be externally synchronized
}

void Device::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pShaderModule);
    }
}

void Device::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(shaderModule, record_obj.location);
    // Host access to shaderModule must be externally synchronized
}

void Device::PostCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                               const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(shaderModule, record_obj.location);
    DestroyObject(shaderModule);
    // Host access to shaderModule must be externally synchronized
}

void Device::PreCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pPipelineCache);
    }
}

void Device::PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                               const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(pipelineCache, record_obj.location);
    // Host access to pipelineCache must be externally synchronized
}

void Device::PostCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(pipelineCache, record_obj.location);
    DestroyObject(pipelineCache);
    // Host access to pipelineCache must be externally synchronized
}

void Device::PreCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipelineCache, record_obj.location);
}

void Device::PostCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipelineCache, record_obj.location);
}

void Device::PreCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                              const VkPipelineCache* pSrcCaches, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(dstCache, record_obj.location);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            StartReadObject(pSrcCaches[index], record_obj.location);
        }
    }
}

void Device::PostCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                               const VkPipelineCache* pSrcCaches, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(dstCache, record_obj.location);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            FinishReadObject(pSrcCaches[index], record_obj.location);
        }
    }
}

void Device::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                  const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipelineCache, record_obj.location);
}

void Device::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                   const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipelineCache, record_obj.location);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void Device::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                 const VkComputePipelineCreateInfo* pCreateInfos,
                                                 const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipelineCache, record_obj.location);
}

void Device::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                  const VkComputePipelineCreateInfo* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipelineCache, record_obj.location);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void Device::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(pipeline, record_obj.location);
    // Host access to pipeline must be externally synchronized
}

void Device::PostCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(pipeline, record_obj.location);
    DestroyObject(pipeline);
    // Host access to pipeline must be externally synchronized
}

void Device::PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pPipelineLayout);
    }
}

void Device::PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(pipelineLayout, record_obj.location);
    // Host access to pipelineLayout must be externally synchronized
}

void Device::PostCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(pipelineLayout, record_obj.location);
    DestroyObject(pipelineLayout);
    // Host access to pipelineLayout must be externally synchronized
}

void Device::PreCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSampler);
    }
}

void Device::PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(sampler, record_obj.location);
    // Host access to sampler must be externally synchronized
}

void Device::PostCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(sampler, record_obj.location);
    DestroyObject(sampler);
    // Host access to sampler must be externally synchronized
}

void Device::PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                     const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(descriptorSetLayout, record_obj.location);
    // Host access to descriptorSetLayout must be externally synchronized
}

void Device::PostCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(descriptorSetLayout, record_obj.location);
    DestroyObject(descriptorSetLayout);
    // Host access to descriptorSetLayout must be externally synchronized
}

void Device::PreCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pDescriptorPool);
    }
}

void Device::PreCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFramebuffer);
    }
}

void Device::PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(framebuffer, record_obj.location);
    // Host access to framebuffer must be externally synchronized
}

void Device::PostCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(framebuffer, record_obj.location);
    DestroyObject(framebuffer);
    // Host access to framebuffer must be externally synchronized
}

void Device::PreCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pRenderPass);
    }
}

void Device::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(renderPass, record_obj.location);
    // Host access to renderPass must be externally synchronized
}

void Device::PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(renderPass, record_obj.location);
    DestroyObject(renderPass);
    // Host access to renderPass must be externally synchronized
}

void Device::PreCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(renderPass, record_obj.location);
}

void Device::PostCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(renderPass, record_obj.location);
}

void Device::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void Device::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void Device::PreCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void Device::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void Device::PreCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void Device::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void Device::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                           VkPipeline pipeline, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                         const VkViewport* pViewports, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                          const VkViewport* pViewports, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                        const VkRect2D* pScissors, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                         const VkRect2D* pScissors, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                          float depthBiasSlopeFactor, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                           float depthBiasSlopeFactor, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask,
                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                    uint32_t compareMask, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                const uint32_t* pDynamicOffsets, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(layout, record_obj.location);

    if (pDescriptorSets) {
        for (uint32_t index = 0; index < descriptorSetCount; index++) {
            StartReadObject(pDescriptorSets[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                 const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                 const uint32_t* pDynamicOffsets, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(layout, record_obj.location);

    if (pDescriptorSets) {
        for (uint32_t index = 0; index < descriptorSetCount; index++) {
            FinishReadObject(pDescriptorSets[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkIndexType indexType, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkIndexType indexType, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                               const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                  uint32_t firstInstance, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                   uint32_t firstVertex, uint32_t firstInstance, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                         uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                          uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                          uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                           uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                      uint32_t groupCountZ, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                       uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                        const VkBufferCopy* pRegions, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcBuffer, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                         uint32_t regionCount, const VkBufferCopy* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcBuffer, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const VkImageCopy* pRegions, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcImage, record_obj.location);
    StartReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageCopy* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcImage, record_obj.location);
    FinishReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const VkImageBlit* pRegions, VkFilter filter, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcImage, record_obj.location);
    StartReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageBlit* pRegions, VkFilter filter, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcImage, record_obj.location);
    FinishReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                               VkImageLayout dstImageLayout, uint32_t regionCount,
                                               const VkBufferImageCopy* pRegions, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcBuffer, record_obj.location);
    StartReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkBufferImageCopy* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcBuffer, record_obj.location);
    FinishReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                               VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcImage, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcImage, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                          VkDeviceSize dataSize, const void* pData, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                           VkDeviceSize dataSize, const void* pData, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize size, uint32_t data, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                         VkDeviceSize size, uint32_t data, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                             const VkClearColorValue* pColor, uint32_t rangeCount,
                                             const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(image, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                              const VkClearColorValue* pColor, uint32_t rangeCount,
                                              const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(image, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                    const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(image, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                     const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                     const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(image, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                              const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                               const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                          VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                          const VkImageResolve* pRegions, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcImage, record_obj.location);
    StartReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkImageResolve* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcImage, record_obj.location);
    FinishReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                        VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                        uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                        uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                        uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            StartReadObject(pEvents[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                         VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            FinishReadObject(pEvents[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                             VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                             uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                             uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                             uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                              VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                        VkQueryControlFlags flags, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                         VkQueryControlFlags flags, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                            uint32_t queryCount, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                             uint32_t queryCount, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                            VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                             VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                  VkDeviceSize stride, VkQueryResultFlags flags, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                   uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize stride, VkQueryResultFlags flags, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                           uint32_t offset, uint32_t size, const void* pValues, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                            uint32_t offset, uint32_t size, const void* pValues, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             VkSubpassContents contents, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                              VkSubpassContents contents, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                             const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pCommandBuffers) {
        for (uint32_t index = 0; index < commandBufferCount; index++) {
            StartReadObject(pCommandBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                              const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pCommandBuffers) {
        for (uint32_t index = 0; index < commandBufferCount; index++) {
            FinishReadObject(pCommandBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                           uint32_t remoteDeviceIndex,
                                                           VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                            uint32_t remoteDeviceIndex,
                                                            VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                          uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                           uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Instance::PreCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                          VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                          const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                           VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                           const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
}

void Device::PreCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                        VkMemoryRequirements2* pMemoryRequirements,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                            uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                             uint32_t* pSparseMemoryRequirementCount,
                                                             VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(commandPool, record_obj.location);
    // Host access to commandPool must be externally synchronized
}

void Device::PostCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(commandPool, record_obj.location);
    // Host access to commandPool must be externally synchronized
}

void Device::PreCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkSamplerYcbcrConversion* pYcbcrConversion, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkSamplerYcbcrConversion* pYcbcrConversion,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pYcbcrConversion);
    }
}

void Device::PreCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                        const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(ycbcrConversion, record_obj.location);
    // Host access to ycbcrConversion must be externally synchronized
}

void Device::PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(ycbcrConversion, record_obj.location);
    DestroyObject(ycbcrConversion);
    // Host access to ycbcrConversion must be externally synchronized
}

void Device::PreCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pDescriptorUpdateTemplate);
    }
}

void Device::PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(descriptorUpdateTemplate, record_obj.location);
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void Device::PostCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(descriptorUpdateTemplate, record_obj.location);
    DestroyObject(descriptorUpdateTemplate);
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void Device::PreCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                        VkDescriptorSetLayoutSupport* pSupport, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                         VkDescriptorSetLayoutSupport* pSupport, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    StartReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    FinishReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    StartReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    FinishReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pRenderPass);
    }
}

void Device::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                              const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                          const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                           const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
}

void Device::PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
}

void Device::PreCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(semaphore, record_obj.location);
}

void Device::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(semaphore, record_obj.location);
}

void Device::PreCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pPrivateDataSlot);
    }
}

void Device::PreCallRecordDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(privateDataSlot, record_obj.location);
    // Host access to privateDataSlot must be externally synchronized
}

void Device::PostCallRecordDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                  const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(privateDataSlot, record_obj.location);
    DestroyObject(privateDataSlot);
    // Host access to privateDataSlot must be externally synchronized
}

void Device::PreCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                         VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(privateDataSlot, record_obj.location);
}

void Device::PostCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                          VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(privateDataSlot, record_obj.location);
}

void Device::PreCallRecordGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                         VkPrivateDataSlot privateDataSlot, uint64_t* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(privateDataSlot, record_obj.location);
}

void Device::PostCallRecordGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                          VkPrivateDataSlot privateDataSlot, uint64_t* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(privateDataSlot, record_obj.location);
}

void Device::PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                       const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                         const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            StartReadObject(pEvents[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            FinishReadObject(pEvents[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                             uint32_t query, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                              uint32_t query, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                       const RecordObject& record_obj) {
    StartWriteObject(queue, record_obj.location);
    StartWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void Device::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                        const RecordObject& record_obj) {
    FinishWriteObject(queue, record_obj.location);
    FinishWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void Device::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                 const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                 const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                           const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                  const VkViewport* pViewports, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                   const VkViewport* pViewports, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                const VkDeviceSize* pStrides, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                 const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                 const VkDeviceSize* pStrides, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                          VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                           VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                       const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                            VkMemoryRequirements2* pMemoryRequirements,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                           VkMemoryRequirements2* pMemoryRequirements,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                            VkMemoryRequirements2* pMemoryRequirements,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                 uint32_t* pSparseMemoryRequirementCount,
                                                                 VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                  uint32_t* pSparseMemoryRequirementCount,
                                                                  VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordMapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordMapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkDeviceSize size, VkIndexType indexType, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkDeviceSize size, VkIndexType indexType, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                      VkExtent2D* pGranularity, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                       VkExtent2D* pGranularity, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                          VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                           VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                     VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(image, record_obj.location);
}

void Device::PostCallRecordGetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                      VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(image, record_obj.location);
}

void Device::PreCallRecordCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                               VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                               const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                           VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                           VkPipelineLayout layout, uint32_t set, const void* pData,
                                                           const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(descriptorUpdateTemplate, record_obj.location);
    StartReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                            VkPipelineLayout layout, uint32_t set, const void* pData,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(descriptorUpdateTemplate, record_obj.location);
    FinishReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer,
                                                             const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer,
                                                              const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetRenderingInputAttachmentIndices(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetRenderingInputAttachmentIndices(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                                 const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                                  const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                 const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                                const VkHostImageLayoutTransitionInfo* pTransitions,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                                 const VkHostImageLayoutTransitionInfo* pTransitions,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Instance::PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
    StartWriteObject(surface, record_obj.location);
    // Host access to surface must be externally synchronized
}

void Instance::PostCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                               const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    FinishWriteObject(surface, record_obj.location);
    DestroyObject(surface);
    // Host access to surface must be externally synchronized
}

void Instance::PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                               VkSurfaceKHR surface, VkBool32* pSupported,
                                                               const RecordObject& record_obj) {
    StartReadObject(surface, record_obj.location);
}

void Instance::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                VkSurfaceKHR surface, VkBool32* pSupported,
                                                                const RecordObject& record_obj) {
    FinishReadObject(surface, record_obj.location);
}

void Instance::PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                    VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                    const RecordObject& record_obj) {
    StartReadObject(surface, record_obj.location);
}

void Instance::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                     const RecordObject& record_obj) {
    FinishReadObject(surface, record_obj.location);
}

void Instance::PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                               uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                               const RecordObject& record_obj) {
    StartReadObject(surface, record_obj.location);
}

void Instance::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                                const RecordObject& record_obj) {
    FinishReadObject(surface, record_obj.location);
}

void Instance::PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                    uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                    const RecordObject& record_obj) {
    StartReadObject(surface, record_obj.location);
}

void Instance::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                     const RecordObject& record_obj) {
    FinishReadObject(surface, record_obj.location);
}

void Device::PreCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObjectParentInstance(pCreateInfo->surface, record_obj.location);
    StartWriteObject(pCreateInfo->oldSwapchain, record_obj.location);
    // Host access to pCreateInfo->surface,pCreateInfo->oldSwapchain must be externally synchronized
}

void Device::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObjectParentInstance(pCreateInfo->surface, record_obj.location);
    FinishWriteObject(pCreateInfo->oldSwapchain, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSwapchain);
    }
    // Host access to pCreateInfo->surface,pCreateInfo->oldSwapchain must be externally synchronized
}

void Device::PreCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                              VkFence fence, uint32_t* pImageIndex, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(swapchain, record_obj.location);
    StartWriteObject(semaphore, record_obj.location);
    StartWriteObject(fence, record_obj.location);
    // Host access to swapchain must be externally synchronized
    // Host access to semaphore must be externally synchronized
    // Host access to fence must be externally synchronized
}

void Device::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                               VkFence fence, uint32_t* pImageIndex, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(swapchain, record_obj.location);
    FinishWriteObject(semaphore, record_obj.location);
    FinishWriteObject(fence, record_obj.location);
    // Host access to swapchain must be externally synchronized
    // Host access to semaphore must be externally synchronized
    // Host access to fence must be externally synchronized
}

void Device::PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(VkDevice device,
                                                               VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                               VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObjectParentInstance(surface, record_obj.location);
    // Host access to surface must be externally synchronized
}

void Device::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObjectParentInstance(surface, record_obj.location);
    // Host access to surface must be externally synchronized
}

void Instance::PreCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  uint32_t* pRectCount, VkRect2D* pRects,
                                                                  const RecordObject& record_obj) {
    StartWriteObject(surface, record_obj.location);
    // Host access to surface must be externally synchronized
}

void Instance::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                   uint32_t* pRectCount, VkRect2D* pRects,
                                                                   const RecordObject& record_obj) {
    FinishWriteObject(surface, record_obj.location);
    // Host access to surface must be externally synchronized
}

void Device::PreCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                               uint32_t* pImageIndex, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                uint32_t* pImageIndex, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Instance::PreCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                 const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                                 const RecordObject& record_obj) {
    StartWriteObject(display, record_obj.location);
    // Host access to display must be externally synchronized
}

void Instance::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                  const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(display, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pMode);
    }
    // Host access to display must be externally synchronized
}

void Instance::PreCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                           uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                           const RecordObject& record_obj) {
    StartWriteObject(mode, record_obj.location);
    // Host access to mode must be externally synchronized
}

void Instance::PostCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                            uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(mode, record_obj.location);
    // Host access to mode must be externally synchronized
}

void Instance::PreCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

void Device::PreCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                    const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    if (pCreateInfos) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            StartWriteObjectParentInstance(pCreateInfos[index].surface, record_obj.location);
            StartWriteObject(pCreateInfos[index].oldSwapchain, record_obj.location);
        }
    }

    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            StartReadObject(pSwapchains[index], record_obj.location);
        }
    }
    // Host access to pCreateInfos[].surface,pCreateInfos[].oldSwapchain must be externally synchronized
}

void Device::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                     const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (pCreateInfos) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            FinishWriteObjectParentInstance(pCreateInfos[index].surface, record_obj.location);
            FinishWriteObject(pCreateInfos[index].oldSwapchain, record_obj.location);
        }
    }
    if (record_obj.result == VK_SUCCESS) {
        if (pSwapchains) {
            for (uint32_t index = 0; index < swapchainCount; index++) {
                CreateObject(pSwapchains[index]);
            }
        }
    }
    // Host access to pCreateInfos[].surface,pCreateInfos[].oldSwapchain must be externally synchronized
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
void Instance::PreCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
void Instance::PreCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void Instance::PreCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
void Instance::PreCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void Instance::PreCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void Device::PreCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pVideoSession);
    }
}

void Device::PreCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(videoSession, record_obj.location);
    // Host access to videoSession must be externally synchronized
}

void Device::PostCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                  const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(videoSession, record_obj.location);
    DestroyObject(videoSession);
    // Host access to videoSession must be externally synchronized
}

void Device::PreCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                               uint32_t* pMemoryRequirementsCount,
                                                               VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(videoSession, record_obj.location);
}

void Device::PostCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                uint32_t* pMemoryRequirementsCount,
                                                                VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(videoSession, record_obj.location);
}

void Device::PreCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                    uint32_t bindSessionMemoryInfoCount,
                                                    const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(videoSession, record_obj.location);
    // Host access to videoSession must be externally synchronized
}

void Device::PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                     uint32_t bindSessionMemoryInfoCount,
                                                     const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(videoSession, record_obj.location);
    // Host access to videoSession must be externally synchronized
}

void Device::PreCallRecordCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateVideoSessionParametersKHR(VkDevice device,
                                                           const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pVideoSessionParameters);
    }
}

void Device::PreCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                          const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(videoSessionParameters, record_obj.location);
}

void Device::PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                           const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(videoSessionParameters, record_obj.location);
}

void Device::PreCallRecordDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(videoSessionParameters, record_obj.location);
    // Host access to videoSessionParameters must be externally synchronized
}

void Device::PostCallRecordDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(videoSessionParameters, record_obj.location);
    DestroyObject(videoSessionParameters);
    // Host access to videoSessionParameters must be externally synchronized
}

void Device::PreCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                   const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                    const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                               const RecordObject& record_obj) {
    PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
}

void Device::PostCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                const RecordObject& record_obj) {
    PostCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
}

void Device::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    PreCallRecordCmdEndRendering(commandBuffer, record_obj);
}

void Device::PostCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    PostCallRecordCmdEndRendering(commandBuffer, record_obj);
}

void Device::PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                              uint32_t remoteDeviceIndex,
                                                              VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                              const RecordObject& record_obj) {
    PreCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures,
                                                  record_obj);
}

void Device::PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                               uint32_t remoteDeviceIndex,
                                                               VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                               const RecordObject& record_obj) {
    PostCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures,
                                                   record_obj);
}

void Device::PreCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask, const RecordObject& record_obj) {
    PreCallRecordCmdSetDeviceMask(commandBuffer, deviceMask, record_obj);
}

void Device::PostCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask, const RecordObject& record_obj) {
    PostCallRecordCmdSetDeviceMask(commandBuffer, deviceMask, record_obj);
}

void Device::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                             uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                             const RecordObject& record_obj) {
    PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                 record_obj);
}

void Device::PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                              uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                              const RecordObject& record_obj) {
    PostCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                  record_obj);
}

void Device::PreCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                             const RecordObject& record_obj) {
    PreCallRecordTrimCommandPool(device, commandPool, flags, record_obj);
}

void Device::PostCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                              const RecordObject& record_obj) {
    PostCallRecordTrimCommandPool(device, commandPool, flags, record_obj);
}

void Instance::PreCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                             VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                             const RecordObject& record_obj) {
    PreCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, record_obj);
}

void Instance::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                              VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                              const RecordObject& record_obj) {
    PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, record_obj);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void Device::PreCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                  HANDLE* pHandle, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                   HANDLE* pHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                            HANDLE handle,
                                                            VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                             HANDLE handle,
                                                             VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void Device::PreCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                   VkMemoryFdPropertiesKHR* pMemoryFdProperties, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                    VkMemoryFdPropertiesKHR* pMemoryFdProperties, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void Device::PreCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                        const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                         const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                     HANDLE* pHandle, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                      HANDLE* pHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void Device::PreCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                  VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                  const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject& record_obj) {
    PreCallRecordCmdPushDescriptorSet(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites,
                                      record_obj);
}

void Device::PostCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                   VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                   const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject& record_obj) {
    PostCallRecordCmdPushDescriptorSet(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites,
                                       record_obj);
}

void Device::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                              VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                              VkPipelineLayout layout, uint32_t set, const void* pData,
                                                              const RecordObject& record_obj) {
    PreCallRecordCmdPushDescriptorSetWithTemplate(commandBuffer, descriptorUpdateTemplate, layout, set, pData, record_obj);
}

void Device::PostCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                               VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                               VkPipelineLayout layout, uint32_t set, const void* pData,
                                                               const RecordObject& record_obj) {
    PostCallRecordCmdPushDescriptorSetWithTemplate(commandBuffer, descriptorUpdateTemplate, layout, set, pData, record_obj);
}

void Device::PreCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                            const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                            const RecordObject& record_obj) {
    PreCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, record_obj);
}

void Device::PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                             const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                             const RecordObject& record_obj) {
    PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, record_obj);
}

void Device::PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             const RecordObject& record_obj) {
    PreCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator, record_obj);
}

void Device::PostCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              const RecordObject& record_obj) {
    PostCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator, record_obj);
}

void Device::PreCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                               const RecordObject& record_obj) {
    PreCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, record_obj);
}

void Device::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                const RecordObject& record_obj) {
    PostCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, record_obj);
}

void Device::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                 const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {
    PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
}

void Device::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                  const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {
    PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
}

void Device::PreCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                             const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {
    PreCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
}

void Device::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                              const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {
    PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
}

void Device::PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                               const RecordObject& record_obj) {
    PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void Device::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                const RecordObject& record_obj) {
    PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void Device::PreCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

void Device::PostCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void Device::PreCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                    const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                     const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                 HANDLE* pHandle, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                  HANDLE* pHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void Device::PreCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                         VkMemoryRequirements2* pMemoryRequirements,
                                                         const RecordObject& record_obj) {
    PreCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements, record_obj);
}

void Device::PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                          VkMemoryRequirements2* pMemoryRequirements,
                                                          const RecordObject& record_obj) {
    PostCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements, record_obj);
}

void Device::PreCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                          VkMemoryRequirements2* pMemoryRequirements,
                                                          const RecordObject& record_obj) {
    PreCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements, record_obj);
}

void Device::PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                           VkMemoryRequirements2* pMemoryRequirements,
                                                           const RecordObject& record_obj) {
    PostCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements, record_obj);
}

void Device::PreCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                               uint32_t* pSparseMemoryRequirementCount,
                                                               VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                               const RecordObject& record_obj) {
    PreCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements,
                                                   record_obj);
}

void Device::PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                uint32_t* pSparseMemoryRequirementCount,
                                                                VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                const RecordObject& record_obj) {
    PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements,
                                                    record_obj);
}

void Device::PreCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkSamplerYcbcrConversion* pYcbcrConversion,
                                                          const RecordObject& record_obj) {
    PreCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, record_obj);
}

void Device::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkSamplerYcbcrConversion* pYcbcrConversion,
                                                           const RecordObject& record_obj) {
    PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, record_obj);
}

void Device::PreCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    PreCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator, record_obj);
}

void Device::PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const RecordObject& record_obj) {
    PostCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator, record_obj);
}

void Device::PreCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                               const RecordObject& record_obj) {
    PreCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void Device::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                                const RecordObject& record_obj) {
    PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void Device::PreCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                              const RecordObject& record_obj) {
    PreCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void Device::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                               const RecordObject& record_obj) {
    PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void Device::PreCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                           VkDescriptorSetLayoutSupport* pSupport, const RecordObject& record_obj) {
    PreCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport, record_obj);
}

void Device::PostCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                            VkDescriptorSetLayoutSupport* pSupport,
                                                            const RecordObject& record_obj) {
    PostCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport, record_obj);
}

void Device::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject& record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

void Device::PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                       record_obj);
}

void Device::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

void Device::PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                              record_obj);
}

void Device::PreCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                      const RecordObject& record_obj) {
    PreCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);
}

void Device::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                       const RecordObject& record_obj) {
    PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);
}

void Device::PreCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                            const RecordObject& record_obj) {
    PreCallRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
}

void Device::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                             const RecordObject& record_obj) {
    PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
}

void Device::PreCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                             const RecordObject& record_obj) {
    PreCallRecordSignalSemaphore(device, pSignalInfo, record_obj);
}

void Device::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                              const RecordObject& record_obj) {
    PostCallRecordSignalSemaphore(device, pSignalInfo, record_obj);
}

void Device::PreCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                       const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                       const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                        const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer,
                                                                const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                                const RecordObject& record_obj) {
    PreCallRecordCmdSetRenderingAttachmentLocations(commandBuffer, pLocationInfo, record_obj);
}

void Device::PostCallRecordCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer,
                                                                 const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                                 const RecordObject& record_obj) {
    PostCallRecordCmdSetRenderingAttachmentLocations(commandBuffer, pLocationInfo, record_obj);
}

void Device::PreCallRecordCmdSetRenderingInputAttachmentIndicesKHR(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
    const RecordObject& record_obj) {
    PreCallRecordCmdSetRenderingInputAttachmentIndices(commandBuffer, pInputAttachmentIndexInfo, record_obj);
}

void Device::PostCallRecordCmdSetRenderingInputAttachmentIndicesKHR(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
    const RecordObject& record_obj) {
    PostCallRecordCmdSetRenderingInputAttachmentIndices(commandBuffer, pInputAttachmentIndexInfo, record_obj);
}

void Device::PreCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                    const RecordObject& record_obj) {
    PreCallRecordGetBufferDeviceAddress(device, pInfo, record_obj);
}

void Device::PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                     const RecordObject& record_obj) {
    PostCallRecordGetBufferDeviceAddress(device, pInfo, record_obj);
}

void Device::PreCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                           const RecordObject& record_obj) {
    PreCallRecordGetBufferOpaqueCaptureAddress(device, pInfo, record_obj);
}

void Device::PostCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                            const RecordObject& record_obj) {
    PostCallRecordGetBufferOpaqueCaptureAddress(device, pInfo, record_obj);
}

void Device::PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                 const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                 const RecordObject& record_obj) {
    PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo, record_obj);
}

void Device::PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                  const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                  const RecordObject& record_obj) {
    PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo, record_obj);
}

void Device::PreCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                     VkDeferredOperationKHR* pDeferredOperation, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                      VkDeferredOperationKHR* pDeferredOperation, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pDeferredOperation);
    }
}

void Device::PreCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(operation, record_obj.location);
    // Host access to operation must be externally synchronized
}

void Device::PostCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                       const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(operation, record_obj.location);
    DestroyObject(operation);
    // Host access to operation must be externally synchronized
}

void Device::PreCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(operation, record_obj.location);
}

void Device::PostCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(operation, record_obj.location);
}

void Device::PreCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(operation, record_obj.location);
}

void Device::PostCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(operation, record_obj.location);
}

void Device::PreCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(operation, record_obj.location);
}

void Device::PostCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(operation, record_obj.location);
}

void Device::PreCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                             uint32_t* pExecutableCount,
                                                             VkPipelineExecutablePropertiesKHR* pProperties,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                              uint32_t* pExecutableCount,
                                                              VkPipelineExecutablePropertiesKHR* pProperties,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                             uint32_t* pStatisticCount,
                                                             VkPipelineExecutableStatisticKHR* pStatistics,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                              uint32_t* pStatisticCount,
                                                              VkPipelineExecutableStatisticKHR* pStatistics,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                        const RecordObject& record_obj) {
    PreCallRecordMapMemory2(device, pMemoryMapInfo, ppData, record_obj);
}

void Device::PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                         const RecordObject& record_obj) {
    PostCallRecordMapMemory2(device, pMemoryMapInfo, ppData, record_obj);
}

void Device::PreCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                          const RecordObject& record_obj) {
    PreCallRecordUnmapMemory2(device, pMemoryUnmapInfo, record_obj);
}

void Device::PostCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                           const RecordObject& record_obj) {
    PostCallRecordUnmapMemory2(device, pMemoryUnmapInfo, record_obj);
}

void Device::PreCallRecordGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                          const RecordObject& record_obj) {
    PreCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
}

void Device::PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                           const RecordObject& record_obj) {
    PostCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
}

void Device::PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                            const RecordObject& record_obj) {
    PreCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
}

void Device::PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                             const RecordObject& record_obj) {
    PostCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
}

void Device::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                            const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {
    PreCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
}

void Device::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                             const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {
    PostCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
}

void Device::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                 const RecordObject& record_obj) {
    PreCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);
}

void Device::PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                  const RecordObject& record_obj) {
    PostCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);
}

void Device::PreCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                                uint32_t query, const RecordObject& record_obj) {
    PreCallRecordCmdWriteTimestamp2(commandBuffer, stage, queryPool, query, record_obj);
}

void Device::PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                                 uint32_t query, const RecordObject& record_obj) {
    PostCallRecordCmdWriteTimestamp2(commandBuffer, stage, queryPool, query, record_obj);
}

void Device::PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                          const RecordObject& record_obj) {
    PreCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

void Device::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                           const RecordObject& record_obj) {
    PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

void Device::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                            const RecordObject& record_obj) {
    PreCallRecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj);
}

void Device::PostCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                             const RecordObject& record_obj) {
    PostCallRecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj);
}

void Device::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                           const RecordObject& record_obj) {
    PreCallRecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj);
}

void Device::PostCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                            const RecordObject& record_obj) {
    PostCallRecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj);
}

void Device::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                   const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                   const RecordObject& record_obj) {
    PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);
}

void Device::PostCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                    const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                    const RecordObject& record_obj) {
    PostCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);
}

void Device::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                   const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                   const RecordObject& record_obj) {
    PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
}

void Device::PostCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                    const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                    const RecordObject& record_obj) {
    PostCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
}

void Device::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                           const RecordObject& record_obj) {
    PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
}

void Device::PostCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                            const RecordObject& record_obj) {
    PostCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
}

void Device::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                              const RecordObject& record_obj) {
    PreCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo, record_obj);
}

void Device::PostCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                               const RecordObject& record_obj) {
    PostCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo, record_obj);
}

void Device::PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                               VkMemoryRequirements2* pMemoryRequirements,
                                                               const RecordObject& record_obj) {
    PreCallRecordGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements, record_obj);
}

void Device::PostCallRecordGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                                VkMemoryRequirements2* pMemoryRequirements,
                                                                const RecordObject& record_obj) {
    PostCallRecordGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements, record_obj);
}

void Device::PreCallRecordGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                              VkMemoryRequirements2* pMemoryRequirements,
                                                              const RecordObject& record_obj) {
    PreCallRecordGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements, record_obj);
}

void Device::PostCallRecordGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                               VkMemoryRequirements2* pMemoryRequirements,
                                                               const RecordObject& record_obj) {
    PostCallRecordGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements, record_obj);
}

void Device::PreCallRecordGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                    uint32_t* pSparseMemoryRequirementCount,
                                                                    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                    const RecordObject& record_obj) {
    PreCallRecordGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements,
                                                        record_obj);
}

void Device::PostCallRecordGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                     uint32_t* pSparseMemoryRequirementCount,
                                                                     VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                     const RecordObject& record_obj) {
    PostCallRecordGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements,
                                                         record_obj);
}

void Device::PreCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkDeviceSize size, VkIndexType indexType, const RecordObject& record_obj) {
    PreCallRecordCmdBindIndexBuffer2(commandBuffer, buffer, offset, size, indexType, record_obj);
}

void Device::PostCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkDeviceSize size, VkIndexType indexType, const RecordObject& record_obj) {
    PostCallRecordCmdBindIndexBuffer2(commandBuffer, buffer, offset, size, indexType, record_obj);
}

void Device::PreCallRecordGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                         VkExtent2D* pGranularity, const RecordObject& record_obj) {
    PreCallRecordGetRenderingAreaGranularity(device, pRenderingAreaInfo, pGranularity, record_obj);
}

void Device::PostCallRecordGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                          VkExtent2D* pGranularity, const RecordObject& record_obj) {
    PostCallRecordGetRenderingAreaGranularity(device, pRenderingAreaInfo, pGranularity, record_obj);
}

void Device::PreCallRecordGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                             VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {
    PreCallRecordGetDeviceImageSubresourceLayout(device, pInfo, pLayout, record_obj);
}

void Device::PostCallRecordGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                              VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {
    PostCallRecordGetDeviceImageSubresourceLayout(device, pInfo, pLayout, record_obj);
}

void Device::PreCallRecordGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                        VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {
    PreCallRecordGetImageSubresourceLayout2(device, image, pSubresource, pLayout, record_obj);
}

void Device::PostCallRecordGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                         VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {
    PostCallRecordGetImageSubresourceLayout2(device, image, pSubresource, pLayout, record_obj);
}

void Device::PreCallRecordDestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary,
                                                   const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(pipelineBinary, record_obj.location);
    // Host access to pipelineBinary must be externally synchronized
}

void Device::PostCallRecordDestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(pipelineBinary, record_obj.location);
    DestroyObject(pipelineBinary);
    // Host access to pipelineBinary must be externally synchronized
}

void Device::PreCallRecordGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR* pPipelineCreateInfo,
                                            VkPipelineBinaryKeyKHR* pPipelineKey, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR* pPipelineCreateInfo,
                                             VkPipelineBinaryKeyKHR* pPipelineKey, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
                                                   VkPipelineBinaryKeyKHR* pPipelineBinaryKey, size_t* pPipelineBinaryDataSize,
                                                   void* pPipelineBinaryData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
                                                    VkPipelineBinaryKeyKHR* pPipelineBinaryKey, size_t* pPipelineBinaryDataSize,
                                                    void* pPipelineBinaryData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(pInfo->pipeline, record_obj.location);
    // Host access to pInfo->pipeline must be externally synchronized
}

void Device::PostCallRecordReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(pInfo->pipeline, record_obj.location);
    // Host access to pInfo->pipeline must be externally synchronized
}

void Device::PreCallRecordCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                               uint16_t lineStipplePattern, const RecordObject& record_obj) {
    PreCallRecordCmdSetLineStipple(commandBuffer, lineStippleFactor, lineStipplePattern, record_obj);
}

void Device::PostCallRecordCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                uint16_t lineStipplePattern, const RecordObject& record_obj) {
    PostCallRecordCmdSetLineStipple(commandBuffer, lineStippleFactor, lineStipplePattern, record_obj);
}

void Device::PreCallRecordGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                     const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                     uint64_t* pMaxDeviation, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                      const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                      uint64_t* pMaxDeviation, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                    const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                                    const RecordObject& record_obj) {
    PreCallRecordCmdBindDescriptorSets2(commandBuffer, pBindDescriptorSetsInfo, record_obj);
}

void Device::PostCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                     const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                                     const RecordObject& record_obj) {
    PostCallRecordCmdBindDescriptorSets2(commandBuffer, pBindDescriptorSetsInfo, record_obj);
}

void Device::PreCallRecordCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                               const RecordObject& record_obj) {
    PreCallRecordCmdPushConstants2(commandBuffer, pPushConstantsInfo, record_obj);
}

void Device::PostCallRecordCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                                const RecordObject& record_obj) {
    PostCallRecordCmdPushConstants2(commandBuffer, pPushConstantsInfo, record_obj);
}

void Device::PreCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                   const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                                   const RecordObject& record_obj) {
    PreCallRecordCmdPushDescriptorSet2(commandBuffer, pPushDescriptorSetInfo, record_obj);
}

void Device::PostCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                    const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                                    const RecordObject& record_obj) {
    PostCallRecordCmdPushDescriptorSet2(commandBuffer, pPushDescriptorSetInfo, record_obj);
}

void Device::PreCallRecordCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
    const RecordObject& record_obj) {
    PreCallRecordCmdPushDescriptorSetWithTemplate2(commandBuffer, pPushDescriptorSetWithTemplateInfo, record_obj);
}

void Device::PostCallRecordCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
    const RecordObject& record_obj) {
    PostCallRecordCmdPushDescriptorSetWithTemplate2(commandBuffer, pPushDescriptorSetWithTemplateInfo, record_obj);
}

void Device::PreCallRecordCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo,
    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Instance::PreCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDebugReportCallbackEXT* pCallback, const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance,
                                                          const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkDebugReportCallbackEXT* pCallback, const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pCallback);
    }
}

void Instance::PreCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
    StartWriteObject(callback, record_obj.location);
    // Host access to callback must be externally synchronized
}

void Instance::PostCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    FinishWriteObject(callback, record_obj.location);
    DestroyObject(callback);
    // Host access to callback must be externally synchronized
}

void Instance::PreCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                  VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                  int32_t messageCode, const char* pLayerPrefix, const char* pMessage,
                                                  const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                   VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                   int32_t messageCode, const char* pLayerPrefix, const char* pMessage,
                                                   const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
}

void Device::PreCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                             uint32_t bindingCount, const VkBuffer* pBuffers,
                                                             const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                              uint32_t bindingCount, const VkBuffer* pBuffers,
                                                              const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                       uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                       const VkDeviceSize* pCounterBufferOffsets, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            StartReadObject(pCounterBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                        uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                        const VkDeviceSize* pCounterBufferOffsets, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            FinishReadObject(pCounterBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                     uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                     const VkDeviceSize* pCounterBufferOffsets, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            StartReadObject(pCounterBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                      uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                      const VkDeviceSize* pCounterBufferOffsets, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            FinishReadObject(pCounterBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                  VkQueryControlFlags flags, uint32_t index, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                   VkQueryControlFlags flags, uint32_t index, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                uint32_t index, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                 uint32_t index, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                      VkBuffer counterBuffer, VkDeviceSize counterBufferOffset,
                                                      uint32_t counterOffset, uint32_t vertexStride,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(counterBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                       uint32_t firstInstance, VkBuffer counterBuffer,
                                                       VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                       uint32_t vertexStride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(counterBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pModule);
    }
}

void Device::PreCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFunction);
    }
}

void Device::PreCallRecordDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(module, record_obj.location);
}

void Device::PostCallRecordDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(module, record_obj.location);
}

void Device::PreCallRecordDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(function, record_obj.location);
}

void Device::PostCallRecordDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(function, record_obj.location);
}

void Device::PreCallRecordCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                               const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
}

void Device::PostCallRecordCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                                const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
}

void Device::PreCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                 VkImageViewAddressPropertiesNVX* pProperties, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(imageView, record_obj.location);
}

void Device::PostCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                  VkImageViewAddressPropertiesNVX* pProperties, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(imageView, record_obj.location);
}

void Device::PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject& record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

void Device::PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                       record_obj);
}

void Device::PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

void Device::PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                              record_obj);
}

void Device::PreCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                           VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
}

void Device::PostCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                            VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
}

#ifdef VK_USE_PLATFORM_GGP
void Instance::PreCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                             const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                             const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                              const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                              const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_GGP
#ifdef VK_USE_PLATFORM_WIN32_KHR
void Device::PreCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                 VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(memory, record_obj.location);
}

void Device::PostCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                  VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(memory, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN
void Instance::PreCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                              const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                               const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_VI_NN
void Device::PreCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                          const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                           const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                   const VkViewportWScalingNV* pViewportWScalings, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                    const VkViewportWScalingNV* pViewportWScalings,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Instance::PreCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                              const RecordObject& record_obj) {
    StartReadObject(display, record_obj.location);
}

void Instance::PostCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                               const RecordObject& record_obj) {
    FinishReadObject(display, record_obj.location);
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
void Instance::PreCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                                  const RecordObject& record_obj) {
    StartReadObject(display, record_obj.location);
}

void Instance::PostCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                                   const RecordObject& record_obj) {
    FinishReadObject(display, record_obj.location);
}

#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
void Instance::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                     const RecordObject& record_obj) {
    StartReadObject(surface, record_obj.location);
}

void Instance::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                      VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                      const RecordObject& record_obj) {
    FinishReadObject(surface, record_obj.location);
}

void Device::PreCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                 const VkDisplayPowerInfoEXT* pDisplayPowerInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObjectParentInstance(display, record_obj.location);
}

void Device::PostCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                  const VkDisplayPowerInfoEXT* pDisplayPowerInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObjectParentInstance(display, record_obj.location);
}

void Device::PreCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                                 uint64_t* pCounterValue, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void Device::PostCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                                  uint64_t* pCounterValue, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void Device::PreCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                        VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

void Device::PostCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                         VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

void Device::PreCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                          uint32_t* pPresentationTimingCount,
                                                          VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

void Device::PostCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                           uint32_t* pPresentationTimingCount,
                                                           VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

void Device::PreCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                    uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                     uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                        VkDiscardRectangleModeEXT discardRectangleMode,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                         VkDiscardRectangleModeEXT discardRectangleMode,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                            const VkHdrMetadataEXT* pMetadata, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);

    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            StartReadObject(pSwapchains[index], record_obj.location);
        }
    }
}

void Device::PostCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                             const VkHdrMetadataEXT* pMetadata, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);

    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            FinishReadObject(pSwapchains[index], record_obj.location);
        }
    }
}

#ifdef VK_USE_PLATFORM_IOS_MVK
void Instance::PreCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
void Instance::PreCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_MACOS_MVK
void Device::PreCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                       const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void Device::PostCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                        const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void Device::PreCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue, const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void Device::PostCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue, const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void Device::PreCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                        const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void Device::PostCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void Device::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Instance::PreCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDebugUtilsMessengerEXT* pMessenger, const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                          const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkDebugUtilsMessengerEXT* pMessenger, const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pMessenger);
    }
}

void Instance::PreCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
    StartWriteObject(messenger, record_obj.location);
    // Host access to messenger must be externally synchronized
}

void Instance::PostCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    FinishWriteObject(messenger, record_obj.location);
    DestroyObject(messenger);
    // Host access to messenger must be externally synchronized
}

void Instance::PreCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                       const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void Device::PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                    VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                     VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                struct AHardwareBuffer** pBuffer, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                 const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                 struct AHardwareBuffer** pBuffer, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
void Device::PreCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                            uint32_t createInfoCount,
                                                            const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipelineCache, record_obj.location);
}

void Device::PostCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                             uint32_t createInfoCount,
                                                             const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipelineCache, record_obj.location);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void Device::PreCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                   VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(executionGraph, record_obj.location);
}

void Device::PostCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                    VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(executionGraph, record_obj.location);
}

void Device::PreCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                 const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                 uint32_t* pNodeIndex, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(executionGraph, record_obj.location);
}

void Device::PostCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                  const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                  uint32_t* pNodeIndex, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(executionGraph, record_obj.location);
}

void Device::PreCallRecordCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkPipeline executionGraph,
                                                              VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                              const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
    StartReadObject(executionGraph, record_obj.location);
}

void Device::PostCallRecordCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkPipeline executionGraph,
                                                               VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                               const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
    FinishReadObject(executionGraph, record_obj.location);
}

void Device::PreCallRecordCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                               const VkDispatchGraphCountInfoAMDX* pCountInfo, const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
}

void Device::PostCallRecordCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                const VkDispatchGraphCountInfoAMDX* pCountInfo, const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
}

void Device::PreCallRecordCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                       VkDeviceSize scratchSize, const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                       const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
}

void Device::PostCallRecordCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                        VkDeviceSize scratchSize, const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                        const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
}

void Device::PreCallRecordCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                            VkDeviceSize scratchSize, VkDeviceAddress countInfo,
                                                            const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
}

void Device::PostCallRecordCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                             VkDeviceSize scratchSize, VkDeviceAddress countInfo,
                                                             const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
}

#endif  // VK_ENABLE_BETA_EXTENSIONS
void Device::PreCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                   const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                    const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                 VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(image, record_obj.location);
}

void Device::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                  VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(image, record_obj.location);
}

void Device::PreCallRecordCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pValidationCache);
    }
}

void Device::PreCallRecordDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(validationCache, record_obj.location);
    // Host access to validationCache must be externally synchronized
}

void Device::PostCallRecordDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                     const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(validationCache, record_obj.location);
    DestroyObject(validationCache);
    // Host access to validationCache must be externally synchronized
}

void Device::PreCallRecordMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                   const VkValidationCacheEXT* pSrcCaches, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(dstCache, record_obj.location);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            StartReadObject(pSrcCaches[index], record_obj.location);
        }
    }
    // Host access to dstCache must be externally synchronized
}

void Device::PostCallRecordMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                    const VkValidationCacheEXT* pSrcCaches, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(dstCache, record_obj.location);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            FinishReadObject(pSrcCaches[index], record_obj.location);
        }
    }
    // Host access to dstCache must be externally synchronized
}

void Device::PreCallRecordGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                    void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(validationCache, record_obj.location);
}

void Device::PostCallRecordGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                     void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(validationCache, record_obj.location);
}

void Device::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout,
                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(imageView, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                     VkImageLayout imageLayout, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(imageView, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                             uint32_t viewportCount,
                                                             const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                              uint32_t viewportCount,
                                                              const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                    uint32_t customSampleOrderCount,
                                                    const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                     uint32_t customSampleOrderCount,
                                                     const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkAccelerationStructureNV* pAccelerationStructure,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkAccelerationStructureNV* pAccelerationStructure,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pAccelerationStructure);
    }
}

void Device::PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(accelerationStructure, record_obj.location);
    // Host access to accelerationStructure must be externally synchronized
}

void Device::PostCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(accelerationStructure, record_obj.location);
    DestroyObject(accelerationStructure);
    // Host access to accelerationStructure must be externally synchronized
}

void Device::PreCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                                       const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                                       VkMemoryRequirements2KHR* pMemoryRequirements,
                                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                            const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                             const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                          VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                          VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                          VkBuffer scratch, VkDeviceSize scratchOffset,
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(instanceData, record_obj.location);
    StartReadObject(dst, record_obj.location);
    StartReadObject(src, record_obj.location);
    StartReadObject(scratch, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                           const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData,
                                                           VkDeviceSize instanceOffset, VkBool32 update,
                                                           VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                           VkBuffer scratch, VkDeviceSize scratchOffset,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(instanceData, record_obj.location);
    FinishReadObject(dst, record_obj.location);
    FinishReadObject(src, record_obj.location);
    FinishReadObject(scratch, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                         VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dst, record_obj.location);
    StartReadObject(src, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                          VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dst, record_obj.location);
    FinishReadObject(src, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                         VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                         VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                         VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                         VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                         VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                         uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(raygenShaderBindingTableBuffer, record_obj.location);
    StartReadObject(missShaderBindingTableBuffer, record_obj.location);
    StartReadObject(hitShaderBindingTableBuffer, record_obj.location);
    StartReadObject(callableShaderBindingTableBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                          VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                          VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                          VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                          VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                          VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                          uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(raygenShaderBindingTableBuffer, record_obj.location);
    FinishReadObject(missShaderBindingTableBuffer, record_obj.location);
    FinishReadObject(hitShaderBindingTableBuffer, record_obj.location);
    FinishReadObject(callableShaderBindingTableBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipelineCache, record_obj.location);
}

void Device::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                       const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipelineCache, record_obj.location);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void Device::PreCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                             uint32_t groupCount, size_t dataSize, void* pData,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
}

void Device::PostCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                              uint32_t groupCount, size_t dataSize, void* pData,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
}

void Device::PreCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                            uint32_t groupCount, size_t dataSize, void* pData,
                                                            const RecordObject& record_obj) {
    PreCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, record_obj);
}

void Device::PostCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                             uint32_t groupCount, size_t dataSize, void* pData,
                                                             const RecordObject& record_obj) {
    PostCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, record_obj);
}

void Device::PreCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                           size_t dataSize, void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(accelerationStructure, record_obj.location);
}

void Device::PostCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                            size_t dataSize, void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(accelerationStructure, record_obj.location);
}

void Device::PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                     uint32_t accelerationStructureCount,
                                                                     const VkAccelerationStructureNV* pAccelerationStructures,
                                                                     VkQueryType queryType, VkQueryPool queryPool,
                                                                     uint32_t firstQuery, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                      uint32_t accelerationStructureCount,
                                                                      const VkAccelerationStructureNV* pAccelerationStructures,
                                                                      VkQueryType queryType, VkQueryPool queryPool,
                                                                      uint32_t firstQuery, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
}

void Device::PostCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
}

void Device::PreCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                            const void* pHostPointer,
                                                            VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                             const void* pHostPointer,
                                                             VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                  VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                   VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                                   VkDeviceSize dstOffset, uint32_t marker, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                                    VkDeviceSize dstOffset, uint32_t marker, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                     const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                     uint64_t* pMaxDeviation, const RecordObject& record_obj) {
    PreCallRecordGetCalibratedTimestampsKHR(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, record_obj);
}

void Device::PostCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                      const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                      uint64_t* pMaxDeviation, const RecordObject& record_obj) {
    PostCallRecordGetCalibratedTimestampsKHR(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, record_obj);
}

void Device::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    StartReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    FinishReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                         uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                          uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                   uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                    uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                   VkCheckpointDataNV* pCheckpointData, const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void Device::PostCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                    VkCheckpointDataNV* pCheckpointData, const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void Device::PreCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                    VkCheckpointData2NV* pCheckpointData, const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void Device::PostCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                     VkCheckpointData2NV* pCheckpointData, const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void Device::PreCallRecordInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordInitializePerformanceApiINTEL(VkDevice device,
                                                         const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordUninitializePerformanceApiINTEL(VkDevice device, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordUninitializePerformanceApiINTEL(VkDevice device, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                       const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                       const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                        const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                             const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                              const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                         const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                          const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                               const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                               VkPerformanceConfigurationINTEL* pConfiguration,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                                const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                                VkPerformanceConfigurationINTEL* pConfiguration,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pConfiguration);
    }
}

void Device::PreCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(configuration, record_obj.location);
    // Host access to configuration must be externally synchronized
}

void Device::PostCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(configuration, record_obj.location);
    DestroyObject(configuration);
    // Host access to configuration must be externally synchronized
}

void Device::PreCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                                const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
    StartReadObject(configuration, record_obj.location);
}

void Device::PostCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                                 const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
    FinishReadObject(configuration, record_obj.location);
}

void Device::PreCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                       VkPerformanceValueINTEL* pValue, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                        VkPerformanceValueINTEL* pValue, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapChain, record_obj.location);
}

void Device::PostCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapChain, record_obj.location);
}

#ifdef VK_USE_PLATFORM_FUCHSIA
void Instance::PreCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                          const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                           const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                           const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
void Instance::PreCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_METAL_EXT
void Device::PreCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                    const RecordObject& record_obj) {
    PreCallRecordGetBufferDeviceAddress(device, pInfo, record_obj);
}

void Device::PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                     const RecordObject& record_obj) {
    PostCallRecordGetBufferDeviceAddress(device, pInfo, record_obj);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void Device::PreCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void Device::PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void Device::PreCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void Device::PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void Device::PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                 const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                 VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void Instance::PreCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                      const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

void Device::PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                               uint16_t lineStipplePattern, const RecordObject& record_obj) {
    PreCallRecordCmdSetLineStipple(commandBuffer, lineStippleFactor, lineStipplePattern, record_obj);
}

void Device::PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                uint16_t lineStipplePattern, const RecordObject& record_obj) {
    PostCallRecordCmdSetLineStipple(commandBuffer, lineStippleFactor, lineStipplePattern, record_obj);
}

void Device::PreCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                            const RecordObject& record_obj) {
    PreCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount, record_obj);
}

void Device::PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                             const RecordObject& record_obj) {
    PostCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount, record_obj);
}

void Device::PreCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                            const RecordObject& record_obj) {
    PreCallRecordCmdSetCullMode(commandBuffer, cullMode, record_obj);
}

void Device::PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                             const RecordObject& record_obj) {
    PostCallRecordCmdSetCullMode(commandBuffer, cullMode, record_obj);
}

void Device::PreCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace, const RecordObject& record_obj) {
    PreCallRecordCmdSetFrontFace(commandBuffer, frontFace, record_obj);
}

void Device::PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                              const RecordObject& record_obj) {
    PostCallRecordCmdSetFrontFace(commandBuffer, frontFace, record_obj);
}

void Device::PreCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                     const RecordObject& record_obj) {
    PreCallRecordCmdSetPrimitiveTopology(commandBuffer, primitiveTopology, record_obj);
}

void Device::PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                      const RecordObject& record_obj) {
    PostCallRecordCmdSetPrimitiveTopology(commandBuffer, primitiveTopology, record_obj);
}

void Device::PreCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                     const VkViewport* pViewports, const RecordObject& record_obj) {
    PreCallRecordCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, record_obj);
}

void Device::PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                      const VkViewport* pViewports, const RecordObject& record_obj) {
    PostCallRecordCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, record_obj);
}

void Device::PreCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                                    const RecordObject& record_obj) {
    PreCallRecordCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, record_obj);
}

void Device::PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                     const VkRect2D* pScissors, const RecordObject& record_obj) {
    PostCallRecordCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, record_obj);
}

void Device::PreCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                   const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                   const VkDeviceSize* pSizes, const VkDeviceSize* pStrides,
                                                   const RecordObject& record_obj) {
    PreCallRecordCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides, record_obj);
}

void Device::PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                    const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                    const VkDeviceSize* pSizes, const VkDeviceSize* pStrides,
                                                    const RecordObject& record_obj) {
    PostCallRecordCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                        record_obj);
}

void Device::PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                   const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, record_obj);
}

void Device::PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                    const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, record_obj);
}

void Device::PreCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                    const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable, record_obj);
}

void Device::PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                     const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable, record_obj);
}

void Device::PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                  const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, record_obj);
}

void Device::PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                   const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, record_obj);
}

void Device::PreCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                         const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable, record_obj);
}

void Device::PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                          const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable, record_obj);
}

void Device::PreCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                     const RecordObject& record_obj) {
    PreCallRecordCmdSetStencilTestEnable(commandBuffer, stencilTestEnable, record_obj);
}

void Device::PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                      const RecordObject& record_obj) {
    PostCallRecordCmdSetStencilTestEnable(commandBuffer, stencilTestEnable, record_obj);
}

void Device::PreCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                             VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                             const RecordObject& record_obj) {
    PreCallRecordCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp, record_obj);
}

void Device::PostCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                              VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                              const RecordObject& record_obj) {
    PostCallRecordCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp, record_obj);
}

void Device::PreCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                               const RecordObject& record_obj) {
    PreCallRecordCopyMemoryToImage(device, pCopyMemoryToImageInfo, record_obj);
}

void Device::PostCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                                const RecordObject& record_obj) {
    PostCallRecordCopyMemoryToImage(device, pCopyMemoryToImageInfo, record_obj);
}

void Device::PreCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                               const RecordObject& record_obj) {
    PreCallRecordCopyImageToMemory(device, pCopyImageToMemoryInfo, record_obj);
}

void Device::PostCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                                const RecordObject& record_obj) {
    PostCallRecordCopyImageToMemory(device, pCopyImageToMemoryInfo, record_obj);
}

void Device::PreCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                              const RecordObject& record_obj) {
    PreCallRecordCopyImageToImage(device, pCopyImageToImageInfo, record_obj);
}

void Device::PostCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                               const RecordObject& record_obj) {
    PostCallRecordCopyImageToImage(device, pCopyImageToImageInfo, record_obj);
}

void Device::PreCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                   const VkHostImageLayoutTransitionInfo* pTransitions,
                                                   const RecordObject& record_obj) {
    PreCallRecordTransitionImageLayout(device, transitionCount, pTransitions, record_obj);
}

void Device::PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                    const VkHostImageLayoutTransitionInfo* pTransitions,
                                                    const RecordObject& record_obj) {
    PostCallRecordTransitionImageLayout(device, transitionCount, pTransitions, record_obj);
}

void Device::PreCallRecordGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                        VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {
    PreCallRecordGetImageSubresourceLayout2(device, image, pSubresource, pLayout, record_obj);
}

void Device::PostCallRecordGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                         VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {
    PostCallRecordGetImageSubresourceLayout2(device, image, pSubresource, pLayout, record_obj);
}

void Device::PreCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                                   const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                                    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                           const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                           const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                            const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                        const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                         const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                       VkPipeline pipeline, uint32_t groupIndex, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipeline pipeline, uint32_t groupIndex, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pIndirectCommandsLayout);
    }
}

void Device::PreCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(indirectCommandsLayout, record_obj.location);
    // Host access to indirectCommandsLayout must be externally synchronized
}

void Device::PostCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(indirectCommandsLayout, record_obj.location);
    DestroyObject(indirectCommandsLayout);
    // Host access to indirectCommandsLayout must be externally synchronized
}

void Device::PreCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Instance::PreCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                                 const RecordObject& record_obj) {
    StartReadObject(display, record_obj.location);
}

void Instance::PostCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                                  const RecordObject& record_obj) {
    FinishReadObject(display, record_obj.location);
}

void Device::PreCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                   const RecordObject& record_obj) {
    PreCallRecordCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot, record_obj);
}

void Device::PostCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                    const RecordObject& record_obj) {
    PostCallRecordCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot, record_obj);
}

void Device::PreCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    PreCallRecordDestroyPrivateDataSlot(device, privateDataSlot, pAllocator, record_obj);
}

void Device::PostCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                     const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    PostCallRecordDestroyPrivateDataSlot(device, privateDataSlot, pAllocator, record_obj);
}

void Device::PreCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                            VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {
    PreCallRecordSetPrivateData(device, objectType, objectHandle, privateDataSlot, data, record_obj);
}

void Device::PostCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                             VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {
    PostCallRecordSetPrivateData(device, objectType, objectHandle, privateDataSlot, data, record_obj);
}

void Device::PreCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                            VkPrivateDataSlot privateDataSlot, uint64_t* pData, const RecordObject& record_obj) {
    PreCallRecordGetPrivateData(device, objectType, objectHandle, privateDataSlot, pData, record_obj);
}

void Device::PostCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                             VkPrivateDataSlot privateDataSlot, uint64_t* pData, const RecordObject& record_obj) {
    PostCallRecordGetPrivateData(device, objectType, objectHandle, privateDataSlot, pData, record_obj);
}

void Device::PreCallRecordCreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule,
                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pModule);
    }
}

void Device::PreCallRecordGetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(module, record_obj.location);
}

void Device::PostCallRecordGetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(module, record_obj.location);
}

void Device::PreCallRecordCreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFunction);
    }
}

void Device::PreCallRecordDestroyCudaModuleNV(VkDevice device, VkCudaModuleNV module, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(module, record_obj.location);
}

void Device::PostCallRecordDestroyCudaModuleNV(VkDevice device, VkCudaModuleNV module, const VkAllocationCallbacks* pAllocator,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(module, record_obj.location);
}

void Device::PreCallRecordDestroyCudaFunctionNV(VkDevice device, VkCudaFunctionNV function, const VkAllocationCallbacks* pAllocator,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(function, record_obj.location);
}

void Device::PostCallRecordDestroyCudaFunctionNV(VkDevice device, VkCudaFunctionNV function,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(function, record_obj.location);
}

void Device::PreCallRecordCmdCudaLaunchKernelNV(VkCommandBuffer commandBuffer, const VkCudaLaunchInfoNV* pLaunchInfo,
                                                const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
}

void Device::PostCallRecordCmdCudaLaunchKernelNV(VkCommandBuffer commandBuffer, const VkCudaLaunchInfoNV* pLaunchInfo,
                                                 const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
}

#ifdef VK_USE_PLATFORM_METAL_EXT
void Device::PreCallRecordExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_METAL_EXT
void Device::PreCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                        VkDeviceSize* pLayoutSizeInBytes, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(layout, record_obj.location);
}

void Device::PostCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                         VkDeviceSize* pLayoutSizeInBytes, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(layout, record_obj.location);
}

void Device::PreCallRecordGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                                 VkDeviceSize* pOffset, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(layout, record_obj.location);
}

void Device::PostCallRecordGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                                  VkDeviceSize* pOffset, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(layout, record_obj.location);
}

void Device::PreCallRecordGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                           void* pDescriptor, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                            void* pDescriptor, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                      const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                       const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                           VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                           const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                           const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                            VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                            const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                     VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                     uint32_t set, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                      VkPipelineBindPoint pipelineBindPoint,
                                                                      VkPipelineLayout layout, uint32_t set,
                                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                  const VkBufferCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                   const VkBufferCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                 void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                  void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                     const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                     void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                      const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                      void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                   const VkSamplerCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                    const VkSamplerCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                          const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                           const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                VkDeviceFaultInfoEXT* pFaultInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                 VkDeviceFaultInfoEXT* pFaultInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void Instance::PreCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                  const RecordObject& record_obj) {
    StartReadObject(display, record_obj.location);
}

void Instance::PostCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                   const RecordObject& record_obj) {
    FinishReadObject(display, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
void Instance::PreCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                      const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
void Device::PreCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                               const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                               uint32_t vertexAttributeDescriptionCount,
                                               const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                                uint32_t vertexAttributeDescriptionCount,
                                                const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

#ifdef VK_USE_PLATFORM_FUCHSIA
void Device::PreCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                       const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                       zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                        const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                        zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                          const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                          zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                           const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                           zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkBufferCollectionFUCHSIA* pCollection, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkBufferCollectionFUCHSIA* pCollection, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pCollection);
    }
}

void Device::PreCallRecordSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                     const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
                                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(collection, record_obj.location);
}

void Device::PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                      const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(collection, record_obj.location);
}

void Device::PreCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                      const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
                                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(collection, record_obj.location);
}

void Device::PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                       const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
                                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(collection, record_obj.location);
}

void Device::PreCallRecordDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(collection, record_obj.location);
}

void Device::PostCallRecordDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(collection, record_obj.location);
}

void Device::PreCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                               VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(collection, record_obj.location);
}

void Device::PostCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(collection, record_obj.location);
}

#endif  // VK_USE_PLATFORM_FUCHSIA
void Device::PreCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                        VkExtent2D* pMaxWorkgroupSize,
                                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(renderpass, record_obj.location);
}

void Device::PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                         VkExtent2D* pMaxWorkgroupSize,
                                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(renderpass, record_obj.location);
}

void Device::PreCallRecordCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                      VkImageLayout imageLayout, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(imageView, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                       VkImageLayout imageLayout, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(imageView, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetMemoryRemoteAddressNV(VkDevice device,
                                                   const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                   VkRemoteAddressNV* pAddress, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMemoryRemoteAddressNV(VkDevice device,
                                                    const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                    VkRemoteAddressNV* pAddress, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                   VkBaseOutStructure* pPipelineProperties, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                    VkBaseOutStructure* pPipelineProperties, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                           const RecordObject& record_obj) {
    PreCallRecordCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable, record_obj);
}

void Device::PostCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                            const RecordObject& record_obj) {
    PostCallRecordCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable, record_obj);
}

void Device::PreCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                   const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable, record_obj);
}

void Device::PostCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                    const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable, record_obj);
}

void Device::PreCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                          const RecordObject& record_obj) {
    PreCallRecordCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable, record_obj);
}

void Device::PostCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                           const RecordObject& record_obj) {
    PostCallRecordCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable, record_obj);
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void Instance::PreCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const RecordObject& record_obj) {
    StartReadObject(instance, record_obj.location);
}

void Instance::PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) {
    FinishReadObject(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_SCREEN_QNX
void Device::PreCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                    const VkBool32* pColorWriteEnables, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkBool32* pColorWriteEnables, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                          uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                           uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                 const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                 uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                  const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                  uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pMicromap);
    }
}

void Device::PreCallRecordDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(micromap, record_obj.location);
    // Host access to micromap must be externally synchronized
}

void Device::PostCallRecordDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(micromap, record_obj.location);
    DestroyObject(micromap);
    // Host access to micromap must be externally synchronized
}

void Device::PreCallRecordCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                               const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                            const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void Device::PostCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                             const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void Device::PreCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                          const VkCopyMicromapInfoEXT* pInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void Device::PostCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                           const VkCopyMicromapInfoEXT* pInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void Device::PreCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                  const VkCopyMicromapToMemoryInfoEXT* pInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void Device::PostCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                   const VkCopyMicromapToMemoryInfoEXT* pInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void Device::PreCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                  const VkCopyMemoryToMicromapInfoEXT* pInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void Device::PostCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                   const VkCopyMemoryToMicromapInfoEXT* pInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void Device::PreCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                                      VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            StartReadObject(pMicromaps[index], record_obj.location);
        }
    }
}

void Device::PostCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                                       VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            FinishReadObject(pMicromaps[index], record_obj.location);
        }
    }
}

void Device::PreCallRecordCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                         const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                         VkQueryPool queryPool, uint32_t firstQuery,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            StartReadObject(pMicromaps[index], record_obj.location);
        }
    }
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                          const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                          VkQueryPool queryPool, uint32_t firstQuery,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            FinishReadObject(pMicromaps[index], record_obj.location);
        }
    }
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                            VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                             VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                   const VkMicromapBuildInfoEXT* pBuildInfo, VkMicromapBuildSizesInfoEXT* pSizeInfo,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                    const VkMicromapBuildInfoEXT* pBuildInfo,
                                                    VkMicromapBuildSizesInfoEXT* pSizeInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(memory, record_obj.location);
}

void Device::PostCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(memory, record_obj.location);
}

void Device::PreCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                                                                     const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                                                     VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping,
                                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                                                                      const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                                                      VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(descriptorSet, record_obj.location);
}

void Device::PostCallRecordGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(descriptorSet, record_obj.location);
}

void Device::PreCallRecordCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                  uint32_t copyCount, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                   uint32_t copyCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                         uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                         VkImageLayout dstImageLayout,
                                                         const VkImageSubresourceLayers* pImageSubresources,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                          uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                          VkImageLayout dstImageLayout,
                                                          const VkImageSubresourceLayers* pImageSubresources,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                const VkDecompressMemoryRegionNV* pDecompressMemoryRegions,
                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                 const VkDecompressMemoryRegionNV* pDecompressMemoryRegions,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress,
                                                             VkDeviceAddress indirectCommandsCountAddress, uint32_t stride,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
                                                              VkDeviceAddress indirectCommandsAddress,
                                                              VkDeviceAddress indirectCommandsCountAddress, uint32_t stride,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                                  VkMemoryRequirements2* pMemoryRequirements,
                                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                            VkPipeline pipeline, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                             VkPipeline pipeline, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                              const VkSampleMask* pSampleMask, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                               const VkSampleMask* pSampleMask, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                    uint32_t attachmentCount, const VkBool32* pColorBlendEnables,
                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                     uint32_t attachmentCount, const VkBool32* pColorBlendEnables,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                      uint32_t attachmentCount, const VkColorBlendEquationEXT* pColorBlendEquations,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                       uint32_t attachmentCount,
                                                       const VkColorBlendEquationEXT* pColorBlendEquations,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                  const VkColorComponentFlags* pColorWriteMasks, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                   uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin,
                                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                       const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                                 VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                                  VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                    float extraPrimitiveOverestimationSize,
                                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                     float extraPrimitiveOverestimationSize,
                                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                      uint32_t attachmentCount, const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                       uint32_t attachmentCount, const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode,
                                                       const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                         VkLineRasterizationModeEXT lineRasterizationMode,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                          VkLineRasterizationModeEXT lineRasterizationMode,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                  const VkViewportSwizzleNV* pViewportSwizzles, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                   const VkViewportSwizzleNV* pViewportSwizzles, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                         VkCoverageModulationModeNV coverageModulationMode,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                          VkCoverageModulationModeNV coverageModulationMode,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                VkBool32 coverageModulationTableEnable,
                                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                 VkBool32 coverageModulationTableEnable,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                          const float* pCoverageModulationTable, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                           const float* pCoverageModulationTable, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                   VkBool32 representativeFragmentTestEnable,
                                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                    VkBool32 representativeFragmentTestEnable,
                                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                        VkCoverageReductionModeNV coverageReductionMode,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                         VkCoverageReductionModeNV coverageReductionMode,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                       VkShaderModuleIdentifierEXT* pIdentifier, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(shaderModule, record_obj.location);
}

void Device::PostCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                        VkShaderModuleIdentifierEXT* pIdentifier, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(shaderModule, record_obj.location);
}

void Device::PreCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                                 VkShaderModuleIdentifierEXT* pIdentifier,
                                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                                  VkShaderModuleIdentifierEXT* pIdentifier,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSession);
    }
}

void Device::PreCallRecordDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(session, record_obj.location);
}

void Device::PostCallRecordDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                       const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(session, record_obj.location);
}

void Device::PreCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                        VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                        VkImageLayout layout, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(session, record_obj.location);
    StartReadObject(view, record_obj.location);
}

void Device::PostCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                         VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                         VkImageLayout layout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(session, record_obj.location);
    FinishReadObject(view, record_obj.location);
}

void Device::PreCallRecordCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                  const VkOpticalFlowExecuteInfoNV* pExecuteInfo, const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
    StartReadObject(session, record_obj.location);
}

void Device::PostCallRecordCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                   const VkOpticalFlowExecuteInfoNV* pExecuteInfo, const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
    FinishReadObject(session, record_obj.location);
}

void Device::PreCallRecordAntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordAntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                           const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                            const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (pShaders) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pShaders[index]) continue;
            CreateObject(pShaders[index]);
        }
    }
}

void Device::PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(shader, record_obj.location);
    // Host access to shader must be externally synchronized
}

void Device::PostCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(shader, record_obj.location);
    DestroyObject(shader);
    // Host access to shader must be externally synchronized
}

void Device::PreCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(shader, record_obj.location);
}

void Device::PostCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(shader, record_obj.location);
}

void Device::PreCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                            const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pShaders) {
        for (uint32_t index = 0; index < stageCount; index++) {
            StartReadObject(pShaders[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                             const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pShaders) {
        for (uint32_t index = 0; index < stageCount; index++) {
            FinishReadObject(pShaders[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                                   const VkDepthClampRangeEXT* pDepthClampRange, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                                    const VkDepthClampRangeEXT* pDepthClampRange, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                           VkTilePropertiesQCOM* pProperties, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(framebuffer, record_obj.location);
}

void Device::PostCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                            VkTilePropertiesQCOM* pProperties, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(framebuffer, record_obj.location);
}

void Device::PreCallRecordGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                                VkTilePropertiesQCOM* pProperties, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                                 VkTilePropertiesQCOM* pProperties,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV* pInfo,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV* pInfo,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                              const VkConvertCooperativeVectorMatrixInfoNV* pInfos,
                                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                               const VkConvertCooperativeVectorMatrixInfoNV* pInfos,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                const VkLatencySleepModeInfoNV* pSleepModeInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void Device::PostCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                 const VkLatencySleepModeInfoNV* pSleepModeInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void Device::PreCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void Device::PostCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void Device::PreCallRecordSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                             const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void Device::PostCallRecordSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                              const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void Device::PreCallRecordGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain,
                                              VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void Device::PostCallRecordGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain,
                                               VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void Device::PreCallRecordQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo,
                                                 const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void Device::PostCallRecordQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo,
                                                  const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void Device::PreCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void Device::PreCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                       VkScreenBufferPropertiesQNX* pProperties, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                        VkScreenBufferPropertiesQNX* pProperties, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_SCREEN_QNX
void Device::PreCallRecordGetClusterAccelerationStructureBuildSizesNV(VkDevice device,
                                                                      const VkClusterAccelerationStructureInputInfoNV* pInfo,
                                                                      VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetClusterAccelerationStructureBuildSizesNV(VkDevice device,
                                                                       const VkClusterAccelerationStructureInputInfoNV* pInfo,
                                                                       VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos,
    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo,
    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device,
                                                                    const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device,
                                                                     const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
                                                                     VkMemoryRequirements2* pMemoryRequirements,
                                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
                                                            const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                            VkCommandBuffer stateCommandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartWriteObject(stateCommandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // Host access to stateCommandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
                                                             const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                             VkCommandBuffer stateCommandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishWriteObject(stateCommandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // Host access to stateCommandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                         const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                          const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCreateIndirectCommandsLayoutEXT(VkDevice device, const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout,
                                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateIndirectCommandsLayoutEXT(VkDevice device,
                                                           const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pIndirectCommandsLayout);
    }
}

void Device::PreCallRecordDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(indirectCommandsLayout, record_obj.location);
    // Host access to indirectCommandsLayout must be externally synchronized
}

void Device::PostCallRecordDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(indirectCommandsLayout, record_obj.location);
    DestroyObject(indirectCommandsLayout);
    // Host access to indirectCommandsLayout must be externally synchronized
}

void Device::PreCallRecordCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkIndirectExecutionSetEXT* pIndirectExecutionSet,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkIndirectExecutionSetEXT* pIndirectExecutionSet,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pIndirectExecutionSet);
    }
}

void Device::PreCallRecordDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(indirectExecutionSet, record_obj.location);
    // Host access to indirectExecutionSet must be externally synchronized
}

void Device::PostCallRecordDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(indirectExecutionSet, record_obj.location);
    DestroyObject(indirectExecutionSet);
    // Host access to indirectExecutionSet must be externally synchronized
}

void Device::PreCallRecordUpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                                uint32_t executionSetWriteCount,
                                                                const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites,
                                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(indirectExecutionSet, record_obj.location);
    // Host access to indirectExecutionSet must be externally synchronized
}

void Device::PostCallRecordUpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                                 uint32_t executionSetWriteCount,
                                                                 const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(indirectExecutionSet, record_obj.location);
    // Host access to indirectExecutionSet must be externally synchronized
}

void Device::PreCallRecordUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                              uint32_t executionSetWriteCount,
                                                              const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(indirectExecutionSet, record_obj.location);
    // Host access to indirectExecutionSet must be externally synchronized
}

void Device::PostCallRecordUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                               uint32_t executionSetWriteCount,
                                                               const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(indirectExecutionSet, record_obj.location);
    // Host access to indirectExecutionSet must be externally synchronized
}

#ifdef VK_USE_PLATFORM_METAL_EXT
void Device::PreCallRecordGetMemoryMetalHandleEXT(VkDevice device, const VkMemoryGetMetalHandleInfoEXT* pGetMetalHandleInfo,
                                                  void** pHandle, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMemoryMetalHandleEXT(VkDevice device, const VkMemoryGetMetalHandleInfoEXT* pGetMetalHandleInfo,
                                                   void** pHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetMemoryMetalHandlePropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                            const void* pHandle,
                                                            VkMemoryMetalHandlePropertiesEXT* pMemoryMetalHandleProperties,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetMemoryMetalHandlePropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                             const void* pHandle,
                                                             VkMemoryMetalHandlePropertiesEXT* pMemoryMetalHandleProperties,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_METAL_EXT
void Device::PreCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkAccelerationStructureKHR* pAccelerationStructure,
                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkAccelerationStructureKHR* pAccelerationStructure,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pAccelerationStructure);
    }
}

void Device::PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(accelerationStructure, record_obj.location);
    // Host access to accelerationStructure must be externally synchronized
}

void Device::PostCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(accelerationStructure, record_obj.location);
    DestroyObject(accelerationStructure);
    // Host access to accelerationStructure must be externally synchronized
}

void Device::PreCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                    const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                    const uint32_t* pIndirectStrides,
                                                                    const uint32_t* const* ppMaxPrimitiveCounts,
                                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                     const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                     const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                     const uint32_t* pIndirectStrides,
                                                                     const uint32_t* const* ppMaxPrimitiveCounts,
                                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         uint32_t infoCount,
                                                         const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                         const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void Device::PostCallRecordBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                          uint32_t infoCount,
                                                          const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                          const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void Device::PreCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                       const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void Device::PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void Device::PreCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                               const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void Device::PostCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void Device::PreCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                               const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void Device::PostCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void Device::PreCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                   const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                   VkQueryType queryType, size_t dataSize, void* pData,
                                                                   size_t stride, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
}

void Device::PostCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                    const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                    VkQueryType queryType, size_t dataSize, void* pData,
                                                                    size_t stride, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
}

void Device::PreCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                           const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                  const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                  const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                   const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                      uint32_t accelerationStructureCount,
                                                                      const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                      VkQueryType queryType, VkQueryPool queryPool,
                                                                      uint32_t firstQuery, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                       uint32_t accelerationStructureCount,
                                                                       const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                       VkQueryType queryType, VkQueryPool queryPool,
                                                                       uint32_t firstQuery, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                                         const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                                         VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                                          const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                                          VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                                const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                                                const uint32_t* pMaxPrimitiveCounts,
                                                                VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void Device::PostCallRecordGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                                 const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                                                 const uint32_t* pMaxPrimitiveCounts,
                                                                 VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void Device::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                          const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                          const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                          const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                          const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                          uint32_t height, uint32_t depth, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                           const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                           uint32_t height, uint32_t depth, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                          uint32_t groupCount, size_t dataSize, void* pData,
                                                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
}

void Device::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                           uint32_t firstGroup, uint32_t groupCount,
                                                                           size_t dataSize, void* pData,
                                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
}

void Device::PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                  const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                  const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                  const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                  const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                  VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                   const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                   VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                               VkShaderGroupShaderKHR groupShader, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
}

void Device::PostCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                VkShaderGroupShaderKHR groupShader,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
}

void Device::PreCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                              uint32_t groupCountZ, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    StartReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void Device::PostCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    FinishReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

}  // namespace threadsafety

// NOLINTEND
