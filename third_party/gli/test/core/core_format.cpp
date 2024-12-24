#include <gli/format.hpp>

namespace valid
{
	int test()
	{
		int Error(0);

		for(std::size_t FormatIndex = gli::FORMAT_FIRST; FormatIndex < gli::FORMAT_COUNT; ++FormatIndex)
			Error += gli::is_valid(static_cast<gli::format>(FormatIndex)) ? 0 : 1;
		Error += !gli::is_valid(gli::FORMAT_UNDEFINED) ? 0 : 1;

		return Error;
	}
}//namespace valid

namespace component
{
	int test()
	{
		int Error(0);

		for(std::size_t FormatIndex = gli::FORMAT_FIRST; FormatIndex < gli::FORMAT_COUNT; ++FormatIndex)
		{
			std::size_t const Components = gli::component_count(static_cast<gli::format>(FormatIndex));
			Error += Components > 0 && Components <= 4 ? 0 : 1;
			GLI_ASSERT(!Error);
		}

		return Error;
	}
}//namespace component

namespace compressed
{
	int test()
	{
		int Error(0);

		Error += !gli::is_compressed(gli::FORMAT_R8_SRGB_PACK8) ? 0 : 1;
		Error += gli::is_compressed(gli::FORMAT_RGB_DXT1_SRGB_BLOCK8) ? 0 : 1;

		return Error;
	}
}//namespace format

namespace block
{
	int test()
	{
		int Error(0);

		Error += gli::block_size(gli::FORMAT_RGBA8_UNORM_PACK8) == 4 ? 0 : 1;
		Error += gli::block_size(gli::FORMAT_RGB10A2_UNORM_PACK32) == 4 ? 0 : 1;

		return Error;
	}
}//namespace block

int main()
{
	int Error(0);

	Error += valid::test();
	Error += component::test();
	Error += compressed::test();
	Error += block::test();

	return Error;
}
