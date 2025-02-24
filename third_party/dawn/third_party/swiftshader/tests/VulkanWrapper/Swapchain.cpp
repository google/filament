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

#include "Swapchain.hpp"
#include "Window.hpp"

Swapchain::Swapchain(vk::PhysicalDevice physicalDevice, vk::Device device, Window &window)
    : device(device)
{
	vk::SurfaceKHR surface = window.getSurface();

	// Create the swapchain
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	extent = surfaceCapabilities.currentExtent;

	vk::SwapchainCreateInfoKHR swapchainCreateInfo;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 2;  // double-buffered
	swapchainCreateInfo.imageFormat = colorFormat;
	swapchainCreateInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapchainCreateInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
	swapchainCreateInfo.presentMode = vk::PresentModeKHR::eFifo;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

	swapchain = device.createSwapchainKHR(swapchainCreateInfo);

	// Obtain the images and create views for them
	images = device.getSwapchainImagesKHR(swapchain);

	imageViews.resize(images.size());
	for(size_t i = 0; i < imageViews.size(); i++)
	{
		vk::ImageViewCreateInfo colorAttachmentView;
		colorAttachmentView.image = images[i];
		colorAttachmentView.viewType = vk::ImageViewType::e2D;
		colorAttachmentView.format = colorFormat;
		colorAttachmentView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		colorAttachmentView.subresourceRange.baseMipLevel = 0;
		colorAttachmentView.subresourceRange.levelCount = 1;
		colorAttachmentView.subresourceRange.baseArrayLayer = 0;
		colorAttachmentView.subresourceRange.layerCount = 1;

		imageViews[i] = device.createImageView(colorAttachmentView);
	}
}

Swapchain::~Swapchain()
{
	for(auto &imageView : imageViews)
	{
		device.destroyImageView(imageView, nullptr);
	}

	device.destroySwapchainKHR(swapchain, nullptr);
}

void Swapchain::acquireNextImage(vk::Semaphore presentCompleteSemaphore, uint32_t &imageIndex)
{
	auto result = device.acquireNextImageKHR(swapchain, UINT64_MAX, presentCompleteSemaphore, vk::Fence());
	imageIndex = result.value;
}

void Swapchain::queuePresent(vk::Queue queue, uint32_t imageIndex, vk::Semaphore waitSemaphore)
{
	vk::PresentInfoKHR presentInfo;
	presentInfo.pWaitSemaphores = &waitSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;

	queue.presentKHR(presentInfo);
}
