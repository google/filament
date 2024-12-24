//////////////////////////////////////////////////////////////////////////////////
/// OpenGL Image (gli.g-truc.net)
///
/// Copyright (c) 2008 - 2015 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @ref core
/// @file gli/core/texture_2d.cpp
/// @date 2011-10-11 / 2013-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <gli/sampler2d.hpp>
#include <gli/comparison.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtc/epsilon.hpp>
#include <ctime>
#include <limits>
#include <array>

namespace load
{
	int test()
	{
		int Error(0);

		gli::texture2D Texture(gli::FORMAT_RGBA8_UNORM, gli::texture2D::dim_type(4, 2), 1);
		*(Texture.data<glm::u8vec4>() + 0) = glm::u8vec4(255,   0,   0, 255);
		*(Texture.data<glm::u8vec4>() + 1) = glm::u8vec4(255, 128,   0, 255);
		*(Texture.data<glm::u8vec4>() + 2) = glm::u8vec4(255, 255,   0, 255);
		*(Texture.data<glm::u8vec4>() + 3) = glm::u8vec4(128, 255,   0, 255);
		*(Texture.data<glm::u8vec4>() + 4) = glm::u8vec4(  0, 255,   0, 255);
		*(Texture.data<glm::u8vec4>() + 5) = glm::u8vec4(  0, 255, 255, 255);
		*(Texture.data<glm::u8vec4>() + 6) = glm::u8vec4(  0,   0, 255, 255);
		*(Texture.data<glm::u8vec4>() + 7) = glm::u8vec4(255,   0, 255, 255);

		glm::u8vec4 Data0 = Texture.load<glm::u8vec4>(gli::texture2D::dim_type(0, 0), 0);
		glm::u8vec4 Data1 = Texture.load<glm::u8vec4>(gli::texture2D::dim_type(1, 0), 0);
		glm::u8vec4 Data2 = Texture.load<glm::u8vec4>(gli::texture2D::dim_type(2, 0), 0);
		glm::u8vec4 Data3 = Texture.load<glm::u8vec4>(gli::texture2D::dim_type(3, 0), 0);
		glm::u8vec4 Data4 = Texture.load<glm::u8vec4>(gli::texture2D::dim_type(0, 1), 0);
		glm::u8vec4 Data5 = Texture.load<glm::u8vec4>(gli::texture2D::dim_type(1, 1), 0);
		glm::u8vec4 Data6 = Texture.load<glm::u8vec4>(gli::texture2D::dim_type(2, 1), 0);
		glm::u8vec4 Data7 = Texture.load<glm::u8vec4>(gli::texture2D::dim_type(3, 1), 0);

		Error += Data0 == glm::u8vec4(255,   0,   0, 255) ? 0 : 1;
		Error += Data1 == glm::u8vec4(255, 128,   0, 255) ? 0 : 1;
		Error += Data2 == glm::u8vec4(255, 255,   0, 255) ? 0 : 1;
		Error += Data3 == glm::u8vec4(128, 255,   0, 255) ? 0 : 1;
		Error += Data4 == glm::u8vec4(  0, 255,   0, 255) ? 0 : 1;
		Error += Data5 == glm::u8vec4(  0, 255, 255, 255) ? 0 : 1;
		Error += Data6 == glm::u8vec4(  0,   0, 255, 255) ? 0 : 1;
		Error += Data7 == glm::u8vec4(255,   0, 255, 255) ? 0 : 1;

		return Error;
	}
}//namespace load

