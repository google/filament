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

#include "Image.hpp"
#include "Util.hpp"

Image::Image(vk::Device device, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height, vk::Format format, vk::SampleCountFlagBits sampleCount /*= vk::SampleCountFlagBits::e1*/)
    : device(device)
{
	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = format;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.initialLayout = vk::ImageLayout::eGeneral;
	imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment;
	imageInfo.samples = sampleCount;
	imageInfo.extent = vk::Extent3D(width, height, 1);
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;

	image = device.createImage(imageInfo);

	vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo allocateInfo;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = Util::getMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits);

	imageMemory = device.allocateMemory(allocateInfo);

	device.bindImageMemory(image, imageMemory, 0);

	vk::ImageViewCreateInfo imageViewInfo;
	imageViewInfo.image = image;
	imageViewInfo.viewType = vk::ImageViewType::e2D;
	imageViewInfo.format = format;
	imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	imageView = device.createImageView(imageViewInfo);
}

Image::~Image()
{
	device.destroyImageView(imageView);
	device.freeMemory(imageMemory);
	device.destroyImage(image);
}
