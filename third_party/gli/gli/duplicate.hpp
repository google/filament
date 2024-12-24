/// @brief Include to duplicate textures, images or a subset of either textures or an image. These operations will cause memory allocations.
/// @file gli/duplicate.hpp

#pragma once

#include "image.hpp"
#include "texture1d.hpp"
#include "texture1d_array.hpp"
#include "texture2d.hpp"
#include "texture2d_array.hpp"
#include "texture3d.hpp"
#include "texture_cube.hpp"
#include "texture_cube_array.hpp"

namespace gli
{
	/// Duplicate an image and create a new image with a new storage_linear allocation.
	image duplicate(image const & Image);

	/// Duplicate a texture and create a new texture with a new storage_linear allocation.
	template <typename texType>
	texture duplicate(texType const& Texture);

	/// Duplicate a texture and create a new texture with a new storage_linear allocation but a different format.
	/// The format must be a compatible format, a format which block size match the original format. 
	template <typename texType>
	texture duplicate(texType const& Texture, format Format);

	/// Duplicate a subset of a texture and create a new texture with a new storage_linear allocation.
	texture duplicate(
		texture1d const& Texture,
		texture1d::size_type BaseLevel, texture1d::size_type MaxLevel);

	/// Duplicate a subset of a texture and create a new texture with a new storage_linear allocation.
	texture duplicate(
		texture1d_array const& Texture,
		texture1d_array::size_type BaseLayer, texture1d_array::size_type MaxLayer,
		texture1d_array::size_type BaseLevel, texture1d_array::size_type MaxLevel);

	/// Duplicate a subset of a texture and create a new texture with a new storage_linear allocation.
	texture duplicate(
		texture2d const& Texture,
		texture2d::size_type BaseLevel, texture2d::size_type MaxLevel);

	/// Duplicate a subset of a texture and create a new texture with a new storage_linear allocation.
	texture duplicate(
		texture2d_array const& Texture,
		texture2d_array::size_type BaseLayer, texture2d_array::size_type MaxLayer,
		texture2d_array::size_type BaseLevel, texture2d_array::size_type MaxLevel);

	/// Duplicate a subset of a texture and create a new texture with a new storage_linear allocation.
	texture duplicate(
		texture3d const& Texture,
		texture3d::size_type BaseLevel, texture3d::size_type MaxLevel);

	/// Duplicate a subset of a texture and create a new texture with a new storage_linear allocation.
	texture duplicate(
		texture_cube const& Texture,
		texture_cube::size_type BaseFace, texture_cube::size_type MaxFace,
		texture_cube::size_type BaseLevel, texture_cube::size_type MaxLevel);

	/// Duplicate a subset of a texture and create a new texture with a new storage_linear allocation.
	texture duplicate(
		texture_cube_array const& Texture,
		texture_cube_array::size_type BaseLayer, texture_cube_array::size_type MaxLayer,
		texture_cube_array::size_type BaseFace, texture_cube_array::size_type MaxFace,
		texture_cube_array::size_type BaseLevel, texture_cube_array::size_type MaxLevel);
}//namespace gli

#include "./core/duplicate.inl"
