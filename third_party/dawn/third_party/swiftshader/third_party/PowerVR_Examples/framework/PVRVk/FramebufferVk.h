/*!
\brief PVRVk Framebuffer Object class
\file PVRVk/FramebufferVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "ImageVk.h"

namespace pvrvk {

/// <summary>Framebuffer creation descriptor.</summary>
struct FramebufferCreateInfo
{
public:
	friend class impl::Framebuffer_;

	/// <summary>Reset this object</summary>
	void clear()
	{
		_width = 0;
		_height = 0;
		_layers = 1;
		_renderPass.reset();
		for (uint32_t i = 0; i < total_max_attachments; ++i) { _attachments[i].reset(); }
	}

	/// <summary>Constructor (zero initialization)</summary>
	FramebufferCreateInfo() : _numAttachments(0), _layers(1), _width(0), _height(0) {}

	/// <summary>Constructor (zero initialization)</summary>
	/// <param name="width">The width of the framebuffer to create</param>
	/// <param name="height">The height of the framebuffer to create</param>
	/// <param name="layers">The number of array layers which will be used by the framebuffer to create</param>
	/// <param name="renderPass">The renderPass which will be used by the framebuffer</param>
	/// <param name="numAttachments">The number of attachments used by the framebuffer to create</param>
	/// <param name="attachments">The numAttachments size list of pvrvk::ImageView elements to be used as framebuffer attachments</param>
	FramebufferCreateInfo(uint32_t width, uint32_t height, uint32_t layers, const RenderPass& renderPass, uint32_t numAttachments = 0, ImageView* attachments = nullptr)
		: _numAttachments(numAttachments), _layers(layers), _width(width), _height(height), _renderPass(renderPass)
	{
		for (uint32_t i = 0; i < numAttachments; ++i) { _attachments[i] = attachments[i]; }
	}

	/// <summary>Return number of color attachment</summary>
	/// <returns>Number of color attachments</returns>
	uint32_t getNumAttachments() const { return _numAttachments; }

	/// <summary>Get a color attachment TextureView</summary>
	/// <param name="index">The index of the Colorattachment to retrieve</param>
	/// <returns>This object</returns>
	const ImageView& getAttachment(uint32_t index) const
	{
		assert(index < _numAttachments && "Invalid attachment index");
		return _attachments[index];
	}
	/// <summary>Get a color attachment TextureView</summary>
	/// <param name="index">The index of the Colorattachment to retrieve</param>
	/// <returns>This object</returns>
	ImageView& getAttachment(uint32_t index)
	{
		assert(index < _numAttachments && "Invalid attachment index");
		return _attachments[index];
	}

	/// <summary>Get the RenderPass (const)</summary>
	/// <returns>The RenderPass (const)</summary>
	const RenderPass& getRenderPass() const { return _renderPass; }

	/// <summary>Get the RenderPass</summary>
	/// <returns>The RenderPass</summary>
	RenderPass& getRenderPass() { return _renderPass; }

	/// <summary>Get the dimensions of the framebuffer</summary>
	/// <returns>The framebuffer dimensions</returns>
	Extent2D getDimensions() const { return Extent2D(_width, _height); }

	/// <summary>Set the framebuffer dimension</summary>
	/// <param name="inWidth">Width</param>
	/// <param name="inHeight">Height</param>
	/// <returns>This object (allow chaining)</returns>
	FramebufferCreateInfo& setDimensions(uint32_t inWidth, uint32_t inHeight)
	{
		this->_width = inWidth;
		this->_height = inHeight;
		return *this;
	}

	/// <summary>Set the framebuffer dimension</summary>
	/// <param name="extent">dimension</param>
	/// <returns>This object (allow chaining)</returns>
	FramebufferCreateInfo& setDimensions(const Extent2D& extent)
	{
		_width = extent.getWidth();
		_height = extent.getHeight();
		return *this;
	}

	/// <summary>Add a color attachment to a specified attachment point.</summary>
	/// <param name="index">The attachment point, the index must be consecutive</param>
	/// <param name="colorView">The color attachment</param>
	/// <returns>this (allow chaining)</returns>
	FramebufferCreateInfo& setAttachment(uint32_t index, const ImageView& colorView)
	{
		assert(index < total_max_attachments && "Index out-of-bound");
		if (index >= _numAttachments) { _numAttachments = index + 1; }
		this->_attachments[index] = colorView;
		return *this;
	}

	/// <summary>Get Layers</summary>
	/// <returns>Layers</returns>
	inline uint32_t getLayers() const { return _layers; }

	/// <summary>Set the number of layers.</summary>
	/// <param name="numLayers">The number of array_layers.</param>
	/// <returns>this (allow chaining)</returns>
	FramebufferCreateInfo& setNumLayers(uint32_t numLayers)
	{
		_layers = numLayers;
		return *this;
	}

	/// <summary>Set the RenderPass which this Framebuffer will be invoking when bound.</summary>
	/// <param name="_renderPass">A renderpass. When binding this Framebuffer, this renderpass will be the one to be bound.</param>
	/// <returns>this (allow chaining)</returns>
	FramebufferCreateInfo& setRenderPass(const RenderPass& renderPass)
	{
		this->_renderPass = renderPass;
		return *this;
	}

private:
	enum
	{
		total_max_attachments = FrameworkCaps::MaxColorAttachments + FrameworkCaps::MaxDepthStencilAttachments
	};
	ImageView _attachments[total_max_attachments];
	uint32_t _numAttachments;

	/// <summary>The number of array layers of the Framebuffer</summary>
	uint32_t _layers;
	/// <summary>The width (in pixels) of the Framebuffer</summary>
	uint32_t _width;
	/// <summary>The hight (in pixels) of the Framebuffer</summary>
	uint32_t _height;
	/// <summary>The render pass that this Framebuffer will render in</summary>
	RenderPass _renderPass;
};

namespace impl {
/// <summary>Vulkan implementation of the Framebuffer (Framebuffer object) class.</summary>
class Framebuffer_ : public PVRVkDeviceObjectBase<VkFramebuffer, ObjectType::e_FRAMEBUFFER>, public DeviceObjectDebugUtils<Framebuffer_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Framebuffer_;
	};

	static Framebuffer constructShared(const DeviceWeakPtr& device, const FramebufferCreateInfo& createInfo)
	{
		return std::make_shared<Framebuffer_>(make_shared_enabler{}, device, createInfo);
	}

	FramebufferCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Framebuffer_)
	Framebuffer_(make_shared_enabler, const DeviceWeakPtr& device, const FramebufferCreateInfo& createInfo);
	~Framebuffer_();
	//!\endcond

	/// <summary>Return the renderpass that this framebuffer uses.</summary>
	/// <returns>The renderpass that this Framebuffer uses.</returns>
	const RenderPass& getRenderPass() const { return _createInfo._renderPass; }

	/// <summary>Return this object create param(const)</summary>
	/// <returns>const FramebufferCreateInfo&</returns>
	const FramebufferCreateInfo& getCreateInfo() const { return _createInfo; }

	/// <summary>Get the Dimension of this framebuffer(const)</summary>
	/// <returns>Framebuffer Dimension</returns>
	Extent2D getDimensions() const { return _createInfo.getDimensions(); }

	/// <summary>Get the color attachment at a specific index</summary>
	/// <param name="index">A color attachment index.</param>
	/// <returns>The Texture that is bound as a color attachment at index.</returns>
	const ImageView& getAttachment(uint32_t index) const { return _createInfo.getAttachment(index); }

	/// <summary>Get the color attachment at a specific index</summary>
	/// <param name="index">A color attachment index.</param>
	/// <returns>The Texture that is bound as a color attachment at index.</returns>
	ImageView& getAttachment(uint32_t index) { return _createInfo.getAttachment(index); }

	/// <summary>getNumAttachments</summary>
	/// <returns>Get number of attachments</returns>
	uint32_t getNumAttachments() const { return _createInfo.getNumAttachments(); }
};
} // namespace impl
} // namespace pvrvk
