#include <gli/gli.hpp>
#include <glm/gtc/color_space.hpp>

namespace
{
	std::string path(std::string const & filename)
	{
		return std::string(SOURCE_DIR) + "/data/" + filename;
	}
}//namespace

namespace alloc
{
	int test()
	{
		int Error(0);

		std::vector<gli::format> Formats;
		Formats.push_back(gli::FORMAT_RGBA8_UNORM_PACK8);
		Formats.push_back(gli::FORMAT_RGB8_UNORM_PACK8);
		Formats.push_back(gli::FORMAT_R8_SNORM_PACK8);
		Formats.push_back(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8);
		Formats.push_back(gli::FORMAT_RGBA_BP_UNORM_BLOCK16);
		Formats.push_back(gli::FORMAT_RGBA32_SFLOAT_PACK32);

		std::vector<gli::texture2d_array::extent_type::value_type> Sizes;
		Sizes.push_back(16);
		Sizes.push_back(32);
		Sizes.push_back(15);
		Sizes.push_back(17);
		Sizes.push_back(1);

		for(std::size_t FormatIndex = 0; FormatIndex < Formats.size(); ++FormatIndex)
		for(std::size_t SizeIndex = 0; SizeIndex < Sizes.size(); ++SizeIndex)
		{
			gli::texture2d_array::extent_type Size(Sizes[SizeIndex]);

			gli::texture2d_array TextureA(Formats[FormatIndex], Size, 1, gli::levels(Size));
			gli::texture2d_array TextureB(Formats[FormatIndex], Size, 1);

			Error += TextureA == TextureB ? 0 : 1;
		}

		return Error;
	}
}//namespace alloc

namespace query
{
	int test()
	{
		int Error(0);

		gli::texture2d_array::size_type Layers(2);
		gli::texture2d_array::size_type Levels(2);

		gli::texture2d_array Texture(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture2d_array::extent_type(2), Layers, Levels);

		gli::texture2d_array::size_type Size = Texture.size();

		Error += Size == sizeof(glm::u8vec4) * 5 * Layers ? 0 : 1;
		Error += Texture.format() == gli::FORMAT_RGBA8_UINT_PACK8 ? 0 : 1;
		Error += Texture.layers() == Layers ? 0 : 1;
		Error += Texture.levels() == Levels ? 0 : 1;
		Error += !Texture.empty() ? 0 : 1;
		Error += Texture.extent().x == 2 ? 0 : 1;
		Error += Texture.extent().y == 2 ? 0 : 1;

		return Error;
	}
}//namespace query

namespace access_map
{
	int test()
	{
		int Error(0);

		{
			gli::texture2d_array Texture2DArray(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture2d_array::extent_type(2), 2, 1);
			GLI_ASSERT(!Texture2DArray.empty());

			gli::texture2d Texture0 = Texture2DArray[0];
			gli::texture2d Texture1 = Texture2DArray[1];
		
			std::size_t Size0 = Texture0.size();
			std::size_t Size1 = Texture1.size();

			Error += Size0 == sizeof(glm::u8vec4) * 4 ? 0 : 1;
			Error += Size1 == sizeof(glm::u8vec4) * 4 ? 0 : 1;

			for(std::size_t i = 0; i < 4; ++i)
			{
				*(Texture0.data<glm::u8vec4>() + i) = glm::u8vec4(255, 127, 0, 255);
				*(Texture1.data<glm::u8vec4>() + i) = glm::u8vec4(0, 127, 255, 255);
			}

			glm::u8vec4 * PointerA = Texture0.data<glm::u8vec4>();
			glm::u8vec4 * PointerB = Texture1.data<glm::u8vec4>();

			glm::u8vec4 * Pointer0 = Texture2DArray.data<glm::u8vec4>() + 0;
			glm::u8vec4 * Pointer1 = Texture2DArray.data<glm::u8vec4>() + 4;

			Error += PointerA == Pointer0 ? 0 : 1;
			Error += PointerB == Pointer1 ? 0 : 1;

			glm::u8vec4 ColorA = *Texture0.data<glm::u8vec4>();
			glm::u8vec4 ColorB = *Texture1.data<glm::u8vec4>();

			glm::u8vec4 Color0 = *Pointer0;
			glm::u8vec4 Color1 = *Pointer1;

			Error += ColorA == Color0 ? 0 : 1;
			Error += ColorB == Color1 ? 0 : 1;

			Error += glm::all(glm::equal(Color0, glm::u8vec4(255, 127, 0, 255))) ? 0 : 1;
			Error += glm::all(glm::equal(Color1, glm::u8vec4(0, 127, 255, 255))) ? 0 : 1;
		}

		return Error;
	}
}//namespace access_map

