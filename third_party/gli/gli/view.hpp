/// @brief Include create views of textures, either to isolate a subset or to reinterpret data without memory copy.
/// @file gli/view.hpp

#pragma once

#include "image.hpp"
#include "texture.hpp"
#include "texture1d.hpp"
#include "texture1d_array.hpp"
#include "texture2d.hpp"
#include "texture2d_array.hpp"
#include "texture3d.hpp"
#include "texture_cube.hpp"
#include "texture_cube_array.hpp"

namespace gli
{
	/// Create an image view of an existing image, sharing the same memory storage_linear.
	image view(image const & Image);

	/// Create a texture view of an existing texture, sharing the same memory storage_linear.
	texture view(texture const & Texture);

	/// Create a texture view of an existing texture, sharing the same memory storage_linear but giving access only to a subset of layers, levels and faces.
	texture view(
		texture const & Texture,
		texture::size_type BaseLayer, texture::size_type MaxLayer,
		texture::size_type BaseFace, texture::size_type MaxFace,
		texture::size_type BaseLevel, texture::size_type MaxLevel);

	/// Create a texture view of an existing texture, sharing the same memory storage_linear.
	template <typename texType>
	texture view(texType const & Texture);

	/// Create a texture view of an existing texture, sharing the same memory storage_linear but a different format.
	/// The format must be a compatible format, a format which block size match the original format. 
	template <typename texType>
	texture view(texType const & Texture, format Format);

	/// Create a texture view of an existing texture, sharing the same memory storage_linear but giving access only to a subset of levels.
	texture view(
		texture1d const & Texture,
		texture1d::size_type BaseLevel, texture1d::size_type MaxLevel);

	/// Create a texture view of an existing texture, sharing the same memory storage_linear but giving access only to a subset of levels and layers.
	texture view(
		texture1d_array const & Texture,
		texture1d_array::size_type BaseLayer, texture1d_array::size_type MaxLayer,
		texture1d_array::size_type BaseLevel, texture1d_array::size_type MaxLevel);

	/// Create a texture view of an existing texture, sharing the same memory storage_linear but giving access only to a subset of levels.
	texture view(
		texture2d const & Texture,
		texture2d::size_type BaseLevel, texture2d::size_type MaxLevel);

	/// Create a texture view of an existing texture, sharing the same memory storage_linear but giving access only to a subset of levels and layers.
	texture view(
		texture2d_array const & Texture,
		texture2d_array::size_type BaseLayer, texture2d_array::size_type MaxLayer,
		texture2d_array::size_type BaseLevel, texture2d_array::size_type MaxLevel);

	/// Create a texture view of an existing texture, sharing the same memory storage_linear but giving access only to a subset of levels.
	texture view(
		texture3d const & Texture,
		texture3d::size_type BaseLevel, texture3d::size_type MaxLevel);

	/// Create a texture view of an existing texture, sharing the same memory storage_linear but giving access only to a subset of levels and faces.
	texture view(
		texture_cube const & Texture,
		texture_cube::size_type BaseFace, texture_cube::size_type MaxFace,
		texture_cube::size_type BaseLevel, texture_cube::size_type MaxLevel);

	/// Create a texture view of an existing texture, sharing the same memory storage_linear but giving access only to a subset of layers, levels and faces.
	texture view(
		texture_cube_array const & Texture,
		texture_cube_array::size_type BaseLayer, texture_cube_array::size_type MaxLayer,
		texture_cube_array::size_type BaseFace, texture_cube_array::size_type MaxFace,
		texture_cube_array::size_type BaseLevel, texture_cube_array::size_type MaxLevel);
}//namespace gli

#include "./core/view.inl"
