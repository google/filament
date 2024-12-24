#include <gli/sampler2d.hpp>
#include <gli/load.hpp>
#include <gli/save.hpp>
#include <gli/comparison.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/ext/vector_relational.hpp>
#include <ctime>
#include <limits>
#include <array>

#define ENABLE_INTEGER_TESTS 0

namespace fetch_rgb10a2_snorm
{
	int test()
	{
		int Error(0);

		glm::vec4 Colors[] =
		{
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
			glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(0.0f, 0.5f, 1.0f, 1.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
			glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)
		};

		glm::uint32 Packed[8];
		for (std::size_t i = 0; i < 8; ++i)
			Packed[i] = glm::packSnorm3x10_1x2(Colors[i]);

		gli::texture2d Texture(gli::FORMAT_RGB10A2_SNORM_PACK32, gli::texture2d::extent_type(4, 2), 1);
		for (std::size_t i = 0; i < 8; ++i)
			*(Texture.data<glm::uint32>() + i) = Packed[i];

		glm::uint32 Loaded[8];
		Loaded[0] = Texture.load<glm::uint32>(gli::texture2d::extent_type(0, 0), 0);
		Loaded[1] = Texture.load<glm::uint32>(gli::texture2d::extent_type(1, 0), 0);
		Loaded[2] = Texture.load<glm::uint32>(gli::texture2d::extent_type(2, 0), 0);
		Loaded[3] = Texture.load<glm::uint32>(gli::texture2d::extent_type(3, 0), 0);
		Loaded[4] = Texture.load<glm::uint32>(gli::texture2d::extent_type(0, 1), 0);
		Loaded[5] = Texture.load<glm::uint32>(gli::texture2d::extent_type(1, 1), 0);
		Loaded[6] = Texture.load<glm::uint32>(gli::texture2d::extent_type(2, 1), 0);
		Loaded[7] = Texture.load<glm::uint32>(gli::texture2d::extent_type(3, 1), 0);

		for (std::size_t i = 0; i < 8; ++i)
			Error += Packed[i] == Loaded[i] ? 0 : 1;

		glm::vec4 Unpacked[8];
		for (std::size_t i = 0; i < 8; ++i)
			Unpacked[i] = glm::unpackSnorm3x10_1x2(Loaded[i]);

		for (std::size_t i = 0; i < 8; ++i)
			Error += glm::all(glm::epsilonEqual(Unpacked[i], Colors[i], 0.01f)) ? 0 : 1;

		gli::fsampler2D Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));

		glm::vec4 Data0 = Sampler.texel_fetch(gli::texture2d::extent_type(0, 0), 0);
		glm::vec4 Data1 = Sampler.texel_fetch(gli::texture2d::extent_type(1, 0), 0);
		glm::vec4 Data2 = Sampler.texel_fetch(gli::texture2d::extent_type(2, 0), 0);
		glm::vec4 Data3 = Sampler.texel_fetch(gli::texture2d::extent_type(3, 0), 0);
		glm::vec4 Data4 = Sampler.texel_fetch(gli::texture2d::extent_type(0, 1), 0);
		glm::vec4 Data5 = Sampler.texel_fetch(gli::texture2d::extent_type(1, 1), 0);
		glm::vec4 Data6 = Sampler.texel_fetch(gli::texture2d::extent_type(2, 1), 0);
		glm::vec4 Data7 = Sampler.texel_fetch(gli::texture2d::extent_type(3, 1), 0);

		float const Epsilon = 1.f / 255.f * 0.5f;

		Error += glm::all(glm::epsilonEqual(Data0, Colors[0], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data1, Colors[1], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data2, Colors[2], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data3, Colors[3], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data4, Colors[4], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data5, Colors[5], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data6, Colors[6], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data7, Colors[7], Epsilon)) ? 0 : 1;

		return Error;
	}
}//namespace fetch_rgb10a2_snorm

