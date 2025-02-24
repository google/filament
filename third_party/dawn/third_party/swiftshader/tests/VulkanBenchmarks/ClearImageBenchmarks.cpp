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

#include "Util.hpp"
#include "VulkanTester.hpp"

#include "benchmark/benchmark.h"

#include <cassert>

class ClearImageBenchmark
{
public:
	void initialize(vk::Format clearFormat, vk::ImageAspectFlagBits clearAspect)
	{
		tester.initialize();
		auto &device = tester.getDevice();
		auto &physicalDevice = tester.getPhysicalDevice();

		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = clearFormat;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst;
		imageInfo.samples = vk::SampleCountFlagBits::e4;
		imageInfo.extent = vk::Extent3D(1024, 1024, 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;

		image = device.createImage(imageInfo);

		vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(image);

		vk::MemoryAllocateInfo allocateInfo;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = Util::getMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits);

		memory = device.allocateMemory(allocateInfo);

		device.bindImageMemory(image, memory, 0);

		vk::CommandPoolCreateInfo commandPoolCreateInfo;
		commandPoolCreateInfo.queueFamilyIndex = tester.getQueueFamilyIndex();

		commandPool = device.createCommandPool(commandPoolCreateInfo);

		vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.commandPool = commandPool;
		commandBufferAllocateInfo.commandBufferCount = 1;

		commandBuffer = device.allocateCommandBuffers(commandBufferAllocateInfo)[0];

		vk::CommandBufferBeginInfo commandBufferBeginInfo;
		commandBufferBeginInfo.flags = {};

		commandBuffer.begin(commandBufferBeginInfo);

		vk::ImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange.aspectMask = clearAspect;
		imageMemoryBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		imageMemoryBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eGeneral;
		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eTopOfPipe,
		                              vk::DependencyFlagBits::eDeviceGroup, {}, {}, imageMemoryBarrier);

		vk::ImageSubresourceRange range;
		range.aspectMask = clearAspect;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		if(clearAspect == vk::ImageAspectFlagBits::eColor)
		{
			vk::ClearColorValue clearColorValue;
			clearColorValue.float32[0] = 0.0f;
			clearColorValue.float32[1] = 1.0f;
			clearColorValue.float32[2] = 0.0f;
			clearColorValue.float32[3] = 1.0f;

			commandBuffer.clearColorImage(image, vk::ImageLayout::eGeneral, &clearColorValue, 1, &range);
		}
		else if(clearAspect == vk::ImageAspectFlagBits::eDepth)
		{
			vk::ClearDepthStencilValue clearDepthStencilValue;
			clearDepthStencilValue.depth = 1.0f;
			clearDepthStencilValue.stencil = 0xFF;

			commandBuffer.clearDepthStencilImage(image, vk::ImageLayout::eGeneral, &clearDepthStencilValue, 1, &range);
		}
		else
			assert(false);

		commandBuffer.end();
	}

	~ClearImageBenchmark()
	{
		auto &device = tester.getDevice();
		device.freeCommandBuffers(commandPool, 1, &commandBuffer);
		device.destroyCommandPool(commandPool, nullptr);
		device.freeMemory(memory, nullptr);
		device.destroyImage(image, nullptr);
	}

	void clear()
	{
		auto &queue = tester.getQueue();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		queue.submit(1, &submitInfo, nullptr);
		queue.waitIdle();
	}

private:
	VulkanTester tester;
	vk::Image image;                  // Owning handle
	vk::DeviceMemory memory;          // Owning handle
	vk::CommandPool commandPool;      // Owning handle
	vk::CommandBuffer commandBuffer;  // Owning handle
};

static void ClearImage(benchmark::State &state, vk::Format clearFormat, vk::ImageAspectFlagBits clearAspect)
{
	ClearImageBenchmark benchmark;
	benchmark.initialize(clearFormat, clearAspect);

	// Execute once to have the Reactor routine generated.
	benchmark.clear();

	for(auto _ : state)
	{
		benchmark.clear();
	}
}

BENCHMARK_CAPTURE(ClearImage, VK_FORMAT_R8G8B8A8_UNORM, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(ClearImage, VK_FORMAT_R32_SFLOAT, vk::Format::eR32Sfloat, vk::ImageAspectFlagBits::eColor)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(ClearImage, VK_FORMAT_D32_SFLOAT, vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
