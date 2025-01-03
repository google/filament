#include <gli/view.hpp>
#include <gli/duplicate.hpp>
#include <gli/levels.hpp>
#include <gli/comparison.hpp>

namespace dim
{
	int test_view1D
	(
		std::vector<gli::format> const & Formats,
		gli::texture1d::extent_type const & TextureSize
	)
	{
		int Error(0);

		for(std::size_t i = 0; i < Formats.size(); ++i)
		{
			gli::texture1d TextureA(Formats[i], TextureSize);
			gli::texture1d TextureViewA(gli::view(
				TextureA, TextureA.base_level(), TextureA.max_level()));

			Error += TextureA == TextureViewA ? 0 : 1;

			gli::texture1d TextureViewC(gli::view(
				TextureA, TextureA.base_level(), TextureA.max_level()));

			Error += TextureA == TextureViewC ? 0 : 1;
			Error += TextureViewA == TextureViewC ? 0 : 1;

			gli::texture1d TextureB(Formats[i], TextureSize / gli::texture1d::extent_type(2));
			gli::texture1d TextureViewB(gli::view(
				TextureA, TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureB == TextureViewB ? 0 : 1;

			gli::texture1d TextureViewD(gli::view(
				TextureA, TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureB == TextureViewD ? 0 : 1;
			Error += TextureViewB == TextureViewD ? 0 : 1;

			gli::texture1d TextureD(gli::view(
				TextureA, 1, 3));

			Error += TextureA[1] == TextureD[0] ? 0 : 1;
			Error += TextureA[2] == TextureD[1] ? 0 : 1;

			gli::texture1d TextureE(gli::view(
				TextureD, 1, 1));

			Error += TextureE[0] == TextureD[1] ? 0 : 1;
			Error += TextureE[0] == TextureA[2] ? 0 : 1;
		}

		return Error;
	}

	int test_view1DArray
	(
		std::vector<gli::format> const & Formats,
		gli::texture1d_array::extent_type const & TextureSize
	)
	{
		int Error(0);

		for(std::size_t i = 0; i < Formats.size(); ++i)
		{
			gli::texture1d_array TextureA(Formats[i], TextureSize, gli::texture1d_array::size_type(4));

			gli::texture1d_array TextureViewA(gli::view(
				TextureA,
				TextureA.base_layer(), TextureA.max_layer(),
				TextureA.base_level(), TextureA.max_level()));

			Error += TextureA == TextureViewA ? 0 : 1;

			gli::texture1d_array TextureViewC(gli::view(
				TextureA,
				TextureA.base_layer(), TextureA.max_layer(),
				TextureA.base_level(), TextureA.max_level()));
		
			Error += TextureA == TextureViewC ? 0 : 1;
			Error += TextureViewC == TextureViewA ? 0 : 1;

			gli::texture1d_array TextureB(
				Formats[i], TextureSize / gli::texture1d_array::extent_type(2), gli::texture1d_array::size_type(4));

			Error += TextureA != TextureB ? 0 : 1;

			gli::texture1d_array TextureViewB(gli::view(
				TextureA,
				TextureA.base_layer(), TextureA.max_layer(),
				TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureA != TextureViewB ? 0 : 1;
			Error += TextureB == TextureViewB ? 0 : 1;

			gli::texture1d_array TextureViewD(gli::view(
				TextureA,
				TextureA.base_layer(), TextureA.max_layer(),
				TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureViewD == TextureViewB ? 0 : 1;

			gli::texture1d_array TextureD(gli::view(
				TextureA, 0, TextureA.layers() -1, 1, 3));

			Error += TextureA[0][1] == TextureD[0][0] ? 0 : 1;
			Error += TextureA[0][2] == TextureD[0][1] ? 0 : 1;

			gli::texture1d_array TextureE(gli::view(
				TextureD, 0, TextureD.layers() -1, 0, TextureD.levels() - 1));

			Error += TextureE == TextureD ? 0 : 1;
			Error += TextureE[0] == TextureD[0] ? 0 : 1;

			gli::texture1d_array TextureF(gli::view(
				TextureE, 1, 3, 0, TextureE.levels() - 1));

			Error += TextureF[0] == TextureD[1] ? 0 : 1;
			Error += TextureF[0] == TextureE[1] ? 0 : 1;
		}

		return Error;
	}

	int test_view2D
	(
		std::vector<gli::format> const & Formats,
		gli::texture2d::extent_type const & TextureSize
	)
	{
		int Error(0);

		for(std::size_t i = 0; i < Formats.size(); ++i)
		{
			gli::texture2d TextureA(Formats[i], TextureSize, gli::levels(TextureSize));

			for(std::size_t Index = 0; Index < TextureA.size(); ++Index)
				*(TextureA.data<gli::byte>() + Index) = gli::byte(Index);

			gli::texture2d TextureViewA(gli::view(
				TextureA, TextureA.base_level(), TextureA.max_level()));

			Error += TextureA == TextureViewA ? 0 : 1;

			gli::texture2d TextureD(
				gli::view(TextureA, 1, 3));

			Error += TextureA[1] == TextureD[0] ? 0 : 1;
			Error += TextureA[2] == TextureD[1] ? 0 : 1;

			gli::texture2d TextureE(TextureD, 1, 1);

			Error += TextureE[0] == TextureD[1] ? 0 : 1;
			Error += TextureE[0] == TextureA[2] ? 0 : 1;

			gli::texture2d TextureViewB(gli::view(
				TextureA,
				TextureA.base_level() + 1, TextureA.max_level()));

			gli::texture2d TextureViewD(gli::view(
				TextureA,
				TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureViewD == TextureViewB ? 0 : 1;
		}

		return Error;
	}

	int test_view2DArray
	(
		std::vector<gli::format> const & Formats,
		gli::texture2d_array::extent_type const & TextureSize
	)
	{
		int Error(0);

		for(std::size_t i = 0; i < Formats.size(); ++i)
		{
			gli::texture2d_array TextureA(Formats[i], TextureSize, 4);

			gli::texture2d_array TextureViewA(gli::view(
				TextureA,
				TextureA.base_layer(), TextureA.max_layer(),
				TextureA.base_level(), TextureA.max_level()));

			Error += TextureA == TextureViewA ? 0 : 1;

			gli::texture2d_array TextureB(Formats[i], TextureSize / gli::texture2d_array::extent_type(2), 4);

			gli::texture2d_array TextureViewB(gli::view(
				TextureA,
				TextureA.base_layer(), TextureA.max_layer(),
				TextureA.base_level() + 1, TextureA.max_level()));

			gli::texture2d_array TextureViewD(gli::view(
				TextureA,
				TextureA.base_layer(), TextureA.max_layer(),
				TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureViewB == TextureViewD ? 0 : 1;
			Error += TextureB == TextureViewB ? 0 : 1;

			gli::texture2d_array TextureD(gli::view(
				TextureA, 0, TextureA.layers() -1, 1, 3));

			Error += TextureA[0][1] == TextureD[0][0] ? 0 : 1;
			Error += TextureA[0][2] == TextureD[0][1] ? 0 : 1;

			gli::texture2d_array TextureE(gli::view(
				TextureD, 0, TextureD.layers() -1, 0, TextureD.levels() - 1));

			Error += TextureE == TextureD ? 0 : 1;
			Error += TextureE[0] == TextureD[0] ? 0 : 1;

			gli::texture2d_array TextureF(gli::view(
				TextureE, 1, 3, 0, TextureE.levels() - 1));

			Error += TextureF[0] == TextureD[1] ? 0 : 1;
			Error += TextureF[0] == TextureE[1] ? 0 : 1;
		}

		return Error;
	}

	int test_view3D
	(
		std::vector<gli::format> const & Formats, 
		gli::texture3d::extent_type const & TextureSize
	)
	{
		int Error(0);

		for(std::size_t i = 0; i < Formats.size(); ++i)
		{
			gli::texture3d TextureA(Formats[i], TextureSize, gli::levels(TextureSize));
			gli::texture3d TextureViewA(gli::view(
				TextureA, TextureA.base_level(), TextureA.max_level()));

			Error += TextureA == TextureViewA ? 0 : 1;

			gli::texture3d::extent_type SizeB(TextureSize / gli::texture3d::extent_type(2));
			gli::texture3d TextureB(Formats[i], SizeB, gli::levels(SizeB));

			gli::texture3d TextureViewB(gli::view(
				TextureA, TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureB == TextureViewB ? 0 : 1;

			gli::texture3d TextureViewD(gli::view(
				TextureA, TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureViewD == TextureViewB ? 0 : 1;

			gli::texture3d TextureD(gli::view(TextureA, 1, 3));

			Error += TextureA[1] == TextureD[0] ? 0 : 1;
			Error += TextureA[2] == TextureD[1] ? 0 : 1;

			gli::texture3d TextureE(gli::view(TextureD, 1, 1));

			Error += TextureE[0] == TextureD[1] ? 0 : 1;
			Error += TextureE[0] == TextureA[2] ? 0 : 1;
		}

		return Error;
	}

	int test_viewCube
	(
		std::vector<gli::format> const & Formats, 
		gli::texture_cube::extent_type const & TextureSize
	)
	{
		int Error(0);

		for(std::size_t i = 0; i < Formats.size(); ++i)
		{
			gli::texture_cube TextureA(Formats[i], TextureSize);

			gli::texture_cube TextureViewA(gli::view(
				TextureA,
				TextureA.base_face(), TextureA.max_face(),
				TextureA.base_level(), TextureA.max_level()));

			Error += TextureA == TextureViewA ? 0 : 1;

			gli::texture_cube::extent_type SizeB(TextureSize / gli::texture_cube::extent_type(2));
			gli::texture_cube TextureB(Formats[i], SizeB);

			gli::texture_cube TextureViewB(gli::view(
				TextureA,
				TextureA.base_face(), TextureA.max_face(),
				TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureB == TextureViewB ? 0 : 1;

			gli::texture_cube TextureViewD(gli::view(
				TextureA,
				TextureA.base_face(), TextureA.max_face(),
				TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureViewD == TextureViewB ? 0 : 1;

			gli::texture_cube TextureD(gli::view(
				TextureA, 0, TextureA.faces() -1, 1, 3));

			Error += TextureA[0][1] == TextureD[0][0] ? 0 : 1;
			Error += TextureA[0][2] == TextureD[0][1] ? 0 : 1;

			gli::texture_cube TextureE(gli::view(
				TextureD, 0, TextureD.faces() -1, 0, TextureD.levels() - 1));

			Error += TextureE == TextureD ? 0 : 1;
			Error += TextureE[0] == TextureD[0] ? 0 : 1;

			gli::texture_cube TextureF(gli::view(
				TextureE, 1, 3, 0, TextureE.levels() - 1));

			Error += TextureF[0] == TextureD[1] ? 0 : 1;
			Error += TextureF[0] == TextureE[1] ? 0 : 1;
		}

		return Error;
	}

	int test_viewCubeArray
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
				4);

			gli::texture_cube_array TextureViewA(gli::view(
				TextureA,
				TextureA.base_layer(), TextureA.max_layer(),
				TextureA.base_face(), TextureA.max_face(),
				TextureA.base_level(), TextureA.max_level()));

			Error += TextureA == TextureViewA ? 0 : 1;

			gli::texture_cube_array::extent_type SizeB(TextureSize / gli::texture_cube_array::extent_type(2));
			gli::texture_cube_array TextureB(
				Formats[i],
				SizeB,
				gli::texture_cube_array::size_type(4));

			gli::texture_cube_array TextureViewB(gli::view(
				TextureA,
				TextureA.base_layer(), TextureA.max_layer(),
				TextureA.base_face(), TextureA.max_face(),
				TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureB == TextureViewB ? 0 : 1;

			gli::texture_cube_array TextureViewD(gli::view(
				TextureA,
				TextureA.base_layer(), TextureA.max_layer(),
				TextureA.base_face(), TextureA.max_face(),
				TextureA.base_level() + 1, TextureA.max_level()));

			Error += TextureViewD == TextureViewB ? 0 : 1;

			gli::texture_cube_array TextureD(gli::view(
				TextureA,
				0, TextureA.layers() -1,
				0, TextureA.faces() -1,
				1, 3));

			Error += TextureA[0][0][1] == TextureD[0][0][0] ? 0 : 1;
			Error += TextureA[0][0][2] == TextureD[0][0][1] ? 0 : 1;

			gli::texture_cube_array TextureE(gli::view(
				TextureD,
				0, TextureA.layers() -1,
				0, TextureD.faces() -1,
				0, TextureD.levels() - 1));

			Error += TextureE == TextureD ? 0 : 1;
			Error += TextureE[0] == TextureD[0] ? 0 : 1;
			Error += TextureE[1] == TextureD[1] ? 0 : 1;

			gli::texture_cube_array TextureF(gli::view(
				TextureE,
				0, TextureA.layers() -1,
				1, 3,
				0, TextureE.levels() - 1));

			Error += TextureF[0][0] == TextureD[0][1] ? 0 : 1;
			Error += TextureF[1][0] == TextureD[1][1] ? 0 : 1;
			Error += TextureF[0][0] == TextureE[0][1] ? 0 : 1;
			Error += TextureF[1][0] == TextureE[1][1] ? 0 : 1;
		}

		return Error;
	}

	int run()
	{
		int Error(0);

		std::vector<gli::format> FormatsA;
		FormatsA.push_back(gli::FORMAT_RGBA8_UNORM_PACK8);
		FormatsA.push_back(gli::FORMAT_RGB8_UNORM_PACK8);
		FormatsA.push_back(gli::FORMAT_R8_SNORM_PACK8);
		FormatsA.push_back(gli::FORMAT_RGBA32_SFLOAT_PACK32);
		FormatsA.push_back(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8);
		FormatsA.push_back(gli::FORMAT_RGBA_BP_UNORM_BLOCK16);

		// 1D textures don't support compressed formats
		std::vector<gli::format> FormatsB;
		FormatsA.push_back(gli::FORMAT_RGBA8_UNORM_PACK8);
		FormatsA.push_back(gli::FORMAT_RGB8_UNORM_PACK8);
		FormatsA.push_back(gli::FORMAT_R8_SNORM_PACK8);
		FormatsA.push_back(gli::FORMAT_RGBA32_SFLOAT_PACK32);

		std::size_t const TextureSize(32);

		Error += test_view1D(FormatsB, gli::texture1d::extent_type(TextureSize));
		Error += test_view1DArray(FormatsB, gli::texture1d_array::extent_type(TextureSize));
		Error += test_view2D(FormatsA, gli::texture2d::extent_type(TextureSize));
		Error += test_view2DArray(FormatsA, gli::texture2d_array::extent_type(TextureSize));
		Error += test_view3D(FormatsA, gli::texture3d::extent_type(TextureSize));
		Error += test_viewCube(FormatsA, gli::texture_cube::extent_type(TextureSize));
		Error += test_viewCubeArray(FormatsA, gli::texture_cube::extent_type(TextureSize));

		return Error;
	}
}//namespace dim

namespace format
{
	int run()
	{
		int Error = 0;

		{
			gli::texture2d TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(1));
			gli::texture2d TextureB(gli::view(TextureA, gli::FORMAT_R32_UINT_PACK32));
			gli::texture2d TextureC(gli::view(TextureA));

			Error += TextureA.extent() == TextureB.extent() ? 0 : 1;
		}

		{
			gli::texture TextureA(gli::TARGET_2D, gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, gli::texture::extent_type(4, 4, 1), 1, 1, 3);
			gli::texture TextureB(gli::view(TextureA, gli::FORMAT_RG32_UINT_PACK32));
			gli::texture TextureC(gli::TARGET_2D, gli::FORMAT_RG32_UINT_PACK32, gli::texture::extent_type(1), 1, 1, 3);

			gli::texture::extent_type const ExtentA0 = TextureA.extent(0);
			gli::texture::extent_type const ExtentB0 = TextureB.extent(0);
			gli::texture::extent_type const ExtentC0 = TextureC.extent(0);
			gli::texture::extent_type const ExtentA1 = TextureA.extent(1);
			gli::texture::extent_type const ExtentB1 = TextureB.extent(1);
			gli::texture::extent_type const ExtentC1 = TextureC.extent(1);
			gli::texture::extent_type const ExtentA2 = TextureA.extent(2);
			gli::texture::extent_type const ExtentB2 = TextureB.extent(2);
			gli::texture::extent_type const ExtentC2 = TextureC.extent(2);

			Error += ExtentA0 == gli::texture::extent_type(4, 4, 1) ? 0 : 1;
			Error += ExtentB0 == gli::texture::extent_type(1, 1, 1) ? 0 : 1;
			Error += ExtentC0 == gli::texture::extent_type(1, 1, 1) ? 0 : 1;
			Error += ExtentA1 == gli::texture::extent_type(2, 2, 1) ? 0 : 1;
			Error += ExtentB1 == gli::texture::extent_type(1, 1, 1) ? 0 : 1;
			Error += ExtentC1 == gli::texture::extent_type(1, 1, 1) ? 0 : 1;
			Error += ExtentA2 == gli::texture::extent_type(1, 1, 1) ? 0 : 1;
			Error += ExtentB2 == gli::texture::extent_type(1, 1, 1) ? 0 : 1;
			Error += ExtentC2 == gli::texture::extent_type(1, 1, 1) ? 0 : 1;

			gli::texture::size_type const SizeA = TextureA.size();
			gli::texture::size_type const SizeB = TextureB.size();
			gli::texture::size_type const SizeC = TextureC.size();

			Error += SizeA == gli::texture::size_type(gli::block_size(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8) * 3) ? 0 : 1;
			Error += SizeB == gli::texture::size_type(gli::block_size(gli::FORMAT_RG32_UINT_PACK32) * 3) ? 0 : 1;
			Error += SizeC == gli::texture::size_type(gli::block_size(gli::FORMAT_RG32_UINT_PACK32) * 3) ? 0 : 1;
		}

		{
			gli::texture2d TextureA(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, gli::texture2d::extent_type(4));
			gli::texture2d TextureB(gli::view(TextureA, gli::FORMAT_RG32_UINT_PACK32));
			gli::texture2d TextureC(gli::FORMAT_RG32_UINT_PACK32, gli::texture2d::extent_type(1), 3);

			gli::texture2d::extent_type const ExtentA = TextureA.extent();
			gli::texture2d::extent_type const ExtentB = TextureB.extent();

			Error += TextureA.size() == TextureB.size() ? 0 : 1;
			Error += TextureA.size() == TextureC.size() ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
			Error += ExtentA == gli::texture2d::extent_type(4) ? 0 : 1;
			Error += ExtentB == gli::texture2d::extent_type(1) ? 0 : 1;
			Error += ExtentA != ExtentB ? 0 : 1;

			{
				gli::texture2d::extent_type const ExtentA0 = TextureA.extent(0);
				gli::texture2d::extent_type const ExtentB0 = TextureB.extent(0);
				gli::texture2d::extent_type const ExtentC0 = TextureC.extent(0);
				gli::texture2d::extent_type const ExtentA1 = TextureA.extent(1);
				gli::texture2d::extent_type const ExtentB1 = TextureB.extent(1);
				gli::texture2d::extent_type const ExtentC1 = TextureC.extent(1);
				gli::texture2d::extent_type const ExtentA2 = TextureA.extent(2);
				gli::texture2d::extent_type const ExtentB2 = TextureB.extent(2);
				gli::texture2d::extent_type const ExtentC2 = TextureC.extent(2);

				Error += ExtentA0 == gli::texture2d::extent_type(4, 4) ? 0 : 1;
				Error += ExtentB0 == gli::texture2d::extent_type(1, 1) ? 0 : 1;
				Error += ExtentC0 == gli::texture2d::extent_type(1, 1) ? 0 : 1;
				Error += ExtentA1 == gli::texture2d::extent_type(2, 2) ? 0 : 1;
				Error += ExtentB1 == gli::texture2d::extent_type(1, 1) ? 0 : 1;
				Error += ExtentC1 == gli::texture2d::extent_type(1, 1) ? 0 : 1;
				Error += ExtentA2 == gli::texture2d::extent_type(1, 1) ? 0 : 1;
				Error += ExtentB2 == gli::texture2d::extent_type(1, 1) ? 0 : 1;
				Error += ExtentC2 == gli::texture2d::extent_type(1, 1) ? 0 : 1;

				gli::texture2d::size_type const SizeA = TextureA.size();
				gli::texture2d::size_type const SizeB = TextureB.size();
				gli::texture2d::size_type const SizeC = TextureC.size();

				Error += SizeA == gli::texture::size_type(gli::block_size(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8) * 3) ? 0 : 1;
				Error += SizeB == gli::texture::size_type(gli::block_size(gli::FORMAT_RG32_UINT_PACK32) * 3) ? 0 : 1;
				Error += SizeC == gli::texture::size_type(gli::block_size(gli::FORMAT_RG32_UINT_PACK32) * 3) ? 0 : 1;
			}

			{
				gli::image const ImageA0 = TextureA[0];
				gli::image const ImageA1 = TextureA[1];
				gli::image const ImageA2 = TextureA[2];

				gli::image const ImageB0 = TextureB[0];
				gli::image const ImageB1 = TextureB[1];
				gli::image const ImageB2 = TextureB[2];

				gli::image const ImageC0 = TextureC[0];
				gli::image const ImageC1 = TextureC[1];
				gli::image const ImageC2 = TextureC[2];

				gli::image::extent_type const ExtentA0 = TextureA[0].extent();
				gli::image::extent_type const ExtentB0 = TextureB[0].extent();
				gli::image::extent_type const ExtentC0 = TextureC[0].extent();
				gli::image::extent_type const ExtentA1 = TextureA[1].extent();
				gli::image::extent_type const ExtentB1 = TextureB[1].extent();
				gli::image::extent_type const ExtentC1 = TextureC[1].extent();
				gli::image::extent_type const ExtentA2 = TextureA[2].extent();
				gli::image::extent_type const ExtentB2 = TextureB[2].extent();
				gli::image::extent_type const ExtentC2 = TextureC[2].extent();

				Error += ExtentA0 == gli::image::extent_type(4, 4, 1) ? 0 : 1;
				Error += ExtentB0 == gli::image::extent_type(1, 1, 1) ? 0 : 1;
				Error += ExtentC0 == gli::image::extent_type(1, 1, 1) ? 0 : 1;
				Error += ExtentA1 == gli::image::extent_type(2, 2, 1) ? 0 : 1;
				Error += ExtentB1 == gli::image::extent_type(1, 1, 1) ? 0 : 1;
				Error += ExtentC1 == gli::image::extent_type(1, 1, 1) ? 0 : 1;
				Error += ExtentA2 == gli::image::extent_type(1, 1, 1) ? 0 : 1;
				Error += ExtentB2 == gli::image::extent_type(1, 1, 1) ? 0 : 1;
				Error += ExtentC2 == gli::image::extent_type(1, 1, 1) ? 0 : 1;

				gli::image::size_type const SizeA0 = ImageA0.size();
				gli::image::size_type const SizeA1 = ImageA1.size();
				gli::image::size_type const SizeA2 = ImageA2.size();

				Error += SizeA0 == gli::image::size_type(gli::block_size(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8)) ? 0 : 1;
				Error += SizeA1 == gli::image::size_type(gli::block_size(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8)) ? 0 : 1;
				Error += SizeA2 == gli::image::size_type(gli::block_size(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8)) ? 0 : 1;

				gli::image::size_type const SizeB0 = ImageB0.size();
				gli::image::size_type const SizeB1 = ImageB1.size();
				gli::image::size_type const SizeB2 = ImageB2.size();

				Error += SizeB0 == gli::image::size_type(gli::block_size(gli::FORMAT_RG32_UINT_PACK32)) ? 0 : 1;
				Error += SizeB1 == gli::image::size_type(gli::block_size(gli::FORMAT_RG32_UINT_PACK32)) ? 0 : 1;
				Error += SizeB2 == gli::image::size_type(gli::block_size(gli::FORMAT_RG32_UINT_PACK32)) ? 0 : 1;

				gli::image::size_type const SizeC0 = ImageC0.size();
				gli::image::size_type const SizeC1 = ImageC1.size();
				gli::image::size_type const SizeC2 = ImageC2.size();

				Error += SizeC0 == gli::image::size_type(gli::block_size(gli::FORMAT_RG32_UINT_PACK32)) ? 0 : 1;
				Error += SizeC1 == gli::image::size_type(gli::block_size(gli::FORMAT_RG32_UINT_PACK32)) ? 0 : 1;
				Error += SizeC2 == gli::image::size_type(gli::block_size(gli::FORMAT_RG32_UINT_PACK32)) ? 0 : 1;
			}
		}

		{
			gli::texture2d TextureA(gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16, gli::texture2d::extent_type(4));
			gli::texture2d TextureB(gli::view(TextureA, gli::FORMAT_RGBA32_UINT_PACK32));
			gli::texture2d TextureC(gli::FORMAT_RGBA32_UINT_PACK32, gli::texture2d::extent_type(1), 3);
			gli::texture2d TextureD(gli::view(TextureC, gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16));

			Error += TextureA == TextureD ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;

			Error += TextureA.extent() == gli::texture2d::extent_type(4) ? 0 : 1;
			Error += TextureB.extent() == gli::texture2d::extent_type(1) ? 0 : 1;
			Error += TextureA.extent() != TextureB.extent() ? 0 : 1;
		}

		{
			gli::texture2d TextureA(gli::FORMAT_RG32_UINT_PACK32, gli::texture2d::extent_type(4));
			gli::texture2d TextureB(gli::view(TextureA, gli::FORMAT_RG32_UINT_PACK32));
			gli::texture2d TextureC(gli::view(TextureA, gli::FORMAT_RGBA16_UINT_PACK16));

			Error += TextureA == TextureB ? 0 : 1;
			Error += !TextureC.empty() ? 0 : 1;
			Error += TextureC.size() == TextureA.size() ? 0 : 1;
			Error += TextureC.extent() == TextureA.extent() ? 0 : 1;
			Error += TextureC.base_level() == TextureA.base_level() ? 0 : 1;
			Error += TextureC.max_level() == TextureA.max_level() ? 0 : 1;
		}

		return Error;
	}
}//namespace format

namespace clear2d
{
	int run()
	{
		int Error = 0;

		glm::u8vec4 const Black(0, 0, 0, 255);
		glm::u8vec4 const Color(255, 127, 0, 255);

		gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(8, 8, 1), 1, 1, 5);
		Texture.clear(Black);

		Texture.clear<glm::u8vec4>(0, 0, 1, glm::u8vec4(255, 127, 0, 255));

		gli::texture TextureView(gli::view(Texture, 0, 0, 0, 0, 1, 1));
		gli::texture TextureCopy(gli::duplicate(TextureView));
		Error += TextureView == TextureCopy ? 0 : 1;

		gli::texture TextureImage(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 1, 1, 1);
		TextureImage.clear(Color);

		Error += TextureView == TextureImage ? 0 : 1;
		Error += TextureView.size() == TextureImage.size() ? 0 : 1;
		Error += TextureView.size<glm::u8vec4>() == TextureImage.size<glm::u8vec4>() ? 0 : 1;

		return Error;
	}
}//namespace clear2d

namespace clear2d_array
{
	int run()
	{
		int Error = 0;

		glm::u8vec4 const Black(0, 0, 0, 255);
		glm::u8vec4 const Color(255, 127, 0, 255);

		gli::texture Texture(gli::TARGET_2D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(8, 8, 1), 3, 1, 5);
		Texture.clear(Black);

		Texture.clear<glm::u8vec4>(1, 0, 1, glm::u8vec4(255, 127, 0, 255));

		gli::texture TextureView(gli::view(Texture, 1, 1, 0, 0, 1, 1));
		gli::texture TextureCopy(gli::duplicate(TextureView));
		Error += TextureView == TextureCopy ? 0 : 1;

		gli::texture TextureImage(gli::TARGET_2D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 1, 1, 1);
		TextureImage.clear(Color);

		Error += TextureView == TextureImage ? 0 : 1;
		Error += TextureView.size() == TextureImage.size() ? 0 : 1;
		Error += TextureView.size<glm::u8vec4>() == TextureImage.size<glm::u8vec4>() ? 0 : 1;

		return Error;
	}
}//namespace clear2d_array

namespace clear_cube
{
	int run()
	{
		int Error = 0;

		glm::u8vec4 const Black(0, 0, 0, 255);
		glm::u8vec4 const Color(255, 127, 0, 255);

		gli::texture Texture(gli::TARGET_CUBE, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(8, 8, 1), 1, 6, 5);
		Texture.clear(Black);

		for(gli::texture::size_type FaceIndex = 0; FaceIndex < 6; ++FaceIndex)
			Texture.clear<glm::u8vec4>(0, FaceIndex, 1, glm::u8vec4(255, 127, 0, 255));

		gli::texture TextureView(gli::view(Texture, 0, 0, 0, 5, 1, 1));
		gli::texture TextureCopy(gli::duplicate(TextureView));
		Error += TextureView == TextureCopy ? 0 : 1;

		gli::texture TextureImage(gli::TARGET_CUBE, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 1, 6, 1);
		TextureImage.clear(Color);

		Error += TextureView == TextureImage ? 0 : 1;
		Error += TextureView.size() == TextureImage.size() ? 0 : 1;
		Error += TextureView.size<glm::u8vec4>() == TextureImage.size<glm::u8vec4>() ? 0 : 1;

		return Error;
	}
}//namespace clear_cube

namespace clear_cube_array
{
	int run()
	{
		int Error = 0;

		glm::u8vec4 const Black(0, 0, 0, 255);
		glm::u8vec4 const Color(255, 127, 0, 255);

		gli::texture Texture(gli::TARGET_CUBE_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(8, 8, 1), 3, 6, 5);
		Texture.clear(Black);

		for(gli::texture::size_type FaceIndex = 0; FaceIndex < 6; ++FaceIndex)
			Texture.clear<glm::u8vec4>(1, FaceIndex, 1, glm::u8vec4(255, 127, 0, 255));

		gli::texture TextureView(gli::view(Texture, 1, 1, 0, 5, 1, 1));
		gli::texture TextureCopy(gli::duplicate(TextureView));
		Error += TextureView == TextureCopy ? 0 : 1;

		gli::texture TextureImage(gli::TARGET_CUBE_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 1, 6, 1);
		TextureImage.clear(Color);

		Error += TextureView == TextureImage ? 0 : 1;
		Error += TextureView.size() == TextureImage.size() ? 0 : 1;
		Error += TextureView.size<glm::u8vec4>() == TextureImage.size<glm::u8vec4>() ? 0 : 1;

		return Error;
	}
}//namespace clear_cube_array

namespace size
{
	int run()
	{
		int Error = 0;

		gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(8, 8, 1), 1, 1, 5);

		gli::texture TextureView(gli::view(Texture, 0, 0, 0, 0, 1, 1));

		gli::texture TextureImage(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 1, 1, 1);

		Error += TextureView.size() == TextureImage.size() ? 0 : 1;

		return Error;
	}
}//namespace size

int main()
{
	int Error = 0;

	Error += dim::run();
	Error += format::run();
	Error += clear2d::run();
	Error += clear2d_array::run();
	Error += clear_cube::run();
	Error += clear_cube_array::run();
	Error += size::run();

	return Error;
}