/*
namespace sampler
{
	int test()
	{
		int Error(0);

		gli::texture2D::dim_type const Size(512);
		gli::texture2D TextureA(gli::FORMAT_RGBA8_UNORM, Size, 1);

		{
			std::clock_t TimeStart = std::clock();

			for(std::size_t y = 0; y < Size.y; ++y)
			for(std::size_t x = 0; x < Size.x; ++x)
			{
				gli::texel_write(TextureA, gli::texture2D::dim_type(x, y), 0, glm::u8vec4(255, 128,   0, 255));
			}

			std::clock_t TimeEnd = std::clock();

			printf("texel_write(texture2D) - Time: %lu\n", TimeEnd - TimeStart);
		}

		gli::texture2D TextureB(gli::FORMAT_RGBA8_UNORM, Size, 1);

		{
			gli::sampler2D<float> Sampler(TextureB, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));

			{
				std::clock_t TimeStart = std::clock();

				for(std::size_t y = 0; y < Size.y; ++y)
				for(std::size_t x = 0; x < Size.x; ++x)
				{
					Sampler.texel_write<glm::tvec4, glm::u8>(gli::texture2D::dim_type(x, y), 0, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
				}

				std::clock_t TimeEnd = std::clock();

				printf("sampler2D::texel_write - Time: %lu\n", TimeEnd - TimeStart);
			}

			{
				std::clock_t TimeStart = std::clock();

				for(float y = -0.5f; y < 1.5f; y += 0.025f)
				for(float x = -0.5f; x < 1.5f; x += 0.025f)
				{
					glm::vec4 ColorA = Sampler.texture_lod<glm::tvec4, glm::u8>(gli::texture2D::texcoord_type(x, y), 0.0f);
					Error += glm::all(glm::equal(ColorA, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f))) ? 0 : 1;
				}

				std::clock_t TimeEnd = std::clock();

				printf("sampler2D::texture_lod - Time: %lu\n", TimeEnd - TimeStart);
			}
		}

		Error += TextureA == TextureB ? 0 : 1;

		return Error;
	}
}//namespace sampler
*/

namespace texture_lod
{
	int test()
	{
		int Error = 0;

		{
			gli::texture2D Texture(gli::FORMAT_RGBA8_UNORM, gli::texture2D::dim_type(4), 1);
			Texture.clear(gli::u8vec4(0, 0, 0, 255));
			Texture.store(gli::dim2_t(0, 0), 0, gli::u8vec4(255, 127,   0, 255));
			Texture.store(gli::dim2_t(1, 0), 0, gli::u8vec4(255, 127,   0, 255));
			Texture.store(gli::dim2_t(0, 1), 0, gli::u8vec4(255, 127,   0, 255));
			Texture.store(gli::dim2_t(1, 1), 0, gli::u8vec4(255, 127,   0, 255));
			Texture.store(gli::dim2_t(2, 2), 0, gli::u8vec4(  0, 127, 255, 255));
			Texture.store(gli::dim2_t(3, 2), 0, gli::u8vec4(  0, 127, 255, 255));
			Texture.store(gli::dim2_t(2, 3), 0, gli::u8vec4(  0, 127, 255, 255));
			Texture.store(gli::dim2_t(3, 3), 0, gli::u8vec4(  0, 127, 255, 255));

			gli::sampler2D<float> Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, gli::vec4(1.0f, 0.5f, 0.0f, 1.0f));

			gli::vec4 SampleA = Sampler.texture_lod(gli::fsampler2D::samplecoord_type(0.25f), 0.0f);
			Error += gli::all(gli::epsilonEqual(SampleA, gli::vec4(1.0f, 0.5f, 0.0f, 1.0f), 0.01f)) ? 0 : 1;

			gli::vec4 SampleB = Sampler.texture_lod(gli::fsampler2D::samplecoord_type(0.8f), 0.0f);
			Error += gli::all(gli::epsilonEqual(SampleB, gli::vec4(0.0f, 0.5f, 1.0f, 1.0f), 0.01f)) ? 0 : 1;

			gli::vec4 SampleC = Sampler.texture_lod(gli::fsampler2D::samplecoord_type(0.20f, 0.8f), 0.0f);
			Error += gli::all(gli::epsilonEqual(SampleC, gli::vec4(0.0f, 0.0f, 0.0f, 1.0f), 0.01f)) ? 0 : 1;
		}

		return Error;
	}
}//namespace texture_lod

namespace sampler_type
{
	int test()
	{
		int Error = 0;

		{
			gli::texture2D Texture(gli::FORMAT_RGBA8_UNORM, gli::texture2D::dim_type(4), 1);
			gli::sampler2D<float> Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR);
		}

		{
			gli::texture2D Texture(gli::FORMAT_RGBA8_UNORM, gli::texture2D::dim_type(4), 1);
			gli::sampler2D<double> Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR);
		}

		{
			gli::texture2D Texture(gli::FORMAT_RGBA8_UNORM, gli::texture2D::dim_type(4), 1);
			gli::sampler2D<int> Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR);
		}

		return Error;
	}
}//namespace sampler_type

int main()
{
	int Error(0);

	Error += texture_lod::test();
	Error += load::test();
	Error += sampler_type::test();

	return Error;
}

