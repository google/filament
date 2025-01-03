#include <gli/load.hpp>
#include <gli/save.hpp>
#include <gli/convert.hpp>
#include <gli/comparison.hpp>
#include <gli/core/bc.hpp>
#include <gli/core/s3tc.hpp>
#include <gli/texture2d.hpp>
#include <gli/duplicate.hpp>
#include <gli/generate_mipmaps.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <ctime>

namespace
{
	std::string path(const char* filename)
	{
		return std::string(SOURCE_DIR) + "/data/" + filename;
	}

	struct params
	{
		params(std::string const & Filename, gli::format Format)
			: Filename(Filename)
			, Format(Format)
		{}

		std::string Filename;
		gli::format Format;
	};
}//namespace

bool convert_rgb32f_rgb9e5(const char* FilenameSrc, const char* FilenameDst)
{
	if(FilenameDst == NULL)
		return false;
	if(std::strstr(FilenameDst, ".dds") != nullptr || std::strstr(FilenameDst, ".ktx") != nullptr)
		return false;

	gli::texture2d TextureSource(gli::load(FilenameSrc));
	if(TextureSource.empty())
		return false;
	if(TextureSource.format() != gli::FORMAT_RGB16_SFLOAT_PACK16 && TextureSource.format() != gli::FORMAT_RGB32_SFLOAT_PACK32)
		return false;

	gli::texture2d TextureMipmaped = gli::generate_mipmaps(TextureSource, gli::FILTER_LINEAR);
	gli::texture2d TextureConverted = gli::convert(TextureMipmaped, gli::FORMAT_RGB9E5_UFLOAT_PACK32);

	gli::save(TextureConverted, FilenameDst);

	return true;
}

namespace r8unorm
{
	int test()
	{
		int Error(0);

		gli::texture2d A(gli::FORMAT_R8_UNORM_PACK8, gli::extent2d(2), 1);
		A.store(gli::extent2d(0, 0), 0, glm::u8vec1(192));
		A.store(gli::extent2d(1, 0), 0, glm::u8vec1(255));
		A.store(gli::extent2d(1, 1), 0, glm::u8vec1(0));
		A.store(gli::extent2d(0, 1), 0, glm::u8vec1(16));

		gli::texture2d const B = gli::convert(A, gli::FORMAT_RGBA8_UNORM_PACK8);
		gli::texture2d const C = gli::convert(B, gli::FORMAT_R8_UNORM_PACK8);
		Error += A == C ? 0 : 1;

		gli::texture2d const D = gli::convert(A, gli::FORMAT_RGB8_UNORM_PACK8);
		gli::texture2d const E = gli::convert(D, gli::FORMAT_R8_UNORM_PACK8);
		Error += A == E ? 0 : 1;

		gli::texture2d const F = gli::convert(A, gli::FORMAT_RG8_UNORM_PACK8);
		gli::texture2d const G = gli::convert(F, gli::FORMAT_R8_UNORM_PACK8);
		Error += A == G ? 0 : 1;

		return Error;
	}
}//namespace r8unorm

namespace rg8unorm
{
	int test()
	{
		int Error(0);

		gli::texture2d A(gli::FORMAT_RG8_UNORM_PACK8, gli::extent2d(2), 1);
		A.store(gli::extent2d(0, 0), 0, glm::u8vec2(192, 48));
		A.store(gli::extent2d(1, 0), 0, glm::u8vec2(255, 128));
		A.store(gli::extent2d(1, 1), 0, glm::u8vec2(0, 128));
		A.store(gli::extent2d(0, 1), 0, glm::u8vec2(16, 48));

		gli::texture2d const B = gli::convert(A, gli::FORMAT_RGBA8_UNORM_PACK8);
		gli::texture2d const C = gli::convert(B, gli::FORMAT_RG8_UNORM_PACK8);
		Error += A == C ? 0 : 1;

		gli::texture2d const D = gli::convert(A, gli::FORMAT_RGB8_UNORM_PACK8);
		gli::texture2d const E = gli::convert(D, gli::FORMAT_RG8_UNORM_PACK8);
		Error += A == E ? 0 : 1;

		return Error;
	}
}//namespace rg8unorm

namespace rgb8unorm
{
	int test()
	{
		int Error(0);

		gli::texture2d A(gli::FORMAT_RGB8_UNORM_PACK8, gli::extent2d(2), 1);
		A.store(gli::extent2d(0, 0), 0, glm::u8vec3(192, 48, 16));
		A.store(gli::extent2d(1, 0), 0, glm::u8vec3(255, 128, 0));
		A.store(gli::extent2d(1, 1), 0, glm::u8vec3(0, 128, 255));
		A.store(gli::extent2d(0, 1), 0, glm::u8vec3(16, 48, 192));

		gli::texture2d const B = gli::convert(A, gli::FORMAT_RGBA8_UNORM_PACK8);
		gli::texture2d const C = gli::convert(B, gli::FORMAT_RGB8_UNORM_PACK8);

		Error += A == C ? 0 : 1;

		return Error;
	}
}//namespace rgb8unorm