namespace size
{
	struct entry
	{
		entry(
			gli::format const & Format,
			gli::texture2d_array::extent_type const & Dimensions,
			gli::texture2d_array::size_type const & Size) :
				Format(Format),
				Dimensions(Dimensions),
				Size(Size)
		{}

		gli::format Format;
		gli::texture2d_array::extent_type Dimensions;
		gli::texture2d_array::size_type Size;
	};

	int test()
	{
		int Error(0);

		std::vector<entry> Tests;
		Tests.push_back(entry(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture2d_array::extent_type(4), 64 * 2));
		Tests.push_back(entry(gli::FORMAT_R8_UINT_PACK8, gli::texture2d_array::extent_type(4), 16 * 2));
		Tests.push_back(entry(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, gli::texture2d_array::extent_type(4), 8 * 2));
		Tests.push_back(entry(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, gli::texture2d_array::extent_type(2), 8 * 2));
		Tests.push_back(entry(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, gli::texture2d_array::extent_type(1), 8 * 2));
		Tests.push_back(entry(gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16, gli::texture2d_array::extent_type(4), 16 * 2));

		for(std::size_t i = 0; i < Tests.size(); ++i)
		{
			gli::texture2d_array Texture2DArray(
				Tests[i].Format,
				gli::texture2d_array::extent_type(4),
				gli::texture2d_array::size_type(2),
				gli::texture2d_array::size_type(1));

			Error += Texture2DArray.size() == Tests[i].Size ? 0 : 1;
			GLI_ASSERT(!Error);
		}

		for(std::size_t i = 0; i < Tests.size(); ++i)
		{
			gli::texture2d_array Texture2DArray(
				Tests[i].Format,
				gli::texture2d_array::extent_type(4),
				gli::texture2d_array::size_type(2),
				gli::texture2d_array::size_type(1));

			gli::texture2d Texture2D = Texture2DArray[0];

			Error += Texture2DArray.size() == Tests[i].Size ? 0 : 1;
			GLI_ASSERT(!Error);
		}

		return Error;
	}
}//namespace size

namespace loader
{
	int test()
	{
		int Error(0);

		gli::texture2d_array TextureArrayRGBA8(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d_array::extent_type(1), 2, 1);

		{
			std::vector<glm::u8vec4> Color;
			Color.push_back(glm::u8vec4(255, 128,   0, 255));
			Color.push_back(glm::u8vec4(  0, 128, 255, 255));

			for(gli::texture2d_array::size_type LayerIndex = 0; LayerIndex < TextureArrayRGBA8.layers(); ++LayerIndex)
			for(gli::texture2d_array::size_type TexelIndex = 0; TexelIndex < TextureArrayRGBA8[LayerIndex].size<glm::u8vec4>(); ++TexelIndex)
				*(TextureArrayRGBA8[LayerIndex].data<glm::u8vec4>() + TexelIndex) = Color[LayerIndex];

			gli::save_dds(TextureArrayRGBA8, "texture2DArray_rgba8_unorm.dds");
		}

		{
			gli::texture2d Texture0(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(1), 1);
			gli::texture2d Texture1(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(1), 1);

			*Texture0.data<glm::u8vec4>() = glm::u8vec4(255, 128, 0, 255);
			*Texture1.data<glm::u8vec4>() = glm::u8vec4(  0, 128, 255, 255);

			gli::save_dds(Texture0, "texture2D0_rgba8_unorm.dds");
			gli::save_dds(Texture1, "texture2D1_rgba8_unorm.dds");
		}

		{
			gli::texture2d_array TextureArrayRGBA8Loaded(gli::load_dds("texture2DArray_rgba8_unorm.dds"));
			gli::texture2d Texture0(gli::load_dds("texture2D0_rgba8_unorm.dds"));
			gli::texture2d Texture1(gli::load_dds("texture2D1_rgba8_unorm.dds"));

			Error += TextureArrayRGBA8Loaded[0] == Texture0 ? 0 : 1;
			Error += TextureArrayRGBA8Loaded[1] == Texture1 ? 0 : 1;
		}

		return Error;
	}
}//namespace loader