namespace fetch_rgb10a2_unorm
{
	int test()
	{
		int Error(0);

		glm::vec4 Colors[] =
		{
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
			glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(0.0f, 0.5f, 1.0f, 1.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
			glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)
		};

		glm::uint32 Packed[8];
		for(std::size_t i = 0; i < 8; ++i)
			Packed[i] = glm::packUnorm3x10_1x2(Colors[i]);

		gli::texture2d Texture(gli::FORMAT_RGB10A2_UNORM_PACK32, gli::texture2d::extent_type(4, 2), 1);
		for(std::size_t i = 0; i < 8; ++i)
			*(Texture.data<glm::uint32>() + i) = Packed[i];

		glm::uint32 Loaded[8];
		Loaded[0] = Texture.load<glm::uint32>(gli::texture2d::extent_type(0, 0), 0);
		Loaded[1] = Texture.load<glm::uint32>(gli::texture2d::extent_type(1, 0), 0);
		Loaded[2] = Texture.load<glm::uint32>(gli::texture2d::extent_type(2, 0), 0);
		Loaded[3] = Texture.load<glm::uint32>(gli::texture2d::extent_type(3, 0), 0);
		Loaded[4] = Texture.load<glm::uint32>(gli::texture2d::extent_type(0, 1), 0);
		Loaded[5] = Texture.load<glm::uint32>(gli::texture2d::extent_type(1, 1), 0);
		Loaded[6] = Texture.load<glm::uint32>(gli::texture2d::extent_type(2, 1), 0);
		Loaded[7] = Texture.load<glm::uint32>(gli::texture2d::extent_type(3, 1), 0);

		for(std::size_t i = 0; i < 8; ++i)
			Error += Packed[i] == Loaded[i] ? 0 : 1;

		glm::vec4 Unpacked[8];
		for(std::size_t i = 0; i < 8; ++i)
			Unpacked[i] = glm::unpackUnorm3x10_1x2(Loaded[i]);

		for (std::size_t i = 0; i < 8; ++i)
			Error += glm::all(glm::epsilonEqual(Unpacked[i], Colors[i], 0.01f)) ? 0 : 1;

		gli::fsampler2D Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));

		glm::vec4 Data0 = Sampler.texel_fetch(gli::texture2d::extent_type(0, 0), 0);
		glm::vec4 Data1 = Sampler.texel_fetch(gli::texture2d::extent_type(1, 0), 0);
		glm::vec4 Data2 = Sampler.texel_fetch(gli::texture2d::extent_type(2, 0), 0);
		glm::vec4 Data3 = Sampler.texel_fetch(gli::texture2d::extent_type(3, 0), 0);
		glm::vec4 Data4 = Sampler.texel_fetch(gli::texture2d::extent_type(0, 1), 0);
		glm::vec4 Data5 = Sampler.texel_fetch(gli::texture2d::extent_type(1, 1), 0);
		glm::vec4 Data6 = Sampler.texel_fetch(gli::texture2d::extent_type(2, 1), 0);
		glm::vec4 Data7 = Sampler.texel_fetch(gli::texture2d::extent_type(3, 1), 0);

		float const Epsilon = 1.f / 255.f * 0.5f;

		Error += glm::all(glm::epsilonEqual(Data0, Colors[0], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data1, Colors[1], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data2, Colors[2], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data3, Colors[3], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data4, Colors[4], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data5, Colors[5], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data6, Colors[6], Epsilon)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Data7, Colors[7], Epsilon)) ? 0 : 1;

		return Error;
	}
}//namespace fetch_rgb10a2_unorm

