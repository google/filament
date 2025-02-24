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
#include "SPIRV/GlslangToSpv.h"
#include "glslang/Public/ResourceLimits.h"

#include <memory>

namespace Util {

uint32_t getMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t typeBits, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties deviceMemoryProperties = physicalDevice.getMemoryProperties();
	for(uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
	{
		if((typeBits & 1) == 1)
		{
			if((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		typeBits >>= 1;
	}

	assert(false);
	return -1;
}

vk::CommandBuffer beginSingleTimeCommands(vk::Device device, vk::CommandPool commandPool)
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	auto commandBuffer = device.allocateCommandBuffers(allocInfo);

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	commandBuffer[0].begin(beginInfo);

	return commandBuffer[0];
}

void endSingleTimeCommands(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::CommandBuffer commandBuffer)
{
	commandBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vk::Fence fence = {};  // TODO: pass in fence?
	queue.submit(1, &submitInfo, fence);
	queue.waitIdle();

	device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

void transitionImageLayout(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	vk::ImageMemoryBarrier barrier{};
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if(oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if(oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		assert(false && "unsupported layout transition!");
	}

	commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags{}, 0, nullptr, 0, nullptr, 1, &barrier);

	endSingleTimeCommands(device, commandPool, queue, commandBuffer);
}

void copyBufferToImage(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	vk::BufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = vk::Offset3D{ 0, 0, 0 };
	region.imageExtent = vk::Extent3D{ width, height, 1 };

	commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

	endSingleTimeCommands(device, commandPool, queue, commandBuffer);
}

std::vector<uint32_t> compileGLSLtoSPIRV(const char *glslSource, EShLanguage glslLanguage)
{
	// glslang requires one-time initialization.
	const struct GlslangProcessInitialiser
	{
		GlslangProcessInitialiser() { glslang::InitializeProcess(); }
		~GlslangProcessInitialiser() { glslang::FinalizeProcess(); }
	} glslangInitialiser;

	std::unique_ptr<glslang::TShader> glslangShader = std::make_unique<glslang::TShader>(glslLanguage);

	glslangShader->setStrings(&glslSource, 1);
	glslangShader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
	glslangShader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);

	const int defaultVersion = 100;
	EShMessages messages = static_cast<EShMessages>(EShMessages::EShMsgDefault | EShMessages::EShMsgSpvRules | EShMessages::EShMsgVulkanRules);
	bool parseResult = glslangShader->parse(GetDefaultResources(), defaultVersion, false, messages);

	if(!parseResult)
	{
		std::string debugLog = glslangShader->getInfoDebugLog();
		std::string infoLog = glslangShader->getInfoLog();
		assert(false && "Failed to parse shader");
	}

	glslang::TIntermediate *intermediateRepresentation = glslangShader->getIntermediate();
	assert(intermediateRepresentation);

	std::vector<uint32_t> spirv;
	glslang::SpvOptions options;
	glslang::GlslangToSpv(*intermediateRepresentation, spirv, &options);
	assert(spirv.size() != 0);

	return spirv;
}

}  // namespace Util
