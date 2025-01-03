#include <gli/gli.hpp>

int test_alloc()
{
	int Error(0);

	std::vector<gli::format> Formats;
	Formats.push_back(gli::FORMAT_RGBA8_UNORM_PACK8);
	Formats.push_back(gli::FORMAT_RGB8_UNORM_PACK8);
	Formats.push_back(gli::FORMAT_R8_SNORM_PACK8);
	Formats.push_back(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8);
	Formats.push_back(gli::FORMAT_RGBA_BP_UNORM_BLOCK16);
	Formats.push_back(gli::FORMAT_RGBA32_SFLOAT_PACK32);

	std::vector<gli::texture_cube::extent_type::value_type> Sizes;
	Sizes.push_back(16);
	Sizes.push_back(32);
	Sizes.push_back(15);
	Sizes.push_back(17);
	Sizes.push_back(1);

	for(std::size_t FormatIndex = 0; FormatIndex < Formats.size(); ++FormatIndex)
	for(std::size_t SizeIndex = 0; SizeIndex < Sizes.size(); ++SizeIndex)
	{
		gli::texture_cube::extent_type Size(Sizes[SizeIndex]);

		gli::texture_cube TextureA(Formats[FormatIndex], Size, gli::levels(Size));

		gli::texture_cube TextureB(Formats[FormatIndex], Size);

		Error += TextureA == TextureB ? 0 : 1;
	}

	return Error;
}

int test_textureCube_query()
{
	int Error(0);

	gli::texture_cube Texture(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture_cube::extent_type(2), 2);

	Error += Texture.size() == sizeof(glm::u8vec4) * 5 * 6 ? 0 : 1;
	Error += Texture.format() == gli::FORMAT_RGBA8_UINT_PACK8 ? 0 : 1;
	Error += Texture.levels() == 2 ? 0 : 1;
	Error += !Texture.empty() ? 0 : 1;
	Error += Texture.extent().x == 2 ? 0 : 1;
	Error += Texture.extent().y == 2 ? 0 : 1;

	return Error;
}

