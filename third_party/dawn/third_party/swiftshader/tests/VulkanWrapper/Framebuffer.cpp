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

#include "Framebuffer.hpp"

Framebuffer::Framebuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::ImageView attachment, vk::Format colorFormat, vk::RenderPass renderPass, vk::Extent2D extent, bool multisample)
    : device(device)
{
	std::vector<vk::ImageView> attachments(multisample ? 2 : 1);

	if(multisample)
	{
		multisampleImage.reset(new Image(device, physicalDevice, extent.width, extent.height, colorFormat, vk::SampleCountFlagBits::e4));

		// We'll be rendering to attachment location 0
		attachments[0] = multisampleImage->getImageView();
		attachments[1] = attachment;  // Resolve attachment
	}
	else
	{
		attachments[0] = attachment;
	}

	vk::FramebufferCreateInfo framebufferCreateInfo;

	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferCreateInfo.pAttachments = attachments.data();
	framebufferCreateInfo.width = extent.width;
	framebufferCreateInfo.height = extent.height;
	framebufferCreateInfo.layers = 1;

	framebuffer = device.createFramebuffer(framebufferCreateInfo);
}

Framebuffer::~Framebuffer()
{
	multisampleImage.reset();
	device.destroyFramebuffer(framebuffer);
}
