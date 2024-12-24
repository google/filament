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

	std::vector<gli::texture3d::extent_type::value_type> Sizes;
	Sizes.push_back(16);
	Sizes.push_back(32);
	Sizes.push_back(15);
	Sizes.push_back(17);
	Sizes.push_back(1);

	for(std::size_t FormatIndex = 0; FormatIndex < Formats.size(); ++FormatIndex)
	for(std::size_t SizeIndex = 0; SizeIndex < Sizes.size(); ++SizeIndex)
	{
		gli::texture3d::extent_type Size(Sizes[SizeIndex]);

		gli::texture3d TextureA(Formats[FormatIndex], Size, gli::levels(Size));
		gli::texture3d TextureB(Formats[FormatIndex], Size);

		Error += TextureA == TextureB ? 0 : 1;
	}

	return Error;
}

int test_texture3d_clear()
{
	int Error(0);

	glm::u8vec4 const Orange(255, 127, 0, 255);

	gli::texture3d::extent_type Size(16);

	gli::texture3d Texture(gli::FORMAT_RGBA8_UINT_PACK8, Size);

	Texture.clear<glm::u8vec4>(Orange);

	return Error;
}

int test_texture3d_query()
{
	int Error(0);

	gli::texture3d::extent_type Size(2);

	gli::texture3d Texture(gli::FORMAT_RGBA8_UINT_PACK8, Size);

	Error += Texture.size() == sizeof(glm::u8vec4) * 9 ? 0 : 1;
	Error += Texture.format() == gli::FORMAT_RGBA8_UINT_PACK8 ? 0 : 1;
	Error += Texture.levels() == 2 ? 0 : 1;
	Error += !Texture.empty() ? 0 : 1;
	Error += Texture.extent().x == 2 ? 0 : 1;
	Error += Texture.extent().y == 2 ? 0 : 1;
	Error += Texture.extent().z == 2 ? 0 : 1;

	return Error;
}

int test_texture3d_access()
{
	int Error(0);

	{
		glm::u8vec4 const Orange(255, 127, 0, 255);

		gli::image Image(gli::FORMAT_RGBA8_UINT_PACK8, gli::image::extent_type(2));

		gli::texture3d Texture(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture3d::extent_type(2), gli::texture3d::size_type(1));

		Error += Image.size() == Texture.size() ? 0 : 1;
	}

	{
		gli::texture3d Texture(
			gli::FORMAT_RGBA8_UINT_PACK8,
			gli::texture3d::extent_type(2),
			gli::texture3d::size_type(2));
		GLI_ASSERT(!Texture.empty());

		gli::image Image0 = Texture[0];
		gli::image Image1 = Texture[1];
		
		std::size_t Size0 = Image0.size();
		std::size_t Size1 = Image1.size();

		Error += Size0 == sizeof(glm::u8vec4) * 8 ? 0 : 1;
		Error += Size1 == sizeof(glm::u8vec4) * 1 ? 0 : 1;

		*Image0.data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);
		*Image1.data<glm::u8vec4>() = glm::u8vec4(0, 127, 255, 255);

		glm::u8vec4 * PointerA = Image0.data<glm::u8vec4>();
		glm::u8vec4 * PointerB = Image1.data<glm::u8vec4>();

		glm::u8vec4 * Pointer0 = Texture.data<glm::u8vec4>() + 0;
		glm::u8vec4 * Pointer1 = Texture.data<glm::u8vec4>() + 8;

		Error += PointerA == Pointer0 ? 0 : 1;
		Error += PointerB == Pointer1 ? 0 : 1;

		glm::u8vec4 ColorA = *Image0.data<glm::u8vec4>();
		glm::u8vec4 ColorB = *Image1.data<glm::u8vec4>();

		glm::u8vec4 Color0 = *Pointer0;
		glm::u8vec4 Color1 = *Pointer1;

		Error += ColorA == Color0 ? 0 : 1;
		Error += ColorB == Color1 ? 0 : 1;

		Error += glm::all(glm::equal(Color0, glm::u8vec4(255, 127, 0, 255))) ? 0 : 1;
		Error += glm::all(glm::equal(Color1, glm::u8vec4(0, 127, 255, 255))) ? 0 : 1;
	}

	return Error;
}

struct test
{
	test(
		gli::format const & Format,
		gli::texture3d::extent_type const & Dimensions,
		gli::texture3d::size_type const & Size) :
		Format(Format),
		Dimensions(Dimensions),
		Size(Size)
	{}

	gli::format Format;
	gli::texture3d::extent_type Dimensions;
	gli::texture3d::size_type Size;
};

