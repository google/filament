namespace gli
{
	inline gli::texture make_texture1d(format Format, extent1d const& Extent, size_t Levels)
	{
		return gli::texture(TARGET_1D, Format, texture::extent_type(Extent.x, 1, 1), 1, 1, Levels);
	}

	inline gli::texture make_texture1d(format Format, extent1d const& Extent)
	{
		return gli::texture(TARGET_1D, Format, texture::extent_type(Extent.x, 1, 1), 1, 1, gli::levels(texture::extent_type(Extent.x, 1, 1)));
	}

	inline gli::texture make_texture1d_array(format Format, extent1d const& Extent, size_t Layers, size_t Levels)
	{
		return gli::texture(TARGET_1D_ARRAY, Format, texture::extent_type(Extent.x, 1, 1), Layers, 1, Levels);
	}

	inline gli::texture make_texture1d_array(format Format, extent1d const& Extent, size_t Layers)
	{
		return gli::texture(TARGET_1D_ARRAY, Format, texture::extent_type(Extent.x, 1, 1), Layers, 1, gli::levels(texture::extent_type(Extent.x, 1, 1)));
	}

	inline gli::texture make_texture2d(format Format, extent2d const& Extent, size_t Levels)
	{
		return gli::texture(TARGET_2D, Format, texture::extent_type(Extent, 1), 1, 1, Levels);
	}

	inline gli::texture make_texture2d(format Format, extent2d const& Extent)
	{
		return gli::texture(TARGET_2D, Format, texture::extent_type(Extent, 1), 1, 1, gli::levels(texture::extent_type(Extent, 1)));
	}

	inline gli::texture make_texture2d_array(format Format, extent2d const& Extent, size_t Layer, size_t Levels)
	{
		return gli::texture(TARGET_2D_ARRAY, Format, texture::extent_type(Extent, 1), Layer, 1, Levels);
	}

	inline gli::texture make_texture2d_array(format Format, extent2d const& Extent, size_t Layer)
	{
		return gli::texture(TARGET_2D_ARRAY, Format, texture::extent_type(Extent, 1), Layer, 1, gli::levels(texture::extent_type(Extent, 1)));
	}

	inline gli::texture make_texture3d(format Format, extent3d const& Extent, size_t Levels)
	{
		return gli::texture(TARGET_3D, Format, texture::extent_type(Extent), 1, 1, Levels);
	}

	inline gli::texture make_texture3d(format Format, extent3d const& Extent)
	{
		return gli::texture(TARGET_3D, Format, texture::extent_type(Extent), 1, 1, gli::levels(texture::extent_type(Extent)));
	}

	inline gli::texture make_texture_cube(format Format, extent2d const& Extent, size_t Levels)
	{
		return gli::texture(TARGET_CUBE, Format, texture::extent_type(Extent, 1), 1, 6, Levels);
	}

	inline gli::texture make_texture_cube(format Format, extent2d const& Extent)
	{
		return gli::texture(TARGET_CUBE, Format, texture::extent_type(Extent, 1), 1, 6, gli::levels(texture::extent_type(Extent, 1)));
	}

	inline gli::texture make_texture_cube_array(format Format, extent2d const& Extent, size_t Layer, size_t Levels)
	{
		return gli::texture(TARGET_CUBE_ARRAY, Format, texture::extent_type(Extent, 1), Layer, 6, Levels);
	}

	inline gli::texture make_texture_cube_array(format Format, extent2d const& Extent, size_t Layer)
	{
		return gli::texture(TARGET_CUBE_ARRAY, Format, texture::extent_type(Extent, 1), Layer, 6, gli::levels(texture::extent_type(Extent, 1)));
	}
}//namespace gli
