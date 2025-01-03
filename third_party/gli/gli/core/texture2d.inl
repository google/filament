#include "../levels.hpp"

namespace gli
{
	inline texture2d::texture2d()
	{}

	inline texture2d::texture2d(format_type Format, extent_type const& Extent, swizzles_type const& Swizzles)
		: texture(TARGET_2D, Format, texture::extent_type(Extent, 1), 1, 1, gli::levels(Extent), Swizzles)
	{}

	inline texture2d::texture2d(format_type Format, extent_type const& Extent, size_type Levels, swizzles_type const& Swizzles)
		: texture(TARGET_2D, Format, texture::extent_type(Extent, 1), 1, 1, Levels, Swizzles)
	{}

	inline texture2d::texture2d(texture const& Texture)
		: texture(Texture, TARGET_2D, Texture.format())
	{}

	inline texture2d::texture2d
	(
		texture const& Texture,
		format_type Format,
		size_type BaseLayer, size_type MaxLayer,
		size_type BaseFace, size_type MaxFace,
		size_type BaseLevel, size_type MaxLevel,
		swizzles_type const& Swizzles
	)
		: texture(
			Texture, TARGET_2D, Format,
			BaseLayer, MaxLayer,
			BaseFace, MaxFace,
			BaseLevel, MaxLevel,
			Swizzles)
	{}

	inline texture2d::texture2d
	(
		texture2d const& Texture,
		size_type BaseLevel, size_type MaxLevel
	)
		: texture(
			Texture, TARGET_2D, Texture.format(),
			Texture.base_layer(), Texture.max_layer(),
			Texture.base_face(), Texture.max_face(),
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel)
	{}

	inline image texture2d::operator[](size_type Level) const
	{
		GLI_ASSERT(Level < this->levels());

		return image(
			this->Storage,
			this->format(),
			this->base_layer(),
			this->base_face(),
			this->base_level() + Level);
	}

	inline texture2d::extent_type texture2d::extent(size_type Level) const
	{
		return extent_type(this->texture::extent(Level));
	}

	template <typename gen_type>
	inline gen_type texture2d::load(extent_type const& TexelCoord, size_type Level) const
	{
		return this->texture::load<gen_type>(texture::extent_type(TexelCoord, 0), 0, 0, Level);
	}

	template <typename gen_type>
	inline void texture2d::store(extent_type const& TexelCoord, size_type Level, gen_type const& Texel)
	{
		this->texture::store<gen_type>(texture::extent_type(TexelCoord, 0), 0, 0, Level, Texel);
	}
}//namespace gli