namespace load
{
	int test()
	{
		int Error(0);

		{
			gli::texture const Texture = gli::load(path("array_r8_uint.ktx"));
			Error += Texture.target() == gli::TARGET_2D_ARRAY ? 0 : 1;
		}

		{
			gli::texture const Texture = gli::load(path("array_r8_uint.dds"));
			Error += Texture.target() == gli::TARGET_2D_ARRAY ? 0 : 1;
		}

		return Error;
	}
}//namespace load

namespace fetch_srgba8_unorm
{
	int test()
	{
		int Error(0);

		gli::texture2d_array TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(2, 2), 2);
		{
			TextureA.clear(glm::u8vec4(255, 127, 0, 255));
			TextureA.store<glm::u8vec4>(gli::texture2d_array::extent_type(0, 0), 1, 0, glm::u8vec4(255, 0, 0, 255));
			TextureA.store<glm::u8vec4>(gli::texture2d_array::extent_type(1, 0), 1, 0, glm::u8vec4(255, 255, 0, 255));
			TextureA.store<glm::u8vec4>(gli::texture2d_array::extent_type(1, 1), 1, 0, glm::u8vec4(0, 255, 0, 255));
			TextureA.store<glm::u8vec4>(gli::texture2d_array::extent_type(0, 1), 1, 0, glm::u8vec4(0, 0, 255, 255));
			TextureA.store<glm::u8vec4>(gli::texture2d_array::extent_type(0, 0), 1, 1, glm::u8vec4(0, 0, 0, 255));
			gli::save_dds(TextureA, "srgba8_unorm_4pixels.dds");
		}

		gli::texture2d_array TextureB(gli::load_dds("srgba8_unorm_4pixels.dds"));
		{
			glm::u8vec4 A = TextureB.load<glm::u8vec4>(gli::texture2d::extent_type(0, 0), 1, 0);
			Error += glm::all(glm::equal(A, glm::u8vec4(255, 0, 0, 255))) ? 0 : 1;
			glm::u8vec4 B = TextureB.load<glm::u8vec4>(gli::texture2d::extent_type(1, 0), 1, 0);
			Error += glm::all(glm::equal(B, glm::u8vec4(255, 255, 0, 255))) ? 0 : 1;
			glm::u8vec4 C = TextureB.load<glm::u8vec4>(gli::texture2d::extent_type(1, 1), 1, 0);
			Error += glm::all(glm::equal(C, glm::u8vec4(0, 255, 0, 255))) ? 0 : 1;
			glm::u8vec4 D = TextureB.load<glm::u8vec4>(gli::texture2d::extent_type(0, 1), 1, 0);
			Error += glm::all(glm::equal(D, glm::u8vec4(0, 0, 255, 255))) ? 0 : 1;
			glm::u8vec4 E = TextureB.load<glm::u8vec4>(gli::texture2d::extent_type(0, 0), 1, 1);
			Error += glm::all(glm::equal(E, glm::u8vec4(0, 0, 0, 255))) ? 0 : 1;
		}

		Error += TextureA == TextureB ? 0 : 1;

		return Error;
	}
}//namespace fetch_srgba8_unorm