int test_textureCube_texture2D_access()
{
	int Error(0);

	{
		gli::texture2d Texture2DA(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture2d::extent_type(2, 2), 1);
		for(std::size_t i = 0; i < Texture2DA.size(); ++i)
			*(Texture2DA.data<gli::byte>() + i) = gli::byte(i);

		gli::texture2d Texture2DB(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture2d::extent_type(2, 2), 1);
		for(std::size_t i = 0; i < Texture2DB.size(); ++i)
			*(Texture2DB.data<gli::byte>() + i) = gli::byte(i + 100);

		gli::texture_cube TextureCube(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture_cube::extent_type(2), 2);

		/// Todo
		/// gli::copy(TextureCube, 0, Texture2DA);
		/// gli::copy(TextureCube, 1, Texture2DB);

		/// Error += TextureCube[0] == Texture2DA ? 0 : 1;
		/// Error += TextureCube[1] == Texture2DB ? 0 : 1;
	}

	{
		gli::texture_cube TextureCube(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture_cube::extent_type(2), 1);
		GLI_ASSERT(!TextureCube.empty());

		std::vector<glm::u8vec4> Colors;
		Colors.push_back(glm::u8vec4(255,   0,   0, 255));
		Colors.push_back(glm::u8vec4(255, 255,   0, 255));
		Colors.push_back(glm::u8vec4(  0, 255,   0, 255));
		Colors.push_back(glm::u8vec4(  0, 255, 255, 255));
		Colors.push_back(glm::u8vec4(  0,   0, 255, 255));
		Colors.push_back(glm::u8vec4(255, 255,   0, 255));

		for(std::size_t ColorIndex = 0; ColorIndex < Colors.size(); ++ColorIndex)
		{
			gli::texture2d Texture2D = TextureCube[ColorIndex];
			for(std::size_t PixelIndex = 0; PixelIndex < 4; ++PixelIndex)
			{
				glm::u8vec4 Color = Colors[ColorIndex];
				*(Texture2D.data<glm::u8vec4>() + PixelIndex) = Color;
			}
		}

		for(std::size_t TexelIndex = 0; TexelIndex < TextureCube.size() / sizeof(glm::u8vec4); ++TexelIndex)
			Error += glm::all(glm::equal(*(TextureCube.data<glm::u8vec4>() + TexelIndex), Colors[TexelIndex / 4])) ? 0 : 1;
	}

	{
		gli::texture_cube TextureCube(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture_cube::extent_type(2), 2);
		GLI_ASSERT(!TextureCube.empty());

		gli::texture2d TextureA = TextureCube[0];
		gli::texture2d TextureB = TextureCube[1];
		
		std::size_t Size0 = TextureA.size();
		std::size_t Size1 = TextureB.size();

		Error += Size0 == sizeof(glm::u8vec4) * 5 ? 0 : 1;
		Error += Size1 == sizeof(glm::u8vec4) * 5 ? 0 : 1;

		*TextureA.data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);
		*TextureB.data<glm::u8vec4>() = glm::u8vec4(0, 127, 255, 255);

		glm::u8vec4 * PointerA = TextureA.data<glm::u8vec4>();
		glm::u8vec4 * PointerB = TextureB.data<glm::u8vec4>();

		glm::u8vec4 * Pointer0 = TextureCube.data<glm::u8vec4>() + 0;
		glm::u8vec4 * Pointer1 = TextureCube.data<glm::u8vec4>() + 5;

		Error += PointerA == Pointer0 ? 0 : 1;
		Error += PointerB == Pointer1 ? 0 : 1;

		glm::u8vec4 ColorA = *TextureA.data<glm::u8vec4>();
		glm::u8vec4 ColorB = *TextureB.data<glm::u8vec4>();

		glm::u8vec4 Color0 = *Pointer0;
		glm::u8vec4 Color1 = *Pointer1;

		Error += ColorA == Color0 ? 0 : 1;
		Error += ColorB == Color1 ? 0 : 1;

		Error += glm::all(glm::equal(Color0, glm::u8vec4(255, 127, 0, 255))) ? 0 : 1;
		Error += glm::all(glm::equal(Color1, glm::u8vec4(0, 127, 255, 255))) ? 0 : 1;
	}

	{
		gli::texture_cube TextureCube(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture_cube::extent_type(2), 1);

		std::size_t SizeA = TextureCube.size();
		Error += SizeA == sizeof(glm::u8vec4) * 4 * 6 ? 0 : 1;

		gli::texture2d Texture2D = TextureCube[0];
		
		std::size_t Size0 = Texture2D.size();
		Error += Size0 == sizeof(glm::u8vec4) * 4 ? 0 : 1;

		*Texture2D.data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		glm::u8vec4 * PointerA = Texture2D.data<glm::u8vec4>();
		glm::u8vec4 * Pointer0 = TextureCube.data<glm::u8vec4>();
		Error += PointerA == Pointer0 ? 0 : 1;

		glm::u8vec4 ColorA = *PointerA;
		glm::u8vec4 Color0 = *Pointer0;

		Error += ColorA == Color0 ? 0 : 1;

		Error += glm::all(glm::equal(ColorA, glm::u8vec4(255, 127, 0, 255))) ? 0 : 1;
		Error += glm::all(glm::equal(Color0, glm::u8vec4(255, 127, 0, 255))) ? 0 : 1;
	}

	return Error;
}

struct test
{
	test(
		gli::format const & Format,
		gli::texture_cube::extent_type const & Dimensions,
		gli::texture_cube::size_type const & Size) :
		Format(Format),
		Dimensions(Dimensions),
		Size(Size)
	{}

	gli::format Format;
	gli::texture_cube::extent_type Dimensions;
	gli::texture_cube::size_type Size;
};

