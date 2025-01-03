#include <gli/duplicate.hpp>
#include <gli/view.hpp>
#include <gli/comparison.hpp>

int test_texture1D
(
	std::vector<gli::format> const & Formats, 
	gli::texture1d::extent_type const & TextureSize
)
{
	int Error(0);

	for(std::size_t i = 0; i < Formats.size(); ++i)
	{
		gli::texture1d TextureA(
			Formats[i],
			TextureSize,
			gli::levels(TextureSize));

		gli::texture1d TextureB(gli::duplicate(TextureA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture1d TextureC(TextureA, gli::texture1d::size_type(1), gli::texture1d::size_type(2));

		Error += TextureA[1] == TextureC[0] ? 0 : 1;
		Error += TextureA[2] == TextureC[1] ? 0 : 1;

		gli::texture1d TextureD(gli::duplicate(TextureC));

		Error += TextureC == TextureD ? 0 : 1;

		gli::texture1d TextureG(gli::duplicate(TextureA, 0, TextureA.levels() - 1));
		Error += TextureA == TextureG ? 0 : 1;

		gli::texture1d TextureE(gli::duplicate(TextureA, 1, TextureA.levels() - 2));
		Error += TextureA[1] == TextureE[0] ? 0 : 1;

		gli::texture1d TextureF(TextureA, 1, TextureA.levels() - 2);

		Error += TextureE == TextureF ? 0 : 1;
	}

	return Error;
}

int test_texture1DArray
(
	std::vector<gli::format> const & Formats,
	gli::texture1d::extent_type const & TextureSize
)
{
	int Error(0);

	for(std::size_t i = 0; i < Formats.size(); ++i)
	{
		gli::texture1d_array TextureA(
			Formats[i],
			TextureSize,
			gli::texture1d_array::size_type(4));

		gli::texture1d_array TextureB(gli::duplicate(TextureA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture1d_array TextureC(TextureA,
			gli::texture1d_array::size_type(0), TextureA.layers() - 1,
			gli::texture1d_array::size_type(1), gli::texture1d_array::size_type(2));

		Error += TextureA[0][1] == TextureC[0][0] ? 0 : 1;
		Error += TextureA[0][2] == TextureC[0][1] ? 0 : 1;
		Error += TextureA[1][1] == TextureC[1][0] ? 0 : 1;
		Error += TextureA[1][2] == TextureC[1][1] ? 0 : 1;

		gli::texture1d_array TextureD(gli::duplicate(TextureC));

		Error += TextureC == TextureD ? 0 : 1;

		gli::texture1d_array TextureG(gli::duplicate(
			TextureA,
			0, TextureA.layers() - 1,
			0, TextureA.levels() - 1));
		Error += TextureA == TextureG ? 0 : 1;

		gli::texture1d_array TextureE(gli::duplicate(
			TextureA,
			1, TextureA.layers() - 1,
			0, TextureA.levels() - 1));
		Error += TextureA[1] == TextureE[0] ? 0 : 1;

		gli::texture1d_array TextureF(
			TextureA, 
			1, TextureA.layers() - 1, 
			0, TextureA.levels() - 1); 

		Error += TextureE == TextureF ? 0 : 1;

		gli::texture1d_array TextureK(
			Formats[i],
			TextureSize,
			gli::texture1d_array::size_type(4),
			gli::levels(TextureSize));

		gli::texture1d_array TextureH(TextureK, 1, 2, 1, 2);
		gli::texture1d_array TextureI(gli::duplicate(TextureH));

		Error += TextureH == TextureI ? 0 : 1;

		gli::texture1d_array TextureJ(gli::duplicate(TextureK, 1, 2, 1, 2));
		Error += TextureH == TextureJ ? 0 : 1;
		Error += TextureI == TextureJ ? 0 : 1;
	}

	return Error;
}

int test_texture2D
(
	std::vector<gli::format> const & Formats, 
	gli::texture2d::extent_type const & TextureSize
)
{
	int Error(0);

	for(std::size_t i = 0; i < Formats.size(); ++i)
	{
		gli::texture2d TextureA(Formats[i], TextureSize);

		gli::texture2d TextureB(gli::duplicate(TextureA));
		Error += TextureA == TextureB ? 0 : 1;

		gli::texture2d TextureC(gli::view(
			TextureA, gli::texture2d::size_type(1), gli::texture2d::size_type(2)));

		Error += TextureA[1] == TextureC[0] ? 0 : 1;
		Error += TextureA[2] == TextureC[1] ? 0 : 1;

		gli::texture2d TextureD(gli::duplicate(TextureC));

		Error += TextureC == TextureD ? 0 : 1;

		gli::texture2d TextureG(gli::duplicate(TextureA, 0, TextureA.levels() - 1));
		Error += TextureA == TextureG ? 0 : 1;

		gli::texture2d TextureE(gli::duplicate(TextureA, 1, TextureA.levels() - 1));
		Error += TextureA[1] == TextureE[0] ? 0 : 1;

		gli::texture2d TextureF(gli::view(
			TextureA, 1, TextureA.levels() - 1));

		Error += TextureE == TextureF ? 0 : 1;
	}

	return Error;
}

int test_texture2DArray
(
	std::vector<gli::format> const & Formats,
	gli::texture2d_array::extent_type const & TextureSize
)
{
	int Error(0);

	for(std::size_t i = 0; i < Formats.size(); ++i)
	{
		gli::texture2d_array TextureA(
			Formats[i],
			TextureSize,
			gli::texture2d_array::size_type(4));

		gli::texture2d_array TextureB(gli::duplicate(TextureA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture2d_array TextureC(TextureA,
			gli::texture2d_array::size_type(0), TextureA.layers() - 1,
			gli::texture2d_array::size_type(1), gli::texture2d_array::size_type(2));

		Error += TextureA[0][1] == TextureC[0][0] ? 0 : 1;
		Error += TextureA[0][2] == TextureC[0][1] ? 0 : 1;
		Error += TextureA[1][1] == TextureC[1][0] ? 0 : 1;
		Error += TextureA[1][2] == TextureC[1][1] ? 0 : 1;

		gli::texture2d_array TextureD(gli::duplicate(TextureC));

		Error += TextureC == TextureD ? 0 : 1;

		gli::texture2d_array TextureG(gli::duplicate(
			TextureA,
			0, TextureA.layers() - 1,
			0, TextureA.levels() - 1));
		Error += TextureA == TextureG ? 0 : 1;

		gli::texture2d_array TextureE(gli::duplicate(
			TextureA,
			1, TextureA.layers() - 1,
			0, TextureA.levels() - 1));
		Error += TextureA[1] == TextureE[0] ? 0 : 1;

		gli::texture2d_array TextureF(
			TextureA,
			1, TextureA.layers() - 1,
			0, TextureA.levels() - 1);

		Error += TextureE == TextureF ? 0 : 1;

		gli::texture2d_array TextureK(
			Formats[i],
			TextureSize,
			gli::texture2d_array::size_type(4));

		gli::texture2d_array TextureH(TextureK, 1, 2, 1, 2);
		gli::texture2d_array TextureI(gli::duplicate(TextureH));

		Error += TextureH == TextureI ? 0 : 1;

		gli::texture2d_array TextureJ(gli::duplicate(TextureK, 1, 2, 1, 2));
		Error += TextureH == TextureJ ? 0 : 1;
		Error += TextureI == TextureJ ? 0 : 1;
	}

	return Error;
}

int test_texture3D
(
	std::vector<gli::format> const & Formats, 
	gli::texture3d::extent_type const & TextureSize
)
{
	int Error(0);

	for(std::size_t i = 0; i < Formats.size(); ++i)
	{
		gli::texture3d TextureA(Formats[i], TextureSize);

		gli::texture3d TextureB(gli::duplicate(TextureA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture3d TextureC(TextureA, gli::texture3d::size_type(1), gli::texture3d::size_type(2));

		Error += TextureA[1] == TextureC[0] ? 0 : 1;
		Error += TextureA[2] == TextureC[1] ? 0 : 1;

		gli::texture3d TextureD(gli::duplicate(TextureC));

		Error += TextureC == TextureD ? 0 : 1;

		gli::texture3d TextureG(gli::duplicate(TextureA, 0, TextureA.levels() - 1));
		Error += TextureA == TextureG ? 0 : 1;

		gli::texture3d TextureE(gli::duplicate(TextureA, 1, TextureA.levels() - 1));
		Error += TextureA[1] == TextureE[0] ? 0 : 1;

		gli::texture3d TextureF(TextureA, 1, TextureA.levels() - 1); 

		Error += TextureE == TextureF ? 0 : 1;
	}

	return Error;
}

int test_textureCube
(
	std::vector<gli::format> const & Formats,
	gli::texture_cube::extent_type const & TextureSize
)
{
	int Error(0);

	for(std::size_t i = 0; i < Formats.size(); ++i)
	{
		gli::texture_cube TextureA(
			Formats[i],
			gli::texture_cube::extent_type(TextureSize));

		gli::texture_cube TextureB(gli::duplicate(TextureA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture_cube TextureC(TextureA, 
			gli::texture_cube::size_type(0), TextureA.faces() - 1,
			gli::texture_cube::size_type(1), gli::texture_cube::size_type(2));

		Error += TextureA[0][1] == TextureC[0][0] ? 0 : 1;
		Error += TextureA[0][2] == TextureC[0][1] ? 0 : 1;
		Error += TextureA[1][1] == TextureC[1][0] ? 0 : 1;
		Error += TextureA[1][2] == TextureC[1][1] ? 0 : 1;

		gli::texture_cube TextureD(gli::duplicate(TextureC));

		Error += TextureC == TextureD ? 0 : 1;

		gli::texture_cube TextureG(gli::duplicate(
			TextureA,
			0, TextureA.faces() - 1,
			0, TextureA.levels() - 1));
		Error += TextureA == TextureG ? 0 : 1;

		gli::texture_cube TextureE(gli::duplicate(
			TextureA,
			0, TextureA.faces() - 1,
			0, TextureA.levels() - 1));
		Error += TextureA[1] == TextureE[0] ? 0 : 1;

		gli::texture_cube TextureF(
			TextureA,
			0, TextureA.faces() - 1,
			0, TextureA.levels() - 1);

		Error += TextureE == TextureF ? 0 : 1;

		gli::texture_cube TextureK(
			Formats[i],
			TextureSize);

		gli::texture_cube TextureH(TextureK, 0, 5, 1, 2);
		gli::texture_cube TextureI(gli::duplicate(TextureH));

		Error += TextureH == TextureI ? 0 : 1;

		gli::texture_cube TextureJ(gli::duplicate(TextureK, 0, 5, 1, 2));
		Error += TextureH == TextureJ ? 0 : 1;
		Error += TextureI == TextureJ ? 0 : 1;
	}

	return Error;
}

int test_textureCubeArray
(
	std::vector<gli::format> const & Formats,
	gli::texture_cube_array::extent_type const & TextureSize
)
{
	int Error(0);

	for(std::size_t i = 0; i < Formats.size(); ++i)
	{
		gli::texture_cube_array TextureA(
			Formats[i],
			TextureSize,
			gli::texture_cube_array::size_type(4));

		gli::texture_cube_array TextureB(gli::duplicate(TextureA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture_cube_array TextureC(TextureA, 
			gli::texture_cube_array::size_type(0), TextureA.layers() - 1,
			gli::texture_cube_array::size_type(0), TextureA.faces() - 1,
			gli::texture_cube_array::size_type(1), gli::texture_cube_array::size_type(2));

		Error += TextureA[0][0][1] == TextureC[0][0][0] ? 0 : 1;
		Error += TextureA[0][0][2] == TextureC[0][0][1] ? 0 : 1;
		Error += TextureA[0][1][1] == TextureC[0][1][0] ? 0 : 1;
		Error += TextureA[0][1][2] == TextureC[0][1][1] ? 0 : 1;

		gli::texture_cube_array TextureD(gli::duplicate(TextureC));

		Error += TextureC == TextureD ? 0 : 1;

		gli::texture_cube_array TextureG(gli::duplicate(
			TextureA,
			gli::texture_cube_array::size_type(0), TextureA.layers() - 1,
			gli::texture_cube_array::size_type(0), TextureA.faces() - 1,
			gli::texture_cube_array::size_type(0), TextureA.levels() - 1));
		Error += TextureA == TextureG ? 0 : 1;

		gli::texture_cube_array TextureK(
			Formats[i],
			TextureSize,
			4);

		gli::texture_cube_array TextureH(TextureK, 1, 2, 0, 5, 1, 2);
		gli::texture_cube_array TextureI(gli::duplicate(TextureH));

		Error += TextureH == TextureI ? 0 : 1;

		gli::texture_cube_array TextureJ(gli::duplicate(TextureK, 1, 2, 0, 5, 1, 2));
		Error += TextureH == TextureJ ? 0 : 1;
		Error += TextureI == TextureJ ? 0 : 1;
	}

	return Error;
}

int main()
{
	int Error(0);

	const int levels = gli::levels(1600);



	std::vector<gli::format> FormatsA;
	FormatsA.push_back(gli::FORMAT_RGBA8_UNORM_PACK8);
	FormatsA.push_back(gli::FORMAT_RGB8_UNORM_PACK8);
	FormatsA.push_back(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8);
	FormatsA.push_back(gli::FORMAT_RGBA_BP_UNORM_BLOCK16);
	FormatsA.push_back(gli::FORMAT_RGBA32_SFLOAT_PACK32);

	std::vector<gli::format> FormatsB;
	FormatsB.push_back(gli::FORMAT_RGBA8_UNORM_PACK8);
	FormatsB.push_back(gli::FORMAT_RGB8_UNORM_PACK8);
	FormatsB.push_back(gli::FORMAT_RGBA32_SFLOAT_PACK32);

	std::size_t const TextureSize = 32;

	Error += test_texture1D(FormatsB, gli::texture1d::extent_type(TextureSize));
	Error += test_texture1DArray(FormatsB, gli::texture1d_array::extent_type(TextureSize));
	Error += test_texture2D(FormatsA, gli::texture2d::extent_type(TextureSize));
	Error += test_texture2DArray(FormatsA, gli::texture2d_array::extent_type(TextureSize));
	Error += test_texture3D(FormatsA, gli::texture3d::extent_type(TextureSize));
	Error += test_textureCube(FormatsA, gli::texture_cube::extent_type(TextureSize));
	Error += test_textureCubeArray(FormatsA, gli::texture_cube_array::extent_type(TextureSize));

	return Error;
}