namespace load_store
{
	template <typename genType>
	int run(gli::format Format, std::array<genType, 8> const & TestSamples)
	{
		int Error = 0;

		gli::texture2d_array::extent_type const Dimensions(8, 4);

		gli::texture2d_array TextureA(Format, Dimensions, 3);
		TextureA.clear();
		for (std::size_t i = 0, n = 8; i < n; ++i)
			*(TextureA.data<genType>(2, 0, 1) + i) = TestSamples[i];

		gli::texture2d_array TextureB(Format, Dimensions, 3);
		TextureB.clear();
		for (std::size_t i = 0, n = 8; i < n; ++i)
			TextureB.store(gli::texture2d_array::extent_type(i % 4, i / 4), 2, 1, TestSamples[i]);

		std::array<genType, 8> LoadedSamplesA;
		for (std::size_t i = 0, n = 8; i < n; ++i)
			LoadedSamplesA[i] = TextureA.load<genType>(gli::texture2d_array::extent_type(i % 4, i / 4), 2, 1);

		std::array<genType, 8> LoadedSamplesB;
		for (std::size_t i = 0, n = 8; i < n; ++i)
			LoadedSamplesB[i] = TextureB.load<genType>(gli::texture2d_array::extent_type(i % 4, i / 4), 2, 1);

		for (std::size_t i = 0, n = 8; i < n; ++i)
			Error += LoadedSamplesA[i] == TestSamples[i] ? 0 : 1;

		for (std::size_t i = 0, n = 8; i < n; ++i)
			Error += LoadedSamplesB[i] == TestSamples[i] ? 0 : 1;

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture2d_array TextureC(TextureA, 2, 2, 1, 1);
		gli::texture2d_array TextureD(TextureB, 2, 2, 1, 1);

		Error += TextureC == TextureD ? 0 : 1;

		return Error;
	}