int test_textureCube_texture2D_size()
{
	int Error(0);

	std::vector<test> Tests;
	Tests.push_back(test(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture_cube::extent_type(4), 384));
	Tests.push_back(test(gli::FORMAT_R8_UINT_PACK8, gli::texture_cube::extent_type(4), 96));
	Tests.push_back(test(gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, gli::texture_cube::extent_type(4), 48));
	Tests.push_back(test(gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, gli::texture_cube::extent_type(2), 48));
	Tests.push_back(test(gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, gli::texture_cube::extent_type(1), 48));
	Tests.push_back(test(gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16, gli::texture_cube::extent_type(4), 96));

	for(std::size_t i = 0; i < Tests.size(); ++i)
	{
		gli::texture_cube Texture(
			Tests[i].Format,
			gli::texture_cube::extent_type(4),
			gli::texture_cube::size_type(1));

		gli::texture_cube::size_type Size = Texture.size();
		Error += Size == Tests[i].Size ? 0 : 1;
		GLI_ASSERT(!Error);
	}

	return Error;
}

namespace loader
{
	int test()
	{
		int Error(0);

		gli::texture_cube TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(8), 1);

		{
			std::vector<glm::u8vec4> Color;
			Color.push_back(glm::u8vec4(255,   0,   0, 255));
			Color.push_back(glm::u8vec4(255, 128,   0, 255));
			Color.push_back(glm::u8vec4(255, 255,   0, 255));
			Color.push_back(glm::u8vec4(  0, 255,   0, 255));
			Color.push_back(glm::u8vec4(  0, 128, 255, 255));
			Color.push_back(glm::u8vec4(  0,   0, 255, 255));

			for(gli::texture_cube::size_type FaceIndex = 0; FaceIndex < TextureA.faces(); ++FaceIndex)
			for(gli::texture2d::size_type TexelIndex = 0, TexelCount = TextureA[FaceIndex].size<glm::u8vec4>(); TexelIndex < TexelCount; ++TexelIndex)
				*(TextureA[FaceIndex].data<glm::u8vec4>() + TexelIndex) = Color[FaceIndex];

			gli::save_dds(TextureA, "textureCubeA_rgba8_unorm.dds");
		}

		{
			gli::texture_cube TextureB(gli::load_dds("textureCubeA_rgba8_unorm.dds"));
			gli::save_dds(TextureB, "textureCubeB_rgba8_unorm.dds");
			gli::texture_cube TextureC(gli::load_dds("textureCubeB_rgba8_unorm.dds"));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		return Error;
	}
}//namespace loader

namespace load_store
{
	template <typename genType>
	int run(gli::format Format, std::array<genType, 6> const & TestSamples)
	{
		int Error = 0;

		gli::texture_cube::extent_type const Dimensions(2, 2);

		gli::texture_cube TextureA(Format, Dimensions);
		TextureA.clear();
		for (std::size_t FaceIndex = 0, FaceCount = 6; FaceIndex < FaceCount; ++FaceIndex)
			*TextureA.data<genType>(0, FaceIndex, 1) = TestSamples[FaceIndex];

		gli::texture_cube TextureB(Format, Dimensions);
		TextureB.clear();
		for (std::size_t FaceIndex = 0, FaceCount = 6; FaceIndex < FaceCount; ++FaceIndex)
			TextureB.store(gli::texture_cube::extent_type(0, 0), FaceIndex, 1, TestSamples[FaceIndex]);

		std::array<genType, 6> LoadedSamplesA;
		for (std::size_t FaceIndex = 0, FaceCount = 6; FaceIndex < FaceCount; ++FaceIndex)
			LoadedSamplesA[FaceIndex] = TextureA.load<genType>(gli::texture_cube::extent_type(0), FaceIndex, 1);

		std::array<genType, 6> LoadedSamplesB;
		for (std::size_t FaceIndex = 0, FaceCount = 6; FaceIndex < FaceCount; ++FaceIndex)
			LoadedSamplesB[FaceIndex] = TextureB.load<genType>(gli::texture_cube::extent_type(0), FaceIndex, 1);

		for (std::size_t FaceIndex = 0, FaceCount = 6; FaceIndex < FaceCount; ++FaceIndex)
			Error += LoadedSamplesA[FaceIndex] == TestSamples[FaceIndex] ? 0 : 1;

		for (std::size_t FaceIndex = 0, FaceCount = 6; FaceIndex < FaceCount; ++FaceIndex)
			Error += LoadedSamplesB[FaceIndex] == TestSamples[FaceIndex] ? 0 : 1;

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture_cube TextureC(TextureA, 0, 5, 1, 1);
		gli::texture_cube TextureD(TextureB, 0, 5, 1, 1);

		Error += TextureC == TextureD ? 0 : 1;

		return Error;
	}

