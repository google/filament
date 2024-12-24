#pragma once

// STD
#include <vector>
#include <queue>
#include <string>
#include <cassert>
#include <cmath>
#include <cstring>
#include <memory>

#include "../type.hpp"
#include "../format.hpp"

// GLM
#include <glm/gtc/round.hpp>
#include <glm/gtc/bitfield.hpp>
#include <glm/gtx/component_wise.hpp>

static_assert(GLM_VERSION >= 99, "GLI requires at least GLM 0.9.9");

namespace gli
{
	class storage_linear
	{
	public:
		typedef extent3d extent_type;
		typedef size_t size_type;
		typedef gli::format format_type;
		typedef gli::byte data_type;

	public:
		storage_linear();

		storage_linear(
			format_type Format,
			extent_type const & Extent,
			size_type Layers,
			size_type Faces,
			size_type Levels);

		bool empty() const;
		size_type size() const; // Express is bytes
		size_type layers() const;
		size_type levels() const;
		size_type faces() const;

		size_type block_size() const;
		extent_type block_extent() const;
		extent_type block_count(size_type Level) const;
		extent_type extent(size_type Level) const;

		data_type* data();
		data_type const* const data() const;

		/// Compute the relative memory offset to access the data for a specific layer, face and level
		size_type base_offset(
			size_type Layer,
			size_type Face,
			size_type Level) const;

		size_type image_offset(extent1d const& Coord, extent1d const& Extent) const;

		size_type image_offset(extent2d const& Coord, extent2d const& Extent) const;

		size_type image_offset(extent3d const& Coord, extent3d const& Extent) const;

		/// Copy a subset of a specific image of a texture 
		void copy(
			storage_linear const& StorageSrc,
			size_t LayerSrc, size_t FaceSrc, size_t LevelSrc, extent_type const& BlockIndexSrc,
			size_t LayerDst, size_t FaceDst, size_t LevelDst, extent_type const& BlockIndexDst,
			extent_type const& BlockCount);

		size_type level_size(
			size_type Level) const;
		size_type face_size(
			size_type BaseLevel, size_type MaxLevel) const;
		size_type layer_size(
			size_type BaseFace, size_type MaxFace,
			size_type BaseLevel, size_type MaxLevel) const;

	private:
		size_type const Layers;
		size_type const Faces;
		size_type const Levels;
		size_type const BlockSize;
		extent_type const BlockCount;
		extent_type const BlockExtent;
		extent_type const Extent;
		std::vector<data_type> Data;
	};
}//namespace gli

#include "storage_linear.inl"
