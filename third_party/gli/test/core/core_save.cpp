#include <gli/save.hpp>
#include <gli/load.hpp>
#include <gli/texture2d.hpp>
#include <gli/comparison.hpp>

namespace
{
	struct params
	{
		params(std::string const& Filename, gli::format Format)
			: Filename(Filename)
			, Format(Format)
		{}

		std::string Filename;
		gli::format Format;
	};
}//namespace

namespace l8_unorm
{
	int test()
	{
		int Error = 0;

		gli::texture2d Texture(gli::FORMAT_L8_UNORM_PACK8, gli::texture2d::extent_type(4));
		Texture.clear(gli::u8vec1(127));

		gli::save(Texture, "orange_l8_unorm.dds");
		gli::texture2d TextureL8unormDDS(gli::load("orange_l8_unorm.dds"));

		gli::save(Texture, "orange_l8_unorm.ktx");
		gli::texture2d TextureL8unormKTX(gli::load("orange_l8_unorm.ktx"));

		Error += Texture == TextureL8unormDDS ? 0 : 1;
		Error += Texture == TextureL8unormKTX ? 0 : 1;
		Error += TextureL8unormDDS == TextureL8unormKTX ? 0 : 1;

		return Error;
	}
}//namespace l8_unorm

namespace la8_unorm
{
	int test()
	{
		int Error = 0;

		gli::texture2d Texture(gli::FORMAT_LA8_UNORM_PACK8, gli::texture2d::extent_type(4));
		Texture.clear(gli::u8vec2(255, 127));

		gli::save(Texture, "orange_la8_unorm.dds");
		gli::texture2d TextureLA8unormDDS(gli::load("orange_la8_unorm.dds"));

		gli::save(Texture, "orange_la8_unorm.ktx");
		gli::texture2d TextureLA8unormKTX(gli::load("orange_la8_unorm.ktx"));

		Error += Texture == TextureLA8unormDDS ? 0 : 1;
		Error += Texture == TextureLA8unormKTX ? 0 : 1;
		Error += TextureLA8unormDDS == TextureLA8unormKTX ? 0 : 1;

		return Error;
	}
}//namespace la8_unorm

namespace rgba8_unorm
{
	int test()
	{
		int Error = 0;

		gli::texture2d Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(4));
		Texture.clear(gli::u8vec4(255, 127, 0, 255));

		gli::save(Texture, "orange_rgba8_unorm.dds");
		gli::texture2d TextureRGBA8unormDDS(gli::load("orange_rgba8_unorm.dds"));

		gli::save(Texture, "orange_rgba8_unorm.ktx");
		gli::texture2d TextureRGBA8unormKTX(gli::load("orange_rgba8_unorm.ktx"));

		Error += Texture == TextureRGBA8unormDDS ? 0 : 1;
		Error += Texture == TextureRGBA8unormKTX ? 0 : 1;
		Error += TextureRGBA8unormDDS == TextureRGBA8unormKTX ? 0 : 1;

		return Error;
	}
}//namespace rgba8_unorm

int main()
{
	int Error = 0;

	Error += l8_unorm::test();
	Error += la8_unorm::test();
	Error += rgba8_unorm::test();

	return Error;
}
