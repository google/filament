/// @brief Include to compress and decompress s3tc blocks
/// @file gli/s3tc.hpp

#pragma once

#include <glm/ext/scalar_uint_sized.hpp>

namespace gli
{
	namespace detail
	{
		struct dxt1_block {
			glm::uint16 Color0;
			glm::uint16 Color1;
			glm::uint8 Row[4];
		};

		struct dxt3_block {
			glm::uint16 AlphaRow[4];
			glm::uint16 Color0;
			glm::uint16 Color1;
			glm::uint8 Row[4];
		};

		struct dxt5_block {
			glm::uint8 Alpha[2];
			glm::uint8 AlphaBitmap[6];
			glm::uint16 Color0;
			glm::uint16 Color1;
			glm::uint8 Row[4];
		};

		struct texel_block4x4 {
			// row x col
			glm::vec4 Texel[4][4];
		};
		
		glm::vec4 decompress_dxt1(const dxt1_block &Block, const extent2d &BlockTexelCoord);
		texel_block4x4 decompress_dxt1_block(const dxt1_block &Block);

		glm::vec4 decompress_dxt3(const dxt3_block &Block, const extent2d &BlockTexelCoord);
		texel_block4x4 decompress_dxt3_block(const dxt3_block &Block);

		glm::vec4 decompress_dxt5(const dxt5_block &Block, const extent2d &BlockTexelCoord);
		texel_block4x4 decompress_dxt5_block(const dxt5_block &Block);

	}//namespace detail
}//namespace gli

#include "./s3tc.inl"
