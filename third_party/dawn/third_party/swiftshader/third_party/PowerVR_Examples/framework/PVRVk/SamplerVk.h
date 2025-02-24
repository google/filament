/*!
\brief The PVRVk Sampler class.
\file PVRVk/SamplerVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
/// <summary>Object describing the state of a sampler. Also used to construct a full-blown Sampler object.</summary>
struct SamplerCreateInfo
{
	/// <summary>Default constructor.</summary>
	/// <summary>Creates a Sampler with following configs. Can be edited after construction magnification filter: nearest,
	/// minifictation filter: nearest, mipmap filter: linear,wrap UVW: Repeat no comparison, no lod-bias, no
	/// anisotropy.</summary>
	SamplerCreateInfo()
		: magFilter(pvrvk::Filter::e_LINEAR), minFilter(pvrvk::Filter::e_NEAREST), mipMapMode(pvrvk::SamplerMipmapMode::e_LINEAR), wrapModeU(pvrvk::SamplerAddressMode::e_REPEAT),
		  wrapModeV(pvrvk::SamplerAddressMode::e_REPEAT), wrapModeW(pvrvk::SamplerAddressMode::e_REPEAT), compareOp(pvrvk::CompareOp::e_NEVER), compareOpEnable(false),
		  enableAnisotropy(false), anisotropyMaximum(1.0f), lodBias(0.0f), lodMinimum(0.0f), lodMaximum(100.0f), unnormalizedCoordinates(false),
		  borderColor(pvrvk::BorderColor::e_FLOAT_TRANSPARENT_BLACK)
	{}

	/// <summary>Constructor that sets the filters. Set the filtering mode explicitly. Wrap. Creates a Sampler with
	/// wrap:Repeat, no comparison, no lod bias, no anisotropy. Can be edited after construction.</summary>
	/// <param name="miniFilter">Minification filter: Nearest or Linear</param>
	/// <param name="magniFilter">Magnification filter: Nearest or Linear</param>
	/// <param name="mipMapFilter">Mipmap interpolation filter: Nearest or Linear</param>
	/// <param name="wrapModeU">The wrapping mode for the U coordinate</param>
	/// <param name="wrapModeV">The wrapping mode for the V coordinate</param>
	/// <param name="wrapModeW">The wrapping mode for the W coordinate</param>
	SamplerCreateInfo(pvrvk::Filter magniFilter, pvrvk::Filter miniFilter, pvrvk::SamplerMipmapMode mipMapFilter = pvrvk::SamplerMipmapMode::e_LINEAR,
		pvrvk::SamplerAddressMode wrapModeU = pvrvk::SamplerAddressMode::e_REPEAT, pvrvk::SamplerAddressMode wrapModeV = pvrvk::SamplerAddressMode::e_REPEAT,
		pvrvk::SamplerAddressMode wrapModeW = pvrvk::SamplerAddressMode::e_REPEAT)
		: magFilter(magniFilter), minFilter(miniFilter), mipMapMode(mipMapFilter), wrapModeU(wrapModeU), wrapModeV(wrapModeV), wrapModeW(wrapModeW),
		  compareOp(pvrvk::CompareOp::e_NEVER), compareOpEnable(false), enableAnisotropy(false), anisotropyMaximum(1.0f), lodBias(0.0f), lodMinimum(0.0f), lodMaximum(100.0f),
		  unnormalizedCoordinates(false), borderColor(pvrvk::BorderColor::e_FLOAT_TRANSPARENT_BLACK)
	{}

	pvrvk::Filter magFilter; //!< Texture Magnification interpolation filter. Nearest or Linear. Default Nearest.
	pvrvk::Filter minFilter; //!< Texture Minification interpolation filter. Nearest or Linear. Default Nearest.
	pvrvk::SamplerMipmapMode mipMapMode; //!< Texture Mipmap interpolation filter. Nearest, Linear or None (for no mipmaps). Default Linear.
	pvrvk::SamplerAddressMode wrapModeU; //!< Texture Wrap mode, U (x) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	pvrvk::SamplerAddressMode wrapModeV; //!< Texture Wrap mode, V (y) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	pvrvk::SamplerAddressMode wrapModeW; //!< Texture Wrap mode, W (z) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	pvrvk::CompareOp compareOp; //!< Comparison mode for comparison samplers. Default None.
	bool compareOpEnable; //!< Enable the compare op
	bool enableAnisotropy; //!< Enable anisotropic filtering
	float anisotropyMaximum; //!< Texture anisotropic filtering. Default 1.
	float lodBias; //!< Texture level-of-detail bias (bias of mipmap to select). Default 0.
	float lodMinimum; //!< Texture minimum level-of-detail (mipmap). Default 0.
	float lodMaximum; //!< Texture maximum level-of-detail (mipmap). Default 0.
	bool unnormalizedCoordinates; //!< Texture Coordinates are Un-Normalized
	pvrvk::BorderColor borderColor; //!< In case of a border address mode, the border color
};

namespace impl {
/// <summary>SamplerVk_ implementation that wraps the vulkan sampler</summary>
class Sampler_ : public PVRVkDeviceObjectBase<VkSampler, ObjectType::e_SAMPLER>, public DeviceObjectDebugUtils<Sampler_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Sampler_;
	};

	static Sampler constructShared(const DeviceWeakPtr& device, const SamplerCreateInfo& createInfo)
	{
		return std::make_shared<Sampler_>(make_shared_enabler{}, device, createInfo);
	}

	SamplerCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Sampler_)
	Sampler_(make_shared_enabler, const DeviceWeakPtr& device, const pvrvk::SamplerCreateInfo& desc);

	/// <summary>destructor</summary>
	~Sampler_();
	//!\endcond

	/// <summary>Get sampler create info (const)</summary>
	/// <returns>SamplerCreateInfo&</returns>
	const SamplerCreateInfo& getCreateInfo() const { return _createInfo; }
};
} // namespace impl
} // namespace pvrvk
