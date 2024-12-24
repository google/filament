#include <gli/copy.hpp>
#include <gli/texture2d.hpp>
#include <gli/comparison.hpp>

int test_sub_copy()
{
	int Error = 0;

	gli::texture2d Source(gli::FORMAT_R8_UNORM_PACK8, gli::extent2d(4, 4), 1);
	Source.clear(gli::u8(0));
	Source.store(gli::texture2d::extent_type(1, 1), 0, gli::u8(1));
	Source.store(gli::texture2d::extent_type(2, 1), 0, gli::u8(2));
	Source.store(gli::texture2d::extent_type(2, 2), 0, gli::u8(3));
	Source.store(gli::texture2d::extent_type(1, 2), 0, gli::u8(4));

	gli::texture2d Destination(Source.format(), Source.extent(), Source.levels());
	Destination.clear(gli::u8(255));

	Destination.copy(Source, 0, 0, 0, gli::texture::extent_type(1, 1, 0), 0, 0, 0, gli::texture::extent_type(1, 1, 0), gli::texture::extent_type(2, 2, 1));
	for(gli::size_t IndexY = 1; IndexY < 3; ++IndexY)
	for(gli::size_t IndexX = 1; IndexX < 3; ++IndexX)
		Error += Source.load<gli::u8>(gli::texture2d::extent_type(IndexX, IndexY), 0) == Destination.load<gli::u8>(gli::texture2d::extent_type(IndexX, IndexY), 0) ? 0 : 1;

	return Error;
}

int test_sub_copy2()
{
	int Error = 0;

	gli::texture2d Source(gli::FORMAT_R8_UNORM_PACK8, gli::extent2d(5, 4), 1);
	Source.clear(gli::u8(0));
	Source.store(gli::texture2d::extent_type(1, 1), 0, gli::u8(1));
	Source.store(gli::texture2d::extent_type(2, 1), 0, gli::u8(2));
	Source.store(gli::texture2d::extent_type(2, 2), 0, gli::u8(3));
	Source.store(gli::texture2d::extent_type(1, 2), 0, gli::u8(4));

	gli::texture2d Destination(Source.format(), Source.extent(), Source.levels());
	Destination.clear(gli::u8(255));

	Destination.copy(Source, 0, 0, 0, gli::texture::extent_type(1, 1, 0), 0, 0, 0, gli::texture::extent_type(1, 1, 0), gli::texture::extent_type(2, 2, 1));
	for(gli::size_t IndexY = 0; IndexY < Source.extent().y; ++IndexY)
	for(gli::size_t IndexX = 0; IndexX < Source.extent().x; ++IndexX)
	{
		gli::texture2d::extent_type TexelCoord(IndexX, IndexY);
		gli::u8 TexelSrc = Source.load<gli::u8>(TexelCoord, 0);
		gli::u8 TexelDst = Destination.load<gli::u8>(TexelCoord, 0);
		Error += TexelSrc == TexelDst || (TexelSrc == 0 && TexelDst == 255) ? 0 : 1;
	}

	return Error;
}

int test_sub_copy_rgb32f()
{
	int Error = 0;

	gli::texture2d Source(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent2d(4, 2), 1);
	for(gli::size_t TexelIndex = 0; TexelIndex < Source.size<gli::vec3>(); ++TexelIndex)
		*(Source.data<gli::vec3>() + TexelIndex) = gli::vec3(static_cast<float>(TexelIndex + 1));

	gli::texture2d Destination(Source.format(), Source.extent(), Source.levels());
	Destination.clear(gli::vec3(255));

	Destination.copy(Source, 0, 0, 0, gli::texture::extent_type(1, 1, 0), 0, 0, 0, gli::texture::extent_type(1, 1, 0), gli::texture::extent_type(2, 1, 1));
	for(gli::size_t IndexY = 0; IndexY < Source.extent().y; ++IndexY)
	for(gli::size_t IndexX = 0; IndexX < Source.extent().x; ++IndexX)
	{
		gli::texture2d::extent_type TexelCoord(IndexX, IndexY);
		gli::vec3 TexelSrc = Source.load<gli::vec3>(TexelCoord, 0);
		gli::vec3 TexelDst = Destination.load<gli::vec3>(TexelCoord, 0);
		Error += TexelSrc == TexelDst || (/*TexelSrc == gli::u8vec4(0) &&*/ TexelDst == gli::vec3(255)) ? 0 : 1;
	}

	return Error;
}

int test_sub_copy_rgba8()
{
	int Error = 0;

	gli::texture2d Source(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4, 2), 1);
	for(gli::size_t TexelIndex = 0; TexelIndex < Source.size<gli::u8vec4>(); ++TexelIndex)
		*(Source.data<gli::u8vec4>() + TexelIndex) = gli::u8vec4(static_cast<gli::u8>(TexelIndex + 1));

	gli::texture2d Destination(Source.format(), Source.extent(), Source.levels());
	Destination.clear(gli::u8vec4(255));

	Destination.copy(Source, 0, 0, 0, gli::texture::extent_type(1, 1, 0), 0, 0, 0, gli::texture::extent_type(1, 1, 0), gli::texture::extent_type(2, 1, 1));
	for(gli::size_t IndexY = 0; IndexY < Source.extent().y; ++IndexY)
	for(gli::size_t IndexX = 0; IndexX < Source.extent().x; ++IndexX)
	{
		gli::texture2d::extent_type TexelCoord(IndexX, IndexY);
		gli::u8vec4 TexelSrc = Source.load<gli::u8vec4>(TexelCoord, 0);
		gli::u8vec4 TexelDst = Destination.load<gli::u8vec4>(TexelCoord, 0);
		Error += TexelSrc == TexelDst || (/*TexelSrc == gli::u8vec4(0) &&*/ TexelDst == gli::u8vec4(255)) ? 0 : 1;
	}

	return Error;
}

int test_sub_clear_rgba8()
{
	int Error = 0;

	gli::texture2d Clear(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4, 2), 1);
	Clear.clear(gli::u8vec4(0));

	gli::texture2d Source(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4, 2), 1);
	Source.clear(gli::u8vec4(0));
	Source.texture::clear(0, 0, 0, gli::texture::extent_type(1, 1, 0), gli::texture::extent_type(2, 1, 1), gli::u8vec4(255));

	Error += Source != Clear ? 0 : 1;

	gli::texture2d Destination(Source.format(), Source.extent(), Source.levels());
	Destination.clear(gli::u8vec4(0));
	Destination.copy(Source, 0, 0, 0, gli::texture::extent_type(1, 1, 0), 0, 0, 0, gli::texture::extent_type(1, 1, 0), gli::texture::extent_type(2, 1, 1));

	Error += Destination != Clear ? 0 : 1;

	Error += Source == Destination ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_sub_clear_rgba8();
	Error += test_sub_copy_rgb32f();
	Error += test_sub_copy_rgba8();
	Error += test_sub_copy();
	Error += test_sub_copy2();

	return Error;
}


