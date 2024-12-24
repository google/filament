#include <glm/ext/vector_packing.hpp>
#include <glm/gtc/packing.hpp>

namespace gli
{
	namespace detail
	{
		inline glm::vec4 decompress_dxt1(const dxt1_block &Block, const extent2d &BlockTexelCoord)
		{
			glm::vec4 Color[4];

			Color[0] = glm::vec4(unpackUnorm1x5_1x6_1x5(Block.Color0), 1.0f);
			std::swap(Color[0].r, Color[0].b);
			Color[1] = glm::vec4(unpackUnorm1x5_1x6_1x5(Block.Color1), 1.0f);
			std::swap(Color[1].r, Color[1].b);

			if(Block.Color0 > Block.Color1)
			{
				Color[2] = (2.0f / 3.0f) * Color[0] + (1.0f / 3.0f) * Color[1];
				Color[3] = (1.0f / 3.0f) * Color[0] + (2.0f / 3.0f) * Color[1];
			}
			else
			{
				Color[2] = (Color[0] + Color[1]) / 2.0f;
				Color[3] = glm::vec4(0.0f);
			}

			glm::uint8 ColorIndex = (Block.Row[BlockTexelCoord.y] >> (BlockTexelCoord.x * 2)) & 0x3;
			return Color[ColorIndex];
		}

		inline texel_block4x4 decompress_dxt1_block(const dxt1_block &Block)
		{
			glm::vec4 Color[4];

			Color[0] = glm::vec4(unpackUnorm1x5_1x6_1x5(Block.Color0), 1.0f);
			std::swap(Color[0].r, Color[0].b);
			Color[1] = glm::vec4(unpackUnorm1x5_1x6_1x5(Block.Color1), 1.0f);
			std::swap(Color[1].r, Color[1].b);

			if(Block.Color0 > Block.Color1)
			{
				Color[2] = (2.0f / 3.0f) * Color[0] + (1.0f / 3.0f) * Color[1];
				Color[3] = (1.0f / 3.0f) * Color[0] + (2.0f / 3.0f) * Color[1];
			}
			else
			{
				Color[2] = (Color[0] + Color[1]) / 2.0f;
				Color[3] = glm::vec4(0.0f);
			}

			texel_block4x4 TexelBlock;
			for(glm::uint8 Row = 0; Row < 4; ++Row)
			{
				for(glm::uint8 Col = 0; Col < 4; ++Col)
				{
					glm::uint8 ColorIndex = (Block.Row[Row] >> (Col * 2)) & 0x3;
					TexelBlock.Texel[Row][Col] = Color[ColorIndex];
				}
			}
			
			return TexelBlock;
		}

		inline glm::vec4 decompress_dxt3(const dxt3_block &Block, const extent2d &BlockTexelCoord)
		{
			glm::vec3 Color[4];

			Color[0] = glm::vec3(unpackUnorm1x5_1x6_1x5(Block.Color0));
			std::swap(Color[0].r, Color[0].b);
			Color[1] = glm::vec3(unpackUnorm1x5_1x6_1x5(Block.Color1));
			std::swap(Color[1].r, Color[1].b);

			Color[2] = (2.0f / 3.0f) * Color[0] + (1.0f / 3.0f) * Color[1];
			Color[3] = (1.0f / 3.0f) * Color[0] + (2.0f / 3.0f) * Color[1];

			glm::uint8 ColorIndex = (Block.Row[BlockTexelCoord.y] >> (BlockTexelCoord.x * 2)) & 0x3;
			float Alpha = ((Block.AlphaRow[BlockTexelCoord.y] >> (BlockTexelCoord.x * 4)) & 0xF) / 15.0f;

			return glm::vec4(Color[ColorIndex], Alpha);
		}

		inline texel_block4x4 decompress_dxt3_block(const dxt3_block &Block)
		{
			glm::vec3 Color[4];

			Color[0] = glm::vec3(unpackUnorm1x5_1x6_1x5(Block.Color0));
			std::swap(Color[0].r, Color[0].b);
			Color[1] = glm::vec3(unpackUnorm1x5_1x6_1x5(Block.Color1));
			std::swap(Color[1].r, Color[1].b);

			Color[2] = (2.0f / 3.0f) * Color[0] + (1.0f / 3.0f) * Color[1];
			Color[3] = (1.0f / 3.0f) * Color[0] + (2.0f / 3.0f) * Color[1];

			texel_block4x4 TexelBlock;
			for(glm::uint8 Row = 0; Row < 4; ++Row)
			{
				for(glm::uint8 Col = 0; Col < 4; ++Col)
				{
					glm::uint8 ColorIndex = (Block.Row[Row] >> (Col * 2)) & 0x3;
					float Alpha = ((Block.AlphaRow[Row] >> (Col * 4)) & 0xF) / 15.0f;
					TexelBlock.Texel[Row][Col] = glm::vec4(Color[ColorIndex], Alpha);
				}
			}

			return TexelBlock;
		}