namespace r16unorm
{
	int test()
	{
		int Error(0);

		gli::texture2d A(gli::FORMAT_R16_UNORM_PACK16, gli::extent2d(2), 1);
		A.store(gli::extent2d(0, 0), 0, glm::u16vec1(0x7777));
		A.store(gli::extent2d(1, 0), 0, glm::u16vec1(0x1111));
		A.store(gli::extent2d(1, 1), 0, glm::u16vec1(0x5555));
		A.store(gli::extent2d(0, 1), 0, glm::u16vec1(0x3333));

		gli::texture2d const B = gli::convert(A, gli::FORMAT_RGBA16_UNORM_PACK16);
		gli::texture2d const C = gli::convert(B, gli::FORMAT_R16_UNORM_PACK16);

		Error += A == C ? 0 : 1;

		return Error;
	}
}//namespace r16unorm

namespace rg16unorm
{
	int test()
	{
		int Error(0);

		gli::texture2d A(gli::FORMAT_RG16_UNORM_PACK16, gli::extent2d(2), 1);
		A.store(gli::extent2d(0, 0), 0, glm::u16vec2(0x7777, 0x5555));
		A.store(gli::extent2d(1, 0), 0, glm::u16vec2(0x1111, 0x3333));
		A.store(gli::extent2d(1, 1), 0, glm::u16vec2(0x5555, 0x2222));
		A.store(gli::extent2d(0, 1), 0, glm::u16vec2(0xaaaa, 0xcccc));

		gli::texture2d const B = gli::convert(A, gli::FORMAT_RGBA16_UNORM_PACK16);
		gli::texture2d const C = gli::convert(B, gli::FORMAT_RG16_UNORM_PACK16);

		Error += A == C ? 0 : 1;

		return Error;
	}
}//namespace rg16unorm

namespace rgb16unorm
{
	int test()
	{
		int Error(0);

		gli::texture2d A(gli::FORMAT_RGB16_UNORM_PACK16, gli::extent2d(2), 1);
		A.store(gli::extent2d(0, 0), 0, glm::u16vec3(0x7777, 0x5555, 0x3333));
		A.store(gli::extent2d(1, 0), 0, glm::u16vec3(0x1111, 0x3333, 0x5555));
		A.store(gli::extent2d(1, 1), 0, glm::u16vec3(0x5555, 0x2222, 0xbbbb));
		A.store(gli::extent2d(0, 1), 0, glm::u16vec3(0xaaaa, 0xcccc, 0xeeee));

		gli::texture2d const B = gli::convert(A, gli::FORMAT_RGBA16_UNORM_PACK16);
		gli::texture2d const C = gli::convert(B, gli::FORMAT_RGB16_UNORM_PACK16);

		Error += A == C ? 0 : 1;

		return Error;
	}
}//namespace rgb16unorm

namespace rgb10a2norm
{
	int test()
	{
		int Error = 0;

		{
			gli::texture1d TextureSrc(gli::FORMAT_RGB10A2_UNORM_PACK32, gli::texture1d::extent_type(4));
			TextureSrc.clear(glm::u8vec4(255, 127, 0, 255));

			gli::texture1d TextureDst = gli::convert(TextureSrc, gli::FORMAT_RGB10A2_UNORM_PACK32);

			Error += TextureSrc == TextureDst ? 0 : 1;
		}

		{
			gli::texture1d_array TextureSrc(gli::FORMAT_RGB10A2_UNORM_PACK32, gli::texture1d_array::extent_type(4), 2);
			TextureSrc.clear(glm::u8vec4(255, 127, 0, 255));

			gli::texture1d_array TextureDst = gli::convert(TextureSrc, gli::FORMAT_RGB10A2_UNORM_PACK32);

			Error += TextureSrc == TextureDst ? 0 : 1;
		}

		{
			gli::texture2d TextureSrc(gli::FORMAT_RGB10A2_UNORM_PACK32, gli::texture2d::extent_type(4));
			TextureSrc.clear(glm::u8vec4(255, 127, 0, 255));

			gli::texture2d TextureDst = gli::convert(TextureSrc, gli::FORMAT_RGB10A2_UNORM_PACK32);

			Error += TextureSrc == TextureDst ? 0 : 1;
		}

		{
			gli::texture2d_array TextureSrc(gli::FORMAT_RGB10A2_UNORM_PACK32, gli::texture2d_array::extent_type(4), 2);
			TextureSrc.clear(glm::u8vec4(255, 127, 0, 255));

			gli::texture2d_array TextureDst = gli::convert(TextureSrc, gli::FORMAT_RGB10A2_UNORM_PACK32);

			Error += TextureSrc == TextureDst ? 0 : 1;
		}

		{
			gli::texture3d TextureSrc(gli::FORMAT_RGB10A2_UNORM_PACK32, gli::texture3d::extent_type(4));
			TextureSrc.clear(glm::u8vec4(255, 127, 0, 255));

			gli::texture3d TextureDst = gli::convert(TextureSrc, gli::FORMAT_RGB10A2_UNORM_PACK32);

			Error += TextureSrc == TextureDst ? 0 : 1;
		}

		{
			gli::texture_cube TextureSrc(gli::FORMAT_RGB10A2_UNORM_PACK32, gli::texture_cube::extent_type(4), 2);
			TextureSrc.clear(glm::u8vec4(255, 127, 0, 255));

			gli::texture_cube TextureDst = gli::convert(TextureSrc, gli::FORMAT_RGB10A2_UNORM_PACK32);

			Error += TextureSrc == TextureDst ? 0 : 1;
		}

		{
			gli::texture_cube_array TextureSrc(gli::FORMAT_RGB10A2_UNORM_PACK32, gli::texture_cube_array::extent_type(4), 2);
			TextureSrc.clear(glm::u8vec4(255, 127, 0, 255));

			gli::texture_cube_array TextureDst = gli::convert(TextureSrc, gli::FORMAT_RGB10A2_UNORM_PACK32);

			Error += TextureSrc == TextureDst ? 0 : 1;
		}

		return Error;
	}
}//namespace rgb10a2norm

