/// @brief Include to copy textures or a subset of either textures. These operations are performed without memory allocations.
/// @file gli/clear.hpp

#pragma once

namespace gli
{
	/// Clear a complete texture
	template <typename texture_type>
	void clear(texture_type& Texture);

	/// Clear a complete texture
	template <typename texture_type, typename gen_type>
	void clear(texture_type& Texture, gen_type const& BlockData);

	/// Clear a specific image of a texture
	template <typename texture_type, typename gen_type>
	void clear(texture_type& Texture, size_t Layer, size_t Face, size_t Level, gen_type const& BlockData);

	// Clear an entire level of a texture
	template <typename texture_type, typename gen_type>
	void clear_level(texture_type& Texture, size_t BaseLevel, gen_type const& BlockData);

	// Clear multiple levels of a texture
	template <typename texture_type, typename gen_type>
	void clear_level(texture_type& Texture, size_t BaseLevel, size_t LevelCount, gen_type const& BlockData);

	// Clear an entire face of a texture
	template <typename texture_type, typename gen_type>
	void clear_face(texture_type& Texture, size_t BaseFace, gen_type const& BlockData);

	// Clear multiple faces of a texture
	template <typename texture_type, typename gen_type>
	void clear_face(texture_type& Texture, size_t BaseFace, size_t FaceCount, gen_type const& BlockData);

	// Clear an entire layer of a texture
	template <typename texture_type, typename gen_type>
	void clear_layer(texture_type& Texture, size_t BaseLayer, gen_type const& BlockData);

	// Clear multiple layers of a texture
	template <typename texture_type, typename gen_type>
	void clear_layer(texture_type& Texture, size_t BaseLayer, size_t LayerCount, gen_type const& BlockData);
}//namespace gli

#include "./core/clear.inl"

