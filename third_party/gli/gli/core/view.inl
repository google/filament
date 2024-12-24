namespace gli
{
	inline image view(image const& Image)
	{
		return Image;
	}

	inline texture view(texture const& Texture)
	{
		return Texture;
	}

	template <typename texType>
	inline texture view(texType const& Texture)
	{
		return Texture;
	}

	inline texture view
	(
		texture const& Texture,
		texture::size_type BaseLayer, texture::size_type MaxLayer,
		texture::size_type BaseFace, texture::size_type MaxFace,
		texture::size_type BaseLevel, texture::size_type MaxLevel
	)
	{
		GLI_ASSERT(!Texture.empty());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < Texture.levels() && MaxLevel >= 0 && MaxLevel < Texture.levels() && BaseLevel <= MaxLevel);
		GLI_ASSERT(BaseFace >= 0 && BaseFace < Texture.faces() && MaxFace >= 0 && MaxFace < Texture.faces() && BaseFace <= MaxFace);
		GLI_ASSERT(BaseLayer >= 0 && BaseLayer < Texture.layers() && MaxLayer >= 0 && MaxLayer < Texture.layers() && BaseLayer <= MaxLayer);

		return texture(
			Texture, Texture.target(), Texture.format(),
			Texture.base_layer() + BaseLayer, Texture.base_layer() + MaxLayer,
			Texture.base_face() + BaseFace, Texture.base_face() + MaxFace,
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel);
	}

	template <typename texType>
	inline texture view(texType const& Texture, format Format)
	{
		GLI_ASSERT(!Texture.empty());
		GLI_ASSERT(block_size(Texture.format()) == block_size(Format));

		return texture(Texture, Texture.target(), Format);
	}

	inline texture view
	(
		texture1d const& Texture,
		texture1d::size_type BaseLevel, texture1d::size_type MaxLevel
	)
	{
		GLI_ASSERT(!Texture.empty());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < Texture.levels() && MaxLevel >= 0 && MaxLevel < Texture.levels() && BaseLevel <= MaxLevel);

		return texture(
			Texture, TARGET_1D, Texture.format(),
			Texture.base_layer(), Texture.max_layer(),
			Texture.base_face(), Texture.max_face(),
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel);
	}

	inline texture view
	(
		texture1d_array const & Texture,
		texture1d_array::size_type BaseLayer, texture1d_array::size_type MaxLayer,
		texture1d_array::size_type BaseLevel, texture1d_array::size_type MaxLevel
	)
	{
		GLI_ASSERT(!Texture.empty());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < Texture.levels() && MaxLevel >= 0 && MaxLevel < Texture.levels() && BaseLevel <= MaxLevel);
		GLI_ASSERT(BaseLayer >= 0 && BaseLayer < Texture.layers() && MaxLayer >= 0 && MaxLayer < Texture.layers() && BaseLayer <= MaxLayer);

		return texture(
			Texture, TARGET_1D_ARRAY, Texture.format(),
			Texture.base_layer() + BaseLayer, Texture.base_layer() + MaxLayer,
			Texture.base_face(), Texture.max_face(),
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel);
	}

	inline texture view
	(
		texture2d const & Texture,
		texture2d::size_type BaseLevel, texture2d::size_type MaxLevel
	)
	{
		GLI_ASSERT(!Texture.empty());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < Texture.levels() && MaxLevel >= 0 && MaxLevel < Texture.levels() && BaseLevel <= MaxLevel);

		return texture(
			Texture, TARGET_2D, Texture.format(),
			Texture.base_layer(), Texture.max_layer(),
			Texture.base_face(), Texture.max_face(),
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel);
	}

	inline texture view
	(
		texture2d_array const & Texture,
		texture2d_array::size_type BaseLayer, texture2d_array::size_type MaxLayer,
		texture2d_array::size_type BaseLevel, texture2d_array::size_type MaxLevel
	)
	{
		GLI_ASSERT(!Texture.empty());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < Texture.levels() && MaxLevel >= 0 && MaxLevel < Texture.levels() && BaseLevel <= MaxLevel);
		GLI_ASSERT(BaseLayer >= 0 && BaseLayer < Texture.layers() && MaxLayer >= 0 && MaxLayer < Texture.layers() && BaseLayer <= MaxLayer);

		return texture(
			Texture, TARGET_2D_ARRAY, Texture.format(),
			Texture.base_layer() + BaseLayer, Texture.base_layer() + MaxLayer,
			Texture.base_face(), Texture.max_face(),
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel);
	}

	inline texture view
	(
		texture3d const & Texture,
		texture3d::size_type BaseLevel, texture3d::size_type MaxLevel
	)
	{
		GLI_ASSERT(!Texture.empty());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < Texture.levels() && MaxLevel >= 0 && MaxLevel < Texture.levels() && BaseLevel <= MaxLevel);

		return texture(
			Texture, TARGET_3D, Texture.format(),
			Texture.base_layer(), Texture.max_layer(),
			Texture.base_face(), Texture.max_face(),
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel);
	}

	inline texture view
	(
		texture_cube const & Texture,
		texture_cube::size_type BaseFace, texture_cube::size_type MaxFace,
		texture_cube::size_type BaseLevel, texture_cube::size_type MaxLevel
	)
	{
		GLI_ASSERT(!Texture.empty());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < Texture.levels() && MaxLevel >= 0 && MaxLevel < Texture.levels() && BaseLevel <= MaxLevel);
		GLI_ASSERT(BaseFace >= 0 && BaseFace < Texture.faces() && MaxFace >= 0 && MaxFace < Texture.faces() && BaseFace <= MaxFace);

		return texture(
			Texture, TARGET_CUBE, Texture.format(),
			Texture.base_layer(), Texture.max_layer(),
			Texture.base_face(), Texture.base_face() + MaxFace,
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel);
	}

	inline texture view
	(
		texture_cube_array const & Texture,
		texture_cube_array::size_type BaseLayer, texture_cube_array::size_type MaxLayer,
		texture_cube_array::size_type BaseFace, texture_cube_array::size_type MaxFace,
		texture_cube_array::size_type BaseLevel, texture_cube_array::size_type MaxLevel
	)
	{
		GLI_ASSERT(!Texture.empty());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < Texture.levels() && MaxLevel >= 0 && MaxLevel < Texture.levels() && BaseLevel <= MaxLevel);
		GLI_ASSERT(BaseFace >= 0 && BaseFace < Texture.faces() && MaxFace >= 0 && MaxFace < Texture.faces() && BaseFace <= MaxFace);
		GLI_ASSERT(BaseLayer >= 0 && BaseLayer < Texture.layers() && MaxLayer >= 0 && MaxLayer < Texture.layers() && BaseLayer <= MaxLayer);

		return texture(
			Texture, TARGET_CUBE_ARRAY, Texture.format(),
			Texture.base_layer() + BaseLayer, Texture.base_layer() + MaxLayer,
			Texture.base_face() + BaseFace, Texture.base_face() + MaxFace,
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel);
	}
}//namespace gli
