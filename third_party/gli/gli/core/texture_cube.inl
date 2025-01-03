namespace gli
{
	inline texture_cube::texture_cube()
	{}

	inline texture_cube::texture_cube(format_type Format, extent_type const& Extent, swizzles_type const& Swizzles)
		: texture(TARGET_CUBE, Format, texture::extent_type(Extent, 1), 1, 6, gli::levels(Extent), Swizzles)
	{}

	inline texture_cube::texture_cube(format_type Format, extent_type const& Extent, size_type Levels, swizzles_type const& Swizzles)
		: texture(TARGET_CUBE, Format, texture::extent_type(Extent, 1), 1, 6, Levels, Swizzles)
	{}

	inline texture_cube::texture_cube(texture const& Texture)
		: texture(Texture, TARGET_CUBE, Texture.format())
	{}

	inline texture_cube::texture_cube
	(
		texture const& Texture,
		format_type Format,
		size_type BaseLayer, size_type MaxLayer,
		size_type BaseFace, size_type MaxFace,
		size_type BaseLevel, size_type MaxLevel,
		swizzles_type const& Swizzles
	)
		: texture(
			Texture, TARGET_CUBE, Format,
			BaseLayer, MaxLayer,
			BaseFace, MaxFace,
			BaseLevel, MaxLevel,
			Swizzles)
	{}

	inline texture_cube::texture_cube
	(
		texture_cube const& Texture,
		size_type BaseFace, size_type MaxFace,
		size_type BaseLevel, size_type MaxLevel
	)
		: texture(
			Texture, TARGET_CUBE, Texture.format(),
			Texture.base_layer(), Texture.max_layer(),
			Texture.base_face() + BaseFace, Texture.base_face() + MaxFace,
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel)
	{}

	inline texture2d texture_cube::operator[](size_type Face) const
	{
		GLI_ASSERT(Face < this->faces());

		return texture2d(
			*this, this->format(),
			this->base_layer(), this->max_layer(),
			this->base_face() + Face, this->base_face() + Face,
			this->base_level(), this->max_level());
	}

	inline texture_cube::extent_type texture_cube::extent(size_type Level) const
	{
		return extent_type(this->texture::extent(Level));
	}

	template <typename gen_type>
	inline gen_type texture_cube::load(extent_type const& TexelCoord, size_type Face, size_type Level) const
	{
		return this->texture::load<gen_type>(texture::extent_type(TexelCoord, 0), 0, Face, Level);
	}

	template <typename gen_type>
	inline void texture_cube::store(extent_type const& TexelCoord, size_type Face, size_type Level, gen_type const& Texel)
	{
		this->texture::store<gen_type>(texture::extent_type(TexelCoord, 0), 0, Face, Level, Texel);
	}
}//namespace gli
