/// @brief Include to generate mipmaps of textures.
/// @file gli/generate_mipmaps.hpp

#pragma once

#include "texture1d.hpp"
#include "texture1d_array.hpp"
#include "texture2d.hpp"
#include "texture2d_array.hpp"
#include "texture3d.hpp"
#include "texture_cube.hpp"
#include "texture_cube_array.hpp"
#include "sampler.hpp"

namespace gli
{
	/// Allocate a texture and generate all the mipmaps of the texture using the Minification filter.
	template <typename texture_type>
	texture_type generate_mipmaps(texture_type const& Texture, filter Minification);

	/// Allocate a texture and generate the mipmaps of the texture from the BaseLevel to the MaxLevel included using the Minification filter.
	texture1d generate_mipmaps(
		texture1d const& Texture,
		texture1d::size_type BaseLevel, texture1d::size_type MaxLevel,
		filter Minification);

	/// Allocate a texture and generate the mipmaps of the texture from the BaseLayer to the MaxLayer and from the BaseLevel to the MaxLevel included levels using the Minification filter.
	texture1d_array generate_mipmaps(
		texture1d_array const& Texture,
		texture1d_array::size_type BaseLayer, texture1d_array::size_type MaxLayer,
		texture1d_array::size_type BaseLevel, texture1d_array::size_type MaxLevel,
		filter Minification);

	/// Allocate a texture and generate the mipmaps of the texture from the BaseLevel to the MaxLevel included using the Minification filter.
	texture2d generate_mipmaps(
		texture2d const& Texture,
		texture2d::size_type BaseLevel, texture2d::size_type MaxLevel,
		filter Minification);

	/// Allocate a texture and generate the mipmaps of the texture from the BaseLayer to the MaxLayer and from the BaseLevel to the MaxLevel included levels using the Minification filter.
	texture2d_array generate_mipmaps(
		texture2d_array const& Texture,
		texture2d_array::size_type BaseLayer, texture2d_array::size_type MaxLayer,
		texture2d_array::size_type BaseLevel, texture2d_array::size_type MaxLevel,
		filter Minification);

	/// Allocate a texture and generate the mipmaps of the texture from the BaseLevel to the MaxLevel included using the Minification filter.
	texture3d generate_mipmaps(
		texture3d const& Texture,
		texture3d::size_type BaseLevel, texture3d::size_type MaxLevel,
		filter Minification);

	/// Allocate a texture and generate the mipmaps of the texture from the BaseLayer to the MaxLayer, from the BaseFace to the MaxFace and from the BaseLevel to the MaxLevel included levels using the Minification filter.
	texture_cube generate_mipmaps(
		texture_cube const& Texture,
		texture_cube::size_type BaseFace, texture_cube::size_type MaxFace,
		texture_cube::size_type BaseLevel, texture_cube::size_type MaxLevel,
		filter Minification);

	/// Allocate a texture and generate the mipmaps of the texture from the BaseLayer to the MaxLayer and from the BaseLevel to the MaxLevel included levels using the Minification filter.
	texture_cube_array generate_mipmaps(
		texture_cube_array const& Texture,
		texture_cube_array::size_type BaseLayer, texture_cube_array::size_type MaxLayer,
		texture_cube_array::size_type BaseFace, texture_cube_array::size_type MaxFace,
		texture_cube_array::size_type BaseLevel, texture_cube_array::size_type MaxLevel,
		filter Minification);
}//namespace gli

#include "./core/generate_mipmaps.inl"