int test_texture3d_size()
{
	int Error(0);

	std::vector<test> Tests;
	Tests.push_back(test(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture3d::extent_type(4), 256));
	Tests.push_back(test(gli::FORMAT_R8_UINT_PACK8, gli::texture3d::extent_type(4), 64));
	Tests.push_back(test(gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, gli::texture3d::extent_type(4), 32));
	Tests.push_back(test(gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, gli::texture3d::extent_type(2), 32));
	Tests.push_back(test(gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, gli::texture3d::extent_type(1), 32));
	Tests.push_back(test(gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16, gli::texture3d::extent_type(4), 64));

	for(std::size_t i = 0; i < Tests.size(); ++i)
	{
		gli::texture3d Texture(
			Tests[i].Format,
			gli::texture3d::extent_type(4),
			1);

		Error += Texture.size() == Tests[i].Size ? 0 : 1;
		GLI_ASSERT(!Error);
	}

	for(std::size_t i = 0; i < Tests.size(); ++i)
	{
		gli::texture3d Texture(
			Tests[i].Format,
			gli::texture3d::extent_type(4),
			1);

		gli::image Image = Texture[0];

		Error += Image.size() == Tests[i].Size ? 0 : 1;
		GLI_ASSERT(!Error);
	}

	return Error;
}

namespace load_store
{
	std::size_t index3D(gli::texture3d const & Texture, std::size_t Level, std::size_t x, std::size_t y, std::size_t z)
	{
		gli::texture3d::extent_type const Extent = Texture.extent(Level);
		return x + y * Extent.x + z * Extent.x * Extent.y;
	}

	template <typename genType>
	int run(gli::format Format, std::array<genType, 8> const & TestSamples)
	{
		int Error = 0;

		gli::texture3d::extent_type const Extent(4, 4, 4);

		gli::texture3d TextureA(Format, Extent);
		TextureA.clear();
		for (std::size_t i = 0, n = 8; i < n; ++i)
			*(TextureA.data<genType>(0, 0, 1) + i) = TestSamples[i];

		gli::texture3d TextureB(Format, Extent);
		TextureB.clear();
		for (std::size_t z = 0; z < 2; ++z)
		for (std::size_t y = 0; y < 2; ++y)
		for (std::size_t x = 0; x < 2; ++x)
			TextureB.store(gli::texture3d::extent_type(x, y, z), 1, TestSamples[index3D(TextureB, 1, x, y, z)]);

		std::array<genType, 8> LoadedSamplesA;
		for (std::size_t z = 0; z < 2; ++z)
		for (std::size_t y = 0; y < 2; ++y)
		for (std::size_t x = 0; x < 2; ++x)
			LoadedSamplesA[index3D(TextureB, 1, x, y, z)] = TextureA.load<genType>(gli::texture3d::extent_type(x, y, z), 1);

		std::array<genType, 8> LoadedSamplesB;
		for (std::size_t z = 0; z < 2; ++z)
		for (std::size_t y = 0; y < 2; ++y)
		for (std::size_t x = 0; x < 2; ++x)
			LoadedSamplesB[index3D(TextureB, 1, x, y, z)] = TextureB.load<genType>(gli::texture3d::extent_type(x, y, z), 1);

		for (std::size_t i = 0, n = 8; i < n; ++i)
			Error += LoadedSamplesA[i] == TestSamples[i] ? 0 : 1;

		for (std::size_t i = 0, n = 8; i < n; ++i)
			Error += LoadedSamplesB[i] == TestSamples[i] ? 0 : 1;

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture3d TextureC(TextureA, 1, 1);
		gli::texture3d TextureD(TextureB, 1, 1);

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

		gli::texture3d Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(8), 5);
		Texture.clear(Black);

		glm::u8vec4 const TexelA = Texture.load<glm::u8vec4>(gli::texture3d::extent_type(0), 0);
		glm::u8vec4 const TexelB = Texture.load<glm::u8vec4>(gli::texture3d::extent_type(0), 1);
		glm::u8vec4 const TexelC = Texture.load<glm::u8vec4>(gli::texture3d::extent_type(0), 2);

		Error += TexelA == Black ? 0 : 1;
		Error += TexelB == Black ? 0 : 1;
		Error += TexelC == Black ? 0 : 1;

		Texture.clear<glm::u8vec4>(0, 0, 1, glm::u8vec4(255, 127, 0, 255));

		gli::texture3d::extent_type Coords(0);
		for(; Coords.z < Texture.extent(1).z; ++Coords.z)
		for(; Coords.y < Texture.extent(1).y; ++Coords.y)
		for(; Coords.x < Texture.extent(1).x; ++Coords.x)
		{
			glm::u8vec4 const TexelD = Texture.load<glm::u8vec4>(Coords, 1);
			Error += TexelD == Color ? 0 : 1;
		}

		gli::texture3d TextureView(Texture, 1, 1);

		gli::texture3d TextureImage(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(4), 1);
		TextureImage.clear(Color);

		Error += TextureView == TextureImage ? 0 : 1;

		return Error;
	}
}//namespace clear

int main()
{
	int Error(0);

	Error += test_alloc();
	Error += test_texture3d_size();
	Error += test_texture3d_query();
	Error += test_texture3d_clear();
	Error += test_texture3d_access();
	Error += load_store::test();
	Error += clear::test();

	return Error;
}

