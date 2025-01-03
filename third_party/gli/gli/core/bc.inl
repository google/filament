#include <glm/ext/vector_packing.hpp>
#include <glm/ext/scalar_uint_sized.hpp>

namespace gli
{
	namespace detail
	{
		inline glm::vec4 decompress_bc1(const bc1_block &Block, const extent2d &BlockTexelCoord)
		{
			return decompress_dxt1(Block, BlockTexelCoord);
		}

		inline texel_block4x4 decompress_bc1_block(const bc1_block &Block)
		{
			return decompress_dxt1_block(Block);
		}

		inline glm::vec4 decompress_bc2(const bc2_block &Block, const extent2d &BlockTexelCoord)
		{
			return decompress_dxt3(Block, BlockTexelCoord);
		}

		inline texel_block4x4 decompress_bc2_block(const bc2_block &Block)
		{
			return decompress_dxt3_block(Block);
		}

		inline glm::vec4 decompress_bc3(const bc3_block &Block, const extent2d &BlockTexelCoord)
		{
			return decompress_dxt5(Block, BlockTexelCoord);
		}

		inline texel_block4x4 decompress_bc3_block(const bc3_block &Block)
		{
			return decompress_dxt5_block(Block);
		}

		inline void create_single_channel_lookup_table(bool Interpolate6, float Min, float *LookupTable)
		{
			if(Interpolate6)
			{
				LookupTable[2] = (6.0f / 7.0f) * LookupTable[0] + (1.0f / 7.0f) * LookupTable[1];
				LookupTable[3] = (5.0f / 7.0f) * LookupTable[0] + (2.0f / 7.0f) * LookupTable[1];
				LookupTable[4] = (4.0f / 7.0f) * LookupTable[0] + (3.0f / 7.0f) * LookupTable[1];
				LookupTable[5] = (3.0f / 7.0f) * LookupTable[0] + (4.0f / 7.0f) * LookupTable[1];
				LookupTable[6] = (2.0f / 7.0f) * LookupTable[0] + (5.0f / 7.0f) * LookupTable[1];
				LookupTable[7] = (1.0f / 7.0f) * LookupTable[0] + (6.0f / 7.0f) * LookupTable[1];
			}
			else
			{
				LookupTable[2] = (4.0f / 5.0f) * LookupTable[0] + (1.0f / 5.0f) * LookupTable[1];
				LookupTable[3] = (3.0f / 5.0f) * LookupTable[0] + (2.0f / 5.0f) * LookupTable[1];
				LookupTable[4] = (2.0f / 5.0f) * LookupTable[0] + (3.0f / 5.0f) * LookupTable[1];
				LookupTable[5] = (1.0f / 5.0f) * LookupTable[0] + (4.0f / 5.0f) * LookupTable[1];
				LookupTable[6] = Min;
				LookupTable[7] = 1.0f;
			}
		}

		inline void single_channel_bitmap_data_unorm(glm::uint8 Channel0, glm::uint8 Channel1, const glm::uint8 *ChannelBitmap, float *LookupTable, glm::uint64 &ContiguousBitmap)
		{
			LookupTable[0] = Channel0 / 255.0f;
			LookupTable[1] = Channel1 / 255.0f;

			create_single_channel_lookup_table(Channel0 > Channel1, 0.0f, LookupTable);

			ContiguousBitmap = ChannelBitmap[0] | (ChannelBitmap[1] << 8) | (ChannelBitmap[2] << 16);
			ContiguousBitmap |= glm::uint64(ChannelBitmap[3] | (ChannelBitmap[4] << 8) | (ChannelBitmap[5] << 16)) << 24;
		}

		inline void single_channel_bitmap_data_snorm(glm::uint8 Channel0, glm::uint8 Channel1, const glm::uint8 *ChannelBitmap, float *LookupTable, glm::uint64 &ContiguousBitmap)
		{
			LookupTable[0] = (Channel0 / 255.0f) * 2.0f - 1.0f;
			LookupTable[1] = (Channel1 / 255.0f) * 2.0f - 1.0f;

			create_single_channel_lookup_table(Channel0 > Channel1, -1.0f, LookupTable);

			ContiguousBitmap = ChannelBitmap[0] | (ChannelBitmap[1] << 8) | (ChannelBitmap[2] << 16);
			ContiguousBitmap |= glm::uint64(ChannelBitmap[3] | (ChannelBitmap[4] << 8) | (ChannelBitmap[5] << 16)) << 24;
		}