namespace rgba_dxt1unorm
{
	int test()
	{
		int Error = 0;

		{
			gli::texture2d TextureSrc(gli::load_dds(path("kueken7_rgba_dxt1_unorm.dds")));
			GLI_ASSERT(!TextureSrc.empty());

			gli::texture2d TextureDst = gli::convert(TextureSrc, gli::FORMAT_RGBA8_UNORM_PACK8);
			gli::save_dds(TextureDst, "kueken7_rgba_dxt1_unorm_decompressed_a.dds");

			gli::texture2d TextureDecompressed(gli::load("kueken7_rgba_dxt1_unorm_decompressed_a.dds"));
			Error += TextureDecompressed == TextureDst ? 0 : 1;

			gli::texture2d TextureBlack(gli::duplicate(TextureDecompressed));
			TextureBlack.clear(glm::u8vec4(0, 0, 0, 255));
			Error += TextureDecompressed != TextureBlack ? 0 : 1;
		}

		{
			gli::texture2d TextureCompressed(gli::load(path("kueken7_rgba_dxt1_unorm.dds")));
			GLI_ASSERT(!TextureCompressed.empty());

			gli::texture2d TextureDecompressed(gli::load(path("kueken7_rgba_dxt1_unorm_decompressed.dds")));
			GLI_ASSERT(!TextureDecompressed.empty());

			GLI_ASSERT(TextureCompressed.extent() == TextureDecompressed.extent());
			GLI_ASSERT(TextureCompressed.levels() == TextureDecompressed.levels());

			gli::texture2d TextureLocalDecompressed(TextureDecompressed.format(), TextureDecompressed.extent(), TextureDecompressed.levels(), TextureDecompressed.swizzles());

			GLI_ASSERT(sizeof(gli::detail::dxt1_block) == gli::block_size(gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8));

			// decompress
			gli::extent2d BlockExtent;
			{
				gli::extent3d TempExtent = gli::block_extent(gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8);
				BlockExtent.x = TempExtent.x;
				BlockExtent.y = TempExtent.y;
			}

			for(size_t Level = 0; Level < TextureCompressed.levels(); ++Level)
			{
				gli::extent2d TexelCoord;
				gli::extent2d BlockCoord;
				gli::extent2d LevelExtent = TextureCompressed.extent(Level);
				gli::extent2d LevelExtentInBlocks = glm::max(gli::extent2d(1, 1), LevelExtent / BlockExtent);
				for(BlockCoord.y = 0, TexelCoord.y = 0; BlockCoord.y < LevelExtentInBlocks.y; ++BlockCoord.y, TexelCoord.y += BlockExtent.y)
				{
					for(BlockCoord.x = 0, TexelCoord.x = 0; BlockCoord.x < LevelExtentInBlocks.x; ++BlockCoord.x, TexelCoord.x += BlockExtent.x)
					{
						const gli::detail::dxt1_block *DXT1Block = TextureCompressed.data<gli::detail::dxt1_block>(0, 0, Level) + (BlockCoord.y * LevelExtentInBlocks.x + BlockCoord.x);
						const gli::detail::texel_block4x4 DecompressedBlock = gli::detail::decompress_dxt1_block(*DXT1Block);

						gli::extent2d DecompressedBlockCoord;
						for(DecompressedBlockCoord.y = 0; DecompressedBlockCoord.y < glm::min(4, LevelExtent.y); ++DecompressedBlockCoord.y)
						{
							for(DecompressedBlockCoord.x = 0; DecompressedBlockCoord.x < glm::min(4, LevelExtent.x); ++DecompressedBlockCoord.x)
							{
								TextureLocalDecompressed.store(TexelCoord + DecompressedBlockCoord, Level, glm::u8vec4(glm::round(DecompressedBlock.Texel[DecompressedBlockCoord.y][DecompressedBlockCoord.x] * 255.0f)));
							}
						}
					}
				}
			}

			Error += (TextureDecompressed == TextureLocalDecompressed) ? 0 : 1;
		
			// Test converting through the convertFunc interface
			// sampling at the corners of each level
			for(int Level = 0; Level < TextureDecompressed.levels(); ++Level)
			{
				gli::extent2d TexelCoord;
				gli::extent2d LevelExtent = TextureDecompressed.extent(Level);
				for(TexelCoord.y = 0; TexelCoord.y < LevelExtent.y; TexelCoord.y += glm::max(1, LevelExtent.y - 1))
				{
					for(TexelCoord.x = 0; TexelCoord.x < LevelExtent.x; TexelCoord.x += glm::max(1, LevelExtent.x - 1))
					{
						glm::u8vec4 ColorFromCompressed = glm::u8vec4(glm::round(gli::detail::convertFunc<gli::texture2d, float, 4, std::uint8_t, glm::defaultp, gli::detail::CONVERT_MODE_DXT1UNORM, true>::fetch(TextureCompressed, TexelCoord, 0, 0, Level) * 255.0f));
						glm::u8vec4 ColorFromDecompressed = TextureDecompressed.load<glm::u8vec4>(TexelCoord, Level);

						Error += (ColorFromCompressed == ColorFromDecompressed) ? 0 : 1;
					}
				}
			}
		}

		return Error;
	}
}//namespace rgba_dxt1unorm