	int test()
	{
		int Error = 0;

		{
			std::array<glm::f32vec1, 6> TestSamples{
			{
				glm::f32vec1(0.0f),
				glm::f32vec1(1.0f),
				glm::f32vec1(-1.0f),
				glm::f32vec1(0.5f),
				glm::f32vec1(-0.5f),
				glm::f32vec1(0.2f)
			}};

			Error += run(gli::FORMAT_R32_SFLOAT_PACK32, TestSamples);
		}

		{
			std::array<glm::f32vec2, 6> TestSamples{
			{
				glm::f32vec2(-1.0f,-1.0f),
				glm::f32vec2(-0.5f,-0.5f),
				glm::f32vec2(0.0f, 0.0f),
				glm::f32vec2(0.5f, 0.5f),
				glm::f32vec2(1.0f, 1.0f),
				glm::f32vec2(-1.0f, 1.0f)
			}};

			Error += run(gli::FORMAT_RG32_SFLOAT_PACK32, TestSamples);
		}

		{
			std::array<glm::f32vec3, 6> TestSamples{
			{
				glm::f32vec3(-1.0f, 0.0f, 1.0f),
				glm::f32vec3(-0.5f, 0.0f, 0.5f),
				glm::f32vec3(-0.2f, 0.0f, 0.2f),
				glm::f32vec3(-0.0f, 0.0f, 0.0f),
				glm::f32vec3(0.1f, 0.2f, 0.3f),
				glm::f32vec3(-0.1f,-0.2f,-0.3f)
			}};

			Error += run(gli::FORMAT_RGB32_SFLOAT_PACK32, TestSamples);
		}

		{
			std::array<glm::f32vec4, 6> TestSamples{
			{
				glm::f32vec4(-1.0f, 0.0f, 1.0f, 1.0f),
				glm::f32vec4(-0.5f, 0.0f, 0.5f, 1.0f),
				glm::f32vec4(-0.2f, 0.0f, 0.2f, 1.0f),
				glm::f32vec4(-0.0f, 0.0f, 0.0f, 1.0f),
				glm::f32vec4(0.1f, 0.2f, 0.3f, 1.0f),
				glm::f32vec4(-0.1f,-0.2f,-0.3f, 1.0f)
			}};

			Error += run(gli::FORMAT_RGBA32_SFLOAT_PACK32, TestSamples);
		}

		{
			std::array<glm::i8vec1, 6> TestSamples{
			{
				glm::i8vec1(-128),
				glm::i8vec1(-127),
				glm::i8vec1(127),
				glm::i8vec1(64),
				glm::i8vec1(-64),
				glm::i8vec1(1)
			}};

			Error += run(gli::FORMAT_R8_SINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_R8_SNORM_PACK8, TestSamples);
		}

		{
			std::array<glm::i8vec2, 6> TestSamples{
			{
				glm::i8vec2(-128, -96),
				glm::i8vec2(-64,  96),
				glm::i8vec2(-128,  64),
				glm::i8vec2(127,  32),
				glm::i8vec2(0, 126),
				glm::i8vec2(-48,  48)
			}};

			Error += run(gli::FORMAT_RG8_UINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RG8_UNORM_PACK8, TestSamples);
		}

		{
			std::array<glm::i8vec3, 6> TestSamples{
			{
				glm::i8vec3(-128,   0,   0),
				glm::i8vec3(-128, 127,   0),
				glm::i8vec3(-128, -96,   0),
				glm::i8vec3(127,-128,   0),
				glm::i8vec3(0, 127,   0),
				glm::i8vec3(0, 127,-127)
			}};

			Error += run(gli::FORMAT_RGB8_SINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGB8_SNORM_PACK8, TestSamples);
		}

		{
			std::array<glm::i8vec4, 6> TestSamples{
			{
				glm::i8vec4(-127,   0,   0, 127),
				glm::i8vec4(-128,  96,   0,-128),
				glm::i8vec4(127,  64,   0,   1),
				glm::i8vec4(0, -64,   0,   2),
				glm::i8vec4(-95,  32,   0,   3),
				glm::i8vec4(95, -32, 127,   4)
			}};

			Error += run(gli::FORMAT_RGBA8_SINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGBA8_SNORM_PACK8, TestSamples);
		}

		{
			std::array<glm::u8vec1, 6> TestSamples{
			{
				glm::u8vec1(255),
				glm::u8vec1(224),
				glm::u8vec1(192),
				glm::u8vec1(128),
				glm::u8vec1(64),
				glm::u8vec1(32)
			}};

			Error += run(gli::FORMAT_R8_UINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_R8_UNORM_PACK8, TestSamples);
			Error += run(gli::FORMAT_R8_SRGB_PACK8, TestSamples);
		}

		{
			std::array<glm::u8vec2, 6> TestSamples{
			{
				glm::u8vec2(255,   0),
				glm::u8vec2(255, 128),
				glm::u8vec2(255, 255),
				glm::u8vec2(128, 255),
				glm::u8vec2(0, 255),
				glm::u8vec2(0, 255)
			}};

			Error += run(gli::FORMAT_RG8_UINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RG8_UNORM_PACK8, TestSamples);
			Error += run(gli::FORMAT_RG8_SRGB_PACK8, TestSamples);
		}

		{
			std::array<glm::u8vec3, 6> TestSamples{
			{
				glm::u8vec3(255,   0,   0),
				glm::u8vec3(255, 128,   0),
				glm::u8vec3(255, 255,   0),
				glm::u8vec3(128, 255,   0),
				glm::u8vec3(0, 255,   0),
				glm::u8vec3(0, 255, 255)
			}};

			Error += run(gli::FORMAT_RGB8_UINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGB8_UNORM_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGB8_SRGB_PACK8, TestSamples);
		}

		{
			std::array<glm::u8vec4, 6> TestSamples{
			{
				glm::u8vec4(255,   0,   0, 255),
				glm::u8vec4(255, 128,   0, 255),
				glm::u8vec4(255, 255,   0, 255),
				glm::u8vec4(128, 255,   0, 255),
				glm::u8vec4(0, 255,   0, 255),
				glm::u8vec4(0, 255, 255, 255)
			}};

			Error += run(gli::FORMAT_RGBA8_UINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGBA8_UNORM_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGBA8_SRGB_PACK8, TestSamples);
		}

		{
			std::array<glm::u16vec1, 6> TestSamples{
			{
				glm::u16vec1(65535),
				glm::u16vec1(32767),
				glm::u16vec1(192),
				glm::u16vec1(128),
				glm::u16vec1(64),
				glm::u16vec1(32)
			}};

			Error += run(gli::FORMAT_R16_UINT_PACK16, TestSamples);
			Error += run(gli::FORMAT_R16_UNORM_PACK16, TestSamples);
		}

		{
			std::array<glm::u16vec2, 6> TestSamples{
			{
				glm::u16vec2(255,   0),
				glm::u16vec2(255, 128),
				glm::u16vec2(255, 255),
				glm::u16vec2(128, 255),
				glm::u16vec2(0, 255),
				glm::u16vec2(0, 255)
			}};

			Error += run(gli::FORMAT_RG16_UINT_PACK16, TestSamples);
			Error += run(gli::FORMAT_RG16_UNORM_PACK16, TestSamples);
		}

		{
			std::array<glm::u16vec3, 6> TestSamples{
			{
				glm::u16vec3(255,   0,   0),
				glm::u16vec3(255, 128,   0),
				glm::u16vec3(255, 255,   0),
				glm::u16vec3(128, 255,   0),
				glm::u16vec3(0, 255,   0),
				glm::u16vec3(0, 255, 255)
			}};

			Error += run(gli::FORMAT_RGB16_UINT_PACK16, TestSamples);
			Error += run(gli::FORMAT_RGB16_UNORM_PACK16, TestSamples);
		}

		{
			std::array<glm::u16vec4, 6> TestSamples{
			{
				glm::u16vec4(255,   0,   0, 255),
				glm::u16vec4(255, 128,   0, 255),
				glm::u16vec4(255, 255,   0, 255),
				glm::u16vec4(128, 255,   0, 255),
				glm::u16vec4(0, 255,   0, 255),
				glm::u16vec4(0, 255, 255, 255)
			}};

			Error += run(gli::FORMAT_RGBA16_UINT_PACK16, TestSamples);
			Error += run(gli::FORMAT_RGBA16_UNORM_PACK16, TestSamples);
		}

		{
			std::array<glm::u32vec1, 6> TestSamples{
			{
				glm::u32vec1(65535),
				glm::u32vec1(32767),
				glm::u32vec1(192),
				glm::u32vec1(128),
				glm::u32vec1(64),
				glm::u32vec1(32)
			}};

			Error += run(gli::FORMAT_R32_UINT_PACK32, TestSamples);
		}

		{
			std::array<glm::u32vec2, 6> TestSamples{
			{
				glm::u32vec2(255,   0),
				glm::u32vec2(255, 128),
				glm::u32vec2(255, 255),
				glm::u32vec2(128, 255),
				glm::u32vec2(0, 255),
				glm::u32vec2(0, 255)
			}};

			Error += run(gli::FORMAT_RG32_UINT_PACK32, TestSamples);
		}

		{
			std::array<glm::u32vec3, 6> TestSamples{
			{
				glm::u32vec3(255,   0,   0),
				glm::u32vec3(255, 128,   0),
				glm::u32vec3(255, 255,   0),
				glm::u32vec3(128, 255,   0),
				glm::u32vec3(0, 255,   0),
				glm::u32vec3(0, 255, 255)
			}};

			Error += run(gli::FORMAT_RGB32_UINT_PACK32, TestSamples);
		}

		{
			std::array<glm::u32vec4, 6> TestSamples{
			{
				glm::u32vec4(255,   0,   0, 255),
				glm::u32vec4(255, 128,   0, 255),
				glm::u32vec4(255, 255,   0, 255),
				glm::u32vec4(128, 255,   0, 255),
				glm::u32vec4(0, 255,   0, 255),
				glm::u32vec4(0, 255, 255, 255)
			}};

			Error += run(gli::FORMAT_RGBA32_UINT_PACK32, TestSamples);
		}

		return Error;
	}
}//namespace load_store