namespace fetch_rgb10a2_uint
{
	int test()
	{
		int Error(0);

		glm::uvec4 Colors[] =
		{
			glm::uvec4(1023,  511,    0, 0),
			glm::uvec4( 511,    0,  255, 3),
			glm::uvec4(   0,   31,   15, 1),
			glm::uvec4( 255, 1023,    1, 0),
			glm::uvec4(1023,  511,    0, 0),
			glm::uvec4( 511,    0,  255, 3),
			glm::uvec4(   0,   31,   15, 1),
			glm::uvec4( 255, 1023,    1, 0)
		};

		glm::uint32 Packed[8];
		for(std::size_t i = 0; i < 8; ++i)
			Packed[i] = gli::packU3x10_1x2(Colors[i]);

		gli::texture2d Texture(gli::FORMAT_RGB10A2_UINT_PACK32, gli::texture2d::extent_type(4, 2), 1);
		for(std::size_t i = 0; i < 8; ++i)
			*(Texture.data<glm::uint32>() + i) = Packed[i];

		glm::uint32 Loaded[8];
		for(std::size_t i = 0; i < 8; ++i)
			Loaded[i] = Texture.load<glm::uint32>(gli::texture2d::extent_type(i % 4, i / 4), 0);

		for(std::size_t i = 0; i < 8; ++i)
			Error += Packed[i] == Loaded[i] ? 0 : 1;

		glm::uvec4 Unpacked[8];
		for(std::size_t i = 0; i < 8; ++i)
			Unpacked[i] = gli::unpackU3x10_1x2(Loaded[i]);

		for (std::size_t i = 0; i < 8; ++i)
			Error += Unpacked[i] == Colors[i] ? 0 : 1;

#if ENABLE_INTEGER_TESTS
		gli::usampler2D Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR);

		glm::uvec4 Data[8];
		for(std::size_t i = 0; i < 8; ++i)
			Data[i] = Sampler.texel_fetch(gli::texture2d::extent_type(i % 4, i / 4), 0);

		for(std::size_t i = 0; i < 8; ++i)
			Error += Data[i] == Colors[i] ? 0 : 1;
#endif//ENABLE_INTEGER_TESTS

		return Error;
	}
}//namespace fetch_rgb10a2_uint