namespace rgba_dxt3unorm
{
	int test()
	{
		int Error = 0;

		{
			gli::texture2d TextureSrc(gli::load_dds(path("kueken7_rgba_dxt3_unorm.dds")));
			GLI_ASSERT(!TextureSrc.empty());

			gli::texture2d TextureDst = gli::convert(TextureSrc, gli::FORMAT_RGBA8_UNORM_PACK8);
			gli::save_dds(TextureDst, "kueken7_rgba_dxt3_unorm_decompressed_a.dds");

			gli::texture2d TextureDecompressed(gli::load("kueken7_rgba_dxt3_unorm_decompressed_a.dds"));
			Error += TextureDecompressed == TextureDst ? 0 : 1;

			gli::texture2d TextureBlack(gli::duplicate(TextureDecompressed));
			TextureBlack.clear(glm::u8vec4(0, 0, 0, 255));
			Error += TextureDecompressed != TextureBlack ? 0 : 1;
		}

		{
			gli::texture2d TextureCompressed(gli::load(path("kueken7_rgba_dxt3_unorm.dds")));
			GLI_ASSERT(!TextureCompressed.empty());

			gli::texture2d TextureDecompressed(gli::load(path("kueken7_rgba_dxt3_unorm_decompressed.dds")));
			GLI_ASSERT(!TextureDecompressed.empty());

			GLI_ASSERT(TextureCompressed.extent() == TextureDecompressed.extent());
			GLI_ASSERT(TextureCompressed.levels() == TextureDecompressed.levels());

			gli::texture2d TextureLocalDecompressed(TextureDecompressed.format(), TextureDecompressed.extent(), TextureDecompressed.levels(), TextureDecompressed.swizzles());

			GLI_ASSERT(sizeof(gli::detail::dxt3_block) == gli::block_size(gli::FORMAT_RGBA_DXT3_UNORM_BLOCK16));

			// decompress
			gli::extent2d BlockExtent;
			{
				gli::extent3d TempExtent = gli::block_extent(gli::FORMAT_RGBA_DXT3_UNORM_BLOCK16);
				BlockExtent.x = TempExtent.x;
				BlockExtent.y = TempExtent.y;
			}

			for(size_t Level = 0; Level < TextureCompressed.levels(); ++Level)
			{
				gli::extent2d TexelCoord;
				gli::extent2d BlockCoord;
				gli::extent2d LevelExtent = TextureCompressed.extent(Level);
				gli::extent2d LevelExtentInBlocks = glm::max(gli::extent2d(1, 1), LevelExtent / BlockExtent);
				for(BlockCoord.y = 0, TexelCoord.y = 0; BlockCoord.y < LevelExtentInBlocks.y; ++BlockCoord.y, TexelCoord.y += BlockExtent.y)
				{
					for(BlockCoord.x = 0, TexelCoord.x = 0; BlockCoord.x < LevelExtentInBlocks.x; ++BlockCoord.x, TexelCoord.x += BlockExtent.x)
					{
						const gli::detail::dxt3_block *DXT3Block = TextureCompressed.data<gli::detail::dxt3_block>(0, 0, Level) + (BlockCoord.y * LevelExtentInBlocks.x + BlockCoord.x);
						const gli::detail::texel_block4x4 DecompressedBlock = gli::detail::decompress_dxt3_block(*DXT3Block);

						gli::extent2d DecompressedBlockCoord;
						for(DecompressedBlockCoord.y = 0; DecompressedBlockCoord.y < glm::min(4, LevelExtent.y); ++DecompressedBlockCoord.y)
						{
							for(DecompressedBlockCoord.x = 0; DecompressedBlockCoord.x < glm::min(4, LevelExtent.x); ++DecompressedBlockCoord.x)
							{
								TextureLocalDecompressed.store(TexelCoord + DecompressedBlockCoord, Level, glm::u8vec4(glm::round(DecompressedBlock.Texel[DecompressedBlockCoord.y][DecompressedBlockCoord.x] * 255.0f)));
							}
						}
					}
				}
			}

			Error += (TextureDecompressed == TextureLocalDecompressed) ? 0 : 1;

			// Test converting through the convertFunc interface
			// sampling at the corners of each level
			for(int Level = 0; Level < TextureDecompressed.levels(); ++Level)
			{
				gli::extent2d TexelCoord;
				gli::extent2d LevelExtent = TextureDecompressed.extent(Level);
				for(TexelCoord.y = 0; TexelCoord.y < LevelExtent.y; TexelCoord.y += glm::max(1, LevelExtent.y - 1))
				{
					for(TexelCoord.x = 0; TexelCoord.x < LevelExtent.x; TexelCoord.x += glm::max(1, LevelExtent.x - 1))
					{
						glm::u8vec4 ColorFromCompressed = glm::u8vec4(glm::round(gli::detail::convertFunc<gli::texture2d, float, 4, std::uint8_t, glm::defaultp, gli::detail::CONVERT_MODE_DXT3UNORM, true>::fetch(TextureCompressed, TexelCoord, 0, 0, Level) * 255.0f));
						glm::u8vec4 ColorFromDecompressed = TextureDecompressed.load<glm::u8vec4>(TexelCoord, Level);

						Error += (ColorFromCompressed == ColorFromDecompressed) ? 0 : 1;
					}
				}
			}
		}

		return Error;
	}
}//namespace rgba_dxt3unorm

