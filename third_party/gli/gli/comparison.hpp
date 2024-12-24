/// @brief Include to use operators to compare whether two textures or images are equal
/// @file gli/comparison.hpp

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
	/// Compare two images. Two images are equal when the date is the same.
	bool operator==(image const& ImageA, image const& ImageB);

	/// Compare two images. Two images are equal when the date is the same.
	bool operator!=(image const& ImageA, image const& ImageB);

	/// Compare two textures. Two textures are the same when the data, the format and the targets are the same.
	bool operator==(texture const& A, texture const& B);

	/// Compare two textures. Two textures are the same when the data, the format and the targets are the same.
	bool operator!=(texture const& A, texture const& B);
}//namespace gli

#include "./core/comparison.inl"
