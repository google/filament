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

#ifndef BENCHMARKS_IMAGE_HPP_
#define BENCHMARKS_IMAGE_HPP_

#include "VulkanHeaders.hpp"

class Image
{
public:
	Image(vk::Device device, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height, vk::Format format, vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1);
	~Image();

	vk::Image getImage()
	{
		return image;
	}

	vk::ImageView getImageView()
	{
		return imageView;
	}

private:
	const vk::Device device;

	vk::Image image;               // Owning handle
	vk::DeviceMemory imageMemory;  // Owning handle
	vk::ImageView imageView;       // Owning handle
};

#endif  // BENCHMARKS_IMAGE_HPP_