namespace rgba_dxt5unorm
{
	int test()
	{
		int Error = 0;

		{
			gli::texture2d TextureSrc(gli::load_dds(path("kueken7_rgba_dxt5_unorm.dds")));
			GLI_ASSERT(!TextureSrc.empty());

			gli::texture2d TextureDst = gli::convert(TextureSrc, gli::FORMAT_RGBA8_UNORM_PACK8);
			gli::save_dds(TextureDst, "kueken7_rgba_dxt5_unorm_decompressed_a.dds");

			gli::texture2d TextureDecompressed(gli::load("kueken7_rgba_dxt5_unorm_decompressed_a.dds"));
			Error += TextureDecompressed == TextureDst ? 0 : 1;

			gli::texture2d TextureBlack(gli::duplicate(TextureDecompressed));
			TextureBlack.clear(glm::u8vec4(0, 0, 0, 255));
			Error += TextureDecompressed != TextureBlack ? 0 : 1;
		}

		{
			gli::texture2d TextureCompressed(gli::load(path("kueken7_rgba_dxt5_unorm.dds")));
			GLI_ASSERT(!TextureCompressed.empty());

			gli::texture2d TextureDecompressed(gli::load(path("kueken7_rgba_dxt5_unorm_decompressed.dds")));
			GLI_ASSERT(!TextureDecompressed.empty());

			GLI_ASSERT(TextureCompressed.extent() == TextureDecompressed.extent());
			GLI_ASSERT(TextureCompressed.levels() == TextureDecompressed.levels());

			gli::texture2d TextureLocalDecompressed(TextureDecompressed.format(), TextureDecompressed.extent(), TextureDecompressed.levels(), TextureDecompressed.swizzles());

			GLI_ASSERT(sizeof(gli::detail::dxt5_block) == gli::block_size(gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16));

			// decompress
			gli::extent2d BlockExtent;
			{
				gli::extent3d TempExtent = gli::block_extent(gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16);
				BlockExtent.x = TempExtent.x;
				BlockExtent.y = TempExtent.y;
			}

			for(size_t Level = 0; Level < TextureCompressed.levels(); ++Level)
			{
				gli::extent2d TexelCoord;
				gli::extent2d BlockCoord;
				gli::extent2d LevelExtent = TextureCompressed.extent(Level);
				gli::extent2d LevelExtentInBlocks = glm::max(gli::extent2d(1, 1), LevelExtent / BlockExtent);
				for(BlockCoord.y = 0, TexelCoord.y = 0; BlockCoord.y < LevelExtentInBlocks.y; ++BlockCoord.y, TexelCoord.y += BlockExtent.y)
				{
					for(BlockCoord.x = 0, TexelCoord.x = 0; BlockCoord.x < LevelExtentInBlocks.x; ++BlockCoord.x, TexelCoord.x += BlockExtent.x)
					{
						const gli::detail::dxt5_block *DXT5Block = TextureCompressed.data<gli::detail::dxt5_block>(0, 0, Level) + (BlockCoord.y * LevelExtentInBlocks.x + BlockCoord.x);
						const gli::detail::texel_block4x4 DecompressedBlock = gli::detail::decompress_dxt5_block(*DXT5Block);

						gli::extent2d DecompressedBlockCoord;
						for(DecompressedBlockCoord.y = 0; DecompressedBlockCoord.y < glm::min(4, LevelExtent.y); ++DecompressedBlockCoord.y)
						{
							for(DecompressedBlockCoord.x = 0; DecompressedBlockCoord.x < glm::min(4, LevelExtent.x); ++DecompressedBlockCoord.x)
							{
								TextureLocalDecompressed.store(TexelCoord + DecompressedBlockCoord, Level, glm::u8vec4(glm::round(DecompressedBlock.Texel[DecompressedBlockCoord.y][DecompressedBlockCoord.x] * 255.0f)));
							}
						}
					}
				}
			}

			Error += (TextureDecompressed == TextureLocalDecompressed) ? 0 : 1;

			// Test converting through the convertFunc interface
			// sampling at the corners of each level
			for(int Level = 0; Level < TextureDecompressed.levels(); ++Level)
			{
				gli::extent2d TexelCoord;
				gli::extent2d LevelExtent = TextureDecompressed.extent(Level);
				for(TexelCoord.y = 0; TexelCoord.y < LevelExtent.y; TexelCoord.y += glm::max(1, LevelExtent.y - 1))
				{
					for(TexelCoord.x = 0; TexelCoord.x < LevelExtent.x; TexelCoord.x += glm::max(1, LevelExtent.x - 1))
					{
						glm::u8vec4 ColorFromCompressed = glm::u8vec4(glm::round(gli::detail::convertFunc<gli::texture2d, float, 4, std::uint8_t, glm::defaultp, gli::detail::CONVERT_MODE_DXT5UNORM, true>::fetch(TextureCompressed, TexelCoord, 0, 0, Level) * 255.0f));
						glm::u8vec4 ColorFromDecompressed = TextureDecompressed.load<glm::u8vec4>(TexelCoord, Level);

						Error += (ColorFromCompressed == ColorFromDecompressed) ? 0 : 1;
					}
				}
			}
		}

		return Error;
	}
}//namespace rgba_dxt5unorm