namespace fetch_rgba32_sfloat
{
	int test()
	{
		int Error(0);

		glm::vec4 Colors[] =
		{
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
			glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(0.0f, 0.5f, 1.0f, 1.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
			glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)
		};

		gli::texture2d TextureA(gli::FORMAT_RGBA32_SFLOAT_PACK32, gli::texture2d::extent_type(4, 2), 1);
		for(std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			*(TextureA.data<glm::vec4>() + i) = Colors[i];

		gli::fsampler2D SamplerA(TextureA, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));

		glm::vec4 DataA[sizeof(Colors) / sizeof(Colors[0])];
		for(std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			DataA[i] = SamplerA.texel_fetch(gli::texture2d::extent_type(i % 4, i / 4), 0);

		float const Epsilon = 1.f / 255.f * 0.5f;
		for(std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			Error += glm::all(glm::epsilonEqual(DataA[i], Colors[i], Epsilon)) ? 0 : 1;

		gli::texture2d TextureB(gli::FORMAT_RGBA32_SFLOAT_PACK32, gli::texture2d::extent_type(8, 4));
		for (std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			TextureB.store<glm::vec4>(gli::extent2d(i % 4, i / 4), 1, Colors[i]);

		gli::fsampler2D SamplerB(TextureB, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));

		glm::vec4 DataB[sizeof(Colors) / sizeof(Colors[0])];
		for (std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			DataB[i] = SamplerB.texel_fetch(gli::texture2d::extent_type(i % 4, i / 4), 1);

		for (std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			Error += glm::all(glm::epsilonEqual(DataB[i], Colors[i], Epsilon)) ? 0 : 1;

		gli::texture2d TextureC(TextureB, 1, 1);

		Error += TextureA == TextureC ? 0 : 1;

		return Error;
	}
}//namespace fetch_rgba32_sfloat

namespace fetch_rgba64_sfloat
{
	int test()
	{
		int Error(0);

		glm::dvec4 Colors[] =
		{
			glm::dvec4(1.0f, 0.0f, 0.0f, 1.0f),
			glm::dvec4(1.0f, 0.5f, 0.0f, 1.0f),
			glm::dvec4(1.0f, 1.0f, 0.0f, 1.0f),
			glm::dvec4(0.0f, 1.0f, 0.0f, 1.0f),
			glm::dvec4(0.0f, 1.0f, 1.0f, 1.0f),
			glm::dvec4(0.0f, 0.5f, 1.0f, 1.0f),
			glm::dvec4(0.0f, 0.0f, 1.0f, 1.0f),
			glm::dvec4(1.0f, 0.0f, 1.0f, 1.0f)
		};

		gli::texture2d TextureA(gli::FORMAT_RGBA64_SFLOAT_PACK64, gli::texture2d::extent_type(4, 2), 1);
		for (std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			*(TextureA.data<glm::dvec4>() + i) = Colors[i];

		gli::dsampler2D SamplerA(TextureA, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::dvec4(0.0f, 0.5f, 1.0f, 1.0f));

		glm::dvec4 DataA[sizeof(Colors) / sizeof(Colors[0])];
		for (std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			DataA[i] = SamplerA.texel_fetch(gli::texture2d::extent_type(i % 4, i / 4), 0);

		double const Epsilon = 1.f / 255.f * 0.5f;
		for (std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			Error += glm::all(glm::epsilonEqual(DataA[i], Colors[i], Epsilon)) ? 0 : 1;

		gli::texture2d TextureB(gli::FORMAT_RGBA64_SFLOAT_PACK64, gli::texture2d::extent_type(8, 4));
		for (std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			TextureB.store<glm::dvec4>(gli::extent2d(i % 4, i / 4), 1, Colors[i]);

		gli::dsampler2D SamplerB(TextureB, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::dvec4(0.0f, 0.5f, 1.0f, 1.0f));

		glm::dvec4 DataB[sizeof(Colors) / sizeof(Colors[0])];
		for (std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			DataB[i] = SamplerB.texel_fetch(gli::texture2d::extent_type(i % 4, i / 4), 1);

		for (std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			Error += glm::all(glm::epsilonEqual(DataB[i], Colors[i], Epsilon)) ? 0 : 1;

		gli::texture2d TextureC(TextureB, 1, 1);

		Error += TextureA == TextureC ? 0 : 1;

		return Error;
	}
}//namespace fetch_rgba64_sfloat

namespace fetch_rgba8_unorm
{
	int test()
	{
		int Error(0);

		glm::vec4 Colors[] =
		{
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
			glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(0.0f, 0.5f, 1.0f, 1.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
			glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)
		};

		gli::texture2d TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(4, 2), 1);
		for (std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			*(TextureA.data<glm::u8vec4>() + i) = glm::u8vec4(Colors[i] * 255.f);

		gli::fsampler2D SamplerA(TextureA, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));

		glm::vec4 Data[sizeof(Colors) / sizeof(Colors[0])];
		for(std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			Data[i] = SamplerA.texel_fetch(gli::texture2d::extent_type(i % 4, i / 4), 0);

		float const Epsilon = 1.f / 255.f * 0.5f;
		for(std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			Error += glm::all(glm::epsilonEqual(Data[i], Colors[i], Epsilon)) ? 0 : 1;

		gli::texture2d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(4, 2), 1);
		TextureB.clear(gli::u8vec4(0, 0, 0, 255));

		gli::fsampler2D SamplerB(TextureB, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));
		for(std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			SamplerB.texel_write(gli::texture2d::extent_type(i % 4, i / 4), 0, Data[i]);

		Error += TextureA == TextureB ? 0 : 1;

		return Error;
	}
}//namespace fetch_rgba8_unorm

namespace fetch_rgba8_uint
{
	int test()
	{
		int Error(0);

		glm::u8vec4 Colors[] =
		{
			glm::u8vec4(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) * 255.f),
			glm::u8vec4(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f) * 255.f),
			glm::u8vec4(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) * 255.f),
			glm::u8vec4(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * 255.f),
			glm::u8vec4(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f) * 255.f),
			glm::u8vec4(glm::vec4(0.0f, 0.5f, 1.0f, 1.0f) * 255.f),
			glm::u8vec4(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * 255.f),
			glm::u8vec4(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) * 255.f)
		};

		gli::texture2d Texture(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture2d::extent_type(4, 2), 1);
		for(std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			*(Texture.data<glm::u8vec4>() + i) = Colors[i];

#if ENABLE_INTEGER_TESTS
		gli::usampler2D Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::u32vec4(0, 0, 0, 1));

		glm::u8vec4 Outputs[8];
		Outputs[0] = Sampler.texel_fetch(gli::texture2d::extent_type(0, 0), 0);
		Outputs[1] = Sampler.texel_fetch(gli::texture2d::extent_type(1, 0), 0);
		Outputs[2] = Sampler.texel_fetch(gli::texture2d::extent_type(2, 0), 0);
		Outputs[3] = Sampler.texel_fetch(gli::texture2d::extent_type(3, 0), 0);
		Outputs[4] = Sampler.texel_fetch(gli::texture2d::extent_type(0, 1), 0);
		Outputs[5] = Sampler.texel_fetch(gli::texture2d::extent_type(1, 1), 0);
		Outputs[6] = Sampler.texel_fetch(gli::texture2d::extent_type(2, 1), 0);
		Outputs[7] = Sampler.texel_fetch(gli::texture2d::extent_type(3, 1), 0);

		for(std::size_t i = 0, n = sizeof(Colors) / sizeof(Colors[0]); i < n; ++i)
			Error += glm::all(glm::equal(Outputs[i], Colors[i])) ? 0 : 1;
#endif//ENABLE_INTEGER_TESTS

		return Error;
	}
}//namespace fetch_rgba8_uint

namespace fetch_rgba8_srgb
{
	int test()
	{
		int Error(0);

		glm::vec4 Colors[] =
		{
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
			glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(0.0f, 0.5f, 1.0f, 1.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
			glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)
		};

		glm::u8vec4 StoreSRGB00(glm::convertLinearToSRGB(Colors[0]) * 255.f);
		glm::u8vec4 StoreSRGB10(glm::convertLinearToSRGB(Colors[1]) * 255.f);
		glm::u8vec4 StoreSRGB20(glm::convertLinearToSRGB(Colors[2]) * 255.f);
		glm::u8vec4 StoreSRGB30(glm::convertLinearToSRGB(Colors[3]) * 255.f);
		glm::u8vec4 StoreSRGB01(glm::convertLinearToSRGB(Colors[4]) * 255.f);
		glm::u8vec4 StoreSRGB11(glm::convertLinearToSRGB(Colors[5]) * 255.f);
		glm::u8vec4 StoreSRGB21(glm::convertLinearToSRGB(Colors[6]) * 255.f);
		glm::u8vec4 StoreSRGB31(glm::convertLinearToSRGB(Colors[7]) * 255.f);

		gli::texture2d Texture(gli::FORMAT_RGBA8_SRGB_PACK8, gli::texture2d::extent_type(4, 2), 1);
		Texture.store(gli::extent2d(0, 0), 0, StoreSRGB00);
		Texture.store(gli::extent2d(1, 0), 0, StoreSRGB10);
		Texture.store(gli::extent2d(2, 0), 0, StoreSRGB20);
		Texture.store(gli::extent2d(3, 0), 0, StoreSRGB30);
		Texture.store(gli::extent2d(0, 1), 0, StoreSRGB01);
		Texture.store(gli::extent2d(1, 1), 0, StoreSRGB11);
		Texture.store(gli::extent2d(2, 1), 0, StoreSRGB21);
		Texture.store(gli::extent2d(3, 1), 0, StoreSRGB31);

		{
			gli::fsampler2D Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));

			glm::vec4 Data0 = Sampler.texel_fetch(gli::texture2d::extent_type(0, 0), 0);
			glm::vec4 Data1 = Sampler.texel_fetch(gli::texture2d::extent_type(1, 0), 0);
			glm::vec4 Data2 = Sampler.texel_fetch(gli::texture2d::extent_type(2, 0), 0);
			glm::vec4 Data3 = Sampler.texel_fetch(gli::texture2d::extent_type(3, 0), 0);
			glm::vec4 Data4 = Sampler.texel_fetch(gli::texture2d::extent_type(0, 1), 0);
			glm::vec4 Data5 = Sampler.texel_fetch(gli::texture2d::extent_type(1, 1), 0);
			glm::vec4 Data6 = Sampler.texel_fetch(gli::texture2d::extent_type(2, 1), 0);
			glm::vec4 Data7 = Sampler.texel_fetch(gli::texture2d::extent_type(3, 1), 0);

			Error += glm::all(glm::epsilonEqual(Data0, Colors[0], 0.01f)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data1, Colors[1], 0.01f)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data2, Colors[2], 0.01f)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data3, Colors[3], 0.01f)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data4, Colors[4], 0.01f)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data5, Colors[5], 0.01f)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data6, Colors[6], 0.01f)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data7, Colors[7], 0.01f)) ? 0 : 1;
		}

		{
			gli::dsampler2D Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::dvec4(0.0f, 0.5f, 1.0f, 1.0f));

			glm::dvec4 Data0 = Sampler.texel_fetch(gli::texture2d::extent_type(0, 0), 0);
			glm::dvec4 Data1 = Sampler.texel_fetch(gli::texture2d::extent_type(1, 0), 0);
			glm::dvec4 Data2 = Sampler.texel_fetch(gli::texture2d::extent_type(2, 0), 0);
			glm::dvec4 Data3 = Sampler.texel_fetch(gli::texture2d::extent_type(3, 0), 0);
			glm::dvec4 Data4 = Sampler.texel_fetch(gli::texture2d::extent_type(0, 1), 0);
			glm::dvec4 Data5 = Sampler.texel_fetch(gli::texture2d::extent_type(1, 1), 0);
			glm::dvec4 Data6 = Sampler.texel_fetch(gli::texture2d::extent_type(2, 1), 0);
			glm::dvec4 Data7 = Sampler.texel_fetch(gli::texture2d::extent_type(3, 1), 0);

			Error += glm::all(glm::epsilonEqual(Data0, glm::dvec4(Colors[0]), 0.01)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data1, glm::dvec4(Colors[1]), 0.01)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data2, glm::dvec4(Colors[2]), 0.01)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data3, glm::dvec4(Colors[3]), 0.01)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data4, glm::dvec4(Colors[4]), 0.01)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data5, glm::dvec4(Colors[5]), 0.01)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data6, glm::dvec4(Colors[6]), 0.01)) ? 0 : 1;
			Error += glm::all(glm::epsilonEqual(Data7, glm::dvec4(Colors[7]), 0.01)) ? 0 : 1;
		}

		return Error;
	}
}//namespace fetch_rgba8_srgb

