/*!
\brief The PVRVk Image class and related classes (SwapchainImage, ImageView).
\file PVRVk/ImageVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/DeviceMemoryVk.h"

namespace pvrvk {
namespace {
inline ImageViewType convertToPVRVkImageViewType(ImageType baseType, uint32_t numArrayLayers, bool isCubeMap)
{
	// if it is a cube map it has to be 2D Texture base
	if (isCubeMap && baseType != ImageType::e_2D)
	{
		assert(baseType == ImageType::e_2D && "Cubemap texture must be 2D");
		return ImageViewType::e_MAX_ENUM;
	}
	// array must be atleast 1
	if (!numArrayLayers)
	{
		assert(false && "Number of array layers must be greater than equal to 0");
		return ImageViewType::e_MAX_ENUM;
	}
	// if it is array it must be 1D or 2D texture base
	if ((numArrayLayers > 1) && (baseType > ImageType::e_2D))
	{
		assert(false && "1D and 2D image type supports array texture");
		return ImageViewType::e_MAX_ENUM;
	}

	ImageViewType vkType[] = { ImageViewType::e_1D, ImageViewType::e_1D_ARRAY, ImageViewType::e_2D, ImageViewType::e_2D_ARRAY, ImageViewType::e_3D, ImageViewType::e_CUBE,
		ImageViewType::e_CUBE_ARRAY };
	if (isCubeMap) { numArrayLayers = (numArrayLayers > 6u) * 6u; }
	return vkType[(static_cast<uint32_t>(baseType) * 2) + (isCubeMap ? 3 : 0) + (numArrayLayers > 1 ? 1 : 0)];
}

inline ImageAspectFlags formatToImageAspect(Format format)
{
	if (format == Format::e_UNDEFINED) { throw ErrorUnknown("Cannot retrieve VkImageAspectFlags from an undefined VkFormat"); }
	if (format < Format::e_D16_UNORM || format > Format::e_D32_SFLOAT_S8_UINT) { return ImageAspectFlags::e_COLOR_BIT; }
	const ImageAspectFlags formats[] = {
		ImageAspectFlags::e_DEPTH_BIT, // VkFormat::e_D16_UNORM
		ImageAspectFlags::e_DEPTH_BIT, // VkFormat::e_X8_D24_UNORM_PACK32
		ImageAspectFlags::e_DEPTH_BIT, // VkFormat::e_D32_SFLOAT
		ImageAspectFlags::e_STENCIL_BIT, // VkFormat::e_S8_UINT
		ImageAspectFlags::e_DEPTH_BIT | ImageAspectFlags::e_STENCIL_BIT, // VkFormat::e_D16_UNORM_S8_UINT
		ImageAspectFlags::e_DEPTH_BIT | ImageAspectFlags::e_STENCIL_BIT, // VkFormat::e_D24_UNORM_S8_UINT
		ImageAspectFlags::e_DEPTH_BIT | ImageAspectFlags::e_STENCIL_BIT, // VkFormat::e_D32_SFLOAT_S8_UINT
	};
	return formats[static_cast<uint32_t>(format) - static_cast<uint32_t>(Format::e_D16_UNORM)];
}
} // namespace

/// <summary>Image creation descriptor.</summary>
struct ImageCreateInfo
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	ImageCreateInfo()
		: _flags(ImageCreateFlags::e_NONE), _imageType(ImageType::e_2D), _extent(Extent3D()), _numMipLevels(1), _numArrayLayers(1), _numSamples(SampleCountFlags::e_1_BIT),
		  _format(Format::e_UNDEFINED), _sharingMode(SharingMode::e_EXCLUSIVE), _usageFlags(ImageUsageFlags::e_NONE), _initialLayout(ImageLayout::e_UNDEFINED),
		  _tiling(ImageTiling::e_OPTIMAL), _numQueueFamilyIndices(0), _queueFamilyIndices(nullptr)
	{}

	/// <summary>Constructor</summary>
	/// <param name="imageType">Image creation type (ImageType)</param>
	/// <param name="format">Image creation format (Format)</param>
	/// <param name="extent">Image creation extent (Extent3D)</param>
	/// <param name="usage">Image creation usage (ImageUsageFlags)</param>
	/// <param name="numMipLevels">The number of Image mip map levels</param>
	/// <param name="numArrayLayers">The number of Image array layers</param>
	/// <param name="samples">The number of Image samples (SampleCountFlags)</param>
	/// <param name="flags">Image creation flags (ImageCreateFlags)</param>
	/// <param name="tiling">The Image tiling mode (ImageTiling)</param>
	/// <param name="sharingMode">The Image sharing mode (SharingMode)</param>
	/// <param name="initialLayout">The initial layout of the Image (ImageLayout)</param>
	/// <param name="queueFamilyIndices">A pointer to a list of queue family indices specifying the list of queue families which can make use of the image</param>
	/// <param name="numQueueFamilyIndices">The number of queue family indices pointed to by queueFamilyIndices</param>
	ImageCreateInfo(ImageType imageType, pvrvk::Format format, const pvrvk::Extent3D& extent, pvrvk::ImageUsageFlags usage, uint32_t numMipLevels = 1, uint32_t numArrayLayers = 1,
		pvrvk::SampleCountFlags samples = pvrvk::SampleCountFlags::e_1_BIT, pvrvk::ImageCreateFlags flags = pvrvk::ImageCreateFlags::e_NONE,
		ImageTiling tiling = ImageTiling::e_OPTIMAL, SharingMode sharingMode = SharingMode::e_EXCLUSIVE, ImageLayout initialLayout = ImageLayout::e_UNDEFINED,
		const uint32_t* queueFamilyIndices = nullptr, uint32_t numQueueFamilyIndices = 0)
		: _flags(flags), _imageType(imageType), _extent(extent), _numMipLevels(numMipLevels), _numArrayLayers(numArrayLayers), _numSamples(samples), _format(format),
		  _sharingMode(sharingMode), _usageFlags(usage), _initialLayout(initialLayout), _tiling(tiling), _numQueueFamilyIndices(numQueueFamilyIndices),
		  _queueFamilyIndices(queueFamilyIndices)
	{}

	/// <summary>Get Image creation Flags</summary>
	/// <returns>Image creation flags (ImageCreateFlags)</returns>
	inline ImageCreateFlags getFlags() const { return _flags; }
	/// <summary>Set PVRVk Image creation flags</summary>
	/// <param name="flags">Flags to use for creating a PVRVk image</param>
	inline void setFlags(ImageCreateFlags flags) { this->_flags = flags; }
	/// <summary>Get Image creation type</summary>
	/// <returns>Image creation type (ImageType)</returns>
	inline ImageType getImageType() const { return _imageType; }
	/// <summary>Set PVRVk Image creation image type</summary>
	/// <param name="imageType">ImageType to use for creating a PVRVk image</param>
	inline void setImageType(ImageType imageType) { this->_imageType = imageType; }
	/// <summary>Get Extent</summary>
	/// <returns>Extent</returns>
	inline const Extent3D& getExtent() const { return _extent; }
	/// <summary>Set PVRVk Image creation image extents</summary>
	/// <param name="extent">extent to use for creating a PVRVk image</param>
	inline void setExtent(Extent3D extent) { this->_extent = extent; }
	/// <summary>Get the number of Image mip map levels</summary>
	/// <returns>Image mip map levels</returns>
	inline uint32_t getNumMipLevels() const { return _numMipLevels; }
	/// <summary>Set the number of mipmap levels for PVRVk Image creation</summary>
	/// <param name="numMipLevels">The number of mipmap levels to use for creating a PVRVk image</param>
	inline void setNumMipLevels(uint32_t numMipLevels) { this->_numMipLevels = numMipLevels; }
	/// <summary>Get the number of Image array layers</summary>
	/// <returns>Image array layers</returns>
	inline uint32_t getNumArrayLayers() const { return _numArrayLayers; }
	/// <summary>Set the number of array layers for PVRVk Image creation</summary>
	/// <param name="numArrayLayers">The number of array layers to use for creating a PVRVk image</param>
	inline void setNumArrayLayers(uint32_t numArrayLayers) { this->_numArrayLayers = numArrayLayers; }
	/// <summary>Get the number of samples taken for the Image</summary>
	/// <returns>Image number of samples (SampleCountFlags)</returns>
	inline SampleCountFlags getNumSamples() const { return _numSamples; }
	/// <summary>Set the number of samples for PVRVk Image creation</summary>
	/// <param name="numSamples">The number of samples to use for creating a PVRVk image</param>
	inline void setNumSamples(SampleCountFlags numSamples) { this->_numSamples = numSamples; }
	/// <summary>Get Image format</summary>
	/// <returns>Image format (Format)</returns>
	inline Format getFormat() const { return _format; }
	/// <summary>Set the Image format for PVRVk Image creation</summary>
	/// <param name="format">The image format to use for creating a PVRVk image</param>
	inline void setFormat(Format format) { this->_format = format; }
	/// <summary>Get Image sharing mode</summary>
	/// <returns>Image sharing mode (SharingMode)</returns>
	inline SharingMode getSharingMode() const { return _sharingMode; }
	/// <summary>Set the Image sharing mode for PVRVk Image creation</summary>
	/// <param name="sharingMode">The image sharing mode to use for creating a PVRVk image</param>
	inline void setSharingMode(SharingMode sharingMode) { this->_sharingMode = sharingMode; }
	/// <summary>Get Image usage flags</summary>
	/// <returns>Image usage flags (ImageUsageFlags)</returns>
	inline ImageUsageFlags getUsageFlags() const { return _usageFlags; }
	/// <summary>Set the Image usage flags for PVRVk Image creation</summary>
	/// <param name="usageFlags">The image usage flags to use for creating a PVRVk image</param>
	inline void setUsageFlags(ImageUsageFlags usageFlags) { this->_usageFlags = usageFlags; }
	/// <summary>Get Image initial layout</summary>
	/// <returns>Image initial layout (ImageLayout)</returns>
	inline ImageLayout getInitialLayout() const { return _initialLayout; }
	/// <summary>Set the Image initial layout for PVRVk Image creation</summary>
	/// <param name="initialLayout">The image initial layout to use for creating a PVRVk image</param>
	inline void setInitialLayout(ImageLayout initialLayout) { this->_initialLayout = initialLayout; }
	/// <summary>Get Image tiling mode</summary>
	/// <returns>Image initial tiling mode (ImageTiling)</returns>
	inline ImageTiling getTiling() const { return _tiling; }
	/// <summary>Set the Image tiling for PVRVk Image creation</summary>
	/// <param name="tiling">The image tiling to use for creating a PVRVk image</param>
	inline void setTiling(ImageTiling tiling) { this->_tiling = tiling; }
	/// <summary>Get the number of queue family inidices for the image</summary>
	/// <returns>The number of Image queue families</returns>
	inline uint32_t getNumQueueFamilyIndices() const { return _numQueueFamilyIndices; }
	/// <summary>Set the number of queue family inidices specifying the number of queue families which can use the PVRVk Image</summary>
	/// <param name="numQueueFamilyIndices">The number of queue family inidices specifying the number of queue families which can use the PVRVk Image</param>
	inline void setNumQueueFamilyIndices(uint32_t numQueueFamilyIndices) { this->_numQueueFamilyIndices = numQueueFamilyIndices; }
	/// <summary>Get a pointer to a list of queue family inidices for the image</summary>
	/// <returns>A pointer to the list of Image queue families</returns>
	inline const uint32_t* getQueueFamilyIndices() const { return _queueFamilyIndices; }

private:
	/// <summary>Flags to use for creating the image</summary>
	ImageCreateFlags _flags;
	/// <summary>The type of the image (1D, 2D, 3D)</summary>
	ImageType _imageType;
	/// <summary>The extent of the image</summary>
	Extent3D _extent;
	/// <summary>The number of mipmap levels</summary>
	uint32_t _numMipLevels;
	/// <summary>The number of array layers</summary>
	uint32_t _numArrayLayers;
	/// <summary>The number of samples to use</summary>
	SampleCountFlags _numSamples;
	/// <summary>The image format</summary>
	Format _format;
	/// <summary>Specifies the image sharing mode specifying how the image can be used by multiple queue families</summary>
	SharingMode _sharingMode;
	/// <summary>describes the images' intended usage</summary>
	ImageUsageFlags _usageFlags;
	/// <summary>Specifies the initial layout of all image subresources of the image</summary>
	ImageLayout _initialLayout;
	/// <summary>Specifies the tiling arrangement of the data elements in memory</summary>
	ImageTiling _tiling;
	/// <summary>The number of queue families in the _queueFamilyIndices array</summary>
	uint32_t _numQueueFamilyIndices;
	/// <summary>The list of queue families that will access this image</summary>
	const uint32_t* _queueFamilyIndices;
};

namespace impl {
/// <summary>ImageVk implementation that wraps the Vulkan Texture object</summary>
class Image_ : public PVRVkDeviceObjectBase<VkImage, ObjectType::e_IMAGE>, public DeviceObjectDebugUtils<Image_>
{
protected:
	friend class Device_;

	/// <summary>A class which restricts the creation of a pvrvk::Image to children or friends of a pvrvk::impl::Image_.</summary>
	class make_shared_enabler
	{
	protected:
		/// <summary>Constructor for a make_shared_enabler.</summary>
		make_shared_enabler() {}
		friend class Image_;
	};

	/// <summary>Protected function used to construct a pvrvk::Image. Note that this function shouldn't normally be called
	/// directly and will be called by a friend of Image_ which will generally be a Device</summary>
	/// <param name="device">The Device from which the image will be creted</param>
	/// <param name="createInfo">The ImageCreateInfo descriptor specifying creation parameters</param>
	/// <returns>Returns a successfully created pvrvk::Image</returns>
	static Image constructShared(const DeviceWeakPtr& device, const ImageCreateInfo& createInfo) { return std::make_shared<Image_>(make_shared_enabler{}, device, createInfo); }

	/// <summary>Protected function used to construct a pvrvk::Image. Note that this function shouldn't normally be called
	/// directly and will be called by a friend of Image_ which will generally be a Device</summary>
	/// <param name="device">The Device from which the image will be creted</param>
	/// <returns>Returns a successfully created pvrvk::Image</returns>
	static Image constructShared(const DeviceWeakPtr& device) { return std::make_shared<Image_>(make_shared_enabler{}, device); }

	/// <summary>Image specific memory requirements</summary>
	pvrvk::MemoryRequirements _memReqs;
	/// <summary>Device memory bound to this image (Only for non sparse image).</summary>
	DeviceMemory _memory;
	/// <summary>Creation information used when creating the image.</summary>
	pvrvk::ImageCreateInfo _createInfo;

#ifdef DEBUG
	//!\cond NO_DOXYGEN
	pvrvk::ImageLayout _currentLayout;
	//!\endcond
#endif

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Image_)
	/// <summary>Destructor. Will properly release all resources held by this object.</summary>
	virtual ~Image_();

	/// <summary>Constructor.</summary>
	/// <param name="device">The Device from which the image will be created</param>
	/// <param name="createInfo">The ImageCreateInfo descriptor specifying creation parameters</param>
	Image_(make_shared_enabler, const DeviceWeakPtr& device, const ImageCreateInfo& createInfo);

	/// <summary>Constructor.</summary>
	/// <param name="device">The Devices from which to create the image from</param>
	Image_(make_shared_enabler, const DeviceWeakPtr& device) : PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils() {}
	//!\endcond

	/// <summary>Query and return a SubresourceLayout object describing the layout of the image</summary>
	/// <param name="subresource">A subresource used to retrieve a subresource layout</param>
	/// <returns>The SubresourceLayout of the image</returns>
	pvrvk::SubresourceLayout getSubresourceLayout(const pvrvk::ImageSubresource& subresource) const;

	/// <summary>Return a reference to the creation descriptor of the image</summary>
	/// <returns>The reference to the ImageCreateInfo</returns>
	pvrvk::ImageCreateInfo getCreateInfo() const { return _createInfo; }

	/// <summary>Get Image creation Flags</summary>
	/// <returns>Image creation flags (ImageCreateFlags)</returns>
	inline ImageCreateFlags getFlags() const { return _createInfo.getFlags(); }

	/// <summary>Get Image creation type</summary>
	/// <returns>Image creation type (ImageType)</returns>
	inline ImageType getImageType() const { return _createInfo.getImageType(); }

	/// <summary>Get Extent</summary>
	/// <returns>Extent</returns>
	inline Extent3D getExtent() const { return _createInfo.getExtent(); }

	/// <summary>Get the number of Image mip map levels</summary>
	/// <returns>Image mip map levels</returns>
	inline uint32_t getNumMipLevels() const { return _createInfo.getNumMipLevels(); }

	/// <summary>Get the number of Image array layers</summary>
	/// <returns>Image array layers</returns>
	inline uint32_t getNumArrayLayers() const { return _createInfo.getNumArrayLayers(); }

	/// <summary>Get the number of samples taken for the Image</summary>
	/// <returns>Image number of samples (SampleCountFlags)</returns>
	inline SampleCountFlags getNumSamples() const { return _createInfo.getNumSamples(); }

	/// <summary>Get Image format</summary>
	/// <returns>Image format (Format)</returns>
	inline Format getFormat() const { return _createInfo.getFormat(); }

	/// <summary>Get Image sharing mode</summary>
	/// <returns>Image sharing mode (SharingMode)</returns>
	inline SharingMode getSharingMode() const { return _createInfo.getSharingMode(); }

	/// <summary>Get Image usage flags</summary>
	/// <returns>Image usage flags (ImageUsageFlags)</returns>
	inline ImageUsageFlags getUsageFlags() const { return _createInfo.getUsageFlags(); }

	/// <summary>Get Image initial layout</summary>
	/// <returns>Image initial layout (ImageLayout)</returns>
	inline ImageLayout getInitialLayout() const { return _createInfo.getInitialLayout(); }

	/// <summary>Get Image tiling mode</summary>
	/// <returns>Image initial tiling mode (ImageTiling)</returns>
	inline ImageTiling getTiling() const { return _createInfo.getTiling(); }

	/// <summary>Get the number of queue family inidices for the image</summary>
	/// <returns>The number of Image queue families</returns>
	inline uint32_t getNumQueueFamilyIndices() const { return _createInfo.getNumQueueFamilyIndices(); }

	/// <summary>Get a pointer to a list of queue family inidices for the image</summary>
	/// <returns>A pointer to the list of Image queue families</returns>
	inline const uint32_t* const getQueueFamilyIndices() const { return _createInfo.getQueueFamilyIndices(); }

	/// <summary>Check if this texture is a cubemap</summary>
	/// <returns>true if the texture is a cubemap, otherwise false</returns>
	bool isCubeMap() const { return (_createInfo.getFlags() & pvrvk::ImageCreateFlags::e_CUBE_COMPATIBLE_BIT) == pvrvk::ImageCreateFlags::e_CUBE_COMPATIBLE_BIT; }

	/// <summary>Check if this texture is allocated.</summary>
	/// <returns>true if the texture is allocated. Otherwise, the texture is empty and must be constructed.</returns>
	bool isAllocated() const { return (getVkHandle() != VK_NULL_HANDLE); }

	/// <summary>Get the width of this texture (number of columns of texels in the lowest mipmap)</summary>
	/// <returns>Texture width</returns>
	uint16_t getWidth() const { return static_cast<uint16_t>(_createInfo.getExtent().getWidth()); }

	/// <summary>Get the height of this texture (number of rows of texels in the lowest mipmap)</summary>
	/// <returns>Texture height</returns>
	uint16_t getHeight() const { return static_cast<uint16_t>(_createInfo.getExtent().getHeight()); }

	/// <summary>Get the depth of this texture (number of (non-array) layers of texels in the lowest mipmap)</summary>
	/// <returns>Texture depth</returns>
	uint16_t getDepth() const { return static_cast<uint16_t>(_createInfo.getExtent().getDepth()); }

	/// <summary>If a memory object has been bound to the object, return it. Otherwise, return empty device memory.</summary>
	/// <returns>If a memory object has been bound to the object, the memory object. Otherwise, empty device memory.</returns>
	const DeviceMemory& getDeviceMemory() const { return _memory; }
	/// <summary>If a memory object has been bound to the object, return it. Otherwise, return empty device memory.</summary>
	/// <returns>If a memory object has been bound to the object, the memory object. Otherwise, empty device memory.</returns>
	DeviceMemory& getDeviceMemory() { return _memory; }
	/// <summary>Bind memory to this non sparse image.</summary>
	/// <param name="memory">The memory to attach to the given image object</param>
	/// <param name="offset">The offset into the given memory to attach to the given image object</param>
	void bindMemoryNonSparse(DeviceMemory memory, VkDeviceSize offset = 0)
	{
		if ((_createInfo.getFlags() &
				(pvrvk::ImageCreateFlags::e_SPARSE_ALIASED_BIT | pvrvk::ImageCreateFlags::e_SPARSE_BINDING_BIT | pvrvk::ImageCreateFlags::e_SPARSE_RESIDENCY_BIT)) != 0)
		{ throw ErrorValidationFailedEXT("Cannot bind memory: Image is Sparce so cannot have bound memory."); }
		if (_memory) { throw ErrorValidationFailedEXT("Cannot bind memory: A memory block is already bound"); }
		vkThrowIfFailed(getDevice()->getVkBindings().vkBindImageMemory(getDevice()->getVkHandle(), getVkHandle(), memory->getVkHandle(), offset),
			"Failed to bind a memory block to this image");
		_memory = memory;
	}

	/// <summary>Get the memory requirements of the image</summary>
	/// <returns>The memory requirements of the image (MemoryRequirements)</returns>
	const pvrvk::MemoryRequirements& getMemoryRequirement() const { return _memReqs; }

#ifdef DEBUG
	//!\cond NO_DOXYGEN
	/// <summary>Gets the current image layout of the image</summary>
	/// <returns>The current image layout of the image</returns>
	pvrvk::ImageLayout getImageLayout() { return _currentLayout; }

	/// <summary>Gets the current image layout of the image</summary>
	/// <param name="imageLayout">The new image layout for the image</param>
	void setImageLayout(pvrvk::ImageLayout imageLayout) { _currentLayout = imageLayout; }
	//!\endcond
#endif
};

/// <summary>The Specialized image for swapchain</summary>
class SwapchainImage_ : public Image_
{
private:
	friend class ::pvrvk::impl::Swapchain_;

	/// <summary>A class which restricts the creation of a pvrvk::SwapchainImage to children or friends of a pvrvk::impl::SwapchainImage_.</summary>
	class make_shared_enabler : public Image_::make_shared_enabler
	{
	private:
		make_shared_enabler() : Image_::make_shared_enabler() {}
		friend class SwapchainImage_;
	};

	/// <summary>Protected function used to construct a pvrvk::SwapchainImage. Note that this function shouldn't normally be called
	/// directly and will be called by a friend of SwapchainImage_ which will generally be a Device</summary>
	/// <param name="device">The Device from which the Swapchain image will be creted</param>
	/// <param name="swapchainImage">The Swapchain image handle</param>
	/// <param name="format">The format of the Swapchain image</param>
	/// <param name="extent">The extent of the Swapchain image</param>
	/// <param name="numArrayLevels">The number of array levels of the Swapchain image</param>
	/// <param name="numMipLevels">The number of mipmap levels of the Swapchain image</param>
	/// <param name="usage">The usage flags supported by the Swapchain image</param>
	/// <returns>Returns a successfully created pvrvk::SwapchainImage</returns>
	static SwapchainImage constructShared(const DeviceWeakPtr& device, const VkImage& swapchainImage, const Format& format, const Extent3D& extent, uint32_t numArrayLevels,
		uint32_t numMipLevels, const ImageUsageFlags& usage)
	{
		return std::make_shared<SwapchainImage_>(make_shared_enabler{}, device, swapchainImage, format, extent, numArrayLevels, numMipLevels, usage);
	}

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(SwapchainImage_)
	~SwapchainImage_();

	SwapchainImage_(make_shared_enabler, const DeviceWeakPtr& device, const VkImage& swapchainImage, const Format& format, const Extent3D& extent, uint32_t numArrayLevels,
		uint32_t numMipLevels, const ImageUsageFlags& usage);
	//!\endcond
};
} // namespace impl

/// <summary>Image view creation descriptor.</summary>
struct ImageViewCreateInfo
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	ImageViewCreateInfo()
		: _viewType(ImageViewType::e_2D), _format(Format::e_UNDEFINED), _components(ComponentMapping()), _subresourceRange(ImageSubresourceRange()),
		  _flags(ImageViewCreateFlags::e_NONE)
	{}

	/// <summary>Constructor</summary>
	/// <param name="image">Image to use in the ImageView</param>
	/// <param name="components">Specifies a set of remappings of color components</param>
	ImageViewCreateInfo(const Image& image, const ComponentMapping& components = ComponentMapping()) : _image(image), _components(components), _flags(ImageViewCreateFlags::e_NONE)
	{
		_viewType = convertToPVRVkImageViewType(_image->getImageType(), _image->getNumArrayLayers(), _image->isCubeMap());
		_format = _image->getFormat();
		_subresourceRange = ImageSubresourceRange(pvrvk::formatToImageAspect(_image->getFormat()), 0, _image->getNumMipLevels(), 0, _image->getNumArrayLayers());
	}

	/// <summary>Constructor</summary>
	/// <param name="image">Image to use in the ImageView</param>
	/// <param name="viewType">The image view type to use in the image view</param>
	/// <param name="format">The format to use in the image view</param>
	/// <param name="subresourceRange">A set of mipmap levels and array layers to be accessible to the view</param>
	/// <param name="components">Specifies a set of remappings of color components</param>
	/// <param name="flags">A set of pvrvk::ImageViewCreateFlags controlling how the ImageView will be created</param>
	ImageViewCreateInfo(const Image& image, pvrvk::ImageViewType viewType, pvrvk::Format format, const ImageSubresourceRange& subresourceRange,
		const ComponentMapping& components = ComponentMapping(), ImageViewCreateFlags flags = ImageViewCreateFlags::e_NONE)
		: _image(image), _viewType(viewType), _format(format), _components(components), _subresourceRange(subresourceRange), _flags(flags)
	{}

	/// <summary>Get Image view creation Flags</summary>
	/// <returns>Image view creation flags (ImageViewCreateFlags)</returns>
	inline ImageViewCreateFlags getFlags() const { return _flags; }
	/// <summary>Set PVRVk Image view creation flags</summary>
	/// <param name="flags">Flags to use for creating a PVRVk image view</param>
	inline void setFlags(ImageViewCreateFlags flags) { this->_flags = flags; }
	/// <summary>Get Image</summary>
	/// <returns>The Image used in the image view</returns>
	inline Image& getImage() { return _image; }
	/// <summary>Get Image</summary>
	/// <returns>The Image used in the image view</returns>
	inline const Image& getImage() const { return _image; }
	/// <summary>Set PVRVk Image view creation image</summary>
	/// <param name="image">Image to use for creating a PVRVk image view</param>
	inline void setImage(const Image& image) { this->_image = image; }
	/// <summary>Get Image view creation type</summary>
	/// <returns>Image view creation image view type (ImageViewType)</returns>
	inline ImageViewType getViewType() const { return _viewType; }
	/// <summary>Set PVRVk Image view creation view type</summary>
	/// <param name="viewType">Flags to use for creating a PVRVk image view</param>
	inline void setViewType(ImageViewType viewType) { this->_viewType = viewType; }
	/// <summary>Get Image view format</summary>
	/// <returns>Image view format (Format)</returns>
	inline Format getFormat() const { return _format; }
	/// <summary>Set the Image view format for PVRVk Image creation</summary>
	/// <param name="format">The image view format to use for creating a PVRVk image</param>
	inline void setFormat(Format format) { this->_format = format; }
	/// <summary>Get Image view components</summary>
	/// <returns>The Image view components used for remapping color components</returns>
	inline const ComponentMapping& getComponents() const { return _components; }
	/// <summary>Set PVRVk Image view creation components</summary>
	/// <param name="components">Specifies a set of remappings of color components</param>
	inline void setComponents(const ComponentMapping& components) { this->_components = components; }
	/// <summary>Get Image view sub resource range</summary>
	/// <returns>The Image view sub resource range used for selecting mipmap levels and array layers to be accessible to the view</returns>
	inline const ImageSubresourceRange& getSubresourceRange() const { return _subresourceRange; }
	/// <summary>Set PVRVk Image view creation sub resource range</summary>
	/// <param name="subresourceRange">Selects the set of mipmap levels and array layers to be accessible to the view</param>
	inline void setSubresourceRange(const ImageSubresourceRange& subresourceRange) { this->_subresourceRange = subresourceRange; }

private:
	/// <summary>The image to use in the image view</summary>
	Image _image;
	/// <summary>The type of image view to create</summary>
	ImageViewType _viewType;
	/// <summary>The format describing the format and type used to interpret data elements in the image</summary>
	Format _format;
	/// <summary>Specifies a set of remappings of color components</summary>
	ComponentMapping _components;
	/// <summary>Selects the set of mipmap levels and array layers to be accessible to the view</summary>
	ImageSubresourceRange _subresourceRange;
	/// <summary>Flags to use for creating the image view</summary>
	ImageViewCreateFlags _flags;
};

namespace impl {
/// <summary>pvrvk::ImageView implementation of a Vulkan VkImageView</summary>
class ImageView_ : public PVRVkDeviceObjectBase<VkImageView, ObjectType::e_IMAGE_VIEW>, public DeviceObjectDebugUtils<ImageView_>
{
private:
	friend class Device_;

	/// <summary>A class which restricts the creation of a pvrvk::ImageView to children or friends of a pvrvk::impl::ImageView_.</summary>
	class make_shared_enabler
	{
	protected:
		/// <summary>Constructor for a make_shared_enabler.</summary>
		make_shared_enabler() {}
		friend class ImageView_;
	};

	/// <summary>Protected function used to create a pvrvk::ImageView. Note that this function shouldn't normally be called
	/// directly and will be called by a friend of ImageView_ which will generally be a Device</summary>
	/// <param name="device">The device used to allocate the ImageView.</param>
	/// <param name="createInfo">The creation info structure which defines how the ImageView will be created.</param>
	/// <returns>Returns a successfully created pvrvk::ImageView</returns>
	static ImageView constructShared(const DeviceWeakPtr& device, const ImageViewCreateInfo& createInfo)
	{
		return std::make_shared<ImageView_>(make_shared_enabler{}, device, createInfo);
	}

	/// <summary>Creation information used when creating the image view.</summary>
	ImageViewCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(ImageView_)
	ImageView_(make_shared_enabler, const DeviceWeakPtr& device, const ImageViewCreateInfo& createInfo);

	~ImageView_();
	//!\endcond

	/// <summary>Get Image view creation Flags</summary>
	/// <returns>Image view creation flags (ImageViewCreateFlags)</returns>
	inline ImageViewCreateFlags getFlags() const { return _createInfo.getFlags(); }
	/// <summary>Get Image</summary>
	/// <returns>The Image used in the image view</returns>
	inline Image& getImage() { return _createInfo.getImage(); }
	/// <summary>Get Image</summary>
	/// <returns>The Image used in the image view</returns>
	inline const Image& getImage() const { return _createInfo.getImage(); }
	/// <summary>Get Image view creation type</summary>
	/// <returns>Image view creation image view type (ImageViewType)</returns>
	inline ImageViewType getViewType() const { return _createInfo.getViewType(); }
	/// <summary>Get Image view format</summary>
	/// <returns>Image view format (Format)</returns>
	inline Format getFormat() const { return _createInfo.getFormat(); }
	/// <summary>Get Image view components</summary>
	/// <returns>The Image view components used for remapping color components</returns>
	inline const ComponentMapping& getComponents() const { return _createInfo.getComponents(); }
	/// <summary>Get Image view sub resource range</summary>
	/// <returns>The Image view sub resource range used for selecting mipmap levels and array layers to be accessible to the view</returns>
	inline const ImageSubresourceRange& getSubresourceRange() const { return _createInfo.getSubresourceRange(); }
	/// <summary>Get this images view's create flags</summary>
	/// <returns>BufferViewCreateInfo</returns>
	ImageViewCreateInfo getCreateInfo() const { return _createInfo; }
};
} // namespace impl
} // namespace pvrvk
