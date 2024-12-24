/// @brief Include to use cube map textures.
/// @file gli/texture_cube.hpp

#pragma once

#include "texture2d.hpp"

namespace gli
{
	/// Cube map texture
	class texture_cube : public texture
	{
	public:
		typedef extent2d extent_type;

	public:
		/// Create an empty texture cube
		texture_cube();

		/// Create a texture_cube and allocate a new storage_linear
		texture_cube(
			format_type Format,
			extent_type const & Extent,
			size_type Levels,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture_cube and allocate a new storage_linear with a complete mipmap chain
		texture_cube(
			format_type Format,
			extent_type const & Extent,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture_cube view with an existing storage_linear
		explicit texture_cube(
			texture const& Texture);

		/// Create a texture_cube view with an existing storage_linear
		texture_cube(
			texture const& Texture,
			format_type Format,
			size_type BaseLayer, size_type MaxLayer,
			size_type BaseFace, size_type MaxFace,
			size_type BaseLevel, size_type MaxLevel,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture_cube view, reference a subset of an existing texture_cube instance
		texture_cube(
			texture_cube const& Texture,
			size_type BaseFace, size_type MaxFace,
			size_type BaseLevel, size_type MaxLevel);

		/// Create a view of the texture identified by Face in the texture cube
		texture2d operator[](size_type Face) const;

		/// Return the dimensions of a texture instance: width and height where both should be equal.
		extent_type extent(size_type Level = 0) const;

		/// Fetch a texel from a texture. The texture format must be uncompressed.
		template <typename gen_type>
		gen_type load(extent_type const& TexelCoord, size_type Face, size_type Level) const;

		/// Write a texel to a texture. The texture format must be uncompressed.
		template <typename gen_type>
		void store(extent_type const& TexelCoord, size_type Face, size_type Level, gen_type const& Texel);
	};
}//namespace gli

#include "./core/texture_cube.inl"
