/// @brief Include to compress and decompress the BC compression scheme
/// @file gli/bc.hpp

#pragma once

#include "./s3tc.hpp"

namespace gli
{
	namespace detail
	{
		typedef dxt1_block bc1_block;
		typedef dxt3_block bc2_block;
		typedef dxt5_block bc3_block;

		struct bc4_block {
			glm::uint8 Red0;
			glm::uint8 Red1;
			glm::uint8 Bitmap[6];
		};

		struct bc5_block {
			glm::uint8 Red0;
			glm::uint8 Red1;
			glm::uint8 RedBitmap[6];
			glm::uint8 Green0;
			glm::uint8 Green1;
			glm::uint8 GreenBitmap[6];
		};

		glm::vec4 decompress_bc1(const bc1_block &Block, const extent2d &BlockTexelCoord);
		texel_block4x4 decompress_dxt1_block(const dxt1_block &Block);

		glm::vec4 decompress_bc2(const bc2_block &Block, const extent2d &BlockTexelCoord);
		texel_block4x4 decompress_dxt3_block(const bc2_block &Block);

		glm::vec4 decompress_bc3(const bc3_block &Block, const extent2d &BlockTexelCoord);
		texel_block4x4 decompress_bc3_block(const dxt5_block &Block);

		glm::vec4 decompress_bc4unorm(const bc4_block &Block, const extent2d &BlockTexelCoord);
		glm::vec4 decompress_bc4snorm(const bc4_block &Block, const extent2d &BlockTexelCoord);
		texel_block4x4 decompress_bc4unorm_block(const bc4_block &Block);
		texel_block4x4 decompress_bc4snorm_block(const bc4_block &Block);

		glm::vec4 decompress_bc5unorm(const bc5_block &Block, const extent2d &BlockTexelCoord);
		glm::vec4 decompress_bc5snorm(const bc5_block &Block, const extent2d &BlockTexelCoord);
		texel_block4x4 decompress_bc5unorm_block(const bc5_block &Block);
		texel_block4x4 decompress_bc5snorm_block(const bc5_block &Block);
	}//namespace detail
}//namespace gli

#include "./bc.inl"
