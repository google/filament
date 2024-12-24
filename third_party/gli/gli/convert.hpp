/// @brief Include to copy textures, images or a subset of either textures or an images. These operations will cause memory allocations.
/// @file gli/convert.hpp

#pragma once

#include "texture1d.hpp"
#include "texture1d_array.hpp"
#include "texture2d.hpp"
#include "texture2d_array.hpp"
#include "texture3d.hpp"
#include "texture_cube.hpp"
#include "texture_cube_array.hpp"

namespace gli
{
	/// Convert texture data to a new format
	///
	/// @param Texture Source texture, the format must be uncompressed.
	/// @param Format Destination Texture format, it must be uncompressed.
	template <typename texture_type>
	texture_type convert(texture_type const& Texture, format Format);
}//namespace gli

#include "./core/convert.inl"