		inline glm::vec4 decompress_dxt5(const dxt5_block &Block, const extent2d &BlockTexelCoord)
		{
			glm::vec3 Color[4];
			float Alpha[8];

			Color[0] = glm::vec3(unpackUnorm1x5_1x6_1x5(Block.Color0));
			std::swap(Color[0].r, Color[0].b);
			Color[1] = glm::vec3(unpackUnorm1x5_1x6_1x5(Block.Color1));
			std::swap(Color[1].r, Color[1].b);

			Color[2] = (2.0f / 3.0f) * Color[0] + (1.0f / 3.0f) * Color[1];
			Color[3] = (1.0f / 3.0f) * Color[0] + (2.0f / 3.0f) * Color[1];

			glm::uint8 ColorIndex = (Block.Row[BlockTexelCoord.y] >> (BlockTexelCoord.x * 2)) & 0x3;

			Alpha[0] = Block.Alpha[0] / 255.0f;
			Alpha[1] = Block.Alpha[1] / 255.0f;

			if(Alpha[0] > Alpha[1])
			{
				Alpha[2] = (6.0f / 7.0f) * Alpha[0] + (1.0f / 7.0f) * Alpha[1];
				Alpha[3] = (5.0f / 7.0f) * Alpha[0] + (2.0f / 7.0f) * Alpha[1];
				Alpha[4] = (4.0f / 7.0f) * Alpha[0] + (3.0f / 7.0f) * Alpha[1];
				Alpha[5] = (3.0f / 7.0f) * Alpha[0] + (4.0f / 7.0f) * Alpha[1];
				Alpha[6] = (2.0f / 7.0f) * Alpha[0] + (5.0f / 7.0f) * Alpha[1];
				Alpha[7] = (1.0f / 7.0f) * Alpha[0] + (6.0f / 7.0f) * Alpha[1];
			}
			else
			{
				Alpha[2] = (4.0f / 5.0f) * Alpha[0] + (1.0f / 5.0f) * Alpha[1];
				Alpha[3] = (3.0f / 5.0f) * Alpha[0] + (2.0f / 5.0f) * Alpha[1];
				Alpha[4] = (2.0f / 5.0f) * Alpha[0] + (3.0f / 5.0f) * Alpha[1];
				Alpha[5] = (1.0f / 5.0f) * Alpha[0] + (4.0f / 5.0f) * Alpha[1];
				Alpha[6] = 0.0f;
				Alpha[7] = 1.0f;
			}

			glm::uint64 Bitmap;
			Bitmap = Block.AlphaBitmap[0] | (Block.AlphaBitmap[1] << 8) | (Block.AlphaBitmap[2] << 16);
			Bitmap |= glm::uint64(Block.AlphaBitmap[3] | (Block.AlphaBitmap[4] << 8) | (Block.AlphaBitmap[5] << 16)) << 24;

			glm::uint8 AlphaIndex = (Bitmap >> ((BlockTexelCoord.y * 4 + BlockTexelCoord.x) * 3)) & 0x7;

			return glm::vec4(Color[ColorIndex], Alpha[AlphaIndex]);
		}

		inline texel_block4x4 decompress_dxt5_block(const dxt5_block &Block)
		{
			glm::vec3 Color[4];
			float Alpha[8];

			Color[0] = glm::vec3(unpackUnorm1x5_1x6_1x5(Block.Color0));
			std::swap(Color[0].r, Color[0].b);
			Color[1] = glm::vec3(unpackUnorm1x5_1x6_1x5(Block.Color1));
			std::swap(Color[1].r, Color[1].b);

			Color[2] = (2.0f / 3.0f) * Color[0] + (1.0f / 3.0f) * Color[1];
			Color[3] = (1.0f / 3.0f) * Color[0] + (2.0f / 3.0f) * Color[1];

			Alpha[0] = Block.Alpha[0] / 255.0f;
			Alpha[1] = Block.Alpha[1] / 255.0f;

			if(Alpha[0] > Alpha[1])
			{
				Alpha[2] = (6.0f / 7.0f) * Alpha[0] + (1.0f / 7.0f) * Alpha[1];
				Alpha[3] = (5.0f / 7.0f) * Alpha[0] + (2.0f / 7.0f) * Alpha[1];
				Alpha[4] = (4.0f / 7.0f) * Alpha[0] + (3.0f / 7.0f) * Alpha[1];
				Alpha[5] = (3.0f / 7.0f) * Alpha[0] + (4.0f / 7.0f) * Alpha[1];
				Alpha[6] = (2.0f / 7.0f) * Alpha[0] + (5.0f / 7.0f) * Alpha[1];
				Alpha[7] = (1.0f / 7.0f) * Alpha[0] + (6.0f / 7.0f) * Alpha[1];
			}
			else
			{
				Alpha[2] = (4.0f / 5.0f) * Alpha[0] + (1.0f / 5.0f) * Alpha[1];
				Alpha[3] = (3.0f / 5.0f) * Alpha[0] + (2.0f / 5.0f) * Alpha[1];
				Alpha[4] = (2.0f / 5.0f) * Alpha[0] + (3.0f / 5.0f) * Alpha[1];
				Alpha[5] = (1.0f / 5.0f) * Alpha[0] + (4.0f / 5.0f) * Alpha[1];
				Alpha[6] = 0.0f;
				Alpha[7] = 1.0f;
			}

			glm::uint64 Bitmap;
			Bitmap = Block.AlphaBitmap[0] | (Block.AlphaBitmap[1] << 8) | (Block.AlphaBitmap[2] << 16);
			Bitmap |= glm::uint64(Block.AlphaBitmap[3] | (Block.AlphaBitmap[4] << 8) | (Block.AlphaBitmap[5] << 16)) << 24;

			texel_block4x4 TexelBlock;
			for(glm::uint8 Row = 0; Row < 4; ++Row)
			{
				for(glm::uint8 Col = 0; Col < 4; ++Col)
				{
					glm::uint8 ColorIndex = (Block.Row[Row] >> (Col * 2)) & 0x3;
					glm::uint8 AlphaIndex = (Bitmap >> ((Row * 4 + Col) * 3)) & 0x7;
					TexelBlock.Texel[Row][Col] = glm::vec4(Color[ColorIndex], Alpha[AlphaIndex]);
				}
			}

			return TexelBlock;
		}
	}//namespace detail
}//namespace gli