namespace clear
{
	int test()
	{
		int Error = 0;

		glm::u8vec4 const Black(0, 0, 0, 255);
		glm::u8vec4 const Color(255, 127, 0, 255);

		gli::texture_cube Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(2));
		Texture.clear(Black);

		glm::u8vec4 const TexelA = Texture.load<glm::u8vec4>(gli::texture_cube::extent_type(0), 0, 0);
		glm::u8vec4 const TexelB = Texture.load<glm::u8vec4>(gli::texture_cube::extent_type(0), 0, 1);

		Error += TexelA == Black ? 0 : 1;
		Error += TexelB == Black ? 0 : 1;

		for(gli::texture_cube::size_type FaceIndex = 0, FaceCount = Texture.faces(); FaceIndex < FaceCount; ++FaceIndex)
			Texture.clear<glm::u8vec4>(0, FaceIndex, 1, Color);

		gli::texture_cube::extent_type Coords(0);
		for(; Coords.y < Texture.extent(1).y; ++Coords.y)
		for(; Coords.x < Texture.extent(1).x; ++Coords.x)
		{
			glm::u8vec4 const TexelD = Texture.load<glm::u8vec4>(Coords, 0, 1);
			Error += TexelD == Color ? 0 : 1;
		}

		gli::texture_cube TextureView(Texture, 0, 5, 1, 1);

		gli::texture_cube TextureImage(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(1), 1);
		TextureImage.clear(Color);

		Error += TextureView == TextureImage ? 0 : 1;

		return Error;
	}
}//namespace clear

int main()
{
	int Error(0);

	Error += clear::test();
	Error += loader::test();
	Error += test_alloc();
	Error += test_textureCube_texture2D_size();
	Error += test_textureCube_query();
	Error += test_textureCube_texture2D_access();
	Error += load_store::test();

	return Error;
}

