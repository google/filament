/// @brief Include to copy textures or a subset of either textures. These operations are performed without memory allocations.
/// @file gli/copy.hpp

#pragma once

#include "type.hpp"

namespace gli
{
	/// Copy a specific image of a texture
	template <typename texture_src_type, typename texture_dst_type>
	void copy(
		texture_src_type const& TextureSrc, size_t LayerSrc, size_t FaceSrc, size_t LevelSrc,
		texture_dst_type& TextureDst, size_t LayerDst, size_t FaceDst, size_t LevelDst);

	/// Copy a texture
	template <typename texture_src_type, typename texture_dst_type>
	void copy(
		texture_src_type const& TextureSrc,
		texture_dst_type& TextureDst);

	// Copy an entire level of a texture
	template <typename texture_src_type, typename texture_dst_type>
	void copy_level(
		texture_src_type const& TextureSrc, size_t BaseLevelSrc,
		texture_dst_type& TextureDst, size_t BaseLevelDst);

	// Copy multiple levels of a texture
	template <typename texture_src_type, typename texture_dst_type>
	void copy_level(
		texture_src_type const& TextureSrc, size_t BaseLevelSrc,
		texture_dst_type& TextureDst, size_t BaseLevelDst,
		size_t LevelCount);

	// Copy an entire face of a texture
	template <typename texture_src_type, typename texture_dst_type>
	void copy_face(
		texture_src_type const& TextureSrc, size_t BaseFaceSrc,
		texture_dst_type& TextureDst, size_t BaseFaceDst);

	// Copy multiple faces of a texture
	template <typename texture_src_type, typename texture_dst_type>
	void copy_face(
		texture_src_type const& TextureSrc, size_t BaseFaceSrc,
		texture_dst_type& TextureDst, size_t BaseFaceDst,
		size_t FaceCount);

	// Copy an entire layer of a texture
	template <typename texture_src_type, typename texture_dst_type>
	void copy_layer(
		texture_src_type const& TextureSrc, size_t BaseLayerSrc,
		texture_dst_type& TextureDst, size_t BaseLayerDst);

	// Copy multiple layers of a texture
	template <typename texture_src_type, typename texture_dst_type>
	void copy_layer(
		texture_src_type const& TextureSrc, size_t BaseLayerSrc,
		texture_dst_type& TextureDst, size_t BaseLayerDst,
		size_t LayerCount);
}//namespace gli

#include "./core/copy.inl"
