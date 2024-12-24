#include <gli/gli.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/vec1.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/color_space.hpp>
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

namespace load_file
{
	int test(params const & Params)
	{
		int Error(0);

		gli::texture TextureA(gli::load_dds(path(Params.Filename.c_str())));
		Error += TextureA.format() == Params.Format ? 0 : 1;
		GLI_ASSERT(!Error);

		gli::save_dds(TextureA, Params.Filename.c_str());
		gli::texture TextureB(gli::load_dds(Params.Filename.c_str()));
		Error += TextureB.format() == Params.Format ? 0 : 1;
		GLI_ASSERT(!Error);

		Error += TextureA == TextureB ? 0 : 1;
		GLI_ASSERT(!Error);

		return Error;
	}
}//namespace load_file

namespace load_mem
{
	int test(params const & Params)
	{
		int Error(0);

		gli::texture TextureA(gli::load_dds(path(Params.Filename.c_str())));
		Error += TextureA.format() == Params.Format ? 0 : 1;
		GLI_ASSERT(!Error);

		gli::save_dds(TextureA, Params.Filename.c_str());
		gli::texture TextureB(gli::load_dds(Params.Filename.c_str()));
		Error += TextureB.format() == Params.Format ? 0 : 1;
		GLI_ASSERT(!Error);

		Error += TextureA == TextureB ? 0 : 1;
		GLI_ASSERT(!Error);

		return Error;
	}
}//namespace load_mem

namespace load_mem_only
{
	int test(std::vector<char> const & Data, params const & Params)
	{
		int Error(0);

		gli::texture TextureA(gli::load_dds(&Data[0], Data.size()));
		Error += TextureA.format() == Params.Format ? 0 : 1;
		GLI_ASSERT(!Error);

		std::vector<char> Memory;
		gli::save_dds(TextureA, Memory);
		gli::texture TextureB(gli::load_dds(&Memory[0], Memory.size()));
		Error += TextureB.format() == Params.Format ? 0 : 1;
		GLI_ASSERT(!Error);

		Error += TextureA == TextureB ? 0 : 1;
		GLI_ASSERT(!Error);

		return Error;
	}
}//namespace load_mem_only