namespace r_bc4unorm
{
	int test()
	{
		int Error = 0;

		{
			gli::texture2d TextureSrc(gli::load_dds(path("kueken7_r_ati1n_unorm.dds")));
			GLI_ASSERT(!TextureSrc.empty());

			gli::texture2d TextureDst = gli::convert(TextureSrc, gli::FORMAT_R8_UNORM_PACK8);
			gli::save_dds(TextureDst, "kueken7_r_ati1n_unorm_decompressed.dds");

			gli::texture2d TextureDecompressed(gli::load("kueken7_r_ati1n_unorm_decompressed.dds"));
			Error += TextureDecompressed == TextureDst ? 0 : 1;

			gli::texture2d TextureBlack(gli::duplicate(TextureDecompressed));
			TextureBlack.clear(glm::u8vec1(0));
			Error += TextureDecompressed != TextureBlack ? 0 : 1;
		}

		{
			gli::texture2d TextureCompressed(gli::load(path("kueken7_r_ati1n_unorm.dds")));
			GLI_ASSERT(!TextureCompressed.empty());

			gli::texture2d TextureDecompressed(gli::load(path("kueken7_r_ati1n_unorm_decompressed.dds")));
			GLI_ASSERT(!TextureDecompressed.empty());

			GLI_ASSERT(TextureCompressed.extent() == TextureDecompressed.extent());
			GLI_ASSERT(TextureCompressed.levels() == TextureDecompressed.levels());

			gli::texture2d TextureLocalDecompressed(TextureDecompressed.format(), TextureDecompressed.extent(), TextureDecompressed.levels(), TextureDecompressed.swizzles());

			GLI_ASSERT(sizeof(gli::detail::bc4_block) == gli::block_size(gli::FORMAT_R_ATI1N_UNORM_BLOCK8));

			// decompress
			gli::extent2d BlockExtent;
			{
				gli::extent3d TempExtent = gli::block_extent(gli::FORMAT_R_ATI1N_UNORM_BLOCK8);
				BlockExtent.x = TempExtent.x;
				BlockExtent.y = TempExtent.y;
			}

			for(size_t Level = 0; Level < TextureCompressed.levels(); ++Level)
			{
				gli::extent2d TexelCoord;
				gli::extent2d BlockCoord;
				gli::extent2d LevelExtent = TextureCompressed.extent(Level);
				gli::extent2d LevelExtentInBlocks = glm::max(gli::extent2d(1, 1), LevelExtent / BlockExtent);
				for(BlockCoord.y = 0, TexelCoord.y = 0; BlockCoord.y < LevelExtentInBlocks.y; ++BlockCoord.y, TexelCoord.y += BlockExtent.y)
				{
					for(BlockCoord.x = 0, TexelCoord.x = 0; BlockCoord.x < LevelExtentInBlocks.x; ++BlockCoord.x, TexelCoord.x += BlockExtent.x)
					{
						const gli::detail::bc4_block *BC4Block = TextureCompressed.data<gli::detail::bc4_block>(0, 0, Level) + (BlockCoord.y * LevelExtentInBlocks.x + BlockCoord.x);
						const gli::detail::texel_block4x4 DecompressedBlock = gli::detail::decompress_bc4unorm_block(*BC4Block);

						gli::extent2d DecompressedBlockCoord;
						for(DecompressedBlockCoord.y = 0; DecompressedBlockCoord.y < glm::min(4, LevelExtent.y); ++DecompressedBlockCoord.y)
						{
							for(DecompressedBlockCoord.x = 0; DecompressedBlockCoord.x < glm::min(4, LevelExtent.x); ++DecompressedBlockCoord.x)
							{
								glm::u8vec4 rgbaTexel(glm::round(DecompressedBlock.Texel[DecompressedBlockCoord.y][DecompressedBlockCoord.x] * 255.0f));							
								TextureLocalDecompressed.store(TexelCoord + DecompressedBlockCoord, Level, rgbaTexel.r);
							}
						}
					}
				}
			}

			Error += (TextureDecompressed == TextureLocalDecompressed) ? 0 : 1;

			// Test converting through the convertFunc interface
			// sampling at the corners of each level
			for(int Level = 0; Level < TextureDecompressed.levels(); ++Level)
			{
				gli::extent2d TexelCoord;
				gli::extent2d LevelExtent = TextureDecompressed.extent(Level);
				for(TexelCoord.y = 0; TexelCoord.y < LevelExtent.y; TexelCoord.y += glm::max(1, LevelExtent.y - 1))
				{
					for(TexelCoord.x = 0; TexelCoord.x < LevelExtent.x; TexelCoord.x += glm::max(1, LevelExtent.x - 1))
					{
						glm::u8vec4 ColorFromCompressed = glm::u8vec4(glm::round(gli::detail::convertFunc<gli::texture2d, float, 4, std::uint8_t, glm::defaultp, gli::detail::CONVERT_MODE_BC4UNORM, true>::fetch(TextureCompressed, TexelCoord, 0, 0, Level) * 255.0f));
						glm::u8 ColorFromDecompressed = TextureDecompressed.load<glm::u8>(TexelCoord, Level);

						Error += (ColorFromCompressed.r == ColorFromDecompressed) ? 0 : 1;
					}
				}
			}
		}

		return Error;
	}
}//namespace r_bc4unorm