		inline void single_channel_bitmap_data_snorm(glm::uint8 Channel0, glm::uint32 Channel1, bool Interpolate6, const glm::uint8 *ChannelBitmap, float *LookupTable, glm::uint64 &ContiguousBitmap)
		{
			LookupTable[0] = (Channel0 / 255.0f) * 2.0f - 1.0f;
			LookupTable[1] = (Channel1 / 255.0f) * 2.0f - 1.0f;

			create_single_channel_lookup_table(Interpolate6, -1.0f, LookupTable);

			ContiguousBitmap = ChannelBitmap[0] | (ChannelBitmap[1] << 8) | (ChannelBitmap[2] << 16);
			ContiguousBitmap |= glm::uint64(ChannelBitmap[3] | (ChannelBitmap[4] << 8) | (ChannelBitmap[5] << 16)) << 24;
		}

		inline glm::vec4 decompress_bc4unorm(const bc4_block &Block, const extent2d &BlockTexelCoord)
		{
			float RedLUT[8];
			glm::uint64 Bitmap;

			single_channel_bitmap_data_unorm(Block.Red0, Block.Red1, Block.Bitmap, RedLUT, Bitmap);

			glm::uint8 RedIndex = (Bitmap >> ((BlockTexelCoord.y * 4 + BlockTexelCoord.x) * 3)) & 0x7;
			return glm::vec4(RedLUT[RedIndex], 0.0f, 0.0f, 1.0f);
		}

		inline glm::vec4 decompress_bc4snorm(const bc4_block &Block, const extent2d &BlockTexelCoord)
		{
			float RedLUT[8];
			glm::uint64 Bitmap;

			single_channel_bitmap_data_snorm(Block.Red0, Block.Red1, Block.Bitmap, RedLUT, Bitmap);

			glm::uint8 RedIndex = (Bitmap >> ((BlockTexelCoord.y * 4 + BlockTexelCoord.x) * 3)) & 0x7;
			return glm::vec4(RedLUT[RedIndex], 0.0f, 0.0f, 1.0f);
		}

		inline texel_block4x4 decompress_bc4unorm_block(const bc4_block &Block)
		{
			float RedLUT[8];
			glm::uint64 Bitmap;

			single_channel_bitmap_data_unorm(Block.Red0, Block.Red1, Block.Bitmap, RedLUT, Bitmap);

			texel_block4x4 TexelBlock;
			for(glm::uint8 Row = 0; Row < 4; ++Row)
			{
				for(glm::uint8 Col = 0; Col < 4; ++Col)
				{
					glm::uint8 RedIndex = (Bitmap >> ((Row * 4 + Col) * 3)) & 0x7;
					TexelBlock.Texel[Row][Col] = glm::vec4(RedLUT[RedIndex], 0.0f, 0.0f, 1.0f);
				}
			}
			
			return TexelBlock;
		}

		inline texel_block4x4 decompress_bc4snorm_block(const bc4_block &Block)
		{
			float RedLUT[8];
			glm::uint64 Bitmap;

			single_channel_bitmap_data_snorm(Block.Red0, Block.Red1, Block.Bitmap, RedLUT, Bitmap);

			texel_block4x4 TexelBlock;
			for(glm::uint8 Row = 0; Row < 4; ++Row)
			{
				for(glm::uint8 Col = 0; Col < 4; ++Col)
				{
					glm::uint8 RedIndex = (Bitmap >> ((Row * 4 + Col) * 3)) & 0x7;
					TexelBlock.Texel[Row][Col] = glm::vec4(RedLUT[RedIndex], 0.0f, 0.0f, 1.0f);
				}
			}

			return TexelBlock;
		}

