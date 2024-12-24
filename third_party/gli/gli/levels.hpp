/// @brief Include to compute the number of mipmaps levels necessary to create a mipmap complete texture.
/// @file gli/levels.hpp

#pragma once

#include "type.hpp"

namespace gli
{
	/// Compute the number of mipmaps levels necessary to create a mipmap complete texture
	/// 
	/// @param Extent Extent of the texture base level mipmap
	/// @tparam vecType Vector type used to express the dimensions of a texture of any kind.
	/// @code
	/// #include <gli/texture2d.hpp>
	/// #include <gli/levels.hpp>
	/// ...
	/// gli::texture2d::extent_type Extent(32, 10);
	/// gli::texture2d Texture(gli::levels(Extent));
	/// @endcode
	template <length_t L, typename T, qualifier P>
	T levels(vec<L, T, P> const& Extent);
/*
	/// Compute the number of mipmaps levels necessary to create a mipmap complete texture
	/// 
	/// @param Extent Extent of the texture base level mipmap
	/// @code
	/// #include <gli/texture2d.hpp>
	/// #include <gli/levels.hpp>
	/// ...
	/// gli::texture2d Texture(32);
	/// @endcode
	size_t levels(size_t Extent);

	/// Compute the number of mipmaps levels necessary to create a mipmap complete texture
	/// 
	/// @param Extent Extent of the texture base level mipmap
	/// @code
	/// #include <gli/texture2d.hpp>
	/// #include <gli/levels.hpp>
	/// ...
	/// gli::texture2d Texture(32);
	/// @endcode
	int levels(int Extent);
*/
}//namespace gli

#include "./core/levels.inl"