namespace rg_bc5unorm
{
	int test()
	{
		int Error = 0;

		{
			gli::texture2d TextureSrc(gli::load_dds(path("kueken7_rg_ati2n_unorm.dds")));
			GLI_ASSERT(!TextureSrc.empty());

			gli::texture2d TextureDst = gli::convert(TextureSrc, gli::FORMAT_RG8_UNORM_PACK8);
			gli::save_dds(TextureDst, "kueken7_rg_ati2n_unorm_decompressed.dds");

			gli::texture2d TextureDecompressed(gli::load("kueken7_rg_ati2n_unorm_decompressed.dds"));
			Error += TextureDecompressed == TextureDst ? 0 : 1;

			gli::texture2d TextureBlack(gli::duplicate(TextureDecompressed));
			TextureBlack.clear(glm::u8vec2(0, 0));
			Error += TextureDecompressed != TextureBlack ? 0 : 1;
		}

		{
			gli::texture2d TextureCompressed(gli::load(path("kueken7_rg_ati2n_unorm.dds")));
			GLI_ASSERT(!TextureCompressed.empty());

			gli::texture2d TextureDecompressed(gli::load(path("kueken7_rg_ati2n_unorm_decompressed.dds")));
			GLI_ASSERT(!TextureDecompressed.empty());

			GLI_ASSERT(TextureCompressed.extent() == TextureDecompressed.extent());
			GLI_ASSERT(TextureCompressed.levels() == TextureDecompressed.levels());

			gli::texture2d TextureLocalDecompressed(TextureDecompressed.format(), TextureDecompressed.extent(), TextureDecompressed.levels(), TextureDecompressed.swizzles());

			GLI_ASSERT(sizeof(gli::detail::bc5_block) == gli::block_size(gli::FORMAT_RG_ATI2N_UNORM_BLOCK16));

			// decompress
			gli::extent2d BlockExtent;
			{
				gli::extent3d TempExtent = gli::block_extent(gli::FORMAT_RG_ATI2N_UNORM_BLOCK16);
				BlockExtent.x = TempExtent.x;
				BlockExtent.y = TempExtent.y;
			}

			uint32_t offByOne = 0;
			uint32_t offByAlot = 0;
			uint32_t onPoint = 0;

			for(size_t Level = 0; Level < TextureCompressed.levels(); ++Level)
			{
				gli::extent2d TexelCoord;
				gli::extent2d BlockCoord;
				gli::extent2d LevelExtent = TextureCompressed.extent(Level);
				gli::extent2d LevelExtentInBlocks = glm::max(gli::extent2d(1, 1), LevelExtent / BlockExtent);
				for(BlockCoord.y = 0, TexelCoord.y = 0; BlockCoord.y < LevelExtentInBlocks.y; ++BlockCoord.y, TexelCoord.y += BlockExtent.y)
				{
					for(BlockCoord.x = 0, TexelCoord.x = 0; BlockCoord.x < LevelExtentInBlocks.x; ++BlockCoord.x, TexelCoord.x += BlockExtent.x)
					{
						const gli::detail::bc5_block *BC5Block = TextureCompressed.data<gli::detail::bc5_block>(0, 0, Level) + (BlockCoord.y * LevelExtentInBlocks.x + BlockCoord.x);
						const gli::detail::texel_block4x4 DecompressedBlock = gli::detail::decompress_bc5unorm_block(*BC5Block);

						gli::extent2d DecompressedBlockCoord;
						for(DecompressedBlockCoord.y = 0; DecompressedBlockCoord.y < glm::min(4, LevelExtent.y); ++DecompressedBlockCoord.y)
						{
							for(DecompressedBlockCoord.x = 0; DecompressedBlockCoord.x < glm::min(4, LevelExtent.x); ++DecompressedBlockCoord.x)
							{
								glm::u8vec4 rgbaTexel(glm::round(DecompressedBlock.Texel[DecompressedBlockCoord.y][DecompressedBlockCoord.x] * 255.0f));
							
								glm::u8vec2 decompressed = TextureDecompressed.load<glm::u8vec2>(TexelCoord + DecompressedBlockCoord, Level);

								if(glm::xy(rgbaTexel) == decompressed)
								{
									++onPoint;
								} else if(glm::abs((int32_t)rgbaTexel.r - (int32_t)decompressed.r) > 1 || glm::abs((int32_t)rgbaTexel.g - (int32_t)decompressed.g) > 1)
								{
									++offByAlot;
								} else
								{
									++offByOne;
								}

								TextureLocalDecompressed.store(TexelCoord + DecompressedBlockCoord, Level, glm::xy(rgbaTexel));
							}
						}
					}
				}
			}

			Error += (TextureDecompressed == TextureLocalDecompressed) ? 0 : 1;

			// Test converting through the convertFunc interface
			// sampling at the corners of each level
			for(int Level = 0; Level < TextureDecompressed.levels(); ++Level)
			{
				gli::extent2d TexelCoord;
				gli::extent2d LevelExtent = TextureDecompressed.extent(Level);
				for(TexelCoord.y = 0; TexelCoord.y < LevelExtent.y; TexelCoord.y += glm::max(1, LevelExtent.y - 1))
				{
					for(TexelCoord.x = 0; TexelCoord.x < LevelExtent.x; TexelCoord.x += glm::max(1, LevelExtent.x - 1))
					{
						glm::u8vec4 ColorFromCompressed = glm::u8vec4(glm::round(gli::detail::convertFunc<gli::texture2d, float, 4, std::uint8_t, glm::defaultp, gli::detail::CONVERT_MODE_BC5UNORM, true>::fetch(TextureCompressed, TexelCoord, 0, 0, Level) * 255.0f));
						glm::u8vec2 ColorFromDecompressed = TextureDecompressed.load<glm::u8vec2>(TexelCoord, Level);

						Error += (glm::xy(ColorFromCompressed) == ColorFromDecompressed) ? 0 : 1;
					}
				}
			}
		}

		return Error;
	}
}//namespace rg_bc5unorm

