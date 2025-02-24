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

#ifndef BENCHMARKS_SWAPCHAIN_HPP_
#define BENCHMARKS_SWAPCHAIN_HPP_

#include "VulkanHeaders.hpp"
#include <vector>

class Window;

class Swapchain
{
public:
	Swapchain(vk::PhysicalDevice physicalDevice, vk::Device device, Window &window);
	~Swapchain();

	void acquireNextImage(vk::Semaphore presentCompleteSemaphore, uint32_t &imageIndex);
	void queuePresent(vk::Queue queue, uint32_t imageIndex, vk::Semaphore waitSemaphore);

	size_t imageCount() const
	{
		return images.size();
	}

	vk::ImageView getImageView(size_t i) const
	{
		return imageViews[i];
	}

	vk::Extent2D getExtent() const
	{
		return extent;
	}

	const vk::Format colorFormat = vk::Format::eB8G8R8A8Unorm;

private:
	const vk::Device device;

	vk::SwapchainKHR swapchain;  // Owning handle
	vk::Extent2D extent;

	std::vector<vk::Image> images;          // Weak pointers. Presentable images owned by swapchain object.
	std::vector<vk::ImageView> imageViews;  // Owning handles
};

#endif  // BENCHMARKS_SWAPCHAIN_HPP_
