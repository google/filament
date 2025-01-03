/// @brief Include to use 2d textures.
/// @file gli/texture2d.hpp

#pragma once

#include "texture.hpp"
#include "image.hpp"

namespace gli
{
	/// 2d texture
	class texture2d : public texture
	{
	public:
		typedef extent2d extent_type;

		/// Create an empty texture 2D.
		texture2d();

		/// Create a texture2d and allocate a new storage_linear.
		texture2d(
			format_type Format,
			extent_type const& Extent,
			size_type Levels,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture2d and allocate a new storage_linear with a complete mipmap chain.
		texture2d(
			format_type Format,
			extent_type const& Extent,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture2d view with an existing storage_linear.
		explicit texture2d(
			texture const& Texture);

		/// Create a texture2d view with an existing storage_linear.
		texture2d(
			texture const& Texture,
			format_type Format,
			size_type BaseLayer, size_type MaxLayer,
			size_type BaseFace, size_type MaxFace,
			size_type BaseLevel, size_type MaxLevel,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture2d view, reference a subset of an existing texture2d instance.
		texture2d(
			texture2d const& Texture,
			size_type BaseLevel, size_type MaxLevel);

		/// Create a view of the image identified by Level in the mipmap chain of the texture.
		image operator[](size_type Level) const;

		/// Return the dimensions of a texture instance: width and height.
		extent_type extent(size_type Level = 0) const;

		/// Fetch a texel from a texture. The texture format must be uncompressed.
		template <typename gen_type>
		gen_type load(extent_type const& TexelCoord, size_type Level) const;

		/// Write a texel to a texture. The texture format must be uncompressed.
		template <typename gen_type>
		void store(extent_type const& TexelCoord, size_type Level, gen_type const& Texel);
	};
}//namespace gli

#include "./core/texture2d.inl"
