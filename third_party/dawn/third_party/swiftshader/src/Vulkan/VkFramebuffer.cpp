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

#include "VkFramebuffer.hpp"

#include "VkImageView.hpp"
#include "VkRenderPass.hpp"
#include "VkStringify.hpp"

#include <memory.h>
#include <algorithm>

namespace vk {

Framebuffer::Framebuffer(const VkFramebufferCreateInfo *pCreateInfo, void *mem)
    : attachments(reinterpret_cast<ImageView **>(mem))
    , extent{ pCreateInfo->width, pCreateInfo->height }
{
	const VkBaseInStructure *curInfo = reinterpret_cast<const VkBaseInStructure *>(pCreateInfo->pNext);
	const VkFramebufferAttachmentsCreateInfo *attachmentsCreateInfo = nullptr;
	while(curInfo)
	{
		switch(curInfo->sType)
		{
		case VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO:
			attachmentsCreateInfo = reinterpret_cast<const VkFramebufferAttachmentsCreateInfo *>(curInfo);
			break;
		case VK_STRUCTURE_TYPE_MAX_ENUM:
			// dEQP tests that this value is ignored.
			break;
		default:
			UNSUPPORTED("pFramebufferCreateInfo->pNext->sType = %s", vk::Stringify(curInfo->sType).c_str());
			break;
		}
		curInfo = curInfo->pNext;
	}

	if(pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)
	{
		// If flags includes VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT, the pNext chain **must**
		// include a VkFramebufferAttachmentsCreateInfo.
		ASSERT(attachmentsCreateInfo != nullptr);
		attachmentCount = attachmentsCreateInfo->attachmentImageInfoCount;
		for(uint32_t i = 0; i < attachmentCount; i++)
		{
			attachments[i] = nullptr;
		}
	}
	else
	{
		attachmentCount = pCreateInfo->attachmentCount;
		for(uint32_t i = 0; i < attachmentCount; i++)
		{
			attachments[i] = vk::Cast(pCreateInfo->pAttachments[i]);
		}
	}
}

void Framebuffer::destroy(const VkAllocationCallbacks *pAllocator)
{
	vk::freeHostMemory(attachments, pAllocator);
}

void Framebuffer::executeLoadOp(const RenderPass *renderPass, uint32_t clearValueCount, const VkClearValue *pClearValues, const VkRect2D &renderArea)
{
	// This gets called at the start of a renderpass. Logically the `loadOp` gets executed at the
	// subpass where an attachment is first used, but since we don't discard contents between subpasses,
	// we can execute it sooner. Only clear operations have an effect.

	ASSERT(attachmentCount == renderPass->getAttachmentCount());

	const uint32_t count = std::min(clearValueCount, attachmentCount);
	for(uint32_t i = 0; i < count; i++)
	{
		const VkAttachmentDescription attachment = renderPass->getAttachment(i);
		VkImageAspectFlags clearMask = 0;

		switch(attachment.loadOp)
		{
		case VK_ATTACHMENT_LOAD_OP_CLEAR:
			clearMask |= VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
		case VK_ATTACHMENT_LOAD_OP_LOAD:
		case VK_ATTACHMENT_LOAD_OP_DONT_CARE:
		case VK_ATTACHMENT_LOAD_OP_NONE_EXT:
			// Don't clear the attachment's color or depth aspect.
			break;
		default:
			UNSUPPORTED("attachment.loadOp %d", attachment.loadOp);
		}

		switch(attachment.stencilLoadOp)
		{
		case VK_ATTACHMENT_LOAD_OP_CLEAR:
			clearMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		case VK_ATTACHMENT_LOAD_OP_LOAD:
		case VK_ATTACHMENT_LOAD_OP_DONT_CARE:
		case VK_ATTACHMENT_LOAD_OP_NONE_EXT:
			// Don't clear the attachment's stencil aspect.
			break;
		default:
			UNSUPPORTED("attachment.stencilLoadOp %d", attachment.stencilLoadOp);
		}

		// Image::clear() demands that we only specify existing aspects.
		clearMask &= Format(attachment.format).getAspects();

		if(!clearMask || !renderPass->isAttachmentUsed(i))
		{
			continue;
		}

		uint32_t viewMask = renderPass->isMultiView() ? renderPass->getAttachmentViewMask(i) : 0;
		attachments[i]->clear(pClearValues[i], clearMask, renderArea, viewMask);
	}
}

void Framebuffer::clearAttachment(const RenderPass *renderPass, uint32_t subpassIndex, const VkClearAttachment &attachment, const VkClearRect &rect)
{
	VkSubpassDescription subpass = renderPass->getSubpass(subpassIndex);
	uint32_t viewMask = renderPass->getViewMask(subpassIndex);

	if(attachment.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT)
	{
		ASSERT(attachment.colorAttachment < subpass.colorAttachmentCount);
		uint32_t attachmentIndex = subpass.pColorAttachments[attachment.colorAttachment].attachment;

		if(attachmentIndex != VK_ATTACHMENT_UNUSED)
		{
			ASSERT(attachmentIndex < attachmentCount);
			ImageView *imageView = attachments[attachmentIndex];

			imageView->clear(attachment.clearValue, attachment.aspectMask, rect, viewMask);
		}
	}
	else if(attachment.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
	{
		uint32_t attachmentIndex = subpass.pDepthStencilAttachment->attachment;

		if(attachmentIndex != VK_ATTACHMENT_UNUSED)
		{
			ASSERT(attachmentIndex < attachmentCount);
			ImageView *imageView = attachments[attachmentIndex];

			imageView->clear(attachment.clearValue, attachment.aspectMask, rect, viewMask);
		}
	}
	else
		UNSUPPORTED("attachment.aspectMask %X", attachment.aspectMask);
}

void Framebuffer::setAttachment(ImageView *imageView, uint32_t index)
{
	ASSERT(index < attachmentCount);
	attachments[index] = imageView;
}

ImageView *Framebuffer::getAttachment(uint32_t index) const
{
	return attachments[index];
}

void Framebuffer::resolve(const RenderPass *renderPass, uint32_t subpassIndex)
{
	const auto &subpass = renderPass->getSubpass(subpassIndex);
	uint32_t viewMask = renderPass->getViewMask(subpassIndex);

	if(subpass.pResolveAttachments)
	{
		for(uint32_t i = 0; i < subpass.colorAttachmentCount; i++)
		{
			uint32_t resolveAttachment = subpass.pResolveAttachments[i].attachment;
			if(resolveAttachment != VK_ATTACHMENT_UNUSED)
			{
				ImageView *imageView = attachments[subpass.pColorAttachments[i].attachment];
				imageView->resolve(attachments[resolveAttachment], viewMask);
			}
		}
	}

	if(renderPass->hasDepthStencilResolve() && subpass.pDepthStencilAttachment != nullptr)
	{
		VkSubpassDescriptionDepthStencilResolve dsResolve = renderPass->getSubpassDepthStencilResolve(subpassIndex);
		uint32_t depthStencilAttachment = subpass.pDepthStencilAttachment->attachment;
		if((depthStencilAttachment != VK_ATTACHMENT_UNUSED) && (dsResolve.pDepthStencilResolveAttachment != nullptr))
		{
			ImageView *imageView = attachments[depthStencilAttachment];
			imageView->resolveDepthStencil(attachments[dsResolve.pDepthStencilResolveAttachment->attachment],
			                               dsResolve.depthResolveMode, dsResolve.stencilResolveMode);
		}
	}
}

size_t Framebuffer::ComputeRequiredAllocationSize(const VkFramebufferCreateInfo *pCreateInfo)
{
	const VkBaseInStructure *curInfo = reinterpret_cast<const VkBaseInStructure *>(pCreateInfo->pNext);
	const VkFramebufferAttachmentsCreateInfo *attachmentsInfo = nullptr;
	while(curInfo)
	{
		switch(curInfo->sType)
		{
		case VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO:
			attachmentsInfo = reinterpret_cast<const VkFramebufferAttachmentsCreateInfo *>(curInfo);
			break;
		case VK_STRUCTURE_TYPE_MAX_ENUM:
			// dEQP tests that this value is ignored.
			break;
		default:
			UNSUPPORTED("pFramebufferCreateInfo->pNext->sType = %s", vk::Stringify(curInfo->sType).c_str());
			break;
		}

		curInfo = curInfo->pNext;
	}

	if(pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)
	{
		ASSERT(attachmentsInfo != nullptr);
		return attachmentsInfo->attachmentImageInfoCount * sizeof(void *);
	}
	else
	{
		return pCreateInfo->attachmentCount * sizeof(void *);
	}
}

}  // namespace vk
