/// @brief Include to use 3d textures.
/// @file gli/texture3d.hpp

#pragma once

#include "texture.hpp"
#include "image.hpp"

namespace gli
{
	/// 3d texture
	class texture3d : public texture
	{
	public:
		typedef extent3d extent_type;

	public:
		/// Create an empty texture 3D
		texture3d();

		/// Create a texture3d and allocate a new storage_linear
		texture3d(
			format_type Format,
			extent_type const& Extent,
			size_type Levels,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture3d and allocate a new storage_linear with a complete mipmap chain
		texture3d(
			format_type Format,
			extent_type const& Extent,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture3d view with an existing storage_linear
		explicit texture3d(
			texture const & Texture);

		/// Create a texture3d view with an existing storage_linear
		texture3d(
			texture const& Texture,
			format_type Format,
			size_type BaseLayer, size_type MaxLayer,
			size_type BaseFace, size_type MaxFace,
			size_type BaseLevel, size_type MaxLevel,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture3d view, reference a subset of an existing texture3d instance
		texture3d(
			texture3d const & Texture,
			size_type BaseLevel, size_type MaxLevel);

		/// Create a view of the image identified by Level in the mipmap chain of the texture
		image operator[](size_type Level) const;

		/// Return the dimensions of a texture instance: width, height and depth 
		extent_type extent(size_type Level = 0) const;

		/// Fetch a texel from a texture. The texture format must be uncompressed.
		template <typename gen_type>
		gen_type load(extent_type const& TexelCoord, size_type Level) const;

		/// Write a texel to a texture. The texture format must be uncompressed.
		template <typename gen_type>
		void store(extent_type const& TexelCoord, size_type Level, gen_type const& Texel);
	};
}//namespace gli

#include "./core/texture3d.inl"