namespace fetch_bc1
{
	int test()
	{
		int Error = 0;

		gli::texture2d TextureLoad(gli::load_dds("kueken8_rgba_dxt1_unorm.dds"));
		gli::texture2d TextureSave(gli::FORMAT_RGBA8_UNORM_PACK8, TextureLoad.extent(), 1);

		gli::fsampler2D Sampler(TextureLoad, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));

		for(glm::uint32_t j = 0, n = TextureLoad.extent().y; j < n; ++j)
		for(glm::uint32_t i = 0, m = TextureLoad.extent().x; i < m; ++i)
		{
			glm::vec4 Data0 = Sampler.texel_fetch(gli::texture2d::extent_type(i, j), 0);
			Sampler.texel_write(gli::texture2d::extent_type(i, j), 0, Data0);
		}

		gli::save_dds(TextureSave, "fetch_rgba_dxt1_unorm_to_rgba8_unorm.dds");

		return Error;
	}
}//namespace fetch_bc1

int main()
{
	int Error(0);

	Error += fetch_rgba32_sfloat::test();
	Error += fetch_rgba64_sfloat::test();
	Error += fetch_rgba8_unorm::test();
	Error += fetch_rgba8_uint::test();
	Error += fetch_rgba8_srgb::test();
	Error += fetch_rgb10a2_unorm::test();
	Error += fetch_rgb10a2_snorm::test();
	Error += fetch_rgb10a2_uint::test();
	Error += fetch_bc1::test();

	return Error;
}

