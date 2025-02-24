/*!
\brief The Swapchain class. The Swapchain is the object wrapping
the on-screen rendering Framebuffer images (aka front/backbuffers)
\file PVRVk/SwapchainVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/ImageVk.h"
namespace pvrvk {
/// <summary>Contains swapchain create informations</summary>
struct SwapchainCreateInfo
{
public:
	SwapchainCreateFlagsKHR flags; //!< Bitmask of SwapchainCreateFlagBitsKHR indicating parameters of swapchain creation.
	Surface surface; //!< The surface that the swapchain will present images to.
	uint32_t minImageCount; //!< The minimum number of presentable images that the application needs. The platform will either create the swapchain with at least that many images,
							//!< or will fail to create the swapchain
	Format imageFormat; //!< Format that is valid for swapchains on the specified surface.
	ColorSpaceKHR imageColorSpace; //!<  ColorSpaceKHR that is valid for swapchains on the specified surface.
	Extent2D imageExtent; //!< Size (in pixels) of the swapchain. Behavior is platform-dependent when the image extent does not match the surface's currentExtent as returned by
						  //!< PysicalDevice::getSurfaceCapabilities
	uint32_t imageArrayLayers; //!< Number of views in a multiview/stereo surface. For non-stereoscopic-3D applications, this value is 1
	ImageUsageFlags imageUsage; //!< Bitmask of ImageUsageFlagBits, indicating how the application will use the swapchain's presentable images
	SharingMode imageSharingMode; //!< Sharing mode used for the images of the swapchain.
	uint32_t numQueueFamilyIndex; //!< Number of queue families having access to the images of the swapchain in case imageSharingMode is VKSharingMode::e_CONCURRENT
	const uint32_t* queueFamilyIndices; //!< Array of queue family indices having access to the images of the swapchain in case imageSharingMode is VKSharingMode::e_CONCURRENT
	SurfaceTransformFlagsKHR preTransform; //!< Bitmask of SurfaceTransformFlagBitsKHR, describing the transform, relative to the presentation engine's natural
										   //!< orientation, applied to the image content prior to presentation
	CompositeAlphaFlagsKHR compositeAlpha; //!< Bitmask of CompositeAlphaFlagBitsKHR indicating the alpha compositing mode to use when this surface is composited
										   //!< together with other surfaces on certain window systems
	PresentModeKHR presentMode; //!< Presentation mode the swapchain will use. A swapchain's present mode determines how incoming present requests will be processed and queued internally
	bool clipped; //!< indicates whether the Vulkan implementation is allowed to discard rendering operations that affect regions of the surface which are not visible
	Swapchain oldSwapchain; //!< If not null handle, specifies the swapchain that will be replaced by the new swapchain being created. The new swapchain will be a descendant of
							//!< oldSwapchain. Further, any descendants of the new swapchain will also be descendants of oldSwapchain

	/// <summary>Constructor. Default intialised</summary>
	SwapchainCreateInfo()
		: flags(SwapchainCreateFlagsKHR(0)), minImageCount(0), imageFormat(Format::e_UNDEFINED), imageColorSpace(ColorSpaceKHR::e_PASS_THROUGH_EXT), imageExtent(Extent2D()),
		  imageArrayLayers(0), imageUsage(ImageUsageFlags::e_COLOR_ATTACHMENT_BIT), imageSharingMode(SharingMode::e_EXCLUSIVE), numQueueFamilyIndex(0),
		  preTransform(SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR), compositeAlpha(CompositeAlphaFlagsKHR::e_OPAQUE_BIT_KHR), presentMode(PresentModeKHR::e_FIFO_KHR), clipped(true)
	{}

	/// <summary>Constructor.</summary>
	/// <param name="surface">The surface onto which the swapchain will present images</param>
	/// <param name="minImageCount">The minimum number of presentable images that the application needs</param>
	/// <param name="imageExtent">The size (in pixels) of the swapchain image(s)</param>
	/// <param name="flags">A set of pvrvk::SwapchainCreateFlagsKHR indicating parameters of the swapchain creation.</param>
	/// <param name="numQueueFamilyIndex">The number of queue families having access to the image(s) of the swapchain when imageSharingMode is VK_SHARING_MODE_CONCURRENT</param>
	/// <param name="queueFamilyIndices">An array of queue family indices having access to the images(s) of the swapchain when imageSharingMode is VK_SHARING_MODE_CONCURRENT</param>
	/// <param name="imageUsage">A set of pvrvk::ImageUsageFlags describing the intended usage of the (acquired) swapchain images.</param>
	/// <param name="imageFormat">A pvrvk::Format value specifying the format the swapchain image(s) will be created with.</param>
	/// <param name="imageColorSpace">A pvrvk::ColorSpaceKHR value specifying the way the swapchain interprets image data.</param>
	/// <param name="presentMode">The presentation mode the swapchain will use.</param>
	/// <param name="imageArrayLayers">The number of views in a multiview/stereo surface</param>
	/// <param name="imageSharingMode">The sharing mode used for the image(s) of the swapchain.</param>
	/// <param name="preTransform">A pvrvk::SurfaceTransformFlagsKHR value describing the transform, relative to the presentation engine's natural orientation, applied to the image content prior to presentation. If it does not match the currentTransform value returned by vkGetPhysicalDeviceSurfaceCapabilitiesKHR, the presentation engine will transform the image content as part of the presentation operation.</param>
	/// <param name="compositeAlpha">A pvrvk::CompositeAlphaFlagsKHR value indicating the alpha compositing mode to use when this surface is composited together with other surfaces on certain window systems.</param>
	/// <param name="clipped">Specifies whether the Vulkan implementation is allowed to discard rendering operations that affect regions of the surface that are not visible.</param>
	/// <param name="oldSwapchain">Is a null pvrvk::Swapchain, or the existing non-retired swapchain currently associated with surface.</param>
	SwapchainCreateInfo(Surface surface, uint32_t minImageCount, Extent2D imageExtent, SwapchainCreateFlagsKHR flags, uint32_t numQueueFamilyIndex,
		const uint32_t* queueFamilyIndices, ImageUsageFlags imageUsage, Format imageFormat = Format::e_R8G8B8A8_UNORM,
		ColorSpaceKHR imageColorSpace = ColorSpaceKHR::e_PASS_THROUGH_EXT, PresentModeKHR presentMode = PresentModeKHR::e_FIFO_KHR, uint32_t imageArrayLayers = 1,
		SharingMode imageSharingMode = SharingMode::e_EXCLUSIVE, SurfaceTransformFlagsKHR preTransform = SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR,
		CompositeAlphaFlagsKHR compositeAlpha = CompositeAlphaFlagsKHR::e_OPAQUE_BIT_KHR, bool clipped = true, Swapchain oldSwapchain = Swapchain())
		: flags(flags), surface(surface), minImageCount(minImageCount), imageFormat(imageFormat), imageColorSpace(imageColorSpace), imageExtent(imageExtent),
		  imageArrayLayers(imageArrayLayers), imageUsage(imageUsage), imageSharingMode(imageSharingMode), numQueueFamilyIndex(numQueueFamilyIndex),
		  queueFamilyIndices(queueFamilyIndices), preTransform(preTransform), compositeAlpha(compositeAlpha), presentMode(presentMode), clipped(clipped), oldSwapchain(oldSwapchain)
	{}
};

namespace impl {
/// <summary>The Swapchain is the object wrapping the on - screen rendering Framebuffer images
/// (aka front/backbuffers)</summary>
class Swapchain_ : public PVRVkDeviceObjectBase<VkSwapchainKHR, ObjectType::e_SWAPCHAIN_KHR>, public DeviceObjectDebugUtils<Swapchain_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() = default;
		friend class Swapchain_;
	};

	static Swapchain constructShared(const DeviceWeakPtr& device, Surface surface, const SwapchainCreateInfo& createInfo)
	{
		return std::make_shared<Swapchain_>(make_shared_enabler{}, device, surface, createInfo);
	}

	uint32_t _swapchainId;
	uint32_t _swapChainLength;
	ImageView _colorImageViews[static_cast<uint32_t>(FrameworkCaps::MaxSwapChains)];
	Surface _surface;
	SwapchainCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Swapchain_)
	Swapchain_(make_shared_enabler, const DeviceWeakPtr& device, Surface surface, const SwapchainCreateInfo& createInfo);
	~Swapchain_();
	//!\endcond

	/// <summary>Acquire next image. The acquired swapchain index can be retrieved by calling getSwapchainIndex.
	///   Note: The presenation engine may still be consuming the swapchain image, therefore the calle must synchronise it before using it.</summary>
	/// <param name="timeOut">indicates how long the function waits, in nanoseconds, if no image is available.</param>
	/// <param name="signalSemaphore"> is null semaphore or a semaphore to signal</param>
	/// <param name="signalFence">is a null fence or fence to signal</param>
	/// <returns>Return true if image was acquired, false if timed out or suboptimal.</returns>
	bool acquireNextImage(uint64_t timeOut, Semaphore signalSemaphore, Fence signalFence);

	/// <summary>Acquire next image. The acquired swapchain index can be retrieved by calling getSwapchainIndex.
	///   Note: The presenation engine may still be consuming the swapchain image, therefore the calle must synchronise it before using it.</summary>
	/// <param name="timeOut">indicates how long the function waits, in nanoseconds, if no image is available.</param>
	/// <param name="signalSemaphore"> is null semaphore or a semaphore to signal</param>
	/// <returns>Return true if image was acquired, false if timed out or suboptimal.</returns>
	bool acquireNextImage(uint64_t timeOut, Semaphore signalSemaphore = Semaphore());

	/// <summary>Get swapchain length</summary>
	/// <returns>uint</returns>32_t
	uint32_t getSwapchainLength() const { return _swapChainLength; }

	/// <summary>Get the acquired swapchain index. Note: The presenation engine may still be consuming
	///   the swapchain image, therefore the calle must synchronise it before using it.  swapchain index</summary>
	/// <returns></returns>
	const uint32_t& getSwapchainIndex() const { return _swapchainId; }

	/// <summary>Get swapchain image view</summary>
	/// <param name="swapchain">swapchain index</param>
	/// <returns>ImageView</returns>
	const ImageView& getImageView(uint32_t swapchain) const
	{
		assert(swapchain < FrameworkCaps::MaxSwapChains && "Index out of bound");
		return _colorImageViews[swapchain];
	}

	/// <summary>Get swapchain image view</summary>
	/// <param name="swapchain">swapchain index</param>
	/// <returns>ImageView</returns>
	ImageView& getImageView(uint32_t swapchain)
	{
		assert(swapchain < FrameworkCaps::MaxSwapChains && "Index out of bound");
		return _colorImageViews[swapchain];
	}

	/// <summary>Get swapchain image</summary>
	/// <param name="swapchain">swapchain index</param>
	/// <returns>Image</returns>
	const Image& getImage(uint32_t swapchain) const { return getImageView(swapchain)->getImage(); }

	/// <summary>Get swapchain image</summary>
	/// <param name="swapchain">swapchain index</param>
	/// <returns>Image</returns>
	Image& getImage(uint32_t swapchain) { return getImageView(swapchain)->getImage(); }

	/// <summary>Get dimension</summary>
	/// <returns>Extent</returns>
	Extent2D getDimension() const { return Extent2D(_createInfo.imageExtent.getWidth(), _createInfo.imageExtent.getHeight()); }

	/// <summary>Gets whether the swapchain images are clipped</summary>
	/// <returns>True if the swapchain images are clipped</returns>
	bool isClipped() const { return _createInfo.clipped; }

	/// <summary>Gets the CompositeAlphaFlagsKHR of the swapchain images</summary>
	/// <returns>The CompositeAlphaFlagsKHR for the swapchain image</returns>
	CompositeAlphaFlagsKHR getCompositeAlphaFlags() const { return _createInfo.compositeAlpha; }

	/// <summary>Gets the number of array layers of the swapchain images</summary>
	/// <returns>The number of array layers of the swapchain images</returns>
	uint32_t getNumArrayLayers() const { return _createInfo.imageArrayLayers; }

	/// <summary>Get swapchain image format</summary>
	/// <returns>Format</returns>
	Format getImageFormat() const { return _createInfo.imageFormat; }

	/// <summary>Gets the color space of the swapchain images</summary>
	/// <returns>The color space of the swapchain images</returns>
	ColorSpaceKHR getColorSpace() const { return _createInfo.imageColorSpace; }

	/// <summary>Gets the surface transform flags of the swapchain images</summary>
	/// <returns>The surface transform flags of the swapchain images</returns>
	SurfaceTransformFlagsKHR getTransformFlags() const { return _createInfo.preTransform; }

	/// <summary>Gets the image sharing mode of the swapchain images</summary>
	/// <returns>The image sharing mode of the swapchain images</returns>
	SharingMode getSharingMode() const { return _createInfo.imageSharingMode; }

	/// <summary>Gets the presentation mode of the swapchain images</summary>
	/// <returns>The presentation mode of the swapchain images</returns>
	PresentModeKHR getPresentationMode() const { return _createInfo.presentMode; }

	/// <summary>Gets the number of queue families which can make use of the swapchain images</summary>
	/// <returns>The number of queue families which can make use of the swapchain images</returns>
	uint32_t getNumQueueFamilyIndices() const { return _createInfo.numQueueFamilyIndex; }

	/// <summary>Gets the queue family indicies for the queues which can make use of the swapchain images</summary>
	/// <returns>The queue family indicies for the queues which can make use of the swapchain images</returns>
	std::vector<uint32_t> getQueueFamilyIndices() const
	{
		std::vector<uint32_t> indices;
		for (uint32_t i = 0; i < _createInfo.numQueueFamilyIndex; i++) { indices.emplace_back(_createInfo.queueFamilyIndices[i]); }
		return indices;
	}

	/// <summary>Gets the swapchain image usage flags</summary>
	/// <returns>The swapchain image usage</returns>
	ImageUsageFlags getUsage() const { return _createInfo.imageUsage; }

	/// <summary>Returns whether the swapchain supports the specified image usage flag bits</summary>
	/// <param name="imageUsage">The ImageUsageFlags bits to check for support</param>
	/// <returns>True if the swapchain supports the specified image usage</returns>
	bool supportsUsage(const ImageUsageFlags& imageUsage) const { return static_cast<uint32_t>(getUsage() & imageUsage) != 0; }
};
} // namespace impl
} // namespace pvrvk
