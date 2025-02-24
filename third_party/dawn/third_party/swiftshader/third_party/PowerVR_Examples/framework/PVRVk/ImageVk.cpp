/*!
\brief Function definitions for PVRVk image class.
\file PVRVk/ImageVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "PVRVk/ImageVk.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/QueueVk.h"
#include "PVRVk/CommandPoolVk.h"
namespace pvrvk {
namespace impl {
Image_::~Image_()
{
	if (isAllocated())
	{
		if (getVkHandle() != VK_NULL_HANDLE)
		{
			if (!_device.expired())
			{
				getDevice()->getVkBindings().vkDestroyImage(getDevice()->getVkHandle(), getVkHandle(), nullptr);
				_vkHandle = VK_NULL_HANDLE;
			}
			else
			{
				reportDestroyedAfterDevice();
			}
		}
	}
}

Image_::Image_(make_shared_enabler, const DeviceWeakPtr& device, const ImageCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils(), _createInfo(createInfo)
{
	Device deviceSharedPtr = device.lock();

	auto& bindings = deviceSharedPtr->getVkBindings();

	VkImageCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_IMAGE_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkImageCreateFlags>(_createInfo.getFlags());
	vkCreateInfo.imageType = static_cast<VkImageType>(_createInfo.getImageType());
	vkCreateInfo.extent.width = _createInfo.getExtent().getWidth();
	vkCreateInfo.extent.height = _createInfo.getExtent().getHeight();
	vkCreateInfo.extent.depth = _createInfo.getExtent().getDepth();
	vkCreateInfo.mipLevels = _createInfo.getNumMipLevels();
	vkCreateInfo.arrayLayers = _createInfo.getNumArrayLayers();
	vkCreateInfo.samples = static_cast<VkSampleCountFlagBits>(_createInfo.getNumSamples());
	vkCreateInfo.format = static_cast<VkFormat>(_createInfo.getFormat());
	vkCreateInfo.sharingMode = static_cast<VkSharingMode>(_createInfo.getSharingMode());
	vkCreateInfo.tiling = static_cast<VkImageTiling>(_createInfo.getTiling());
	vkCreateInfo.usage = static_cast<VkImageUsageFlags>(_createInfo.getUsageFlags());
	vkCreateInfo.queueFamilyIndexCount = _createInfo.getNumQueueFamilyIndices();
	vkCreateInfo.pQueueFamilyIndices = _createInfo.getQueueFamilyIndices();
	vkCreateInfo.initialLayout = static_cast<VkImageLayout>(_createInfo.getInitialLayout());

#ifdef DEBUG
	deviceSharedPtr->getPhysicalDevice()->getImageFormatProperties(static_cast<pvrvk::Format>(_createInfo.getFormat()), static_cast<pvrvk::ImageType>(_createInfo.getImageType()),
		static_cast<pvrvk::ImageTiling>(_createInfo.getTiling()), static_cast<pvrvk::ImageUsageFlags>(_createInfo.getUsageFlags()),
		static_cast<pvrvk::ImageCreateFlags>(_createInfo.getFlags()));
#endif

	impl::vkThrowIfFailed(bindings.vkCreateImage(deviceSharedPtr->getVkHandle(), &vkCreateInfo, NULL, &_vkHandle), "ImageVk createImage");

	bindings.vkGetImageMemoryRequirements(deviceSharedPtr->getVkHandle(), _vkHandle, (VkMemoryRequirements*)(&_memReqs));

#ifdef DEBUG
	_currentLayout = _createInfo.getInitialLayout();
#endif
}

pvrvk::SubresourceLayout Image_::getSubresourceLayout(const pvrvk::ImageSubresource& subresource) const
{
	pvrvk::SubresourceLayout layout;

	getDevice()->getVkBindings().vkGetImageSubresourceLayout(getDevice()->getVkHandle(), _vkHandle, &(VkImageSubresource&)subresource, &(VkSubresourceLayout&)layout);
	return layout;
}
} // namespace impl

namespace impl {
SwapchainImage_::~SwapchainImage_()
{
	if (isAllocated())
	{
		if (_device.expired()) { reportDestroyedAfterDevice(); }
	}
	_vkHandle = VK_NULL_HANDLE;
}

SwapchainImage_::SwapchainImage_(make_shared_enabler, const DeviceWeakPtr& device, const VkImage& swapchainImage, const Format& format, const Extent3D& extent,
	uint32_t numArrayLevels, uint32_t numMipLevels, const ImageUsageFlags& usage)
	: Image_(make_shared_enabler{}, device)
{
	_vkHandle = swapchainImage;

	_createInfo = pvrvk::ImageCreateInfo();
	_createInfo.setImageType(ImageType::e_2D);
	_createInfo.setFormat(format);
	_createInfo.setExtent(extent);
	_createInfo.setNumArrayLayers(numArrayLevels);
	_createInfo.setNumMipLevels(numMipLevels);
	_createInfo.setUsageFlags(usage);

	_memReqs = {};
}
} // namespace impl

namespace impl {
ImageView_::ImageView_(make_shared_enabler, const DeviceWeakPtr& device, const ImageViewCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils(), _createInfo(createInfo)
{
	VkImageViewCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_IMAGE_VIEW_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkImageViewCreateFlags>(_createInfo.getFlags());
	vkCreateInfo.image = _createInfo.getImage()->getVkHandle();
	vkCreateInfo.viewType = static_cast<VkImageViewType>(_createInfo.getViewType());
	vkCreateInfo.format = static_cast<VkFormat>(_createInfo.getFormat());
	vkCreateInfo.components = (VkComponentMapping&)_createInfo.getComponents();
	vkCreateInfo.subresourceRange = (VkImageSubresourceRange&)_createInfo.getSubresourceRange();

	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateImageView(getDevice()->getVkHandle(), &vkCreateInfo, nullptr, &_vkHandle), "Failed to create ImageView");
}
ImageView_::~ImageView_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyImageView(getDevice()->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
}

} // namespace impl
} // namespace pvrvk

//!\endcond
