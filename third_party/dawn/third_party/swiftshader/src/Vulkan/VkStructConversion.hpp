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

#ifndef VK_STRUCT_CONVERSION_HPP_
#define VK_STRUCT_CONVERSION_HPP_

#include "VkMemory.hpp"
#include "VkStringify.hpp"

#include "System/Debug.hpp"

#include <cstring>
#include <vector>

namespace vk {

struct CopyBufferInfo : public VkCopyBufferInfo2
{
	CopyBufferInfo(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy *pRegions)
	    : VkCopyBufferInfo2{
		    VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
		    nullptr,
		    srcBuffer,
		    dstBuffer,
		    regionCount,
		    nullptr
	    }
	{
		regions.resize(regionCount);
		for(uint32_t i = 0; i < regionCount; i++)
		{
			regions[i] = {
				VK_STRUCTURE_TYPE_BUFFER_COPY_2,
				nullptr,
				pRegions[i].srcOffset,
				pRegions[i].dstOffset,
				pRegions[i].size
			};
		}

		this->pRegions = &regions.front();
	}

private:
	std::vector<VkBufferCopy2> regions;
};

struct CopyImageInfo : public VkCopyImageInfo2
{
	CopyImageInfo(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions)
	    : VkCopyImageInfo2{
		    VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
		    nullptr,
		    srcImage,
		    srcImageLayout,
		    dstImage,
		    dstImageLayout,
		    regionCount,
		    nullptr
	    }
	{
		regions.resize(regionCount);
		for(uint32_t i = 0; i < regionCount; i++)
		{
			regions[i] = {
				VK_STRUCTURE_TYPE_IMAGE_COPY_2,
				nullptr,
				pRegions[i].srcSubresource,
				pRegions[i].srcOffset,
				pRegions[i].dstSubresource,
				pRegions[i].dstOffset,
				pRegions[i].extent
			};
		}

		this->pRegions = &regions.front();
	}

private:
	std::vector<VkImageCopy2> regions;
};

struct BlitImageInfo : public VkBlitImageInfo2
{
	BlitImageInfo(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter)
	    : VkBlitImageInfo2{
		    VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		    nullptr,
		    srcImage,
		    srcImageLayout,
		    dstImage,
		    dstImageLayout,
		    regionCount,
		    nullptr,
		    filter
	    }
	{
		regions.resize(regionCount);
		for(uint32_t i = 0; i < regionCount; i++)
		{
			regions[i] = {
				VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
				nullptr,
				pRegions[i].srcSubresource,
				{ pRegions[i].srcOffsets[0], pRegions[i].srcOffsets[1] },
				pRegions[i].dstSubresource,
				{ pRegions[i].dstOffsets[0], pRegions[i].dstOffsets[1] }
			};
		}

		this->pRegions = &regions.front();
	}

private:
	std::vector<VkImageBlit2> regions;
};

struct CopyBufferToImageInfo : public VkCopyBufferToImageInfo2
{
	CopyBufferToImageInfo(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions)
	    : VkCopyBufferToImageInfo2{
		    VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
		    nullptr,
		    srcBuffer,
		    dstImage,
		    dstImageLayout,
		    regionCount,
		    nullptr
	    }
	{
		regions.resize(regionCount);
		for(uint32_t i = 0; i < regionCount; i++)
		{
			regions[i] = {
				VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
				nullptr,
				pRegions[i].bufferOffset,
				pRegions[i].bufferRowLength,
				pRegions[i].bufferImageHeight,
				pRegions[i].imageSubresource,
				pRegions[i].imageOffset,
				pRegions[i].imageExtent
			};
		}

		this->pRegions = &regions.front();
	}

private:
	std::vector<VkBufferImageCopy2> regions;
};

struct CopyImageToBufferInfo : public VkCopyImageToBufferInfo2
{
	CopyImageToBufferInfo(VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions)
	    : VkCopyImageToBufferInfo2{
		    VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2,
		    nullptr,
		    srcImage,
		    srcImageLayout,
		    dstBuffer,
		    regionCount,
		    nullptr
	    }
	{
		regions.resize(regionCount);
		for(uint32_t i = 0; i < regionCount; i++)
		{
			regions[i] = {
				VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
				nullptr,
				pRegions[i].bufferOffset,
				pRegions[i].bufferRowLength,
				pRegions[i].bufferImageHeight,
				pRegions[i].imageSubresource,
				pRegions[i].imageOffset,
				pRegions[i].imageExtent
			};
		}

		this->pRegions = &regions.front();
	}

private:
	std::vector<VkBufferImageCopy2> regions;
};

struct ResolveImageInfo : public VkResolveImageInfo2
{
	ResolveImageInfo(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions)
	    : VkResolveImageInfo2{
		    VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2,
		    nullptr,
		    srcImage,
		    srcImageLayout,
		    dstImage,
		    dstImageLayout,
		    regionCount,
		    nullptr
	    }
	{
		regions.resize(regionCount);
		for(uint32_t i = 0; i < regionCount; i++)
		{
			regions[i] = {
				VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2,
				nullptr,
				pRegions[i].srcSubresource,
				pRegions[i].srcOffset,
				pRegions[i].dstSubresource,
				pRegions[i].dstOffset,
				pRegions[i].extent
			};
		}

		this->pRegions = &regions.front();
	}

private:
	std::vector<VkImageResolve2> regions;
};

struct DependencyInfo : public VkDependencyInfo
{
	DependencyInfo(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
	               VkDependencyFlags dependencyFlags,
	               uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
	               uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
	               uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers)
	    : VkDependencyInfo{
		    VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		    nullptr,
		    dependencyFlags,
		    memoryBarrierCount,
		    nullptr,
		    bufferMemoryBarrierCount,
		    nullptr,
		    imageMemoryBarrierCount,
		    nullptr
	    }
	{
		if((memoryBarrierCount == 0) &&
		   (bufferMemoryBarrierCount == 0) &&
		   (imageMemoryBarrierCount == 0))
		{
			// Create a single memory barrier entry to store the source and destination stage masks
			memoryBarriers.resize(1);
			memoryBarriers[0] = {
				VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
				nullptr,
				srcStageMask,
				VK_ACCESS_2_NONE,
				dstStageMask,
				VK_ACCESS_2_NONE
			};
		}
		else
		{
			memoryBarriers.resize(memoryBarrierCount);
			for(uint32_t i = 0; i < memoryBarrierCount; i++)
			{
				memoryBarriers[i] = {
					VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
					pMemoryBarriers[i].pNext,
					srcStageMask,
					pMemoryBarriers[i].srcAccessMask,
					dstStageMask,
					pMemoryBarriers[i].dstAccessMask
				};
			}

			bufferMemoryBarriers.resize(bufferMemoryBarrierCount);
			for(uint32_t i = 0; i < bufferMemoryBarrierCount; i++)
			{
				bufferMemoryBarriers[i] = {
					VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
					pBufferMemoryBarriers[i].pNext,
					srcStageMask,
					pBufferMemoryBarriers[i].srcAccessMask,
					dstStageMask,
					pBufferMemoryBarriers[i].dstAccessMask,
					pBufferMemoryBarriers[i].srcQueueFamilyIndex,
					pBufferMemoryBarriers[i].dstQueueFamilyIndex,
					pBufferMemoryBarriers[i].buffer,
					pBufferMemoryBarriers[i].offset,
					pBufferMemoryBarriers[i].size
				};
			}

			imageMemoryBarriers.resize(imageMemoryBarrierCount);
			for(uint32_t i = 0; i < imageMemoryBarrierCount; i++)
			{
				imageMemoryBarriers[i] = {
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
					pImageMemoryBarriers[i].pNext,
					srcStageMask,
					pImageMemoryBarriers[i].srcAccessMask,
					dstStageMask,
					pImageMemoryBarriers[i].dstAccessMask,
					pImageMemoryBarriers[i].oldLayout,
					pImageMemoryBarriers[i].newLayout,
					pImageMemoryBarriers[i].srcQueueFamilyIndex,
					pImageMemoryBarriers[i].dstQueueFamilyIndex,
					pImageMemoryBarriers[i].image,
					pImageMemoryBarriers[i].subresourceRange
				};
			}
		}

		this->pMemoryBarriers = memoryBarriers.empty() ? nullptr : &memoryBarriers.front();
		this->pBufferMemoryBarriers = bufferMemoryBarriers.empty() ? nullptr : &bufferMemoryBarriers.front();
		this->pImageMemoryBarriers = imageMemoryBarriers.empty() ? nullptr : &imageMemoryBarriers.front();
	}

private:
	std::vector<VkMemoryBarrier2> memoryBarriers;
	std::vector<VkBufferMemoryBarrier2> bufferMemoryBarriers;
	std::vector<VkImageMemoryBarrier2> imageMemoryBarriers;
};

struct ImageSubresource : VkImageSubresource
{
	ImageSubresource(const VkImageSubresourceLayers &subresourceLayers)
	    : VkImageSubresource{
		    subresourceLayers.aspectMask,
		    subresourceLayers.mipLevel,
		    subresourceLayers.baseArrayLayer
	    }
	{}
};

struct ImageSubresourceRange : VkImageSubresourceRange
{
	ImageSubresourceRange(const VkImageSubresourceLayers &subresourceLayers)
	    : VkImageSubresourceRange{
		    subresourceLayers.aspectMask,
		    subresourceLayers.mipLevel,
		    1,
		    subresourceLayers.baseArrayLayer,
		    subresourceLayers.layerCount
	    }
	{}
};

struct Extent2D : VkExtent2D
{
	Extent2D(const VkExtent3D &extent3D)
	    : VkExtent2D{ extent3D.width, extent3D.height }
	{}
};

struct SubmitInfo
{
	static SubmitInfo *Allocate(uint32_t submitCount, const VkSubmitInfo *pSubmits)
	{
		size_t submitSize = sizeof(SubmitInfo) * submitCount;
		size_t totalSize = Align8(submitSize);
		for(uint32_t i = 0; i < submitCount; i++)
		{
			totalSize += Align8(pSubmits[i].waitSemaphoreCount * sizeof(VkSemaphore));
			totalSize += Align8(pSubmits[i].waitSemaphoreCount * sizeof(VkPipelineStageFlags));
			totalSize += Align8(pSubmits[i].signalSemaphoreCount * sizeof(VkSemaphore));
			totalSize += Align8(pSubmits[i].commandBufferCount * sizeof(VkCommandBuffer));

			for(const auto *extension = reinterpret_cast<const VkBaseInStructure *>(pSubmits[i].pNext);
			    extension != nullptr; extension = reinterpret_cast<const VkBaseInStructure *>(extension->pNext))
			{
				switch(extension->sType)
				{
				case VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO:
					{
						const auto *tlsSubmitInfo = reinterpret_cast<const VkTimelineSemaphoreSubmitInfo *>(extension);
						totalSize += Align8(tlsSubmitInfo->waitSemaphoreValueCount * sizeof(uint64_t));
						totalSize += Align8(tlsSubmitInfo->signalSemaphoreValueCount * sizeof(uint64_t));
					}
					break;
				case VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO:
					// SwiftShader doesn't use device group submit info because it only supports a single physical device.
					// However, this extension is core in Vulkan 1.1, so we must treat it as a valid structure type.
					break;
				case VK_STRUCTURE_TYPE_MAX_ENUM:
					// dEQP tests that this value is ignored.
					break;
				default:
					UNSUPPORTED("submitInfo[%d]->pNext sType: %s", i, vk::Stringify(extension->sType).c_str());
					break;
				}
			}
		}

		uint8_t *buffer = static_cast<uint8_t *>(
		    vk::allocateHostMemory(totalSize, vk::HOST_MEMORY_ALLOCATION_ALIGNMENT, vk::NULL_ALLOCATION_CALLBACKS, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT));
		uint8_t *mem = buffer;

		auto submits = new(mem) SubmitInfo[submitCount];
		mem += Align8(submitSize);

		for(uint32_t i = 0; i < submitCount; i++)
		{
			submits[i].commandBufferCount = pSubmits[i].commandBufferCount;
			submits[i].signalSemaphoreCount = pSubmits[i].signalSemaphoreCount;
			submits[i].waitSemaphoreCount = pSubmits[i].waitSemaphoreCount;

			submits[i].pWaitSemaphores = nullptr;
			submits[i].pWaitDstStageMask = nullptr;
			submits[i].pSignalSemaphores = nullptr;
			submits[i].pCommandBuffers = nullptr;

			if(pSubmits[i].waitSemaphoreCount > 0)
			{
				size_t size = pSubmits[i].waitSemaphoreCount * sizeof(VkSemaphore);
				submits[i].pWaitSemaphores = reinterpret_cast<VkSemaphore *>(mem);
				memcpy(mem, pSubmits[i].pWaitSemaphores, size);
				mem += Align8(size);

				size = pSubmits[i].waitSemaphoreCount * sizeof(VkPipelineStageFlags);
				submits[i].pWaitDstStageMask = reinterpret_cast<VkPipelineStageFlags *>(mem);
				memcpy(mem, pSubmits[i].pWaitDstStageMask, size);
				mem += Align8(size);
			}

			if(pSubmits[i].signalSemaphoreCount > 0)
			{
				size_t size = pSubmits[i].signalSemaphoreCount * sizeof(VkSemaphore);
				submits[i].pSignalSemaphores = reinterpret_cast<VkSemaphore *>(mem);
				memcpy(mem, pSubmits[i].pSignalSemaphores, size);
				mem += Align8(size);
			}

			if(pSubmits[i].commandBufferCount > 0)
			{
				size_t size = pSubmits[i].commandBufferCount * sizeof(VkCommandBuffer);
				submits[i].pCommandBuffers = reinterpret_cast<VkCommandBuffer *>(mem);
				memcpy(mem, pSubmits[i].pCommandBuffers, size);
				mem += Align8(size);
			}

			submits[i].waitSemaphoreValueCount = 0;
			submits[i].pWaitSemaphoreValues = nullptr;
			submits[i].signalSemaphoreValueCount = 0;
			submits[i].pSignalSemaphoreValues = nullptr;

			for(const auto *extension = reinterpret_cast<const VkBaseInStructure *>(pSubmits[i].pNext);
			    extension != nullptr; extension = reinterpret_cast<const VkBaseInStructure *>(extension->pNext))
			{
				switch(extension->sType)
				{
				case VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO:
					{
						const VkTimelineSemaphoreSubmitInfo *tlsSubmitInfo = reinterpret_cast<const VkTimelineSemaphoreSubmitInfo *>(extension);

						if(tlsSubmitInfo->waitSemaphoreValueCount > 0)
						{
							submits[i].waitSemaphoreValueCount = tlsSubmitInfo->waitSemaphoreValueCount;
							size_t size = tlsSubmitInfo->waitSemaphoreValueCount * sizeof(uint64_t);
							submits[i].pWaitSemaphoreValues = reinterpret_cast<uint64_t *>(mem);
							memcpy(mem, tlsSubmitInfo->pWaitSemaphoreValues, size);
							mem += Align8(size);
						}

						if(tlsSubmitInfo->signalSemaphoreValueCount > 0)
						{
							submits[i].signalSemaphoreValueCount = tlsSubmitInfo->signalSemaphoreValueCount;
							size_t size = tlsSubmitInfo->signalSemaphoreValueCount * sizeof(uint64_t);
							submits[i].pSignalSemaphoreValues = reinterpret_cast<uint64_t *>(mem);
							memcpy(mem, tlsSubmitInfo->pSignalSemaphoreValues, size);
							mem += Align8(size);
						}
					}
					break;
				case VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO:
					// SwiftShader doesn't use device group submit info because it only supports a single physical device.
					// However, this extension is core in Vulkan 1.1, so we must treat it as a valid structure type.
					break;
				case VK_STRUCTURE_TYPE_MAX_ENUM:
					// dEQP tests that this value is ignored.
					break;
				default:
					UNSUPPORTED("submitInfo[%d]->pNext sType: %s", i, vk::Stringify(extension->sType).c_str());
					break;
				}
			}
		}

		ASSERT(static_cast<size_t>(mem - buffer) == totalSize);
		return submits;
	}

