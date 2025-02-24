/*!
\brief The PVRVk RenderPass class.
\file PVRVk/RenderPassVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {

/// <summary>RenderPass creation parameters.</summary>
struct RenderPassCreateInfo
{
private:
	enum
	{
		total_max_attachments = static_cast<uint32_t>(FrameworkCaps::MaxColorAttachments) + static_cast<uint32_t>(FrameworkCaps::MaxDepthStencilAttachments)
	};
	friend class impl::RenderPass_;
	AttachmentDescription _attachmentDescriptions[total_max_attachments];
	std::vector<SubpassDescription> _subpass;
	std::vector<SubpassDependency> _subpassDependency;
	uint32_t _numAttachmentDescription;

public:
	/// <summary>RenderPassCreateInfo</summary>
	RenderPassCreateInfo() : _numAttachmentDescription(0) {}

	/// <summary>Clear all entries</summary>
	void clear()
	{
		_subpass.clear();
		_subpassDependency.clear();
	}

	/// <summary>Return number of subpasses (const)</summary>
	/// <returns>Number of subpasses</returns>
	uint32_t getNumSubpasses() const { return static_cast<uint32_t>(_subpass.size()); }

	/// <summary>Get subpass (const)</summary>
	/// <param name="index">Subpass index</param>
	/// <returns>Subpas</returns>
	const SubpassDescription& getSubpass(uint32_t index) const
	{
		assert(index < getNumSubpasses() && "Invalid subpass index");
		return _subpass[index];
	}

	/// <summary>Get number of subpass dependencies (const)</summary>
	/// <returns>Number of subpass dependencies</returns>
	uint32_t getNumSubpassDependencies() const { return static_cast<uint32_t>(_subpassDependency.size()); }

	/// <summary>Get subpass dependency (const)</summary>
	/// <param name="index">Subpass dependency index</param>
	/// <returns>SubpassDependency</returns>
	const SubpassDependency& getSubpassDependency(uint32_t index) const
	{
		assert(index < getNumSubpassDependencies() && "Invalid subpass dependency index");
		return _subpassDependency[index];
	}

	/// <summary>Return number of color attachments (const)</summary>
	/// <returns>Number of color attachments</returns>
	uint32_t getNumAttachmentDescription() const { return _numAttachmentDescription; }

	/// <summary>Get render pass color info (const)</summary>
	/// <param name="index">Color info index</param>
	/// <returns>RenderPassColorInfo</returns>
	const AttachmentDescription& getAttachmentDescription(uint32_t index) const
	{
		assert(index < getNumAttachmentDescription() && "Invalid color info index");
		return _attachmentDescriptions[index];
	}

	/// <summary>Add color info to the specified color attachment point.</summary>
	/// <param name="index">The color attachment point to add the color info, index must be consecutive</param>
	/// <param name="attachmentDescription">The attachment description to add to the attachment point</param>
	/// <returns>Reference to this object. (allow chaining)</returns>
	RenderPassCreateInfo& setAttachmentDescription(uint32_t index, const AttachmentDescription& attachmentDescription)
	{
		if (index >= total_max_attachments) { assert(false && "AttachmentDescription exceeds the max attachment limit"); }
		_numAttachmentDescription += static_cast<uint32_t>(this->_attachmentDescriptions[index].getFormat() == pvrvk::Format::e_UNDEFINED);
		this->_attachmentDescriptions[index] = attachmentDescription;
		return *this;
	}

	/// <summary>Add a subpass to this renderpass</summary>
	/// <param name="index">Index where to add the subpass, the index must be consective</param>
	/// <param name="subpass">The Subpass to add</param>
	/// <returns>Reference to this object. (allow chaining)</returns>
	RenderPassCreateInfo& setSubpass(uint32_t index, const SubpassDescription& subpass)
	{
		if (index >= this->_subpass.size()) { this->_subpass.resize(index + 1); }
		this->_subpass[index] = subpass;
		return *this;
	}

	/// <summary>Add a subpass dependecy to this renderpass</summary>
	/// <param name="subPassDependency">The Subpass dependency to add</param>
	/// <returns>Reference to this object. (allow chaining)</returns>
	RenderPassCreateInfo& addSubpassDependency(const SubpassDependency& subPassDependency)
	{
		if ((subPassDependency.getSrcSubpass() != pvrvk::SubpassExternal) && (subPassDependency.getSrcSubpass() > subPassDependency.getDstSubpass()))
		{ assert(false && " Source Sub pass must be less than or equal to destination Sub pass"); } _subpassDependency.emplace_back(subPassDependency);
		return *this;
	}

	/// <summary>Add subpass dependencies</summary>
	/// <param name="subPassDependencies">Pointer to supass dependencies</param>
	/// <param name="numDependencies">Number of supass dependencies</param>
	/// <returns>Return this.</returns>
	RenderPassCreateInfo& addSubpassDependencies(const SubpassDependency* subPassDependencies, uint32_t numDependencies)
	{
		for (uint32_t i = 0; i < numDependencies; ++i) { addSubpassDependency(subPassDependencies[i]); }
		return *this;
	}
};

namespace impl {

/// <summary>Vulkan implementation of the RenderPass class.
/// Use through the Reference counted framework object
/// pvrvk::api::RenderPass. RenderPass Compatibility: Framebuffers and graphics pipelines are created based on a
/// specific render pass object. They must only be used with that render pass object, or one compatible with it.
/// Two attachment references are compatible if they have matching format and sample count, Two arrays of
/// attachment references are compatible if all corresponding pairs of attachments are compatible. If the arrays
/// are of different lengths, attachment references not present in the smaller array are treated as unused. Two
/// render passes that contain only a single subpass are compatible if their corresponding color, input, resolve,
/// and depth/stencil attachment references are compatible. If two render passes contain more than one subpass,
/// they are compatible if they are identical except for: - Initial and final image layout in attachment
/// descriptions - Load and store operations in attachment descriptions - Image layout in attachment references A
/// framebuffer is compatible with a render pass if it was created using the same render pass or a compatible
/// render pass.</summary>
class RenderPass_ : public PVRVkDeviceObjectBase<VkRenderPass, ObjectType::e_RENDER_PASS>, public DeviceObjectDebugUtils<RenderPass_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() = default;
		friend class RenderPass_;
	};

	static RenderPass constructShared(const DeviceWeakPtr& device, const RenderPassCreateInfo& createInfo)
	{
		return std::make_shared<RenderPass_>(make_shared_enabler{}, device, createInfo);
	}

	RenderPassCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(RenderPass_)
	RenderPass_(make_shared_enabler, const DeviceWeakPtr& device, const RenderPassCreateInfo& createInfo);

	/// <summary>destructor</summary>
	~RenderPass_();
	//!\endcond

	/// <summary>getCreateInfo</summary>
	/// <returns></returns>
	const RenderPassCreateInfo& getCreateInfo() const { return _createInfo; }
};
} // namespace impl
} // namespace pvrvk