		inline glm::vec4 decompress_bc5unorm(const bc5_block &Block, const extent2d &BlockTexelCoord)
		{
			float RedLUT[8];
			glm::uint64 RedBitmap;

			single_channel_bitmap_data_unorm(Block.Red0, Block.Red1, Block.RedBitmap, RedLUT, RedBitmap);
			glm::uint8 RedIndex = (RedBitmap >> ((BlockTexelCoord.y * 4 + BlockTexelCoord.x) * 3)) & 0x7;
			
			float GreenLUT[8];
			glm::uint64 GreenBitmap;

			single_channel_bitmap_data_unorm(Block.Green0, Block.Green1, Block.GreenBitmap, GreenLUT, GreenBitmap);
			glm::uint8 GreenIndex = (GreenBitmap >> ((BlockTexelCoord.y * 4 + BlockTexelCoord.x) * 3)) & 0x7;

			return glm::vec4(RedLUT[RedIndex], GreenLUT[GreenIndex], 0.0f, 1.0f);
		}

		inline glm::vec4 decompress_bc5snorm(const bc5_block &Block, const extent2d &BlockTexelCoord)
		{
			float RedLUT[8];
			glm::uint64 RedBitmap;

			single_channel_bitmap_data_snorm(Block.Red0, Block.Red1, Block.RedBitmap, RedLUT, RedBitmap);
			glm::uint8 RedIndex = (RedBitmap >> ((BlockTexelCoord.y * 4 + BlockTexelCoord.x) * 3)) & 0x7;

			float GreenLUT[8];
			glm::uint64 GreenBitmap;

			single_channel_bitmap_data_snorm(Block.Green0, Block.Green1, Block.GreenBitmap, GreenLUT, GreenBitmap);
			glm::uint8 GreenIndex = (GreenBitmap >> ((BlockTexelCoord.y * 4 + BlockTexelCoord.x) * 3)) & 0x7;

			return glm::vec4(RedLUT[RedIndex], GreenLUT[GreenIndex], 0.0f, 1.0f);
		}

		inline texel_block4x4 decompress_bc5unorm_block(const bc5_block &Block)
		{
			float RedLUT[8];
			glm::uint64 RedBitmap;

			single_channel_bitmap_data_unorm(Block.Red0, Block.Red1, Block.RedBitmap, RedLUT, RedBitmap);
			
			float GreenLUT[8];
			glm::uint64 GreenBitmap;

			single_channel_bitmap_data_unorm(Block.Green0, Block.Green1, Block.GreenBitmap, GreenLUT, GreenBitmap);

			texel_block4x4 TexelBlock;
			for(glm::uint8 Row = 0; Row < 4; ++Row)
			{
				for(glm::uint8 Col = 0; Col < 4; ++Col)
				{
					glm::uint8 RedIndex = (RedBitmap >> ((Row * 4 + Col) * 3)) & 0x7;
					glm::uint8 GreenIndex = (GreenBitmap >> ((Row * 4 + Col) * 3)) & 0x7;
					TexelBlock.Texel[Row][Col] = glm::vec4(RedLUT[RedIndex], GreenLUT[GreenIndex], 0.0f, 1.0f);
				}
			}
			
			return TexelBlock;
		}

		inline texel_block4x4 decompress_bc5snorm_block(const bc5_block &Block)
		{
			float RedLUT[8];
			glm::uint64 RedBitmap;

			single_channel_bitmap_data_snorm(Block.Red0, Block.Red1, Block.RedBitmap, RedLUT, RedBitmap);

			float GreenLUT[8];
			glm::uint64 GreenBitmap;

			single_channel_bitmap_data_snorm(Block.Green0, Block.Green1, Block.Red0 > Block.Red1, Block.GreenBitmap, GreenLUT, GreenBitmap);

			texel_block4x4 TexelBlock;
			for(glm::uint8 Row = 0; Row < 4; ++Row)
			{
				for(glm::uint8 Col = 0; Col < 4; ++Col)
				{
					glm::uint8 RedIndex = (RedBitmap >> ((Row * 4 + Col) * 3)) & 0x7;
					glm::uint8 GreenIndex = (GreenBitmap >> ((Row * 4 + Col) * 3)) & 0x7;
					TexelBlock.Texel[Row][Col] = glm::vec4(RedLUT[RedIndex], GreenLUT[GreenIndex], 0.0f, 1.0f);
				}
			}

			return TexelBlock;
		}

	}//namespace detail
}//namespace gli