	int test()
	{
		int Error = 0;

		{
			std::array<glm::f32vec1, 8> TestSamples{
			{
				glm::f32vec1(0.0f),
				glm::f32vec1(1.0f),
				glm::f32vec1(-1.0f),
				glm::f32vec1(0.5f),
				glm::f32vec1(-0.5f),
				glm::f32vec1(0.2f),
				glm::f32vec1(-0.2f),
				glm::f32vec1(0.9f)
			}};

			Error += run(gli::FORMAT_R32_SFLOAT_PACK32, TestSamples);
		}

		{
			std::array<glm::f32vec2, 8> TestSamples{
			{
				glm::f32vec2(-1.0f,-1.0f),
				glm::f32vec2(-0.5f,-0.5f),
				glm::f32vec2(0.0f, 0.0f),
				glm::f32vec2(0.5f, 0.5f),
				glm::f32vec2(1.0f, 1.0f),
				glm::f32vec2(-1.0f, 1.0f),
				glm::f32vec2(-0.5f, 0.5f),
				glm::f32vec2(0.0f, 0.0f)
			}};

			Error += run(gli::FORMAT_RG32_SFLOAT_PACK32, TestSamples);
		}

		{
			std::array<glm::f32vec3, 8> TestSamples{
			{
				glm::f32vec3(-1.0f, 0.0f, 1.0f),
				glm::f32vec3(-0.5f, 0.0f, 0.5f),
				glm::f32vec3(-0.2f, 0.0f, 0.2f),
				glm::f32vec3(-0.0f, 0.0f, 0.0f),
				glm::f32vec3(0.1f, 0.2f, 0.3f),
				glm::f32vec3(-0.1f,-0.2f,-0.3f),
				glm::f32vec3(0.7f, 0.8f, 0.9f),
				glm::f32vec3(-0.7f,-0.8f,-0.9f)
			}};

			Error += run(gli::FORMAT_RGB32_SFLOAT_PACK32, TestSamples);
		}

		{
			std::array<glm::f32vec4, 8> TestSamples{
			{
				glm::f32vec4(-1.0f, 0.0f, 1.0f, 1.0f),
				glm::f32vec4(-0.5f, 0.0f, 0.5f, 1.0f),
				glm::f32vec4(-0.2f, 0.0f, 0.2f, 1.0f),
				glm::f32vec4(-0.0f, 0.0f, 0.0f, 1.0f),
				glm::f32vec4(0.1f, 0.2f, 0.3f, 1.0f),
				glm::f32vec4(-0.1f,-0.2f,-0.3f, 1.0f),
				glm::f32vec4(0.7f, 0.8f, 0.9f, 1.0f),
				glm::f32vec4(-0.7f,-0.8f,-0.9f, 1.0f)
			}};

			Error += run(gli::FORMAT_RGBA32_SFLOAT_PACK32, TestSamples);
		}

		{
			std::array<glm::i8vec1, 8> TestSamples{
			{
				glm::i8vec1(-128),
				glm::i8vec1(-127),
				glm::i8vec1(127),
				glm::i8vec1(64),
				glm::i8vec1(-64),
				glm::i8vec1(1),
				glm::i8vec1(-1),
				glm::i8vec1(0)
			}};

			Error += run(gli::FORMAT_R8_SINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_R8_SNORM_PACK8, TestSamples);
		}

		{
			std::array<glm::i8vec2, 8> TestSamples{
			{
				glm::i8vec2(-128, -96),
				glm::i8vec2(-64,  96),
				glm::i8vec2(-128,  64),
				glm::i8vec2(127,  32),
				glm::i8vec2(0, 126),
				glm::i8vec2(-48,  48),
				glm::i8vec2(-127, 127),
				glm::i8vec2(64,   0)
			}};

			Error += run(gli::FORMAT_RG8_UINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RG8_UNORM_PACK8, TestSamples);
		}

		{
			std::array<glm::i8vec3, 8> TestSamples{
			{
				glm::i8vec3(-128,   0,   0),
				glm::i8vec3(-128, 127,   0),
				glm::i8vec3(-128, -96,   0),
				glm::i8vec3(127,-128,   0),
				glm::i8vec3(0, 127,   0),
				glm::i8vec3(0, 127,-127),
				glm::i8vec3(0,  64, -64),
				glm::i8vec3(-32,  32,  96)
			}};

			Error += run(gli::FORMAT_RGB8_SINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGB8_SNORM_PACK8, TestSamples);
		}

		{
			std::array<glm::i8vec4, 8> TestSamples{
			{
				glm::i8vec4(-127,   0,   0, 127),
				glm::i8vec4(-128,  96,   0,-128),
				glm::i8vec4(127,  64,   0,   1),
				glm::i8vec4(0, -64,   0,   2),
				glm::i8vec4(-95,  32,   0,   3),
				glm::i8vec4(95, -32, 127,   4),
				glm::i8vec4(-63,  16,-128,  -1),
				glm::i8vec4(63, -16,-127,  -2)
			}};

			Error += run(gli::FORMAT_RGBA8_SINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGBA8_SNORM_PACK8, TestSamples);
		}

		{
			std::array<glm::u8vec1, 8> TestSamples{
			{
				glm::u8vec1(255),
				glm::u8vec1(224),
				glm::u8vec1(192),
				glm::u8vec1(128),
				glm::u8vec1(64),
				glm::u8vec1(32),
				glm::u8vec1(16),
				glm::u8vec1(0)
			}};

			Error += run(gli::FORMAT_R8_UINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_R8_UNORM_PACK8, TestSamples);
			Error += run(gli::FORMAT_R8_SRGB_PACK8, TestSamples);
		}

		{
			std::array<glm::u8vec2, 8> TestSamples{
			{
				glm::u8vec2(255,   0),
				glm::u8vec2(255, 128),
				glm::u8vec2(255, 255),
				glm::u8vec2(128, 255),
				glm::u8vec2(0, 255),
				glm::u8vec2(0, 255),
				glm::u8vec2(0,   0),
				glm::u8vec2(255,   0)
			}};

			Error += run(gli::FORMAT_RG8_UINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RG8_UNORM_PACK8, TestSamples);
			Error += run(gli::FORMAT_RG8_SRGB_PACK8, TestSamples);
		}

		{
			std::array<glm::u8vec3, 8> TestSamples{
			{
				glm::u8vec3(255,   0,   0),
				glm::u8vec3(255, 128,   0),
				glm::u8vec3(255, 255,   0),
				glm::u8vec3(128, 255,   0),
				glm::u8vec3(0, 255,   0),
				glm::u8vec3(0, 255, 255),
				glm::u8vec3(0,   0, 255),
				glm::u8vec3(255,   0, 255)
			}};

			Error += run(gli::FORMAT_RGB8_UINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGB8_UNORM_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGB8_SRGB_PACK8, TestSamples);
		}

		{
			std::array<glm::u8vec4, 8> TestSamples{
			{
				glm::u8vec4(255,   0,   0, 255),
				glm::u8vec4(255, 128,   0, 255),
				glm::u8vec4(255, 255,   0, 255),
				glm::u8vec4(128, 255,   0, 255),
				glm::u8vec4(0, 255,   0, 255),
				glm::u8vec4(0, 255, 255, 255),
				glm::u8vec4(0,   0, 255, 255),
				glm::u8vec4(255,   0, 255, 255)
			}};

			Error += run(gli::FORMAT_RGBA8_UINT_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGBA8_UNORM_PACK8, TestSamples);
			Error += run(gli::FORMAT_RGBA8_SRGB_PACK8, TestSamples);
		}

		{
			std::array<glm::u16vec1, 8> TestSamples{
			{
				glm::u16vec1(65535),
				glm::u16vec1(32767),
				glm::u16vec1(192),
				glm::u16vec1(128),
				glm::u16vec1(64),
				glm::u16vec1(32),
				glm::u16vec1(16),
				glm::u16vec1(0)
			}};

			Error += run(gli::FORMAT_R16_UINT_PACK16, TestSamples);
			Error += run(gli::FORMAT_R16_UNORM_PACK16, TestSamples);
		}

		{
			std::array<glm::u16vec2, 8> TestSamples{
			{
				glm::u16vec2(255,   0),
				glm::u16vec2(255, 128),
				glm::u16vec2(255, 255),
				glm::u16vec2(128, 255),
				glm::u16vec2(0, 255),
				glm::u16vec2(0, 255),
				glm::u16vec2(0,   0),
				glm::u16vec2(255,   0)
			}};

			Error += run(gli::FORMAT_RG16_UINT_PACK16, TestSamples);
			Error += run(gli::FORMAT_RG16_UNORM_PACK16, TestSamples);
		}

		{
			std::array<glm::u16vec3, 8> TestSamples{
			{
				glm::u16vec3(255,   0,   0),
				glm::u16vec3(255, 128,   0),
				glm::u16vec3(255, 255,   0),
				glm::u16vec3(128, 255,   0),
				glm::u16vec3(0, 255,   0),
				glm::u16vec3(0, 255, 255),
				glm::u16vec3(0,   0, 255),
				glm::u16vec3(255,   0, 255)
			}};

			Error += run(gli::FORMAT_RGB16_UINT_PACK16, TestSamples);
			Error += run(gli::FORMAT_RGB16_UNORM_PACK16, TestSamples);
		}

		{
			std::array<glm::u16vec4, 8> TestSamples{
			{
				glm::u16vec4(255,   0,   0, 255),
				glm::u16vec4(255, 128,   0, 255),
				glm::u16vec4(255, 255,   0, 255),
				glm::u16vec4(128, 255,   0, 255),
				glm::u16vec4(0, 255,   0, 255),
				glm::u16vec4(0, 255, 255, 255),
				glm::u16vec4(0,   0, 255, 255),
				glm::u16vec4(255,   0, 255, 255)
			}};

			Error += run(gli::FORMAT_RGBA16_UINT_PACK16, TestSamples);
			Error += run(gli::FORMAT_RGBA16_UNORM_PACK16, TestSamples);
		}

		{
			std::array<glm::u32vec1, 8> TestSamples{
			{
				glm::u32vec1(65535),
				glm::u32vec1(32767),
				glm::u32vec1(192),
				glm::u32vec1(128),
				glm::u32vec1(64),
				glm::u32vec1(32),
				glm::u32vec1(16),
				glm::u32vec1(0)
			}};

			Error += run(gli::FORMAT_R32_UINT_PACK32, TestSamples);
		}

		{
			std::array<glm::u32vec2, 8> TestSamples{
			{
				glm::u32vec2(255,   0),
				glm::u32vec2(255, 128),
				glm::u32vec2(255, 255),
				glm::u32vec2(128, 255),
				glm::u32vec2(0, 255),
				glm::u32vec2(0, 255),
				glm::u32vec2(0,   0),
				glm::u32vec2(255,   0)
			}};

			Error += run(gli::FORMAT_RG32_UINT_PACK32, TestSamples);
		}

		{
			std::array<glm::u32vec3, 8> TestSamples{
			{
				glm::u32vec3(255,   0,   0),
				glm::u32vec3(255, 128,   0),
				glm::u32vec3(255, 255,   0),
				glm::u32vec3(128, 255,   0),
				glm::u32vec3(0, 255,   0),
				glm::u32vec3(0, 255, 255),
				glm::u32vec3(0,   0, 255),
				glm::u32vec3(255,   0, 255)
			}};

			Error += run(gli::FORMAT_RGB32_UINT_PACK32, TestSamples);
		}

		{
			std::array<glm::u32vec4, 8> TestSamples{
			{
				glm::u32vec4(255,   0,   0, 255),
				glm::u32vec4(255, 128,   0, 255),
				glm::u32vec4(255, 255,   0, 255),
				glm::u32vec4(128, 255,   0, 255),
				glm::u32vec4(0, 255,   0, 255),
				glm::u32vec4(0, 255, 255, 255),
				glm::u32vec4(0,   0, 255, 255),
				glm::u32vec4(255,   0, 255, 255)
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

		gli::texture2d_array Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d_array::extent_type(8), 1, 5);
		Texture.clear(Black);

		glm::u8vec4 const TexelA = Texture.load<glm::u8vec4>(gli::texture2d_array::extent_type(0), 0, 0);
		glm::u8vec4 const TexelB = Texture.load<glm::u8vec4>(gli::texture2d_array::extent_type(0), 0, 1);
		glm::u8vec4 const TexelC = Texture.load<glm::u8vec4>(gli::texture2d_array::extent_type(0), 0, 2);

		Error += TexelA == Black ? 0 : 1;
		Error += TexelB == Black ? 0 : 1;
		Error += TexelC == Black ? 0 : 1;

		Texture.clear<glm::u8vec4>(0, 0, 1, glm::u8vec4(255, 127, 0, 255));

		gli::texture2d::extent_type Coords(0);
		for(; Coords.y < Texture.extent(1).y; ++Coords.y)
		for(; Coords.x < Texture.extent(1).x; ++Coords.x)
		{
			glm::u8vec4 const TexelD = Texture.load<glm::u8vec4>(Coords, 0, 1);
			Error += TexelD == Color ? 0 : 1;
		}

		gli::texture2d_array TextureView(Texture, 0, 0, 1, 1);

		gli::texture2d_array TextureImage(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d_array::extent_type(4), 1, 1);
		TextureImage.clear(Color);

		Error += TextureView == TextureImage ? 0 : 1;

		return Error;
	}
}//namespace clear

int main()
{
	int Error(0);

	Error += alloc::test();
	Error += size::test();
	Error += query::test();
	Error += access_map::test();
	Error += loader::test();
	Error += load::test();
	Error += fetch_srgba8_unorm::test();
	Error += load_store::test();
	Error += clear::test();

	return Error;
}