int main()
{
	std::vector<params> Params;

	// GLI DDS extensions:
	//Params.push_back(params("kueken7_rgb_etc2_srgb.dds", gli::FORMAT_RGB_ETC2_SRGB_BLOCK8));
	//Params.push_back(params("kueken7_rgb_etc2_unorm.dds", gli::FORMAT_RGB_ETC2_UNORM_BLOCK8));
	//Params.push_back(params("kueken7_rgba_pvrtc2_4bpp_unorm.dds", gli::FORMAT_RGBA_PVRTC2_4X4_UNORM_BLOCK8));

	Params.push_back(params("kueken7_bgrx8_unorm.dds", gli::FORMAT_BGR8_UNORM_PACK32));
	Params.push_back(params("kueken7_rgba_dxt5_unorm1.dds", gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16));
	Params.push_back(params("kueken7_rgba_dxt5_unorm2.dds", gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16));
	Params.push_back(params("array_r8_uint.dds", gli::FORMAT_R8_UINT_PACK8));
	Params.push_back(params("kueken7_rgba_astc4x4_srgb.dds", gli::FORMAT_RGBA_ASTC_4X4_SRGB_BLOCK16));
	Params.push_back(params("kueken7_bgra8_srgb.dds", gli::FORMAT_BGRA8_SRGB_PACK8));
	Params.push_back(params("kueken7_r16_unorm.dds", gli::FORMAT_R16_UINT_PACK16));
	Params.push_back(params("kueken7_r8_sint.dds", gli::FORMAT_R8_SINT_PACK8));
	Params.push_back(params("kueken7_r8_uint.dds", gli::FORMAT_R8_UINT_PACK8));
	Params.push_back(params("kueken7_rgba4_unorm.dds", gli::FORMAT_BGRA4_UNORM_PACK16));
	Params.push_back(params("kueken7_r5g6b5_unorm.dds", gli::FORMAT_B5G6R5_UNORM_PACK16));
	Params.push_back(params("kueken7_rgb5a1_unorm.dds", gli::FORMAT_BGR5A1_UNORM_PACK16));
	Params.push_back(params("kueken7_rgba_dxt1_unorm.dds", gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8));
	Params.push_back(params("kueken7_rgba_dxt1_srgb.dds", gli::FORMAT_RGBA_DXT1_SRGB_BLOCK8));
	Params.push_back(params("kueken8_rgba_dxt1_unorm.dds", gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8));
	Params.push_back(params("kueken7_rgba_dxt5_unorm.dds", gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16));
	Params.push_back(params("kueken7_rgba_dxt5_srgb.dds", gli::FORMAT_RGBA_DXT5_SRGB_BLOCK16));
	Params.push_back(params("kueken7_rgb_etc1_unorm.dds", gli::FORMAT_RGB_ETC_UNORM_BLOCK8));
	Params.push_back(params("kueken7_rgb_atc_unorm.dds", gli::FORMAT_RGB_ATC_UNORM_BLOCK8));
	Params.push_back(params("kueken7_rgba_atc_explicit_unorm.dds", gli::FORMAT_RGBA_ATCA_UNORM_BLOCK16));
	Params.push_back(params("kueken7_rgba_atc_interpolate_unorm.dds", gli::FORMAT_RGBA_ATCI_UNORM_BLOCK16));
	Params.push_back(params("kueken7_rgb_pvrtc_2bpp_unorm.dds", gli::FORMAT_RGB_PVRTC1_16X8_UNORM_BLOCK32));
	Params.push_back(params("kueken7_rgb_pvrtc_4bpp_unorm.dds", gli::FORMAT_RGB_PVRTC1_8X8_UNORM_BLOCK32));
	Params.push_back(params("kueken7_r_ati1n_unorm.dds", gli::FORMAT_R_ATI1N_UNORM_BLOCK8));
	Params.push_back(params("kueken7_rg_ati2n_unorm.dds", gli::FORMAT_RG_ATI2N_UNORM_BLOCK16));
	Params.push_back(params("kueken7_bgr8_unorm.dds", gli::FORMAT_BGR8_UNORM_PACK8));
	Params.push_back(params("kueken7_rgba8_srgb.dds", gli::FORMAT_RGBA8_SRGB_PACK8));
	Params.push_back(params("kueken7_bgra8_unorm.dds", gli::FORMAT_BGRA8_UNORM_PACK8));
	Params.push_back(params("kueken7_a8_unorm.dds", gli::FORMAT_A8_UNORM_PACK8));
	Params.push_back(params("kueken7_l8_unorm.dds", gli::FORMAT_L8_UNORM_PACK8));
	Params.push_back(params("kueken7_rgb10a2_unorm.dds", gli::FORMAT_RGB10A2_UNORM_PACK32));
	Params.push_back(params("kueken7_rgb10a2u.dds", gli::FORMAT_RGB10A2_UINT_PACK32));
	Params.push_back(params("kueken7_rgba8_snorm.dds", gli::FORMAT_RGBA8_SNORM_PACK8));
	Params.push_back(params("kueken7_rgba16_sfloat.dds", gli::FORMAT_RGBA16_SFLOAT_PACK16));
	Params.push_back(params("kueken7_rg11b10_ufloat.dds", gli::FORMAT_RG11B10_UFLOAT_PACK32));
	Params.push_back(params("kueken7_rgb9e5_ufloat.dds", gli::FORMAT_RGB9E5_UFLOAT_PACK32));

	int Error(0);

	std::clock_t TimeFileStart = std::clock();
	{
		for(std::size_t Index = 0; Index < Params.size(); ++Index)
			Error += load_file::test(Params[Index]);
	}
	std::clock_t TimeFileEnd = std::clock();

	std::clock_t TimeMemStart = std::clock();
	{
		for(std::size_t Index = 0; Index < Params.size(); ++Index)
			Error += load_mem::test(Params[Index]);
	}
	std::clock_t TimeMemEnd = std::clock();

	std::clock_t TimeMemOnlyStart = 0;
	{
		std::vector<std::vector<char> > Memory(Params.size());

		for(std::size_t Index = 0; Index < Params.size(); ++Index)
		{
			FILE* File = std::fopen(Params[Index].Filename.c_str(), "rb");
			GLI_ASSERT(File);

			long Beg = std::ftell(File);
			std::fseek(File, 0, SEEK_END);
			long End = std::ftell(File);
			std::fseek(File, 0, SEEK_SET);

			Memory[Index].resize(End - Beg);

			std::fread(&Memory[Index][0], 1, Memory[Index].size(), File);
			std::fclose(File);
		}

		TimeMemOnlyStart = std::clock();

		for(std::size_t Index = 0; Index < Params.size(); ++Index)
			Error += load_mem_only::test(Memory[Index], Params[Index]);
	}
	std::clock_t TimeMemOnlyEnd = std::clock();

	std::printf("File: %lu, Mem: %lu, Mem Only: %lu\n", TimeFileEnd - TimeFileStart, TimeMemEnd - TimeMemStart, TimeMemOnlyEnd - TimeMemOnlyStart);

	return Error;
}