namespace load_file
{
	int test()
	{
		int Error = 0;

		gli::texture2d TextureA(gli::load(path("kueken7_rgba16_sfloat.ktx")));
		GLI_ASSERT(!TextureA.empty());

		gli::texture2d Convert = gli::convert(TextureA, gli::FORMAT_RG11B10_UFLOAT_PACK32);

		gli::save(Convert, "kueken7_rg11b10_ufloat.dds");
		gli::save(Convert, "kueken7_rg11b10_ufloat.ktx");

		gli::texture2d TextureDDS(gli::load("kueken7_rg11b10_ufloat.dds"));
		GLI_ASSERT(!TextureDDS.empty());
		gli::texture2d TextureKTX(gli::load("kueken7_rg11b10_ufloat.ktx"));
		GLI_ASSERT(!TextureKTX.empty());

		Error += TextureDDS == TextureKTX ? 0 : 1;
		Error += TextureDDS == Convert ? 0 : 1;

		GLI_ASSERT(!Error);

		return Error;
	}
}//namespace load_file

int main()
{
	int Error = 0;

	Error += load_file::test();
	Error += r8unorm::test();
	Error += rg8unorm::test();
	Error += rgb8unorm::test();
	Error += r16unorm::test();
	Error += rg16unorm::test();
	Error += rgb16unorm::test();
	Error += rgb10a2norm::test();
	Error += rgba_dxt1unorm::test();
	Error += rgba_dxt3unorm::test();
	Error += rgba_dxt5unorm::test();
	Error += r_bc4unorm::test();
	Error += rg_bc5unorm::test();

	return Error;
}

