/*!
\brief Function implementations for the Frambuffer Object class
\file PVRVk/FramebufferVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/FramebufferVk.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/RenderPassVk.h"
namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
Framebuffer_::~Framebuffer_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyFramebuffer(getDevice()->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
	_createInfo.clear();
}

Framebuffer_::Framebuffer_(make_shared_enabler, const DeviceWeakPtr& device, const FramebufferCreateInfo& createInfo) : PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils()
{
	// validate
	assert(createInfo.getRenderPass() && "Invalid RenderPass");
	// validate the dimension.
	if (createInfo.getDimensions().getWidth() == 0 || createInfo.getDimensions().getHeight() == 0)
	{ throw ErrorValidationFailedEXT("Framebuffer with and height must be valid size"); }
	_createInfo = createInfo;
	VkFramebufferCreateInfo framebufferCreateInfo = {};

	framebufferCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_FRAMEBUFFER_CREATE_INFO);
	framebufferCreateInfo.width = createInfo.getDimensions().getWidth();
	framebufferCreateInfo.height = createInfo.getDimensions().getHeight();
	framebufferCreateInfo.layers = createInfo.getLayers();
	framebufferCreateInfo.renderPass = createInfo.getRenderPass()->getVkHandle();
	framebufferCreateInfo.attachmentCount = createInfo.getNumAttachments();

	pvrvk::ArrayOrVector<VkImageView, 4> imageViews(framebufferCreateInfo.attachmentCount);
	framebufferCreateInfo.pAttachments = imageViews.get();
	// do all the color attachments
	for (uint32_t i = 0; i < createInfo.getNumAttachments(); ++i) { imageViews[i] = createInfo.getAttachment(i)->getVkHandle(); }
	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateFramebuffer(getDevice()->getVkHandle(), &framebufferCreateInfo, NULL, &_vkHandle), "Create Framebuffer Failed");
}
//!\endcond
} // namespace impl
} // namespace pvrvk