	static SubmitInfo *Allocate(uint32_t submitCount, const VkSubmitInfo2 *pSubmits)
	{
		size_t submitSize = sizeof(SubmitInfo) * submitCount;
		size_t totalSize = Align8(submitSize);
		for(uint32_t i = 0; i < submitCount; i++)
		{
			totalSize += Align8(pSubmits[i].waitSemaphoreInfoCount * sizeof(VkSemaphore));
			totalSize += Align8(pSubmits[i].waitSemaphoreInfoCount * sizeof(VkPipelineStageFlags));
			totalSize += Align8(pSubmits[i].waitSemaphoreInfoCount * sizeof(uint64_t));
			totalSize += Align8(pSubmits[i].signalSemaphoreInfoCount * sizeof(VkSemaphore));
			totalSize += Align8(pSubmits[i].signalSemaphoreInfoCount * sizeof(uint64_t));
			totalSize += Align8(pSubmits[i].commandBufferInfoCount * sizeof(VkCommandBuffer));

			for(const auto *extension = reinterpret_cast<const VkBaseInStructure *>(pSubmits[i].pNext);
			    extension != nullptr; extension = reinterpret_cast<const VkBaseInStructure *>(extension->pNext))
			{
				switch(extension->sType)
				{
				case VK_STRUCTURE_TYPE_MAX_ENUM:
					// dEQP tests that this value is ignored.
					break;
				case VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR:           // VK_KHR_performance_query
				case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR:  // VK_KHR_win32_keyed_mutex
				case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV:   // VK_NV_win32_keyed_mutex
				default:
					UNSUPPORTED("submitInfo[%d]->pNext sType: %s", i, vk::Stringify(extension->sType).c_str());
					break;
				}
			}
		}

		uint8_t *buffer = static_cast<uint8_t *>(
		    vk::allocateHostMemory(totalSize, vk::HOST_MEMORY_ALLOCATION_ALIGNMENT, vk::NULL_ALLOCATION_CALLBACKS, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT));
		uint8_t *mem = buffer;

		auto submits = new(mem) SubmitInfo[submitCount];
		mem += Align8(submitSize);

		for(uint32_t i = 0; i < submitCount; i++)
		{
			submits[i].commandBufferCount = pSubmits[i].commandBufferInfoCount;
			submits[i].signalSemaphoreCount = pSubmits[i].signalSemaphoreInfoCount;
			submits[i].waitSemaphoreCount = pSubmits[i].waitSemaphoreInfoCount;

			submits[i].signalSemaphoreValueCount = pSubmits[i].signalSemaphoreInfoCount;
			submits[i].waitSemaphoreValueCount = pSubmits[i].waitSemaphoreInfoCount;

			submits[i].pWaitSemaphores = nullptr;
			submits[i].pWaitDstStageMask = nullptr;
			submits[i].pSignalSemaphores = nullptr;
			submits[i].pCommandBuffers = nullptr;
			submits[i].pWaitSemaphoreValues = nullptr;
			submits[i].pSignalSemaphoreValues = nullptr;

			if(submits[i].waitSemaphoreCount > 0)
			{
				size_t size = submits[i].waitSemaphoreCount * sizeof(VkSemaphore);
				submits[i].pWaitSemaphores = reinterpret_cast<VkSemaphore *>(mem);
				mem += Align8(size);

				size = submits[i].waitSemaphoreCount * sizeof(VkPipelineStageFlags);
				submits[i].pWaitDstStageMask = reinterpret_cast<VkPipelineStageFlags *>(mem);
				mem += Align8(size);

				size = submits[i].waitSemaphoreCount * sizeof(uint64_t);
				submits[i].pWaitSemaphoreValues = reinterpret_cast<uint64_t *>(mem);
				mem += Align8(size);

				for(uint32_t j = 0; j < submits[i].waitSemaphoreCount; j++)
				{
					submits[i].pWaitSemaphores[j] = pSubmits[i].pWaitSemaphoreInfos[j].semaphore;
					submits[i].pWaitDstStageMask[j] = pSubmits[i].pWaitSemaphoreInfos[j].stageMask;
					submits[i].pWaitSemaphoreValues[j] = pSubmits[i].pWaitSemaphoreInfos[j].value;
				}
			}

			if(submits[i].signalSemaphoreCount > 0)
			{
				size_t size = submits[i].signalSemaphoreCount * sizeof(VkSemaphore);
				submits[i].pSignalSemaphores = reinterpret_cast<VkSemaphore *>(mem);
				mem += Align8(size);

				size = submits[i].signalSemaphoreCount * sizeof(uint64_t);
				submits[i].pSignalSemaphoreValues = reinterpret_cast<uint64_t *>(mem);
				mem += Align8(size);

				for(uint32_t j = 0; j < submits[i].signalSemaphoreCount; j++)
				{
					submits[i].pSignalSemaphores[j] = pSubmits[i].pSignalSemaphoreInfos[j].semaphore;
					submits[i].pSignalSemaphoreValues[j] = pSubmits[i].pSignalSemaphoreInfos[j].value;
				}
			}

			if(submits[i].commandBufferCount > 0)
			{
				size_t size = submits[i].commandBufferCount * sizeof(VkCommandBuffer);
				submits[i].pCommandBuffers = reinterpret_cast<VkCommandBuffer *>(mem);
				mem += Align8(size);

				for(uint32_t j = 0; j < submits[i].commandBufferCount; j++)
				{
					submits[i].pCommandBuffers[j] = pSubmits[i].pCommandBufferInfos[j].commandBuffer;
				}
			}
		}

		ASSERT(static_cast<size_t>(mem - buffer) == totalSize);
		return submits;
	}

	static void Release(SubmitInfo *submitInfo)
	{
		vk::freeHostMemory(submitInfo, NULL_ALLOCATION_CALLBACKS);
	}

	uint32_t waitSemaphoreCount;
	VkSemaphore *pWaitSemaphores;
	VkPipelineStageFlags *pWaitDstStageMask;
	uint32_t commandBufferCount;
	VkCommandBuffer *pCommandBuffers;
	uint32_t signalSemaphoreCount;
	VkSemaphore *pSignalSemaphores;
	uint32_t waitSemaphoreValueCount;
	uint64_t *pWaitSemaphoreValues;
	uint32_t signalSemaphoreValueCount;
	uint64_t *pSignalSemaphoreValues;

private:
	static size_t Align8(size_t size)
	{
		// Keep all arrays 8-byte aligned, so that an odd number of `VkPipelineStageFlags` does not break the
		// alignment of the other fields.
		constexpr size_t align = 8;
		return (size + align - 1) & ~(align - 1);
	}
};

}  // namespace vk

#endif  // VK_STRUCT_CONVERSION_HPP_
