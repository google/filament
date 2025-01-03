#include <gli/core/storage_linear.hpp>
#include <gli/format.hpp>

namespace layers
{
	struct test
	{
		test
		(
			gli::storage_linear::extent_type const & Dimensions,
			gli::format const & Format,
			std::size_t const & BaseOffset,
			std::size_t const & Size
		) :
			Dimensions(Dimensions),
			Format(Format),
			BaseOffset(BaseOffset),
			Size(Size)
		{}

		gli::storage_linear::extent_type Dimensions;
		gli::format Format;
		std::size_t BaseOffset;
		std::size_t Size;
	};

	int run()
	{
		int Error(0);

		std::vector<test> Tests;
		Tests.push_back(test(gli::storage_linear::extent_type(4, 4, 1), gli::FORMAT_RGBA8_UINT_PACK8, 64, 128));
		Tests.push_back(test(gli::storage_linear::extent_type(4, 4, 1), gli::FORMAT_RGB16_SFLOAT_PACK16, 96, 192));
		Tests.push_back(test(gli::storage_linear::extent_type(4, 4, 1), gli::FORMAT_RGBA32_SFLOAT_PACK32, 256, 512));
		Tests.push_back(test(gli::storage_linear::extent_type(4, 4, 1), gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, 8, 16));
		Tests.push_back(test(gli::storage_linear::extent_type(8, 8, 1), gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, 32, 64));
		Tests.push_back(test(gli::storage_linear::extent_type(4, 4, 1), gli::FORMAT_R_ATI1N_SNORM_BLOCK8, 8, 16));

		for(std::size_t i = 0; i < Tests.size(); ++i)
		{
			gli::storage_linear Storage(
				Tests[i].Format,
				Tests[i].Dimensions,
				2,
				1,
				1);

			gli::storage_linear::size_type const BaseOffset = Storage.base_offset(1, 0, 0);
			gli::storage_linear::size_type const Size = Storage.size();

			Error += BaseOffset == Tests[i].BaseOffset ? 0 : 1;
			Error += Size == Tests[i].Size ? 0 : 1;
		}

		return Error;
	}
}//namespace layers

namespace faces
{
	struct test
	{
		test
		(
			gli::format const & Format,
			std::size_t const & Level,
			std::size_t const & BaseOffset,
			std::size_t const & Size
		) :
			Format(Format),
			Level(Level),
			BaseOffset(BaseOffset),
			Size(Size)
		{}

		gli::format Format;
		std::size_t Level;
		std::size_t BaseOffset;
		std::size_t Size;
	};

	int run()
	{
		int Error(0);

		std::vector<test> Tests;
		Tests.push_back(test(gli::FORMAT_RGBA8_UINT_PACK8, 0, 0, 340));
		Tests.push_back(test(gli::FORMAT_RGBA8_UINT_PACK8, 1, 256, 340));
		Tests.push_back(test(gli::FORMAT_R8_UINT_PACK8, 1, 64, 85));
		Tests.push_back(test(gli::FORMAT_RGBA8_UINT_PACK8, 3, 336, 340));
		Tests.push_back(test(gli::FORMAT_RGBA32_SFLOAT_PACK32, 0, 0, 1360));
		Tests.push_back(test(gli::FORMAT_RGBA32_SFLOAT_PACK32, 1, 1024, 1360));
		Tests.push_back(test(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, 0, 0, 56));
		Tests.push_back(test(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, 1, 32, 56));
		Tests.push_back(test(gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16, 1, 64, 112));

		for(std::size_t i = 0; i < Tests.size(); ++i)
		{
			gli::storage_linear Storage(Tests[i].Format, gli::storage_linear::extent_type(8, 8, 1), 1, 1, 4);
			gli::storage_linear::size_type BaseOffset = Storage.base_offset(0, 0, Tests[i].Level);
			gli::storage_linear::size_type Size = Storage.size();

			Error += BaseOffset == Tests[i].BaseOffset ? 0 : 1;
			Error += Size == Tests[i].Size ? 0 : 1;
		}

		return Error;
	}
}//namespace faces

namespace levels
{
	struct test
	{
		test
		(
			gli::format const & Format,
			std::size_t const & Level,
			std::size_t const & BaseOffset,
			std::size_t const & Size
		) :
			Format(Format),
			Level(Level),
			BaseOffset(BaseOffset),
			Size(Size)
		{}

		gli::format Format;
		std::size_t Level;
		std::size_t BaseOffset;
		std::size_t Size;
	};

	int run()
	{
		int Error(0);

		std::vector<test> Tests;
		Tests.push_back(test(gli::FORMAT_RGBA8_UINT_PACK8, 0, 0, 340));
		Tests.push_back(test(gli::FORMAT_RGBA8_UINT_PACK8, 1, 256, 340));
		Tests.push_back(test(gli::FORMAT_RGBA8_UINT_PACK8, 3, 336, 340));
		Tests.push_back(test(gli::FORMAT_RGBA32_SFLOAT_PACK32, 0, 0, 1360));
		Tests.push_back(test(gli::FORMAT_RGBA32_SFLOAT_PACK32, 1, 1024, 1360));
		Tests.push_back(test(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, 0, 0, 56));
		Tests.push_back(test(gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, 1, 32, 56));

		for(std::size_t i = 0; i < Tests.size(); ++i)
		{
			gli::storage_linear Storage(
				Tests[i].Format,
				gli::storage_linear::extent_type(8, 8, 1),
				1,
				1,
				4);

			gli::storage_linear::size_type BaseOffset = Storage.base_offset(0, 0, Tests[i].Level);
			gli::storage_linear::size_type Size = Storage.size();

			Error += BaseOffset == Tests[i].BaseOffset ? 0 : 1;
			Error += Size == Tests[i].Size ? 0 : 1;
		}

		return Error;
	}
}//namespace levels

int main()
{
	int Error(0);

	Error += layers::run();
	Error += faces::run();
	Error += levels::run();

	return Error;
}
